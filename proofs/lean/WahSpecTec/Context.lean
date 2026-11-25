/-
Heap subtyping of WahProofs/Subtype.lean is sound for `Heaptype_sub` of the SpecTec model between type indices of a
module, in contexts with the types of the module as the spec has them (rolled, `specTypes`). This is how the spec
checks types of instructions, which refer to types by index.

Supertype chains go through the rolled types of the context: unrolling a type gives its supertype as a defined type
when it is in the same rec group, and as a type index otherwise, which `typeidx_l` resolves in the context. Chains
end with `Deftype_sub.refl`, as types are equivalent when they close to the same type.
-/
import WahSpecTec.Closed

set_option linter.unusedSectionVars false

namespace Wah.SpecTec
open Wah.Types Wah.Subtype

variable {A : Nat → storagetype}

/-- Type `x` of the module as the context has it. -/
def T (A : Nat → storagetype) (m : Types.M) (x : Nat) : deftype := ._DEF (rolledG A m (m.start x)) (x - m.start x)

theorem specTypes_get (m : Types.M) {x : Nat} (hx : x < m.n) : (specTypes A m m.n)[x]! = T A m x := by
  simp [specTypes, T, List.getElem!_eq_getElem?_getD, List.getElem?_map, List.getElem?_range hx]

/-- References of type `x` once unrolled: types of its group are defined types, others type indices. -/
def g3 (A : Nat → storagetype) (m : Types.M) (s : Nat) : Nat → typeuse := fun y =>
  if s ≤ y ∧ y < s + m.gsize s then ._DEF (rolledG A m s) (y - s) else ._IDX ⟨y⟩

theorem look_g1 (m : Types.M) (s : Nat) :
    look ((List.range (m.gsize s)).map fun i => (.REC i : typevar))
      ((List.range (m.gsize s)).map fun i => (._DEF (rolledG A m s) i : typeuse)) ∘ g1 s (m.gsize s) = g3 A m s := by
  funext y
  simp only [Function.comp, g1, g3, look]
  by_cases h : s ≤ y ∧ y < s + m.gsize s
  · rw [if_pos h, if_pos h]
    simp only [tvOf, lookup_rec, List.mem_range]
    rw [if_pos (by omega)]
  · rw [if_neg h, if_neg h]
    simp only [tvOf, lookup_idx]

theorem wf_rolledG (hA : AtomsOK A) {m : Types.M} (hw : WF m) (hn : m.n < 2 ^ 32)
    (hf : ∀ t < m.n, (m.d t).fields.length < 2 ^ 32) {x : Nat} (hx : x < m.n) :
    wf_rectype (rolledG A m (m.start x)) := by
  have hg := hw.group x hx
  have hsg := gsize_start m hw x hx
  refine wf_rtM hA m _ (by omega) (fun k hk => hf _ (by omega)) _ (fun k hk y hy => ?_)
  simp only [g1]; split
  · exact .typeuse_case_2 _
  · have := hw.refs _ (show m.start x + k < m.n by omega) y hy
    have hst := hw.same x hx (m.start x + k) (by omega) (by omega)
    rw [hst.1, hst.2] at this
    exact .typeuse_case_0 _ (wf_uN32 (by omega))

theorem g3_wf (hA : AtomsOK A) {m : Types.M} (hw : WF m) (hn : m.n < 2 ^ 32)
    (hf : ∀ t < m.n, (m.d t).fields.length < 2 ^ 32) {x : Nat} (hx : x < m.n) {y : Nat} (hy : y ∈ (m.d x).refs) :
    wf_typeuse (g3 A m (m.start x) y) := by
  simp only [g3]; split
  · exact .typeuse_case_1 _ _ (wf_rolledG hA hw hn hf hx)
  · have := hw.refs x hx y hy
    have := hw.group x hx
    exact .typeuse_case_0 _ (wf_uN32 (by omega))

