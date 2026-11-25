/-
Completeness of heap subtyping for `Heaptype_sub` of the SpecTec model between type indices of a module, in
contexts with the types of the module (`specTypes`, as in Context.lean) and no recursive types.

Every heap type of such contexts that subtyping can meet closes to a closed type (`Closes`): type indices to the
closed forms of `dtOf`, and defined types by substituting the closed types of the context for type indices. Subtyping
in the context implies the algorithmic subtyping of SpecTec/SubtypeComplete.lean between the closed types, which are
good (keep kinds along supertypes): closing keeps kinds and closes supertypes, and supertypes of valid types are
valid or type indices, whose closed forms are good by WahProofs.
-/
import WahSpecTec.Context

set_option linter.unusedSectionVars false

namespace Wah.SpecTec
open Wah.Types Wah.Subtype

variable {A : Nat → storagetype}

/-! ### Substitutions are unique -/

theorem minusRecs_len : ∀ (tvs : List typevar) (tus : List typeuse), tvs.length = tus.length →
    (Option.get! (minusRecs tvs tus)).1.length = (Option.get! (minusRecs tvs tus)).2.length
  | [], [], _ => rfl
  | .REC _ :: tvs, _ :: tus, h => minusRecs_len tvs tus (by simpa using h)
  | ._IDX x :: tvs, tu :: tus, h => by
    have ih := minusRecs_len tvs tus (by simpa using h)
    simp only [minusRecs]
    cases e : minusRecs tvs tus with
    | none => rfl
    | some p => rw [e] at ih; simpa using ih
  | [], _ :: _, h | _ :: _, [], h => by simp at h

theorem list_det {α β : Type} {R : α → β → Prop} {l1 l2 : List α} {l : List β} (h1 : Forall₂ R l1 l)
    (h2 : Forall₂ R l2 l) (e1 : l1.length = l.length) (e2 : l2.length = l.length)
    (hd : ∀ a b c, (a, b) ∈ l1.zip l → (c, b) ∈ l2.zip l → a = c) : l1 = l2 := by
  apply List.ext_getElem (by omega)
  intro i a b
  apply hd _ (l[i]'(by omega)) _
  · have : (l1.zip l)[i]'(by simp; omega) = (l1[i], l[i]'(by omega)) := by simp
    rw [← this]; exact List.getElem_mem _
  · have : (l2.zip l)[i]'(by simp; omega) = (l2[i], l[i]'(by omega)) := by simp
    rw [← this]; exact List.getElem_mem _

variable (hMR : MinusRecs)
include hMR

mutual
theorem det_typeuse : ∀ {x tvs tus y1 y2}, fun_subst_typeuse x tvs tus y1 → fun_subst_typeuse x tvs tus y2 →
    tvs.length = tus.length → y1 = y2
  | _, _, _, _, _, .fun_subst_typeuse_case_0 _ _ _ _ h1, h2, hl => by
    cases h2 with | fun_subst_typeuse_case_0 _ _ _ _ h2 => rw [subst_typevar_det h1 hl, subst_typevar_det h2 hl]
  | _, _, _, _, _, .fun_subst_typeuse_case_1 _ _ _ _ h1, h2, hl => by
    cases h2 with | fun_subst_typeuse_case_1 _ _ _ _ h2 => rw [subst_typevar_det h1 hl, subst_typevar_det h2 hl]
  | _, _, _, _, _, .fun_subst_typeuse_case_2 _ _ _ _ _ h1, h2, hl => by
    cases h2 with | fun_subst_typeuse_case_2 _ _ _ _ _ h2 => rw [det_deftype h1 h2 hl]
termination_by structural _ _ _ _ _ h1 => h1

theorem det_heaptype : ∀ {x tvs tus y1 y2}, fun_subst_heaptype x tvs tus y1 → fun_subst_heaptype x tvs tus y2 →
    tvs.length = tus.length → y1 = y2
  | _, _, _, _, _, .fun_subst_heaptype_case_0 n tvs tus _ h1, h2, hl => by
    cases h2 with
    | fun_subst_heaptype_case_0 _ _ _ _ h2 => rw [subst_typevar_det h1 hl, subst_typevar_det h2 hl]
    | fun_subst_heaptype_case_3 _ _ _ hb => exact absurd (.fun_subst_heaptype_case_0 n tvs tus none) hb
  | _, _, _, _, _, .fun_subst_heaptype_case_1 x tvs tus _ h1, h2, hl => by
    cases h2 with
    | fun_subst_heaptype_case_1 _ _ _ _ h2 => rw [subst_typevar_det h1 hl, subst_typevar_det h2 hl]
    | fun_subst_heaptype_case_3 _ _ _ hb => exact absurd (.fun_subst_heaptype_case_1 x tvs tus none) hb
  | _, _, _, _, _, .fun_subst_heaptype_case_2 rt i tvs tus _ h1, h2, hl => by
    cases h2 with
    | fun_subst_heaptype_case_2 _ _ _ _ _ h2 => rw [det_deftype h1 h2 hl]
    | fun_subst_heaptype_case_3 _ _ _ hb =>
      exact absurd (.fun_subst_heaptype_case_2 rt i tvs tus (._DEF rt i)) hb
  | _, _, _, _, _, .fun_subst_heaptype_case_3 _ tvs tus hb, h2, _ => by
    cases h2 with
    | fun_subst_heaptype_case_3 => rfl
    | fun_subst_heaptype_case_0 n => exact absurd (.fun_subst_heaptype_case_0 n tvs tus none) hb
    | fun_subst_heaptype_case_1 x => exact absurd (.fun_subst_heaptype_case_1 x tvs tus none) hb
    | fun_subst_heaptype_case_2 rt i => exact absurd (.fun_subst_heaptype_case_2 rt i tvs tus (._DEF rt i)) hb
termination_by structural _ _ _ _ _ h1 => h1

theorem det_reftype : ∀ {x tvs tus y1 y2}, fun_subst_reftype x tvs tus y1 → fun_subst_reftype x tvs tus y2 →
    tvs.length = tus.length → y1 = y2
  | _, _, _, _, _, .fun_subst_reftype_case_0 _ _ _ _ _ h1, h2, hl => by
    cases h2 with | fun_subst_reftype_case_0 _ _ _ _ _ h2 => rw [det_heaptype h1 h2 hl]
termination_by structural _ _ _ _ _ h1 => h1

theorem det_valtype : ∀ {x tvs tus y1 y2}, fun_subst_valtype x tvs tus y1 → fun_subst_valtype x tvs tus y2 →
    tvs.length = tus.length → y1 = y2
  | _, _, _, _, _, .fun_subst_valtype_case_5 _ _ _ _ _ h1, h2, hl => by
    cases h2 with | fun_subst_valtype_case_5 _ _ _ _ _ h2 => rw [det_reftype h1 h2 hl]
  | _, _, _, _, _, .fun_subst_valtype_case_0 .., h2, _ | _, _, _, _, _, .fun_subst_valtype_case_1 .., h2, _
  | _, _, _, _, _, .fun_subst_valtype_case_2 .., h2, _ | _, _, _, _, _, .fun_subst_valtype_case_3 .., h2, _
  | _, _, _, _, _, .fun_subst_valtype_case_4 .., h2, _ | _, _, _, _, _, .fun_subst_valtype_case_6 .., h2, _ => by
    cases h2; rfl
termination_by structural _ _ _ _ _ h1 => h1

theorem det_storagetype : ∀ {x tvs tus y1 y2}, fun_subst_storagetype x tvs tus y1 →
    fun_subst_storagetype x tvs tus y2 → tvs.length = tus.length → y1 = y2
  | _, _, _, _, _, .fun_subst_storagetype_case_0 _ _ _ h1, h2, hl => by
    cases h2 with | fun_subst_storagetype_case_0 _ _ _ h2 => rw [det_valtype h1 h2 hl]
  | _, _, _, _, _, .fun_subst_storagetype_case_1 _ _ _ _ _ h1, h2, hl => by
    cases h2 with | fun_subst_storagetype_case_1 _ _ _ _ _ h2 => rw [det_valtype h1 h2 hl]
  | _, _, _, _, _, .fun_subst_storagetype_case_2 _ _ _ h1, h2, hl => by
    cases h2 with | fun_subst_storagetype_case_2 _ _ _ h2 => rw [det_valtype h1 h2 hl]
  | _, _, _, _, _, .fun_subst_storagetype_case_3 _ _ _ h1, h2, hl => by
    cases h2 with | fun_subst_storagetype_case_3 _ _ _ h2 => rw [det_valtype h1 h2 hl]
  | _, _, _, _, _, .fun_subst_storagetype_case_4 _ _ _ h1, h2, hl => by
    cases h2 with | fun_subst_storagetype_case_4 _ _ _ h2 => rw [det_valtype h1 h2 hl]
  | _, _, _, _, _, .fun_subst_storagetype_case_5 _ _ _ h1, h2, hl => by
    cases h2 with | fun_subst_storagetype_case_5 _ _ _ h2 => rw [det_valtype h1 h2 hl]
  | _, _, _, _, _, .fun_subst_storagetype_case_6 _ _ _ h1, h2, hl => by
    cases h2 with | fun_subst_storagetype_case_6 _ _ _ h2 => rw [det_valtype h1 h2 hl]
  | _, _, _, _, _, .fun_subst_storagetype_case_7 .., h2, _
  | _, _, _, _, _, .fun_subst_storagetype_case_8 .., h2, _ => by
    cases h2; rfl
termination_by structural _ _ _ _ _ h1 => h1

theorem det_fieldtype : ∀ {x tvs tus y1 y2}, fun_subst_fieldtype x tvs tus y1 → fun_subst_fieldtype x tvs tus y2 →
    tvs.length = tus.length → y1 = y2
  | _, _, _, _, _, .fun_subst_fieldtype_case_0 _ _ _ _ _ h1, h2, hl => by
    cases h2 with | fun_subst_fieldtype_case_0 _ _ _ _ _ h2 => rw [det_storagetype h1 h2 hl]
