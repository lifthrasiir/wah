/-
Model of type canonicalization in wah.h: `wah_canonicalize_type_ref`, `wah_canon_ref_eq`,
`wah_types_structurally_equal` and the construction of `canonical_map` in `wah_module_build_type_metadata`.

Types are equivalent in the spec (iso-recursive equivalence) when their closed forms are equal: references within
the rec group are rolled to relative indices (`inner`), and references to earlier groups are replaced by the closed
form of that group with the relative index (`dt`). The theorem is that `canonical_map` maps each type to the least
equivalent type, so two types are equivalent exactly when they have the same canonical type.
-/
namespace Wah.Types

/-- Value and storage types: anything but a concrete reference is an atom (`wah_type_t` < 0). -/
inductive VT where
  | atom (a : Nat)
  | ref (x : Nat) (null : Bool)
  deriving DecidableEq

/-- `wah_type_def_t` with its `wah_func_type_t`. Functions have params ++ results as immutable fields and
`(param_count, result_count)` as counts; structs and arrays have `(field_count, 0)`. -/
structure TD where
  final : Bool
  kind : Nat
  super : Option Nat
  counts : Nat × Nat
  fields : List (VT × Bool)
  deriving Inhabited

/-- Types `0 .. n-1`; type `t` is in the rec group of `gsize t` types from `start t`. -/
structure M where
  n : Nat
  d : Nat → TD
  start : Nat → Nat
  gsize : Nat → Nat

def TD.refs (td : TD) : List Nat :=
  td.super.toList ++ td.fields.filterMap fun f => match f.1 with | .ref x _ => some x | .atom _ => none

/-- Valid modules: rec groups partition the types, and types only refer to earlier groups or their own. -/
structure WF (m : M) : Prop where
  group : ∀ t < m.n, m.start t ≤ t ∧ t < m.start t + m.gsize t ∧ m.start t + m.gsize t ≤ m.n
  same : ∀ t < m.n, ∀ u, m.start t ≤ u → u < m.start t + m.gsize t → m.start u = m.start t ∧ m.gsize u = m.gsize t
  refs : ∀ t < m.n, ∀ x ∈ (m.d t).refs, x < m.start t + m.gsize t

/-! ### Spec: closed forms -/

/-- Closed forms. `sub` is a closed subtype, `dt g i` the `i`-th type of a closed rec group `g`. -/
inductive CT where
  | atom (a : Nat)
  | inner (i : Nat) (null : Bool)
  | dt (g : List CT) (i : Nat) (null : Bool)
  | sub (final : Bool) (kind : Nat) (super : Option CT) (counts : Nat × Nat) (fields : List (CT × Bool))

/-- The closed rec group starting at `s`. Invalid references become `atom 0`, which `WF` rules out. -/
def closeGroup (m : M) (s : Nat) : List CT :=
  (List.range (m.gsize s)).map fun k =>
    let cref : VT → CT := fun v => match v with
      | .atom a => .atom a
      | .ref x null =>
        if s ≤ x ∧ x < s + m.gsize s then .inner (x - s) null
        else if _ : m.start x < s then .dt (closeGroup m (m.start x)) (x - m.start x) null
        else .atom 0
    let td := m.d (s + k)
    .sub td.final td.kind (td.super.map fun x => cref (.ref x false)) td.counts
      (td.fields.map fun f => (cref f.1, f.2))
termination_by s

/-- The defined type of type `t`; types are equivalent iff they have the same defined type. -/
def dtOf (m : M) (t : Nat) : List CT × Nat := (closeGroup m (m.start t), t - m.start t)

/-! ### WAH -/

/-- `wah_canonicalize_type_ref` for a reference from the group at `s`. -/
def canonRef (m : M) (canon : Nat → Nat) (s : Nat) : VT → VT
  | .atom a => .atom a
  | .ref x null => if s ≤ x ∧ x < s + m.gsize s then .ref x null else if m.n ≤ x then .ref x null
      else .ref (canon x) null

/-- `wah_canon_ref_eq` for canonicalized references from groups at `s1` and `s2`. -/
def refEq (m : M) (s1 s2 : Nat) : VT → VT → Bool
  | .ref x nx, .ref y ny =>
    if s1 ≤ x ∧ x < s1 + m.gsize s1 then
      (if s2 ≤ y ∧ y < s2 + m.gsize s2 then nx == ny && x - s1 == y - s2 else false)
    else (if s2 ≤ y ∧ y < s2 + m.gsize s2 then false else nx == ny && x == y)
  | .atom a, .atom b => a == b
  | _, _ => false

def refEqC (m : M) (canon : Nat → Nat) (s1 s2 : Nat) (a b : VT) : Bool :=
  refEq m s1 s2 (canonRef m canon s1 a) (canonRef m canon s2 b)

def fieldsEq (m : M) (canon : Nat → Nat) (s1 s2 : Nat) : List (VT × Bool) → List (VT × Bool) → Bool
  | [], [] => true
  | a :: as, b :: bs => refEqC m canon s1 s2 a.1 b.1 && a.2 == b.2 && fieldsEq m canon s1 s2 as bs
  | _, _ => false