/-- Unrolling type `x` of the context. -/
theorem unroll_T (hA : AtomsOK A) (hMR : MinusRecs) {m : Types.M} (hw : WF m) (hn : m.n < 2 ^ 32)
    (hf : ∀ t < m.n, (m.d t).fields.length < 2 ^ 32) {x : Nat} (hx : x < m.n) :
    fun_unrolldt (T A m x) (fS A (g3 A m (m.start x)) (m.d x)) := by
  have hg := hw.group x hx
  have hsg := gsize_start m hw x hx
  have hss := start_start m hw x hx
  have w1 := wf_rolledG hA hw hn hf hx
  have F := forall2_map (List.range (m.gsize (m.start x)))
    (fun k => fS A (g1 (m.start x) (m.gsize (m.start x))) (m.d (m.start x + k)))
    (fun k => fS A (g3 A m (m.start x)) (m.d (m.start x + k)))
    (fun v st => fun_subst_subtype st ((List.range (m.gsize (m.start x))).map fun i => (.REC i : typevar))
      ((List.range (m.gsize (m.start x))).map fun i => (._DEF (rolledG A m (m.start x)) i : typeuse)) v)
    (fun k _ => by
      have := subst_fS_ex hA (g1_var (m.start x) (m.gsize (m.start x))) (by simp) (m.d (m.start x + k))
        (tvs := (List.range (m.gsize (m.start x))).map fun i => (.REC i : typevar))
        (tus := (List.range (m.gsize (m.start x))).map fun i => (._DEF (rolledG A m (m.start x)) i : typeuse))
      rw [look_g1] at this; exact this)
  have w3 : wf_rectype (.REC (.mk_list ((List.range (m.gsize (m.start x))).map fun k =>
      fS A (g3 A m (m.start x)) (m.d (m.start x + k))))) := by
    refine wf_rtM hA m _ (by omega) (fun k hk => hf _ (by omega)) _ (fun k hk y hy => ?_)
    have hst := hw.same x hx (m.start x + k) (by omega) (by omega)
    have := g3_wf (A := A) hA hw hn hf (x := m.start x + k) (by omega) hy
    rw [hst.1] at this; exact this
  have hr : fun_unrollrt (rolledG A m (m.start x)) (.REC (.mk_list ((List.range (m.gsize (m.start x))).map fun k =>
      fS A (g3 A m (m.start x)) (m.d (m.start x + k))))) :=
    .fun_unrollrt_case_0 _ 0 _ _ _ _ F F rfl w3 w1 (by simp) (by simp) (by simp)
  have h := fun_unrolldt.fun_unrolldt_case_0 _ (x - m.start x) _ _ hr rfl w3
  have e : ((List.range (m.gsize (m.start x))).map fun k =>
      fS A (g3 A m (m.start x)) (m.d (m.start x + k)))[x - m.start x]! = fS A (g3 A m (m.start x)) (m.d x) := by
    rw [List.getElem!_eq_getElem?_getD, List.getElem?_map, List.getElem?_range (by omega)]
    simp only [Option.map_some, Option.getD_some]
    congr 2; omega
  rw [e] at h
  exact h

theorem wf_T (hA : AtomsOK A) {m : Types.M} (hw : WF m) (hn : m.n < 2 ^ 32)
    (hf : ∀ t < m.n, (m.d t).fields.length < 2 ^ 32) {x : Nat} (hx : x < m.n) : wf_deftype (T A m x) :=
  .deftype_case_0 _ _ (wf_rolledG hA hw hn hf hx)

/-- The context closes type `x` to its closed form. -/
theorem clos_T (hA : AtomsOK A) (hMR : MinusRecs) {m : Types.M} (hw : WF m) (hn : m.n < 2 ^ 32)
    (hf : ∀ t < m.n, (m.d t).fields.length < 2 ^ 32) {C : context} (hT : C.TYPES = specTypes A m m.n) {x : Nat}
    (hx : x < m.n) : fun_clos_deftype C (T A m x) (tD A (dtOf m x)) := by
  have st := close_step_ex hA hMR hw hn hf (t := x) (k := m.n) hx (by have := hw.group x hx; omega) (Nat.le_refl _)
  have cl := clos_ex hA hMR hw hn hf m.n (Nat.le_refl _)
  rw [← hT] at cl
  exact .fun_clos_deftype_case_0 _ _ _ _ _ _ st cl st rfl
    (.deftype_case_0 _ _ (wf_trG hA _ (okG_closeGroup hw hn hf x hx))) (wf_closedTypes hA hw hn hf (Nat.le_refl _))

/-! ### Subtyping in the context -/

section
variable (hA : AtomsOK A) (hMR : MinusRecs) {m : Types.M} (hw : SubWF m) (hn : m.n < 2 ^ 32)
  (hf : ∀ t < m.n, (m.d t).fields.length < 2 ^ 32) {C : context} (hC : wf_context C)
  (hT : C.TYPES = specTypes A m m.n)