termination_by structural _ _ _ _ _ h1 => h1

theorem det_comptype : ∀ {x tvs tus y1 y2}, fun_subst_comptype x tvs tus y1 → fun_subst_comptype x tvs tus y2 →
    tvs.length = tus.length → y1 = y2
  | _, _, _, _, _, .fun_subst_comptype_case_0 _ _ _ _ F1 l1, h2, hl => by
    cases h2 with
    | fun_subst_comptype_case_0 _ _ _ _ F2 l2 =>
      rw [list_det F1 F2 l1 l2 (fun a b c m1 m2 => det_fieldtype (F1 a b m1) (F2 c b m2) hl)]
  | _, _, _, _, _, .fun_subst_comptype_case_1 _ _ _ _ h1, h2, hl => by
    cases h2 with | fun_subst_comptype_case_1 _ _ _ _ h2 => rw [det_fieldtype h1 h2 hl]
  | _, _, _, _, _, .fun_subst_comptype_case_2 _ _ _ _ _ _ F1 F0 l1 l0, h2, hl => by
    cases h2 with
    | fun_subst_comptype_case_2 _ _ _ _ _ _ G1 G0 m1 m0 =>
      rw [list_det F1 G1 l1 m1 (fun a b c n1 n2 => det_valtype (F1 a b n1) (G1 c b n2) hl),
        list_det F0 G0 l0 m0 (fun a b c n1 n2 => det_valtype (F0 a b n1) (G0 c b n2) hl)]
termination_by structural _ _ _ _ _ h1 => h1

theorem det_subtype : ∀ {x tvs tus y1 y2}, fun_subst_subtype x tvs tus y1 → fun_subst_subtype x tvs tus y2 →
    tvs.length = tus.length → y1 = y2
  | _, _, _, _, _, .fun_subst_subtype_case_0 _ _ _ _ _ _ _ hc1 F1 l1, h2, hl => by
    cases h2 with
    | fun_subst_subtype_case_0 _ _ _ _ _ _ _ hc2 F2 l2 =>
      rw [det_comptype hc1 hc2 hl, list_det F1 F2 l1 l2 (fun a b c n1 n2 => det_typeuse (F1 a b n1) (F2 c b n2) hl)]
termination_by structural _ _ _ _ _ h1 => h1

theorem det_rectype : ∀ {x tvs tus y1 y2}, fun_subst_rectype x tvs tus y1 → fun_subst_rectype x tvs tus y2 →
    tvs.length = tus.length → y1 = y2
  | _, tvs, tus, _, _, .fun_subst_rectype_case_0 _ _ _ tv1 tu1 _ _ _ F1 he1 _ _ _ _ l1, h2, hl => by
    cases h2 with
    | fun_subst_rectype_case_0 _ _ _ tv2 tu2 _ _ _ F2 he2 _ _ _ _ l2 =>
      have e := he1.trans he2.symm
      obtain ⟨e1, e2⟩ := Prod.mk.inj e
      have hl' : tv1.length = tu1.length := by
        have := minusRecs_len tvs tus hl
        rw [← hMR] at this
        rw [← he1] at this; exact this
      subst e1 e2
      rw [list_det F1 F2 l1 l2 (fun a b c n1 n2 => det_subtype (F1 a b n1) (F2 c b n2) hl')]
termination_by structural _ _ _ _ _ h1 => h1

theorem det_deftype : ∀ {x tvs tus y1 y2}, fun_subst_deftype x tvs tus y1 → fun_subst_deftype x tvs tus y2 →
    tvs.length = tus.length → y1 = y2
  | _, _, _, _, _, .fun_subst_deftype_case_0 _ _ _ _ _ _ _ h1 _, h2, hl => by
    cases h2 with | fun_subst_deftype_case_0 _ _ _ _ _ _ _ h2 _ => rw [det_rectype h1 h2 hl]
termination_by structural _ _ _ _ _ h1 => h1

end

/-! ### Substitutions of type indices exist -/

omit hMR in
theorem lookupTV_mem (tv : typevar) : ∀ (tvs : List typevar) (tus : List typeuse),
    lookupTV tv tvs tus ∈ tus ∨ lookupTV tv tvs tus = typeuse_typevar tv ∨ lookupTV tv tvs tus = default
  | [], [] => .inr (.inl rfl)
  | a :: as, b :: bs => by
    simp only [lookupTV]
    split
    · exact .inl (by simp)
    · rcases lookupTV_mem tv as bs with h | h | h
      · exact .inl (by simp [h])
      · exact .inr (.inl h)
      · exact .inr (.inr h)
  | [], _ :: _ | _ :: _, [] => .inr (.inr rfl)

section
variable (k : Nat) (tus : List typeuse) (hk : tus.length = k) (hk32 : k ≤ 2 ^ 32)
  (hwu : ∀ u ∈ tus, wf_typeuse u)
include hk hk32 hwu

/-- Type indices below `k`. -/
abbrev tvsK (k : Nat) : List typevar := (List.range k).map fun i => (._IDX ⟨i⟩ : typevar)

omit hMR in
theorem look_wf {tv : typevar} (hv : wf_typeuse (typeuse_typevar tv)) :
    wf_typeuse (lookupTV tv (tvsK k) tus) := by
  rcases lookupTV_mem tv (tvsK k) tus with h | h | h
  · exact hwu _ h
  · rw [h]; exact hv
  · rw [h]; exact .typeuse_case_0 _ (wf_uN32 (by decide))

omit hMR in
theorem hlenK : (tvsK k).length = tus.length := by simp [hk]

mutual
theorem ex_U : ∀ (u : typeuse), wf_typeuse u → ∃ u', fun_subst_typeuse u (tvsK k) tus u' ∧ wf_typeuse u'
  | .REC i, _ => ⟨_, .fun_subst_typeuse_case_0 i _ _ _ (subst_typevar_ex _ (hlenK k tus hk hk32 hwu)),
      look_wf k tus hk hk32 hwu (.typeuse_case_2 i)⟩
  | ._IDX x, hw => ⟨_, .fun_subst_typeuse_case_1 x _ _ _ (subst_typevar_ex _ (hlenK k tus hk hk32 hwu)),
      look_wf k tus hk hk32 hwu hw⟩
  | ._DEF rt i, hw => by
    cases hw with
    | typeuse_case_1 _ _ hr =>
      obtain ⟨rt', h, w⟩ := ex_R rt hr
      exact ⟨._DEF rt' i, .fun_subst_typeuse_case_2 _ _ _ _ (._DEF rt' i)
        (.fun_subst_deftype_case_0 _ _ _ _ _ _ h h (.deftype_case_0 _ _ w)), .typeuse_case_1 _ _ w⟩

theorem ex_H : ∀ (h : heaptype), wf_heaptype h → ∃ h', fun_subst_heaptype h (tvsK k) tus h' ∧ wf_heaptype h'
  | .REC i, _ => ⟨_, .fun_subst_heaptype_case_0 i _ _ _ (subst_typevar_ex _ (hlenK k tus hk hk32 hwu)),
      wf_heap_of_use (look_wf k tus hk hk32 hwu (.typeuse_case_2 i))⟩
  | ._IDX x, hw => by
    cases hw with
    | heaptype_case_13 _ hx =>
      exact ⟨_, .fun_subst_heaptype_case_1 x _ _ _ (subst_typevar_ex _ (hlenK k tus hk hk32 hwu)),
        wf_heap_of_use (look_wf k tus hk hk32 hwu (.typeuse_case_0 _ hx))⟩
  | ._DEF rt i, hw => by
    cases hw with
    | heaptype_case_14 _ _ hr =>
      obtain ⟨rt', h, w⟩ := ex_R rt hr
      exact ⟨._DEF rt' i, .fun_subst_heaptype_case_2 _ _ _ _ (._DEF rt' i)
        (.fun_subst_deftype_case_0 _ _ _ _ _ _ h h (.deftype_case_0 _ _ w)), .heaptype_case_14 _ _ w⟩
  | .ANY, hw | .EQ, hw | .I31, hw | .STRUCT, hw | .ARRAY, hw | .NONE, hw | .FUNC, hw | .NOFUNC, hw | .EXN, hw
  | .NOEXN, hw | .EXTERN, hw | .NOEXTERN, hw | .BOT, hw =>
    ⟨_, .fun_subst_heaptype_case_3 _ _ _ (fun hb => nomatch hb), hw⟩

theorem ex_V : ∀ (v : valtype), wf_valtype v → ∃ v', fun_subst_valtype v (tvsK k) tus v' ∧ wf_valtype v'
  | .REF n h, hw => by
    cases hw with
    | valtype_case_5 _ _ hh =>
      obtain ⟨h', s, w⟩ := ex_H h hh
      exact ⟨.REF n h', .fun_subst_valtype_case_5 _ _ _ _ _ (.fun_subst_reftype_case_0 _ _ _ _ _ s),
        .valtype_case_5 _ _ w⟩
  | .I32, _ => ⟨_, .fun_subst_valtype_case_0 _ _, .valtype_case_0⟩
  | .I64, _ => ⟨_, .fun_subst_valtype_case_1 _ _, .valtype_case_1⟩
  | .F32, _ => ⟨_, .fun_subst_valtype_case_2 _ _, .valtype_case_2⟩
  | .F64, _ => ⟨_, .fun_subst_valtype_case_3 _ _, .valtype_case_3⟩
  | .V128, _ => ⟨_, .fun_subst_valtype_case_4 _ _, .valtype_case_4⟩
  | .BOT, _ => ⟨_, .fun_subst_valtype_case_6 _ _, .valtype_case_6⟩

