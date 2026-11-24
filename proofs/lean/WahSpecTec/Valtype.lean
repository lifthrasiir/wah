/-
Value subtyping of WahProofs/Subtype.lean (`VSub`, over closed value types `CV`) agrees with `Valtype_sub` of the
SpecTec model, for closed types in the empty context. Numbers and vectors are their `wah_type_t` codes in `CV`.
-/
import WahSpecTec.SubtypeComplete

set_option linter.unusedSectionVars false

namespace Wah.SpecTec
open Wah.Types Wah.Subtype

/-- Number and vector types by their `wah_type_t`. -/
def numOf (t : Int) : Option valtype :=
  if t = -2 then some .I32 else if t = -4 then some .I64 else if t = -6 then some .F32
  else if t = -8 then some .F64 else if t = -10 then some .V128 else none

def trCV (A : Nat → storagetype) : CV → valtype
  | .other t => (numOf t).getD .BOT
  | .ref n h => .REF (nullOpt n) (trH A h)
  | .bot => .BOT

/-- Closed value types that translate: numbers and vectors, references to types that translate, and bottom. -/
def OkCV (A : Nat → storagetype) : CV → Prop
  | .other t => (numOf t).isSome
  | .ref _ h => OkH h ∧ ShapeH A h
  | .bot => True

theorem numOf_cases {t : Int} {x : valtype} (h : numOf t = some x) :
    (t = -2 ∧ x = .I32) ∨ (t = -4 ∧ x = .I64) ∨ (t = -6 ∧ x = .F32) ∨ (t = -8 ∧ x = .F64) ∨
      (t = -10 ∧ x = .V128) := by
  unfold numOf at h
  repeat' split at h
  all_goals simp_all

variable {A : Nat → storagetype}

theorem wf_trH (hA : AtomsOK A) {h : CH} (hh : OkH h) : wf_heaptype (trH A h) := by
  cases h with
  | abs x => exact wf_trAbs x
  | defd d => exact wf_trD hA hh

theorem wf_trCV (hA : AtomsOK A) {v : CV} (hv : OkCV A v) : wf_valtype (trCV A v) := by
  cases v with
  | other t =>
    obtain ⟨x, hx⟩ := Option.isSome_iff_exists.mp hv
    simp only [trCV, hx, Option.getD_some]
    rcases numOf_cases hx with ⟨-, rfl⟩ | ⟨-, rfl⟩ | ⟨-, rfl⟩ | ⟨-, rfl⟩ | ⟨-, rfl⟩ <;> constructor
  | ref n h => exact .valtype_case_5 _ _ (wf_trH hA hv.1)
  | bot => exact .valtype_case_6

theorem valtype_refl (hA : AtomsOK A) {C : context} (hC : wf_context C) {v : CV} (hv : OkCV A v) :
    Valtype_sub C (trCV A v) (trCV A v) := by
  cases v with
  | other t =>
    obtain ⟨x, hx⟩ := Option.isSome_iff_exists.mp hv
    simp only [trCV, hx, Option.getD_some]
    rcases numOf_cases hx with ⟨-, rfl⟩ | ⟨-, rfl⟩ | ⟨-, rfl⟩ | ⟨-, rfl⟩ | ⟨-, rfl⟩
    · exact .num C .I32 .I32 (.mk_Numtype_sub _ _ hC) hC
    · exact .num C .I64 .I64 (.mk_Numtype_sub _ _ hC) hC
    · exact .num C .F32 .F32 (.mk_Numtype_sub _ _ hC) hC
    · exact .num C .F64 .F64 (.mk_Numtype_sub _ _ hC) hC
    · exact .vec C .V128 .V128 (.mk_Vectype_sub _ _ hC) hC
  | ref n h =>
    have wh := wf_trH hA hv.1
    have hs : Heaptype_sub C (trH A h) (trH A h) := .refl C _ hC wh
    have r1 := wf_reftype.reftype_case_0 none _ wh
    have r2 := wf_reftype.reftype_case_0 (some .NULL) _ wh
    cases n with
    | false => exact .ref C _ _ (.nonnull C _ _ hs hC r1 r1) hC r1 r1
    | true => exact .ref C _ _ (.null C _ _ _ hs (.inr rfl) hC r2 r2) hC r2 r2
  | bot => exact .bot C _ hC .valtype_case_6 .valtype_case_6

