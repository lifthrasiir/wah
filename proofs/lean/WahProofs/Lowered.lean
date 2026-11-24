/-
Model of the numeric and control part of `wah_check_lowered_code` in wah.h (P3a), and its soundness against an
abstract machine of the interpreter.

Code is a sequence of decoded instructions, indexed by instruction rather than by byte offset; branch targets that
are not instruction starts are out of range. Instructions only carry what the checker needs: numeric operators,
memory accesses and global accesses are `op` with the types that they pop and push (as resolved against the module
by the dumper of the bridge), and calls are `call` with the types of the callee.

The machine (`steps`) runs on the types of values only: conditions are not known, so it takes every branch. This is
enough for the properties that the interpreter relies on without checking them: operands have their types, branch
targets and fall-throughs are instructions, `keep + drop` values are there, the stack never grows beyond
`maxDepth` (for which the interpreter has made room), locals keep their types, results are there on returns, ref maps
at suspension points are exact (`poll ok` and `tick ok`, which the dumper reports, as no references are tracked here),
and calls are right after a POLL, whose ref map scans the frame while the callee runs. Going wrong is `Out.wrong`;
traps are allowed.

The checker (`check`) mirrors the C code: pass 1 checks targets of all instructions, and pass 2 walks from each join
point with the operand stack recorded there, merging the stack at each branch target and at fall-throughs into join
points (`T`), where it must equal the stack recorded first. Join points at `end` (and at stack maps, `ann`) check
the types on top of the stack.

`check_sound`: no state reachable from the entry of code accepted by `check` goes wrong.
-/
namespace Wah.Lowered

inductive VT | i32 | i64 | f32 | f64 | v128
  deriving DecidableEq, Repr, Inhabited

inductive Instr
  | const (t : VT)
  | op (args res : List VT)
  | call (args res : List VT)
  | retCall (args res : List VT)
  | drop | select
  | localGet (i : Nat) | localSet (i : Nat) | localTee (i : Nat)
  | br (t keep drop : Nat) | brIf (t keep drop : Nat) | brTable (keep : Nat) (recs : List (Nat × Nat))
  | if_ (t : Nat) | else_ (t : Nat) | meter (t : Nat)
  | poll (ok : Bool) | tick (ok : Bool)
  | end_ | ret | unreachable | bad
  deriving Repr, Inhabited

structure Fn where
  code : Array Instr
  locals : List VT
  results : List VT
  maxDepth : Nat
  maps : List (Nat × List VT)

/-- Pops the types `ts` (top first) off the stack `s` (top first). -/
def popT : List VT → List VT → Option (List VT)
  | [], s => some s
  | t :: ts, x :: s => if x = t then popT ts s else none
  | _ :: _, [] => none

/-! ## The machine -/

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

def stepI (F : Fn) (pc : Nat) (s : List VT) (lp : Bool) : Instr → List Out
  | .const t => [pushO F (pc + 1) [t] s]
  | .op args res => match popT args.reverse s with
    | some s0 => [.trap, pushO F (pc + 1) res s0]
    | none => [.wrong]
  | .call args res => if lp then match popT args.reverse s with
      | some s0 => [.trap, pushO F (pc + 1) res s0]
      | none => [.wrong]
    else [.wrong]
  | .retCall args res => match popT args.reverse s with
    | some _ => if res = F.results then [.trap, .done] else [.wrong]
    | none => [.wrong]
  | .drop => match s with
    | _ :: s0 => [go (pc + 1) s0]
    | [] => [.wrong]
  | .select => match s with
    | .i32 :: a :: b :: s0 => [go (pc + 1) (a :: s0), go (pc + 1) (b :: s0)]
    | _ => [.wrong]
  | .localGet i => match F.locals[i]? with
    | some t => [pushO F (pc + 1) [t] s]
    | none => [.wrong]
  | .localSet i => match F.locals[i]?, s with
    | some t, x :: s0 => if x = t then [go (pc + 1) s0] else [.wrong]
    | _, _ => [.wrong]
  | .localTee i => match F.locals[i]?, s with
    | some t, x :: s0 => if x = t then [pushO F (pc + 1) [t] s0] else [.wrong]
    | _, _ => [.wrong]
  | .br t k d => [jump t k d s]
  | .brIf t k d => match s with
    | .i32 :: s0 => [go (pc + 1) s0, jump t k d s0]
    | _ => [.wrong]
  | .brTable k recs => match s with
    | .i32 :: s0 => if recs = [] then [.wrong] else recs.map (fun r => jump r.1 k r.2 s0)
    | _ => [.wrong]
  | .if_ t => match s with
    | .i32 :: s0 => [go (pc + 1) s0, go t s0]
    | _ => [.wrong]
  | .else_ t => [go t s]
  | .meter t => [go (pc + 1) s, go t s]
  | .poll ok => if ok then [go (pc + 1) s true] else [.wrong]
  | .tick ok => if ok then [go (pc + 1) s] else [.wrong]
  | .end_ => if s = F.results.reverse then [.done] else [.wrong]
  | .ret => if (popT F.results.reverse s).isSome then [.done] else [.wrong]
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

