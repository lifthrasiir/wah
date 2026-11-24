/-
Model of `wah_check_lowered_code` in wah.h without GC objects and exceptions (P3a, P3b), and its soundness against
an abstract machine of the interpreter.

Code is a sequence of decoded instructions, indexed by instruction rather than by byte offset; branch targets that
are not instruction starts are out of range. Instructions only carry what the checker needs: numeric operators,
memory, table and global accesses, typed select and the like are `op` with the types that they pop and push (as
resolved against the module by the dumper of the bridge), bulk operations that can be suspended midway are `opMap`
with their ref maps, and calls are `call` with the types of the callee. Types are `wah_type_t` after
`wah_check_norm`, with the subtyping `sub` of the module (`wah_type_is_subtype`), which `SubOK` requires to be a
preorder that keeps references apart from other types and is monotone under `nn` (`WAH_TYPE_AS_NON_NULL`).

The machine (`steps`) runs on types of values only: conditions and nulls are not known, so it takes every branch.
A value may have a subtype of what the checker has for it, as stack maps join paths by subtyping. This is enough
for the properties that the interpreter relies on without checking them: operands have their types, branch targets
and fall-throughs are instructions, `keep + drop` values are there, the stack never grows beyond `maxDepth` (for
which the interpreter has made room), locals keep their types, results are there on returns, ref maps at
suspension points list exactly the slots of references (`exactMap`), as they are the GC roots, and calls are right
after a POLL, whose ref map scans the frame while the callee runs. Going wrong is `Out.wrong`; traps are allowed.

The checker (`check`) mirrors the C code: pass 1 checks targets of all instructions, and pass 2 walks from each join
point with the operand stack recorded there, merging the stack at each branch target and at fall-throughs into join
points (`T`), where it must equal the stack recorded first. Join points at `end` and at stack maps (`ann`) take
subtypes of the types annotated there on top of the stack, and record the annotated types instead.

`check_sound`: no state reachable from the entry of code accepted by `check` goes wrong.
-/
namespace Wah.Lowered

abbrev VT := Int

def i32 : VT := -2

/-- `WAH_TYPE_IS_REF` -/
def isRef (t : VT) : Bool := decide ((t ≤ -31 ∧ -54 ≤ t) ∨ 0 ≤ t)

/-- `WAH_TYPE_AS_NON_NULL` -/
def nn (t : VT) : VT := t - t % 2

inductive Instr
  | const (t : VT)
  | op (args res : List VT)
  | opMap (map : Option (List Nat)) (args res : List VT)
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
  deriving Inhabited

structure Fn where
  code : Array Instr
  locals : List VT
  results : List VT
  maxDepth : Nat
  maps : List (Nat × List VT)
  sub : VT → VT → Bool

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

/-! ## The machine -/

/-- A ref map that lists the slots of references exactly once each. -/
def exactMap (ps : List Nat) (s : List VT) : Bool :=
  decide ps.Nodup && ps.all (refAt s) && ps.length == refCount s

structure St where
  pc : Nat
  stk : List VT
  polled : Bool

inductive Out
  | next (st : St) | done | trap | wrong

def go (pc : Nat) (s : List VT) (p : Bool := false) : Out := .next ⟨pc, s, p⟩

def pushO (F : Fn) (pc : Nat) (ts s : List VT) : Out :=
  if s.length + ts.length ≤ F.maxDepth then go pc (ts.reverse ++ s) else .wrong

/-- Branches as `WAH_DROP_KEEP` does: the top `k` values stay and the `d` below them go. -/
def jump (t k d : Nat) (s : List VT) : Out :=
  if k + d ≤ s.length then go t (s.take k ++ s.drop (k + d)) else .wrong

def opO (F : Fn) (pc : Nat) (args res s : List VT) : List Out :=
  match popT F args.reverse s with
  | some s0 => [.trap, pushO F (pc + 1) res s0]
  | none => [.wrong]

def mapO (m : Option (List Nat)) (s : List VT) : Bool :=
  match m with
  | some ps => exactMap ps s
  | none => false