/-- `wah_types_structurally_equal`. -/
def typesEq (m : M) (canon : Nat → Nat) (a b : Nat) : Bool :=
  let ta := m.d a; let tb := m.d b
  let s1 := m.start a; let s2 := m.start b
  ta.kind == tb.kind && ta.final == tb.final &&
  (match ta.super, tb.super with
    | none, none => true
    | some x, some y => refEqC m canon s1 s2 (.ref x false) (.ref y false)
    | _, _ => false) &&
  ta.counts == tb.counts && fieldsEq m canon s1 s2 ta.fields tb.fields

/-- Rec groups at `i` and `j` are equal (the match loop in `wah_module_build_type_metadata`). -/
def groupEq (m : M) (canon : Nat → Nat) (i j : Nat) : Bool :=
  m.gsize j == m.gsize i && (List.range (m.gsize i)).all fun k => typesEq m canon (i + k) (j + k)

/-- Canonicalizes the group at `i`: the first earlier canonical group of the same hash that is equal. Groups are
sorted by hash and start, so candidates are the earlier groups of the same hash in order. -/
def findCanon (m : M) (hash : Nat → Nat) (canon : Nat → Nat) (i : Nat) : Option Nat :=
  let cands := (List.range i).filter fun j => m.start j == j && hash j == hash i
  cands.find? (fun j => canon j == j && groupEq m canon i j)

def applyCanon (m : M) (canon : Nat → Nat) (i : Nat) : Option Nat → Nat → Nat
  | some j => fun x => if i ≤ x ∧ x < i + m.gsize i then j + (x - i) else canon x
  | none => canon

def processGroup (m : M) (hash : Nat → Nat) (canon : Nat → Nat) (i : Nat) : Nat → Nat :=
  applyCanon m canon i (findCanon m hash canon i)

/-- A canonical map. It is a structure so that compiled code (Oracle.lean) computes each group once, instead of
for every lookup as a function would be compiled. -/
structure CMap where
  get : Nat → Nat

/-- `canonical_map` after processing the groups starting below `t`. -/
def build (m : M) (hash : Nat → Nat) : Nat → CMap
  | 0 => ⟨id⟩
  | t + 1 =>
    let c := build m hash t
    if m.start t = t then
      let j := findCanon m hash c.get t
      ⟨applyCanon m c.get t j⟩
    else c

theorem build_succ (m : M) (hash : Nat → Nat) (t : Nat) :
    (build m hash (t + 1)).get =
      if m.start t = t then processGroup m hash (build m hash t).get t else (build m hash t).get := by
  simp only [build, processGroup]; split <;> rfl

def canonicalMap (m : M) (hash : Nat → Nat) : Nat → Nat := (build m hash m.n).get

/-! ### Closed forms, unfolded -/

def cref (m : M) (s : Nat) : VT → CT
  | .atom a => .atom a
  | .ref x null =>
    if s ≤ x ∧ x < s + m.gsize s then .inner (x - s) null
    else if _ : m.start x < s then .dt (closeGroup m (m.start x)) (x - m.start x) null
    else .atom 0

def closeSub (m : M) (s t : Nat) : CT :=
  let td := m.d t
  .sub td.final td.kind (td.super.map fun x => cref m s (.ref x false)) td.counts
    (td.fields.map fun f => (cref m s f.1, f.2))

theorem closeGroup_eq (m : M) (s : Nat) :
    closeGroup m s = (List.range (m.gsize s)).map fun k => closeSub m s (s + k) := by
  rw [closeGroup]; rfl

/-- What `canonical_map` is required to be below a group: it maps types to the least equivalent type. -/
def Least (m : M) (canon : Nat → Nat) (x : Nat) : Prop :=
  canon x ≤ x ∧ dtOf m (canon x) = dtOf m x ∧ ∀ y < canon x, dtOf m y ≠ dtOf m x

theorem least_iff {m : M} {canon : Nat → Nat} {x y : Nat} (hx : Least m canon x) (hy : Least m canon y) :
    canon x = canon y ↔ dtOf m x = dtOf m y := by
  constructor
  · intro e; rw [← hx.2.1, ← hy.2.1, e]
  · intro e
    rcases Nat.lt_trichotomy (canon x) (canon y) with h | h | h
    · exact absurd (hx.2.1.trans e) (hy.2.2 _ h)
    · exact h
    · exact absurd (hy.2.1.trans e.symm) (hx.2.2 _ h)

/-- A reference from the group at `s`, which `WF` bounds by the end of the group. -/
def RefOK (m : M) (s : Nat) : VT → Prop
  | .atom _ => True
  | .ref x _ => x < s + m.gsize s

