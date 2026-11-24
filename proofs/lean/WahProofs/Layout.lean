/-
Model of the struct layout builder `wah_build_struct_repr_info` in wah.h.

A field of class `k` (k <= 4) has `2^k` bytes: i8 = 0, i16 = 1, i32/f32 = 2, i64/f64/ref = 3, v128 = 4.
The builder keeps at most one free slot per class below 16 bytes, and the frontier (a multiple of 16).
Placing a field of class `k` takes the smallest class `j >= k` with a free slot, where `j = 4` is a new
16-byte chunk at the frontier, puts the field at its start and returns the rest of the slot as one free
slot per class `k <= i < j` at `offset + 2^i`. The struct size is the end of the last byte of any field.
-/
namespace Wah.Layout

structure St where
  slot : Nat → Option Nat
  frontier : Nat

def St.init : St := ⟨fun _ => none, 0⟩

/-- The smallest class `j >= k` with a free slot, or 4 when there is none. -/
def findClass (slot : Nat → Option Nat) (k : Nat) : Nat :=
  if k < 4 then (if (slot k).isSome then k else findClass slot (k + 1)) else 4
termination_by 4 - k

/-- Places a field of class `k`, returning its offset. -/
def place (st : St) (k : Nat) : Nat × St :=
  let j := findClass st.slot k
  let o := if j < 4 then (st.slot j).getD 0 else st.frontier
  (o, { slot := fun i => if k ≤ i ∧ i < j then some (o + 2 ^ i) else if i = j then none else st.slot i,
        frontier := if j < 4 then st.frontier else st.frontier + 16 })

/-- Placed fields as `(offset, size)` in the order of their definition, and the builder state. -/
def step (acc : List (Nat × Nat) × St) (k : Nat) : List (Nat × Nat) × St :=
  let r := place acc.2 k
  (acc.1 ++ [(r.1, 2 ^ k)], r.2)

def run (ks : List Nat) : List (Nat × Nat) × St := ks.foldl step ([], St.init)

def layout (ks : List Nat) : List (Nat × Nat) := (run ks).1

/-- `live_end` in C. -/
def size (fs : List (Nat × Nat)) : Nat := fs.foldl (fun m f => max m (f.1 + f.2)) 0

def Disj (a b : Nat × Nat) : Prop := a.1 + a.2 ≤ b.1 ∨ b.1 + b.2 ≤ a.1

/-! ### `findClass` -/

theorem findClass_ge (slot : Nat → Option Nat) (k : Nat) (hk : k ≤ 4) :
    k ≤ findClass slot k ∧ findClass slot k ≤ 4 := by
  induction k using findClass.induct slot with
  | case1 k h hs => unfold findClass; simp [h, hs]; omega
  | case2 k h hs ih => unfold findClass; simp [h, hs]; have := ih (by omega); omega
  | case3 k h => unfold findClass; simp [h]; omega

theorem findClass_none (slot : Nat → Option Nat) (k : Nat) :
    ∀ i, k ≤ i → i < findClass slot k → slot i = none := by
  induction k using findClass.induct slot with
  | case1 k h hs => unfold findClass; simp [h, hs]; omega
  | case2 k h hs ih =>
    intro i h1 h2; unfold findClass at h2; simp [h, hs] at h2
    by_cases hi : i = k
    · subst hi; simpa using hs
    · exact ih i (by omega) h2
  | case3 k h => unfold findClass; simp [h]; omega

theorem findClass_some (slot : Nat → Option Nat) (k : Nat) :
    findClass slot k < 4 → (slot (findClass slot k)).isSome := by
  induction k using findClass.induct slot with
  | case1 k h hs => unfold findClass; simp [h, hs]
  | case2 k h hs ih => unfold findClass; simp [h, hs]; exact ih
  | case3 k h => unfold findClass; simp [h]

/-! ### Invariant -/

def slotBytes (slot : Nat → Option Nat) (i : Nat) : Nat := if (slot i).isSome then 2 ^ i else 0

def freeBytes (slot : Nat → Option Nat) : Nat :=
  slotBytes slot 0 + slotBytes slot 1 + slotBytes slot 2 + slotBytes slot 3