include hA hMR hw hn hf hC hT

theorem wf_tD {x : Nat} (hx : x < m.n) : wf_deftype (tD A (dtOf m x)) :=
  .deftype_case_0 _ _ (wf_trG hA _ (okG_closeGroup hw.toWF hn hf x hx))

theorem fS_wf {x : Nat} (hx : x < m.n) : wf_subtype (fS A (g3 A m (m.start x)) (m.d x)) :=
  wf_fS hA (fun y hy => g3_wf hA hw.toWF hn hf hx hy) (hf x hx)

theorem types_len : (proj_uN_0 (⟨y⟩ : uN)) < C.TYPES.length ↔ y < m.n := by
  simp [hT, specTypes, proj_uN_0]

/-- Up the supertypes of `x` to a type equivalent to `y`. -/
theorem T_chain {y : Nat} (hy : y < m.n) : ∀ {x x' : Nat}, IChain m x x' → x < m.n → dtOf m x' = dtOf m y →
    Heaptype_sub C (heaptype_deftype (T A m x)) (heaptype_deftype (T A m y)) := by
  intro x x' c
  induction c with
  | refl x =>
    intro hx he
    exact .def C _ _ (.refl C _ _ _ _ (clos_T hA hMR hw.toWF hn hf hT hy) (clos_T hA hMR hw.toWF hn hf hT hx)
      (by rw [he]) hC (wf_T hA hw.toWF hn hf hx) (wf_T hA hw.toWF hn hf hy) (wf_tD hA hMR hw hn hf hC hT hx)
      (wf_tD hA hMR hw hn hf hC hT hy)) hC (wf_T hA hw.toWF hn hf hx) (wf_T hA hw.toWF hn hf hy)
  | @step x z x' hs _ ih =>
    intro hx he
    have hz : z < m.n := by have := hw.super_lt x hx z hs; omega
    have r := ih hz he
    have hsub : Heaptype_sub C (heaptype_typeuse (g3 A m (m.start x) z)) (heaptype_deftype (T A m y)) := by
      simp only [g3]
      split
      · rename_i hin
        have hst := (hw.same x hx z hin.1 (by rw [← gsize_start m hw.toWF x hx]; exact hin.2)).1
        have : heaptype_typeuse (._DEF (rolledG A m (m.start x)) (z - m.start x)) = heaptype_deftype (T A m z) := by
          simp [T, hst, heaptype_typeuse, heaptype_deftype]
        rw [this]; exact r
      · refine .typeidx_l C ⟨z⟩ _ ((types_len hA hMR hw hn hf hC hT).mpr hz) ?_ hC
          (.heaptype_case_14 _ _ (wf_rolledG hA hw.toWF hn hf hy)) (.heaptype_case_13 _ (wf_uN32 (by omega)))
        simp only [proj_uN_0, hT, specTypes_get m hz]
        exact r
    have hd : Deftype_sub C (T A m x) (T A m y) := by
      refine .super C _ _ _ ((m.d x).super.toList.map (g3 A m (m.start x))) _ 0 _ (unroll_T hA hMR hw.toWF hn hf hx)
        rfl (by simp [hs]) ?_ hC (wf_T hA hw.toWF hn hf hx) (wf_T hA hw.toWF hn hf hy)
        (fS_wf hA hMR hw hn hf hC hT hx) (fS_wf hA hMR hw hn hf hC hT hx)
      simp only [hs, Option.toList_some, List.map_cons, List.map_nil]
      exact hsub
    exact .def C _ _ hd hC (wf_T hA hw.toWF hn hf hx) (wf_T hA hw.toWF hn hf hy)

/-- From type `x` to the abstract heap types of its kind. -/
theorem T_abs {x : Nat} (hx : x < m.n) {b : Abs} (hb : defAbs (kindOf (dtOf m x)) b = true) :
    Heaptype_sub C (heaptype_deftype (T A m x)) (trAbs b) := by
  rw [kindOf_dtOf m hw.toWF x hx] at hb
  have hwS := fS_wf hA hMR hw hn hf hC hT hx
  have he : Expand (T A m x) (trComp (m.d x).kind (m.d x).counts ((m.d x).fields.map (fF A (g3 A m (m.start x))))) :=
    .mk_Expand _ _ _ _ _ (unroll_T hA hMR hw.toWF hn hf hx) rfl (wf_T hA hw.toWF hn hf hx) hwS hwS
  have hw' : wf_comptype (trComp (m.d x).kind (m.d x).counts ((m.d x).fields.map (fF A (g3 A m (m.start x))))) := by
    cases hwS; assumption
  have hd := wf_T hA hw.toWF hn hf hx
  have hd' : wf_heaptype (heaptype_deftype (T A m x)) := .heaptype_case_14 _ _ (wf_rolledG hA hw.toWF hn hf hx)
  have toStruct : (m.d x).kind = kStruct → Heaptype_sub C (heaptype_deftype (T A m x)) .STRUCT := fun e => by
    simp only [trComp, e, ite_true] at he hw'
    exact .struct C _ _ he hC hd .heaptype_case_3 hw'
  have toArray : (m.d x).kind = kArray → Heaptype_sub C (heaptype_deftype (T A m x)) .ARRAY := fun e => by
    simp only [trComp, e, kArray, kStruct] at he hw'
    exact .array C _ _ he hC hd .heaptype_case_4 hw'
  have toFunc : (m.d x).kind = kFunc → Heaptype_sub C (heaptype_deftype (T A m x)) .FUNC := fun e => by
    simp only [trComp, e, kArray, kStruct, kFunc] at he hw'
    exact .func C _ _ _ he hC hd .heaptype_case_6 hw'
  have st : Heaptype_sub C .STRUCT .EQ := .struct_eq C hC .heaptype_case_3 .heaptype_case_1
  have ar : Heaptype_sub C .ARRAY .EQ := .array_eq C hC .heaptype_case_4 .heaptype_case_1
  have toEq : ((m.d x).kind = kStruct ∨ (m.d x).kind = kArray) → Heaptype_sub C (heaptype_deftype (T A m x)) .EQ :=
    fun e => by
      rcases e with e | e
      · exact sub_trans hC (ok_trAbs hC .struct) hd' .heaptype_case_3 .heaptype_case_1 (toStruct e) st
      · exact sub_trans hC (ok_trAbs hC .array) hd' .heaptype_case_4 .heaptype_case_1 (toArray e) ar
  cases b <;> simp [defAbs] at hb
  · exact sub_eq_any hC hd' (toEq hb)
  · exact toEq hb
  · exact toStruct hb
  · exact toArray hb
  · exact toFunc hb

/-- From the abstract bottom types of its kind to type `y`. -/
theorem abs_T {y : Nat} (hy : y < m.n) {a : Abs} (ha : absDef (kindOf (dtOf m y)) a = true) :
    Heaptype_sub C (trAbs a) (heaptype_deftype (T A m y)) := by
  have hd : wf_heaptype (heaptype_deftype (T A m y)) := .heaptype_case_14 _ _ (wf_rolledG hA hw.toWF hn hf hy)
  have ne : heaptype_deftype (T A m y) ≠ .BOT := by simp [T, heaptype_deftype]
  cases a <;> simp [absDef] at ha
  · have hb : defAbs (kindOf (dtOf m y)) .any = true := by simp [defAbs, ha]
    exact .none C _ (T_abs hA hMR hw hn hf hC hT hy hb) ne hC hd .heaptype_case_5 .heaptype_case_0 .heaptype_case_12
  · have hb : defAbs (kindOf (dtOf m y)) .func = true := by simp [defAbs, ha]
    exact .nofunc C _ (T_abs hA hMR hw hn hf hC hT hy hb) ne hC hd .heaptype_case_7 .heaptype_case_6
      .heaptype_case_12

theorem idx_l {x : Nat} (hx : x < m.n) {h : heaptype} (wh : wf_heaptype h)
    (s : Heaptype_sub C (heaptype_deftype (T A m x)) h) : Heaptype_sub C (._IDX ⟨x⟩) h :=
  .typeidx_l C ⟨x⟩ _ ((types_len hA hMR hw hn hf hC hT).mpr hx)
    (by simp only [proj_uN_0, hT, specTypes_get m hx]; exact s)
    hC wh (.heaptype_case_13 _ (wf_uN32 (by omega)))

theorem idx_r {y : Nat} (hy : y < m.n) {h : heaptype} (wh : wf_heaptype h)
    (s : Heaptype_sub C h (heaptype_deftype (T A m y))) : Heaptype_sub C h (._IDX ⟨y⟩) :=
  .typeidx_r C _ ⟨y⟩ ((types_len hA hMR hw hn hf hC hT).mpr hy)
    (by simp only [proj_uN_0, hT, specTypes_get m hy]; exact s)
    hC wh (.heaptype_case_13 _ (wf_uN32 (by omega)))

end

/-- The context with just the types of a module, which satisfies the hypotheses of the theorems. -/
def moduleCtx (A : Nat → storagetype) (m : Types.M) : context :=
  { TYPES := specTypes A m m.n, TAGS := [], GLOBALS := [], MEMS := [], TABLES := [], FUNCS := [], DATAS := [],
    ELEMS := [], LOCALS := [], LABELS := [], RETURN := none, REFS := [], RECS := [] }

theorem wf_moduleCtx (hA : AtomsOK A) {m : Types.M} (hw : WF m) (hn : m.n < 2 ^ 32)
    (hf : ∀ t < m.n, (m.d t).fields.length < 2 ^ 32) : wf_context (moduleCtx A m) := by
  unfold moduleCtx
  refine .context_case_ _ _ _ _ _ _ _ _ _ _ _ _ _ ?_ (by simp [Forall]) (by simp [Forall]) (by simp [Forall])
    (by simp [Forall]) (by simp [Forall]) (by simp [Forall]) (by simp [Forall]) (by simp [Forall])
    (by simp [Forall]) (by simp [Forall]) (by simp [Forall]) (by simp [Forall]) (by simp [Forall])
  intro d hd
  obtain ⟨t, ht, rfl⟩ := List.mem_map.mp hd
  rw [List.mem_range] at ht
  exact wf_T hA hw hn hf ht

/-! ### Heap types of a module -/

/-- Heap types as instructions have them: abstract ones and type indices. -/
inductive MH where
  | abs (h : Abs)
  | idx (x : Nat)

def denMH (m : Types.M) : MH → CH
  | .abs h => .abs h
  | .idx x => .defd (dtOf m x)

def specMH : MH → heaptype
  | .abs h => trAbs h
  | .idx x => ._IDX ⟨x⟩

def OkMH (m : Types.M) : MH → Prop
  | .abs _ => True
  | .idx x => x < m.n

/-- Heap subtyping of WahProofs/Subtype.lean between types of a module implies `Heaptype_sub` of the spec between
their type indices, in contexts with the types of the module. -/
theorem idx_hsub_sound (hA : AtomsOK A) (hMR : MinusRecs) {m : Types.M} (hw : SubWF m) (hn : m.n < 2 ^ 32)
    (hf : ∀ t < m.n, (m.d t).fields.length < 2 ^ 32) {C : context} (hC : wf_context C)
    (hT : C.TYPES = specTypes A m m.n) {a b : MH} (ha : OkMH m a) (hb : OkMH m b) (h : HSub (denMH m a) (denMH m b)) :
    Heaptype_sub C (specMH a) (specMH b) := by
  have hv : ValidH (denMH m a) := by
    cases a with
    | abs _ => trivial
    | idx x => exact kindsOK_dtOf m hw x ha
  have al := algo_of_hsub h hv
  have wI : ∀ {x : Nat}, x < m.n → wf_heaptype (._IDX ⟨x⟩) := fun hx => .heaptype_case_13 _ (wf_uN32 (by omega))
  cases a with
  | abs x => cases b with
    | abs y => exact abs_sound hC x y al
    | idx y => exact idx_r hA hMR hw hn hf hC hT hb (wf_trAbs x) (abs_T hA hMR hw hn hf hC hT hb al)
  | idx x => cases b with
    | abs y => exact idx_l hA hMR hw hn hf hC hT ha (wf_trAbs y) (T_abs hA hMR hw hn hf hC hT ha al)
    | idx y =>
      obtain ⟨x', c, he⟩ := (chain_dtOf m hw x ha (dtOf m y)).mp al
      exact idx_l hA hMR hw hn hf hC hT ha (wI hb) (idx_r hA hMR hw hn hf hC hT hb
        (.heaptype_case_14 _ _ (wf_rolledG hA hw.toWF hn hf ha)) (T_chain hA hMR hw hn hf hC hT hb c ha he))

end Wah.SpecTec