/-- `wah_canon_ref_eq` after `wah_canonicalize_type_ref` agrees with equality of closed forms, once earlier
groups are canonicalized. -/
theorem cref_eq_iff (m : M) (canon : Nat → Nat) (s1 s2 : Nat) (hw : WF m)
    (h1 : s1 + m.gsize s1 ≤ m.n) (h2 : s2 + m.gsize s2 ≤ m.n)
    (hc : ∀ x < m.n, x < s1 ∨ x < s2 → Least m canon x)
    (a b : VT) (ha : RefOK m s1 a) (hb : RefOK m s2 b) :
    cref m s1 a = cref m s2 b ↔ refEqC m canon s1 s2 a b = true := by
  cases a with
  | atom a => cases b with
    | atom b => simp [cref, refEqC, canonRef, refEq]
    | ref y ny =>
      simp only [cref, refEqC, canonRef, refEq]; simp only [RefOK] at hb
      by_cases hy : s2 ≤ y ∧ y < s2 + m.gsize s2
      · simp [hy]
      · have : m.start y < s2 := by have := (hw.group y (by omega)).1; omega
        simp [hy, this, show ¬ m.n ≤ y by omega]
  | ref x nx => cases b with
    | atom b =>
      simp only [cref, refEqC, canonRef, refEq]; simp only [RefOK] at ha
      by_cases hx : s1 ≤ x ∧ x < s1 + m.gsize s1
      · simp [hx]
      · have : m.start x < s1 := by have := (hw.group x (by omega)).1; omega
        simp [hx, this, show ¬ m.n ≤ x by omega]
    | ref y ny =>
      simp only [RefOK] at ha hb
      have hsx : ¬(s1 ≤ x ∧ x < s1 + m.gsize s1) → m.start x < s1 ∧ Least m canon x := fun hx =>
        ⟨by have := (hw.group x (by omega)).1; omega, hc x (by omega) (by omega)⟩
      have hsy : ¬(s2 ≤ y ∧ y < s2 + m.gsize s2) → m.start y < s2 ∧ Least m canon y := fun hy =>
        ⟨by have := (hw.group y (by omega)).1; omega, hc y (by omega) (by omega)⟩
      simp only [cref, refEqC, canonRef, refEq]
      by_cases hx : s1 ≤ x ∧ x < s1 + m.gsize s1 <;> by_cases hy : s2 ≤ y ∧ y < s2 + m.gsize s2
      · simp [hx, hy]; constructor
        · rintro ⟨h3, h4⟩; exact ⟨h4, h3⟩
        · rintro ⟨h3, h4⟩; exact ⟨h4, h3⟩
      · obtain ⟨h3, h4⟩ := hsy hy; have := h4.1
        simp [hx, hy, h3, show ¬ m.n ≤ y by omega]; omega
      · obtain ⟨h3, h4⟩ := hsx hx; have := h4.1
        simp [hx, hy, h3, show ¬ m.n ≤ x by omega]; omega
      · obtain ⟨h3, lx⟩ := hsx hx; obtain ⟨h4, ly⟩ := hsy hy
        have e := least_iff lx ly; have := lx.1; have := ly.1
        simp only [hx, hy, h3, h4, dite_true, ite_false, show ¬ m.n ≤ x by omega, show ¬ m.n ≤ y by omega,
          show ¬ (s1 ≤ canon x ∧ canon x < s1 + m.gsize s1) by omega,
          show ¬ (s2 ≤ canon y ∧ canon y < s2 + m.gsize s2) by omega]
        simp only [CT.dt.injEq, Bool.and_eq_true, beq_iff_eq, e, dtOf, Prod.mk.injEq]
        exact ⟨fun ⟨h5, h6, h7⟩ => ⟨h7, h5, h6⟩, fun ⟨h7, h5, h6⟩ => ⟨h5, h6, h7⟩⟩

theorem fields_eq_iff (m : M) (canon : Nat → Nat) (s1 s2 : Nat) :
    ∀ (as bs : List (VT × Bool)),
    (∀ a ∈ as, ∀ b ∈ bs, (cref m s1 a.1 = cref m s2 b.1 ↔ refEqC m canon s1 s2 a.1 b.1 = true)) →
    (as.map (fun f => (cref m s1 f.1, f.2)) = bs.map (fun f => (cref m s2 f.1, f.2)) ↔
      fieldsEq m canon s1 s2 as bs = true)
  | [], [] => by simp [fieldsEq]
  | [], _ :: _ => by simp [fieldsEq]
  | _ :: _, [] => by simp [fieldsEq]
  | a :: as, b :: bs => by
    intro h
    have ih := fields_eq_iff m canon s1 s2 as bs (fun a' ha b' hb => h a' (by simp [ha]) b' (by simp [hb]))
    have hab := h a (by simp) b (by simp)
    simp only [List.map_cons, List.cons.injEq, Prod.mk.injEq, fieldsEq, Bool.and_eq_true, beq_iff_eq, hab, ih]

theorem gsize_start (m : M) (hw : WF m) (t : Nat) (ht : t < m.n) : m.gsize (m.start t) = m.gsize t := by
  have := hw.group t ht; exact (hw.same t ht (m.start t) (by omega) (by omega)).2

