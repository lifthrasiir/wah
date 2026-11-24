/-
Model of `wah_check_lowered_code` in wah.h (P3a-P3d), and its soundness against an abstract machine of the
interpreter.

Code is a sequence of decoded instructions, indexed by instruction rather than by byte offset; branch targets that
are not instruction starts are out of range. Instructions only carry what the checker needs: numeric operators,
memory, table and global accesses, typed select and the like are `op` with the types that they pop and push (as
resolved against the module by the dumper of the bridge), bulk operations that can be suspended midway are `opMap`
with their ref maps, SIMD instructions with lane immediates (which the interpreter indexes vectors by) are `lanes`
with the lane count, and calls are `call` with the types of the callee. Types are `wah_type_t` after
`wah_check_norm`, with the subtyping `sub` of the module (`wah_type_is_subtype`), which `SubOK` requires to be a
preorder that keeps references apart from other types and is monotone under `nn` (`WAH_TYPE_AS_NON_NULL`).
Struct and array types have layouts (`lay`: field offsets, types and mutability), which `LayOK` requires to agree
with subtyping: fields of a supertype are at the same offsets of its subtypes with the same storage, of subtypes
for immutable ones and of the same type (both ways) for mutable ones, and only bottoms are below bottoms.

The machine (`steps`) runs on types of values only: conditions and nulls are not known, so it takes every branch. A
value may have a subtype of what the checker has for it, as stack maps join paths by subtyping. This is enough for
the properties that the interpreter relies on without checking them: operands have their types, lanes are in range,
branch targets and fall-throughs are instructions, `keep + drop` values are there, the stack never grows beyond
`maxDepth` (for which the interpreter has made room), locals keep their types, results are there on returns, ref
maps at suspension points list exactly the slots of references (`exactMap`), as they are the GC roots, and calls are
right after a POLL, whose ref map scans the frame while the callee runs. Objects are accessed by layouts of the
types of values, which may be subtypes of what the checker has: accessors find fields at their offsets with the
storage that they access, writes need mutable fields and values of their types, and array.copy and array.init_elem
copy elements of subtypes. try_tables push handlers with their catch clauses and the stack height, which
END_TRY_TABLE and TRIM_HANDLERS pop, and anything that throws (throw, throw_ref and calls, after taking the
arguments) may go to any clause of any handler, rewinding the stack to below the height there, which has to be
there. Going wrong is `Out.wrong`; traps are allowed.

The checker (`check`) mirrors the C code: pass 1 checks targets of all instructions, and pass 2 walks from each join
point with the operand stack recorded there, merging the stack at each branch target and at fall-throughs into join
points (`T`), where it must equal the stack recorded first. Join points at `end` and at stack maps (`ann`) take
subtypes of the types annotated there on top of the stack, and record the annotated types instead. Join points
record handlers as well, which have to be equal (by their clauses), except that labels followed by TRIM_HANDLERS
(`trimAt`) only record those that it keeps, and `end` none. Paths keep the other handlers until TRIM_HANDLERS, as
the machine does (`HRel`), so METERs before it may only branch to slow islands that trim them too.

`check_sound`: no state reachable from the entry of code accepted by `check` goes wrong, given `SubOK` and `LayOK`,
which `tableOK_sound` and `layOK_sound` reduce to finite checks of the subtyping table.
-/
namespace Wah.Lowered

abbrev VT := Int

def i32 : VT := -2

/-- `WAH_TYPE_IS_REF` -/
def isRef (t : VT) : Bool := decide ((t ≤ -31 ∧ -54 ≤ t) ∨ 0 ≤ t)

/-- `WAH_TYPE_AS_NON_NULL` -/
def nn (t : VT) : VT := t - t % 2

/-- `WAH_TYPE_NONE` and `WAH_TYPE_NULLREF`, the bottoms of struct and array types, whose references are null. -/
def nullOnly (t : VT) : Bool := t == -52 || t == -51

/-- `wah_check_storage`: the size of a field, with 0x100 for packed ones and 0x200 for references (and others). -/
def storage (t : VT) : Nat :=
  if t = -16 then 0x101 else if t = -18 then 0x102 else if t = -2 ∨ t = -6 then 4
  else if t = -4 ∨ t = -8 then 8 else if t = -10 then 16 else 0x208

/-- `wah_check_accessible`: whether accessors of the storage `acc` (`wah_check_access`) read or write fields of `t`. -/
def accessible (acc : Nat) (t : VT) : Bool := (if storage t = 0x208 then 8 else storage t) == acc

/-- `CK_UNPACKED`: packed fields are i32 on the stack. -/
def unp (t : VT) : VT := if storage t = 0x101 ∨ storage t = 0x102 then i32 else t

structure Field where
  off : Nat
  ty : VT
  mutable : Bool

/-- A struct or array type: `wah_type_def_t` with field offsets of `wah_repr_info_t`. Arrays have one field. -/
structure Comp where
  isStruct : Bool
  fields : List Field

/-- A catch clause of a try_table: its kind and tag (by which the checker compares handlers), what it pushes (bottom
first: the payload of the tag for `catch` and `catch_ref`, then an exnref for `catch_ref` and `catch_all_ref`), its
target, and how many values below the try_table it drops. -/
structure Clause where
  kind : Nat
  tag : Nat
  push : List VT
  target : Nat
  drop : Nat
  deriving DecidableEq

/-- An active exception handler: the catch clauses of its try_table and the stack height there. -/
abbrev H := List Clause × Nat

/-- `WAH_MAX_CONTROL_DEPTH`, which bounds active handlers in the checker. -/
def maxHandlers : Nat := 256

inductive Instr
  | const (t : VT)
  | op (args res : List VT)
  | opMap (map : Option (List Nat)) (args res : List VT)
  | lanes (ks : List Nat) (n : Nat) (args res : List VT)
  | call (args res : List VT)
  | retCall (args res : List VT)
  | drop | select
  | localGet (i : Nat) | localSet (i : Nat) | localTee (i : Nat)
  | refIsNull | refAsNonNull
  | br (t keep drop : Nat) | brIf (t keep drop : Nat) | brTable (keep : Nat) (recs : List (Nat × Nat))
  | brOnNull (t keep drop : Nat) | brOnNonNull (t keep drop : Nat)
  | if_ (t : Nat) | else_ (t : Nat) | meter (t : Nat)
  | poll (map : Option (List Nat)) | tick (map : Option (List Nat))
  | end_ | ret | unreachable | bad
  | refEq | refCast (t : VT)
  | brOnCast (onFail nul : Bool) (dst : VT) (t keep drop : Nat)
  | structGet (acc off : Nat) | structSet (acc off : Nat) | arrayGet (acc : Nat) | arraySet (acc : Nat)
  | arrayCopy (map : Option (List Nat)) | arrayInitElem (map : Option (List Nat)) (seg : VT)
  | tryTable (cs : List Clause) | endTry | trim (depth : Nat) | throw (args : List VT)
  deriving Inhabited

structure Fn where
  code : Array Instr
  locals : List VT
  results : List VT
  maxDepth : Nat
  maps : List (Nat × List VT)
  sub : VT → VT → Bool
  /-- Struct and array types by type index. -/
  lay : Nat → Option Comp

structure SubOK (F : Fn) : Prop where
  refl : ∀ a, F.sub a a = true
  trans : ∀ a b c, F.sub a b = true → F.sub b c = true → F.sub a c = true
  ref : ∀ a b, F.sub a b = true → isRef a = isRef b
  nn : ∀ a b, F.sub a b = true → isRef a = true → F.sub (nn a) (nn b) = true

/-- Pops the types `ts` (top first) off the stack `s` (top first), taking their subtypes. -/
def popT (F : Fn) : List VT → List VT → Option (List VT)
  | [], s => some s
  | t :: ts, x :: s => if F.sub x t then popT F ts s else none
  | _ :: _, [] => none

/-- Whether types are pointwise subtypes. -/
def leB (F : Fn) : List VT → List VT → Bool
  | [], [] => true
  | x :: xs, y :: ys => F.sub x y && leB F xs ys
  | _, _ => false

/-- Whether position `p` (from the bottom) of the stack `s` (top first) has a reference. -/
def refAt (s : List VT) (p : Nat) : Bool := decide (p < s.length) && isRef (s.getD (s.length - 1 - p) 0)

def refCount (s : List VT) : Nat := (s.filter isRef).length

/-! ## Objects -/

/-- `CK_COMP`: the struct or array type of `t`, `some none` for bottoms (whose accessors trap, so that paths end), or
`none` for others. -/
def compX (F : Fn) (t : VT) (isStruct : Bool) : Option (Option Comp) :=
  if nullOnly t then some none
  else if 0 ≤ t then match F.lay (t.toNat / 2) with
    | some c => if c.isStruct = isStruct then some (some c) else none
    | none => none
  else none

/-- `wah_check_field_at`: the first field at `off`. -/
def fieldAt (off : Nat) : List Field → Option Field
  | [] => none
  | f :: fs => if f.off = off then some f else fieldAt off fs

/-- A field of a subtype that agrees with the field of its supertype at the same position. -/
def fieldB (F : Fn) (a b : Field) : Bool :=
  a.off == b.off && storage a.ty == storage b.ty && F.sub a.ty b.ty && (!b.mutable || (a.mutable && F.sub b.ty a.ty))

def prefB (F : Fn) : List Field → List Field → Bool
  | _, [] => true
  | a :: as, b :: bs => fieldB F a b && prefB F as bs
  | [], _ :: _ => false

def extB (F : Fn) (a b : Comp) : Bool := a.isStruct == b.isStruct && prefB F a.fields b.fields

/-- What `LayOK` requires of a pair of a subtype `a` and a supertype `b`. -/
def layPairOK (F : Fn) (a b : VT) : Bool :=
  (!nullOnly b || nullOnly a) &&
  (nullOnly b || decide (b < 0) || match F.lay (b.toNat / 2) with
    | none => true
    | some cb => nullOnly a || (decide (0 ≤ a) && match F.lay (a.toNat / 2) with
      | some ca => extB F ca cb
      | none => false))

/-- Objects of a subtype can be accessed as those of its supertypes (`wah_check_layouts`, `LayoutSub.lean`), and
bottoms only have bottoms below them. -/
def LayOK (F : Fn) : Prop := ∀ a b, F.sub a b = true → layPairOK F a b = true

/-! ## The machine -/

/-- A ref map that lists the slots of references exactly once each. -/
def exactMap (ps : List Nat) (s : List VT) : Bool :=
  decide ps.Nodup && ps.all (refAt s) && ps.length == refCount s

structure St where
  pc : Nat
  stk : List VT
  polled : Bool
  /-- Active handlers of the frame, innermost last. -/
  hs : List H

inductive Out
  | next (st : St) | done | trap | wrong

def go (pc : Nat) (s : List VT) (hs : List H) (p : Bool := false) : Out := .next ⟨pc, s, p, hs⟩

def pushO (F : Fn) (pc : Nat) (ts s : List VT) (hs : List H) : Out :=
  if s.length + ts.length ≤ F.maxDepth then go pc (ts.reverse ++ s) hs else .wrong

/-- Branches as `WAH_DROP_KEEP` does: the top `k` values stay and the `d` below them go. -/
def jump (t k d : Nat) (s : List VT) (hs : List H) : Out :=
  if k + d ≤ s.length then go t (s.take k ++ s.drop (k + d)) hs else .wrong

def opO (F : Fn) (pc : Nat) (args res s : List VT) (hs : List H) : List Out :=
  match popT F args.reverse s with
  | some s0 => [.trap, pushO F (pc + 1) res s0 hs]
  | none => [.wrong]

/-- Catch clauses of the handlers `hs` (after `pre`), each with the handlers outside of its own, which stay active
at its target, and the stack height at its try_table. -/
def catchesFrom : List H → List H → List (List H × Nat × Clause)
  | _, [] => []
  | pre, x :: xs => x.1.map (fun c => (pre, x.2, c)) ++ catchesFrom (pre ++ [x]) xs

def catches (hs : List H) : List (List H × Nat × Clause) := catchesFrom [] hs

/-- Throws with the stack `s`: the exception leaves the frame, or any clause catches it, as tags are not known. The
stack is rewound to `c.drop` values below the height `h` at the try_table, which must still be there, and gets
what the clause pushes. -/
def throwO (F : Fn) (s : List VT) (hs : List H) : List Out :=
  .trap :: (catches hs).map fun (pre, h, c) =>
    if c.drop ≤ h ∧ h - c.drop ≤ s.length ∧ h - c.drop + c.push.length ≤ F.maxDepth then
      go c.target (c.push.reverse ++ s.drop (s.length - (h - c.drop))) pre
    else .wrong

def mapO (m : Option (List Nat)) (s : List VT) : Bool :=
  match m with
  | some ps => exactMap ps s
  | none => false