structure Inv (ps : List (Nat × Nat)) (st : St) : Prop where
  frontier_aligned : st.frontier % 16 = 0
  slot_high : ∀ i, 4 ≤ i → st.slot i = none
  slot_aligned : ∀ i o, st.slot i = some o → 2 ^ i ∣ o
  slot_bound : ∀ i o, st.slot i = some o → o + 2 ^ i ≤ st.frontier
  field_aligned : ∀ p ∈ ps, p.2 ∣ p.1
  field_bound : ∀ p ∈ ps, p.1 + p.2 ≤ st.frontier
  field_disj : ps.Pairwise Disj
  field_slot_disj : ∀ p ∈ ps, ∀ i o, st.slot i = some o → Disj p (o, 2 ^ i)
  slot_disj : ∀ i j oi oj, i ≠ j → st.slot i = some oi → st.slot j = some oj → Disj (oi, 2 ^ i) (oj, 2 ^ j)
  bytes : (ps.map Prod.snd).sum + freeBytes st.slot = st.frontier

theorem inv_init : Inv [] St.init := by
  constructor <;> simp [St.init, freeBytes, slotBytes]

theorem disj_sub {x r a : Nat × Nat} (h : Disj x r) (h1 : r.1 ≤ a.1) (h2 : a.1 + a.2 ≤ r.1 + r.2) :
    Disj x a := by
  unfold Disj at *; omega

theorem disj_symm {a b : Nat × Nat} (h : Disj a b) : Disj b a := by
  unfold Disj at *; omega

theorem pow_le {a b : Nat} (h : a ≤ b) : 2 ^ a ≤ 2 ^ b := Nat.pow_le_pow_right (by decide) h

theorem pow_succ_le {a b : Nat} (h : a < b) : 2 ^ a + 2 ^ a ≤ 2 ^ b := by
  have := pow_le (Nat.succ_le_of_lt h); rw [Nat.pow_succ] at this; omega

theorem pow_dvd {a b : Nat} (h : a ≤ b) : 2 ^ a ∣ 2 ^ b := Nat.pow_dvd_pow 2 h

/-- The pieces of a slot of class `j` split down to class `k` add up to the slot. -/
theorem freeBytes_place (s : Nat → Option Nat) (k j o : Nat) (hk : k ≤ j) (hj : j ≤ 4)
    (hnone : ∀ i, k ≤ i → i < j → s i = none) (hsome : j < 4 → (s j).isSome) :
    freeBytes (fun i => if k ≤ i ∧ i < j then some (o + 2 ^ i) else if i = j then none else s i) + 2 ^ k
      = freeBytes s + (if j < 4 then 0 else 16) := by
  have h0 := hnone 0; have h1 := hnone 1; have h2 := hnone 2; have h3 := hnone 3
  unfold freeBytes slotBytes
  rcases (show j = 0 ∨ j = 1 ∨ j = 2 ∨ j = 3 ∨ j = 4 by omega) with rfl | rfl | rfl | rfl | rfl <;>
  rcases (show k = 0 ∨ k = 1 ∨ k = 2 ∨ k = 3 ∨ k = 4 by omega) with rfl | rfl | rfl | rfl | rfl <;>
  simp_all <;> omega