/-- Value subtyping of WahProofs/Subtype.lean implies `Valtype_sub` of the spec, in any context. -/
theorem vsub_sound (hA : AtomsOK A) {C : context} (hC : wf_context C) (hMR : MinusRecs) {v w : CV} (hv : OkCV A v)
    (hw : OkCV A w) (vv : ValidV v)
    (h : VSub v w) : Valtype_sub C (trCV A v) (trCV A w) := by
  cases h with
  | refl => exact valtype_refl hA hC hv
  | bot => exact .bot C _ hC (wf_trCV hA hw) .valtype_case_6
  | @ref n1 n2 h1 h2 hs hn =>
    have w1 := wf_trH hA hv.1
    have w2 := wf_trH hA hw.1
    have s := hsub_sound hA hC hMR hv.1 hw.1 vv hs
    cases n1 <;> cases n2
    · exact .ref C _ _ (.nonnull C _ _ s hC (.reftype_case_0 _ _ w1) (.reftype_case_0 _ _ w2)) hC
        (.reftype_case_0 _ _ w1) (.reftype_case_0 _ _ w2)
    · exact .ref C _ _ (.null C _ _ _ s (.inl rfl) hC (.reftype_case_0 _ _ w1) (.reftype_case_0 _ _ w2))
        hC (.reftype_case_0 _ _ w1) (.reftype_case_0 _ _ w2)
    · exact absurd (hn rfl) (by simp)
    · exact .ref C _ _ (.null C _ _ _ s (.inr rfl) hC (.reftype_case_0 _ _ w1) (.reftype_case_0 _ _ w2))
        hC (.reftype_case_0 _ _ w1) (.reftype_case_0 _ _ w2)

theorem trCV_num {v : CV} (hv : OkCV A v) {x : valtype} (hx : trCV A v = x) (hn : ∀ n h, x ≠ .REF n h)
    (hb : x ≠ .BOT) : ∃ t, v = .other t ∧ numOf t = some x := by
  cases v with
  | other t =>
    obtain ⟨y, hy⟩ := Option.isSome_iff_exists.mp hv
    simp only [trCV, hy, Option.getD_some] at hx
    exact ⟨t, rfl, hx ▸ hy⟩
  | ref n h => simp only [trCV] at hx; exact absurd hx.symm (hn _ _)
  | bot => simp only [trCV] at hx; exact absurd hx.symm hb

