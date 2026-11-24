/-
The closed forms of the types of a module (`dtOf` of WahProofs/Types.lean) are what the spec makes of them: rolling
each rec group (`$rollrt`, with references to its own types as `REC`) and closing the types in order
(`$clos_deftypes`, substituting the types for earlier type indices) gives the translation of `dtOf`.

Types of the module are in SpecTec syntax with references as type indices (`_IDX`). The theorems are for both
directions: what the spec rolls and closes them to is unique, and it exists.
-/
import WahSpecTec.Valtype

set_option linter.unusedSectionVars false

namespace Wah.SpecTec
open Wah.Types Wah.Subtype

variable {A : Nat → storagetype}

/-! ### Types of a module in SpecTec syntax, with references by `gu` -/

def fV (A : Nat → storagetype) (gu : Nat → typeuse) : VT → storagetype
  | .atom a => A a
  | .ref y null => .REF (nullOpt null) (heaptype_typeuse (gu y))

def fF (A : Nat → storagetype) (gu : Nat → typeuse) (f : VT × Bool) : fieldtype :=
  .mk_fieldtype (mutOpt f.2) (fV A gu f.1)

def fS (A : Nat → storagetype) (gu : Nat → typeuse) (td : TD) : subtype :=
  .SUB (finalOpt td.final) (td.super.toList.map gu) (trComp td.kind td.counts (td.fields.map (fF A gu)))

/-- References that substitutions look up. -/
def IsVar : typeuse → Prop
  | .REC _ | ._IDX _ => True
  | _ => False

def tvOf : typeuse → typevar
  | .REC i => .REC i
  | ._IDX x => ._IDX x
  | _ => .REC 0

/-- What a substitution makes of a reference. -/
def look (tvs : List typevar) (tus : List typeuse) (u : typeuse) : typeuse := lookupTV (tvOf u) tvs tus

/-! ### Substitutions of these types are unique -/

/-- Atoms are left alone by substitutions, and only by the way of leaving them alone. -/
structure AtomsDet (A : Nat → storagetype) : Prop where
  det : ∀ a tvs tus r, fun_subst_storagetype (A a) tvs tus r → r = A a
  detV : ∀ a tvs tus r, fun_subst_valtype (toVal (A a)) tvs tus r → r = toVal (A a)

theorem subst_var_heap_det {u : typeuse} (hu : IsVar u) {tvs tus r} (hl : tvs.length = tus.length)
    (h : fun_subst_heaptype (heaptype_typeuse u) tvs tus r) : r = heaptype_typeuse (look tvs tus u) := by
  cases u with
  | REC i =>
    cases h with
    | fun_subst_heaptype_case_0 _ _ _ v hv => rw [subst_typevar_det hv hl]; rfl
    | fun_subst_heaptype_case_3 _ _ _ hb => exact absurd (.fun_subst_heaptype_case_0 i tvs tus none) hb
  | _IDX x =>
    cases h with
    | fun_subst_heaptype_case_1 _ _ _ v hv => rw [subst_typevar_det hv hl]; rfl
    | fun_subst_heaptype_case_3 _ _ _ hb => exact absurd (.fun_subst_heaptype_case_1 x tvs tus none) hb
  | _DEF _ _ => exact hu.elim

theorem subst_fV_det (hAd : AtomsDet A) {gu : Nat → typeuse} (hg : ∀ y, IsVar (gu y)) {tvs tus}
    (hl : tvs.length = tus.length) {v : VT} {r} (h : fun_subst_storagetype (fV A gu v) tvs tus r) :
    r = fV A (look tvs tus ∘ gu) v := by
  cases v with
  | atom a => exact hAd.det a tvs tus r h
  | ref y null =>
    simp only [fV] at h ⊢
    cases h with
    | fun_subst_storagetype_case_1 _ _ _ _ _ hv =>
      cases hv with
      | fun_subst_valtype_case_5 _ _ _ _ _ hr =>
        cases hr with
        | fun_subst_reftype_case_0 _ _ _ _ _ hh => rw [subst_var_heap_det (hg y) hl hh]; rfl

theorem subst_fF_det (hAd : AtomsDet A) {gu : Nat → typeuse} (hg : ∀ y, IsVar (gu y)) {tvs tus}
    (hl : tvs.length = tus.length) {f : VT × Bool} {r} (h : fun_subst_fieldtype (fF A gu f) tvs tus r) :
    r = fF A (look tvs tus ∘ gu) f := by
  cases h with
  | fun_subst_fieldtype_case_0 _ _ _ _ _ hz => simp only [fF]; rw [subst_fV_det hAd hg hl hz]

theorem subst_fval_det (hAd : AtomsDet A) {gu : Nat → typeuse} (hg : ∀ y, IsVar (gu y)) {tvs tus}
    (hl : tvs.length = tus.length) {f : VT × Bool} {r} (h : fun_subst_valtype (fval (fF A gu f)) tvs tus r) :
    r = fval (fF A (look tvs tus ∘ gu) f) := by
  obtain ⟨v, m⟩ := f
  cases v with
  | atom a => exact hAd.detV a tvs tus r h
  | ref y null =>
    simp only [fval, fF, fV, toVal] at h ⊢
    cases h with
    | fun_subst_valtype_case_5 _ _ _ _ _ hr =>
      cases hr with
      | fun_subst_reftype_case_0 _ _ _ _ _ hh => rw [subst_var_heap_det (hg y) hl hh]; rfl

theorem subst_bot_det {tvs tus r} (h : fun_subst_fieldtype (.mk_fieldtype none .BOT) tvs tus r) :
    r = .mk_fieldtype none .BOT := by
  cases h with
  | fun_subst_fieldtype_case_0 _ _ _ _ _ hz =>
    cases hz with
    | fun_subst_storagetype_case_0 _ _ _ hv => cases hv; rfl