def xfer (F : Fn) (s : List VT) (p : Bool) : Instr → Xf
  | .const t => nextX F [t] s
  | .op args res => match popT args.reverse s with
    | some s0 => nextX F res s0
    | none => .fail
  | .call args res => if p then match popT args.reverse s with
      | some s0 => nextX F res s0
      | none => .fail
    else .fail
  | .retCall args res => match popT args.reverse s with
    | some _ => if res = F.results then .stop [] else .fail
    | none => .fail
  | .drop => match s with
    | _ :: s0 => .next [] s0 false
    | [] => .fail
  | .select => match s with
    | .i32 :: a :: b :: s0 => if a = b then .next [] (a :: s0) false else .fail
    | _ => .fail
  | .localGet i => match F.locals[i]? with
    | some t => nextX F [t] s
    | none => .fail
  | .localSet i => match F.locals[i]?, s with
    | some t, x :: s0 => if x = t then .next [] s0 false else .fail
    | _, _ => .fail
  | .localTee i => match F.locals[i]?, s with
    | some t, x :: s0 => if x = t then nextX F [t] s0 else .fail
    | _, _ => .fail
  | .br t k d => match edge t k d s with
    | some e => .stop [e]
    | none => .fail
  | .brIf t k d => match s with
    | .i32 :: s0 => match edge t k d s0 with
      | some e => .next [e] s0 false
      | none => .fail
    | _ => .fail
  | .brTable k recs => match s with
    | .i32 :: s0 => if recs = [] then .fail else match edges k s0 recs with
      | some es => .stop es
      | none => .fail
    | _ => .fail
  | .if_ t => match s with
    | .i32 :: s0 => .next [(t, s0)] s0 false
    | _ => .fail
  | .else_ t => .stop [(t, s)]
  | .meter t => .next [(t, s)] s false
  | .poll ok => if ok then .next [] s true else .fail
  | .tick ok => if ok then .next [] s false else .fail
  | .end_ => if s = F.results.reverse then .stop [] else .fail
  | .ret => if (popT F.results.reverse s).isSome then .stop [] else .fail
  | .unreachable => .stop []
  | .bad => .fail

/-- Types annotated at a join point: results at `end`, or a stack map, which are on top of the stack bottom first. -/
def ann (F : Fn) (t : Nat) : Option (List VT) :=
  match F.code[t]? with
  | some .end_ => some F.results
  | _ => (F.maps.find? (·.1 == t)).map (·.2)

def annOK (F : Fn) (t : Nat) (s : List VT) : Bool :=
  match ann F t with
  | some a => decide (a.length ≤ s.length) && decide (s.take a.length = a.reverse)
  | none => true

/-- Recorded stacks at join points. -/
abbrev Joins := Nat → Option (List VT)

def upd (J : Joins) (t : Nat) (s : List VT) : Joins := fun u => if u = t then some s else J u