def stepI (F : Fn) (pc : Nat) (s : List VT) (lp : Bool) : Instr → List Out
  | .const t => [pushO F (pc + 1) [t] s]
  | .op args res => opO F pc args res s
  | .opMap m args res => if mapO m s then opO F pc args res s else [.wrong]
  | .call args res => if lp then opO F pc args res s else [.wrong]
  | .retCall args res => match popT F args.reverse s with
    | some _ => if leB F res F.results then [.trap, .done] else [.wrong]
    | none => [.wrong]
  | .drop => match s with
    | _ :: s0 => [go (pc + 1) s0]
    | [] => [.wrong]
  | .select => match s with
    | c :: a :: b :: s0 => if F.sub c i32 then [go (pc + 1) (a :: s0), go (pc + 1) (b :: s0)] else [.wrong]
    | _ => [.wrong]
  | .localGet i => match F.locals[i]? with
    | some t => [pushO F (pc + 1) [t] s]
    | none => [.wrong]
  | .localSet i => match F.locals[i]?, s with
    | some t, x :: s0 => if F.sub x t then [go (pc + 1) s0] else [.wrong]
    | _, _ => [.wrong]
  | .localTee i => match F.locals[i]?, s with
    | some t, x :: s0 => if F.sub x t then [pushO F (pc + 1) [t] s0] else [.wrong]
    | _, _ => [.wrong]
  | .refIsNull => match s with
    | x :: s0 => if isRef x then [pushO F (pc + 1) [i32] s0] else [.wrong]
    | [] => [.wrong]
  | .refAsNonNull => match s with
    | x :: s0 => if isRef x then [.trap, go (pc + 1) (nn x :: s0)] else [.wrong]
    | [] => [.wrong]
  | .br t k d => [jump t k d s]
  | .brIf t k d => match s with
    | c :: s0 => if F.sub c i32 then [go (pc + 1) s0, jump t k d s0] else [.wrong]
    | _ => [.wrong]
  | .brTable k recs => match s with
    | c :: s0 => if F.sub c i32 && !recs.isEmpty then recs.map (fun r => jump r.1 k r.2 s0) else [.wrong]
    | _ => [.wrong]
  | .brOnNull t k d => match s with
    | x :: s0 => if isRef x then [jump t k d s0, go (pc + 1) (nn x :: s0)] else [.wrong]
    | [] => [.wrong]
  | .brOnNonNull t k d => match s with
    | x :: s0 => if isRef x then [jump t k d (nn x :: s0), go (pc + 1) s0] else [.wrong]
    | [] => [.wrong]
  | .if_ t => match s with
    | c :: s0 => if F.sub c i32 then [go (pc + 1) s0, go t s0] else [.wrong]
    | _ => [.wrong]
  | .else_ t => [go t s]
  | .meter t => [go (pc + 1) s, go t s]
  | .poll m => if mapO m s then [go (pc + 1) s true] else [.wrong]
  | .tick m => if mapO m s then [go (pc + 1) s] else [.wrong]
  | .end_ => if leB F s F.results.reverse then [.done] else [.wrong]
  | .ret => if (popT F F.results.reverse s).isSome then [.done] else [.wrong]
  | .unreachable => [.trap]
  | .bad => [.wrong]

def steps (F : Fn) (st : St) : List Out :=
  match F.code[st.pc]? with
  | some i => stepI F st.pc st.stk st.polled i
  | none => [.wrong]

