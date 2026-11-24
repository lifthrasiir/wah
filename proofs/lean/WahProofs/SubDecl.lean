/-
Validation of declared supertypes in `wah_module_build_type_metadata`: a type is checked against its supertype by
the composite type subtyping of the spec, where mutable fields must be equivalent.
-/
import WahProofs.Subtype

namespace Wah.SubDecl
open Wah.Types Wah.Subtype

/-! ### Antisymmetry -/

/-- Supertype depth by index. -/
def idepth (m : M) (t : Nat) : Nat :=
  match h : (m.d t).super with
  | none => 0
  | some s => if s < t then idepth m s + 1 else 0
termination_by t
decreasing_by omega

theorem idepth_super (m : M) (hw : SubWF m) (t s : Nat) (ht : t < m.n) (hs : (m.d t).super = some s) :
    idepth m t = idepth m s + 1 := by
  rw [idepth]; split
  · rename_i h; rw [hs] at h; cases h
  · rename_i s' h; rw [hs] at h; cases h; simp [hw.super_lt t ht s hs]

theorem idepth_none (m : M) (t : Nat) (hs : (m.d t).super = none) : idepth m t = 0 := by
  rw [idepth]; split
  · rfl
  · rename_i s' h; rw [hs] at h; cases h

/-- Equivalent types have the same depth. -/
theorem idepth_eq (m : M) (hw : SubWF m) : ∀ N t u, t + u < N → t < m.n → u < m.n →
    dtOf m t = dtOf m u → idepth m t = idepth m u
  | 0, _, _, h, _, _, _ => absurd h (Nat.not_lt_zero _)
  | N + 1, t, u, hN, ht, hu, e => by
    have st := superOf_dtOf m hw.toWF t ht; have su := superOf_dtOf m hw.toWF u hu
    rw [e, su] at st
    cases hs : (m.d t).super <;> cases hs' : (m.d u).super <;> simp [hs, hs'] at st
    · rw [idepth_none m t hs, idepth_none m u hs']
    · rename_i s s'
      have h1 := hw.super_lt t ht s hs; have h2 := hw.super_lt u hu s' hs'
      rw [idepth_super m hw t s ht hs, idepth_super m hw u s' hu hs',
        idepth_eq m hw N s s' (by omega) (by omega) (by omega) st.symm]

theorem ichain_depth (m : M) (hw : SubWF m) {t t' : Nat} (c : IChain m t t') (ht : t < m.n) :
    t' = t ∨ idepth m t' < idepth m t := by
  induction c with
  | refl => left; rfl
  | @step t s t' hs _ ih =>
    right
    have hsn : s < m.n := by have := hw.super_lt t ht s hs; omega
    rw [idepth_super m hw t s ht hs]
    rcases ih hsn with rfl | h <;> omega

theorem ichain_depth_le (m : M) (hw : SubWF m) {t t' : Nat} (c : IChain m t t') (ht : t < m.n) :
    idepth m t' ≤ idepth m t := by
  rcases ichain_depth m hw c ht with rfl | h <;> omega

/-- Supertype chains of module types are acyclic up to equivalence. -/
theorem chain_antisymm (m : M) (hw : SubWF m) (t u : Nat) (ht : t < m.n) (_hu : u < m.n)
    (h1 : Chain (dtOf m t) (dtOf m u)) (h2 : Chain (dtOf m u) (dtOf m t)) : dtOf m t = dtOf m u := by
  obtain ⟨t1, c1, e1⟩ := (chain_dtOf m hw t ht _).mp h1
  have ht1 := ichain_lt hw c1 ht
  rw [← e1] at h2
  obtain ⟨t2, c2, e2⟩ := (chain_dtOf m hw t1 ht1 _).mp h2
  have ht2 := ichain_lt hw c2 ht1
  have d1 := ichain_depth_le m hw c2 ht1
  have d2 := idepth_eq m hw (t2 + t + 1) t2 t (by omega) ht2 ht e2
  rcases ichain_depth m hw c1 ht with h | h
  · subst h; exact e1
  · omega

theorem absSub_antisymm (a b : Abs) : absSub a b = true → absSub b a = true → a = b := by
  cases a <;> cases b <;> decide

/-- A code of the module or a built-in type. -/
def closeVM (m : M) (t : Int) : CV := closeV m t

/-- Mutual subtyping is equality for types of the module. -/
theorem valgo_antisymm (m : M) (hw : SubWF m) (a b : Int) (ha : ValidCode m a) (hb : ValidCode m b)
    (h1 : VAlgo (closeV m a) (closeV m b)) (h2 : VAlgo (closeV m b) (closeV m a)) : closeV m a = closeV m b := by
  generalize ea : closeV m a = va at h1 h2
  generalize eb : closeV m b = vb at h1 h2
  have ia : ∀ n d, va = .ref n (.defd d) → ∃ t < m.n, d = dtOf m t := by
    intro n d e; rw [← ea] at e; unfold closeV at e; split at e
    · rcases ha with h | ⟨_, hn⟩
      · have := builtin_neg a h; omega
      · simp at e; exact ⟨_, hn, e.2.symm⟩
    · unfold closeNeg at e; split at e
      · cases e
      · split at e <;> simp at e
  have ib : ∀ n d, vb = .ref n (.defd d) → ∃ t < m.n, d = dtOf m t := by
    intro n d e; rw [← eb] at e; unfold closeV at e; split at e
    · rcases hb with h | ⟨_, hn⟩
      · have := builtin_neg b h; omega
      · simp at e; exact ⟨_, hn, e.2.symm⟩
    · unfold closeNeg at e; split at e
      · cases e
      · split at e <;> simp at e
  cases va with
  | other x => simp only [VAlgo] at h1; exact h1.symm
  | bot => cases vb with
    | bot => rfl
    | other _ => simp only [VAlgo] at h2; cases h2
    | ref _ _ => simp only [VAlgo] at h2
  | ref n1 x => cases vb with
    | bot => simp only [VAlgo] at h1
    | other _ => simp only [VAlgo] at h1
    | ref n2 y =>
      simp only [VAlgo] at h1 h2
      have hn : n1 = n2 := by cases n1 <;> cases n2 <;> simp_all
      subst hn
      cases x <;> cases y <;> simp only [Algo] at h1 h2
      · rename_i a' b'; rw [absSub_antisymm a' b' h1.2 h2.2]
      · rename_i a' d; cases a' <;> simp [absDef, defAbs] at h1 h2 <;> omega
      · rename_i d a'; cases a' <;> simp [absDef, defAbs] at h1 h2 <;> omega
      · rename_i d d'
        obtain ⟨t, ht, rfl⟩ := ia n1 d rfl; obtain ⟨u, hu, rfl⟩ := ib n1 d' rfl
        rw [chain_antisymm m hw t u ht hu h1.2 h2.2]

/-- Inverse of `closeNeg` on built-in codes. -/
def codeOfNeg : CV → Int
  | .bot => -99
  | .other t => t
  | .ref n (.abs h) => absCode h + if n then 1 else 0
  | .ref _ (.defd _) => 0

theorem codeOfNeg_closeNeg : builtinCodes.all (fun a => codeOfNeg (closeNeg a) == a) = true := by decide

theorem closeNeg_inj (a b : Int) (ha : a ∈ builtinCodes) (hb : b ∈ builtinCodes) (e : closeNeg a = closeNeg b) :
    a = b := by
  have h := codeOfNeg_closeNeg; simp only [List.all_eq_true, beq_iff_eq] at h
  rw [← h a ha, ← h b hb, e]

theorem closeNeg_not_defd (a : Int) (n : Bool) (d : DT) : closeNeg a ≠ .ref n (.defd d) := by
  unfold closeNeg; split
  · simp
  · split <;> simp

/-- The check of mutable fields in `wah_module_build_type_metadata`. -/
def mutEq (canon : Nat → Nat) (st tt : Int) : Bool :=
  st == tt || (0 ≤ st && 0 ≤ tt && st % 2 == tt % 2 && canon (st / 2).toNat == canon (tt / 2).toNat)

theorem mutEq_iff (m : M) (canon : Nat → Nat) (hw : SubWF m)
    (hc : ∀ x y, x < m.n → y < m.n → (canon x = canon y ↔ dtOf m x = dtOf m y))
    (a b : Int) (ha : ValidCode m a) (hb : ValidCode m b) :
    mutEq canon a b = true ↔ (VSub (closeV m a) (closeV m b) ∧ VSub (closeV m b) (closeV m a)) := by
  have va := validV_closeV m hw a ha; have vb := validV_closeV m hw b hb
  rw [vsub_iff va vb, vsub_iff vb va]
  constructor
  · intro h
    have e : closeV m a = closeV m b := by
      unfold mutEq at h
      simp only [Bool.or_eq_true, beq_iff_eq, Bool.and_eq_true, decide_eq_true_eq] at h
      rcases h with rfl | ⟨⟨⟨h1, h2⟩, h3⟩, h4⟩
      · rfl
      · have hna : (a / 2).toNat < m.n := by rcases ha with h | h; exact absurd (builtin_neg _ h) (by omega); exact h.2
        have hnb : (b / 2).toNat < m.n := by rcases hb with h | h; exact absurd (builtin_neg _ h) (by omega); exact h.2
        unfold closeV; simp only [h1, h2, ite_true, h3, (hc _ _ hna hnb).mp h4]
    rw [e]; exact ⟨(vsub_iff vb vb).mp (.refl _), (vsub_iff vb vb).mp (.refl _)⟩
  · rintro ⟨h1, h2⟩
    have e := valgo_antisymm m hw a b ha hb h1 h2
    unfold mutEq
    simp only [Bool.or_eq_true, beq_iff_eq, Bool.and_eq_true, decide_eq_true_eq]
    unfold closeV at e
    by_cases h1 : 0 ≤ a <;> by_cases h2 : 0 ≤ b <;> simp only [h1, h2, ite_true, ite_false] at e
    · simp only [CV.ref.injEq, CH.defd.injEq] at e
      have hna : (a / 2).toNat < m.n := by rcases ha with h | h; exact absurd (builtin_neg _ h) (by omega); exact h.2
      have hnb : (b / 2).toNat < m.n := by rcases hb with h | h; exact absurd (builtin_neg _ h) (by omega); exact h.2
      right; refine ⟨⟨⟨h1, h2⟩, ?_⟩, (hc _ _ hna hnb).mpr e.2⟩
      have e1 := e.1
      rcases (show a % 2 = 0 ∨ a % 2 = 1 by omega) with x | x <;>
        rcases (show b % 2 = 0 ∨ b % 2 = 1 by omega) with y | y <;> simp [x, y] at e1 ⊢
    · exact absurd e.symm (closeNeg_not_defd _ _ _)
    · exact absurd e (closeNeg_not_defd _ _ _)
    · left
      exact closeNeg_inj a b (by rcases ha with h | h; exact h; omega) (by rcases hb with h | h; exact h; omega) e

/-! ### Declared supertypes -/

/-- `wah_type_t` of a value type (the inverse of `decodeVT` in Oracle.lean). -/
def vtCode : VT → Int
  | .atom a => (a : Int) - 2 ^ 32
  | .ref x null => 2 * (x : Int) + if null then 1 else 0

def fcode (td : TD) (j : Nat) : Int := (td.fields[j]?.map fun f => vtCode f.1).getD 0
def fmut (td : TD) (j : Nat) : Bool := (td.fields[j]?.map (·.2)).getD false

/-- Field `j` of the sub against field `j` of the super, as in `wah_module_build_type_metadata`. -/
def fieldCheck (m : M) (canon : Nat → Nat) (a b : TD) (j : Nat) : Bool :=
  if fmut b j then fmut a j && mutEq canon (fcode b j) (fcode a j)
  else !fmut a j && wahSub m canon (fcode a j) (fcode b j)

/-- The check of a type `t` against its supertype `s` in `wah_module_build_type_metadata`. -/
def declCheck (m : M) (canon : Nat → Nat) (t s : Nat) : Bool :=
  let a := m.d t
  let b := m.d s
  if a.kind = kFunc then
    a.counts.1 == b.counts.1 && a.counts.2 == b.counts.2 &&
    (List.range a.counts.1).all (fun j => wahSub m canon (fcode b j) (fcode a j)) &&
    (List.range a.counts.2).all (fun j => wahSub m canon (fcode a (a.counts.1 + j)) (fcode b (a.counts.1 + j)))
  else if a.kind = kStruct then
    decide (b.fields.length ≤ a.fields.length) && (List.range b.fields.length).all (fieldCheck m canon a b)
  else if a.kind = kArray then
    a.fields.length == 1 && b.fields.length == 1 && fieldCheck m canon a b 0
  else true

/-- Composite type subtyping of the spec, between the declarations of `t` and `s`: functions are contravariant in
params and covariant in results, and structs and arrays have more fields, which are covariant if immutable and
equivalent if mutable. -/
def CompSub (m : M) (t s : Nat) : Prop :=
  let a := m.d t
  let b := m.d s
  let cv := fun (td : TD) j => closeV m (fcode td j)
  if a.kind = kFunc then
    a.counts = b.counts ∧ (∀ j < a.counts.1, VSub (cv b j) (cv a j)) ∧
    (∀ j < a.counts.2, VSub (cv a (a.counts.1 + j)) (cv b (a.counts.1 + j)))
  else
    b.fields.length ≤ a.fields.length ∧
    ∀ j < b.fields.length, fmut a j = fmut b j ∧ VSub (cv a j) (cv b j) ∧ (fmut b j = true → VSub (cv b j) (cv a j))

/-- The shape of `TD` for each kind, which is a function, a struct or an array. -/
def Shape (td : TD) : Prop :=
  if td.kind = kFunc then td.fields.length = td.counts.1 + td.counts.2
  else if td.kind = kArray then td.fields.length = 1 else td.kind = kStruct

/-- Field types are valid. -/
def FieldsOK (m : M) (td : TD) : Prop := ∀ j < td.fields.length, ValidCode m (fcode td j)

theorem fieldCheck_iff (m : M) (canon : Nat → Nat) (hw : SubWF m)
    (hc : ∀ x y, x < m.n → y < m.n → (canon x = canon y ↔ dtOf m x = dtOf m y))
    (a b : TD) (j : Nat) (ha : ValidCode m (fcode a j)) (hb : ValidCode m (fcode b j)) :
    fieldCheck m canon a b j = true ↔ (fmut a j = fmut b j ∧ VSub (closeV m (fcode a j)) (closeV m (fcode b j)) ∧
      (fmut b j = true → VSub (closeV m (fcode b j)) (closeV m (fcode a j)))) := by
  unfold fieldCheck
  cases hb' : fmut b j <;> cases ha' : fmut a j <;>
    simp [wahSub_iff m canon hw hc _ _ ha hb, mutEq_iff m canon hw hc _ _ hb ha]
  · exact ⟨fun ⟨h1, h2⟩ => ⟨h2, h1⟩, fun ⟨h1, h2⟩ => ⟨h2, h1⟩⟩

/-- `wah_module_build_type_metadata` accepts a declared supertype exactly when the spec does. -/
theorem declCheck_iff (m : M) (canon : Nat → Nat) (hw : SubWF m)
    (hc : ∀ x y, x < m.n → y < m.n → (canon x = canon y ↔ dtOf m x = dtOf m y))
    (t s : Nat) (ht : t < m.n) (hs : (m.d t).super = some s)
    (sa : Shape (m.d t)) (sb : Shape (m.d s)) (fa : FieldsOK m (m.d t)) (fb : FieldsOK m (m.d s)) :
    declCheck m canon t s = true ↔ CompSub m t s := by
  have hk := hw.super_kind t ht s hs
  have shF : (m.d t).kind = 96 → (m.d t).fields.length = (m.d t).counts.1 + (m.d t).counts.2 ∧
      (m.d s).fields.length = (m.d s).counts.1 + (m.d s).counts.2 := by
    intro h; unfold Shape at sa sb; rw [hk] at sb; simp only [kFunc, h, ite_true] at sa sb; exact ⟨sa, sb⟩
  have shA : (m.d t).kind = 94 → (m.d t).fields.length = 1 ∧ (m.d s).fields.length = 1 := by
    intro h; unfold Shape at sa sb; rw [hk] at sb; simp only [kFunc, kArray, h, ite_true] at sa sb; exact ⟨sa, sb⟩
  have shS : (m.d t).kind = 96 ∨ (m.d t).kind = 95 ∨ (m.d t).kind = 94 := by
    unfold Shape at sa; simp only [kFunc, kArray, kStruct] at sa
    by_cases h1 : (m.d t).kind = 96
    · exact .inl h1
    · by_cases h2 : (m.d t).kind = 94
      · exact .inr (.inr h2)
      · simp only [h1, h2, ite_false] at sa; exact .inr (.inl sa)
  clear sa sb
  unfold declCheck CompSub
  simp only [kFunc, kStruct, kArray]
  by_cases hf : (m.d t).kind = 96
  · obtain ⟨sa, sb⟩ := shF hf
    simp only [hf, ite_true]
    simp only [Bool.and_eq_true, beq_iff_eq, List.all_eq_true, List.mem_range]
    constructor
    · rintro ⟨⟨⟨e1, e2⟩, h1⟩, h2⟩
      refine ⟨Prod.ext e1 e2, fun j hj => ?_, fun j hj => ?_⟩
      · exact (wahSub_iff m canon hw hc _ _ (fb j (by omega)) (fa j (by omega))).mp (h1 j hj)
      · exact (wahSub_iff m canon hw hc _ _ (fa _ (by omega)) (fb _ (by omega))).mp (h2 j hj)
    · rintro ⟨e, h1, h2⟩
      have e1 : (m.d t).counts.1 = (m.d s).counts.1 := by rw [e]
      have e2 : (m.d t).counts.2 = (m.d s).counts.2 := by rw [e]
      refine ⟨⟨⟨e1, e2⟩, fun j hj => ?_⟩, fun j hj => ?_⟩
      · exact (wahSub_iff m canon hw hc _ _ (fb j (by omega)) (fa j (by omega))).mpr (h1 j hj)
      · exact (wahSub_iff m canon hw hc _ _ (fa _ (by omega)) (fb _ (by omega))).mpr (h2 j hj)
  · simp only [hf, ite_false]
    by_cases hst : (m.d t).kind = 95
    · simp only [hst, ite_true, Bool.and_eq_true, decide_eq_true_eq, List.all_eq_true, List.mem_range]
      constructor
      · rintro ⟨hl, h⟩
        exact ⟨hl, fun j hj => (fieldCheck_iff m canon hw hc _ _ j (fa j (by omega)) (fb j hj)).mp (h j hj)⟩
      · rintro ⟨hl, h⟩
        exact ⟨hl, fun j hj => (fieldCheck_iff m canon hw hc _ _ j (fa j (by omega)) (fb j hj)).mpr (h j hj)⟩
    · have ha : (m.d t).kind = 94 := by omega
      obtain ⟨sa, sb⟩ := shA ha
      simp only [ha, ite_true, sa, sb, true_and,
        Nat.le_refl, Nat.lt_one_iff, forall_eq]
      exact fieldCheck_iff m canon hw hc _ _ 0 (fa 0 (by omega)) (fb 0 (by omega))

end Wah.SubDecl