/-- Accesses an object of type `x` by `k`: traps on null, and goes wrong unless it is a struct (or an array). -/
def objO (F : Fn) (x : VT) (isStruct : Bool) (k : Comp → List Out) : List Out :=
  match compX F x isStruct with
  | some (some c) => .trap :: k c
  | some none => [.trap]
  | none => [.wrong]

/-- What `br_on_cast` leaves when the cast fails: not null if nulls pass. -/
def castSrc (nul : Bool) (x : VT) : VT := if nul then nn x else x

def stepI (F : Fn) (pc : Nat) (s : List VT) (hs : List H) (lp : Bool) : Instr → List Out
  | .const t => [pushO F (pc + 1) [t] s hs]
  | .op args res => opO F pc args res s hs
  | .opMap m args res => if mapO m s then opO F pc args res s hs else [.wrong]
  | .lanes ks n args res => if ks.all (· < n) then opO F pc args res s hs else [.wrong]
  | .call args res => if lp then match popT F args.reverse s with
      | some s0 => .trap :: pushO F (pc + 1) res s0 hs :: throwO F s0 hs
      | none => [.wrong]
    else [.wrong]
  | .retCall args res => match popT F args.reverse s with
    | some _ => if leB F res F.results then [.trap, .done] else [.wrong]
    | none => [.wrong]
  | .drop => match s with
    | _ :: s0 => [go (pc + 1) s0 hs]
    | [] => [.wrong]
  | .select => match s with
    | c :: a :: b :: s0 => if F.sub c i32 then [go (pc + 1) (a :: s0) hs, go (pc + 1) (b :: s0) hs] else [.wrong]
    | _ => [.wrong]
  | .localGet i => match F.locals[i]? with
    | some t => [pushO F (pc + 1) [t] s hs]
    | none => [.wrong]
  | .localSet i => match F.locals[i]?, s with
    | some t, x :: s0 => if F.sub x t then [go (pc + 1) s0 hs] else [.wrong]
    | _, _ => [.wrong]
  | .localTee i => match F.locals[i]?, s with
    | some t, x :: s0 => if F.sub x t then [pushO F (pc + 1) [t] s0 hs] else [.wrong]
    | _, _ => [.wrong]
  | .refIsNull => match s with
    | x :: s0 => if isRef x then [pushO F (pc + 1) [i32] s0 hs] else [.wrong]
    | [] => [.wrong]
  | .refAsNonNull => match s with
    | x :: s0 => if isRef x then [.trap, go (pc + 1) (nn x :: s0) hs] else [.wrong]
    | [] => [.wrong]
  | .br t k d => [jump t k d s hs]
  | .brIf t k d => match s with
    | c :: s0 => if F.sub c i32 then [go (pc + 1) s0 hs, jump t k d s0 hs] else [.wrong]
    | _ => [.wrong]
  | .brTable k recs => match s with
    | c :: s0 => if F.sub c i32 && !recs.isEmpty then recs.map (fun r => jump r.1 k r.2 s0 hs) else [.wrong]
    | _ => [.wrong]
  | .brOnNull t k d => match s with
    | x :: s0 => if isRef x then [jump t k d s0 hs, go (pc + 1) (nn x :: s0) hs] else [.wrong]
    | [] => [.wrong]
  | .brOnNonNull t k d => match s with
    | x :: s0 => if isRef x then [jump t k d (nn x :: s0) hs, go (pc + 1) s0 hs] else [.wrong]
    | [] => [.wrong]
  | .if_ t => match s with
    | c :: s0 => if F.sub c i32 then [go (pc + 1) s0 hs, go t s0 hs] else [.wrong]
    | _ => [.wrong]
  | .else_ t => [go t s hs]
  | .meter t => [go (pc + 1) s hs, go t s hs]
  | .poll m => if mapO m s then [go (pc + 1) s hs true] else [.wrong]
  | .tick m => if mapO m s then [go (pc + 1) s hs] else [.wrong]
  | .end_ => if leB F s F.results.reverse then [.done] else [.wrong]
  | .ret => if (popT F F.results.reverse s).isSome then [.done] else [.wrong]
  | .unreachable => [.trap]
  | .bad => [.wrong]
  | .refEq => match s with
    | x :: y :: s0 => if isRef x && isRef y then [pushO F (pc + 1) [i32] s0 hs] else [.wrong]
    | _ => [.wrong]
  | .refCast t => match s with
    | x :: s0 => if isRef x then [.trap, pushO F (pc + 1) [t] s0 hs] else [.wrong]
    | [] => [.wrong]
  | .brOnCast onFail nul dst t k d => match s with
    | x :: s0 => if isRef x then
        [jump t k d ((if onFail then castSrc nul x else dst) :: s0) hs,
         go (pc + 1) ((if onFail then dst else castSrc nul x) :: s0) hs]
      else [.wrong]
    | [] => [.wrong]
  | .structGet acc off => match s with
    | x :: s0 => objO F x true fun c => match fieldAt off c.fields with
      | some f => if accessible acc f.ty then [pushO F (pc + 1) [unp f.ty] s0 hs] else [.wrong]
      | none => [.wrong]
    | [] => [.wrong]
  | .structSet acc off => match s with
    | v :: x :: s0 => objO F x true fun c => match fieldAt off c.fields with
      | some f => if accessible acc f.ty && f.mutable && F.sub v (unp f.ty) then [go (pc + 1) s0 hs] else [.wrong]
      | none => [.wrong]
    | _ => [.wrong]
  | .arrayGet acc => match s with
    | i :: x :: s0 => if F.sub i i32 then objO F x false fun c => match c.fields with
        | f :: _ => if accessible acc f.ty then [pushO F (pc + 1) [unp f.ty] s0 hs] else [.wrong]
        | [] => [.wrong]
      else [.wrong]
    | _ => [.wrong]
  | .arraySet acc => match s with
    | v :: i :: x :: s0 => if F.sub i i32 then objO F x false fun c => match c.fields with
        | f :: _ => if accessible acc f.ty && f.mutable && F.sub v (unp f.ty) then [go (pc + 1) s0 hs] else [.wrong]
        | [] => [.wrong]
      else [.wrong]
    | _ => [.wrong]
  | .arrayCopy m => match s with
    | n :: so :: y :: dof :: x :: s0 =>
      if mapO m s && F.sub n i32 && F.sub so i32 && F.sub dof i32 then
        objO F x false fun cd => objO F y false fun cs => match cd.fields, cs.fields with
        | fd :: _, fs :: _ => if fd.mutable && F.sub fs.ty fd.ty then [go (pc + 1) s0 hs] else [.wrong]
        | _, _ => [.wrong]
      else [.wrong]
    | _ => [.wrong]
  | .arrayInitElem m seg => match s with
    | n :: so :: dof :: x :: s0 =>
      if mapO m s && F.sub n i32 && F.sub so i32 && F.sub dof i32 then
        objO F x false fun c => match c.fields with
        | f :: _ => if f.mutable && storage f.ty == 0x208 && F.sub seg f.ty then [go (pc + 1) s0 hs] else [.wrong]
        | [] => [.wrong]
      else [.wrong]
    | _ => [.wrong]
  | .tryTable cs => [go (pc + 1) s (hs ++ [(cs, s.length)])]
  | .endTry => if hs.isEmpty then [.wrong] else [go (pc + 1) s hs.dropLast]
  | .trim d => if d ≤ hs.length then [go (pc + 1) s (hs.take d)] else [.trap]
  | .throw args => match popT F args.reverse s with
    | some s0 => throwO F s0 hs
    | none => [.wrong]

def steps (F : Fn) (st : St) : List Out :=
  match F.code[st.pc]? with
  | some i => stepI F st.pc st.stk st.hs st.polled i
  | none => [.wrong]

inductive Reach (F : Fn) : St → Prop
  | init : Reach F ⟨0, [], false, []⟩
  | step {st st'} : Reach F st → Out.next st' ∈ steps F st → Reach F st'

/-! ## The checker -/

/-- An edge to a target, with the stack and active handlers there. -/
abbrev E := Nat × List VT × List H

/-- What an instruction does to the operand stack and active handlers: branch edges, and the stack and handlers
after it if it falls through. -/
inductive Xf
  | fail
  | stop (es : List E)
  | next (es : List E) (s : List VT) (hs : List H) (p : Bool)

def pushX (F : Fn) (ts s : List VT) : Option (List VT) :=
  if s.length + ts.length ≤ F.maxDepth then some (ts.reverse ++ s) else none

def edge (t k d : Nat) (s : List VT) (hs : List H) : Option E :=
  if k + d ≤ s.length then some (t, s.take k ++ s.drop (k + d), hs) else none

def edges (k : Nat) (s : List VT) (hs : List H) : List (Nat × Nat) → Option (List E)
  | [] => some []
  | r :: rs => match edge r.1 k r.2 s hs, edges k s hs rs with
    | some e, some es => some (e :: es)
    | _, _ => none

def nextX (F : Fn) (ts s : List VT) (hs : List H) : Xf :=
  match pushX F ts s with
  | some s' => .next [] s' hs false
  | none => .fail

def opX (F : Fn) (args res s : List VT) (hs : List H) : Xf :=
  match popT F args.reverse s with
  | some s0 => nextX F res s0 hs
  | none => .fail

/-- The stack at the target of the clause `c` of a handler at the height `h` (`h - c.drop` is unsigned in C, so
that it fails if `c.drop > h`). -/
def catchS (F : Fn) (h : Nat) (c : Clause) (s : List VT) : Option (List VT) :=
  if c.drop ≤ h ∧ h - c.drop ≤ s.length ∧ h - c.drop + c.push.length ≤ F.maxDepth then
    some (c.push.reverse ++ s.drop (s.length - (h - c.drop)))
  else none

def optAll {α : Type} : List (Option α) → Option (List α)
  | [] => some []
  | some a :: l => (optAll l).map (a :: ·)
  | none :: _ => none

/-- `CK_THROW`: edges to every catch clause of the handlers `hs`, from the stack `s`. -/
def catchesX (F : Fn) (s : List VT) (hs : List H) : Option (List E) :=
  optAll ((catches hs).map fun q => (catchS F q.2.1 q.2.2 s).map fun s' => (q.2.2.target, s', q.1))

/-- `wah_check_trim_depth`: the depth of TRIM_HANDLERS at `pc` or after POLLs, METERs and TICKs there, looking at
`n` instructions at most. -/
def trimAtN (F : Fn) : Nat → Nat → Option Nat
  | 0, _ => none
  | n + 1, pc => match F.code[pc]? with
    | some (.trim d) => some d
    | some (.poll _) | some (.tick _) | some (.meter _) => trimAtN F n (pc + 1)
    | _ => none

def trimAt (F : Fn) (pc : Nat) : Option Nat := trimAtN F F.code.size pc

/-- `wah_check_ref_map`: positions strictly decrease from the top, have references, and are all of them. -/
def decr (s : List VT) (below : Nat) : List Nat → Bool
  | [] => true
  | p :: ps => decide (p < below) && refAt s p && decr s p ps

def mapX (m : Option (List Nat)) (s : List VT) : Bool :=
  match m with
  | some ps => decr s s.length ps && ps.length == refCount s
  | none => false

/-- Checks an access to an object of type `t` by `k`; paths end at bottoms. -/
def objX (F : Fn) (t : VT) (isStruct : Bool) (k : Comp → Xf) : Xf :=
  match compX F t isStruct with
  | some (some c) => k c
  | some none => .stop []
  | none => .fail