theorem inv_step (ps : List (Nat × Nat)) (st : St) (k : Nat) (hk : k ≤ 4) (h : Inv ps st) :
    Inv (step (ps, st) k).1 (step (ps, st) k).2 := by
  obtain ⟨hkj, hj4⟩ := findClass_ge st.slot k hk
  have hnone := findClass_none st.slot k
  have hsome := findClass_some st.slot k
  generalize hj : findClass st.slot k = j at hkj hj4 hnone hsome
  generalize ho : (if j < 4 then (st.slot j).getD 0 else st.frontier) = o
  generalize hfr : (if j < 4 then st.frontier else st.frontier + 16) = fr
  -- The slot (or chunk) that the field and new slots are taken from
  have hRal : 2 ^ j ∣ o := by
    by_cases hj' : j < 4
    · obtain ⟨o', ho'⟩ := Option.isSome_iff_exists.mp (hsome hj')
      simp [hj', ho'] at ho; subst ho; exact h.slot_aligned j o' ho'
    · have : j = 4 := by omega
      subst this; simp at ho; subst ho; exact Nat.dvd_of_mod_eq_zero h.frontier_aligned
  have hRb : o + 2 ^ j ≤ fr := by
    by_cases hj' : j < 4
    · obtain ⟨o', ho'⟩ := Option.isSome_iff_exists.mp (hsome hj')
      simp [hj', ho'] at ho hfr; subst ho hfr; exact h.slot_bound j o' ho'
    · have : j = 4 := by omega
      subst this; simp at ho hfr; omega
  have hRps : ∀ p ∈ ps, Disj p (o, 2 ^ j) := by
    intro p hp
    by_cases hj' : j < 4
    · obtain ⟨o', ho'⟩ := Option.isSome_iff_exists.mp (hsome hj')
      simp [hj', ho'] at ho; subst ho; exact h.field_slot_disj p hp j o' ho'
    · have : j = 4 := by omega
      subst this; simp at ho; subst ho; have := h.field_bound p hp; unfold Disj; omega
  have hRsl : ∀ i oi, i ≠ j → st.slot i = some oi → Disj (oi, 2 ^ i) (o, 2 ^ j) := by
    intro i oi hij hi
    by_cases hj' : j < 4
    · obtain ⟨o', ho'⟩ := Option.isSome_iff_exists.mp (hsome hj')
      simp [hj', ho'] at ho; subst ho; exact h.slot_disj i j oi o' hij hi ho'
    · have : j = 4 := by omega
      subst this; simp at ho; subst ho; have := h.slot_bound i oi hi; unfold Disj; simp; omega
  have hfrle : st.frontier ≤ fr := by split at hfr <;> omega
  have hpk := pow_le hkj
  have hslot : ∀ i oi, (if k ≤ i ∧ i < j then some (o + 2 ^ i) else if i = j then none else st.slot i) = some oi →
      (k ≤ i ∧ i < j ∧ oi = o + 2 ^ i) ∨ (i ≠ j ∧ ¬(k ≤ i ∧ i < j) ∧ st.slot i = some oi) := by
    intro i oi; split
    · intro e; simp at e; left; omega
    · split
      · simp
      · rename_i h1 h2; intro e; right; exact ⟨h2, h1, e⟩
  simp only [step, place, hj, ho, hfr]
  constructor
  · simp only; split at hfr <;> (have := h.frontier_aligned; omega)
  · intro i hi; simp only; split
    · omega
    · split
      · rfl
      · exact h.slot_high i hi
  · intro i oi hi; rcases hslot i oi hi with ⟨h1, h2, rfl⟩ | ⟨_, _, h3⟩
    · exact Nat.dvd_add (Nat.dvd_trans (pow_dvd (by omega)) hRal) (Nat.dvd_refl _)
    · exact h.slot_aligned i oi h3
  · intro i oi hi; rcases hslot i oi hi with ⟨h1, h2, rfl⟩ | ⟨_, _, h3⟩
    · have := pow_succ_le h2; dsimp only; omega
    · have := h.slot_bound i oi h3; dsimp only; omega
  · intro p hp; simp at hp; rcases hp with hp | rfl
    · exact h.field_aligned p hp
    · exact Nat.dvd_trans (pow_dvd hkj) hRal
  · intro p hp; simp at hp; rcases hp with hp | rfl
    · have := h.field_bound p hp; dsimp only; omega
    · simp; omega
  · rw [List.pairwise_append]; refine ⟨h.field_disj, by simp, ?_⟩
    intro p hp q hq; simp at hq; subst hq
    exact disj_sub (hRps p hp) (by simp) (by simp; omega)
  · intro p hp i oi hi; simp at hp
    rcases hslot i oi hi with ⟨h1, h2, rfl⟩ | ⟨h1, h2, h3⟩
    · rcases hp with hp | rfl
      · have := pow_succ_le h2; exact disj_sub (hRps p hp) (by simp) (by simp; omega)
      · have := pow_le h1; unfold Disj; simp; omega
    · rcases hp with hp | rfl
      · exact h.field_slot_disj p hp i oi h3
      · exact disj_symm (disj_sub (hRsl i oi h1 h3) (by simp) (by simp; omega))
  · intro i i' oi oi' hii hi hi'
    rcases hslot i oi hi with ⟨h1, h2, rfl⟩ | ⟨h1, h2, h3⟩ <;>
      rcases hslot i' oi' hi' with ⟨h1', h2', rfl⟩ | ⟨h1', h2', h3'⟩
    · unfold Disj; simp
      rcases Nat.lt_or_gt_of_ne hii with hlt | hlt
      · have := pow_succ_le hlt; omega
      · have := pow_succ_le hlt; omega
    · have := pow_succ_le h2
      exact disj_symm (disj_sub (hRsl i' oi' h1' h3') (by simp) (by simp; omega))
    · have := pow_succ_le h2'
      exact disj_sub (hRsl i oi h1 h3) (by simp) (by simp; omega)
    · exact h.slot_disj i i' oi oi' hii h3 h3'
  · have hb := freeBytes_place st.slot k j o hkj hj4 hnone hsome
    have := h.bytes; simp [List.sum_append]
    split at hfr <;> simp_all <;> omega

/-! ### Counting bytes of disjoint intervals -/

def inI (p : Nat × Nat) (b : Nat) : Bool := decide (p.1 ≤ b ∧ b < p.1 + p.2)

def cov (L : List (Nat × Nat)) (b : Nat) : Bool := L.any (fun p => inI p b)

theorem count_inI (p : Nat × Nat) (n : Nat) :
    (List.range n).countP (inI p) = min (p.1 + p.2) n - min p.1 n := by
  induction n with
  | zero => simp
  | succ n ih => rw [List.range_succ, List.countP_append, ih]; simp [inI]; split <;> omega

theorem countP_or (l : List Nat) (f g : Nat → Bool) (h : ∀ b, ¬(f b = true ∧ g b = true)) :
    l.countP (fun b => f b || g b) = l.countP f + l.countP g := by
  induction l with
  | nil => simp
  | cons x l ih =>
    simp only [List.countP_cons, ih]; have := h x
    rcases Bool.eq_false_or_eq_true (f x) with hf | hf <;> rcases Bool.eq_false_or_eq_true (g x) with hg | hg <;>
      simp [hf, hg] at this ⊢ <;> omega

theorem count_cov (L : List (Nat × Nat)) (n : Nat) (hd : L.Pairwise Disj) (hb : ∀ p ∈ L, p.1 + p.2 ≤ n) :
    (List.range n).countP (cov L) = (L.map Prod.snd).sum := by
  induction L with
  | nil => simp [cov]
  | cons x L ih =>
    rw [List.pairwise_cons] at hd
    have e : cov (x :: L) = fun b => inI x b || cov L b := by funext b; simp [cov]
    rw [e, countP_or, count_inI, ih hd.2 (fun p hp => hb p (by simp [hp]))]
    · have := hb x (by simp); simp; omega
    · intro b ⟨h1, h2⟩; simp [cov, inI] at h1 h2
      obtain ⟨a, c, hq, h3⟩ := h2; have := hd.1 (a, c) hq; unfold Disj at this; simp at this; omega

/-- Disjoint intervals in `[0, n)` have at most `n` bytes in total. -/
theorem sum_le_of_disj (L : List (Nat × Nat)) (n : Nat) (hd : L.Pairwise Disj) (hb : ∀ p ∈ L, p.1 + p.2 ≤ n) :
    (L.map Prod.snd).sum ≤ n := by
  rw [← count_cov L n hd hb]; have := List.countP_le_length (p := cov L) (l := List.range n); simpa using this

/-! ### Struct size -/

theorem size_aux_ge (fs : List (Nat × Nat)) (m : Nat) :
    m ≤ fs.foldl (fun m f => max m (f.1 + f.2)) m ∧ ∀ p ∈ fs, p.1 + p.2 ≤ fs.foldl (fun m f => max m (f.1 + f.2)) m := by
  induction fs generalizing m with
  | nil => simp
  | cons x fs ih =>
    have := ih (max m (x.1 + x.2)); simp only [List.foldl_cons, List.mem_cons]
    refine ⟨by omega, fun p hp => ?_⟩
    rcases hp with rfl | hp
    · omega
    · exact this.2 p hp

theorem size_aux_le (fs : List (Nat × Nat)) (m n : Nat) (hm : m ≤ n) (h : ∀ p ∈ fs, p.1 + p.2 ≤ n) :
    fs.foldl (fun m f => max m (f.1 + f.2)) m ≤ n := by
  induction fs generalizing m with
  | nil => simpa
  | cons x fs ih => simp only [List.foldl_cons]; exact ih _ (by have := h x (by simp); omega) (fun p hp => h p (by simp [hp]))

theorem field_le_size (fs : List (Nat × Nat)) : ∀ p ∈ fs, p.1 + p.2 ≤ size fs := (size_aux_ge fs 0).2

theorem freeBytes_le (s : Nat → Option Nat) : freeBytes s ≤ 15 := by
  unfold freeBytes slotBytes; repeat' split
  all_goals simp

/-! ### Main theorems -/

theorem foldl_inv (ks : List Nat) (acc : List (Nat × Nat) × St) (hk : ∀ k ∈ ks, k ≤ 4)
    (h : Inv acc.1 acc.2) : Inv (ks.foldl step acc).1 (ks.foldl step acc).2 := by
  induction ks generalizing acc with
  | nil => simpa
  | cons k ks ih =>
    simp only [List.foldl_cons]
    exact ih _ (fun k' hk' => hk k' (by simp [hk'])) (inv_step acc.1 acc.2 k (hk k (by simp)) h)

theorem run_inv (ks : List Nat) (hk : ∀ k ∈ ks, k ≤ 4) : Inv (run ks).1 (run ks).2 :=
  foldl_inv ks _ hk inv_init

theorem foldl_sizes (ks : List Nat) (acc : List (Nat × Nat) × St) :
    (ks.foldl step acc).1.map Prod.snd = acc.1.map Prod.snd ++ ks.map (2 ^ ·) := by
  induction ks generalizing acc with
  | nil => simp
  | cons k ks ih => simp only [List.foldl_cons, ih]; simp [step]

theorem foldl_prefix (ks : List Nat) (acc : List (Nat × Nat) × St) : acc.1 <+: (ks.foldl step acc).1 := by
  induction ks generalizing acc with
  | nil => simp
  | cons k ks ih =>
    simp only [List.foldl_cons]
    exact List.IsPrefix.trans (List.prefix_append _ _) (ih (step acc k))

theorem step_frontier (acc : List (Nat × Nat) × St) (k : Nat) : (step acc k).2.frontier ≤ acc.2.frontier + 16 := by
  simp only [step, place]; split <;> omega

theorem foldl_frontier (ks : List Nat) (acc : List (Nat × Nat) × St) :
    (ks.foldl step acc).2.frontier ≤ acc.2.frontier + 16 * ks.length := by
  induction ks generalizing acc with
  | nil => simp
  | cons k ks ih =>
    simp only [List.foldl_cons, List.length_cons]
    have := ih (step acc k); have := step_frontier acc k; omega

/-- Field `i` has the size of its class. -/
theorem layout_sizes (ks : List Nat) : (layout ks).map Prod.snd = ks.map (2 ^ ·) := by
  unfold layout run; simpa using foldl_sizes ks ([], St.init)

/-- Every field is aligned to its size. -/
theorem layout_aligned (ks : List Nat) (hk : ∀ k ∈ ks, k ≤ 4) : ∀ p ∈ layout ks, p.2 ∣ p.1 :=
  (run_inv ks hk).field_aligned

/-- No two fields overlap. -/
theorem layout_disjoint (ks : List Nat) (hk : ∀ k ∈ ks, k ≤ 4) : (layout ks).Pairwise Disj :=
  (run_inv ks hk).field_disj

/-- Every field lies within the struct. -/
theorem layout_within (ks : List Nat) : ∀ p ∈ layout ks, p.1 + p.2 ≤ size (layout ks) :=
  field_le_size _

/-- The struct has at most 15 bytes of padding. -/
theorem layout_padding (ks : List Nat) (hk : ∀ k ∈ ks, k ≤ 4) :
    ((layout ks).map Prod.snd).sum ≤ size (layout ks) ∧ size (layout ks) ≤ ((layout ks).map Prod.snd).sum + 15 := by
  have h := run_inv ks hk
  refine ⟨sum_le_of_disj _ _ h.field_disj (field_le_size _), ?_⟩
  have := size_aux_le (layout ks) 0 (run ks).2.frontier (by omega) h.field_bound
  have := h.bytes; have := freeBytes_le (run ks).2.slot
  unfold size layout at *; omega

/-- Adding fields doesn't move earlier fields, so a struct and its prefixes agree on common fields. -/
theorem layout_prefix (ks ks' : List Nat) : layout ks <+: layout (ks ++ ks') := by
  unfold layout run; rw [List.foldl_append]; exact foldl_prefix _ _

/-- The frontier grows by at most 16 bytes per field, so `live_end <= 16 * field_count`. -/
theorem layout_size_le (ks : List Nat) (hk : ∀ k ∈ ks, k ≤ 4) : size (layout ks) ≤ 16 * ks.length := by
  have := size_aux_le (layout ks) 0 (run ks).2.frontier (by omega) (run_inv ks hk).field_bound
  have := foldl_frontier ks ([], St.init); rw [show St.init.frontier = 0 from rfl] at this
  unfold size layout run at *; omega

end Wah.Layout