/-- `CK_MERGE`: records the stack at a join point, or checks it against the recorded one. -/
def mergeM (F : Fn) (jw : Joins × List Nat) (t : Nat) (s : List VT) : Option (Joins × List Nat) :=
  if annOK F t s then
    match jw.1 t with
    | none => some (upd jw.1 t s, t :: jw.2)
    | some s' => if s' = s then some jw else none
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
  | .br t _ _ | .brIf t _ _ | .if_ t | .else_ t | .meter t => [t]
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
  F.maps.all (·.1 < F.code.size) && (F.maps.map (·.1)).Nodup &&
  match mergeM F (fun _ => none, []) 0 [] with
  | some jw => (loopM F T N (N + 1) jw.1 jw.2).isSome
  | none => false

/-! ## Soundness -/

/-- The checks of a walk against final joins. -/
def mergeOK (F : Fn) (J : Joins) (t : Nat) (s : List VT) : Bool := annOK F t s && J t == some s

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

/-- What the machine may do from an instruction that the checker passes. -/
def Covered (pc : Nat) (o : Out) : Xf → Prop
  | .fail => True
  | .stop es => o ≠ .wrong ∧ ∀ st, o = .next st → st.polled = false ∧ (st.pc, st.stk) ∈ es
  | .next es s' p' => o ≠ .wrong ∧ ∀ st, o = .next st →
      (st.polled = false ∧ (st.pc, st.stk) ∈ es) ∨ st = ⟨pc + 1, s', p'⟩

theorem nextX_covered {F : Fn} {pc ts s o} (ho : o ∈ [pushO F (pc + 1) ts s]) :
    Covered pc o (nextX F ts s) := by
  simp only [List.mem_singleton] at ho
  subst ho
  simp only [nextX, pushX, pushO]
  split
  · simp [Covered, go]
  · simp [Covered]

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

theorem jump_covered {t k d s e o} (he : edge t k d s = some e) (ho : o = jump t k d s) :
    o ≠ .wrong ∧ ∀ st, o = .next st → st.polled = false ∧ (st.pc, st.stk) = e := by
  simp only [edge] at he
  split at he
  · rename_i hk
    cases he
    subst ho
    simp only [jump, go, hk, ite_true]
    refine ⟨by simp, fun st h => ?_⟩
    cases h
    exact ⟨rfl, rfl⟩
  · cases he