inductive Reach (F : Fn) : St → Prop
  | init : Reach F ⟨0, [], false⟩
  | step {st st'} : Reach F st → Out.next st' ∈ steps F st → Reach F st'

/-! ## The checker -/

/-- What an instruction does to the operand stack: branch edges, and the stack after it if it falls through. -/
inductive Xf
  | fail
  | stop (es : List (Nat × List VT))
  | next (es : List (Nat × List VT)) (s : List VT) (p : Bool)

def pushX (F : Fn) (ts s : List VT) : Option (List VT) :=
  if s.length + ts.length ≤ F.maxDepth then some (ts.reverse ++ s) else none

def edge (t k d : Nat) (s : List VT) : Option (Nat × List VT) :=
  if k + d ≤ s.length then some (t, s.take k ++ s.drop (k + d)) else none

def edges (k : Nat) (s : List VT) : List (Nat × Nat) → Option (List (Nat × List VT))
  | [] => some []
  | r :: rs => match edge r.1 k r.2 s, edges k s rs with
    | some e, some es => some (e :: es)
    | _, _ => none

def nextX (F : Fn) (ts s : List VT) : Xf :=
  match pushX F ts s with
  | some s' => .next [] s' false
  | none => .fail

def opX (F : Fn) (args res s : List VT) : Xf :=
  match popT F args.reverse s with
  | some s0 => nextX F res s0
  | none => .fail

/-- `wah_check_ref_map`: positions strictly decrease from the top, have references, and are all of them. -/
def decr (s : List VT) (below : Nat) : List Nat → Bool
  | [] => true
  | p :: ps => decide (p < below) && refAt s p && decr s p ps

def mapX (m : Option (List Nat)) (s : List VT) : Bool :=
  match m with
  | some ps => decr s s.length ps && ps.length == refCount s
  | none => false

def xfer (F : Fn) (s : List VT) (p : Bool) : Instr → Xf
  | .const t => nextX F [t] s
  | .op args res => opX F args res s
  | .opMap m args res => if mapX m s then opX F args res s else .fail
  | .call args res => if p then opX F args res s else .fail
  | .retCall args res => match popT F args.reverse s with
    | some _ => if leB F res F.results then .stop [] else .fail
    | none => .fail
  | .drop => match s with
    | _ :: s0 => .next [] s0 false
    | [] => .fail
  | .select => match s with
    | c :: a :: b :: s0 => if F.sub c i32 && a == b then .next [] (a :: s0) false else .fail
    | _ => .fail
  | .localGet i => match F.locals[i]? with
    | some t => nextX F [t] s
    | none => .fail
  | .localSet i => match F.locals[i]?, s with
    | some t, x :: s0 => if F.sub x t then .next [] s0 false else .fail
    | _, _ => .fail
  | .localTee i => match F.locals[i]?, s with
    | some t, x :: s0 => if F.sub x t then nextX F [t] s0 else .fail
    | _, _ => .fail
  | .refIsNull => match s with
    | x :: s0 => if isRef x then nextX F [i32] s0 else .fail
    | [] => .fail
  | .refAsNonNull => match s with
    | x :: s0 => if isRef x then .next [] (nn x :: s0) false else .fail
    | [] => .fail
  | .br t k d => match edge t k d s with
    | some e => .stop [e]
    | none => .fail
  | .brIf t k d => match s with
    | c :: s0 => if F.sub c i32 then match edge t k d s0 with
        | some e => .next [e] s0 false
        | none => .fail
      else .fail
    | _ => .fail
  | .brTable k recs => match s with
    | c :: s0 => if F.sub c i32 && !recs.isEmpty then match edges k s0 recs with
        | some es => .stop es
        | none => .fail
      else .fail
    | _ => .fail
  | .brOnNull t k d => match s with
    | x :: s0 => if isRef x then match edge t k d s0 with
        | some e => .next [e] (nn x :: s0) false
        | none => .fail
      else .fail
    | [] => .fail
  | .brOnNonNull t k d => match s with
    | x :: s0 => if isRef x then match edge t k d (nn x :: s0) with
        | some e => .next [e] s0 false
        | none => .fail
      else .fail
    | [] => .fail
  | .if_ t => match s with
    | c :: s0 => if F.sub c i32 then .next [(t, s0)] s0 false else .fail
    | _ => .fail
  | .else_ t => .stop [(t, s)]
  | .meter t => .next [(t, s)] s false
  | .poll m => if mapX m s then .next [] s true else .fail
  | .tick m => if mapX m s then .next [] s false else .fail
  | .end_ => if leB F s F.results.reverse then .stop [] else .fail
  | .ret => if (popT F F.results.reverse s).isSome then .stop [] else .fail
  | .unreachable => .stop []
  | .bad => .fail

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

/-- Recorded stacks at join points. -/
abbrev Joins := Nat → Option (List VT)

def upd (J : Joins) (t : Nat) (s : List VT) : Joins := fun u => if u = t then some s else J u

/-- `CK_MERGE`: records the stack at a join point, or checks it against the recorded one. -/
def mergeM (F : Fn) (jw : Joins × List Nat) (t : Nat) (s : List VT) : Option (Joins × List Nat) :=
  if annOK F t s then
    match jw.1 t with
    | none => some (upd jw.1 t (nrm F t s), t :: jw.2)
    | some s' => if s' = nrm F t s then some jw else none
  else none

def mergeAllM (F : Fn) : Joins × List Nat → List (Nat × List VT) → Option (Joins × List Nat)
  | jw, [] => some jw
  | jw, e :: es => match mergeM F jw e.1 e.2 with
    | some jw' => mergeAllM F jw' es
    | none => none

/-- Walks from `pc` with the stack `s`, where `p` tells whether the last instruction was POLL, until the path ends or
falls through into a join point (other than where it started, `first`). -/
def walkM (F : Fn) (T : Nat → Bool) : Nat → Joins × List Nat → Nat → List VT → Bool → Bool → Option (Joins × List Nat)
  | 0, _, _, _, _, _ => none
  | n + 1, jw, pc, s, p, first => match F.code[pc]? with
    | none => none
    | some i => if !first && T pc then mergeM F jw pc s else
      match xfer F s p i with
      | .fail => none
      | .stop es => mergeAllM F jw es
      | .next es s' p' => match mergeAllM F jw es with
        | some jw' => walkM F T n jw' (pc + 1) s' p' false
        | none => none

/-- The worklist of join points to walk from, last in first out. -/
def loopM (F : Fn) (T : Nat → Bool) (N : Nat) : Nat → Joins → List Nat → Option Joins
  | 0, _, _ => none
  | _ + 1, J, [] => some J
  | n + 1, J, t :: w => match J t with
    | some s => match walkM F T N (J, w) t s false true with
      | some jw => loopM F T N n jw.1 jw.2
      | none => none
    | none => none

def targets : Instr → List Nat
  | .br t _ _ | .brIf t _ _ | .brOnNull t _ _ | .brOnNonNull t _ _ | .if_ t | .else_ t | .meter t => [t]
  | .brTable _ recs => recs.map (·.1)
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
  match mergeM F (fun _ => none, []) 0 [] with
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
def mergeOK (F : Fn) (J : Joins) (t : Nat) (s : List VT) : Bool := annOK F t s && J t == some (nrm F t s)

def wkB (F : Fn) (T : Nat → Bool) (J : Joins) : Nat → Nat → List VT → Bool → Bool → Bool
  | 0, _, _, _, _ => false
  | n + 1, pc, s, p, first => match F.code[pc]? with
    | none => false
    | some i => if !first && T pc then mergeOK F J pc s else
      match xfer F s p i with
      | .fail => false
      | .stop es => es.all (fun e => mergeOK F J e.1 e.2)
      | .next es s' p' => es.all (fun e => mergeOK F J e.1 e.2) && wkB F T J n (pc + 1) s' p' false

def Ext (J J' : Joins) : Prop := ∀ t s, J t = some s → J' t = some s

theorem Ext.refl (J : Joins) : Ext J J := fun _ _ h => h
theorem Ext.trans {J J' J'' : Joins} (a : Ext J J') (b : Ext J' J'') : Ext J J'' := fun t s h => b t s (a t s h)

theorem mergeOK_mono {F : Fn} {J J' : Joins} (h : Ext J J') {t s} (hm : mergeOK F J t s = true) :
    mergeOK F J' t s = true := by
  simp only [mergeOK, Bool.and_eq_true, beq_iff_eq] at hm ⊢
  exact ⟨hm.1, h _ _ hm.2⟩

theorem all_mergeOK_mono {F : Fn} {J J' : Joins} (h : Ext J J') {es : List (Nat × List VT)}
    (hm : es.all (fun e => mergeOK F J e.1 e.2) = true) : es.all (fun e => mergeOK F J' e.1 e.2) = true := by
  simp only [List.all_eq_true] at hm ⊢
  exact fun e he => mergeOK_mono h (hm e he)

theorem wkB_mono {F : Fn} {T} {J J' : Joins} (h : Ext J J') :
    ∀ n pc s p first, wkB F T J n pc s p first = true → wkB F T J' n pc s p first = true := by
  intro n
  induction n with
  | zero => intro _ _ _ _ hw; simp [wkB] at hw
  | succ n ih =>
    intro pc s p first hw
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
          exact ⟨all_mergeOK_mono h hw.1, ih _ _ _ _ hw.2⟩

/-- Joins whose new entries are all in the worklist. -/
def Tracked (J J' : Joins) (w : List Nat) : Prop := ∀ t, J' t ≠ none → J t ≠ none ∨ t ∈ w

def Listed (J : Joins) (w : List Nat) : Prop := ∀ t ∈ w, J t ≠ none

theorem mergeM_spec {F : Fn} {jw jw' : Joins × List Nat} {t s} (h : mergeM F jw t s = some jw') :
    Ext jw.1 jw'.1 ∧ mergeOK F jw'.1 t s = true ∧ Tracked jw.1 jw'.1 jw'.2 ∧ (∀ u ∈ jw.2, u ∈ jw'.2) ∧
    (Listed jw.1 jw.2 → Listed jw'.1 jw'.2) := by
  unfold mergeM at h
  split at h
  · rename_i ha
    split at h
    · rename_i hn
      cases h
      refine ⟨?_, ?_, ?_, ?_, ?_⟩
      · intro u s' hu
        simp only [upd]
        split
        · subst_vars; rw [hn] at hu; cases hu
        · exact hu
      · simp [mergeOK, ha, upd]
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
    · rename_i s' hs
      split at h
      · cases h
        rename_i e
        refine ⟨Ext.refl _, ?_, fun u hu => Or.inl hu, fun u hu => hu, id⟩
        simp [mergeOK, ha, hs, e]
      · cases h
  · cases h

theorem mergeAllM_spec {F : Fn} : ∀ {es : List (Nat × List VT)} {jw jw' : Joins × List Nat},
    mergeAllM F jw es = some jw' →
    Ext jw.1 jw'.1 ∧ es.all (fun e => mergeOK F jw'.1 e.1 e.2) = true ∧ Tracked jw.1 jw'.1 jw'.2 ∧
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

theorem walkM_spec {F : Fn} {T} : ∀ {n pc s p first} {jw jw' : Joins × List Nat},
    walkM F T n jw pc s p first = some jw' →
    Ext jw.1 jw'.1 ∧ wkB F T jw'.1 n pc s p first = true ∧ Tracked jw.1 jw'.1 jw'.2 ∧
    (∀ u ∈ jw.2, u ∈ jw'.2) ∧ (Listed jw.1 jw.2 → Listed jw'.1 jw'.2)
  | 0, _, _, _, _, _, _, h => by simp [walkM] at h
  | n + 1, pc, s, p, first, jw, jw', h => by
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
        · rename_i es s' p' hx
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
  ∀ t s, J t = some s → wkB F T J N t s false true = true

theorem loopM_spec {F : Fn} {T} {N} : ∀ {k J w J'}, loopM F T N k J w = some J' →
    Listed J w → (∀ t s, J t = some s → t ∈ w ∨ wkB F T J N t s false true = true) →
    Ext J J' ∧ Final F T N J'
  | 0, _, _, _, h, _, _ => by simp [loopM] at h
  | k + 1, J, [], J', h, _, hd => by
    simp only [loopM, Option.some.injEq] at h
    subst h
    refine ⟨Ext.refl _, fun t s ht => ?_⟩
    rcases hd t s ht with a | a
    · cases a
    · exact a
  | k + 1, J, t :: w, J', h, hl, hd => by
    simp only [loopM] at h
    split at h
    · rename_i s hs
      split at h
      · rename_i jw hw
        obtain ⟨x, wk, tr, ww, ll⟩ := walkM_spec hw
        have hl' : Listed jw.1 jw.2 := ll (fun u hu => hl u (List.mem_cons_of_mem _ hu))
        obtain ⟨x2, fin⟩ := loopM_spec h hl' (by
          intro u s' hu
          by_cases e : J u = none
          · rcases tr u (by rw [hu]; simp) with a | a
            · exact absurd e a
            · exact Or.inl a
          · obtain ⟨s2, hs2⟩ := Option.ne_none_iff_exists'.mp e
            have e2 := x u s2 hs2
            rw [hu] at e2
            obtain rfl := Option.some.inj e2
            rcases hd u _ hs2 with a | a
            · rcases List.mem_cons.mp a with b | b
              · subst b
                rw [hs] at hs2
                cases hs2
                exact Or.inr wk
              · exact Or.inl (ww u b)
            · exact Or.inr (wkB_mono x _ _ _ _ _ a))
        exact ⟨Ext.trans x x2, fin⟩
      · cases h
    · cases h

/-! ### Each step agrees with `xfer` -/

/-- What the machine may do from an instruction that the checker passes, with a stack of subtypes. -/
def Covered (F : Fn) (pc : Nat) (o : Out) : Xf → Prop
  | .fail => True
  | .stop es => o ≠ .wrong ∧ ∀ st, o = .next st → st.polled = false ∧ ∃ e ∈ es, st.pc = e.1 ∧ Le F st.stk e.2
  | .next es s' p' => o ≠ .wrong ∧ ∀ st, o = .next st →
      (st.polled = false ∧ ∃ e ∈ es, st.pc = e.1 ∧ Le F st.stk e.2) ∨
      (st.pc = pc + 1 ∧ st.polled = p' ∧ Le F st.stk s')

theorem jump_covered {F : Fn} {t k d M S e o} (hl : Le F M S) (he : edge t k d S = some e) (ho : o = jump t k d M) :
    o ≠ .wrong ∧ ∀ st, o = .next st → st.polled = false ∧ st.pc = e.1 ∧ Le F st.stk e.2 := by
  simp only [edge] at he
  split at he
  · rename_i hk
    cases he
    subst ho
    simp only [jump, go, hl.length, hk, ite_true]
    refine ⟨by simp, fun st h => ?_⟩
    simp only [Out.next.injEq] at h
    subst h
    exact ⟨rfl, rfl, Le.append (Le.take k hl) (Le.drop (k + d) hl)⟩
  · cases he

theorem edges_mem {k s} : ∀ {recs : List (Nat × Nat)} {es}, edges k s recs = some es →
    ∀ r ∈ recs, ∃ e, edge r.1 k r.2 s = some e ∧ e ∈ es
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

section
variable {F : Fn} (hs : SubOK F)
include hs

theorem push_covered {pc ts M S o} (hl : Le F M S) (ho : o = pushO F (pc + 1) ts M) :
    Covered F pc o (nextX F ts S) := by
  subst ho
  simp only [nextX, pushX, pushO, hl.length]
  split
  · refine ⟨by simp [go], fun st h => Or.inr ?_⟩
    simp only [go, Out.next.injEq] at h
    subst h
    exact ⟨rfl, rfl, Le.append (Le.refl hs _) hl⟩
  · simp [Covered]

theorem op_covered {pc args res M S o} (hl : Le F M S) (ho : o ∈ opO F pc args res M) :
    Covered F pc o (opX F args res S) := by
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

theorem step_covered (pc : Nat) (M S : List VT) (hl : Le F M S) (p lp : Bool) (hp : p = true → lp = true)
    (i : Instr) (o : Out) (ho : o ∈ stepI F pc M lp i) : Covered F pc o (xfer F S p i) := by
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
  | call args res =>
    simp only [xfer]
    split
    · rename_i hpt
      simp only [stepI, hp hpt, ite_true] at ho
      exact op_covered hs hl ho
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
        simp_all
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
              · exact ⟨rfl, rfl, .cons h2 t3⟩
              · exact ⟨rfl, rfl, .cons h3 t3⟩
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
          exact ⟨rfl, rfl, t1⟩
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
            exact ⟨rfl, rfl, .cons (hs.nn _ _ h1 ey) t1⟩
      · trivial
    · trivial
  | br t k d =>
    simp only [xfer]
    split
    · rename_i e he
      obtain ⟨nw, hn⟩ := jump_covered hl he (List.mem_singleton.mp ho)
      exact ⟨nw, fun st h => by obtain ⟨a, b, c⟩ := hn st h; exact ⟨a, e, by simp, b, c⟩⟩
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
              exact ⟨rfl, rfl, t1⟩
            · obtain ⟨nw, hn⟩ := jump_covered t1 he h
              exact ⟨nw, fun st h' => Or.inl (by obtain ⟨a, b, c⟩ := hn st h'; exact ⟨a, e, by simp, b, c⟩)⟩
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
            exact ⟨nw, fun st h => by obtain ⟨a, b, c⟩ := hn st h; exact ⟨a, e, hm, b, c⟩⟩
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
              exact ⟨nw, fun st h' => Or.inl (by obtain ⟨a, b, c⟩ := hn st h'; exact ⟨a, e, by simp, b, c⟩)⟩
            · subst h
              refine ⟨by simp [go], fun st h => Or.inr ?_⟩
              simp only [go, Out.next.injEq] at h
              subst h
              exact ⟨rfl, rfl, .cons (hs.nn _ _ h1 ey) t1⟩
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
              exact ⟨nw, fun st h' => Or.inl (by obtain ⟨a, b, c⟩ := hn st h'; exact ⟨a, e, by simp, b, c⟩)⟩
            · subst h
              refine ⟨by simp [go], fun st h => Or.inr ?_⟩
              simp only [go, Out.next.injEq] at h
              subst h
              exact ⟨rfl, rfl, t1⟩
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
          · exact Or.inr ⟨rfl, rfl, t1⟩
          · exact Or.inl ⟨rfl, (t, S0), by simp, rfl, t1⟩
      · trivial
    · trivial
  | else_ t =>
    simp only [stepI, List.mem_singleton] at ho
    subst ho
    refine ⟨by simp [go], fun st h => ?_⟩
    simp only [go, Out.next.injEq] at h
    subst h
    exact ⟨rfl, (t, S), by simp, rfl, hl⟩
  | meter t =>
    simp only [stepI, List.mem_cons, List.not_mem_nil, or_false] at ho
    simp only [xfer]
    refine ⟨by rcases ho with h | h <;> simp [h, go], fun st h => ?_⟩
    rcases ho with e | e <;> subst e <;> simp only [go, Out.next.injEq] at h <;> subst h
    · exact Or.inr ⟨rfl, rfl, hl⟩
    · exact Or.inl ⟨rfl, (t, S), by simp, rfl, hl⟩
  | poll m =>
    simp only [xfer]
    split
    · rename_i hm
      simp only [stepI, mapX_exact hs hl hm, ite_true, List.mem_singleton] at ho
      subst ho
      refine ⟨by simp [go], fun st h => Or.inr ?_⟩
      simp only [go, Out.next.injEq] at h
      subst h
      exact ⟨rfl, rfl, hl⟩
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
      exact ⟨rfl, rfl, hl⟩
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

end

/-! ### Reachable states are walked -/

def Inv (F : Fn) (T : Nat → Bool) (J : Joins) (st : St) : Prop :=
  ∃ n p S, (p = true → st.polled = true) ∧ Le F st.stk S ∧ wkB F T J n st.pc S p true = true

theorem inv_of_join {F : Fn} (hs : SubOK F) {T N J} (fin : Final F T N J) {t M S} (hl : Le F M S)
    (h : mergeOK F J t S = true) : Inv F T J ⟨t, M, false⟩ := by
  simp only [mergeOK, Bool.and_eq_true, beq_iff_eq] at h
  exact ⟨N, false, _, by simp, le_nrm hs h.1 hl, fin t _ h.2⟩

theorem inv_step {F : Fn} (hs : SubOK F) {T N J} (fin : Final F T N J) {st : St} (hi : Inv F T J st) :
    ∀ o ∈ steps F st, o ≠ .wrong ∧ ∀ st', o = .next st' → Inv F T J st' := by
  intro o ho
  obtain ⟨n, p, S, hp, hl, hw⟩ := hi
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
    have cov := step_covered hs st.pc st.stk S hl p st.polled hp i o ho
    revert cov hw
    cases xfer F S p i with
    | fail => simp
    | stop es =>
      intro hw cov
      refine ⟨cov.1, fun st' h => ?_⟩
      obtain ⟨hpo, e, hm, hpc, hle⟩ := cov.2 st' h
      simp only [List.all_eq_true] at hw
      have := inv_of_join hs fin hle (hw _ hm)
      cases st'; simp only at hpo hpc; subst hpo hpc; exact this
    | next es s' p' =>
      intro hw cov
      simp only [Bool.and_eq_true, List.all_eq_true] at hw
      refine ⟨cov.1, fun st' h => ?_⟩
      rcases cov.2 st' h with ⟨hpo, e, hm, hpc, hle⟩ | ⟨hpc, hpo, hle⟩
      · have := inv_of_join hs fin hle (hw.1 _ hm)
        cases st'; simp only at hpo hpc; subst hpo hpc; exact this
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
            obtain ⟨n', p'', S', hp'', hl'', hw''⟩ := inv_of_join hs fin hle h2
            refine ⟨n', p'', S', fun e => absurd (hp'' e) (by simp), hl'', ?_⟩
            rw [hpc]; exact hw''
          · refine ⟨n + 1, p', s', fun e => by rw [hpo]; exact e, hle, ?_⟩
            rw [hpc]
            simp only [wkB, hc', Bool.not_true, Bool.false_and]
            simp only [hT, Bool.and_false] at h2
            exact h2

theorem check_final {F : Fn} (h : check F = true) :
    ∃ J, Final F (fun pc => (joinPoints F)[pc]?.getD false) (F.code.size + 1) J ∧ mergeOK F J 0 [] = true := by
  simp only [check, Bool.and_eq_true] at h
  obtain ⟨_, hm⟩ := h
  split at hm
  · rename_i jw hjw
    obtain ⟨J, hJ⟩ := Option.isSome_iff_exists.mp hm
    obtain ⟨x, m, tr, _, ll⟩ := mergeM_spec hjw
    have l0 : Listed jw.1 jw.2 := ll (fun _ h => by cases h)
    obtain ⟨x2, fin⟩ := loopM_spec hJ l0 (by
      intro t s ht
      rcases tr t (by rw [ht]; simp) with a | a
      · exact absurd rfl a
      · exact Or.inl a)
    exact ⟨J, fin, mergeOK_mono x2 m⟩
  · exact absurd hm (by simp)

/-- Code accepted by the checker never goes wrong, given a subtyping like that of the spec. -/
theorem check_sound {F : Fn} (hs : SubOK F) (h : check F = true) {st : St} (hr : Reach F st) :
    ∀ o ∈ steps F st, o ≠ .wrong := by
  obtain ⟨J, fin, h0⟩ := check_final h
  have : Inv F (fun pc => (joinPoints F)[pc]?.getD false) J st := by
    induction hr with
    | init => exact inv_of_join hs fin .nil h0
    | step _ hs' ih => exact (inv_step hs fin ih _ hs').2 _ rfl
  exact fun o ho => (inv_step hs fin this o ho).1

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

end Wah.Lowered