def xfer (F : Fn) (pc : Nat) (s : List VT) (hs : List H) (p : Bool) : Instr → Xf
  | .const t => nextX F [t] s hs
  | .op args res => opX F args res s hs
  | .opMap m args res => if mapX m s then opX F args res s hs else .fail
  | .lanes ks n args res => if ks.all (· < n) then opX F args res s hs else .fail
  | .call args res => if p then match popT F args.reverse s with
      | some s0 => match catchesX F s0 hs, pushX F res s0 with
        | some es, some s' => .next es s' hs false
        | _, _ => .fail
      | none => .fail
    else .fail
  | .retCall args res => match popT F args.reverse s with
    | some _ => if leB F res F.results then .stop [] else .fail
    | none => .fail
  | .drop => match s with
    | _ :: s0 => .next [] s0 hs false
    | [] => .fail
  | .select => match s with
    | c :: a :: b :: s0 => if F.sub c i32 && a == b then .next [] (a :: s0) hs false else .fail
    | _ => .fail
  | .localGet i => match F.locals[i]? with
    | some t => nextX F [t] s hs
    | none => .fail
  | .localSet i => match F.locals[i]?, s with
    | some t, x :: s0 => if F.sub x t then .next [] s0 hs false else .fail
    | _, _ => .fail
  | .localTee i => match F.locals[i]?, s with
    | some t, x :: s0 => if F.sub x t then nextX F [t] s0 hs else .fail
    | _, _ => .fail
  | .refIsNull => match s with
    | x :: s0 => if isRef x then nextX F [i32] s0 hs else .fail
    | [] => .fail
  | .refAsNonNull => match s with
    | x :: s0 => if isRef x then .next [] (nn x :: s0) hs false else .fail
    | [] => .fail
  | .br t k d => match edge t k d s hs with
    | some e => .stop [e]
    | none => .fail
  | .brIf t k d => match s with
    | c :: s0 => if F.sub c i32 then match edge t k d s0 hs with
        | some e => .next [e] s0 hs false
        | none => .fail
      else .fail
    | _ => .fail
  | .brTable k recs => match s with
    | c :: s0 => if F.sub c i32 && !recs.isEmpty then match edges k s0 hs recs with
        | some es => .stop es
        | none => .fail
      else .fail
    | _ => .fail
  | .brOnNull t k d => match s with
    | x :: s0 => if isRef x then match edge t k d s0 hs with
        | some e => .next [e] (nn x :: s0) hs false
        | none => .fail
      else .fail
    | [] => .fail
  | .brOnNonNull t k d => match s with
    | x :: s0 => if isRef x then match edge t k d (nn x :: s0) hs with
        | some e => .next [e] s0 hs false
        | none => .fail
      else .fail
    | [] => .fail
  | .if_ t => match s with
    | c :: s0 => if F.sub c i32 then .next [(t, s0, hs)] s0 hs false else .fail
    | _ => .fail
  | .else_ t => .stop [(t, s, hs)]
  | .meter t => if (trimAt F pc).isNone || (trimAt F t).isSome then .next [(t, s, hs)] s hs false else .fail
  | .poll m => if mapX m s then .next [] s hs true else .fail
  | .tick m => if mapX m s then .next [] s hs false else .fail
  | .end_ => if leB F s F.results.reverse then .stop [] else .fail
  | .ret => if (popT F F.results.reverse s).isSome then .stop [] else .fail
  | .unreachable => .stop []
  | .bad => .fail
  | .refEq => match s with
    | x :: y :: s0 => if isRef x && isRef y then nextX F [i32] s0 hs else .fail
    | _ => .fail
  | .refCast t => match s with
    | x :: s0 => if isRef x then nextX F [t] s0 hs else .fail
    | [] => .fail
  | .brOnCast onFail nul dst t k d => match s with
    | x :: s0 => if isRef x then
        match edge t k d ((if onFail then castSrc nul x else dst) :: s0) hs with
        | some e => .next [e] ((if onFail then dst else castSrc nul x) :: s0) hs false
        | none => .fail
      else .fail
    | [] => .fail
  | .structGet acc off => match s with
    | x :: s0 => objX F x true fun c => match fieldAt off c.fields with
      | some f => if accessible acc f.ty then nextX F [unp f.ty] s0 hs else .fail
      | none => .fail
    | [] => .fail
  | .structSet acc off => match s with
    | v :: x :: s0 => objX F x true fun c => match fieldAt off c.fields with
      | some f => if accessible acc f.ty && f.mutable && F.sub v (unp f.ty) then .next [] s0 hs false else .fail
      | none => .fail
    | _ => .fail
  | .arrayGet acc => match s with
    | i :: x :: s0 => if F.sub i i32 then objX F x false fun c => match c.fields with
        | f :: _ => if accessible acc f.ty then nextX F [unp f.ty] s0 hs else .fail
        | [] => .fail
      else .fail
    | _ => .fail
  | .arraySet acc => match s with
    | v :: i :: x :: s0 => if F.sub i i32 then objX F x false fun c => match c.fields with
        | f :: _ => if accessible acc f.ty && f.mutable && F.sub v (unp f.ty) then .next [] s0 hs false else .fail
        | [] => .fail
      else .fail
    | _ => .fail
  | .arrayCopy m => match s with
    | n :: so :: y :: dof :: x :: s0 =>
      if mapX m s && F.sub n i32 && F.sub so i32 && F.sub dof i32 && (compX F y false).isSome then
        objX F x false fun cd => objX F y false fun cs => match cd.fields, cs.fields with
        | fd :: _, fs :: _ => if fd.mutable && F.sub fs.ty fd.ty then .next [] s0 hs false else .fail
        | _, _ => .fail
      else .fail
    | _ => .fail
  | .arrayInitElem m seg => match s with
    | n :: so :: dof :: x :: s0 =>
      if mapX m s && F.sub n i32 && F.sub so i32 && F.sub dof i32 then
        objX F x false fun c => match c.fields with
        | f :: _ => if f.mutable && storage f.ty == 0x208 && F.sub seg f.ty then .next [] s0 hs false else .fail
        | [] => .fail
      else .fail
    | _ => .fail
  | .tryTable cs =>
    if decide (hs.length < maxHandlers) && cs.all (fun c => decide (c.drop ≤ s.length)) then
      .next [] s (hs ++ [(cs, s.length)]) false
    else .fail
  | .endTry => if hs.isEmpty then .fail else .next [] s hs.dropLast false
  | .trim d => if d ≤ hs.length then .next [] s (hs.take d) false else .fail
  | .throw args => match popT F args.reverse s with
    | some s0 => match catchesX F s0 hs with
      | some es => .stop es
      | none => .fail
    | none => .fail

/-- Types annotated at a join point: results at `end`, or a stack map, which are on top of the stack bottom first. -/
def ann (F : Fn) (t : Nat) : Option (List VT) :=
  match F.code[t]? with
  | some .end_ => some F.results
  | _ => (F.maps.find? (·.1 == t)).map (·.2)

def annOK (F : Fn) (t : Nat) (s : List VT) : Bool :=
  match ann F t with
  | some a => decide (a.length ≤ s.length) && leB F (s.take a.length) a.reverse
  | none => true

/-- The stack recorded at a join point, with the annotated types on top. -/
def nrm (F : Fn) (t : Nat) (s : List VT) : List VT :=
  match ann F t with
  | some a => a.reverse ++ s.drop a.length
  | none => s

/-- The handlers recorded at a join point: none at `end` (returning drops them), and those that TRIM_HANDLERS keeps
there, which have to be active. -/
def hsAt (F : Fn) (t : Nat) (hs : List H) : Option (List H) :=
  match F.code[t]? with
  | some .end_ => some []
  | _ => match trimAt F t with
    | some d => if d ≤ hs.length then some (hs.take d) else none
    | none => some hs

/-- Recorded stacks and handlers at join points. -/
abbrev Joins := Nat → Option (List VT × List H)

def upd (J : Joins) (t : Nat) (r : List VT × List H) : Joins := fun u => if u = t then some r else J u