theorem ex_Z : ∀ (z : storagetype), wf_storagetype z → ∃ z', fun_subst_storagetype z (tvsK k) tus z' ∧
    wf_storagetype z'
  | .REF n h, hw => by
    cases hw with
    | storagetype_case_5 _ _ hh =>
      obtain ⟨v', s, w⟩ := ex_V (.REF n h) (.valtype_case_5 _ _ hh)
      refine ⟨storagetype_valtype v', .fun_subst_storagetype_case_1 _ _ _ _ _ s, ?_⟩
      cases w <;> constructor <;> assumption
  | .I32, _ => ⟨_, .fun_subst_storagetype_case_6 _ _ _ (.fun_subst_valtype_case_0 _ _), .storagetype_case_0⟩
  | .I64, _ => ⟨_, .fun_subst_storagetype_case_5 _ _ _ (.fun_subst_valtype_case_1 _ _), .storagetype_case_1⟩
  | .F32, _ => ⟨_, .fun_subst_storagetype_case_4 _ _ _ (.fun_subst_valtype_case_2 _ _), .storagetype_case_2⟩
  | .F64, _ => ⟨_, .fun_subst_storagetype_case_3 _ _ _ (.fun_subst_valtype_case_3 _ _), .storagetype_case_3⟩
  | .V128, _ => ⟨_, .fun_subst_storagetype_case_2 _ _ _ (.fun_subst_valtype_case_4 _ _), .storagetype_case_4⟩
  | .BOT, _ => ⟨_, .fun_subst_storagetype_case_0 _ _ _ (.fun_subst_valtype_case_6 _ _), .storagetype_case_6⟩
  | .I8, _ => ⟨_, .fun_subst_storagetype_case_7 _ _, .storagetype_case_7⟩
  | .I16, _ => ⟨_, .fun_subst_storagetype_case_8 _ _, .storagetype_case_8⟩

theorem ex_F : ∀ (f : fieldtype), wf_fieldtype f → ∃ f', fun_subst_fieldtype f (tvsK k) tus f' ∧ wf_fieldtype f'
  | .mk_fieldtype mu z, hw => by
    cases hw with
    | fieldtype_case_0 _ _ hz =>
      obtain ⟨z', s, w⟩ := ex_Z z hz
      exact ⟨_, .fun_subst_fieldtype_case_0 _ _ _ _ _ s, .fieldtype_case_0 _ _ w⟩

theorem ex_Fs : ∀ (fs : List fieldtype), (∀ f ∈ fs, wf_fieldtype f) → ∃ fs',
    Forall₂ (fun v t => fun_subst_fieldtype t (tvsK k) tus v) fs' fs ∧ fs'.length = fs.length ∧
      ∀ f ∈ fs', wf_fieldtype f
  | [], _ => ⟨[], fun _ _ h => by simp at h, rfl, fun _ h => by simp at h⟩
  | f :: fs, hw => by
    obtain ⟨f', s, w⟩ := ex_F f (hw f (by simp))
    obtain ⟨fs', F, l, W⟩ := ex_Fs fs (fun g hg => hw g (by simp [hg]))
    refine ⟨f' :: fs', fun x y h => ?_, by simp [l], fun g hg => ?_⟩
    · simp only [List.zip_cons_cons, List.mem_cons, Prod.mk.injEq] at h
      rcases h with ⟨rfl, rfl⟩ | h
      · exact s
      · exact F x y h
    · simp only [List.mem_cons] at hg
      rcases hg with rfl | hg
      · exact w
      · exact W g hg

theorem ex_Vs : ∀ (vs : List valtype), (∀ v ∈ vs, wf_valtype v) → ∃ vs',
    Forall₂ (fun v t => fun_subst_valtype t (tvsK k) tus v) vs' vs ∧ vs'.length = vs.length ∧
      ∀ v ∈ vs', wf_valtype v
  | [], _ => ⟨[], fun _ _ h => by simp at h, rfl, fun _ h => by simp at h⟩
  | v :: vs, hw => by
    obtain ⟨v', s, w⟩ := ex_V v (hw v (by simp))
    obtain ⟨vs', F, l, W⟩ := ex_Vs vs (fun g hg => hw g (by simp [hg]))
    refine ⟨v' :: vs', fun x y h => ?_, by simp [l], fun g hg => ?_⟩
    · simp only [List.zip_cons_cons, List.mem_cons, Prod.mk.injEq] at h
      rcases h with ⟨rfl, rfl⟩ | h
      · exact s
      · exact F x y h
    · simp only [List.mem_cons] at hg
      rcases hg with rfl | hg
      · exact w
      · exact W g hg

theorem ex_C : ∀ (c : comptype), wf_comptype c → ∃ c', fun_subst_comptype c (tvsK k) tus c' ∧ wf_comptype c'
  | .STRUCT (.mk_list fs), hw => by
    cases hw with
    | comptype_case_0 _ hl hf =>
      obtain ⟨fs', F, l, W⟩ := ex_Fs fs (by simpa [proj_list_0, Forall] using hf)
      exact ⟨_, .fun_subst_comptype_case_0 _ _ _ _ F l,
        .comptype_case_0 _ (by simpa [proj_list_0, l] using hl) (by simpa [proj_list_0, Forall] using W)⟩
  | .ARRAY f, hw => by
    cases hw with
    | comptype_case_1 _ hf =>
      obtain ⟨f', s, w⟩ := ex_F f hf
      exact ⟨_, .fun_subst_comptype_case_1 _ _ _ _ s, .comptype_case_1 _ w⟩
  | .FUNC (.mk_list a) (.mk_list b), hw => by
    cases hw with
    | comptype_case_2 _ _ la wa lb wb =>
      obtain ⟨a', Fa, l1, Wa⟩ := ex_Vs a (by simpa [proj_list_0, Forall] using wa)
      obtain ⟨b', Fb, l2, Wb⟩ := ex_Vs b (by simpa [proj_list_0, Forall] using wb)
      exact ⟨_, .fun_subst_comptype_case_2 _ _ _ _ _ _ Fb Fa l2 l1,
        .comptype_case_2 _ _ (by simpa [proj_list_0, l1] using la) (by simpa [proj_list_0, Forall] using Wa)
          (by simpa [proj_list_0, l2] using lb) (by simpa [proj_list_0, Forall] using Wb)⟩

theorem ex_Us : ∀ (us : List typeuse), (∀ u ∈ us, wf_typeuse u) → ∃ us',
    Forall₂ (fun v t => fun_subst_typeuse t (tvsK k) tus v) us' us ∧ us'.length = us.length ∧
      ∀ u ∈ us', wf_typeuse u
  | [], _ => ⟨[], fun _ _ h => by simp at h, rfl, fun _ h => by simp at h⟩
  | u :: us, hw => by
    obtain ⟨u', s, w⟩ := ex_U u (hw u (by simp))
    obtain ⟨us', F, l, W⟩ := ex_Us us (fun g hg => hw g (by simp [hg]))
    refine ⟨u' :: us', fun x y h => ?_, by simp [l], fun g hg => ?_⟩
    · simp only [List.zip_cons_cons, List.mem_cons, Prod.mk.injEq] at h
      rcases h with ⟨rfl, rfl⟩ | h
      · exact s
      · exact F x y h
    · simp only [List.mem_cons] at hg
      rcases hg with rfl | hg
      · exact w
      · exact W g hg

theorem ex_S : ∀ (st : subtype), wf_subtype st → ∃ st', fun_subst_subtype st (tvsK k) tus st' ∧ wf_subtype st'
  | .SUB f us c, hw => by
    cases hw with
    | subtype_case_0 _ _ _ hu hc =>
      obtain ⟨us', F, l, W⟩ := ex_Us us (by simpa [Forall] using hu)
      obtain ⟨c', s, w⟩ := ex_C c hc
      exact ⟨_, .fun_subst_subtype_case_0 _ _ _ _ _ _ _ s F l, .subtype_case_0 _ _ _ (by simpa [Forall] using W) w⟩

theorem ex_Ss : ∀ (ss : List subtype), (∀ st ∈ ss, wf_subtype st) → ∃ ss',
    Forall₂ (fun v t => fun_subst_subtype t (tvsK k) tus v) ss' ss ∧ ss'.length = ss.length ∧
      ∀ st ∈ ss', wf_subtype st
  | [], _ => ⟨[], fun _ _ h => by simp at h, rfl, fun _ h => by simp at h⟩
  | st :: ss, hw => by
    obtain ⟨st', s, w⟩ := ex_S st (hw st (by simp))
    obtain ⟨ss', F, l, W⟩ := ex_Ss ss (fun g hg => hw g (by simp [hg]))
    refine ⟨st' :: ss', fun x y h => ?_, by simp [l], fun g hg => ?_⟩
    · simp only [List.zip_cons_cons, List.mem_cons, Prod.mk.injEq] at h
      rcases h with ⟨rfl, rfl⟩ | h
      · exact s
      · exact F x y h
    · simp only [List.mem_cons] at hg
      rcases hg with rfl | hg
      · exact w
      · exact W g hg

