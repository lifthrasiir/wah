/-
Subtyping across modules: `wah_cross_module_subtype`, which checks equivalence across modules with
`wah_cross_module_type_ref_eq` along the supertype chain, and uses `wah_type_is_subtype` for built-in types.
-/
import WahProofs.Cross
import WahProofs.Subtype

namespace Wah.CrossSub
open Wah.Types Wah.Subtype Wah.Cross

/-- wah_type_t as `VT` (as `decodeVT` in Oracle.lean). -/
def vtOf (t : Int) : VT := if t < 0 then .atom (t + 2 ^ 32).toNat else .ref (t / 2).toNat (t % 2 == 1)

/-- The supertype walk of `wah_cross_module_subtype` from type `t` of `ma`, for type `pi` of `mb`. -/
def walkX (c : Ctx) (pi : Nat) : Nat → Nat → Bool
  | 0, _ => false
  | f + 1, t => match (c.ma.d t).super with
    | none => false
    | some s => typeRefEq c (.ref s false) (.ref pi false) || walkX c pi f s

/-- `wah_cross_module_subtype` from `sub` of `ma` to `sup` of `mb`. -/
def crossSub (c : Ctx) (sub sup : Int) : Bool :=
  if (sub = sup ∧ c.same = true) ∨ sub = -99 then true
  else if sub % 2 = 1 ∧ sup % 2 ≠ 1 then false
  else if typeRefEq c (vtOf sub) (vtOf sup) then true
  else if sub < 0 ∨ sup < 0 then (if sub < 0 then wahSub c.mb c.cb sub sup else wahSub c.ma c.ca sub sup)
  else
    let a := sub - sub % 2
    let b := sup - sup % 2
    if (a ≠ sub ∨ b ≠ sup) ∧ typeRefEq c (vtOf a) (vtOf b) = true then true
    else walkX c (b / 2).toNat c.ma.n (a / 2).toNat

/-- Assumptions: valid modules with canonical maps like `canonical_map`, and rec groups of `ma` nested at most
`WAH_MAX_TYPE_RECURSION_DEPTH` deep (for completeness of `wah_cross_module_type_ref_eq`). -/
structure Good2 (c : Ctx) : Prop extends Good c where
  sa : SubWF c.ma
  sb : SubWF c.mb
  depth : ∀ x < c.ma.n, need c.ma c.ca (c.ma.start (c.ca x)) ≤ maxDepth

theorem builtin_ge : ∀ t ∈ builtinCodes, -99 ≤ t := by decide