theorem start_start (m : M) (hw : WF m) (t : Nat) (ht : t < m.n) : m.start (m.start t) = m.start t := by
  have := hw.group t ht; exact (hw.same t ht (m.start t) (by omega) (by omega)).1

theorem field_refOK (m : M) (hw : WF m) (t : Nat) (ht : t < m.n) :
    ∀ f ∈ (m.d t).fields, RefOK m (m.start t) f.1 := by
  intro f hf
  match e : f.1 with
  | .atom _ => trivial
  | .ref x _ =>
    show x < _; rw [gsize_start m hw t ht]
    apply hw.refs t ht x; simp only [TD.refs, List.mem_append, List.mem_filterMap]
    right; exact ⟨f, hf, by rw [e]⟩

theorem super_refOK (m : M) (hw : WF m) (t : Nat) (ht : t < m.n) (x : Nat) (hx : (m.d t).super = some x) :
    RefOK m (m.start t) (.ref x false) := by
  show x < _; rw [gsize_start m hw t ht]
  apply hw.refs t ht x; simp [TD.refs, hx]

/-- `wah_types_structurally_equal` agrees with equality of closed subtypes. -/
theorem typesEq_iff (m : M) (canon : Nat → Nat) (hw : WF m) (a b : Nat) (ha : a < m.n) (hb : b < m.n)
    (hc : ∀ x < m.n, x < m.start a ∨ x < m.start b → Least m canon x) :
    closeSub m (m.start a) a = closeSub m (m.start b) b ↔ typesEq m canon a b = true := by
  have h1 := (hw.group a ha).2.2; have h2 := (hw.group b hb).2.2
  rw [← gsize_start m hw a ha] at h1; rw [← gsize_start m hw b hb] at h2
  have hr := cref_eq_iff m canon (m.start a) (m.start b) hw h1 h2 hc
  have hf := fields_eq_iff m canon (m.start a) (m.start b) (m.d a).fields (m.d b).fields
    (fun f hf g hg => hr f.1 g.1 (field_refOK m hw a ha f hf) (field_refOK m hw b hb g hg))
  have hsa := super_refOK m hw a ha; have hsb := super_refOK m hw b hb
  unfold closeSub typesEq
  simp only [CT.sub.injEq, Bool.and_eq_true, beq_iff_eq, hf]
  generalize (m.d a).super = sa at hsa; generalize (m.d b).super = sb at hsb
  cases sa <;> cases sb <;> simp only [Option.map_none, Option.map_some, Option.some.injEq, reduceCtorEq]
  · exact ⟨fun ⟨h3, h4, _, h5, h6⟩ => ⟨⟨⟨⟨h4, h3⟩, trivial⟩, h5⟩, h6⟩,
      fun ⟨⟨⟨⟨h4, h3⟩, _⟩, h5⟩, h6⟩ => ⟨h3, h4, trivial, h5, h6⟩⟩
  · simp
  · simp
  · rename_i x y; rw [hr _ _ (hsa x rfl) (hsb y rfl)]
    exact ⟨fun ⟨h3, h4, h7, h5, h6⟩ => ⟨⟨⟨⟨h4, h3⟩, h7⟩, h5⟩, h6⟩,
      fun ⟨⟨⟨⟨h4, h3⟩, h7⟩, h5⟩, h6⟩ => ⟨h3, h4, h7, h5, h6⟩⟩

theorem map_range_eq_iff {α : Type} (f g : Nat → α) (n1 n2 : Nat) :
    (List.range n1).map f = (List.range n2).map g ↔ n1 = n2 ∧ ∀ k < n1, f k = g k := by
  constructor
  · intro h
    have hl := congrArg List.length h; simp only [List.length_map, List.length_range] at hl
    subst hl
    refine ⟨rfl, fun k hk => ?_⟩
    have := congrArg (fun l => l[k]?) h; simpa [hk] using this
  · rintro ⟨rfl, h⟩; apply List.map_congr_left; intro k hk; exact h k (by simpa using hk)

/-- Group types are in the group. -/
theorem start_in (m : M) (hw : WF m) (i k : Nat) (hi : i < m.n) (hs : m.start i = i) (hk : k < m.gsize i) :
    i + k < m.n ∧ m.start (i + k) = i ∧ m.gsize (i + k) = m.gsize i := by
  have := hw.group i hi; have := hw.same i hi (i + k) (by omega) (by omega)
  rw [hs] at this; exact ⟨by omega, this⟩