theorem subst_use_det {u : typeuse} (hu : IsVar u) {tvs tus r} (hl : tvs.length = tus.length)
    (h : fun_subst_typeuse u tvs tus r) : r = look tvs tus u := by
  cases u with
  | REC i => cases h with | fun_subst_typeuse_case_0 _ _ _ v hv => rw [subst_typevar_det hv hl]; rfl
  | _IDX x => cases h with | fun_subst_typeuse_case_1 _ _ _ v hv => rw [subst_typevar_det hv hl]; rfl
  | _DEF _ _ => exact hu.elim

theorem forall2_map_mem {α β : Type} {R : α → β → Prop} (g : β → α) :
    ∀ {l1 : List α} {l2 : List β}, (∀ y ∈ l2, ∀ x, R x y → x = g y) → l1.length = l2.length →
      Forall₂ R l1 l2 →
      l1 = l2.map g
  | [], [], _, _, _ => rfl
  | a :: l1, b :: l2, hg, hl, h => by
    rw [List.map_cons, hg b (by simp) a (h a b (by simp)),
      forall2_map_mem g (l1 := l1) (l2 := l2) (fun y hy => hg y (by simp [hy])) (by simpa using hl)
        (fun x y hm => h x y (by simp [hm]))]
  | [], _ :: _, _, hl, _ | _ :: _, [], _, hl, _ => by simp at hl

theorem forall2_src {α β γ : Type} {R : α → β → Prop} (p : γ → β) (q : γ → α) :
    ∀ {l1 : List α} {l : List γ}, (∀ x ∈ l, ∀ r, R r (p x) → r = q x) → l1.length = l.length →
      Forall₂ R l1 (l.map p) → l1 = l.map q
  | [], [], _, _, _ => rfl
  | a :: l1, b :: l, hg, hl, h => by
    rw [List.map_cons, hg b (by simp) a (h a (p b) (by simp)),
      forall2_src p q (l1 := l1) (l := l) (fun y hy => hg y (by simp [hy])) (by simpa using hl)
        (fun x y hm => h x y (by simp [hm]))]
  | [], _ :: _, _, hl, _ | _ :: _, [], _, hl, _ => by simp at hl