/-- `CK_MERGE`: records the stack and handlers at a join point, or checks them against the recorded ones. -/
def mergeM (F : Fn) (jw : Joins × List Nat) (t : Nat) (s : List VT) (hs : List H) : Option (Joins × List Nat) :=
  if annOK F t s then
    match hsAt F t hs with
    | some hs' => match jw.1 t with
      | none => some (upd jw.1 t (nrm F t s, hs'), t :: jw.2)
      | some r => if r = (nrm F t s, hs') then some jw else none
    | none => none
  else none

def mergeAllM (F : Fn) : Joins × List Nat → List E → Option (Joins × List Nat)
  | jw, [] => some jw
  | jw, e :: es => match mergeM F jw e.1 e.2.1 e.2.2 with
    | some jw' => mergeAllM F jw' es
    | none => none

/-- Walks from `pc` with the stack `s`, where `p` tells whether the last instruction was POLL, until the path ends or
falls through into a join point (other than where it started, `first`). -/
def walkM (F : Fn) (T : Nat → Bool) :
    Nat → Joins × List Nat → Nat → List VT → List H → Bool → Bool → Option (Joins × List Nat)
  | 0, _, _, _, _, _, _ => none
  | n + 1, jw, pc, s, hs, p, first => match F.code[pc]? with
    | none => none
    | some i => if !first && T pc then mergeM F jw pc s hs else
      match xfer F pc s hs p i with
      | .fail => none
      | .stop es => mergeAllM F jw es
      | .next es s' hs' p' => match mergeAllM F jw es with
        | some jw' => walkM F T n jw' (pc + 1) s' hs' p' false
        | none => none

/-- The worklist of join points to walk from, last in first out. -/
def loopM (F : Fn) (T : Nat → Bool) (N : Nat) : Nat → Joins → List Nat → Option Joins
  | 0, _, _ => none
  | _ + 1, J, [] => some J
  | n + 1, J, t :: w => match J t with
    | some r => match walkM F T N (J, w) t r.1 r.2 false true with
      | some jw => loopM F T N n jw.1 jw.2
      | none => none
    | none => none

def targets : Instr → List Nat
  | .br t _ _ | .brIf t _ _ | .brOnNull t _ _ | .brOnNonNull t _ _ | .if_ t | .else_ t | .meter t => [t]
  | .brOnCast _ _ _ t _ _ => [t]
  | .brTable _ recs => recs.map (·.1)
  | .tryTable cs => cs.map (·.target)
  | _ => []

/-- Join points: the entry and every branch target. -/
def joinPoints (F : Fn) : Array Bool :=
  F.code.foldl (fun a i => (targets i).foldl (fun a t => a.setIfInBounds t true) a)
    ((Array.replicate F.code.size false).setIfInBounds 0 true)

def check (F : Fn) : Bool :=
  let N := F.code.size + 1
  let jp := joinPoints F
  let T := fun pc => jp[pc]?.getD false
  decide (0 < F.code.size) &&
  F.code.all (fun i => (targets i).all (· < F.code.size)) &&
  F.maps.all (·.1 < F.code.size) && decide (F.maps.map (·.1)).Nodup &&
  match mergeM F (fun _ => none, []) 0 [] [] with
  | some jw => (loopM F T N (N + 1) jw.1 jw.2).isSome
  | none => false

/-! ## Soundness -/

/-! ### Stacks of subtypes -/

/-- The machine stack `a` has subtypes of the checker stack `b`. -/
inductive Le (F : Fn) : List VT → List VT → Prop
  | nil : Le F [] []
  | cons {x y xs ys} : F.sub x y = true → Le F xs ys → Le F (x :: xs) (y :: ys)

section
variable {F : Fn} (hs : SubOK F)
include hs

theorem Le.refl : ∀ (s : List VT), Le F s s
  | [] => .nil
  | x :: s => .cons (hs.refl x) (Le.refl s)

theorem Le.trans : ∀ {a b c : List VT}, Le F a b → Le F b c → Le F a c
  | _, _, _, .nil, .nil => .nil
  | _, _, _, .cons h1 t1, .cons h2 t2 => .cons (hs.trans _ _ _ h1 h2) (Le.trans t1 t2)

omit hs in
theorem leB_iff : ∀ {a b : List VT}, leB F a b = true ↔ Le F a b
  | [], [] => by simp [leB]; exact .nil
  | x :: xs, y :: ys => by
    simp only [leB, Bool.and_eq_true]
    constructor
    · intro ⟨h1, h2⟩; exact .cons h1 (leB_iff.mp h2)
    · intro h; cases h with | cons h1 h2 => exact ⟨h1, leB_iff.mpr h2⟩
  | [], _ :: _ => by simp [leB]; intro h; cases h
  | _ :: _, [] => by simp [leB]; intro h; cases h

theorem popT_le : ∀ {ts : List VT} {M S S0 : List VT}, Le F M S → popT F ts S = some S0 →
    ∃ M0, popT F ts M = some M0 ∧ Le F M0 S0
  | [], _, _, _, hl, h => by simp only [popT, Option.some.injEq] at h ⊢; subst h; exact ⟨_, rfl, hl⟩
  | t :: ts, M, S, S0, hl, h => by
    cases hl with
    | nil => simp [popT] at h
    | cons h1 h2 =>
      simp only [popT] at h ⊢
      split at h
      · rename_i e
        simp only [hs.trans _ _ _ h1 e, ite_true]
        exact popT_le h2 h
      · cases h

theorem popT_none_le {ts : List VT} {M S S0 : List VT} (hl : Le F M S) (h : popT F ts S = some S0) :
    (popT F ts M).isSome = true := by
  obtain ⟨_, h', _⟩ := popT_le hs hl h
  simp [h']

end

theorem Le.length {F : Fn} : ∀ {a b : List VT}, Le F a b → a.length = b.length
  | _, _, .nil => rfl
  | _, _, .cons _ t => by simp [Le.length t]

theorem Le.take {F : Fn} : ∀ (k : Nat) {a b : List VT}, Le F a b → Le F (a.take k) (b.take k)
  | 0, _, _, _ => by simp; exact .nil
  | _ + 1, _, _, .nil => by simp; exact .nil
  | k + 1, _, _, .cons h t => by simp; exact .cons h (Le.take k t)

theorem Le.drop {F : Fn} : ∀ (k : Nat) {a b : List VT}, Le F a b → Le F (a.drop k) (b.drop k)
  | 0, _, _, h => by simpa using h
  | _ + 1, _, _, .nil => by simp; exact .nil
  | k + 1, _, _, .cons _ t => by simpa using Le.drop k t

theorem Le.append {F : Fn} : ∀ {a b c d : List VT}, Le F a b → Le F c d → Le F (a ++ c) (b ++ d)
  | _, _, _, _, .nil, h => h
  | _, _, _, _, .cons h t, h2 => .cons h (Le.append t h2)

theorem Le.refs {F : Fn} (hs : SubOK F) : ∀ {a b : List VT}, Le F a b →
    refCount a = refCount b ∧ ∀ i, isRef (a.getD i 0) = isRef (b.getD i 0)
  | _, _, .nil => ⟨rfl, fun _ => rfl⟩
  | x :: xs, y :: ys, .cons h t => by
    obtain ⟨c, g⟩ := Le.refs hs t
    refine ⟨?_, fun i => ?_⟩
    · simp only [refCount, List.filter_cons, hs.ref _ _ h] at c ⊢
      split <;> simp [c]
    · cases i with
      | zero => simpa using hs.ref _ _ h
      | succ i => simpa using g i

theorem refAt_le {F : Fn} (hs : SubOK F) {a b : List VT} (h : Le F a b) (p : Nat) : refAt a p = refAt b p := by
  simp only [refAt, h.length, (Le.refs hs h).2]

theorem decr_spec {s : List VT} : ∀ {below : Nat} {ps : List Nat}, decr s below ps = true →
    ps.Nodup ∧ (∀ p ∈ ps, refAt s p = true ∧ p < below)
  | _, [], _ => ⟨List.nodup_nil, fun _ h => by cases h⟩
  | below, p :: ps, h => by
    simp only [decr, Bool.and_eq_true, decide_eq_true_eq] at h
    obtain ⟨nd, hp⟩ := decr_spec h.2
    refine ⟨List.nodup_cons.mpr ⟨fun hm => by have := (hp p hm).2; omega, nd⟩, fun q hq => ?_⟩
    rcases List.mem_cons.mp hq with e | e
    · subst e; exact ⟨h.1.2, h.1.1⟩
    · have := hp q e; exact ⟨this.1, by omega⟩

theorem mapX_exact {F : Fn} (hs : SubOK F) {m : Option (List Nat)} {M S : List VT} (hl : Le F M S)
    (h : mapX m S = true) : mapO m M = true := by
  cases m with
  | none => simp [mapX] at h
  | some ps =>
    simp only [mapX, Bool.and_eq_true, beq_iff_eq] at h
    obtain ⟨nd, hp⟩ := decr_spec h.1
    simp only [mapO, exactMap, Bool.and_eq_true, decide_eq_true_eq, beq_iff_eq, List.all_eq_true]
    refine ⟨⟨nd, fun p hm => by rw [refAt_le hs hl]; exact (hp p hm).1⟩, ?_⟩
    rw [(Le.refs hs hl).1]; exact h.2

theorem le_nrm {F : Fn} (hs : SubOK F) {t : Nat} {M S : List VT} (ha : annOK F t S = true) (hl : Le F M S) :
    Le F M (nrm F t S) := by
  simp only [annOK, nrm] at ha ⊢
  split at ha
  · rename_i a e
    simp only [Bool.and_eq_true, decide_eq_true_eq] at ha
    have h1 := leB_iff.mp ha.2
    have := Le.append (Le.trans hs (Le.take a.length hl) h1) (Le.drop a.length hl)
    simpa using this
  · exact hl

/-! ### The worklist computes joins that every walk agrees with -/

/-- The checks of a walk against final joins. -/
def mergeOK (F : Fn) (J : Joins) (t : Nat) (s : List VT) (hs : List H) : Bool :=
  annOK F t s && match hsAt F t hs with
    | some hs' => decide (J t = some (nrm F t s, hs'))
    | none => false

def wkB (F : Fn) (T : Nat → Bool) (J : Joins) : Nat → Nat → List VT → List H → Bool → Bool → Bool
  | 0, _, _, _, _, _ => false
  | n + 1, pc, s, hs, p, first => match F.code[pc]? with
    | none => false
    | some i => if !first && T pc then mergeOK F J pc s hs else
      match xfer F pc s hs p i with
      | .fail => false
      | .stop es => es.all (fun e => mergeOK F J e.1 e.2.1 e.2.2)
      | .next es s' hs' p' => es.all (fun e => mergeOK F J e.1 e.2.1 e.2.2) && wkB F T J n (pc + 1) s' hs' p' false

def Ext (J J' : Joins) : Prop := ∀ t r, J t = some r → J' t = some r

theorem Ext.refl (J : Joins) : Ext J J := fun _ _ h => h
theorem Ext.trans {J J' J'' : Joins} (a : Ext J J') (b : Ext J' J'') : Ext J J'' := fun t r h => b t r (a t r h)

theorem mergeOK_mono {F : Fn} {J J' : Joins} (h : Ext J J') {t s hs} (hm : mergeOK F J t s hs = true) :
    mergeOK F J' t s hs = true := by
  simp only [mergeOK, Bool.and_eq_true] at hm ⊢
  refine ⟨hm.1, ?_⟩
  have := hm.2
  split at this
  · simp only [decide_eq_true_eq] at this ⊢
    exact h _ _ this
  · cases this

theorem all_mergeOK_mono {F : Fn} {J J' : Joins} (h : Ext J J') {es : List E}
    (hm : es.all (fun e => mergeOK F J e.1 e.2.1 e.2.2) = true) :
    es.all (fun e => mergeOK F J' e.1 e.2.1 e.2.2) = true := by
  simp only [List.all_eq_true] at hm ⊢
  exact fun e he => mergeOK_mono h (hm e he)

theorem wkB_mono {F : Fn} {T} {J J' : Joins} (h : Ext J J') :
    ∀ n pc s hs p first, wkB F T J n pc s hs p first = true → wkB F T J' n pc s hs p first = true := by
  intro n
  induction n with
  | zero => intro _ _ _ _ _ hw; simp [wkB] at hw
  | succ n ih =>
    intro pc s hs p first hw
    simp only [wkB] at hw ⊢
    split at hw
    · exact absurd hw (by simp)
    · rename_i i _
      split at hw
      · rename_i hT
        simp only [hT, ite_true]; exact mergeOK_mono h hw
      · rename_i hT
        simp only [hT, Bool.false_eq_true, ite_false]
        split at hw
        · exact absurd hw (by simp)
        · exact all_mergeOK_mono h hw
        · simp only [Bool.and_eq_true] at hw ⊢
          exact ⟨all_mergeOK_mono h hw.1, ih _ _ _ _ _ hw.2⟩

/-- Joins whose new entries are all in the worklist. -/
def Tracked (J J' : Joins) (w : List Nat) : Prop := ∀ t, J' t ≠ none → J t ≠ none ∨ t ∈ w

def Listed (J : Joins) (w : List Nat) : Prop := ∀ t ∈ w, J t ≠ none

theorem mergeM_spec {F : Fn} {jw jw' : Joins × List Nat} {t s hs} (h : mergeM F jw t s hs = some jw') :
    Ext jw.1 jw'.1 ∧ mergeOK F jw'.1 t s hs = true ∧ Tracked jw.1 jw'.1 jw'.2 ∧ (∀ u ∈ jw.2, u ∈ jw'.2) ∧
    (Listed jw.1 jw.2 → Listed jw'.1 jw'.2) := by
  unfold mergeM at h
  split at h
  · rename_i ha
    split at h
    · rename_i hs' hh
      split at h
      · rename_i hn
        cases h
        refine ⟨?_, ?_, ?_, ?_, ?_⟩
        · intro u r hu
          simp only [upd]
          split
          · subst_vars; rw [hn] at hu; cases hu
          · exact hu
        · simp [mergeOK, ha, hh, upd]
        · intro u hu
          simp only [upd] at hu
          by_cases e : u = t
          · exact Or.inr (by simp [e])
          · simp only [e, ite_false] at hu; exact Or.inl hu
        · intro u hu; exact List.mem_cons_of_mem _ hu
        · intro hl u hu
          simp only [upd]
          split
          · simp
          · rcases List.mem_cons.mp hu with e | e
            · contradiction
            · exact hl u e
      · rename_i r hr
        split at h
        · cases h
          rename_i e
          refine ⟨Ext.refl _, ?_, fun u hu => Or.inl hu, fun u hu => hu, id⟩
          simp [mergeOK, ha, hh, hr, e]
        · cases h
    · cases h
  · cases h

theorem mergeAllM_spec {F : Fn} : ∀ {es : List E} {jw jw' : Joins × List Nat},
    mergeAllM F jw es = some jw' →
    Ext jw.1 jw'.1 ∧ es.all (fun e => mergeOK F jw'.1 e.1 e.2.1 e.2.2) = true ∧ Tracked jw.1 jw'.1 jw'.2 ∧
    (∀ u ∈ jw.2, u ∈ jw'.2) ∧ (Listed jw.1 jw.2 → Listed jw'.1 jw'.2)
  | [], jw, jw', h => by
    simp only [mergeAllM, Option.some.injEq] at h
    subst h
    exact ⟨Ext.refl _, rfl, fun u hu => Or.inl hu, fun u hu => hu, id⟩
  | e :: es, jw, jw', h => by
    simp only [mergeAllM] at h
    split at h
    · rename_i jw1 h1
      obtain ⟨x1, m1, t1, w1, l1⟩ := mergeM_spec h1
      obtain ⟨x2, m2, t2, w2, l2⟩ := mergeAllM_spec h
      refine ⟨Ext.trans x1 x2, ?_, ?_, fun u hu => w2 u (w1 u hu), fun hl => l2 (l1 hl)⟩
      · simp only [List.all_cons, Bool.and_eq_true]
        exact ⟨mergeOK_mono x2 m1, m2⟩
      · intro u hu
        rcases t2 u hu with a | a
        · rcases t1 u a with b | b
          · exact Or.inl b
          · exact Or.inr (w2 u b)
        · exact Or.inr a
    · cases h

theorem walkM_spec {F : Fn} {T} : ∀ {n pc s hs p first} {jw jw' : Joins × List Nat},
    walkM F T n jw pc s hs p first = some jw' →
    Ext jw.1 jw'.1 ∧ wkB F T jw'.1 n pc s hs p first = true ∧ Tracked jw.1 jw'.1 jw'.2 ∧
    (∀ u ∈ jw.2, u ∈ jw'.2) ∧ (Listed jw.1 jw.2 → Listed jw'.1 jw'.2)
  | 0, _, _, _, _, _, _, _, h => by simp [walkM] at h
  | n + 1, pc, s, hs, p, first, jw, jw', h => by
    simp only [walkM] at h
    split at h
    · cases h
    · rename_i i hi
      split at h
      · rename_i hT
        obtain ⟨x, m, t, w, l⟩ := mergeM_spec h
        refine ⟨x, ?_, t, w, l⟩
        simp only [wkB, hi, hT, ite_true]; exact m
      · rename_i hT
        split at h
        · cases h
        · rename_i es hx
          obtain ⟨x, m, t, w, l⟩ := mergeAllM_spec h
          refine ⟨x, ?_, t, w, l⟩
          simp only [wkB, hi, hT, hx]; exact m
        · rename_i es s' hs' p' hx
          split at h
          · rename_i jw1 h1
            obtain ⟨x1, m1, t1, w1, l1⟩ := mergeAllM_spec h1
            obtain ⟨x2, m2, t2, w2, l2⟩ := walkM_spec h
            refine ⟨Ext.trans x1 x2, ?_, ?_, fun u hu => w2 u (w1 u hu), fun hl => l2 (l1 hl)⟩
            · simp [wkB, hi, hT, hx, all_mergeOK_mono x2 m1, m2]
            · intro u hu
              rcases t2 u hu with a | a
              · rcases t1 u a with b | b
                · exact Or.inl b
                · exact Or.inr (w2 u b)
              · exact Or.inr a
          · cases h

/-- Every recorded stack has been walked from, against the final joins. -/
def Final (F : Fn) (T : Nat → Bool) (N : Nat) (J : Joins) : Prop :=
  ∀ t r, J t = some r → wkB F T J N t r.1 r.2 false true = true

theorem loopM_spec {F : Fn} {T} {N} : ∀ {k J w J'}, loopM F T N k J w = some J' →
    Listed J w → (∀ t r, J t = some r → t ∈ w ∨ wkB F T J N t r.1 r.2 false true = true) →
    Ext J J' ∧ Final F T N J'
  | 0, _, _, _, h, _, _ => by simp [loopM] at h
  | k + 1, J, [], J', h, _, hd => by
    simp only [loopM, Option.some.injEq] at h
    subst h
    refine ⟨Ext.refl _, fun t r ht => ?_⟩
    rcases hd t r ht with a | a
    · cases a
    · exact a
  | k + 1, J, t :: w, J', h, hl, hd => by
    simp only [loopM] at h
    split at h
    · rename_i r hr
      split at h
      · rename_i jw hw
        obtain ⟨x, wk, tr, ww, ll⟩ := walkM_spec hw
        have hl' : Listed jw.1 jw.2 := ll (fun u hu => hl u (List.mem_cons_of_mem _ hu))
        obtain ⟨x2, fin⟩ := loopM_spec h hl' (by
          intro u r' hu
          by_cases e : J u = none
          · rcases tr u (by rw [hu]; simp) with a | a
            · exact absurd e a
            · exact Or.inl a
          · obtain ⟨r2, hr2⟩ := Option.ne_none_iff_exists'.mp e
            have e2 := x u r2 hr2
            rw [hu] at e2
            obtain rfl := Option.some.inj e2
            rcases hd u _ hr2 with a | a
            · rcases List.mem_cons.mp a with b | b
              · subst b
                rw [hr] at hr2
                cases hr2
                exact Or.inr wk
              · exact Or.inl (ww u b)
            · exact Or.inr (wkB_mono x _ _ _ _ _ _ a))
        exact ⟨Ext.trans x x2, fin⟩
      · cases h
    · cases h

/-! ### Each step agrees with `xfer` -/

/-- The machine has the handlers of the checker, and more only where they are about to be trimmed (after a label
that records fewer, until TRIM_HANDLERS, where nothing throws) or at `end`. -/
def HRel (F : Fn) (pc : Nat) (hm hc : List H) : Prop :=
  ∃ x, hm = hc ++ x ∧ (x = [] ∨ (trimAt F pc).isSome ∨ F.code[pc]? = some .end_)

theorem HRel.rfl {F : Fn} {pc hs} : HRel F pc hs hs := ⟨[], by simp, Or.inl (Eq.refl _)⟩

/-- A machine state at the target of an edge. -/
def EdgeOK (F : Fn) (st : St) (e : E) : Prop :=
  st.polled = false ∧ st.pc = e.1 ∧ Le F st.stk e.2.1 ∧ HRel F st.pc st.hs e.2.2

/-- What the machine may do from an instruction that the checker passes, with a stack of subtypes. -/
def Covered (F : Fn) (pc : Nat) (o : Out) : Xf → Prop
  | .fail => True
  | .stop es => o ≠ .wrong ∧ ∀ st, o = .next st → ∃ e ∈ es, EdgeOK F st e
  | .next es s' hs' p' => o ≠ .wrong ∧ ∀ st, o = .next st →
      (∃ e ∈ es, EdgeOK F st e) ∨ (st.pc = pc + 1 ∧ st.polled = p' ∧ Le F st.stk s' ∧ HRel F st.pc st.hs hs')

theorem jump_covered {F : Fn} {t k d M S hd e o} (hl : Le F M S) (he : edge t k d S hd = some e)
    (ho : o = jump t k d M hd) : o ≠ .wrong ∧ ∀ st, o = .next st → EdgeOK F st e := by
  simp only [edge] at he
  split at he
  · rename_i hk
    cases he
    subst ho
    simp only [jump, go, hl.length, hk, ite_true]
    refine ⟨by simp, fun st h => ?_⟩
    simp only [Out.next.injEq] at h
    subst h
    exact ⟨rfl, rfl, Le.append (Le.take k hl) (Le.drop (k + d) hl), HRel.rfl⟩
  · cases he

theorem edges_mem {k s hd} : ∀ {recs : List (Nat × Nat)} {es}, edges k s hd recs = some es →
    ∀ r ∈ recs, ∃ e, edge r.1 k r.2 s hd = some e ∧ e ∈ es
  | [], _, _, r, hr => by cases hr
  | r :: rs, es, h, r', hr => by
    simp only [edges] at h
    split at h
    · rename_i e es' he hes
      cases h
      rcases List.mem_cons.mp hr with a | a
      · subst a; exact ⟨e, he, by simp⟩
      · obtain ⟨e', he', hm⟩ := edges_mem hes r' a
        exact ⟨e', he', List.mem_cons_of_mem _ hm⟩
    · cases h

theorem trap_covered {F : Fn} {pc} : ∀ x, Covered F pc .trap x
  | .fail => trivial
  | .stop _ => ⟨by simp, fun _ h => by cases h⟩
  | .next _ _ _ _ => ⟨by simp, fun _ h => by cases h⟩

/-! ### Objects -/

theorem nullOnly_neg {t : VT} (h : nullOnly t = true) : t < 0 := by
  simp only [nullOnly, Bool.or_eq_true, beq_iff_eq] at h; rcases h with rfl | rfl <;> decide

theorem compX_le {F : Fn} (hL : LayOK F) {x y st oy} (h : F.sub x y = true) (hy : compX F y st = some oy) :
    ∃ ox, compX F x st = some ox ∧ (oy = none → ox = none) ∧
      (ox = none ∨ ∃ cx cy, ox = some cx ∧ oy = some cy ∧ extB F cx cy = true) := by
  have hp := hL x y h
  simp only [layPairOK, Bool.and_eq_true] at hp
  obtain ⟨hp1, hp2⟩ := hp
  have bot : nullOnly x = true → compX F x st = some none := fun e => by simp [compX, e]
  unfold compX at hy
  by_cases ny : nullOnly y = true
  · simp only [ny, ite_true, Option.some.injEq] at hy
    subst hy
    have nx : nullOnly x = true := by simpa [ny] using hp1
    exact ⟨none, bot nx, fun _ => rfl, Or.inl rfl⟩
  · simp only [ny, Bool.false_eq_true, ite_false] at hy
    split at hy
    · rename_i y0
      split at hy
      · rename_i cy hly
        split at hy
        · rename_i hk
          cases hy
          have ny' : nullOnly y = false := by simpa using ny
          have yn : decide (y < 0) = false := by simp; omega
          simp only [ny', yn, hly, Bool.false_or, Bool.or_eq_true, Bool.and_eq_true, decide_eq_true_eq] at hp2
          rcases hp2 with nx | ⟨x0, a⟩
          · exact ⟨none, bot nx, (fun h => by cases h), Or.inl rfl⟩
          · split at a
            · rename_i cx hlx
              have nx : nullOnly x = false := by
                cases e : nullOnly x
                · rfl
                · exact absurd x0 (Int.not_le.mpr (nullOnly_neg e))
              have hks : cx.isStruct = st := by
                simp only [extB, Bool.and_eq_true, beq_iff_eq] at a; rw [a.1, hk]
              refine ⟨some cx, ?_, (fun h => by cases h), Or.inr ⟨cx, cy, rfl, rfl, a⟩⟩
              simp [compX, nx, x0, hlx, hks]
            · cases a
        · cases hy
      · cases hy
    · cases hy

theorem fieldAt_ext {F : Fn} {off} : ∀ {as bs : List Field} {fb}, prefB F as bs = true → fieldAt off bs = some fb →
    ∃ fa, fieldAt off as = some fa ∧ fieldB F fa fb = true
  | _, [], _, _, hb => by simp [fieldAt] at hb
  | [], _ :: _, _, h, _ => by simp [prefB] at h
  | a :: as, b :: bs, fb, h, hb => by
    simp only [prefB, Bool.and_eq_true] at h
    have ho : a.off = b.off := by
      have := h.1; simp only [fieldB, Bool.and_eq_true, beq_iff_eq] at this; exact this.1.1.1
    simp only [fieldAt] at hb ⊢
    by_cases e : b.off = off
    · simp only [e, ite_true, Option.some.injEq] at hb
      subst hb
      exact ⟨a, by simp [ho, e], h.1⟩
    · simp only [e, ite_false] at hb
      obtain ⟨fa, h1, h2⟩ := fieldAt_ext h.2 hb
      exact ⟨fa, by simp [ho, e, h1], h2⟩

theorem head_ext {F : Fn} {as : List Field} {b bs} (h : prefB F as (b :: bs) = true) :
    ∃ a as', as = a :: as' ∧ fieldB F a b = true := by
  cases as with
  | nil => simp [prefB] at h
  | cons a as' =>
    simp only [prefB, Bool.and_eq_true] at h
    exact ⟨a, as', rfl, h.1⟩

theorem extB_pref {F : Fn} {a b : Comp} (h : extB F a b = true) : prefB F a.fields b.fields = true := by
  simp only [extB, Bool.and_eq_true] at h; exact h.2

theorem fieldB_spec {F : Fn} {a b : Field} (h : fieldB F a b = true) :
    storage a.ty = storage b.ty ∧ F.sub a.ty b.ty = true ∧
      (b.mutable = true → a.mutable = true ∧ F.sub b.ty a.ty = true) := by
  simp only [fieldB, Bool.and_eq_true, beq_iff_eq, Bool.or_eq_true, Bool.not_eq_true'] at h
  obtain ⟨⟨⟨_, h1⟩, h2⟩, h3⟩ := h
  refine ⟨h1, h2, fun hm => ?_⟩
  rcases h3 with h3 | h3
  · rw [hm] at h3; cases h3
  · exact h3

theorem accessible_eq {acc} {a b : VT} (h : storage a = storage b) : accessible acc a = accessible acc b := by
  simp [accessible, h]

section
variable {F : Fn} (hs : SubOK F)
include hs

theorem unp_sub {a b : VT} (he : storage a = storage b) (h : F.sub a b = true) : F.sub (unp a) (unp b) = true := by
  unfold unp; rw [he]; split
  · exact hs.refl _
  · exact h

theorem castSrc_sub {nul x y} (h : F.sub x y = true) (hx : isRef x = true) :
    F.sub (castSrc nul x) (castSrc nul y) = true := by
  cases nul
  · exact h
  · exact hs.nn _ _ h hx

omit hs in
theorem obj_covered (hL : LayOK F) {pc x y st kM kX} (h : F.sub x y = true)
    (hk : ∀ cx cy, extB F cx cy = true → ∀ o ∈ kM cx, Covered F pc o (kX cy)) :
    ∀ o ∈ objO F x st kM, Covered F pc o (objX F y st kX) := by
  intro o ho
  unfold objX
  split
  · rename_i cy hy
    obtain ⟨ox, hx, _, hc⟩ := compX_le hL h hy
    unfold objO at ho; rw [hx] at ho
    rcases hc with rfl | ⟨cx, cy', rfl, e, he⟩
    · simp only [List.mem_singleton] at ho; subst ho; exact trap_covered _
    · cases e
      rcases List.mem_cons.mp ho with a | a
      · subst a; exact trap_covered _
      · exact hk _ _ he _ a
  · rename_i hy
    obtain ⟨ox, hx, hn, _⟩ := compX_le hL h hy
    unfold objO at ho; rw [hx, hn rfl] at ho
    simp only [List.mem_singleton] at ho; subst ho; exact trap_covered _
  · trivial

omit hs in
theorem next_covered {pc M S hd o} (hl : Le F M S) (ho : o = go (pc + 1) M hd) :
    Covered F pc o (.next [] S hd false) := by
  subst ho
  refine ⟨by simp [go], fun st h => Or.inr ?_⟩
  simp only [go, Out.next.injEq] at h
  subst h
  exact ⟨rfl, rfl, hl, HRel.rfl⟩

omit hs in
theorem push1_covered {pc x y M S hd o} (hl : Le F M S) (h : F.sub x y = true)
    (ho : o = pushO F (pc + 1) [x] M hd) : Covered F pc o (nextX F [y] S hd) := by
  subst ho
  simp only [nextX, pushX, pushO, hl.length, List.length_singleton]
  split
  · refine ⟨by simp [go], fun st h' => Or.inr ?_⟩
    simp only [go, Out.next.injEq] at h'
    subst h'
    exact ⟨rfl, rfl, .cons h hl, HRel.rfl⟩
  · simp [Covered]

theorem push_covered {pc ts M S hd o} (hl : Le F M S) (ho : o = pushO F (pc + 1) ts M hd) :
    Covered F pc o (nextX F ts S hd) := by
  subst ho
  simp only [nextX, pushX, pushO, hl.length]
  split
  · refine ⟨by simp [go], fun st h => Or.inr ?_⟩
    simp only [go, Out.next.injEq] at h
    subst h
    exact ⟨rfl, rfl, Le.append (Le.refl hs _) hl, HRel.rfl⟩
  · simp [Covered]

theorem op_covered {pc args res M S hd o} (hl : Le F M S) (ho : o ∈ opO F pc args res M hd) :
    Covered F pc o (opX F args res S hd) := by
  simp only [opX]
  split
  · rename_i S0 h0
    obtain ⟨M0, hm, hl0⟩ := popT_le hs hl h0
    simp only [opO, hm, List.mem_cons, List.not_mem_nil, or_false] at ho
    rcases ho with a | a
    · subst a
      simp only [nextX, pushX]
      split <;> simp [Covered]
    · exact push_covered hs hl0 a
  · trivial

omit hs in
theorem optAll_mem {α : Type} : ∀ {l : List (Option α)} {es}, optAll l = some es →
    ∀ o ∈ l, ∃ e, o = some e ∧ e ∈ es
  | [], _, _, o, ho => by cases ho
  | some a :: l, es, h, o, ho => by
    simp only [optAll] at h
    cases hl : optAll l with
    | none => rw [hl] at h; cases h
    | some es' =>
      rw [hl] at h
      simp only [Option.map_some, Option.some.injEq] at h
      subst h
      rcases List.mem_cons.mp ho with rfl | ho
      · exact ⟨a, rfl, by simp⟩
      · obtain ⟨e, he, hm⟩ := optAll_mem hl o ho
        exact ⟨e, he, List.mem_cons_of_mem _ hm⟩
  | none :: _, _, h, _, _ => by simp [optAll] at h

theorem throw_covered {M S hd es} (hl : Le F M S) (h : catchesX F S hd = some es) :
    ∀ o ∈ throwO F M hd, o ≠ .wrong ∧ ∀ st, o = .next st → ∃ e ∈ es, EdgeOK F st e := by
  intro o ho
  rcases List.mem_cons.mp ho with rfl | ho
  · exact ⟨by simp, fun _ h => by cases h⟩
  obtain ⟨q, hq, rfl⟩ := List.mem_map.mp ho
  obtain ⟨e, he, hm⟩ := optAll_mem h _ (List.mem_map.mpr ⟨q, hq, rfl⟩)
  simp only [catchS] at he
  rw [hl.length]
  split at he
  · rename_i hc
    simp only [Option.map_some, Option.some.injEq] at he
    subst he
    simp only [hc, and_self, ite_true]
    refine ⟨by simp [go], fun st h => ⟨_, hm, ?_⟩⟩
    simp only [go, Out.next.injEq] at h
    subst h
    exact ⟨rfl, rfl, Le.append (Le.refl hs _) (Le.drop _ hl), HRel.rfl⟩
  · simp at he

/-- Each step with the handlers of the checker. -/
theorem step_covered (hL : LayOK F) (pc : Nat) (M S : List VT) (hl : Le F M S) (hd : List H) (p lp : Bool)
    (hp : p = true → lp = true) (i : Instr) (o : Out) (ho : o ∈ stepI F pc M hd lp i) :
    Covered F pc o (xfer F pc S hd p i) := by
  cases i with
  | const t => exact push_covered hs hl (List.mem_singleton.mp ho)
  | op args res => exact op_covered hs hl ho
  | opMap m args res =>
    simp only [xfer]
    split
    · rename_i hm
      simp only [stepI, mapX_exact hs hl hm, ite_true] at ho
      exact op_covered hs hl ho
    · trivial
  | lanes ks n args res =>
    simp only [xfer]
    split
    · rename_i hk
      simp only [stepI, hk, ite_true] at ho
      exact op_covered hs hl ho
    · trivial
  | call args res =>
    simp only [xfer]
    split
    · rename_i hpt
      split
      · rename_i S0 h0
        obtain ⟨M0, hm, hl0⟩ := popT_le hs hl h0
        split
        · rename_i es s' hes hs'
          simp only [stepI, hp hpt, ite_true, hm] at ho
          rcases List.mem_cons.mp ho with a | a
          · subst a; exact trap_covered _
          rcases List.mem_cons.mp a with a | a
          · subst a
            simp only [pushX] at hs'
            split at hs'
            · rename_i hk
              cases hs'
              simp only [pushO, hl0.length, hk, ite_true]
              refine ⟨by simp [go], fun st h => Or.inr ?_⟩
              simp only [go, Out.next.injEq] at h
              subst h
              exact ⟨rfl, rfl, Le.append (Le.refl hs _) hl0, HRel.rfl⟩
            · cases hs'
          · obtain ⟨nw, hn⟩ := throw_covered hs hl0 hes o a
            exact ⟨nw, fun st h => Or.inl (hn st h)⟩
        · trivial
      · trivial
    · trivial
  | retCall args res =>
    simp only [xfer]
    split
    · rename_i S0 h0
      obtain ⟨M0, hm, _⟩ := popT_le hs hl h0
      split
      · rename_i e
        simp only [stepI, hm, e, ite_true, List.mem_cons, List.not_mem_nil, or_false] at ho
        rcases ho with a | a <;> subst a <;> simp [Covered]
      · trivial
    · trivial
  | drop =>
    simp only [xfer]
    split
    · cases hl with
      | cons _ t =>
        simp only [stepI, List.mem_singleton] at ho
        subst ho
        refine ⟨by simp [go], fun st h => Or.inr ?_⟩
        simp only [go, Out.next.injEq] at h
        subst h
        exact ⟨rfl, rfl, t, HRel.rfl⟩
    · trivial
  | select =>
    simp only [xfer]
    split
    · rename_i c a b S0
      split
      · rename_i e
        simp only [Bool.and_eq_true, beq_iff_eq] at e
        obtain ⟨ec, rfl⟩ := e
        cases hl with
        | cons h1 t1 => cases t1 with
          | cons h2 t2 => cases t2 with
            | cons h3 t3 =>
              simp only [stepI, hs.trans _ _ _ h1 ec, ite_true, List.mem_cons, List.not_mem_nil,
                or_false] at ho
              refine ⟨by rcases ho with h | h <;> simp [h, go], fun st h => Or.inr ?_⟩
              rcases ho with e | e <;> subst e <;> simp only [go, Out.next.injEq] at h <;> subst h
              · exact ⟨rfl, rfl, .cons h2 t3, HRel.rfl⟩
              · exact ⟨rfl, rfl, .cons h3 t3, HRel.rfl⟩
      · trivial
    · trivial
  | localGet i =>
    simp only [xfer]
    split
    · rename_i t ht
      simp only [stepI, ht, List.mem_singleton] at ho
      exact push_covered hs hl ho
    · trivial
  | localSet i =>
    simp only [xfer]
    split
    · rename_i t x S0 ht
      split
      · rename_i e
        cases hl with
        | cons h1 t1 =>
          simp only [stepI, ht, hs.trans _ _ _ h1 e, ite_true, List.mem_singleton] at ho
          subst ho
          refine ⟨by simp [go], fun st h => Or.inr ?_⟩
          simp only [go, Out.next.injEq] at h
          subst h
          exact ⟨rfl, rfl, t1, HRel.rfl⟩
      · trivial
    · trivial
  | localTee i =>
    simp only [xfer]
    split
    · rename_i t x S0 ht
      split
      · rename_i e
        cases hl with
        | cons h1 t1 =>
          simp only [stepI, ht, hs.trans _ _ _ h1 e, ite_true, List.mem_singleton] at ho
          exact push_covered hs t1 ho
      · trivial
    · trivial
  | refIsNull =>
    simp only [xfer]
    split
    · rename_i x S0
      split
      · rename_i e
        cases hl with
        | cons h1 t1 =>
          simp only [stepI, hs.ref _ _ h1, e, ite_true, List.mem_singleton] at ho
          exact push_covered hs t1 ho
      · trivial
    · trivial
  | refAsNonNull =>
    simp only [xfer]
    split
    · rename_i x S0
      split
      · rename_i e
        cases hl with
        | cons h1 t1 =>
          rename_i y M0
          have ey : isRef y = true := by rw [hs.ref _ _ h1]; exact e
          simp only [stepI, ey, ite_true, List.mem_cons, List.not_mem_nil, or_false] at ho
          rcases ho with a | a
          · subst a; simp [Covered]
          · subst a
            refine ⟨by simp [go], fun st h => Or.inr ?_⟩
            simp only [go, Out.next.injEq] at h
            subst h
            exact ⟨rfl, rfl, .cons (hs.nn _ _ h1 ey) t1, HRel.rfl⟩
      · trivial
    · trivial
  | br t k d =>
    simp only [xfer]
    split
    · rename_i e he
      obtain ⟨nw, hn⟩ := jump_covered hl he (List.mem_singleton.mp ho)
      exact ⟨nw, fun st h => ⟨e, by simp, hn st h⟩⟩
    · trivial
  | brIf t k d =>
    simp only [xfer]
    split
    · rename_i c S0
      split
      · rename_i ec
        split
        · rename_i e he
          cases hl with
          | cons h1 t1 =>
            simp only [stepI, hs.trans _ _ _ h1 ec, ite_true, List.mem_cons, List.not_mem_nil, or_false] at ho
            rcases ho with h | h
            · subst h
              refine ⟨by simp [go], fun st h => Or.inr ?_⟩
              simp only [go, Out.next.injEq] at h
              subst h
              exact ⟨rfl, rfl, t1, HRel.rfl⟩
            · obtain ⟨nw, hn⟩ := jump_covered t1 he h
              exact ⟨nw, fun st h' => Or.inl ⟨e, by simp, hn st h'⟩⟩
        · trivial
      · trivial
    · trivial
  | brTable k recs =>
    simp only [xfer]
    split
    · rename_i c S0
      split
      · rename_i ec
        split
        · rename_i es hes
          simp only [Bool.and_eq_true] at ec
          cases hl with
          | cons h1 t1 =>
            simp only [stepI, hs.trans _ _ _ h1 ec.1, ec.2, Bool.and_self, ite_true] at ho
            obtain ⟨r, hr', hro⟩ := List.mem_map.mp ho
            obtain ⟨e, he, hm⟩ := edges_mem hes r hr'
            obtain ⟨nw, hn⟩ := jump_covered t1 he hro.symm
            exact ⟨nw, fun st h => ⟨e, hm, hn st h⟩⟩
        · trivial
      · trivial
    · trivial
  | brOnNull t k d =>
    simp only [xfer]
    split
    · rename_i x S0
      split
      · rename_i ex
        split
        · rename_i e he
          cases hl with
          | cons h1 t1 =>
            rename_i y M0
            have ey : isRef y = true := by rw [hs.ref _ _ h1]; exact ex
            simp only [stepI, ey, ite_true, List.mem_cons, List.not_mem_nil, or_false] at ho
            rcases ho with h | h
            · obtain ⟨nw, hn⟩ := jump_covered t1 he h
              exact ⟨nw, fun st h' => Or.inl ⟨e, by simp, hn st h'⟩⟩
            · subst h
              refine ⟨by simp [go], fun st h => Or.inr ?_⟩
              simp only [go, Out.next.injEq] at h
              subst h
              exact ⟨rfl, rfl, .cons (hs.nn _ _ h1 ey) t1, HRel.rfl⟩
        · trivial
      · trivial
    · trivial
  | brOnNonNull t k d =>
    simp only [xfer]
    split
    · rename_i x S0
      split
      · rename_i ex
        split
        · rename_i e he
          cases hl with
          | cons h1 t1 =>
            rename_i y M0
            have ey : isRef y = true := by rw [hs.ref _ _ h1]; exact ex
            simp only [stepI, ey, ite_true, List.mem_cons, List.not_mem_nil, or_false] at ho
            rcases ho with h | h
            · obtain ⟨nw, hn⟩ := jump_covered (.cons (hs.nn _ _ h1 ey) t1) he h
              exact ⟨nw, fun st h' => Or.inl ⟨e, by simp, hn st h'⟩⟩
            · subst h
              refine ⟨by simp [go], fun st h => Or.inr ?_⟩
              simp only [go, Out.next.injEq] at h
              subst h
              exact ⟨rfl, rfl, t1, HRel.rfl⟩
        · trivial
      · trivial
    · trivial
  | if_ t =>
    simp only [xfer]
    split
    · rename_i c S0
      split
      · rename_i ec
        cases hl with
        | cons h1 t1 =>
          simp only [stepI, hs.trans _ _ _ h1 ec, ite_true, List.mem_cons, List.not_mem_nil, or_false] at ho
          refine ⟨by rcases ho with h | h <;> simp [h, go], fun st h => ?_⟩
          rcases ho with e | e <;> subst e <;> simp only [go, Out.next.injEq] at h <;> subst h
          · exact Or.inr ⟨rfl, rfl, t1, HRel.rfl⟩
          · exact Or.inl ⟨(t, S0, hd), by simp, rfl, rfl, t1, HRel.rfl⟩
      · trivial
    · trivial
  | else_ t =>
    simp only [stepI, List.mem_singleton] at ho
    subst ho
    refine ⟨by simp [go], fun st h => ?_⟩
    simp only [go, Out.next.injEq] at h
    subst h
    exact ⟨(t, S, hd), by simp, rfl, rfl, hl, HRel.rfl⟩
  | meter t =>
    simp only [xfer]
    split
    · simp only [stepI, List.mem_cons, List.not_mem_nil, or_false] at ho
      refine ⟨by rcases ho with h | h <;> simp [h, go], fun st h => ?_⟩
      rcases ho with e | e <;> subst e <;> simp only [go, Out.next.injEq] at h <;> subst h
      · exact Or.inr ⟨rfl, rfl, hl, HRel.rfl⟩
      · exact Or.inl ⟨(t, S, hd), by simp, rfl, rfl, hl, HRel.rfl⟩
    · trivial
  | poll m =>
    simp only [xfer]
    split
    · rename_i hm
      simp only [stepI, mapX_exact hs hl hm, ite_true, List.mem_singleton] at ho
      subst ho
      refine ⟨by simp [go], fun st h => Or.inr ?_⟩
      simp only [go, Out.next.injEq] at h
      subst h
      exact ⟨rfl, rfl, hl, HRel.rfl⟩
    · trivial
  | tick m =>
    simp only [xfer]
    split
    · rename_i hm
      simp only [stepI, mapX_exact hs hl hm, ite_true, List.mem_singleton] at ho
      subst ho
      refine ⟨by simp [go], fun st h => Or.inr ?_⟩
      simp only [go, Out.next.injEq] at h
      subst h
      exact ⟨rfl, rfl, hl, HRel.rfl⟩
    · trivial
  | end_ =>
    simp only [xfer]
    split
    · rename_i e
      have : leB F M F.results.reverse = true :=
        leB_iff.mpr (Le.trans hs hl (leB_iff.mp e))
      simp only [stepI, this, ite_true, List.mem_singleton] at ho
      subst ho
      simp [Covered]
    · trivial
  | ret =>
    simp only [xfer]
    split
    · rename_i e
      obtain ⟨S0, h0⟩ := Option.isSome_iff_exists.mp e
      simp only [stepI, popT_none_le hs hl h0, ite_true, List.mem_singleton] at ho
      subst ho
      simp [Covered]
    · trivial
  | unreachable =>
    simp only [stepI, List.mem_singleton] at ho
    subst ho
    simp [xfer, Covered]
  | bad => simp [xfer, Covered]
  | refEq =>
    simp only [xfer]
    split
    · split
      · rename_i e
        cases hl with
        | cons h1 t1 => cases t1 with
          | cons h2 t2 =>
            simp only [stepI, hs.ref _ _ h1, hs.ref _ _ h2, e, ite_true, List.mem_singleton] at ho
            exact push_covered hs t2 ho
      · trivial
    · trivial
  | refCast t =>
    simp only [xfer]
    split
    · split
      · rename_i e
        cases hl with
        | cons h1 t1 =>
          simp only [stepI, hs.ref _ _ h1, e, ite_true, List.mem_cons, List.not_mem_nil, or_false] at ho
          rcases ho with a | a
          · subst a; exact trap_covered _
          · exact push_covered hs t1 a
      · trivial
    · trivial
  | brOnCast onFail nul dst t k d =>
    simp only [xfer]
    split
    · rename_i x S0
      split
      · rename_i ex
        split
        · rename_i e he
          cases hl with
          | cons h1 t1 =>
            rename_i y M0
            have ey : isRef y = true := by rw [hs.ref _ _ h1]; exact ex
            have hc := castSrc_sub hs (nul := nul) h1 ey
            have pick : ∀ b : Bool, F.sub (if b then castSrc nul y else dst) (if b then castSrc nul x else dst) = true ∧
                F.sub (if b then dst else castSrc nul y) (if b then dst else castSrc nul x) = true := by
              intro b; cases b <;> simp [hc, hs.refl]
            simp only [stepI, ey, ite_true, List.mem_cons, List.not_mem_nil, or_false] at ho
            rcases ho with h | h
            · obtain ⟨nw, hn⟩ := jump_covered (.cons (pick onFail).1 t1) he h
              exact ⟨nw, fun st h' => Or.inl ⟨e, by simp, hn st h'⟩⟩
            · subst h
              refine ⟨by simp [go], fun st h => Or.inr ?_⟩
              simp only [go, Out.next.injEq] at h
              subst h
              exact ⟨rfl, rfl, .cons (pick onFail).2 t1, HRel.rfl⟩
        · trivial
      · trivial
    · trivial
  | structGet acc off =>
    simp only [xfer]
    split
    · cases hl with
      | cons h1 t1 =>
        simp only [stepI] at ho
        refine obj_covered hL h1 (fun cx cy he o ho => ?_) o ho
        split
        · rename_i fy hfy
          split
          · rename_i ha
            obtain ⟨fx, hfx, hb⟩ := fieldAt_ext (extB_pref he) hfy
            obtain ⟨st1, sb, _⟩ := fieldB_spec hb
            simp only [hfx, accessible_eq st1, ha, ite_true, List.mem_singleton] at ho
            exact push1_covered t1 (unp_sub hs st1 sb) ho
          · trivial
        · trivial
    · trivial
  | structSet acc off =>
    simp only [xfer]
    split
    · cases hl with
      | cons hv t1 => cases t1 with
        | cons h1 t2 =>
          simp only [stepI] at ho
          refine obj_covered hL h1 (fun cx cy he o ho => ?_) o ho
          split
          · rename_i fy hfy
            split
            · rename_i ha
              simp only [Bool.and_eq_true] at ha
              obtain ⟨⟨ac, mu⟩, sv⟩ := ha
              obtain ⟨fx, hfx, hb⟩ := fieldAt_ext (extB_pref he) hfy
              obtain ⟨st1, _, hm⟩ := fieldB_spec hb
              obtain ⟨mx, sb⟩ := hm mu
              have := hs.trans _ _ _ (hs.trans _ _ _ hv sv) (unp_sub hs st1.symm sb)
              simp only [hfx, accessible_eq st1, ac, mx, this, Bool.and_self, ite_true, List.mem_singleton] at ho
              exact next_covered t2 ho
            · trivial
          · trivial
    · trivial
  | arrayGet acc =>
    simp only [xfer]
    split
    · split
      · rename_i ei
        cases hl with
        | cons hi t1 => cases t1 with
          | cons h1 t2 =>
            simp only [stepI, hs.trans _ _ _ hi ei, ite_true] at ho
            refine obj_covered hL h1 (fun cx cy he o ho => ?_) o ho
            split
            · rename_i fy _ hfy
              split
              · rename_i ha
                obtain ⟨fx, _, hcx, hb⟩ := head_ext (hfy ▸ extB_pref he)
                obtain ⟨st1, sb, _⟩ := fieldB_spec hb
                simp only [hcx, accessible_eq st1, ha, ite_true, List.mem_singleton] at ho
                exact push1_covered t2 (unp_sub hs st1 sb) ho
              · trivial
            · trivial
      · trivial
    · trivial
  | arraySet acc =>
    simp only [xfer]
    split
    · split
      · rename_i ei
        cases hl with
        | cons hv t1 => cases t1 with
          | cons hi t2 => cases t2 with
            | cons h1 t3 =>
              simp only [stepI, hs.trans _ _ _ hi ei, ite_true] at ho
              refine obj_covered hL h1 (fun cx cy he o ho => ?_) o ho
              split
              · rename_i fy _ hfy
                split
                · rename_i ha
                  simp only [Bool.and_eq_true] at ha
                  obtain ⟨⟨ac, mu⟩, sv⟩ := ha
                  obtain ⟨fx, _, hcx, hb⟩ := head_ext (hfy ▸ extB_pref he)
                  obtain ⟨st1, _, hm⟩ := fieldB_spec hb
                  obtain ⟨mx, sb⟩ := hm mu
                  have := hs.trans _ _ _ (hs.trans _ _ _ hv sv) (unp_sub hs st1.symm sb)
                  simp only [hcx, accessible_eq st1, ac, mx, this, Bool.and_self, ite_true,
                    List.mem_singleton] at ho
                  exact next_covered t3 ho
                · trivial
              · trivial
      · trivial
    · trivial
  | arrayCopy m =>
    simp only [xfer]
    split
    · split
      · rename_i ec
        simp only [Bool.and_eq_true] at ec
        obtain ⟨⟨⟨⟨em, en⟩, eso⟩, edo⟩, _⟩ := ec
        have hm := mapX_exact hs hl em
        cases hl with
        | cons hn t1 => cases t1 with
          | cons hso t2 => cases t2 with
            | cons hy t3 => cases t3 with
              | cons hdo t4 => cases t4 with
                | cons hx t5 =>
                  simp only [stepI, hm, hs.trans _ _ _ hn en, hs.trans _ _ _ hso eso, hs.trans _ _ _ hdo edo,
                    Bool.and_self, ite_true] at ho
                  refine obj_covered hL hx (fun cxd cyd hed o ho => ?_) o ho
                  refine obj_covered hL hy (fun cxs cys hes o ho => ?_) o ho
                  split
                  · rename_i fd' _ fs' _ hfd hfs
                    split
                    · rename_i ha
                      simp only [Bool.and_eq_true] at ha
                      obtain ⟨mu, sfs⟩ := ha
                      obtain ⟨fd, _, hcd, hbd⟩ := head_ext (hfd ▸ extB_pref hed)
                      obtain ⟨fs, _, hcs, hbs⟩ := head_ext (hfs ▸ extB_pref hes)
                      obtain ⟨mx, sd⟩ := (fieldB_spec hbd).2.2 mu
                      have := hs.trans _ _ _ (hs.trans _ _ _ (fieldB_spec hbs).2.1 sfs) sd
                      simp only [hcd, hcs, mx, this, Bool.and_self, ite_true, List.mem_singleton] at ho
                      exact next_covered t5 ho
                    · trivial
                  · trivial
      · trivial
    · trivial
  | arrayInitElem m seg =>
    simp only [xfer]
    split
    · split
      · rename_i ec
        simp only [Bool.and_eq_true] at ec
        obtain ⟨⟨⟨em, en⟩, eso⟩, edo⟩ := ec
        have hm := mapX_exact hs hl em
        cases hl with
        | cons hn t1 => cases t1 with
          | cons hso t2 => cases t2 with
            | cons hdo t3 => cases t3 with
              | cons hx t4 =>
                simp only [stepI, hm, hs.trans _ _ _ hn en, hs.trans _ _ _ hso eso, hs.trans _ _ _ hdo edo,
                  Bool.and_self, ite_true] at ho
                refine obj_covered hL hx (fun cx cy he o ho => ?_) o ho
                split
                · rename_i fy _ hfy
                  split
                  · rename_i ha
                    simp only [Bool.and_eq_true, beq_iff_eq] at ha
                    obtain ⟨⟨mu, sr⟩, ss⟩ := ha
                    obtain ⟨fx, _, hcx, hb⟩ := head_ext (hfy ▸ extB_pref he)
                    obtain ⟨st1, _, hm'⟩ := fieldB_spec hb
                    obtain ⟨mx, sb⟩ := hm' mu
                    have := hs.trans _ _ _ ss sb
                    simp only [hcx, mx, st1, sr, this, beq_self_eq_true, Bool.and_self, ite_true,
                      List.mem_singleton] at ho
                    exact next_covered t4 ho
                  · trivial
                · trivial
      · trivial
    · trivial

  | tryTable cs =>
    simp only [xfer]
    split
    · simp only [stepI, List.mem_singleton] at ho
      subst ho
      refine ⟨by simp [go], fun st h => Or.inr ?_⟩
      simp only [go, Out.next.injEq] at h
      subst h
      refine ⟨rfl, rfl, hl, ?_⟩
      simp only [hl.length]
      exact HRel.rfl
    · trivial
  | endTry =>
    simp only [xfer]
    split
    · trivial
    · rename_i he
      simp only [stepI, he, Bool.false_eq_true, ite_false, List.mem_singleton] at ho
      exact next_covered hl ho
  | trim d =>
    simp only [xfer]
    split
    · rename_i hk
      simp only [stepI, hk, ite_true, List.mem_singleton] at ho
      exact next_covered hl ho
    · trivial
  | throw args =>
    simp only [xfer]
    split
    · rename_i S0 h0
      obtain ⟨M0, hm, hl0⟩ := popT_le hs hl h0
      split
      · rename_i es hes
        simp only [stepI, hm] at ho
        exact throw_covered hs hl0 hes o ho
      · trivial
    · trivial

end

/-! ### Handlers left to trim -/

theorem trimAtN_succ {F : Fn} : ∀ {n pc}, F.code.size ≤ pc + n → trimAtN F (n + 1) pc = trimAtN F n pc
  | 0, pc, h => by
    have : F.code[pc]? = none := Array.getElem?_eq_none (by omega)
    simp [trimAtN, this]
  | n + 1, pc, h => by
    have ih := trimAtN_succ (F := F) (n := n) (pc := pc + 1) (by omega)
    rw [trimAtN.eq_2 F pc (n + 1), trimAtN.eq_2 F pc n]
    cases hc : F.code[pc]? with
    | none => rfl
    | some i => cases i <;> first | rfl | exact ih

/-- `trimAt` after an instruction. -/
theorem trimAt_eq {F : Fn} {pc i} (hc : F.code[pc]? = some i) : trimAt F pc = match i with
    | .trim d => some d
    | .poll _ | .tick _ | .meter _ => trimAt F (pc + 1)
    | _ => none := by
  have hlt : pc < F.code.size := by
    rcases Nat.lt_or_ge pc F.code.size with h | h
    · exact h
    · rw [Array.getElem?_eq_none h] at hc; cases hc
  obtain ⟨m, hm⟩ : ∃ m, F.code.size = m + 1 := ⟨F.code.size - 1, by omega⟩
  have hs := trimAtN_succ (F := F) (n := m) (pc := pc + 1) (by omega)
  unfold trimAt
  rw [hm, trimAtN.eq_2 F pc m]
  simp only [hc]
  cases i <;> first | rfl | exact hs.symm

theorem hrel_hsAt {F : Fn} {t hm hc hs'} (hr : HRel F t hm hc) (h : hsAt F t hc = some hs') : HRel F t hm hs' := by
  obtain ⟨x, rfl, hx⟩ := hr
  unfold hsAt at h
  split at h
  · rename_i e
    cases h
    exact ⟨hc ++ x, by simp, Or.inr (Or.inr e)⟩
  · split at h
    · rename_i d hd
      split at h
      · cases h
        exact ⟨hc.drop d ++ x, by rw [← List.append_assoc, List.take_append_drop], Or.inr (Or.inl (by simp [hd]))⟩
      · cases h
    · cases h
      exact ⟨x, rfl, hx⟩

/-- Each step, where the machine may have more handlers than the checker. -/
theorem step_any {F : Fn} (hs : SubOK F) (hL : LayOK F) {pc i M S hm hc p lp o} (hi : F.code[pc]? = some i)
    (hl : Le F M S) (hr : HRel F pc hm hc) (hp : p = true → lp = true) (ho : o ∈ stepI F pc M hm lp i) :
    Covered F pc o (xfer F pc S hc p i) := by
  obtain ⟨x, rfl, hx⟩ := hr
  by_cases x0 : x = []
  · subst x0
    simp only [List.append_nil] at ho
    exact step_covered hs hL pc M S hl hc p lp hp i o ho
  have tr : (trimAt F pc).isSome = true ∨ i = .end_ := by
    rcases hx with h | h | h
    · exact absurd h x0
    · exact Or.inl h
    · rw [hi] at h; cases h; exact Or.inr rfl
  have te := trimAt_eq hi
  have next : ∀ {st : St}, st.pc = pc + 1 → st.hs = hc ++ x → (trimAt F pc).isSome = true →
      (trimAt F (pc + 1)) = trimAt F pc → HRel F st.pc st.hs hc := fun e1 e2 t1 t2 =>
    ⟨x, e2, Or.inr (Or.inl (by rw [e1, t2]; exact t1))⟩
  cases i with
  | poll m =>
    have t1 : (trimAt F pc).isSome = true := tr.resolve_right (fun h => by cases h)
    simp only [xfer]
    split
    · rename_i hmx
      simp only [stepI, mapX_exact hs hl hmx, ite_true, List.mem_singleton] at ho
      subst ho
      refine ⟨by simp [go], fun st h => Or.inr ?_⟩
      simp only [go, Out.next.injEq] at h
      subst h
      exact ⟨rfl, rfl, hl, next rfl rfl t1 te.symm⟩
    · trivial
  | tick m =>
    have t1 : (trimAt F pc).isSome = true := tr.resolve_right (fun h => by cases h)
    simp only [xfer]
    split
    · rename_i hmx
      simp only [stepI, mapX_exact hs hl hmx, ite_true, List.mem_singleton] at ho
      subst ho
      refine ⟨by simp [go], fun st h => Or.inr ?_⟩
      simp only [go, Out.next.injEq] at h
      subst h
      exact ⟨rfl, rfl, hl, next rfl rfl t1 te.symm⟩
    · trivial
  | meter t =>
    have t1 : (trimAt F pc).isSome = true := tr.resolve_right (fun h => by cases h)
    simp only [xfer]
    split
    · rename_i hc'
      have t2 : (trimAt F t).isSome = true := by
        simp only [Bool.or_eq_true, Option.isNone_iff_eq_none] at hc'
        rcases hc' with h | h
        · rw [h] at t1; cases t1
        · exact h
      simp only [stepI, List.mem_cons, List.not_mem_nil, or_false] at ho
      refine ⟨by rcases ho with h | h <;> simp [h, go], fun st h => ?_⟩
      rcases ho with e | e <;> subst e <;> simp only [go, Out.next.injEq] at h <;> subst h
      · exact Or.inr ⟨rfl, rfl, hl, next rfl rfl t1 te.symm⟩
      · exact Or.inl ⟨(t, S, hc), by simp, rfl, rfl, hl, ⟨x, rfl, Or.inr (Or.inl t2)⟩⟩
    · trivial
  | trim d =>
    simp only [xfer]
    split
    · rename_i hk
      have hk' : d ≤ (hc ++ x).length := by simp; omega
      simp only [stepI, hk', ite_true, List.mem_singleton, List.take_append_of_le_length hk] at ho
      exact next_covered hl ho
    · trivial
  | end_ => exact step_covered hs hL pc M S hl hc p lp hp .end_ o ho
  | _ => rcases tr with h | h <;> simp_all

/-! ### Reachable states are walked -/

def Inv (F : Fn) (T : Nat → Bool) (J : Joins) (st : St) : Prop :=
  ∃ n p S HS, (p = true → st.polled = true) ∧ Le F st.stk S ∧ HRel F st.pc st.hs HS ∧
    wkB F T J n st.pc S HS p true = true

theorem inv_of_join {F : Fn} (hs : SubOK F) {T N J} (fin : Final F T N J) {t M S hm hc} (hl : Le F M S)
    (hr : HRel F t hm hc) (h : mergeOK F J t S hc = true) : Inv F T J ⟨t, M, false, hm⟩ := by
  simp only [mergeOK, Bool.and_eq_true] at h
  obtain ⟨ha, h2⟩ := h
  split at h2
  · rename_i hs' e
    simp only [decide_eq_true_eq] at h2
    exact ⟨N, false, _, _, by simp, le_nrm hs ha hl, hrel_hsAt hr e, fin t _ h2⟩
  · cases h2

theorem inv_step {F : Fn} (hs : SubOK F) (hL : LayOK F) {T N J} (fin : Final F T N J) {st : St} (hi : Inv F T J st) :
    ∀ o ∈ steps F st, o ≠ .wrong ∧ ∀ st', o = .next st' → Inv F T J st' := by
  intro o ho
  obtain ⟨n, p, S, HS, hp, hl, hr, hw⟩ := hi
  cases n with
  | zero => simp [wkB] at hw
  | succ n =>
  simp only [wkB] at hw
  simp only [steps] at ho
  split at hw
  · exact absurd hw (by simp)
  · rename_i i hc
    rw [hc] at ho
    simp only [Bool.not_true, Bool.false_and, Bool.false_eq_true, ite_false] at hw
    have cov := step_any hs hL hc hl hr hp ho
    revert cov hw
    cases xfer F st.pc S HS p i with
    | fail => simp
    | stop es =>
      intro hw cov
      refine ⟨cov.1, fun st' h => ?_⟩
      obtain ⟨e, hm, hpo, hpc, hle, hre⟩ := cov.2 st' h
      simp only [List.all_eq_true] at hw
      cases st'; simp only at hpo hpc hle hre; subst hpo hpc
      exact inv_of_join hs fin hle hre (hw _ hm)
    | next es s' hs' p' =>
      intro hw cov
      simp only [Bool.and_eq_true, List.all_eq_true] at hw
      refine ⟨cov.1, fun st' h => ?_⟩
      rcases cov.2 st' h with ⟨e, hm, hpo, hpc, hle, hre⟩ | ⟨hpc, hpo, hle, hre⟩
      · cases st'; simp only at hpo hpc hle hre; subst hpo hpc
        exact inv_of_join hs fin hle hre (hw.1 _ hm)
      · have h2 := hw.2
        cases n with
        | zero => simp [wkB] at h2
        | succ n =>
        simp only [wkB] at h2
        split at h2
        · exact absurd h2 (by simp)
        · rename_i i' hc'
          by_cases hT : T (st.pc + 1) = true
          · simp only [hT, Bool.not_false, Bool.and_self, ite_true] at h2
            cases st'; simp only at hpo hpc hle hre; subst hpc
            obtain ⟨n', p'', S', H', hp'', hl'', hr'', hw''⟩ := inv_of_join hs fin hle hre h2
            exact ⟨n', p'', S', H', fun e => absurd (hp'' e) (by simp), hl'', hr'', hw''⟩
          · refine ⟨n + 1, p', s', hs', fun e => by rw [hpo]; exact e, hle, hre, ?_⟩
            rw [hpc]
            simp only [wkB, hc', Bool.not_true, Bool.false_and]
            simp only [hT, Bool.and_false] at h2
            exact h2

theorem check_final {F : Fn} (h : check F = true) :
    ∃ J, Final F (fun pc => (joinPoints F)[pc]?.getD false) (F.code.size + 1) J ∧ mergeOK F J 0 [] [] = true := by
  simp only [check, Bool.and_eq_true] at h
  obtain ⟨_, hm⟩ := h
  split at hm
  · rename_i jw hjw
    obtain ⟨J, hJ⟩ := Option.isSome_iff_exists.mp hm
    obtain ⟨x, m, tr, _, ll⟩ := mergeM_spec hjw
    have l0 : Listed jw.1 jw.2 := ll (fun _ h => by cases h)
    obtain ⟨x2, fin⟩ := loopM_spec hJ l0 (by
      intro t r ht
      rcases tr t (by rw [ht]; simp) with a | a
      · exact absurd rfl a
      · exact Or.inl a)
    exact ⟨J, fin, mergeOK_mono x2 m⟩
  · exact absurd hm (by simp)

/-- Code accepted by the checker never goes wrong, given a subtyping like that of the spec. -/
theorem check_sound {F : Fn} (hs : SubOK F) (hL : LayOK F) (h : check F = true) {st : St} (hr : Reach F st) :
    ∀ o ∈ steps F st, o ≠ .wrong := by
  obtain ⟨J, fin, h0⟩ := check_final h
  have : Inv F (fun pc => (joinPoints F)[pc]?.getD false) J st := by
    induction hr with
    | init => exact inv_of_join hs fin .nil HRel.rfl h0
    | step _ hs' ih => exact (inv_step hs hL fin ih _ hs').2 _ rfl
  exact fun o ho => (inv_step hs hL fin this o ho).1

/-- Subtyping by a table of pairs of distinct types, as dumped for the types of a function. -/
def tableSub (tbl : List (VT × VT)) (a b : VT) : Bool := a == b || tbl.contains (a, b)

/-- A finite check that `tableSub tbl` satisfies `SubOK`. -/
def tableOK (tbl : List (VT × VT)) : Bool :=
  tbl.all (fun e => isRef e.1 == isRef e.2 && (!isRef e.1 || tableSub tbl (nn e.1) (nn e.2))) &&
  tbl.all (fun e => tbl.all (fun e' => e.2 != e'.1 || tableSub tbl e.1 e'.2))

theorem tableOK_sound {F : Fn} {tbl : List (VT × VT)} (hf : F.sub = tableSub tbl) (h : tableOK tbl = true) :
    SubOK F := by
  simp only [tableOK, Bool.and_eq_true, List.all_eq_true] at h
  obtain ⟨h1, h2⟩ := h
  have mem : ∀ {a b}, tableSub tbl a b = true → a = b ∨ (a, b) ∈ tbl := by
    intro a b hab
    simp only [tableSub, Bool.or_eq_true, beq_iff_eq, List.contains_iff_mem] at hab
    exact hab
  refine ⟨fun a => by simp [hf, tableSub], fun a b c hab hbc => ?_, fun a b hab => ?_, fun a b hab ha => ?_⟩
  · rw [hf] at hab hbc ⊢
    rcases mem hab with e | e
    · subst e; exact hbc
    · rcases mem hbc with e' | e'
      · subst e'; exact hab
      · have := h2 _ e _ e'
        simpa using this
  · rw [hf] at hab
    rcases mem hab with e | e
    · subst e; rfl
    · have := (h1 _ e).1
      simpa using this
  · rw [hf] at hab ⊢
    rcases mem hab with e | e
    · subst e; simp [tableSub]
    · have := (h1 _ e).2
      simp only [ha, Bool.not_true, Bool.false_or] at this
      exact this

/-- A finite check that `LayOK` holds for subtyping by `tableSub tbl`. -/
def layOK (F : Fn) (tbl : List (VT × VT)) : Bool := tbl.all (fun e => layPairOK F e.1 e.2)

theorem layOK_sound {F : Fn} (hs : SubOK F) {tbl : List (VT × VT)} (hf : F.sub = tableSub tbl)
    (h : layOK F tbl = true) : LayOK F := by
  intro a b hab
  rw [hf] at hab
  simp only [tableSub, Bool.or_eq_true, beq_iff_eq, List.contains_iff_mem] at hab
  rcases hab with e | e
  · subst e
    have pref : ∀ fs : List Field, prefB F fs fs = true := by
      intro fs
      induction fs with
      | nil => rfl
      | cons f fs ih => simp [prefB, fieldB, ih, hs.refl]
    simp only [layPairOK, Bool.not_or_self, Bool.true_and]
    cases ha : decide (a < 0)
    · simp only [decide_eq_false_iff_not, Int.not_lt] at ha
      split
      · simp
      · simp [extB, *]
    · simp
  · simp only [layOK, List.all_eq_true] at h
    exact h _ e

end Wah.Lowered