theorem step_covered (F : Fn) (pc : Nat) (s : List VT) (p lp : Bool) (hp : p = true → lp = true) (i : Instr)
    (o : Out) (ho : o ∈ stepI F pc s lp i) : Covered pc o (xfer F s p i) := by
  cases i with
  | const t => exact nextX_covered ho
  | op args res =>
    simp only [stepI] at ho
    simp only [xfer]
    split
    · rename_i s0 h0
      rw [h0] at ho
      rcases List.mem_cons.mp ho with a | a
      · subst a
        simp only [nextX, pushX]
        split <;> simp [Covered]
      · exact nextX_covered a
    · trivial
  | call args res =>
    simp only [xfer]
    split
    · rename_i hpt
      have hl := hp hpt
      simp only [stepI, hl, ite_true] at ho
      split
      · rename_i s0 h0
        rw [h0] at ho
        rcases List.mem_cons.mp ho with a | a
        · subst a
          simp only [nextX, pushX]
          split <;> simp [Covered]
        · exact nextX_covered a
      · trivial
    · trivial
  | retCall args res =>
    simp only [stepI] at ho
    simp only [xfer]
    split
    · rename_i s0 h0
      rw [h0] at ho
      split
      · rename_i e
        simp only [e, ite_true, List.mem_cons, List.not_mem_nil, or_false] at ho
        rcases ho with a | a <;> subst a <;> simp [Covered]
      · trivial
    · trivial
  | drop =>
    simp only [stepI] at ho
    simp only [xfer]
    split
    · rename_i x s0
      simp only [List.mem_singleton] at ho
      subst ho
      simp [Covered, go]
    · trivial
  | select =>
    simp only [stepI] at ho
    simp only [xfer]
    split
    · rename_i a b s0
      split
      · rename_i e
        subst e
        simp only [List.mem_cons, List.not_mem_nil, or_false] at ho
        rcases ho with h | h <;> subst h <;> simp [Covered, go]
      · trivial
    · trivial
  | localGet i =>
    simp only [stepI] at ho
    simp only [xfer]
    split
    · rename_i t ht
      rw [ht] at ho
      exact nextX_covered ho
    · trivial
  | localSet i =>
    simp only [stepI] at ho
    simp only [xfer]
    split
    · rename_i t x s0 ht
      split
      · rename_i e
        simp only [ht, e, ite_true, List.mem_singleton] at ho
        subst ho
        simp [Covered, go]
      · trivial
    · trivial
  | localTee i =>
    simp only [stepI] at ho
    simp only [xfer]
    split
    · rename_i t x s0 ht
      split
      · rename_i e
        simp only [ht, e, ite_true] at ho
        exact nextX_covered ho
      · trivial
    · trivial
  | br t k d =>
    simp only [stepI, List.mem_singleton] at ho
    simp only [xfer]
    split
    · rename_i e he
      obtain ⟨nw, hn⟩ := jump_covered he ho
      refine ⟨nw, fun st h => ?_⟩
      obtain ⟨a, b⟩ := hn st h
      exact ⟨a, by simp [b]⟩
    · trivial
  | brIf t k d =>
    simp only [stepI] at ho
    simp only [xfer]
    split
    · rename_i s0
      split
      · rename_i e he
        simp only [List.mem_cons, List.not_mem_nil, or_false] at ho
        rcases ho with h | h
        · subst h; simp [Covered, go]
        · obtain ⟨nw, hn⟩ := jump_covered he h
          refine ⟨nw, fun st h' => Or.inl ?_⟩
          obtain ⟨a, b⟩ := hn st h'
          exact ⟨a, by simp [b]⟩
      · trivial
    · trivial
  | brTable k recs =>
    simp only [stepI] at ho
    simp only [xfer]
    split
    · rename_i s0
      split
      · trivial
      · rename_i hr
        simp only [hr, ite_false] at ho
        split
        · rename_i es hes
          obtain ⟨r, hr', hro⟩ := List.mem_map.mp ho
          obtain ⟨e, he, hm⟩ := edges_mem hes r hr'
          obtain ⟨nw, hn⟩ := jump_covered he hro.symm
          refine ⟨nw, fun st h => ?_⟩
          obtain ⟨a, b⟩ := hn st h
          exact ⟨a, by rw [b]; exact hm⟩
        · trivial
    · trivial
  | if_ t =>
    simp only [stepI] at ho
    simp only [xfer]
    split
    · rename_i s0
      simp only [List.mem_cons, List.not_mem_nil, or_false] at ho
      rcases ho with h | h <;> subst h <;> simp [Covered, go]
    · trivial
  | else_ t =>
    simp only [stepI, List.mem_singleton] at ho
    subst ho
    simp [xfer, Covered, go]
  | meter t =>
    simp only [stepI, List.mem_cons, List.not_mem_nil, or_false] at ho
    rcases ho with h | h <;> subst h <;> simp [xfer, Covered, go]
  | poll ok =>
    simp only [xfer]
    split
    · rename_i e
      simp only [stepI, e, ite_true, List.mem_singleton] at ho
      subst ho
      simp [Covered, go]
    · trivial
  | tick ok =>
    simp only [xfer]
    split
    · rename_i e
      simp only [stepI, e, ite_true, List.mem_singleton] at ho
      subst ho
      simp [Covered, go]
    · trivial
  | end_ =>
    simp only [xfer]
    split
    · rename_i e
      simp only [stepI, e, ite_true, List.mem_singleton] at ho
      subst ho
      simp [Covered]
    · trivial
  | ret =>
    simp only [xfer]
    split
    · rename_i e
      simp only [stepI, e, ite_true, List.mem_singleton] at ho
      subst ho
      simp [Covered]
    · trivial
  | unreachable =>
    simp only [stepI, List.mem_singleton] at ho
    subst ho
    simp [xfer, Covered]
  | bad => simp [xfer, Covered]