/-- Substitutions of subtypes with variables as references look the variables up. -/
theorem subst_fS_det (hAd : AtomsDet A) {gu : Nat → typeuse} (hg : ∀ y, IsVar (gu y)) {tvs tus}
    (hl : tvs.length = tus.length) {td : TD} {r} (h : fun_subst_subtype (fS A gu td) tvs tus r) :
    r = fS A (look tvs tus ∘ gu) td := by
  cases h with
  | fun_subst_subtype_case_0 _ _ _ _ _ c us hc hus hlen =>
    simp only [fS]
    rw [forall2_map_mem (look tvs tus) (fun u hu x hx => subst_use_det (by
        obtain ⟨y, -, rfl⟩ := List.mem_map.mp hu; exact hg y) hl hx) hlen hus]
    congr 1
    · simp [List.map_map]
    · revert hc
      unfold trComp
      split
      · intro hc
        cases hc with
        | fun_subst_comptype_case_0 _ _ _ vs F hl' =>
          rw [forall2_src (fF A gu) (fF A (look tvs tus ∘ gu)) (fun f _ r hr => subst_fF_det hAd hg hl hr)
            (by simpa using hl') F]
      · split
        · intro hc
          cases hc with
          | fun_subst_comptype_case_1 _ _ _ v hv =>
            cases hf : td.fields with
            | nil => rw [hf] at hv; simp at hv ⊢; exact subst_bot_det hv
            | cons f _ => rw [hf] at hv; simp at hv ⊢; exact subst_fF_det hAd hg hl hv
        · intro hc
          cases hc with
          | fun_subst_comptype_case_2 _ _ _ _ v1 v0 F1 F0 l1 l0 =>
            have e : ∀ (l' : List (VT × Bool)) (vs : List valtype), vs.length = l'.length →
                Forall₂ (fun v t => fun_subst_valtype t tvs tus v) vs (l'.map (fval ∘ fF A gu)) →
                vs = l'.map (fval ∘ fF A (look tvs tus ∘ gu)) := fun l' vs hlen F =>
              forall2_src (fval ∘ fF A gu) (fval ∘ fF A (look tvs tus ∘ gu))
                (fun f _ r hr => subst_fval_det hAd hg hl hr) hlen F
            simp only [← List.map_take, ← List.map_drop, List.map_map] at F1 F0 l1 l0 ⊢
            rw [e _ _ (by simpa using l0) F0, e _ _ (by simpa using l1) F1]

/-! ### Substitutions of these types exist -/

theorem subst_typevar_ex (tv : typevar) : ∀ {tvs : List typevar} {tus : List typeuse}, tvs.length = tus.length →
    fun_subst_typevar tv tvs tus (some (lookupTV tv tvs tus))
  | [], [], _ => .fun_subst_typevar_case_0 tv
  | a :: tvs, b :: tus, hl => by
    have h := fun_subst_typevar.fun_subst_typevar_case_1 tv a tvs b tus _ (subst_typevar_ex tv (tvs := tvs)
      (tus := tus) (by simpa using hl))
    simp only [Option.map_some] at h
    simpa [lookupTV] using h
  | [], _ :: _, hl | _ :: _, [], hl => by simp at hl

theorem subst_use_ex {u : typeuse} (hu : IsVar u) {tvs tus} (hl : tvs.length = tus.length) :
    fun_subst_typeuse u tvs tus (look tvs tus u) := by
  cases u with
  | REC i => exact .fun_subst_typeuse_case_0 i tvs tus _ (subst_typevar_ex _ hl)
  | _IDX x => exact .fun_subst_typeuse_case_1 x tvs tus _ (subst_typevar_ex _ hl)
  | _DEF _ _ => exact hu.elim

theorem subst_heap_ex {u : typeuse} (hu : IsVar u) {tvs tus} (hl : tvs.length = tus.length) :
    fun_subst_heaptype (heaptype_typeuse u) tvs tus (heaptype_typeuse (look tvs tus u)) := by
  cases u with
  | REC i => exact .fun_subst_heaptype_case_0 i tvs tus _ (subst_typevar_ex _ hl)
  | _IDX x => exact .fun_subst_heaptype_case_1 x tvs tus _ (subst_typevar_ex _ hl)
  | _DEF _ _ => exact hu.elim

theorem subst_fS_ex (hA : AtomsOK A) {gu : Nat → typeuse} (hg : ∀ y, IsVar (gu y)) {tvs tus}
    (hl : tvs.length = tus.length) (td : TD) :
    fun_subst_subtype (fS A gu td) tvs tus (fS A (look tvs tus ∘ gu) td) := by
  have hf : ∀ f ∈ td.fields, fun_subst_fieldtype (fF A gu f) tvs tus (fF A (look tvs tus ∘ gu) f) := by
    intro f _
    obtain ⟨v, m⟩ := f
    refine .fun_subst_fieldtype_case_0 _ _ _ _ _ ?_
    cases v with
    | atom a => exact hA.subst a tvs tus
    | ref y null => exact subst_ref (subst_heap_ex (hg y) hl)
  refine .fun_subst_subtype_case_0 _ _ _ _ _ _ _ (subst_comp _ _ _ _ _ hf) ?_ (by simp)
  exact forall2_map _ gu (look tvs tus ∘ gu) _ (fun y _ => subst_use_ex (hg y) hl)

/-! ### Looking up type indices -/

theorem lookup_idx_shift (s y : Nat) : ∀ l : List Nat,
    lookupTV (._IDX ⟨y⟩) (l.map fun i => (._IDX ⟨s + i⟩ : typevar)) (l.map fun i => (.REC i : typeuse)) =
      if s ≤ y ∧ y - s ∈ l then .REC (y - s) else ._IDX ⟨y⟩
  | [] => by simp [lookupTV, typeuse_typevar]
  | j :: l => by
    simp only [List.map_cons, lookupTV, lookup_idx_shift s y l]
    simp only [List.mem_cons]
    by_cases h : y = s + j
    · subst h; simp
    · have : (typevar._IDX ⟨y⟩ == typevar._IDX ⟨s + j⟩) = false := by simp [h]
      rw [this]
      simp only [Bool.false_eq_true, ite_false]
      by_cases h3 : s ≤ y ∧ y - s ∈ l
      · rw [if_pos h3, if_pos ⟨h3.1, .inr h3.2⟩]
      · rw [if_neg h3, if_neg (fun ⟨a, b⟩ => b.elim (fun e => h (by omega)) (fun m => h3 ⟨a, m⟩))]

theorem lookup_rec_idx (i : Nat) (tus : List typeuse) : ∀ l : List Nat, l.length = tus.length →
    lookupTV (.REC i) (l.map fun j => (._IDX ⟨j⟩ : typevar)) tus = .REC i
  | [], h => by cases tus <;> simp_all [lookupTV, typeuse_typevar]
  | j :: l, h => by
    cases tus with
    | nil => simp at h
    | cons tu tus => simp only [List.map_cons, lookupTV]; simp [lookup_rec_idx i tus l (by simpa using h)]

theorem lookup_idx_get (y : Nat) (tus : List typeuse) (hy : y < tus.length) :
    lookupTV (._IDX ⟨y⟩) ((List.range tus.length).map fun j => (._IDX ⟨j⟩ : typevar)) tus = tus[y] := by
  have key : ∀ (o : Nat) (tus : List typeuse) (y : Nat) (h1 : o ≤ y) (h2 : y < o + tus.length),
      lookupTV (._IDX ⟨y⟩) ((List.range' o tus.length).map fun j => (._IDX ⟨j⟩ : typevar)) tus =
        tus[y - o]'(by omega) := by
    intro o tus
    induction tus generalizing o with
    | nil => intro y h1 h2; simp at h2; omega
    | cons tu tus ih =>
      intro y h1 h2
      simp only [List.length_cons, List.range'_succ, List.map_cons, lookupTV]
      by_cases h : y = o
      · subst h; simp
      · have : (typevar._IDX ⟨y⟩ == typevar._IDX ⟨o⟩) = false := by simp [h]
        rw [this]
        simp only [Bool.false_eq_true, ite_false]
        rw [ih (o + 1) y (by omega) (by simp at h2; omega)]
        have : y - o = (y - (o + 1)) + 1 := by omega
        simp [this]
  rw [List.range_eq_range']
  simpa using key 0 tus y (by omega) (by simpa using hy)

theorem lookup_idx_get' (y k : Nat) (tus : List typeuse) (hk : tus.length = k) (hy : y < k) :
    lookupTV (._IDX ⟨y⟩) ((List.range k).map fun j => (._IDX ⟨j⟩ : typevar)) tus = tus[y]'(by omega) := by
  subst hk; exact lookup_idx_get y tus hy

theorem minusRecs_idx (l : List Nat) (tus : List typeuse) (h : l.length = tus.length) :
    minusRecs (l.map fun j => (._IDX ⟨j⟩ : typevar)) tus =
      some (l.map fun j => (._IDX ⟨j⟩ : typevar), tus) := by
  induction l generalizing tus with
  | nil => cases tus <;> simp_all [minusRecs]
  | cons j l ih => cases tus with
    | nil => simp at h
    | cons tu tus => simp only [List.map_cons, minusRecs]; rw [ih tus (by simpa using h)]; rfl

/-! ### Well-formedness and congruence -/

theorem wf_heap_of_use {u : typeuse} (h : wf_typeuse u) : wf_heaptype (heaptype_typeuse u) := by
  cases h <;> first | (constructor; assumption) | constructor

theorem mem_refs_field {td : TD} {y : Nat} {null : Bool} {m : Bool} (h : (VT.ref y null, m) ∈ td.fields) :
    y ∈ td.refs := by
  simp only [TD.refs, List.mem_append, List.mem_filterMap]
  exact .inr ⟨_, h, rfl⟩

theorem mem_refs_super {td : TD} {y : Nat} (h : td.super = some y) : y ∈ td.refs := by
  simp [TD.refs, h]

theorem wf_fS (hA : AtomsOK A) {gu : Nat → typeuse} {td : TD} (hw : ∀ y ∈ td.refs, wf_typeuse (gu y))
    (hl : td.fields.length < 2 ^ 32) : wf_subtype (fS A gu td) := by
  refine .subtype_case_0 _ _ _ ?_ (wf_comp _ _ _ (by simpa using hl) ?_)
  · intro u hu
    obtain ⟨y, hy, rfl⟩ := List.mem_map.mp hu
    exact hw y (mem_refs_super (by simpa using hy))
  · intro f hf
    obtain ⟨⟨v, m⟩, hm, rfl⟩ := List.mem_map.mp hf
    refine .fieldtype_case_0 _ _ ?_
    cases v with
    | atom a => exact hA.wf a
    | ref y null => exact .storagetype_case_5 _ _ (wf_heap_of_use (hw y (mem_refs_field hm)))

theorem fS_congr {gu gu' : Nat → typeuse} {td : TD} (h : ∀ y ∈ td.refs, gu y = gu' y) :
    fS A gu td = fS A gu' td := by
  have hf : td.fields.map (fF A gu) = td.fields.map (fF A gu') := by
    apply List.map_congr_left
    intro ⟨v, m⟩ hm
    cases v with
    | atom a => rfl
    | ref y null => simp only [fF, fV]; rw [h y (mem_refs_field hm)]
  have hs : td.super.toList.map gu = td.super.toList.map gu' := by
    cases hsup : td.super with
    | none => rfl
    | some y => simp [h y (mem_refs_super hsup)]
  simp only [fS, hf, hs]

theorem wf_uN32 {y : Nat} (h : y < 2 ^ 32) : wf_uN 32 ⟨y⟩ :=
  .uN_case_0 _ _ ⟨Nat.zero_le _, by simp; omega⟩ (by simp)

/-! ### Rolling a rec group -/

/-- References of the module as type indices. -/
def g0 : Nat → typeuse := fun y => ._IDX ⟨y⟩

/-- References of a rolled group starting at `s` with `n` types. -/
def g1 (s n : Nat) : Nat → typeuse := fun y => if s ≤ y ∧ y < s + n then .REC (y - s) else ._IDX ⟨y⟩

/-- The rec group starting at `s`, as in the module. -/
def rtM (A : Nat → storagetype) (m : Types.M) (s : Nat) : rectype :=
  .REC (.mk_list ((List.range (m.gsize s)).map fun k => fS A g0 (m.d (s + k))))

/-- The rec group starting at `s`, rolled. -/
def rolledG (A : Nat → storagetype) (m : Types.M) (s : Nat) : rectype :=
  .REC (.mk_list ((List.range (m.gsize s)).map fun k => fS A (g1 s (m.gsize s)) (m.d (s + k))))

theorem look_g0 (s n : Nat) :
    look ((List.range n).map fun i => (._IDX ⟨s + i⟩ : typevar)) ((List.range n).map fun i => (.REC i : typeuse))
      ∘ g0 = g1 s n := by
  funext y
  simp only [Function.comp, g0, look, tvOf, lookup_idx_shift, g1, List.mem_range]
  by_cases h : s ≤ y ∧ y < s + n
  · rw [if_pos ⟨h.1, by omega⟩, if_pos h]
  · rw [if_neg (by omega), if_neg h]

/-- What `$rollrt` makes of a rec group of the module is unique. -/
theorem rollrt_det (hAd : AtomsDet A) (m : Types.M) (s : Nat) {r : rectype} (h : fun_rollrt ⟨s⟩ (rtM A m s) r) :
    r = rolledG A m s := by
  cases h with
  | fun_rollrt_case_0 _ vn stl v1 v0 _ F0 hrt _ _ _ l2 l0 =>
    simp only [rtM, rectype.REC.injEq, list.mk_list.injEq] at hrt
    subst hrt
    simp only [List.length_map, List.length_range] at l2
    subst l2
    simp only [proj_uN_0] at F0
    simp only [rolledG, rectype.REC.injEq, list.mk_list.injEq]
    refine forall2_src _ _ (fun k _ r hr => ?_) (by simpa using l0) F0
    rw [subst_fS_det hAd (gu := g0) (fun y => trivial) (by simp) hr, look_g0]

theorem wf_rtM (hA : AtomsOK A) (m : Types.M) (s : Nat) (hn : m.gsize s < 2 ^ 32)
    (hf : ∀ k < m.gsize s, (m.d (s + k)).fields.length < 2 ^ 32) (gu : Nat → typeuse)
    (hg : ∀ k < m.gsize s, ∀ y ∈ (m.d (s + k)).refs, wf_typeuse (gu y)) :
    wf_rectype (.REC (.mk_list ((List.range (m.gsize s)).map fun k => fS A gu (m.d (s + k))))) := by
  refine .rectype_case_0 _ (by simpa [proj_list_0] using hn) ?_
  intro st hst
  simp only [proj_list_0, List.mem_map, List.mem_range] at hst
  obtain ⟨k, hk, rfl⟩ := hst
  exact wf_fS hA (hg k hk) (hf k hk)

/-- `$rollrt` rolls a rec group of the module. -/
theorem rollrt_ex (hA : AtomsOK A) (m : Types.M) (s : Nat) (hn : m.gsize s < 2 ^ 32)
    (hy : ∀ k < m.gsize s, ∀ y ∈ (m.d (s + k)).refs, y < 2 ^ 32)
    (hf : ∀ k < m.gsize s, (m.d (s + k)).fields.length < 2 ^ 32) :
    fun_rollrt ⟨s⟩ (rtM A m s) (rolledG A m s) := by
  have F := forall2_map (List.range (m.gsize s)) (fun k => fS A g0 (m.d (s + k)))
    (fun k => fS A (g1 s (m.gsize s)) (m.d (s + k)))
    (fun v st => fun_subst_subtype st
      ((List.range (m.gsize s)).map fun i => (._IDX ⟨proj_uN_0 ⟨s⟩ + i⟩ : typevar))
      ((List.range (m.gsize s)).map fun i => (.REC i : typeuse)) v) (fun k _ => by
        have := subst_fS_ex hA (gu := g0) (fun y => trivial)
          (tvs := (List.range (m.gsize s)).map fun i => (._IDX ⟨s + i⟩ : typevar))
          (tus := (List.range (m.gsize s)).map fun i => (.REC i : typeuse)) (by simp) (m.d (s + k))
        rw [look_g0] at this; exact this)
  have w1 := wf_rtM hA m s hn hf (g1 s (m.gsize s)) (fun k hk y _ => by
    simp only [g1]; split
    · exact .typeuse_case_2 _
    · exact .typeuse_case_0 _ (wf_uN32 (hy k hk y ‹_›)))
  have w0 := wf_rtM hA m s hn hf g0 (fun k hk y hy' => .typeuse_case_0 _ (wf_uN32 (hy k hk y hy')))
  exact .fun_rollrt_case_0 _ _ 0 (m.gsize s) _ _ _ F F rfl w1 w0 (by simp) (by simp) (by simp)

/-! ### Closing the types in order -/

/-- References of the group at `s` once closed: its own types are `REC`, the others their closed forms. -/
def gcl (A : Nat → storagetype) (m : Types.M) (s : Nat) : Nat → typeuse := fun y =>
  if s ≤ y ∧ y < s + m.gsize s then .REC (y - s) else typeuse_deftype (tD A (dtOf m y))

/-- References of valid modules are in their group or before it. -/
theorem ref_valid {m : Types.M} (hw : WF m) {t : Nat} (ht : t < m.n) {y : Nat} (hy : y ∈ (m.d t).refs) :
    (m.start t ≤ y ∧ y < m.start t + m.gsize (m.start t)) ∨
      (y < m.start t ∧ m.start y < m.start t ∧ y < m.n) := by
  have h1 := hw.refs t ht y hy
  have hg := hw.group t ht
  rw [gsize_start m hw t ht]
  by_cases h : m.start t ≤ y
  · exact .inl ⟨h, h1⟩
  · have hyn : y < m.n := by omega
    have := (hw.group y hyn).1
    exact .inr ⟨by omega, by omega, hyn⟩

theorem trS_closeSub {m : Types.M} (hw : WF m) {t : Nat} (ht : t < m.n) :
    trS A none (closeSub m (m.start t) t) = fS A (gcl A m (m.start t)) (m.d t) := by
  have hv : ∀ y ∈ (m.d t).refs, ∀ b,
      trV A none (cref m (m.start t) (.ref y b)) = fV A (gcl A m (m.start t)) (.ref y b)
      ∧ trU A none (cref m (m.start t) (.ref y b)) = gcl A m (m.start t) y := by
    intro y hy b
    rcases ref_valid hw ht hy with h | ⟨h1, h2, _⟩
    · simp [cref, h, trV, trU, recRef, recUse, fV, gcl, heaptype_typeuse]
    · have hn : ¬ (m.start t ≤ y ∧ y < m.start t + m.gsize (m.start t)) := by omega
      simp [cref, hn, h2, trV, trU, fV, gcl, tD, dtOf, trG, typeuse_deftype, heaptype_typeuse]
  simp only [closeSub]
  rw [trS.eq_1, fS, trFs_eq]
  congr 1
  · cases hs : (m.d t).super with
    | none => rfl
    | some x => simp [trSup, (hv x (mem_refs_super hs) false).2]
  · congr 1
    rw [List.map_map]
    apply List.map_congr_left
    intro ⟨v, mu⟩ hm
    cases v with
    | atom a => rfl
    | ref y b => simp only [Function.comp, fF]; rw [(hv y (mem_refs_field hm) b).1]

/-- The types of the module with their groups rolled, as the spec has them in contexts. -/
def specTypes (A : Nat → storagetype) (m : Types.M) (k : Nat) : List deftype :=
  (List.range k).map fun t => ._DEF (rolledG A m (m.start t)) (t - m.start t)

def closedTypes (A : Nat → storagetype) (m : Types.M) (k : Nat) : List deftype :=
  (List.range k).map fun t => tD A (dtOf m t)

theorem trG_closeGroup (m : Types.M) (s : Nat) :
    trG A (closeGroup m s) =
      .REC (.mk_list ((List.range (m.gsize s)).map fun j => trS A none (closeSub m s (s + j)))) := by
  simp [trG, trSs_eq, closeGroup_eq, List.map_map, Function.comp]

/-- Closing the subtypes of the group of `k` with the closed types before `k` gives the closed group. -/
theorem close_subs {m : Types.M} (hw : WF m) {k : Nat} (hk : k < m.n) :
    ((List.range (m.gsize (m.start k))).map fun j =>
      fS A (look ((List.range k).map fun i => (._IDX ⟨i⟩ : typevar)) ((closedTypes A m k).map typeuse_deftype) ∘
        g1 (m.start k) (m.gsize (m.start k))) (m.d (m.start k + j))) =
      (List.range (m.gsize (m.start k))).map fun j => trS A none (closeSub m (m.start k) (m.start k + j)) := by
  apply List.map_congr_left
  intro j hj
  rw [List.mem_range] at hj
  have hg := hw.group k hk
  have hsg := gsize_start m hw k hk
  have ht : m.start k + j < m.n := by omega
  have hst : m.start (m.start k + j) = m.start k := (hw.same k hk _ (by omega) (by omega)).1
  have := trS_closeSub (A := A) hw ht
  rw [hst] at this
  rw [this]
  apply fS_congr
  intro y hy
  have hv := ref_valid hw ht hy
  rw [hst] at hv
  simp only [Function.comp, g1, gcl, look]
  rcases hv with h | ⟨h1, _, hyn⟩
  · rw [if_pos h, if_pos h]
    simp only [tvOf]
    exact lookup_rec_idx _ _ _ (by simp [closedTypes])
  · have hn : ¬ (m.start k ≤ y ∧ y < m.start k + m.gsize (m.start k)) := by omega
    rw [if_neg hn, if_neg hn]
    simp only [tvOf]
    have hyk : y < k := by omega
    have hlen : ((closedTypes A m k).map typeuse_deftype).length = k := by simp [closedTypes]
    rw [lookup_idx_get' y k _ hlen hyk]
    simp [closedTypes, List.getElem_map, List.getElem_range]

/-- Closed groups of valid modules translate to well-formed types. -/
theorem okG_closeGroup {m : Types.M} (hw : WF m) (hn : m.n < 2 ^ 32)
    (hf : ∀ t < m.n, (m.d t).fields.length < 2 ^ 32) : ∀ t < m.n, OkG (closeGroup m (m.start t)) := by
  intro t ht
  induction hs : m.start t using Nat.strongRecOn generalizing t with
  | _ s ih =>
    subst hs
    have hg := hw.group t ht
    have hsg := gsize_start m hw t ht
    have hss := start_start m hw t ht
    refine ⟨by rw [closeGroup_eq]; simp; omega, ?_⟩
    rw [closeGroup_eq]
    have hcref : ∀ k < m.gsize (m.start t), ∀ y ∈ (m.d (m.start t + k)).refs, ∀ b,
        OkV ((List.range (m.gsize (m.start t))).map fun k => closeSub m (m.start t) (m.start t + k)).length
          (cref m (m.start t) (.ref y b)) ∧
        OkSuper ((List.range (m.gsize (m.start t))).map fun k => closeSub m (m.start t) (m.start t + k)).length
          (some (cref m (m.start t) (.ref y b))) := by
      intro k hk y hy b
      have htk : m.start t + k < m.n := by omega
      have hst : m.start (m.start t + k) = m.start t := (hw.same t ht _ (by omega) (by omega)).1
      have hv := ref_valid hw htk hy
      rw [hst] at hv
      rcases hv with h | ⟨h1, h2, hyn⟩
      · rw [show cref m (m.start t) (.ref y b) = .inner (y - m.start t) b by simp [cref, h]]
        have : y - m.start t < ((List.range (m.gsize (m.start t))).map fun k =>
            closeSub m (m.start t) (m.start t + k)).length := by simp; omega
        exact ⟨this, this⟩
      · have hn' : ¬ (m.start t ≤ y ∧ y < m.start t + m.gsize (m.start t)) := by omega
        rw [show cref m (m.start t) (.ref y b) = .dt (closeGroup m (m.start y)) (y - m.start y) b by
          simp [cref, hn', h2]]
        have hgy := hw.group y hyn
        have := ih (m.start y) h2 y hyn rfl
        have hl : y - m.start y < (closeGroup m (m.start y)).length := by
          rw [closeGroup_eq]; simp; rw [gsize_start m hw y hyn]; omega
        exact ⟨⟨this.1, this.2, hl⟩, this.1, this.2, hl⟩
    have key : ∀ l : List Nat, (∀ k ∈ l, k < m.gsize (m.start t)) →
        OkL ((List.range (m.gsize (m.start t))).map fun k => closeSub m (m.start t) (m.start t + k)).length
          (l.map fun k => closeSub m (m.start t) (m.start t + k)) := by
      intro l hl
      induction l with
      | nil => trivial
      | cons k l ihl =>
        refine ⟨?_, ihl (fun j hj => hl j (by simp [hj]))⟩
        have hk := hl k (by simp)
        have htk : m.start t + k < m.n := by omega
        simp only [closeSub, OkS]
        refine ⟨?_, by simpa using hf _ htk, ?_⟩
        · cases hsup : (m.d (m.start t + k)).super with
          | none => trivial
          | some x =>
            simp only [Option.map_some]
            exact (hcref k hk x (mem_refs_super hsup) false).2
        · have : ∀ fs : List (VT × Bool), (∀ f ∈ fs, f ∈ (m.d (m.start t + k)).fields) →
              OkFs ((List.range (m.gsize (m.start t))).map fun k => closeSub m (m.start t) (m.start t + k)).length
                (fs.map fun f => (cref m (m.start t) f.1, f.2)) := by
            intro fs hfs
            induction fs with
            | nil => trivial
            | cons f fs ihf =>
              refine ⟨?_, ihf (fun g hg => hfs g (by simp [hg]))⟩
              obtain ⟨v, mu⟩ := f
              cases v with
              | atom a => trivial
              | ref y b => exact (hcref k hk y (mem_refs_field (y := y) (null := b) (m := mu) (hfs _ (by simp))) b).1
          exact this _ (fun f hf => hf)
    exact key _ (fun k hk => by simpa using hk)

theorem g1_var (s n y : Nat) : IsVar (g1 s n y) := by simp only [g1]; split <;> trivial

theorem closedTypes_len (m : Types.M) (k : Nat) : ((closedTypes A m k).map typeuse_deftype).length = k := by
  simp [closedTypes]

theorem minus_idx (hMR : MinusRecs) (k : Nat) (tus : List typeuse) (h : tus.length = k) :
    Option.get! (minus_recs ((List.range k).map fun i => (._IDX ⟨i⟩ : typevar)) tus) =
      ((List.range k).map fun i => (._IDX ⟨i⟩ : typevar), tus) := by
  rw [hMR, minusRecs_idx _ _ (by simp [h])]; rfl

theorem rectype_inv {rt tvs tus r} (h : fun_subst_rectype rt tvs tus r) :
    ∃ stl tv' tu' vv0, rt = .REC (.mk_list stl) ∧ r = .REC (.mk_list vv0) ∧
      (tv', tu') = Option.get! (minus_recs tvs tus) ∧
      Forall₂ (fun v st => fun_subst_subtype st tv' tu' v) vv0 stl ∧ vv0.length = stl.length := by
  cases h with
  | fun_subst_rectype_case_0 stl _ _ tv' tu' _ vv0 _ F0 he _ _ _ _ l0 =>
    exact ⟨stl, tv', tu', vv0, rfl, rfl, he, F0, l0⟩

/-- Closing type `k` with the closed types before it gives its closed form, which is unique. -/
theorem close_step_det (hAd : AtomsDet A) (hMR : MinusRecs) {m : Types.M} (hw : WF m) {k : Nat} (hk : k < m.n)
    {v : deftype} (h : fun_subst_all_deftype (._DEF (rolledG A m (m.start k)) (k - m.start k))
      ((closedTypes A m k).map typeuse_deftype) v) : v = tD A (dtOf m k) := by
  match h with
  | .fun_subst_all_deftype_case_0 _ vn _ _ _ _ _ H0 _ _ hvn =>
    rw [closedTypes_len] at hvn
    subst hvn
    match H0 with
    | .fun_subst_deftype_case_0 _ _ _ _ _ r0 _ R0 _ =>
      obtain ⟨stl, tv', tu', vv0, hrt, rfl, he, F0, l0⟩ := rectype_inv R0
      simp only [rolledG, rectype.REC.injEq, list.mk_list.injEq] at hrt
      subst hrt
      rw [minus_idx hMR _ _ (closedTypes_len m vn)] at he
      obtain ⟨e1, e2⟩ := Prod.mk.inj he
      subst e1 e2
      have := forall2_src (fun j => fS A (g1 (m.start vn) (m.gsize (m.start vn))) (m.d (m.start vn + j)))
        (fun j => fS A (look ((List.range vn).map fun i => (._IDX ⟨i⟩ : typevar))
          ((closedTypes A m vn).map typeuse_deftype) ∘ g1 (m.start vn) (m.gsize (m.start vn))) (m.d (m.start vn + j)))
        (l := List.range (m.gsize (m.start vn)))
        (fun j _ r hr => subst_fS_det hAd (g1_var _ _) (by simp [closedTypes]) hr)
        (by simpa using l0) F0
      rw [this, close_subs hw hk]
      simp only [tD, dtOf, trG_closeGroup]

theorem close_step_ex (hA : AtomsOK A) (hMR : MinusRecs) {m : Types.M} (hw : WF m) (hn : m.n < 2 ^ 32)
    (hf : ∀ t < m.n, (m.d t).fields.length < 2 ^ 32) {k : Nat} (hk : k < m.n) :
    fun_subst_all_deftype (._DEF (rolledG A m (m.start k)) (k - m.start k))
      ((closedTypes A m k).map typeuse_deftype) (tD A (dtOf m k)) := by
  have hg := okG_closeGroup hw hn hf k hk
  have hwt : wf_rectype (trG A (closeGroup m (m.start k))) := wf_trG hA _ hg
  have F := forall2_map (List.range (m.gsize (m.start k)))
    (fun j => fS A (g1 (m.start k) (m.gsize (m.start k))) (m.d (m.start k + j)))
    (fun j => fS A (look ((List.range k).map fun i => (._IDX ⟨i⟩ : typevar))
      ((closedTypes A m k).map typeuse_deftype) ∘ g1 (m.start k) (m.gsize (m.start k))) (m.d (m.start k + j)))
    (fun v st => fun_subst_subtype st ((List.range k).map fun i => (._IDX ⟨i⟩ : typevar))
      ((closedTypes A m k).map typeuse_deftype) v)
    (fun j _ => subst_fS_ex hA (g1_var _ _) (by simp [closedTypes]) _)
  rw [close_subs hw hk] at F
  have hR : fun_subst_rectype (rolledG A m (m.start k)) ((List.range k).map fun i => (._IDX ⟨i⟩ : typevar))
      ((closedTypes A m k).map typeuse_deftype) (trG A (closeGroup m (m.start k))) := by
    rw [trG_closeGroup]
    rw [trG_closeGroup] at hwt
    refine .fun_subst_rectype_case_0 _ _ _ _ _ _ _ F F (by rw [minus_idx hMR _ _ (closedTypes_len m k)]) ?_ ?_
      hwt (by simp) (by simp)
    · rw [minus_idx hMR _ _ (closedTypes_len m k)]
      intro tv htv
      obtain ⟨i, hi, rfl⟩ := List.mem_map.mp htv
      exact .typevar_case_0 _ (wf_uN32 (by rw [List.mem_range] at hi; omega))
    · rw [minus_idx hMR _ _ (closedTypes_len m k)]
      intro tu htu
      obtain ⟨d, hd, rfl⟩ := List.mem_map.mp htu
      obtain ⟨t, ht, rfl⟩ := List.mem_map.mp hd
      rw [List.mem_range] at ht
      exact .typeuse_case_1 _ _ (wf_trG hA _ (okG_closeGroup hw hn hf t (by omega)))
  have hD : fun_subst_deftype (._DEF (rolledG A m (m.start k)) (k - m.start k))
      ((List.range k).map fun i => (._IDX ⟨i⟩ : typevar)) ((closedTypes A m k).map typeuse_deftype)
      (tD A (dtOf m k)) := .fun_subst_deftype_case_0 _ _ _ _ _ _ hR hR (.deftype_case_0 _ _ hwt)
  exact .fun_subst_all_deftype_case_0 _ k _ 0 _ _ hD hD (.deftype_case_0 _ _ hwt)
    (fun i hi => .typevar_case_0 _ (wf_uN32 (by omega))) (closedTypes_len m k).symm

theorem specTypes_succ (m : Types.M) (k : Nat) :
    specTypes A m (k + 1) = specTypes A m k ++ [._DEF (rolledG A m (m.start k)) (k - m.start k)] := by
  simp [specTypes, List.range_succ]

theorem closedTypes_succ (m : Types.M) (k : Nat) :
    closedTypes A m (k + 1) = closedTypes A m k ++ [tD A (dtOf m k)] := by
  simp [closedTypes, List.range_succ]

theorem clos_det (hAd : AtomsDet A) (hMR : MinusRecs) {m : Types.M} (hw : WF m) :
    ∀ k ≤ m.n, ∀ cl, fun_clos_deftypes (specTypes A m k) cl → cl = closedTypes A m k
  | 0, _, cl, h => by rw [clos_nil h (by simp [specTypes])]; simp [closedTypes]
  | k + 1, hk, cl, h => by
    generalize hL : specTypes A m (k + 1) = L at h
    cases h with
    | fun_clos_deftypes_case_0 => simp [specTypes, List.range_succ] at hL
    | fun_clos_deftypes_case_1 dl dn dl' _ _ v1 v0 _ _ h1 hs0 heq _ _ =>
      rw [specTypes_succ] at hL
      obtain ⟨e1, e2⟩ := List.append_inj' hL rfl
      simp only [List.cons.injEq, and_true] at e2
      subst e1 e2 heq
      rw [clos_det hAd hMR hw k (by omega) _ h1] at hs0 ⊢
      rw [close_step_det hAd hMR hw (by omega) hs0, closedTypes_succ]

theorem wf_closedTypes (hA : AtomsOK A) {m : Types.M} (hw : WF m) (hn : m.n < 2 ^ 32)
    (hf : ∀ t < m.n, (m.d t).fields.length < 2 ^ 32) {k : Nat} (hk : k ≤ m.n) :
    Forall (fun d => wf_deftype d) (closedTypes A m k) := by
  intro d hd
  obtain ⟨t, ht, rfl⟩ := List.mem_map.mp hd
  rw [List.mem_range] at ht
  exact .deftype_case_0 _ _ (wf_trG hA _ (okG_closeGroup hw hn hf t (by omega)))

theorem clos_ex (hA : AtomsOK A) (hMR : MinusRecs) {m : Types.M} (hw : WF m) (hn : m.n < 2 ^ 32)
    (hf : ∀ t < m.n, (m.d t).fields.length < 2 ^ 32) :
    ∀ k ≤ m.n, fun_clos_deftypes (specTypes A m k) (closedTypes A m k)
  | 0, _ => by simp only [specTypes, closedTypes, List.range_zero, List.map_nil]; exact .fun_clos_deftypes_case_0
  | k + 1, hk => by
    rw [specTypes_succ, closedTypes_succ]
    have ih := clos_ex hA hMR hw hn hf k (by omega)
    have st := close_step_ex hA hMR hw hn hf (k := k) (by omega)
    exact .fun_clos_deftypes_case_1 _ _ _ _ _ _ _ ih st ih st rfl
      (.deftype_case_0 _ _ (wf_trG hA _ (okG_closeGroup hw hn hf k (by omega)))) (wf_closedTypes hA hw hn hf (by omega))

/-- What the spec makes of the types of a module: `$rollrt` rolls each rec group to `rolledG`, and
`$clos_deftypes` closes the types with their groups rolled to the closed forms of `dtOf`, both uniquely. -/
theorem dtOf_spec (hA : AtomsOK A) (hAd : AtomsDet A) (hMR : MinusRecs) {m : Types.M} (hw : WF m)
    (hn : m.n < 2 ^ 32) (hf : ∀ t < m.n, (m.d t).fields.length < 2 ^ 32) :
    (∀ t < m.n, ∀ r, fun_rollrt ⟨m.start t⟩ (rtM A m (m.start t)) r ↔ r = rolledG A m (m.start t)) ∧
    (∀ cl, fun_clos_deftypes (specTypes A m m.n) cl ↔ cl = (List.range m.n).map fun t => tD A (dtOf m t)) := by
  refine ⟨fun t ht r => ⟨rollrt_det hAd m _, fun e => ?_⟩,
    fun cl => ⟨clos_det hAd hMR hw m.n (Nat.le_refl _) cl,
    fun e => e ▸ clos_ex hA hMR hw hn hf m.n (Nat.le_refl _)⟩⟩
  subst e
  have hg := hw.group t ht
  have hsg := gsize_start m hw t ht
  refine rollrt_ex hA m _ (by omega) (fun k hk y hy => ?_) (fun k hk => hf _ (by omega))
  have htk : m.start t + k < m.n := by omega
  have := hw.refs _ htk y hy
  have := hw.group _ htk
  omega

/-! ### The atoms of WAH -/

theorem subst_abs_det {h : Abs} {tvs tus r} (hh : fun_subst_heaptype (trAbs h) tvs tus r) : r = trAbs h := by
  cases h <;> cases hh <;> rfl

theorem wahAtom_det : AtomsDet wahAtom where
  det a tvs tus r h := by
    revert h; unfold wahAtom; dsimp only
    repeat' split
    all_goals intro h
    all_goals first
      | (cases h <;> (rename_i hv; cases hv) <;> rfl)
      | (cases h with
          | fun_subst_storagetype_case_1 _ _ _ _ _ hv =>
            cases hv with
            | fun_subst_valtype_case_5 _ _ _ _ _ hr =>
              cases hr with
              | fun_subst_reftype_case_0 _ _ _ _ _ hh => rw [subst_abs_det hh]; rfl)
      | (cases h; rfl)
  detV a tvs tus r h := by
    revert h; unfold wahAtom; dsimp only
    repeat' split
    all_goals intro h
    all_goals simp only [toVal] at h
    all_goals first
      | (cases h; rfl)
      | (cases h with
          | fun_subst_valtype_case_5 _ _ _ _ _ hr =>
            cases hr with
            | fun_subst_reftype_case_0 _ _ _ _ _ hh => rw [subst_abs_det hh]; rfl)

end Wah.SpecTec