/-- The match loop over a rec group agrees with equality of closed rec groups. -/
theorem groupEq_iff (m : M) (canon : Nat → Nat) (hw : WF m) (i j : Nat) (hi : i < m.n) (hj : j < m.n)
    (hsi : m.start i = i) (hsj : m.start j = j) (hji : j ≤ i) (hc : ∀ x < m.n, x < i → Least m canon x) :
    closeGroup m i = closeGroup m j ↔ groupEq m canon i j = true := by
  rw [closeGroup_eq, closeGroup_eq, map_range_eq_iff]
  unfold groupEq
  simp only [Bool.and_eq_true, beq_iff_eq, List.all_eq_true, List.mem_range]
  constructor
  · rintro ⟨e, h⟩; refine ⟨e.symm, fun k hk => ?_⟩
    obtain ⟨ha, hsa, _⟩ := start_in m hw i k hi hsi hk
    obtain ⟨hb, hsb, _⟩ := start_in m hw j k hj hsj (by omega)
    have := typesEq_iff m canon hw (i + k) (j + k) ha hb (by rw [hsa, hsb]; intro x hx h; exact hc x hx (by omega))
    rw [hsa, hsb] at this; exact this.mp (h k hk)
  · rintro ⟨e, h⟩; refine ⟨e.symm, fun k hk => ?_⟩
    obtain ⟨ha, hsa, _⟩ := start_in m hw i k hi hsi hk
    obtain ⟨hb, hsb, _⟩ := start_in m hw j k hj hsj (by omega)
    have := typesEq_iff m canon hw (i + k) (j + k) ha hb (by rw [hsa, hsb]; intro x hx h; exact hc x hx (by omega))
    rw [hsa, hsb] at this; exact this.mpr (h k hk)

/-! ### `canonical_map` -/

/-- Hashes of rec groups only depend on their closed forms (proved for `wah_rec_group_hash` separately). -/
def HashOK (m : M) (hash : Nat → Nat) : Prop :=
  ∀ i j, i < m.n → j < m.n → m.start i = i → m.start j = j → closeGroup m i = closeGroup m j → hash i = hash j

/-- Groups below `t` are canonicalized, and later types are mapped to themselves. -/
def BuildInv (m : M) (canon : Nat → Nat) (t : Nat) : Prop :=
  ∀ x < m.n, (m.start x < t → Least m canon x) ∧ (t ≤ m.start x → canon x = x)

theorem in_group_iff (m : M) (hw : WF m) (i x : Nat) (hi : i < m.n) (hsi : m.start i = i) (hx : x < m.n) :
    (i ≤ x ∧ x < i + m.gsize i) ↔ m.start x = i := by
  constructor
  · intro h; have := hw.same i hi x (by omega) (by omega); rw [hsi] at this; exact this.1
  · intro h; have := hw.group x hx; rw [← h, gsize_start m hw x hx]; omega

theorem least_congr {m : M} {c c' : Nat → Nat} {x : Nat} (e : c' x = c x) (h : Least m c x) : Least m c' x := by
  unfold Least at *; rw [e]; exact h

