/-
Struct layouts of subtypes: fields of a declared supertype are laid out as in the supertype, so the check of
`wah_check_layouts` that subtypes agree with their supertypes on common fields always succeeds.
-/
import WahProofs.Layout
import WahProofs.SubDecl

namespace Wah.LayoutSub
open Wah.Types Wah.Subtype Wah.SubDecl

/-- The size class of a storage type in `wah_field_type_layout` (2^k bytes; references are 8 bytes). -/
def fieldClass (t : Int) : Nat :=
  if t = -16 then 0 else if t = -18 then 1 else if t = -2 ∨ t = -6 then 2 else if t = -10 then 4 else 3

/-- Storage types: anything valid except the bottom type of validation. -/
def StorageCode (m : M) (t : Int) : Prop := ValidCode m t ∧ t ≠ -99

theorem closeV_ref_class (m : M) (t : Int) (ht : StorageCode m t) (n : Bool) (h : CH)
    (e : closeV m t = .ref n h) : fieldClass t = 3 := by
  unfold closeV at e
  split at e
  · unfold fieldClass; split <;> (try split) <;> (try split) <;> (try split) <;> omega
  · rename_i hneg
    have hb : t ∈ builtinCodes := by rcases ht.1 with h | h; exact h; omega
    have := ht.2
    simp only [builtinCodes, List.mem_cons, List.not_mem_nil, or_false] at hb
    rcases hb with rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl |
      rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl
    all_goals first | rfl | simp [closeNeg, absOfCode, allAbs, absCode] at e | omega

/-- Subtypes of storage types have the same size class. -/
theorem vsub_class (m : M) (a b : Int) (ha : StorageCode m a) (hb : StorageCode m b)
    (h : VSub (closeV m a) (closeV m b)) : fieldClass a = fieldClass b := by
  generalize ea : closeV m a = va at h
  generalize eb : closeV m b = vb at h
  cases h with
  | refl v =>
    subst eb
    by_cases h1 : 0 ≤ a <;> by_cases h2 : 0 ≤ b
    · unfold fieldClass; simp only [show a ≠ -16 by omega, show a ≠ -18 by omega, show ¬(a = -2 ∨ a = -6) by omega,
        show a ≠ -10 by omega, show b ≠ -16 by omega, show b ≠ -18 by omega, show ¬(b = -2 ∨ b = -6) by omega,
        show b ≠ -10 by omega, ite_false]
    · unfold closeV at ea; simp only [h1, h2, ite_true, ite_false] at ea
      exact absurd ea.symm (closeNeg_not_defd _ _ _)
    · unfold closeV at ea; simp only [h1, h2, ite_true, ite_false] at ea
      exact absurd ea (closeNeg_not_defd _ _ _)
    · unfold closeV at ea; simp only [h1, h2, ite_false] at ea
      rw [closeNeg_inj a b (by rcases ha.1 with h | h; exact h; omega) (by rcases hb.1 with h | h; exact h; omega) ea]
  | bot =>
    exfalso; apply ha.2
    unfold closeV at ea; split at ea
    · cases ea
    · unfold closeNeg at ea; split at ea
      · assumption
      · split at ea <;> cases ea
  | ref _ _ =>
    rw [closeV_ref_class m a ha _ _ ea, closeV_ref_class m b hb _ _ eb]

/-- Size classes of the fields of a struct. -/
def classes (td : TD) : List Nat := (List.range td.fields.length).map fun j => fieldClass (fcode td j)

/-- A struct with a declared supertype lays out the fields of the supertype as the supertype does. -/
theorem layout_sub (m : M) (t s : Nat) (h : CompSub m t s) (hk : (m.d t).kind = kStruct)
    (fa : ∀ j < (m.d t).fields.length, StorageCode m (fcode (m.d t) j))
    (fb : ∀ j < (m.d s).fields.length, StorageCode m (fcode (m.d s) j)) :
    Layout.layout (classes (m.d s)) <+: Layout.layout (classes (m.d t)) := by
  unfold CompSub at h
  simp only [hk, kStruct, kFunc, show (95 : Nat) ≠ 96 by decide, ite_false] at h
  obtain ⟨hl, hf⟩ := h
  have e : classes (m.d t) = classes (m.d s) ++ (classes (m.d t)).drop (m.d s).fields.length := by
    apply List.ext_getElem?; intro j
    by_cases hj : j < (m.d s).fields.length
    · rw [List.getElem?_append_left (by simp [classes]; exact hj)]
      simp only [classes, List.getElem?_map, List.getElem?_range hj, List.getElem?_range (show j < (m.d t).fields.length by omega),
        Option.map_some, Option.some.injEq]
      exact vsub_class m _ _ (fa j (by omega)) (fb j hj) (hf j hj).2.1
    · rw [List.getElem?_append_right (by simp [classes]; omega), List.getElem?_drop]
      simp only [classes, List.length_map, List.length_range]
      congr 1; omega
  rw [e]; exact Layout.layout_prefix _ _

end Wah.LayoutSub