theorem ex_R : ∀ (rt : rectype), wf_rectype rt → ∃ rt', fun_subst_rectype rt (tvsK k) tus rt' ∧ wf_rectype rt'
  | .REC (.mk_list ss), hw => by
    cases hw with
    | rectype_case_0 _ hl hs =>
      obtain ⟨ss', F, l, W⟩ := ex_Ss ss (by simpa [proj_list_0, Forall] using hs)
      have wr : wf_rectype (.REC (.mk_list ss')) :=
        .rectype_case_0 _ (by simpa [proj_list_0, l] using hl) (by simpa [proj_list_0, Forall] using W)
      have hm := minus_idx hMR k tus hk
      refine ⟨_, .fun_subst_rectype_case_0 _ _ _ (tvsK k) tus _ _ F F (by rw [hm]) ?_ ?_ wr l l, wr⟩
      · rw [hm]; intro tv htv
        obtain ⟨i, hi, rfl⟩ := List.mem_map.mp htv
        exact .typevar_case_0 _ (wf_uN32 (by rw [List.mem_range] at hi; omega))
      · rw [hm]; exact hwu
end

end

/-! ### Closing in module contexts -/

omit hMR in
/-- The closed types of the module as type uses. -/
abbrev tusN (A : Nat → storagetype) (m : Types.M) : List typeuse := (closedTypes A m m.n).map typeuse_deftype

omit hMR in
/-- Closing a defined type by substituting the closed types of the module for type indices. -/
def ClosesD (A : Nat → storagetype) (m : Types.M) (d d' : deftype) : Prop :=
  fun_subst_deftype d (tvsK m.n) (tusN A m) d'

omit hMR in
inductive Closes (A : Nat → storagetype) (m : Types.M) : heaptype → heaptype → Prop where
  | abs (x : Abs) : Closes A m (trAbs x) (trAbs x)
  | bot : Closes A m .BOT .BOT
  | idx {y : Nat} : y < m.n → Closes A m (._IDX ⟨y⟩) (heaptype_deftype (tD A (dtOf m y)))
  | defd {d d' : deftype} : ClosesD A m d d' → Closes A m (heaptype_deftype d) (heaptype_deftype d')

omit hMR in
theorem tusN_len (m : Types.M) : (tvsK m.n).length = (tusN A m).length := by simp [closedTypes]

theorem closesD_det {m : Types.M} {d d1 d2 : deftype} (h1 : ClosesD A m d d1) (h2 : ClosesD A m d d2) :
    d1 = d2 := det_deftype hMR h1 h2 (tusN_len m)

section
variable (hA : AtomsOK A) {m : Types.M} (hw : SubWF m) (hn : m.n < 2 ^ 32)
  (hf : ∀ t < m.n, (m.d t).fields.length < 2 ^ 32)
include hA hw hn hf

omit hMR in
theorem tusN_wf : ∀ u ∈ tusN A m, wf_typeuse u := by
  intro u hu
  obtain ⟨d, hd, rfl⟩ := List.mem_map.mp hu
  obtain ⟨t, ht, rfl⟩ := List.mem_map.mp hd
  rw [List.mem_range] at ht
  exact .typeuse_case_1 _ _ (wf_trG hA _ (okG_closeGroup hw.toWF hn hf t ht))

theorem closesD_ex {d : deftype} (hd : wf_deftype d) : ∃ d', ClosesD A m d d' ∧ wf_deftype d' := by
  cases hd with
  | deftype_case_0 rt i hr =>
    obtain ⟨rt', h, w⟩ := ex_R hMR m.n (tusN A m) (by simp [closedTypes]) (by omega) (tusN_wf hA hw hn hf) rt hr
    exact ⟨._DEF rt' i, .fun_subst_deftype_case_0 _ _ _ _ _ _ h h (.deftype_case_0 _ _ w), .deftype_case_0 _ _ w⟩

theorem closesD_T {x : Nat} (hx : x < m.n) : ClosesD A m (T A m x) (tD A (dtOf m x)) := by
  have st := close_step_ex hA hMR hw.toWF hn hf (t := x) (k := m.n) hx (by have := hw.group x hx; omega)
    (Nat.le_refl _)
  match st with
  | .fun_subst_all_deftype_case_0 _ vn _ _ _ _ _ H0 _ _ hvn =>
    rw [closedTypes_len] at hvn
    subst hvn
    exact H0

/-- `fun_clos_deftype` in contexts with the types of the module is closing. -/
theorem clos_closes (hAd : AtomsDet A) {C : context} (hT : C.TYPES = specTypes A m m.n) {d v : deftype}
    (h : fun_clos_deftype C d v) : ClosesD A m d v := by
  match h with
  | .fun_clos_deftype_case_0 _ _ dl _ v1 _ _ hc hs0 he _ _ =>
    rw [hT] at hc
    have := clos_det hAd hMR hw.toWF m.n (Nat.le_refl _) _ hc
    subst this he
    match hs0 with
    | .fun_subst_all_deftype_case_0 _ vn _ _ _ _ _ H0 _ _ hvn =>
      rw [closedTypes_len] at hvn
      subst hvn
      exact H0

end

/-- The structure of closing: the same index, with the rec group closed. -/
theorem closesD_inv {m : Types.M} {rt : rectype} {i : Nat} {d' : deftype} (h : ClosesD A m (._DEF rt i) d') :
    ∃ rt', d' = ._DEF rt' i ∧ fun_subst_rectype rt (tvsK m.n) (tusN A m) rt' ∧ wf_rectype rt' := by
  match h with
  | .fun_subst_deftype_case_0 _ _ _ _ rt1 rt' R1 R0 w =>
    rw [det_rectype hMR R1 R0 (tusN_len m)] at w
    cases w with
    | deftype_case_0 _ _ w => exact ⟨rt', rfl, R0, w⟩

theorem rect_subs {m : Types.M} {rt rt' : rectype} (h : fun_subst_rectype rt (tvsK m.n) (tusN A m) rt') :
    (rtSubs rt').length = (rtSubs rt).length ∧ ∀ j (hj : j < (rtSubs rt).length) (hj' : j < (rtSubs rt').length),
      fun_subst_subtype (rtSubs rt)[j] (tvsK m.n) (tusN A m) (rtSubs rt')[j] := by
  obtain ⟨stl, tv', tu', vv0, rfl, rfl, he, F, l⟩ := rectype_inv h
  rw [minus_idx hMR m.n (tusN A m) (by simp [closedTypes])] at he
  obtain ⟨e1, e2⟩ := Prod.mk.inj he
  subst e1 e2
  exact ⟨l, fun j hj _ => forall2_get F l hj⟩

omit hMR in
theorem closesD_mk {m : Types.M} {rt rt' : rectype} (h : fun_subst_rectype rt (tvsK m.n) (tusN A m) rt')
    (w : wf_rectype rt') (j : Nat) : ClosesD A m (._DEF rt j) (._DEF rt' j) :=
  .fun_subst_deftype_case_0 _ _ _ _ _ _ h h (.deftype_case_0 _ _ w)

omit hMR in
theorem subst_sub_inv {st st' : subtype} {tvs tus} (h : fun_subst_subtype st tvs tus st') :
    ∃ f us c us' c', st = .SUB f us c ∧ st' = .SUB f us' c' ∧ fun_subst_comptype c tvs tus c' ∧
      Forall₂ (fun v t => fun_subst_typeuse t tvs tus v) us' us ∧ us'.length = us.length := by
  cases h with
  | fun_subst_subtype_case_0 f us c _ _ c' us' hc F l => exact ⟨f, us, c, us', c', rfl, rfl, hc, F, l⟩

/-- What closing makes of the subtype of a defined type. -/
theorem rolled_closes {m : Types.M} {d d' : deftype} (h : ClosesD A m d d') {f us c}
    (hr : rolled d = some (.SUB f us c)) : ∃ us' c', rolled d' = some (.SUB f us' c') ∧
      Forall₂ (fun v t => fun_subst_typeuse t (tvsK m.n) (tusN A m) v) us' us ∧ us'.length = us.length ∧
      ckind c' = ckind c := by
  cases d with
  | _DEF rt i =>
    obtain ⟨rt', rfl, hs, _⟩ := closesD_inv hMR h
    obtain ⟨l, hj⟩ := rect_subs hMR hs
    simp only [rolled] at hr ⊢
    have hi : i < (rtSubs rt).length := by
      rcases Nat.lt_or_ge i (rtSubs rt).length with h | h
      · exact h
      · simp [List.getElem?_eq_none h] at hr
    rw [List.getElem?_eq_getElem hi] at hr
    have hsub := hj i hi (by rw [l]; exact hi)
    rw [Option.some.inj hr] at hsub
    obtain ⟨f1, us1, c1, us', c', e, e', hc, F, ll⟩ := subst_sub_inv hsub
    cases e
    refine ⟨us', c', ?_, F, ll, subst_ckind hc⟩
    rw [List.getElem?_eq_getElem (l := rtSubs rt') (by rw [l]; exact hi), e']

theorem kindS_closes {m : Types.M} {d d' : deftype} (h : ClosesD A m d d') : kindS d' = kindS d := by
  cases d with
  | _DEF rt i =>
    cases hr : rolled (._DEF rt i) with
    | some st =>
      cases st with
      | SUB f us c =>
        obtain ⟨us', c', hr', _, _, hk⟩ := rolled_closes hMR h hr
        rw [kindS_of hr, kindS_of hr', hk]
    | none =>
      obtain ⟨rt', rfl, hs, _⟩ := closesD_inv hMR h
      obtain ⟨l, _⟩ := rect_subs hMR hs
      simp only [kindS, hr]
      simp only [rolled] at hr ⊢
      rw [List.getElem?_eq_none (l := rtSubs rt') (by have := List.getElem?_eq_none_iff.mp hr; omega)]

omit hMR in
theorem lookup_idx_out (y : Nat) : ∀ (l : List Nat) (tus : List typeuse), y ∉ l → l.length = tus.length →
    lookupTV (._IDX ⟨y⟩) (l.map fun j => (._IDX ⟨j⟩ : typevar)) tus = ._IDX ⟨y⟩
  | [], [], _, _ => rfl
  | j :: l, tu :: tus, h, hl => by
    simp only [List.mem_cons, not_or] at h
    simp only [List.map_cons, lookupTV]
    have : (typevar._IDX ⟨y⟩ == typevar._IDX ⟨j⟩) = false := by simp [h.1]
    rw [this]
    exact lookup_idx_out y l tus h.2 (by simpa using hl)
  | [], _ :: _, _, hl | _ :: _, [], _, hl => by simp at hl

omit hMR in
theorem closes_heap_def {m : Types.M} {d : deftype} {k : heaptype} (h : Closes A m (heaptype_deftype d) k) :
    ∃ d', k = heaptype_deftype d' ∧ ClosesD A m d d' := by
  cases d with
  | _DEF rt i =>
    generalize e : heaptype_deftype (._DEF rt i) = x at h
    cases h with
    | abs y => cases y <;> simp [trAbs, heaptype_deftype] at e
    | bot => simp [heaptype_deftype] at e
    | idx => simp [heaptype_deftype] at e
    | @defd d0 d' hd =>
      cases d0 with
      | _DEF rt0 i0 =>
        simp only [heaptype_deftype, heaptype._DEF.injEq] at e
        obtain ⟨rfl, rfl⟩ := e
        exact ⟨d', rfl, hd⟩

/-- Closing commutes with taking supertypes: the supertype of a closed type is the closed supertype. -/
theorem super_closes {m : Types.M} {d d' : deftype} (hd : ClosesD A m d d') {f tu c}
    (hr : rolled d = some (.SUB f [tu] c)) {e' : deftype} (hs : superS d' = some e') :
    Closes A m (heaptype_typeuse (unUse (rtOf d) tu)) (heaptype_deftype e') := by
  cases d with
  | _DEF rt i =>
  obtain ⟨rt', rfl, hst, w⟩ := closesD_inv hMR hd
  obtain ⟨us', c', hr', F, l, _⟩ := rolled_closes hMR hd hr
  match us', F, l with
  | [tu'], F, _ =>
    have htu : fun_subst_typeuse tu (tvsK m.n) (tusN A m) tu' := F tu' tu (by simp)
    simp only [superS, hr', rtOf] at hs
    obtain ⟨len, _⟩ := rect_subs hMR hst
    cases tu with
    | REC j =>
      have e1 : tu' = .REC j := by
        cases htu with
        | fun_subst_typeuse_case_0 _ _ _ v hv =>
          rw [subst_typevar_det hv (tusN_len m)]
          simp only [Option.get!_some]
          exact lookup_rec_idx _ _ _ (by simp [closedTypes])
      subst e1
      simp only [unUse, rtOf] at hs ⊢
      by_cases hj : j < (rtSubs rt').length
      · rw [if_pos hj] at hs
        cases hs
        have hj2 : j < (rtSubs rt).length := by rw [← len]; exact hj
        simp only [hj2, ite_true]
        exact .defd (closesD_mk hst w j)
      · rw [if_neg hj] at hs; cases hs
    | _IDX x =>
      obtain ⟨y⟩ := x
      have e1 : tu' = lookupTV (._IDX ⟨y⟩) (tvsK m.n) (tusN A m) := by
        cases htu with
        | fun_subst_typeuse_case_1 _ _ _ v hv => rw [subst_typevar_det hv (tusN_len m)]; rfl
      simp only [unUse] at hs ⊢
      by_cases hy : y < m.n
      · rw [e1, lookup_idx_get' y m.n _ (by simp [closedTypes]) hy] at hs
        simp only [tusN, List.getElem_map, closedTypes, List.getElem_range, typeuse_deftype, tD] at hs
        cases hs
        exact .idx hy
      · rw [e1, lookup_idx_out y _ _ (by simpa using hy) (by simp [closedTypes])] at hs
        cases hs
    | _DEF rt2 j =>
      cases htu with
      | fun_subst_typeuse_case_2 _ _ _ _ d'' hd'' =>
        obtain ⟨r, rfl, _, _⟩ := closesD_inv hMR hd''
        simp only [unUse, typeuse_deftype] at hs ⊢
        cases hs
        exact .defd hd''

/-! ### Shapes of the types of a module -/

omit hMR in
/-- The types of a module have the shapes of `wah_type_def_t`, with atoms of well-shaped closed forms. -/
def ShapeM (A : Nat → storagetype) (m : Types.M) : Prop :=
  ∀ t < m.n,
    (((m.d t).kind = kStruct ∧ (m.d t).counts = ((m.d t).fields.length, 0)) ∨
     ((m.d t).kind = kArray ∧ (m.d t).fields.length = 1 ∧ (m.d t).counts = (1, 0)) ∨
     ((m.d t).kind = kFunc ∧ (m.d t).counts.1 + (m.d t).counts.2 = (m.d t).fields.length ∧
      ∀ f ∈ (m.d t).fields, f.2 = false ∧ ∀ a, f.1 = .atom a → A a ≠ .I8 ∧ A a ≠ .I16)) ∧
    ∀ f ∈ (m.d t).fields, ∀ a, f.1 = .atom a → ShapeAtom A a

omit hMR in
theorem shape_closeGroup {m : Types.M} (hw : WF m) (hs : ShapeM A m) :
    ∀ t < m.n, ShapeL A (closeGroup m (m.start t)) := by
  intro t ht
  induction hst : m.start t using Nat.strongRecOn generalizing t with
  | _ s ih =>
    subst hst
    have hg := hw.group t ht
    have hsg := gsize_start m hw t ht
    rw [closeGroup_eq]
    have hcref : ∀ k < m.gsize (m.start t), ∀ y ∈ (m.d (m.start t + k)).refs, ∀ b,
        ShapeV A (cref m (m.start t) (.ref y b)) ∧ NotPacked A (cref m (m.start t) (.ref y b)) ∧
        ShapeSup A (some (cref m (m.start t) (.ref y false))) := by
      intro k hk y hy b
      have htk : m.start t + k < m.n := by omega
      have hst : m.start (m.start t + k) = m.start t := (hw.same t ht _ (by omega) (by omega)).1
      have hv := ref_valid hw htk hy
      rw [hst] at hv
      rcases hv with h | ⟨h1, h2, hyn⟩
      · rw [show cref m (m.start t) (.ref y b) = .inner (y - m.start t) b by simp [cref, h],
          show cref m (m.start t) (.ref y false) = .inner (y - m.start t) false by simp [cref, h]]
        exact ⟨trivial, trivial, rfl⟩
      · have hn' : ¬ (m.start t ≤ y ∧ y < m.start t + m.gsize (m.start t)) := by omega
        rw [show cref m (m.start t) (.ref y b) = .dt (closeGroup m (m.start y)) (y - m.start y) b by
            simp [cref, hn', h2],
          show cref m (m.start t) (.ref y false) = .dt (closeGroup m (m.start y)) (y - m.start y) false by
            simp [cref, hn', h2]]
        have := ih (m.start y) h2 y hyn rfl
        exact ⟨this, trivial, rfl, this⟩
    have key : ∀ l : List Nat, (∀ k ∈ l, k < m.gsize (m.start t)) →
        ShapeL A (l.map fun k => closeSub m (m.start t) (m.start t + k)) := by
      intro l hl
      induction l with
      | nil => trivial
      | cons k l ihl =>
        refine ⟨?_, ihl (fun j hj => hl j (by simp [hj]))⟩
        have hk := hl k (by simp)
        have htk : m.start t + k < m.n := by omega
        obtain ⟨hshape, hatoms⟩ := hs _ htk
        have hfs : ∀ fs : List (VT × Bool), (∀ f ∈ fs, f ∈ (m.d (m.start t + k)).fields) →
            ShapeFs A (fs.map fun f => (cref m (m.start t) f.1, f.2)) := by
          intro fs hfs
          induction fs with
          | nil => trivial
          | cons f fs ihf =>
            refine ⟨?_, ihf (fun g hg => hfs g (by simp [hg]))⟩
            obtain ⟨v, mu⟩ := f
            cases v with
            | atom a => exact hatoms (VT.atom a, mu) (hfs _ (by simp)) a rfl
            | ref y b => exact (hcref k hk y (mem_refs_field (y := y) (null := b) (m := mu) (hfs _ (by simp))) b).1
        simp only [closeSub, ShapeS]
        refine ⟨?_, hfs _ (fun f hf => hf), ?_⟩
        · cases hsup : (m.d (m.start t + k)).super with
          | none => trivial
          | some x =>
            simp only [Option.map_some]
            exact (hcref k hk x (mem_refs_super hsup) false).2.2
        · simp only [List.length_map]
          rcases hshape with h | h | ⟨h1, h2, h3⟩
          · exact .inl h
          · exact .inr (.inl h)
          · refine .inr (.inr ⟨h1, h2, ?_⟩)
            intro f hf
            obtain ⟨⟨v, mu⟩, hm, rfl⟩ := List.mem_map.mp hf
            refine ⟨(h3 _ hm).1, ?_⟩
            cases v with
            | atom a => exact (h3 _ hm).2 a rfl
            | ref y b => exact (hcref k hk y (mem_refs_field (y := y) (null := b) (m := mu) hm) b).2.1
    exact key _ (fun k hk => by simpa using hk)

/-! ### Closed forms of valid types are good -/

omit hMR in
theorem ckind_trComp {kind : Nat} (h : kind = kStruct ∨ kind = kArray ∨ kind = kFunc) (counts : Nat × Nat)
    (fs : List fieldtype) : ckind (trComp kind counts fs) = kind := by
  rcases h with rfl | rfl | rfl <;> rfl

omit hMR in
theorem kind_valid {m : Types.M} (hs : ShapeM A m) {t : Nat} (ht : t < m.n) :
    (m.d t).kind = kStruct ∨ (m.d t).kind = kArray ∨ (m.d t).kind = kFunc := by
  rcases (hs t ht).1 with h | h | h
  · exact .inl h.1
  · exact .inr (.inl h.1)
  · exact .inr (.inr h.1)

omit hMR in
theorem rolled_T {m : Types.M} (hw : WF m) {x : Nat} (hx : x < m.n) :
    rolled (T A m x) = some (fS A (g1 (m.start x) (m.gsize (m.start x))) (m.d x)) := by
  have hg := hw.group x hx
  have hsg := gsize_start m hw x hx
  simp only [T, rolled, rolledG, rtSubs, List.getElem?_map,
    List.getElem?_range (show x - m.start x < m.gsize (m.start x) by omega)]
  simp only [Option.map_some]
  congr 3; omega

omit hMR in
theorem unrollht_idx {C x s} (h : fun_unrollht_ C (._IDX x) s) : fun_unrolldt ((C.TYPES)[proj_uN_0 x]!) s := by
  cases h; assumption

/-- Defined types that close to `e` and are valid in contexts with the types of the module, or closed forms of
types of the module. -/
def QD (A : Nat → storagetype) (m : Types.M) (e : deftype) : Prop :=
  (∃ (C' : context) (d : deftype), C'.TYPES = specTypes A m m.n ∧ Deftype_ok C' d ∧ ClosesD A m d e) ∨
  (∃ y < m.n, e = tD A (dtOf m y))

section
variable (hA : AtomsOK A) {m : Types.M} (hw : SubWF m) (hn : m.n < 2 ^ 32)
  (hf : ∀ t < m.n, (m.d t).fields.length < 2 ^ 32) (hs : ShapeM A m)
include hA hw hn hf hs

omit hMR in
theorem goodD_dtOf {y : Nat} (hy : y < m.n) : GoodD (tD A (dtOf m y)) := by
  have hok : OkD (dtOf m y) := ⟨okG_closeGroup hw.toWF hn hf y hy, by
    have := hw.group y hy; rw [dtOf, closeGroup_eq]; simp; rw [gsize_start m hw.toWF y hy]; omega⟩
  exact goodD_tD hok (shape_closeGroup hw.toWF hs y hy) (kindsOK_dtOf m hw y hy)

omit hMR in
theorem kindS_dtOf {y : Nat} (hy : y < m.n) : kindS (tD A (dtOf m y)) = (m.d y).kind := by
  have hok : OkD (dtOf m y) := ⟨okG_closeGroup hw.toWF hn hf y hy, by
    have := hw.group y hy; rw [dtOf, closeGroup_eq]; simp; rw [gsize_start m hw.toWF y hy]; omega⟩
  rw [kindS_tD hok (shape_closeGroup hw.toWF hs y hy), kindOf_dtOf m hw.toWF y hy]

/-- One step up from a good type: its supertype has its kind and is good. -/
theorem q_step {e : deftype} (hq : QD A m e) :
    NiceD e ∧ ∀ e', superS e = some e' → kindS e' = kindS e ∧ QD A m e' := by
  rcases hq with ⟨C', d, hT', hok, hcl⟩ | ⟨y, hy, rfl⟩
  · cases hok with
    | mk_Deftype_ok _ rt i nn sts hR hrt hi wfC _ wfX wfR hnn =>
      subst hrt hnn
      have hsub := rec_ok_get hR i hi
      simp only [Nat.zero_add] at hsub
      have hr : rolled (._DEF (.REC (.mk_list sts)) i) = some sts[i] := by
        simp [rolled, rtSubs, List.getElem?_eq_getElem hi]
      obtain ⟨rt', rfl, hst, wst⟩ := closesD_inv hMR hcl
      obtain ⟨len, _⟩ := rect_subs hMR hst
      revert hsub hr
      cases sts[i] with
      | SUB f tus ct =>
      intro hsub hr
      obtain ⟨tus', ct', hr', F, l, hk⟩ := rolled_closes hMR hcl hr
      have hke : kindS (._DEF rt' i) = ckind ct := by rw [kindS_of hr', hk]
      rw [hke]
      rcases subtype_ok_super hsub with h0 | ⟨tu, rfl, htu, hbf, tus'', ct'', hun, hck⟩
      · subst h0
        have : tus' = [] := by simpa using l
        subst this
        exact ⟨⟨f, [], ct', hr', .inl rfl⟩, fun e' he => by simp [superS, hr'] at he⟩
      · match tus', F, l with
        | [tu'], F, _ =>
        have hsu : fun_subst_typeuse tu (tvsK m.n) (tusN A m) tu' := F tu' tu (by simp)
        cases htu with
        | typeidx _ x _ hx _ _ _ =>
          obtain ⟨y⟩ := x
          rw [app_types, hT'] at hx
          simp only [specTypes, List.length_append, List.length_map, List.length_range, proj_uN_0] at hx
          have hy : y < m.n := by simpa using hx
          have e1 : tu' = typeuse_deftype (tD A (dtOf m y)) := by
            cases hsu with
            | fun_subst_typeuse_case_1 _ _ _ v hv =>
              rw [subst_typevar_det hv (tusN_len m)]
              simp only [Option.get!_some]
              rw [lookup_idx_get' y m.n _ (by simp [closedTypes]) hy]
              simp [tusN, closedTypes]
          subst e1
          have hsup : superS (._DEF rt' i) = some (tD A (dtOf m y)) := by
            simp [superS, hr', unUse, typeuse_deftype, tD]
          refine ⟨⟨f, _, ct', hr', .inr ⟨_, rfl, by simp [hsup]⟩⟩, fun e' he => ?_⟩
          rw [hsup] at he; cases he
          refine ⟨?_, .inr ⟨y, hy, rfl⟩⟩
          rw [kindS_dtOf hA hw hn hf hs hy]
          have hu := unrollht_idx hun
          rw [app_types, hT'] at hu
          simp only [proj_uN_0, List.nil_append] at hu
          rw [specTypes_get m hy] at hu
          obtain ⟨c, hc, hc'⟩ := unroll_shape hMR hu (rolled_T hw.toWF hy)
          obtain ⟨-, -, e3⟩ := subtype.SUB.inj hc
          rw [← hck, e3, hc', ckind_trComp (kind_valid hs hy)]
        | rec_ _ k _ hk' _ _ _ =>
          simp [before] at hbf
          have hkn : k < sts.length := Nat.lt_trans hbf hi
          have e1 : tu' = .REC k := by
            cases hsu with
            | fun_subst_typeuse_case_0 _ _ _ v hv =>
              rw [subst_typevar_det hv (tusN_len m)]
              simp only [Option.get!_some]
              exact lookup_rec_idx _ _ _ (by simp [closedTypes])
          subst e1
          have hk2 : k < (rtSubs rt').length := by rw [len]; exact hkn
          have hsup : superS (._DEF rt' i) = some (._DEF rt' k) := by
            simp [superS, hr', unUse, rtOf, hk2]
          refine ⟨⟨f, _, ct', hr', .inr ⟨_, rfl, by simp [hsup]⟩⟩, fun e' he => ?_⟩
          rw [hsup] at he; cases he
          refine ⟨?_, .inl ⟨C', _, hT',
            .mk_Deftype_ok _ _ k _ sts hR rfl hkn wfC (.deftype_case_0 _ _ wfR) wfX wfR rfl,
            closesD_mk hst wst k⟩⟩
          rw [kindS_closes hMR (closesD_mk hst wst k)]
          have e1 := unrollht_rec hun
          rw [app_recs, List.getElem!_eq_getElem?_getD, List.getElem?_append_left hkn,
            List.getElem?_eq_getElem hkn] at e1
          simp only [Option.getD_some] at e1
          have hr2 : rolled (._DEF (.REC (.mk_list sts)) k) = some (.SUB none tus'' ct'') := by
            simp [rolled, rtSubs, List.getElem?_eq_getElem hkn, e1]
          rw [kindS_of hr2, hck]
        | deftype _ d0 hd0 _ _ =>
          cases d0 with
          | _DEF rt2 k =>
          cases hsu with
          | fun_subst_typeuse_case_2 _ _ _ _ d'' hd'' =>
            obtain ⟨r, rfl, _, _⟩ := closesD_inv hMR hd''
            have hsup : superS (._DEF rt' i) = some (._DEF r k) := by
              simp [superS, hr', unUse, typeuse_deftype]
            refine ⟨⟨f, _, ct', hr', .inr ⟨_, rfl, by simp [hsup]⟩⟩, fun e' he => ?_⟩
            rw [hsup] at he; cases he
            refine ⟨?_, .inl ⟨_, _, by rw [app_types, hT']; rfl, hd0, hd''⟩⟩
            rw [kindS_closes hMR hd'']
            obtain ⟨f2, tus2, ct2, hr2⟩ := ok_rolled hd0
            obtain ⟨c, hc, hc'⟩ := unroll_shape hMR (unrollht_def hun) hr2
            cases hc
            rw [kindS_of hr2, ← hc', hck]
  · have g := goodD_dtOf hA hw hn hf hs hy
    refine ⟨(g _ (.refl _)).1, fun e' he => ⟨(g _ (.refl _)).2 e' he, ?_⟩⟩
    have hok : OkD (dtOf m y) := ⟨okG_closeGroup hw.toWF hn hf y hy, by
      have := hw.group y hy; rw [dtOf, closeGroup_eq]; simp; rw [gsize_start m hw.toWF y hy]; omega⟩
    rw [show tD A (dtOf m y) = tD A (dtOf m y) from rfl, superS_tD hok, superOf_dtOf m hw.toWF y hy] at he
    cases hsup : (m.d y).super with
    | none => simp [hsup] at he
    | some z =>
      simp [hsup] at he
      subst he
      exact .inr ⟨z, by have := hw.super_lt y hy z hsup; omega, rfl⟩

theorem goodD_of_q {e : deftype} (hq : QD A m e) : GoodD e := by
  intro x c
  induction c with
  | refl => exact ⟨(q_step hMR hA hw hn hf hs hq).1, fun e' h => ((q_step hMR hA hw hn hf hs hq).2 e' h).1⟩
  | step sx _ ih => exact ih ((q_step hMR hA hw hn hf hs hq).2 _ sx).2

/-- Valid heap types close to good ones. -/
theorem ok_goodC {C : context} (hT : C.TYPES = specTypes A m m.n) (hR : C.RECS = []) {h : heaptype}
    (hk : Heaptype_ok C h) : ∃ k, Closes A m h k ∧ GoodH k := by
  cases hk with
  | abs _ x _ =>
    cases x
    all_goals first
      | exact ⟨_, .bot, trivial⟩
      | exact ⟨_, .abs .any, trivial⟩ | exact ⟨_, .abs .eq, trivial⟩ | exact ⟨_, .abs .i31, trivial⟩
      | exact ⟨_, .abs .struct, trivial⟩ | exact ⟨_, .abs .array, trivial⟩ | exact ⟨_, .abs .none, trivial⟩
      | exact ⟨_, .abs .func, trivial⟩ | exact ⟨_, .abs .nofunc, trivial⟩ | exact ⟨_, .abs .exn, trivial⟩
      | exact ⟨_, .abs .noexn, trivial⟩ | exact ⟨_, .abs .extern, trivial⟩
      | exact ⟨_, .abs .noextern, trivial⟩
  | typeuse _ tu ht _ hw' =>
    cases ht with
    | typeidx _ x _ hx _ _ _ =>
      obtain ⟨y⟩ := x
      rw [hT] at hx
      simp only [specTypes, List.length_map, List.length_range, proj_uN_0] at hx
      exact ⟨_, .idx hx, (goodH_def _).mpr (goodD_dtOf hA hw hn hf hs hx)⟩
    | rec_ _ _ _ hx _ _ _ => simp [hR] at hx
    | deftype _ d hd _ hwd =>
      obtain ⟨d', hcl, _⟩ := closesD_ex hMR hA hw hn hf hwd
      exact ⟨_, .defd hcl, (goodH_def _).mpr (goodD_of_q hMR hA hw hn hf hs (.inl ⟨C, d, hT, hd, hcl⟩))⟩
  | bot => exact ⟨_, .bot, trivial⟩

end

/-! ### Subtyping in module contexts is algorithmic on closed types -/

omit hMR in
theorem closes_abs {m : Types.M} {x : Abs} {k : heaptype} (h : Closes A m (trAbs x) k) : k = trAbs x := by
  generalize e : trAbs x = z at h
  cases h with
  | abs y => cases x <;> cases y <;> simp_all [trAbs]
  | bot => cases x <;> simp [trAbs] at e
  | idx => cases x <;> simp [trAbs] at e
  | defd => cases x <;> simp [trAbs, heaptype_deftype] at e

omit hMR in
theorem closes_bot {m : Types.M} {k : heaptype} (h : Closes A m .BOT k) : k = .BOT := by
  generalize e : heaptype.BOT = z at h
  cases h with
  | abs y => cases y <;> simp [trAbs] at e
  | bot => rfl
  | idx => simp at e
  | defd => simp [heaptype_deftype] at e

omit hMR in
theorem closes_idx {m : Types.M} {y : Nat} {k : heaptype} (h : Closes A m (._IDX ⟨y⟩) k) :
    k = heaptype_deftype (tD A (dtOf m y)) ∧ y < m.n := by
  generalize e : heaptype._IDX ⟨y⟩ = z at h
  cases h with
  | abs x => cases x <;> simp [trAbs] at e
  | bot => simp at e
  | @idx y' hy => simp at e; subst e; exact ⟨rfl, hy⟩
  | defd => simp [heaptype_deftype] at e

omit hMR in
theorem closes_ne_bot {m : Types.M} {h k : heaptype} (hc : Closes A m h k) (hk : k = .BOT) : h = .BOT := by
  cases hc with
  | abs x => cases x <;> simp [trAbs] at hk
  | bot => rfl
  | idx => simp [heaptype_deftype] at hk
  | defd => simp [heaptype_deftype] at hk

omit hMR in
theorem closes_idx' {m : Types.M} {x : uN} {k : heaptype} (h : Closes A m (._IDX x) k) :
    k = heaptype_deftype (tD A (dtOf m (proj_uN_0 x))) ∧ proj_uN_0 x < m.n := by
  obtain ⟨y⟩ := x
  exact closes_idx h

theorem closes_det {m : Types.M} {h k1 k2 : heaptype} (c1 : Closes A m h k1) (c2 : Closes A m h k2) : k1 = k2 := by
  cases c1 with
  | abs x => exact (closes_abs c2).symm
  | bot => exact (closes_bot c2).symm
  | idx hy => exact (closes_idx c2).1.symm
  | defd hd =>
    obtain ⟨d', rfl, hd'⟩ := closes_heap_def c2
    rw [closesD_det hMR hd hd']

/-- Closing keeps the subtypes of defined types. -/
theorem rolled_closes_rev {m : Types.M} {d d' : deftype} (h : ClosesD A m d d') {st' : subtype}
    (hr : rolled d' = some st') : ∃ st, rolled d = some st := by
  cases d with
  | _DEF rt i =>
    obtain ⟨rt', rfl, hs, _⟩ := closesD_inv hMR h
    obtain ⟨l, _⟩ := rect_subs hMR hs
    simp only [rolled] at hr ⊢
    have hi : i < (rtSubs rt').length := by
      rcases Nat.lt_or_ge i (rtSubs rt').length with h | h
      · exact h
      · simp [List.getElem?_eq_none h] at hr
    exact ⟨_, List.getElem?_eq_getElem (l := rtSubs rt) (by rw [← l]; exact hi)⟩

theorem expand_kind' {d ct} (h : Expand d ct) {st} (hr : rolled d = some st) : ckind ct = kindS d := by
  match h with
  | .mk_Expand _ _ f tus s hun hs _ _ _ =>
    subst hs
    cases st with
    | SUB f0 tus0 ct0 =>
      cases d with
      | _DEF rt i =>
        obtain ⟨c, hc, hc'⟩ := unroll_shape hMR hun hr
        obtain ⟨-, -, e3⟩ := subtype.SUB.inj hc
        rw [kindS_of hr, e3, hc']

section
variable (hA : AtomsOK A) (hAd : AtomsDet A) {m : Types.M} (hw : SubWF m) (hn : m.n < 2 ^ 32)
  (hf : ∀ t < m.n, (m.d t).fields.length < 2 ^ 32) (hs : ShapeM A m)
include hA hAd hw hn hf hs

theorem closes_T {x : uN} (hx : proj_uN_0 x < m.n) {C : context} (hT : C.TYPES = specTypes A m m.n) :
    Closes A m (heaptype_deftype ((C.TYPES)[proj_uN_0 x]!)) (heaptype_deftype (tD A (dtOf m (proj_uN_0 x)))) := by
  rw [hT, specTypes_get m hx]
  exact .defd (closesD_T hMR hA hw hn hf hx)

mutual
theorem sub_closes : ∀ {C h1 h2}, Heaptype_sub C h1 h2 → C.TYPES = specTypes A m m.n → C.RECS = [] →
    ∀ {k1 k2}, Closes A m h1 k1 → Closes A m h2 k2 → GoodH k1 → GoodH k2 → SAlgo k1 k2
  | _, _, _, .refl _ _ _ _, _, _, _, _, c1, c2, g1, _ => by rw [closes_det hMR c1 c2] at g1 ⊢; exact salgo_refl g1
  | _, _, _, .trans _ _ _ _ ok s1 s2 _ _ _ _, hT, hR, _, _, c1, c2, g1, g2 => by
    obtain ⟨k', c', g'⟩ := ok_goodC hMR hA hw hn hf hs hT hR ok
    exact salgo_trans g1 g' (sub_closes s1 hT hR c1 c' g1 g') (sub_closes s2 hT hR c' c2 g' g2) g2
  | _, _, _, .eq_any .., _, _, _, _, c1, c2, _, _ => by
    rw [closes_abs (x := .eq) c1, closes_abs (x := .any) c2]; exact (salgo_abs_abs .eq .any).mpr rfl
  | _, _, _, .i31_eq .., _, _, _, _, c1, c2, _, _ => by
    rw [closes_abs (x := .i31) c1, closes_abs (x := .eq) c2]; exact (salgo_abs_abs .i31 .eq).mpr rfl
  | _, _, _, .struct_eq .., _, _, _, _, c1, c2, _, _ => by
    rw [closes_abs (x := .struct) c1, closes_abs (x := .eq) c2]; exact (salgo_abs_abs .struct .eq).mpr rfl
  | _, _, _, .array_eq .., _, _, _, _, c1, c2, _, _ => by
    rw [closes_abs (x := .array) c1, closes_abs (x := .eq) c2]; exact (salgo_abs_abs .array .eq).mpr rfl
  | _, _, _, .struct _ d _ he _ _ _ _, _, _, _, _, c1, c2, g1, _ => by
    obtain ⟨d', rfl, hd⟩ := closes_heap_def c1
    rw [closes_abs (x := .struct) c2]
    obtain ⟨f, tus, ct, hr', _⟩ := (((goodH_def d').mp g1) d' (.refl _)).1
    obtain ⟨st, hr⟩ := rolled_closes_rev hMR hd hr'
    have := expand_kind' hMR he hr
    cases d' with
    | _DEF rt i => exact (salgo_def_abs rt i .struct).mpr (by rw [kindS_closes hMR hd, ← this]; rfl)
  | _, _, _, .array _ d _ he _ _ _ _, _, _, _, _, c1, c2, g1, _ => by
    obtain ⟨d', rfl, hd⟩ := closes_heap_def c1
    rw [closes_abs (x := .array) c2]
    obtain ⟨f, tus, ct, hr', _⟩ := (((goodH_def d').mp g1) d' (.refl _)).1
    obtain ⟨st, hr⟩ := rolled_closes_rev hMR hd hr'
    have := expand_kind' hMR he hr
    cases d' with
    | _DEF rt i => exact (salgo_def_abs rt i .array).mpr (by rw [kindS_closes hMR hd, ← this]; rfl)
  | _, _, _, .func _ d _ _ he _ _ _ _, _, _, _, _, c1, c2, g1, _ => by
    obtain ⟨d', rfl, hd⟩ := closes_heap_def c1
    rw [closes_abs (x := .func) c2]
    obtain ⟨f, tus, ct, hr', _⟩ := (((goodH_def d').mp g1) d' (.refl _)).1
    obtain ⟨st, hr⟩ := rolled_closes_rev hMR hd hr'
    have := expand_kind' hMR he hr
    cases d' with
    | _DEF rt i => exact (salgo_def_abs rt i .func).mpr (by rw [kindS_closes hMR hd, ← this]; rfl)
  | _, _, _, .def _ d1 d2 ds _ _ _, hT, hR, _, _, c1, c2, g1, g2 => by
    obtain ⟨e1, rfl, h1⟩ := closes_heap_def c1
    obtain ⟨e2, rfl, h2⟩ := closes_heap_def c2
    exact (salgo_defs e1 e2).mpr (dsub_closes ds hT hR h1 h2 ((goodH_def e1).mp g1) ((goodH_def e2).mp g2))
  | _, _, _, .typeidx_l _ x _ _ s _ _ _, hT, hR, _, _, c1, c2, g1, g2 => by
    obtain ⟨rfl, hy⟩ := closes_idx' c1
    exact sub_closes s hT hR (closes_T hMR hA hAd hw hn hf hs hy hT) c2 g1 g2
  | _, _, _, .typeidx_r _ _ x _ s _ _ _, hT, hR, _, _, c1, c2, g1, g2 => by
    obtain ⟨rfl, hy⟩ := closes_idx' c2
    exact sub_closes s hT hR c1 (closes_T hMR hA hAd hw hn hf hs hy hT) g1 g2
  | _, _, _, .rec_struct _ _ _ _ hx _ _ _ _ _, _, hR, _, _, _, _, _, _ => by simp [hR] at hx
  | _, _, _, .rec_array _ _ _ _ hx _ _ _ _ _, _, hR, _, _, _, _, _, _ => by simp [hR] at hx
  | _, _, _, .rec_func _ _ _ _ _ hx _ _ _ _ _, _, hR, _, _, _, _, _, _ => by simp [hR] at hx
  | _, _, _, .rec_sub _ _ _ _ _ _ _ hx _ _ _ _, _, hR, _, _, _, _, _, _ => by simp [hR] at hx
  | _, _, _, .none _ _ s ne _ _ _ _ _, hT, hR, _, k2, c1, c2, _, g2 => by
    rw [closes_abs (x := .none) c1]
    exact salgo_bottom g2 (fun e => ne (closes_ne_bot c2 e)) .none .any (by intro x; cases x <;> decide)
      (by intro k; simp [defAbs, absDef]) (sub_closes s hT hR c2 (.abs .any) g2 trivial)
  | _, _, _, .nofunc _ _ s ne _ _ _ _ _, hT, hR, _, k2, c1, c2, _, g2 => by
    rw [closes_abs (x := .nofunc) c1]
    exact salgo_bottom g2 (fun e => ne (closes_ne_bot c2 e)) .nofunc .func (by intro x; cases x <;> decide)
      (by intro k; simp [defAbs, absDef]) (sub_closes s hT hR c2 (.abs .func) g2 trivial)
  | _, _, _, .noexn _ _ s ne _ _ _ _ _, hT, hR, _, k2, c1, c2, _, g2 => by
    rw [closes_abs (x := .noexn) c1]
    exact salgo_bottom g2 (fun e => ne (closes_ne_bot c2 e)) .noexn .exn (by intro x; cases x <;> decide)
      (by intro k; simp [defAbs]) (sub_closes s hT hR c2 (.abs .exn) g2 trivial)
  | _, _, _, .noextern _ _ s ne _ _ _ _ _, hT, hR, _, k2, c1, c2, _, g2 => by
    rw [closes_abs (x := .noextern) c1]
    exact salgo_bottom g2 (fun e => ne (closes_ne_bot c2 e)) .noextern .extern (by intro x; cases x <;> decide)
      (by intro k; simp [defAbs]) (sub_closes s hT hR c2 (.abs .extern) g2 trivial)
  | _, _, _, .bot .., _, _, _, _, c1, _, _, _ => by rw [closes_bot c1]; trivial
termination_by structural _ _ _ h => h

theorem dsub_closes : ∀ {C d1 d2}, Deftype_sub C d1 d2 → C.TYPES = specTypes A m m.n → C.RECS = [] →
    ∀ {e1 e2}, ClosesD A m d1 e1 → ClosesD A m d2 e2 → GoodD e1 → GoodD e2 → ChainS e1 e2
  | _, _, _, .refl _ _ _ _ _ cl2 cl1 e _ _ _ _ _, hT, _, _, _, h1, h2, _, _ => by
    rw [closesD_det hMR h1 (clos_closes hMR hA hw hn hf hAd hT cl1),
      closesD_det hMR h2 (clos_closes hMR hA hw hn hf hAd hT cl2), e]
    exact .refl _
  | _, _, _, .super _ d1 d2 f tus ct i v0 hun hv hi hsub _ _ _ _ _, hT, hR, e1, e2, h1, h2, g1, g2 => by
    obtain ⟨f', tus', ct', hr', hn'⟩ := (g1 e1 (.refl _)).1
    obtain ⟨st, hr⟩ := rolled_closes_rev hMR h1 hr'
    cases st with
    | SUB f0 tus0 ct0 =>
    obtain ⟨us', c', hr'', F, l, _⟩ := rolled_closes hMR h1 hr
    rw [hr'] at hr''
    obtain ⟨-, rfl, -⟩ := subtype.SUB.inj (Option.some.inj hr'')
    have key : ∃ tu e', tus = [unUse (rtOf d1) tu] ∧ i = 0 ∧ rolled d1 = some (.SUB f0 [tu] ct0) ∧
        superS e1 = some e' := by
      cases d1 with
      | _DEF rt k =>
      obtain ⟨c, hc, _⟩ := unroll_shape hMR hun hr
      rw [hc] at hv
      obtain ⟨-, htus, -⟩ := subtype.SUB.inj hv
      rcases hn' with h0 | ⟨tu', rfl, hsome⟩
      · subst h0
        have : tus0 = [] := by simpa using l.symm
        subst this; subst htus; simp at hi
      · obtain ⟨e', he'⟩ := Option.isSome_iff_exists.mp hsome
        match tus0, l with
        | [tu], _ =>
          refine ⟨tu, e', htus.symm, by simp [← htus] at hi; omega, hr, he'⟩
    obtain ⟨tu, e', htus, hi0, hr1, he'⟩ := key
    have hcl : Closes A m (heaptype_typeuse tus[i]!) (heaptype_deftype e') := by
      rw [htus, hi0]
      exact super_closes hMR h1 hr1 he'
    have ge' := goodD_super g1 he'
    have r := sub_closes hsub hT hR hcl (.defd h2) ((goodH_def e').mpr ge') ((goodH_def e2).mpr g2)
    exact .step he' ((salgo_defs e' e2).mp r)
termination_by structural _ _ _ h => h
end

omit hAd in
theorem okD_dtOf {x : Nat} (hx : x < m.n) : OkD (dtOf m x) := ⟨okG_closeGroup hw.toWF hn hf x hx, by
  have := hw.group x hx; rw [dtOf, closeGroup_eq]; simp; rw [gsize_start m hw.toWF x hx]; omega⟩

omit hAd in
theorem closes_MH {a : MH} (ha : OkMH m a) :
    Closes A m (specMH a) (trH A (denMH m a)) ∧ GoodH (trH A (denMH m a)) := by
  cases a with
  | abs x => exact ⟨.abs x, by cases x <;> trivial⟩
  | idx x => exact ⟨.idx ha, (goodH_def _).mpr (goodD_dtOf hA hw hn hf hs ha)⟩

/-- `Heaptype_sub` of the spec between type indices of a module, in contexts with the types of the module and no
recursive types, implies heap subtyping of WahProofs/Subtype.lean between the types. -/
theorem idx_hsub_complete {C : context} (hT : C.TYPES = specTypes A m m.n) (hR : C.RECS = []) {a b : MH}
    (ha : OkMH m a) (hb : OkMH m b) (h : Heaptype_sub C (specMH a) (specMH b)) : HSub (denMH m a) (denMH m b) := by
  obtain ⟨ca, ga⟩ := closes_MH hMR hA hw hn hf hs ha
  obtain ⟨cb, gb⟩ := closes_MH hMR hA hw hn hf hs hb
  have r := sub_closes hMR hA hAd hw hn hf hs h hT hR ca cb ga gb
  have ok : ∀ {c : MH}, OkMH m c → OkH (denMH m c) ∧ ShapeH A (denMH m c) ∧ ValidH (denMH m c) := by
    intro c hc
    cases c with
    | abs _ => exact ⟨trivial, trivial, trivial⟩
    | idx x => exact ⟨okD_dtOf hMR hA hw hn hf hs hc, shape_closeGroup hw.toWF hs x hc, kindsOK_dtOf m hw x hc⟩
  obtain ⟨oa, sa, va⟩ := ok ha
  obtain ⟨ob, sb, vb⟩ := ok hb
  exact hsub_of_algo va vb (algo_complete oa ob sa sb r)

end

/-- Both directions: in contexts with the types of a module, `Heaptype_sub` between type indices is heap
subtyping of the types. -/
theorem idx_hsub_iff (hA : AtomsOK A) (hAd : AtomsDet A) (hMR : MinusRecs) {m : Types.M} (hw : SubWF m)
    (hn : m.n < 2 ^ 32) (hf : ∀ t < m.n, (m.d t).fields.length < 2 ^ 32) (hs : ShapeM A m) {C : context}
    (hC : wf_context C) (hT : C.TYPES = specTypes A m m.n) (hR : C.RECS = []) {a b : MH} (ha : OkMH m a)
    (hb : OkMH m b) : HSub (denMH m a) (denMH m b) ↔ Heaptype_sub C (specMH a) (specMH b) :=
  ⟨idx_hsub_sound hA hMR hw hn hf hC hT ha hb, idx_hsub_complete hMR hA hAd hw hn hf hs hT hR ha hb⟩

end Wah.SpecTec