theorem inv_process (m : M) (hash : Nat → Nat) (hw : WF m) (hh : HashOK m hash) (canon : Nat → Nat) (i : Nat)
    (hi : i < m.n) (hsi : m.start i = i) (h : BuildInv m canon i) :
    BuildInv m (processGroup m hash canon i) (i + 1) := by
  have hc : ∀ x < m.n, x < i → Least m canon x := fun x hx hxi =>
    (h x hx).1 (by have := (hw.group x hx).1; omega)
  unfold processGroup findCanon
  generalize hcands : (List.range i).filter (fun j => m.start j == j && hash j == hash i) = cands
  have hmem : ∀ j, j ∈ cands ↔ j < i ∧ m.start j = j ∧ hash j = hash i := by
    intro j; rw [← hcands]; simp [List.mem_filter]
  cases hf : cands.find? (fun j => canon j == j && groupEq m canon i j) with
  | some j =>
    have hp := List.find?_some hf; have hj := (hmem j).mp (List.mem_of_find?_eq_some hf)
    simp only [Bool.and_eq_true, beq_iff_eq] at hp
    obtain ⟨hcj, hg⟩ := hp; obtain ⟨hji, hsj, _⟩ := hj
    have hjn : j < m.n := by omega
    have hG := (groupEq_iff m canon hw i j hi hjn hsi hsj (by omega) hc).mpr hg
    have hsz : m.gsize j = m.gsize i := by unfold groupEq at hg; simp only [Bool.and_eq_true, beq_iff_eq] at hg; exact hg.1
    have lj := hc j hjn hji
    simp only [applyCanon]
    intro x hx
    have hin := in_group_iff m hw i x hi hsi hx
    refine ⟨fun hlt => ?_, fun hge => ?_⟩
    · by_cases hxi : m.start x = i
      · have hr : i ≤ x ∧ x < i + m.gsize i := hin.mpr hxi
        unfold Least; simp only [hr, and_self, ite_true]
        obtain ⟨_, hs', _⟩ := start_in m hw j (x - i) hjn hsj (by omega)
        refine ⟨by omega, ?_, ?_⟩
        · unfold dtOf; rw [hs', hxi, hG]; simp <;> omega
        · intro y hy e
          unfold dtOf at e; simp only [Prod.mk.injEq] at e; rw [hxi] at e
          have hyn : y < m.n := by omega
          have hsy := (hw.group y hyn).1
          have : dtOf m (m.start y) = dtOf m j := by
            unfold dtOf; rw [start_start m hw y hyn, hsj, e.1, hG]; simp
          unfold Least at lj; rw [hcj] at lj; exact lj.2.2 (m.start y) (by omega) this
      · have hr : ¬(i ≤ x ∧ x < i + m.gsize i) := fun h' => hxi (hin.mp h')
        unfold Least; simp only [hr, ite_false]; exact (h x hx).1 (by omega)
    · have hr : ¬(i ≤ x ∧ x < i + m.gsize i) := fun h' => by have := hin.mp h'; omega
      simp only [hr, ite_false]; exact (h x hx).2 (by omega)
  | none =>
    have hno := List.find?_eq_none.mp hf
    simp only [applyCanon]
    intro x hx
    refine ⟨fun hlt => ?_, fun hge => (h x hx).2 (by omega)⟩
    by_cases hxi : m.start x = i
    · have hcx := (h x hx).2 (by omega)
      refine ⟨by omega, by rw [hcx], fun y hy e => ?_⟩
      rw [hcx] at hy
      unfold dtOf at e; simp only [Prod.mk.injEq] at e; rw [hxi] at e
      have hyn : y < m.n := by omega
      have hsy := (hw.group y hyn).1
      have hsyi : m.start y < i := by omega
      have syn : m.start y < m.n := by omega
      have ls := hc (m.start y) syn hsyi
      obtain ⟨hc1, hc2, hc3⟩ := ls
      generalize hcv : canon (m.start y) = c at hc1 hc2 hc3
      have hcn : c < m.n := by omega
      have hsc : m.start c = c := by
        unfold dtOf at hc2; simp only [Prod.mk.injEq] at hc2; rw [start_start m hw y hyn] at hc2
        have := (hw.group c hcn).1; omega
      have hGc : closeGroup m c = closeGroup m i := by
        unfold dtOf at hc2; simp only [Prod.mk.injEq] at hc2; rw [hsc, start_start m hw y hyn] at hc2
        rw [hc2.1, e.1]
      have hcc : canon c = c := by
        have lc := hc c hcn (by omega)
        rcases Nat.lt_or_ge (canon c) c with hlt' | hge'
        · exact absurd (lc.2.1.trans hc2) (hc3 _ hlt')
        · exact Nat.le_antisymm lc.1 hge'
      have hcm : c ∈ cands := (hmem c).mpr ⟨by omega, hsc, hh c i hcn hi hsc hsi hGc⟩
      have := hno c hcm
      have hg := (groupEq_iff m canon hw i c hi hcn hsi hsc (by omega) hc).mp hGc.symm
      simp [hcc, hg] at this
    · exact (h x hx).1 (by omega)

theorem build_inv (m : M) (hash : Nat → Nat) (hw : WF m) (hh : HashOK m hash) :
    ∀ t ≤ m.n, BuildInv m (build m hash t).get t
  | 0, _ => fun x _ => ⟨fun h => absurd h (Nat.not_lt_zero _), fun _ => rfl⟩
  | t + 1, ht => by
    have ih := build_inv m hash hw hh t (by omega)
    rw [build_succ]
    split
    · rename_i hst; exact inv_process m hash hw hh _ t (by omega) hst ih
    · rename_i hst
      intro x hx
      have hss := start_start m hw x hx
      refine ⟨fun hlt => (ih x hx).1 ?_, fun hge => (ih x hx).2 (by omega)⟩
      rcases Nat.lt_or_ge (m.start x) t with h' | h'
      · exact h'
      · exfalso; have : m.start x = t := by omega
        rw [this] at hss; exact hst hss

/-- `canonical_map` maps every type to the least equivalent type. -/
theorem canonicalMap_least (m : M) (hash : Nat → Nat) (hw : WF m) (hh : HashOK m hash) :
    ∀ x < m.n, Least m (canonicalMap m hash) x := fun x hx =>
  (build_inv m hash hw hh m.n (Nat.le_refl _) x hx).1 (by have := (hw.group x hx).1; omega)

/-- Types are equivalent exactly when `canonical_map` maps them to the same type. -/
theorem canonicalMap_iff (m : M) (hash : Nat → Nat) (hw : WF m) (hh : HashOK m hash) (x y : Nat)
    (hx : x < m.n) (hy : y < m.n) :
    canonicalMap m hash x = canonicalMap m hash y ↔ dtOf m x = dtOf m y :=
  least_iff (canonicalMap_least m hash hw hh x hx) (canonicalMap_least m hash hw hh y hy)

/-! ### `wah_rec_group_hash` -/

section Hash
variable (m : M) (mix : Nat → Nat → Nat) (funcKind : Nat)

/-- `wah_rec_group_hash_type`; `gh` is `group_hashes`. -/
def hashRef (gh : Nat → Nat) (s h : Nat) : VT → Nat
  | .atom a => mix (mix h 0) a
  | .ref x null =>
    if s ≤ x ∧ x < s + m.gsize s then mix (mix h 1) (2 * (x - s) + null.toNat)
    else mix (mix (mix h 2) (gh (m.start x))) (2 * (x - m.start x) + null.toNat)

/-- One type of `wah_rec_group_hash`, for any `mix` and `WAH_COMP_FUNC` = `funcKind`. -/
def hashSub (gh : Nat → Nat) (s h : Nat) (td : TD) : Nat :=
  let h := mix h (2 * td.kind + td.final.toNat)
  let h := match td.super with
    | none => mix h 3
    | some x => hashRef m mix gh s h (.ref x false)
  if td.kind = funcKind then
    td.fields.foldl (fun h f => hashRef m mix gh s h f.1) (mix h (td.counts.1 * 2 ^ 32 + td.counts.2))
  else td.fields.foldl (fun h f => mix (hashRef m mix gh s h f.1) f.2.toNat) (mix h td.counts.1)

def groupHash (gh : Nat → Nat) (s : Nat) : Nat :=
  (List.range (m.gsize s)).foldl (fun h k => hashSub m mix funcKind gh s h (m.d (s + k))) (mix 0 (m.gsize s))

/-- `group_hashes[s]`, computed from the hashes of earlier groups. -/
def ghash (s : Nat) : Nat :=
  groupHash m mix funcKind (fun x => if _ : x < s then ghash x else 0) s
termination_by s

end Hash

theorem foldl_range_congr {α : Type} (f g : α → Nat → α) (n : Nat) (h0 : α) (h : ∀ k < n, ∀ a, f a k = g a k) :
    (List.range n).foldl f h0 = (List.range n).foldl g h0 := by
  have : ∀ (l : List Nat) (a : α), (∀ k ∈ l, k < n) → l.foldl f a = l.foldl g a := by
    intro l; induction l with
    | nil => simp
    | cons k l ih => intro a hl; simp only [List.foldl_cons]; rw [h k (hl k (by simp)) a]; exact ih _ (fun k' hk => hl k' (by simp [hk]))
  exact this _ _ (fun k hk => by simpa using hk)

theorem foldl_map_congr {α β γ δ : Type} (F : β → δ) (G : γ → δ) (f : α → β → α) (g : α → γ → α) :
    ∀ (as : List β) (bs : List γ) (a : α), as.map F = bs.map G → (∀ x ∈ as, ∀ y ∈ bs, F x = G y → ∀ a, f a x = g a y) →
    as.foldl f a = bs.foldl g a
  | [], [], _, _, _ => rfl
  | [], _ :: _, _, e, _ => by simp at e
  | _ :: _, [], _, e, _ => by simp at e
  | x :: as, y :: bs, a, e, h => by
    simp only [List.map_cons, List.cons.injEq] at e
    simp only [List.foldl_cons]; rw [h x (by simp) y (by simp) e.1 a]
    exact foldl_map_congr F G f g as bs _ e.2 (fun x' hx y' hy => h x' (by simp [hx]) y' (by simp [hy]))

theorem hashRef_congr (m : M) (mix : Nat → Nat → Nat) (hw : WF m) (gh1 gh2 : Nat → Nat) (s1 s2 : Nat)
    (h1 : s1 + m.gsize s1 ≤ m.n) (h2 : s2 + m.gsize s2 ≤ m.n)
    (hg : ∀ x y, x < m.n → y < m.n → m.start x < s1 → m.start y < s2 →
      closeGroup m (m.start x) = closeGroup m (m.start y) → gh1 (m.start x) = gh2 (m.start y))
    (a b : VT) (ha : RefOK m s1 a) (hb : RefOK m s2 b) (e : cref m s1 a = cref m s2 b) (h : Nat) :
    hashRef m mix gh1 s1 h a = hashRef m mix gh2 s2 h b := by
  cases a with
  | atom a => cases b with
    | atom b => simp [cref] at e; simp [hashRef, e]
    | ref y ny =>
      simp only [RefOK] at hb
      have : m.start y < s2 ∨ (s2 ≤ y ∧ y < s2 + m.gsize s2) := by have := (hw.group y (by omega)).1; omega
      by_cases hy : s2 ≤ y ∧ y < s2 + m.gsize s2 <;> simp [cref, hy] at e
      have : m.start y < s2 := by omega
      simp [this] at e
  | ref x nx => cases b with
    | atom b =>
      simp only [RefOK] at ha
      have : m.start x < s1 ∨ (s1 ≤ x ∧ x < s1 + m.gsize s1) := by have := (hw.group x (by omega)).1; omega
      by_cases hx : s1 ≤ x ∧ x < s1 + m.gsize s1 <;> simp [cref, hx] at e
      have : m.start x < s1 := by omega
      simp [this] at e
    | ref y ny =>
      simp only [RefOK] at ha hb
      by_cases hx : s1 ≤ x ∧ x < s1 + m.gsize s1 <;> by_cases hy : s2 ≤ y ∧ y < s2 + m.gsize s2
      · simp [cref, hx, hy] at e; simp [hashRef, hx, hy, e]
      · have : m.start y < s2 := by have := (hw.group y (by omega)).1; omega
        simp [cref, hx, hy, this] at e
      · have : m.start x < s1 := by have := (hw.group x (by omega)).1; omega
        simp [cref, hx, hy, this] at e
      · have hsx : m.start x < s1 := by have := (hw.group x (by omega)).1; omega
        have hsy : m.start y < s2 := by have := (hw.group y (by omega)).1; omega
        simp only [cref, hx, hy, hsx, hsy, dite_true, ite_false, CT.dt.injEq] at e
        simp only [hashRef, hx, hy, ite_false, hg x y (by omega) (by omega) hsx hsy e.1, e.2.1, e.2.2]

/-- `wah_rec_group_hash` only depends on closed rec groups, whatever the mixer is. -/
theorem ghash_ok (m : M) (mix : Nat → Nat → Nat) (funcKind : Nat) (hw : WF m) : HashOK m (ghash m mix funcKind) := by
  suffices ∀ N i j, i + j < N → i < m.n → j < m.n → m.start i = i → m.start j = j →
      closeGroup m i = closeGroup m j → ghash m mix funcKind i = ghash m mix funcKind j from
    fun i j hi hj hsi hsj e => this (i + j + 1) i j (by omega) hi hj hsi hsj e
  intro N; induction N with
  | zero => intro i j h; omega
  | succ N ih =>
    intro i j hN hi hj hsi hsj e
    have e' := e
    rw [closeGroup_eq, closeGroup_eq, map_range_eq_iff] at e'
    obtain ⟨hsz, hk⟩ := e'
    rw [ghash, ghash]; unfold groupHash; rw [← hsz]
    apply foldl_range_congr; intro k hk' a
    obtain ⟨ha, hsa, _⟩ := start_in m hw i k hi hsi hk'
    obtain ⟨hb, hsb, _⟩ := start_in m hw j k hj hsj (by omega)
    have hgi := (hw.group (i + k) ha).2.2; have hgj := (hw.group (j + k) hb).2.2
    rw [hsa, ← gsize_start m hw (i + k) ha, hsa] at hgi; rw [hsb, ← gsize_start m hw (j + k) hb, hsb] at hgj
    have hg : ∀ x y, x < m.n → y < m.n → m.start x < i → m.start y < j →
        closeGroup m (m.start x) = closeGroup m (m.start y) →
        (fun x => if _ : x < i then ghash m mix funcKind x else 0) (m.start x) =
        (fun x => if _ : x < j then ghash m mix funcKind x else 0) (m.start y) := by
      intro x y hx hy hxi hyj e2
      simp only [hxi, hyj, dite_true]
      exact ih _ _ (by omega) (by have := (hw.group x hx).1; omega) (by have := (hw.group y hy).1; omega)
        (start_start m hw x hx) (start_start m hw y hy) e2
    have hr := hashRef_congr m mix hw (fun x => if _ : x < i then ghash m mix funcKind x else 0)
      (fun x => if _ : x < j then ghash m mix funcKind x else 0) i j hgi hgj hg
    have e3 := hk k hk'
    have hfa := field_refOK m hw (i + k) ha; have hfb := field_refOK m hw (j + k) hb
    have hsa' := super_refOK m hw (i + k) ha; have hsb' := super_refOK m hw (j + k) hb
    rw [hsa] at hfa hsa'; rw [hsb] at hfb hsb'
    unfold closeSub at e3
    simp only [CT.sub.injEq] at e3
    obtain ⟨efin, ekind, esup, ecnt, efld⟩ := e3
    unfold hashSub
    rw [efin, ekind, ecnt]
    have hsup : ∀ a, (match (m.d (i + k)).super with
        | none => mix a 3 | some x => hashRef m mix (fun x => if _ : x < i then ghash m mix funcKind x else 0) i a (.ref x false)) =
        (match (m.d (j + k)).super with
        | none => mix a 3 | some x => hashRef m mix (fun x => if _ : x < j then ghash m mix funcKind x else 0) j a (.ref x false)) := by
      intro a
      generalize (m.d (i + k)).super = sa at esup hsa'; generalize (m.d (j + k)).super = sb at esup hsb'
      cases sa <;> cases sb <;> simp at esup
      · rfl
      · rename_i x y; exact hr _ _ (hsa' x rfl) (hsb' y rfl) esup a
    dsimp only; rw [hsup]
    split
    · exact foldl_map_congr _ _ _ _ _ _ _ efld (fun f hf g hg' ef a => by
        simp only [Prod.mk.injEq] at ef; exact hr _ _ (hfa f hf) (hfb g hg') ef.1 a)
    · exact foldl_map_congr _ _ _ _ _ _ _ efld (fun f hf g hg' ef a => by
        simp only [Prod.mk.injEq] at ef; rw [hr _ _ (hfa f hf) (hfb g hg') ef.1 a, ef.2])

/-- The canonical map that WAH builds maps every type to the least equivalent type. -/
theorem wah_canonicalMap_least (m : M) (mix : Nat → Nat → Nat) (funcKind : Nat) (hw : WF m) :
    ∀ x < m.n, Least m (canonicalMap m (ghash m mix funcKind)) x :=
  canonicalMap_least m _ hw (ghash_ok m mix funcKind hw)

end Wah.Types