/-! ### Reachable states are walked -/

def Inv (F : Fn) (T : Nat → Bool) (J : Joins) (st : St) : Prop :=
  ∃ n p, (p = true → st.polled = true) ∧ wkB F T J n st.pc st.stk p true = true

theorem inv_of_join {F : Fn} {T N J} (fin : Final F T N J) {t s} (h : mergeOK F J t s = true) :
    Inv F T J ⟨t, s, false⟩ := by
  simp only [mergeOK, Bool.and_eq_true, beq_iff_eq] at h
  exact ⟨N, false, by simp, fin t s h.2⟩

theorem inv_step {F : Fn} {T N J} (fin : Final F T N J) {st : St} (hi : Inv F T J st) :
    ∀ o ∈ steps F st, o ≠ .wrong ∧ ∀ st', o = .next st' → Inv F T J st' := by
  intro o ho
  obtain ⟨n, p, hp, hw⟩ := hi
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
    have cov := step_covered F st.pc st.stk p st.polled hp i o ho
    revert cov hw
    cases xfer F st.stk p i with
    | fail => simp
    | stop es =>
      intro hw cov
      refine ⟨cov.1, fun st' h => ?_⟩
      obtain ⟨hpo, hm⟩ := cov.2 st' h
      simp only [List.all_eq_true] at hw
      have := inv_of_join fin (hw _ hm)
      cases st'; simp only at hpo; subst hpo; exact this
    | next es s' p' =>
      intro hw cov
      simp only [Bool.and_eq_true, List.all_eq_true] at hw
      refine ⟨cov.1, fun st' h => ?_⟩
      rcases cov.2 st' h with ⟨hpo, hm⟩ | e
      · have := inv_of_join fin (hw.1 _ hm)
        cases st'; simp only at hpo; subst hpo; exact this
      · subst e
        have h2 := hw.2
        cases n with
        | zero => simp [wkB] at h2
        | succ n =>
        simp only [wkB] at h2
        split at h2
        · exact absurd h2 (by simp)
        · rename_i i' hc'
          by_cases hT : T (st.pc + 1) = true
          · simp only [hT, Bool.not_false, Bool.and_self, ite_true] at h2
            obtain ⟨n', p'', hp'', hw''⟩ := inv_of_join fin h2
            exact ⟨n', p'', fun e => absurd (hp'' e) (by simp), hw''⟩
          · refine ⟨n + 1, p', fun e => e, ?_⟩
            simp only [wkB, hc', Bool.not_true, Bool.false_and]
            simp only [hT, Bool.and_false] at h2
            exact h2

theorem check_final {F : Fn} (h : check F = true) :
    ∃ J, Final F (fun pc => (joinPoints F)[pc]?.getD false) (F.code.size + 1) J ∧ J 0 = some [] := by
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
    refine ⟨J, fin, x2 0 [] ?_⟩
    simp only [mergeOK, Bool.and_eq_true, beq_iff_eq] at m
    exact m.2
  · exact absurd hm (by simp)

/-- Code accepted by the checker never goes wrong. -/
theorem check_sound {F : Fn} (h : check F = true) {st : St} (hr : Reach F st) : ∀ o ∈ steps F st, o ≠ .wrong := by
  obtain ⟨J, fin, h0⟩ := check_final h
  have : Inv F (fun pc => (joinPoints F)[pc]?.getD false) J st := by
    induction hr with
    | init => exact ⟨_, false, by simp, fin 0 [] h0⟩
    | step _ hs ih => exact (inv_step fin ih _ hs).2 _ rfl
  exact fun o ho => (inv_step fin this o ho).1

end Wah.Lowered