theorem numOf_inj {t t' : Int} {x : valtype} (h : numOf t = some x) (h' : numOf t' = some x) : t = t' := by
  rcases numOf_cases h with ⟨rfl, rfl⟩ | ⟨rfl, rfl⟩ | ⟨rfl, rfl⟩ | ⟨rfl, rfl⟩ | ⟨rfl, rfl⟩ <;>
  rcases numOf_cases h' with ⟨rfl, h2⟩ | ⟨rfl, h2⟩ | ⟨rfl, h2⟩ | ⟨rfl, h2⟩ | ⟨rfl, h2⟩ <;> simp_all

theorem trCV_ref {v : CV} {n : Option null} {ht : heaptype} (hv : OkCV A v) (hx : trCV A v = .REF n ht) :
    ∃ b h, v = .ref b h ∧ n = nullOpt b ∧ ht = trH A h := by
  cases v with
  | other t =>
    obtain ⟨y, hy⟩ := Option.isSome_iff_exists.mp hv
    simp only [trCV, hy, Option.getD_some] at hx
    rcases numOf_cases hy with ⟨-, rfl⟩ | ⟨-, rfl⟩ | ⟨-, rfl⟩ | ⟨-, rfl⟩ | ⟨-, rfl⟩ <;> cases hx
  | ref b h => simp only [trCV, valtype.REF.injEq] at hx; exact ⟨b, h, rfl, hx.1.symm, hx.2.symm⟩
  | bot => simp [trCV] at hx

/-- `Valtype_sub` of the spec implies value subtyping of WahProofs/Subtype.lean. -/
theorem vsub_complete (hMR : MinusRecs) {v w : CV} (hv : OkCV A v) (hw : OkCV A w) (vv : ValidV v) (vw : ValidV w)
    (h : Valtype_sub C0 (trCV A v) (trCV A w)) : VSub v w := by
  generalize ex : trCV A v = x at h
  generalize ey : trCV A w = y at h
  cases h with
  | num _ n1 n2 hn _ =>
    cases hn
    obtain ⟨t, rfl, ht⟩ := trCV_num hv ex (by cases n1 <;> simp [valtype_numtype]) (by cases n1 <;>
        simp [valtype_numtype])
    obtain ⟨t', rfl, ht'⟩ := trCV_num hw ey (by cases n1 <;> simp [valtype_numtype]) (by cases n1 <;>
        simp [valtype_numtype])
    rw [numOf_inj ht ht']; exact .refl _
  | vec _ v1 v2 hn _ =>
    cases hn
    obtain ⟨t, rfl, ht⟩ :=
      trCV_num hv ex (by cases v1; simp [valtype_vectype]) (by cases v1; simp [valtype_vectype])
    obtain ⟨t', rfl, ht'⟩ :=
      trCV_num hw ey (by cases v1; simp [valtype_vectype]) (by cases v1; simp [valtype_vectype])
    rw [numOf_inj ht ht']; exact .refl _
  | ref _ r1 r2 hr _ _ _ =>
    cases hr with
    | nonnull _ h1 h2 hs _ _ _ =>
      obtain ⟨b1, k1, rfl, hn1, rfl⟩ := trCV_ref hv ex
      obtain ⟨b2, k2, rfl, hn2, rfl⟩ := trCV_ref hw ey
      have e1 : b1 = false := by cases b1 <;> simp [nullOpt] at hn1 ⊢
      have e2 : b2 = false := by cases b2 <;> simp [nullOpt] at hn2 ⊢
      exact .ref (hsub_complete hMR hv.1 hw.1 hv.2 hw.2 vv vw hs) (by simp [e1, e2])
    | null _ h1 h2 o hs _ _ _ _ =>
      obtain ⟨b1, k1, rfl, hn1, rfl⟩ := trCV_ref hv ex
      obtain ⟨b2, k2, rfl, hn2, rfl⟩ := trCV_ref hw ey
      have e2 : b2 = true := by cases b2 <;> simp [nullOpt] at hn2 ⊢
      exact .ref (hsub_complete hMR hv.1 hw.1 hv.2 hw.2 vv vw hs) (fun _ => e2)
  | bot _ _ _ _ _ =>
    cases v with
    | other t =>
      obtain ⟨z, hz⟩ := Option.isSome_iff_exists.mp hv
      simp only [trCV, hz, Option.getD_some] at ex
      rcases numOf_cases hz with ⟨-, rfl⟩ | ⟨-, rfl⟩ | ⟨-, rfl⟩ | ⟨-, rfl⟩ | ⟨-, rfl⟩ <;> cases ex
    | ref b h => simp [trCV] at ex
    | bot => exact .bot _

/-- Both directions. -/
theorem vsub_iff_spec (hA : AtomsOK A) (hMR : MinusRecs) {v w : CV} (hv : OkCV A v) (hw : OkCV A w)
    (vv : ValidV v) (vw : ValidV w) : VSub v w ↔ Valtype_sub C0 (trCV A v) (trCV A w) :=
  ⟨vsub_sound hA wfC0 hMR hv hw vv, vsub_complete hMR hv hw vv vw⟩

end Wah.SpecTec