theorem closeV_neg_indep (m m' : M) (t : Int) (h : t < 0) : closeV m t = closeV m' t := by
  unfold closeV; simp [show ¬(0 ≤ t) by omega]

theorem least_iff' (m : M) (canon : Nat → Nat) (hl : ∀ x < m.n, Least m canon x) :
    ∀ x y, x < m.n → y < m.n → (canon x = canon y ↔ dtOf m x = dtOf m y) :=
  fun x y hx hy => least_iff (hl x hx) (hl y hy)

/-- `wah_cross_module_type_ref_eq` between codes. -/
theorem tre_iff (c : Ctx) (hc : Good2 c) (a b : Int) (ha : ValidCode c.ma a) (hb : ValidCode c.mb b) :
    typeRefEq c (vtOf a) (vtOf b) = true ↔ SpecEq c (vtOf a) (vtOf b) := by
  have va : ∀ x n, vtOf a = .ref x n → x < c.ma.n := by
    intro x n e; unfold vtOf at e; split at e
    · cases e
    · simp at e; obtain ⟨e1, _⟩ := e
      rcases ha with h | h; exact absurd (builtin_neg _ h) (by omega); rw [← e1]; exact h.2
  have vb : ∀ y n, vtOf b = .ref y n → y < c.mb.n := by
    intro y n e; unfold vtOf at e; split at e
    · cases e
    · simp at e; obtain ⟨e1, _⟩ := e
      rcases hb with h | h; exact absurd (builtin_neg _ h) (by omega); rw [← e1]; exact h.2
  exact ⟨typeRefEq_sound c hc.toGood _ _ va vb,
    typeRefEq_complete c hc.toGood _ _ va vb (fun x n e => hc.depth x (va x n e))⟩

theorem walkX_iff (c : Ctx) (hc : Good2 c) (pi : Nat) (hpi : pi < c.mb.n) :
    ∀ f t, t < f → t < c.ma.n → (walkX c pi f t = true ↔
      ∃ s, (c.ma.d t).super = some s ∧ ∃ t', IChain c.ma s t' ∧ dtOf c.ma t' = dtOf c.mb pi)
  | 0, _, h, _ => absurd h (Nat.not_lt_zero _)
  | f + 1, t, hf, ht => by
    unfold walkX
    cases hs : (c.ma.d t).super with
    | none => simp
    | some st =>
      have hst := hc.sa.super_lt t ht st hs
      have hstn : st < c.ma.n := by omega
      have e := typeRefEq_sound c hc.toGood (.ref st false) (.ref pi false) (fun x n h => by cases h; exact hstn)
        (fun y n h => by cases h; exact hpi)
      have e' := typeRefEq_complete c hc.toGood (.ref st false) (.ref pi false) (fun x n h => by cases h; exact hstn)
        (fun y n h => by cases h; exact hpi) (fun x n h => by cases h; exact hc.depth st hstn)
      simp only [SpecEq, true_and] at e e'
      simp only [Bool.or_eq_true, walkX_iff c hc pi hpi f st (by omega) hstn, Option.some.injEq, exists_eq_left']
      constructor
      · rintro (h | ⟨s', hs', t', ch, h⟩)
        · exact ⟨st, .refl _, e h⟩
        · exact ⟨t', .step hs' ch, h⟩
      · rintro ⟨t', ch, h⟩
        rcases ichain_cases ch with rfl | ⟨s', hs', ch'⟩
        · exact .inl (e' h)
        · exact .inr ⟨s', hs', t', ch', h⟩

/-- The nullability of closed types. -/
theorem closeV_null (m : M) (t : Int) (ht : ValidCode m t) (hb : t ≠ -99) :
    (∃ h, closeV m t = .ref (t % 2 == 1) h) ∨ (closeV m t = .other t ∧ t % 2 = 0) := by
  unfold closeV
  split
  · left; exact ⟨_, rfl⟩
  · have hc : t ∈ builtinCodes := by rcases ht with h | h; exact h; omega
    simp only [builtinCodes, List.mem_cons, List.not_mem_nil, or_false] at hc
    rcases hc with rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl |
      rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl
    all_goals first | (exfalso; exact hb rfl) | simp [closeNeg, absOfCode, allAbs, absCode]

/-- `wah_cross_module_subtype` is the subtyping of the spec between types of different modules, unless rec groups
are nested too deeply. -/
theorem crossSub_iff (c : Ctx) (hc : Good2 c) (sub sup : Int) (hs : ValidCode c.ma sub) (hp : ValidCode c.mb sup) :
    crossSub c sub sup = true ↔ VSub (closeV c.ma sub) (closeV c.mb sup) := by
  have hca := least_iff' c.ma c.ca hc.la
  have hcb := least_iff' c.mb c.cb hc.lb
  have vs := validV_closeV c.ma hc.sa sub hs
  have vp := validV_closeV c.mb hc.sb sup hp
  have tre := tre_iff c hc sub sup hs hp
  unfold crossSub
  -- The first two checks
  by_cases h1 : (sub = sup ∧ c.same = true) ∨ sub = -99
  · simp only [h1, ite_true, true_iff]
    rcases h1 with ⟨rfl, hsm⟩ | rfl
    · obtain ⟨h2, _⟩ := hc.same hsm; rw [h2]; exact .refl _
    · unfold closeV; simp [closeNeg]; exact .bot _
  simp only [h1, ite_false]
  have hnb : sub ≠ -99 := fun h => h1 (.inr h)
  by_cases h2 : sub % 2 = 1 ∧ sup % 2 ≠ 1
  · simp only [show (sub % 2 = 1 ∧ sup % 2 ≠ 1) = True from propext ⟨fun _ => trivial, fun _ => h2⟩, ite_true,
      Bool.false_eq_true, false_iff]
    intro hv
    rw [vsub_iff vs vp] at hv
    rcases closeV_null c.ma sub hs hnb with ⟨h, e⟩ | ⟨_, e⟩
    · rw [e] at hv
      have hnp : sup ≠ -99 := by intro h; rw [h] at h2; simp at h2
      rcases closeV_null c.mb sup hp hnp with ⟨h', e'⟩ | ⟨e', _⟩
      · rw [e'] at hv; simp only [VAlgo] at hv
        have := hv.1 (by simp [h2.1]); simp only [beq_iff_eq] at this; exact h2.2 this
      · rw [e'] at hv; exact hv
    · omega
  simp only [h2, ite_false]
  -- Equivalent types
  by_cases h3 : typeRefEq c (vtOf sub) (vtOf sup) = true
  · simp only [h3, ite_true, true_iff]
    have e := tre.mp h3
    by_cases ha : sub < 0
    · by_cases hb : sup < 0
      · simp only [vtOf, ha, hb, ite_true, SpecEq] at e
        have l1 := builtin_ge sub (by rcases hs with h | h; exact h; omega)
        have l2 := builtin_ge sup (by rcases hp with h | h; exact h; omega)
        have : sub = sup := by omega
        subst this; rw [closeV_neg_indep c.ma c.mb sub ha]; exact .refl _
      · simp [vtOf, ha, hb, SpecEq] at e
    · by_cases hb : sup < 0
      · simp [vtOf, ha, hb, SpecEq] at e
      · simp only [vtOf, ha, hb, ite_false, SpecEq] at e
        have e' : closeV c.ma sub = closeV c.mb sup := by
          unfold closeV; simp only [show 0 ≤ sub by omega, show 0 ≤ sup by omega, ite_true, e.1, e.2]
        rw [e']; exact .refl _
  simp only [h3, Bool.false_eq_true, ite_false]
  -- Built-in types
  by_cases h4 : sub < 0 ∨ sup < 0
  · simp only [h4, ite_true]
    by_cases h5 : sub < 0
    · simp only [h5, ite_true]
      have hs' : ValidCode c.mb sub := by rcases hs with h | h; exact .inl h; omega
      rw [wahSub_iff c.mb c.cb hc.sb hcb sub sup hs' hp, closeV_neg_indep c.mb c.ma sub h5]
    · simp only [h5, ite_false]
      have h6 : sup < 0 := by omega
      have hp' : ValidCode c.ma sup := by rcases hp with h | h; exact .inl h; omega
      rw [wahSub_iff c.ma c.ca hc.sa hca sub sup hs hp', closeV_neg_indep c.ma c.mb sup h6]
  simp only [h4, ite_false]
  -- Type indices
  have h5 : 0 ≤ sub := by omega
  have h6 : 0 ≤ sup := by omega
  have hsn : (sub / 2).toNat < c.ma.n := by rcases hs with h | h; exact absurd (builtin_neg _ h) (by omega); exact h.2
  have hpn : (sup / 2).toNat < c.mb.n := by rcases hp with h | h; exact absurd (builtin_neg _ h) (by omega); exact h.2
  have e1 : (sub - sub % 2) / 2 = sub / 2 := by omega
  have e2 : (sup - sup % 2) / 2 = sup / 2 := by omega
  have tnn := tre_iff c hc (sub - sub % 2) (sup - sup % 2) (.inr ⟨by omega, by rw [e1]; exact hsn⟩)
    (.inr ⟨by omega, by rw [e2]; exact hpn⟩)
  have hw := walkX_iff c hc _ hpn _ _ hsn hsn
  rw [e1, e2]
  rw [vsub_iff vs vp]
  unfold closeV; simp only [h5, h6, ite_true, VAlgo, Algo]
  rw [chain_dtOf c.ma hc.sa _ hsn]
  have hnull : (sub % 2 == 1) = true → (sup % 2 == 1) = true := by
    intro h; simp only [beq_iff_eq] at h ⊢; omega
  -- Equivalent or a supertype is
  have key : ((∃ t', IChain c.ma (sub / 2).toNat t' ∧ dtOf c.ma t' = dtOf c.mb (sup / 2).toNat) ↔
      (dtOf c.ma (sub / 2).toNat = dtOf c.mb (sup / 2).toNat ∨ ∃ s, (c.ma.d (sub / 2).toNat).super = some s ∧
        ∃ t', IChain c.ma s t' ∧ dtOf c.ma t' = dtOf c.mb (sup / 2).toNat)) := by
    constructor
    · rintro ⟨t', ch, e⟩
      rcases ichain_cases ch with rfl | ⟨s', hs', ch'⟩
      · exact .inl e
      · exact .inr ⟨s', hs', t', ch', e⟩
    · rintro (e | ⟨s', hs', t', ch', e⟩)
      · exact ⟨_, .refl _, e⟩
      · exact ⟨t', .step hs' ch', e⟩
  rw [key]
  split
  · rename_i hcond
    have := tnn.mp hcond.2
    simp only [vtOf, show ¬(sub - sub % 2 < 0) by omega, show ¬(sup - sup % 2 < 0) by omega, ite_false, e1, e2,
      SpecEq] at this
    exact ⟨fun _ => ⟨hnull, .inl this.2⟩, fun _ => rfl⟩
  · rename_i hcond
    rw [hw]
    constructor
    · intro w; exact ⟨hnull, .inr w⟩
    · rintro ⟨_, he | w⟩
      · exfalso
        by_cases hnn : sub - sub % 2 ≠ sub ∨ sup - sup % 2 ≠ sup
        · apply hcond; refine ⟨hnn, tnn.mpr ?_⟩
          simp only [vtOf, show ¬(sub - sub % 2 < 0) by omega, show ¬(sup - sup % 2 < 0) by omega, ite_false, e1, e2,
            SpecEq]
          refine ⟨?_, he⟩; congr 1; omega
        · apply h3; apply tre.mpr
          simp only [vtOf, show ¬(sub < 0) by omega, show ¬(sup < 0) by omega, ite_false, SpecEq]
          simp only [not_or, Decidable.not_not] at hnn
          refine ⟨?_, he⟩; congr 1; omega
      · exact w

end Wah.CrossSub
