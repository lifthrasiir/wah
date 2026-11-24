/-
Equivalence of types across modules: `wah_cross_module_type_ref_eq` with `wah_cross_module_rec_group_eq`,
`wah_cross_module_type_eq_in_recgroup` and `wah_cross_module_ref_in_recgroup`.

Types of different modules are equivalent in the spec when their closed forms (`dtOf`) are equal. WAH compares rec
groups of canonical types recursively, remembering pairs of groups found equal (the memo), down to the depth limit
`WAH_MAX_TYPE_RECURSION_DEPTH`. Allocation failures of the memo are not modeled.
-/
import WahProofs.Types

namespace Wah.Cross
open Wah.Types

abbrev Memo := List (Nat × Nat)

/-- Two modules with their canonical maps; `same` is whether they are the same module (`ma == mb` in C). -/
structure Ctx where
  ma : M
  mb : M
  ca : Nat → Nat
  cb : Nat → Nat
  same : Bool

/-- How nested rec groups are compared: by the next level, or not at all at the depth limit. -/
abbrev Nest := Option (Memo → Nat → Nat → Bool × Memo)

/-- `wah_cross_module_ref_in_recgroup` between references from the groups at `sa` and `sb`. -/
def refIn (c : Ctx) (nest : Nest) (sa sb : Nat) (memo : Memo) : VT → VT → Bool × Memo
  | .atom a, .atom b => (a == b, memo)
  | .ref x nx, .ref y ny =>
    if nx != ny then (false, memo) else
    let cx := c.ca x
    let cy := c.cb y
    let ain := decide (sa ≤ cx ∧ cx < sa + c.ma.gsize sa)
    let bin := decide (sb ≤ cy ∧ cy < sb + c.mb.gsize sb)
    if ain && bin then (cx - sa == cy - sb, memo)
    else if ain || bin then (false, memo)
    else if c.same && cx == cy then (true, memo)
    else match nest with
      | none => (false, memo)
      | some g =>
        let gx := c.ma.start cx
        let gy := c.mb.start cy
        if c.ma.gsize cx != c.mb.gsize cy || cx - gx != cy - gy then (false, memo)
        else if (gx, gy) ∈ memo then (true, memo)
        else
          let r := g memo gx gy
          if r.1 then (true, (gx, gy) :: r.2) else (false, r.2)
  | _, _ => (false, memo)

/-- Fields (or params and results) in order, stopping at the first difference. -/
def fieldsEq (c : Ctx) (nest : Nest) (sa sb : Nat) : Memo → List (VT × Bool) → List (VT × Bool) → Bool × Memo
  | memo, [], [] => (true, memo)
  | memo, f :: fs, g :: gs =>
    let r := refIn c nest sa sb memo f.1 g.1
    if !r.1 then (false, r.2)
    else if f.2 != g.2 then (false, r.2)
    else fieldsEq c nest sa sb r.2 fs gs
  | memo, _, _ => (false, memo)

/-- `wah_cross_module_type_eq_in_recgroup` -/
def typeEq (c : Ctx) (nest : Nest) (sa sb : Nat) (memo : Memo) (ia ib : Nat) : Bool × Memo :=
  let da := c.ma.d ia
  let db := c.mb.d ib
  if da.kind != db.kind || da.final != db.final then (false, memo) else
  let r : Bool × Memo := match da.super, db.super with
    | none, none => (true, memo)
    | some x, some y => refIn c nest sa sb memo (.ref x false) (.ref y false)
    | _, _ => (false, memo)
  if !r.1 then (false, r.2)
  else if da.counts != db.counts then (false, r.2)
  else fieldsEq c nest sa sb r.2 da.fields db.fields

/-- The types of the groups in order, stopping at the first difference. -/
def groupLoop (c : Ctx) (nest : Nest) (sa sb : Nat) : Memo → List Nat → Bool × Memo
  | memo, [] => (true, memo)
  | memo, k :: ks =>
    let r := typeEq c nest sa sb memo (sa + k) (sb + k)
    if !r.1 then (false, r.2) else groupLoop c nest sa sb r.2 ks

/-- `wah_cross_module_rec_group_eq` at one level of nesting. -/
def levelEq (c : Ctx) (nest : Nest) (memo : Memo) (sa sb : Nat) : Bool × Memo :=
  if c.ma.gsize sa != c.mb.gsize sb then (false, memo)
  else if c.same && sa == sb then (true, memo)
  else groupLoop c nest sa sb memo (List.range (c.ma.gsize sa))

/-- `wah_cross_module_rec_group_eq` with `r` more levels of nesting allowed. -/
def grpEq (c : Ctx) : Nat → Memo → Nat → Nat → Bool × Memo
  | 0 => levelEq c none
  | r + 1 => levelEq c (some (grpEq c r))

/-- `WAH_MAX_TYPE_RECURSION_DEPTH` -/
def maxDepth : Nat := 256

/-- `wah_cross_module_type_ref_eq` -/
def typeRefEq (c : Ctx) : VT → VT → Bool
  | .atom a, .atom b => a == b
  | .ref x nx, .ref y ny =>
    if nx != ny then false else
    let cx := c.ca x
    let cy := c.cb y
    if c.same && cx == cy then true
    else
      let gx := c.ma.start cx
      let gy := c.mb.start cy
      if c.ma.gsize cx != c.mb.gsize cy || cx - gx != cy - gy then false
      else (grpEq c maxDepth [] gx gy).1
  | _, _ => false

/-- Equivalence of value types across modules in the spec. -/
def SpecEq (c : Ctx) : VT → VT → Prop
  | .atom a, .atom b => a = b
  | .ref x nx, .ref y ny => nx = ny ∧ dtOf c.ma x = dtOf c.mb y
  | _, _ => False

/-! ### Canonical groups -/

/-- `s` starts a rec group of canonical types. -/
def CStart (m : M) (canon : Nat → Nat) (s : Nat) : Prop := s < m.n ∧ m.start s = s ∧ canon s = s

theorem canon_member (m : M) (canon : Nat → Nat) (hw : WF m) (hl : ∀ x < m.n, Least m canon x)
    (s k : Nat) (hs : CStart m canon s) (hk : k < m.gsize s) : canon (s + k) = s + k := by
  obtain ⟨hsn, hss, hcs⟩ := hs
  obtain ⟨hn, hst, _⟩ := start_in m hw s k hsn hss hk
  obtain ⟨h1, h2, _⟩ := hl (s + k) hn
  rcases Nat.lt_or_ge (canon (s + k)) (s + k) with hlt | hge
  · exfalso
    generalize hy : canon (s + k) = y at h1 h2 hlt
    have hyn : y < m.n := by omega
    unfold dtOf at h2; simp only [Prod.mk.injEq, hst] at h2
    have hsy := (hw.group y hyn).1
    have : m.start y < s := by omega
    have e : dtOf m (m.start y) = dtOf m s := by
      unfold dtOf; rw [start_start m hw y hyn, hss, h2.1]; simp
    have ls := (hl s hsn).2.2; rw [hcs] at ls
    exact ls _ this e
  · omega

theorem canon_start (m : M) (canon : Nat → Nat) (hw : WF m) (hl : ∀ x < m.n, Least m canon x)
    (x : Nat) (hx : x < m.n) : CStart m canon (m.start (canon x)) := by
  obtain ⟨h1, h2, h3⟩ := hl x hx
  have hcn : canon x < m.n := by omega
  have hg := hw.group (canon x) hcn
  generalize hgx : m.start (canon x) = g at hg
  have hgn : g < m.n := by omega
  have hsg : m.start g = g := by rw [← hgx]; exact start_start m hw _ hcn
  refine ⟨hgn, hsg, ?_⟩
  obtain ⟨h4, h5, _⟩ := hl g hgn
  rcases Nat.lt_or_ge (canon g) g with hlt | hge
  · exfalso
    generalize hy : canon g = y at h4 h5 hlt
    have hyn : y < m.n := by omega
    unfold dtOf at h5; simp only [Prod.mk.injEq, hsg, Nat.sub_self] at h5
    have hy0 : m.start y = y := by have := (hw.group y hyn).1; omega
    rw [hy0] at h5
    have hsz : m.gsize y = m.gsize g := by
      have := congrArg List.length h5.1; simp [closeGroup_eq] at this; exact this
    have hgs := gsize_start m hw _ hcn; rw [hgx] at hgs
    obtain ⟨hyk, hsyk, _⟩ := start_in m hw y (canon x - g) hyn hy0 (by rw [hsz, hgs]; omega)
    have e : dtOf m (y + (canon x - g)) = dtOf m x := by
      rw [← h2]; unfold dtOf; rw [hsyk, h5.1, hgx]; simp
    exact h3 _ (by omega) e
  · omega

/-! ### Soundness -/

/-- Assumptions on the modules: valid, with canonical maps like `canonical_map`. -/
structure Good (c : Ctx) : Prop where
  wa : WF c.ma
  wb : WF c.mb
  la : ∀ x < c.ma.n, Least c.ma c.ca x
  lb : ∀ x < c.mb.n, Least c.mb c.cb x
  same : c.same = true → c.ma = c.mb ∧ c.ca = c.cb

/-- Pairs in the memo are equal rec groups. -/
def MemoOK (c : Ctx) (memo : Memo) : Prop := ∀ p ∈ memo, closeGroup c.ma p.1 = closeGroup c.mb p.2

/-- Comparisons of canonical rec groups that keep the memo and only find equal groups equal. -/
def SoundG (c : Ctx) (g : Memo → Nat → Nat → Bool × Memo) : Prop :=
  ∀ memo sa sb, MemoOK c memo → CStart c.ma c.ca sa → CStart c.mb c.cb sb →
    MemoOK c (g memo sa sb).2 ∧ ((g memo sa sb).1 = true → closeGroup c.ma sa = closeGroup c.mb sb)

def SoundNest (c : Ctx) : Nest → Prop
  | none => True
  | some g => SoundG c g

/-- References in a canonical group, canonicalized, are within the group exactly when they were. -/
theorem in_canon (m : M) (canon : Nat → Nat) (hw : WF m) (hl : ∀ x < m.n, Least m canon x) (s x : Nat)
    (hs : CStart m canon s) (hx : x < s + m.gsize s) :
    (s ≤ canon x ∧ canon x < s + m.gsize s) ↔ (s ≤ x ∧ x < s + m.gsize s) := by
  have hg := hw.group s hs.1; rw [hs.2.1] at hg
  by_cases h : s ≤ x
  · have := canon_member m canon hw hl s (x - s) hs (by omega)
    rw [show s + (x - s) = x by omega] at this; rw [this]
  · have := (hl x (by omega)).1; constructor <;> intro h' <;> omega

theorem refIn_sound (c : Ctx) (hc : Good c) (nest : Nest) (hn : SoundNest c nest) (sa sb : Nat)
    (hsa : CStart c.ma c.ca sa) (hsb : CStart c.mb c.cb sb) (memo : Memo) (hm : MemoOK c memo)
    (a b : VT) (ha : RefOK c.ma sa a) (hb : RefOK c.mb sb b) :
    MemoOK c (refIn c nest sa sb memo a b).2 ∧
    ((refIn c nest sa sb memo a b).1 = true → cref c.ma sa a = cref c.mb sb b) := by
  cases a with
  | atom a => cases b with
    | atom b => simp only [refIn, beq_iff_eq]; exact ⟨hm, fun h => by simp [cref, h]⟩
    | ref => simp only [refIn]; exact ⟨hm, fun h => by cases h⟩
  | ref x nx => cases b with
    | atom => simp only [refIn]; exact ⟨hm, fun h => by cases h⟩
    | ref y ny =>
      simp only [RefOK] at ha hb
      have ga := hc.wa.group sa hsa.1; rw [hsa.2.1] at ga
      have gb := hc.wb.group sb hsb.1; rw [hsb.2.1] at gb
      have ia := in_canon c.ma c.ca hc.wa hc.la sa x hsa ha
      have ib := in_canon c.mb c.cb hc.wb hc.lb sb y hsb hb
      -- Closed forms of references outside the groups
      have oa : ¬(sa ≤ x ∧ x < sa + c.ma.gsize sa) → cref c.ma sa (.ref x nx) = .dt (dtOf c.ma x).1 (dtOf c.ma x).2 nx := by
        intro h; have := (hc.wa.group x (by omega)).1
        simp only [cref, h, ite_false, show c.ma.start x < sa by omega, dite_true]; rfl
      have ob : ¬(sb ≤ y ∧ y < sb + c.mb.gsize sb) → cref c.mb sb (.ref y ny) = .dt (dtOf c.mb y).1 (dtOf c.mb y).2 ny := by
        intro h; have := (hc.wb.group y (by omega)).1
        simp only [cref, h, ite_false, show c.mb.start y < sb by omega, dite_true]; rfl
      unfold refIn
      by_cases hnn : nx = ny
      · subst hnn
        simp only [bne_self_eq_false, Bool.false_eq_true, ite_false]
        by_cases hxa : sa ≤ x ∧ x < sa + c.ma.gsize sa <;> by_cases hyb : sb ≤ y ∧ y < sb + c.mb.gsize sb
        · have e1 : decide (sa ≤ c.ca x ∧ c.ca x < sa + c.ma.gsize sa) = true := by simp only [decide_eq_true_eq]; exact ia.mpr hxa
          have e2 : decide (sb ≤ c.cb y ∧ c.cb y < sb + c.mb.gsize sb) = true := by simp only [decide_eq_true_eq]; exact ib.mpr hyb
          have cx := canon_member c.ma c.ca hc.wa hc.la sa (x - sa) hsa (by omega)
          have cy := canon_member c.mb c.cb hc.wb hc.lb sb (y - sb) hsb (by omega)
          rw [show sa + (x - sa) = x by omega] at cx; rw [show sb + (y - sb) = y by omega] at cy
          simp only [e1, e2, Bool.and_self, ite_true, beq_iff_eq]
          refine ⟨hm, fun h => ?_⟩
          rw [cx, cy] at h
          simp only [cref, hxa, hyb, and_self, ite_true, h]
        · have e1 : decide (sa ≤ c.ca x ∧ c.ca x < sa + c.ma.gsize sa) = true := by simp only [decide_eq_true_eq]; exact ia.mpr hxa
          have e2 : decide (sb ≤ c.cb y ∧ c.cb y < sb + c.mb.gsize sb) = false := by simp only [decide_eq_false_iff_not]; exact fun h => hyb (ib.mp h)
          simp only [e1, e2, Bool.and_false, Bool.or_false, Bool.false_eq_true, ite_false, ite_true]
          exact ⟨hm, fun h => by cases h⟩
        · have e1 : decide (sa ≤ c.ca x ∧ c.ca x < sa + c.ma.gsize sa) = false := by simp only [decide_eq_false_iff_not]; exact fun h => hxa (ia.mp h)
          have e2 : decide (sb ≤ c.cb y ∧ c.cb y < sb + c.mb.gsize sb) = true := by simp only [decide_eq_true_eq]; exact ib.mpr hyb
          simp only [e1, e2, Bool.false_and, Bool.false_or, Bool.false_eq_true, ite_false, ite_true]
          exact ⟨hm, fun h => by cases h⟩
        · have e1 : decide (sa ≤ c.ca x ∧ c.ca x < sa + c.ma.gsize sa) = false := by simp only [decide_eq_false_iff_not]; exact fun h => hxa (ia.mp h)
          have e2 : decide (sb ≤ c.cb y ∧ c.cb y < sb + c.mb.gsize sb) = false := by simp only [decide_eq_false_iff_not]; exact fun h => hyb (ib.mp h)
          simp only [e1, e2, Bool.false_and, Bool.false_or, Bool.false_eq_true, ite_false]
          rw [oa hxa, ob hyb]
          have hxn : x < c.ma.n := by omega
          have hyn : y < c.mb.n := by omega
          have lx := hc.la x hxn; have ly := hc.lb y hyn
          -- The closed forms are equal when the canonical types are
          have key : dtOf c.ma (c.ca x) = dtOf c.mb (c.cb y) → (CT.dt (dtOf c.ma x).1 (dtOf c.ma x).2 nx =
              CT.dt (dtOf c.mb y).1 (dtOf c.mb y).2 nx) := by
            intro e; rw [← lx.2.1, ← ly.2.1, e]
          split
          · rename_i hs
            simp only [Bool.and_eq_true, beq_iff_eq] at hs
            obtain ⟨h1, h2⟩ := hc.same hs.1
            refine ⟨hm, fun _ => key ?_⟩
            rw [hs.2, ← h1]
          · split
            · exact ⟨hm, fun h => by cases h⟩
            · rename_i g
              simp only [SoundNest] at hn
              have hcx : c.ca x < c.ma.n := by have := lx.1; omega
              have hcy : c.cb y < c.mb.n := by have := ly.1; omega
              have gx := canon_start c.ma c.ca hc.wa hc.la x hxn
              have gy := canon_start c.mb c.cb hc.wb hc.lb y hyn
              have dx : dtOf c.ma (c.ca x) = (closeGroup c.ma (c.ma.start (c.ca x)), c.ca x - c.ma.start (c.ca x)) := rfl
              have dy : dtOf c.mb (c.cb y) = (closeGroup c.mb (c.mb.start (c.cb y)), c.cb y - c.mb.start (c.cb y)) := rfl
              split
              · exact ⟨hm, fun h => by cases h⟩
              · rename_i hsz
                simp only [Bool.or_eq_true, bne_iff_ne, ne_eq, not_or, Decidable.not_not] at hsz
                split
                · rename_i hmem
                  refine ⟨hm, fun _ => key ?_⟩
                  rw [dx, dy, hm _ hmem, hsz.2]
                · obtain ⟨hm', hg⟩ := hn memo _ _ hm gx gy
                  split
                  · rename_i hr
                    refine ⟨?_, fun _ => key ?_⟩
                    · intro p hp; simp only [List.mem_cons] at hp
                      rcases hp with rfl | hp
                      · exact hg hr
                      · exact hm' p hp
                    · rw [dx, dy, hg hr, hsz.2]
                  · exact ⟨hm', fun h => by cases h⟩
      · have : (nx != ny) = true := by simp [hnn]
        simp only [this, ite_true]; exact ⟨hm, fun h => by cases h⟩

theorem fieldsEq_sound (c : Ctx) (hc : Good c) (nest : Nest) (hn : SoundNest c nest) (sa sb : Nat)
    (hsa : CStart c.ma c.ca sa) (hsb : CStart c.mb c.cb sb) :
    ∀ (memo : Memo) (fs gs : List (VT × Bool)), MemoOK c memo →
    (∀ f ∈ fs, RefOK c.ma sa f.1) → (∀ g ∈ gs, RefOK c.mb sb g.1) →
    MemoOK c (fieldsEq c nest sa sb memo fs gs).2 ∧ ((fieldsEq c nest sa sb memo fs gs).1 = true →
      fs.map (fun f => (cref c.ma sa f.1, f.2)) = gs.map (fun f => (cref c.mb sb f.1, f.2)))
  | memo, [], [], hm, _, _ => ⟨hm, fun _ => rfl⟩
  | memo, [], _ :: _, hm, _, _ => ⟨hm, fun h => by cases h⟩
  | memo, _ :: _, [], hm, _, _ => ⟨hm, fun h => by cases h⟩
  | memo, f :: fs, g :: gs, hm, hf, hg => by
    have r := refIn_sound c hc nest hn sa sb hsa hsb memo hm f.1 g.1 (hf f (by simp)) (hg g (by simp))
    unfold fieldsEq
    dsimp only
    split
    · exact ⟨r.1, fun h => by cases h⟩
    · rename_i h1; simp only [Bool.not_eq_true', Bool.not_eq_false] at h1
      split
      · exact ⟨r.1, fun h => by cases h⟩
      · rename_i h2; simp only [bne_iff_ne, ne_eq, Decidable.not_not] at h2
        have ih := fieldsEq_sound c hc nest hn sa sb hsa hsb _ fs gs r.1
          (fun f' h => hf f' (by simp [h])) (fun g' h => hg g' (by simp [h]))
        refine ⟨ih.1, fun h => ?_⟩
        simp only [List.map_cons, List.cons.injEq, Prod.mk.injEq]
        exact ⟨⟨r.2 h1, h2⟩, ih.2 h⟩

theorem typeEq_sound (c : Ctx) (hc : Good c) (nest : Nest) (hn : SoundNest c nest) (sa sb : Nat)
    (hsa : CStart c.ma c.ca sa) (hsb : CStart c.mb c.cb sb) (memo : Memo) (hm : MemoOK c memo)
    (ia ib : Nat) (hia : ia < c.ma.n) (hib : ib < c.mb.n) (hsia : c.ma.start ia = sa) (hsib : c.mb.start ib = sb) :
    MemoOK c (typeEq c nest sa sb memo ia ib).2 ∧
    ((typeEq c nest sa sb memo ia ib).1 = true → closeSub c.ma sa ia = closeSub c.mb sb ib) := by
  have fa := field_refOK c.ma hc.wa ia hia; have fb := field_refOK c.mb hc.wb ib hib
  have pa := super_refOK c.ma hc.wa ia hia; have pb := super_refOK c.mb hc.wb ib hib
  rw [hsia] at fa pa; rw [hsib] at fb pb
  unfold typeEq
  dsimp only
  split
  · exact ⟨hm, fun h => by cases h⟩
  · rename_i hk; simp only [Bool.or_eq_true, bne_iff_ne, ne_eq, not_or, Decidable.not_not] at hk
    -- The supertypes
    have hs : MemoOK c (match (c.ma.d ia).super, (c.mb.d ib).super with
        | none, none => (true, memo)
        | some x, some y => refIn c nest sa sb memo (.ref x false) (.ref y false)
        | _, _ => (false, memo)).2 ∧
      ((match (c.ma.d ia).super, (c.mb.d ib).super with
        | none, none => (true, memo)
        | some x, some y => refIn c nest sa sb memo (.ref x false) (.ref y false)
        | _, _ => (false, memo)).1 = true →
        (c.ma.d ia).super.map (fun x => cref c.ma sa (.ref x false)) =
        (c.mb.d ib).super.map (fun x => cref c.mb sb (.ref x false))) := by
      generalize (c.ma.d ia).super = xa at pa; generalize (c.mb.d ib).super = xb at pb
      cases xa <;> cases xb
      · exact ⟨hm, fun _ => rfl⟩
      · exact ⟨hm, fun h => by cases h⟩
      · exact ⟨hm, fun h => by cases h⟩
      · rename_i x y
        have r := refIn_sound c hc nest hn sa sb hsa hsb memo hm _ _ (pa x rfl) (pb y rfl)
        exact ⟨r.1, fun h => by simp only [Option.map_some, Option.some.injEq]; exact r.2 h⟩
    generalize (match (c.ma.d ia).super, (c.mb.d ib).super with
        | none, none => (true, memo)
        | some x, some y => refIn c nest sa sb memo (.ref x false) (.ref y false)
        | _, _ => (false, memo)) = rs at hs
    split
    · exact ⟨hs.1, fun h => by cases h⟩
    · rename_i h1; simp only [Bool.not_eq_true', Bool.not_eq_false] at h1
      split
      · exact ⟨hs.1, fun h => by cases h⟩
      · rename_i h2; simp only [bne_iff_ne, ne_eq, Decidable.not_not] at h2
        have r := fieldsEq_sound c hc nest hn sa sb hsa hsb rs.2 _ _ hs.1 fa fb
        refine ⟨r.1, fun h => ?_⟩
        unfold closeSub
        simp only [CT.sub.injEq]
        exact ⟨hk.2, hk.1, hs.2 h1, h2, r.2 h⟩

theorem groupLoop_sound (c : Ctx) (hc : Good c) (nest : Nest) (hn : SoundNest c nest) (sa sb : Nat)
    (hsa : CStart c.ma c.ca sa) (hsb : CStart c.mb c.cb sb) (hsz : c.ma.gsize sa = c.mb.gsize sb) :
    ∀ (ks : List Nat) (memo : Memo), MemoOK c memo → (∀ k ∈ ks, k < c.ma.gsize sa) →
    MemoOK c (groupLoop c nest sa sb memo ks).2 ∧ ((groupLoop c nest sa sb memo ks).1 = true →
      ∀ k ∈ ks, closeSub c.ma sa (sa + k) = closeSub c.mb sb (sb + k))
  | [], memo, hm, _ => ⟨hm, fun _ k hk => by cases hk⟩
  | k :: ks, memo, hm, hk => by
    have hk0 := hk k (by simp)
    obtain ⟨ha, hsa', _⟩ := start_in c.ma hc.wa sa k hsa.1 hsa.2.1 hk0
    obtain ⟨hb, hsb', _⟩ := start_in c.mb hc.wb sb k hsb.1 hsb.2.1 (by omega)
    have r := typeEq_sound c hc nest hn sa sb hsa hsb memo hm _ _ ha hb hsa' hsb'
    unfold groupLoop
    dsimp only
    split
    · exact ⟨r.1, fun h => by cases h⟩
    · rename_i h1; simp only [Bool.not_eq_true', Bool.not_eq_false] at h1
      have ih := groupLoop_sound c hc nest hn sa sb hsa hsb hsz ks _ r.1 (fun k' h => hk k' (by simp [h]))
      refine ⟨ih.1, fun h k' hk' => ?_⟩
      simp only [List.mem_cons] at hk'
      rcases hk' with rfl | hk'
      · exact r.2 h1
      · exact ih.2 h k' hk'

theorem levelEq_sound (c : Ctx) (hc : Good c) (nest : Nest) (hn : SoundNest c nest) : SoundG c (levelEq c nest) := by
  intro memo sa sb hm hsa hsb
  unfold levelEq
  split
  · exact ⟨hm, fun h => by cases h⟩
  · rename_i hsz; simp only [bne_iff_ne, ne_eq, Decidable.not_not] at hsz
    split
    · rename_i hs; simp only [Bool.and_eq_true, beq_iff_eq] at hs
      obtain ⟨h1, _⟩ := hc.same hs.1
      refine ⟨hm, fun _ => ?_⟩
      rw [hs.2, h1]
    · have r := groupLoop_sound c hc nest hn sa sb hsa hsb hsz (List.range (c.ma.gsize sa)) memo hm
        (fun k h => by simpa using h)
      refine ⟨r.1, fun h => ?_⟩
      rw [closeGroup_eq, closeGroup_eq, map_range_eq_iff]
      exact ⟨hsz, fun k hk => r.2 h k (by simpa using hk)⟩

theorem grpEq_sound (c : Ctx) (hc : Good c) : ∀ r, SoundG c (grpEq c r)
  | 0 => levelEq_sound c hc none trivial
  | r + 1 => levelEq_sound c hc (some (grpEq c r)) (grpEq_sound c hc r)

/-- `wah_cross_module_type_ref_eq` only finds equivalent types equal. -/
theorem typeRefEq_sound (c : Ctx) (hc : Good c) (a b : VT) (ha : ∀ x n, a = .ref x n → x < c.ma.n)
    (hb : ∀ y n, b = .ref y n → y < c.mb.n) (h : typeRefEq c a b = true) : SpecEq c a b := by
  cases a with
  | atom a => cases b with
    | atom b => simpa [typeRefEq, SpecEq] using h
    | ref => simp [typeRefEq] at h
  | ref x nx => cases b with
    | atom => simp [typeRefEq] at h
    | ref y ny =>
      have hxn := ha x nx rfl; have hyn := hb y ny rfl
      have lx := hc.la x hxn; have ly := hc.lb y hyn
      unfold typeRefEq at h
      simp only [SpecEq]
      by_cases hnn : nx = ny
      · subst hnn
        simp only [bne_self_eq_false, Bool.false_eq_true, ite_false] at h
        refine ⟨rfl, ?_⟩
        rw [← lx.2.1, ← ly.2.1]
        split at h
        · rename_i hs; simp only [Bool.and_eq_true, beq_iff_eq] at hs
          obtain ⟨h1, _⟩ := hc.same hs.1
          rw [hs.2, ← h1]
        · split at h
          · cases h
          · rename_i hsz; simp only [Bool.or_eq_true, bne_iff_ne, ne_eq, not_or, Decidable.not_not] at hsz
            have gx := canon_start c.ma c.ca hc.wa hc.la x hxn
            have gy := canon_start c.mb c.cb hc.wb hc.lb y hyn
            have e := (grpEq_sound c hc maxDepth [] _ _ (fun p hp => by cases hp) gx gy).2 h
            show (closeGroup c.ma _, _) = (closeGroup c.mb _, _)
            rw [e, hsz.2]
      · simp [hnn] at h

/-! ### Completeness -/

/-- Levels of nested rec groups that comparing the group at `s` goes down at most. -/
def need (m : M) (canon : Nat → Nat) (s : Nat) : Nat :=
  ((List.range (m.gsize s)).flatMap fun k => (m.d (s + k)).refs).foldl
    (fun acc x => if h : x < s ∧ m.start (canon x) < s then max acc (need m canon (m.start (canon x)) + 1) else acc) 0
termination_by s
decreasing_by exact h.2

theorem foldl_guard_max (L : List Nat) (P : Nat → Prop) [DecidablePred P] (v : Nat → Nat) :
    ∀ acc, acc ≤ L.foldl (fun acc x => if P x then max acc (v x) else acc) acc ∧
      ∀ x ∈ L, P x → v x ≤ L.foldl (fun acc x => if P x then max acc (v x) else acc) acc := by
  induction L with
  | nil => intro acc; simp
  | cons y L ih =>
    intro acc
    simp only [List.foldl_cons, List.mem_cons]
    by_cases hp : P y
    · simp only [hp, ite_true]
      have := ih (max acc (v y))
      refine ⟨by omega, fun x hx hpx => ?_⟩
      rcases hx with rfl | hx
      · omega
      · exact this.2 x hx hpx
    · simp only [hp, ite_false]
      have := ih acc
      refine ⟨this.1, fun x hx hpx => ?_⟩
      rcases hx with rfl | hx
      · exact absurd hpx hp
      · exact this.2 x hx hpx

/-- Outer references of a group need one level less than the group. -/
theorem need_ref (m : M) (canon : Nat → Nat) (s k x : Nat) (hk : k < m.gsize s) (hx : x ∈ (m.d (s + k)).refs)
    (h1 : x < s) (h2 : m.start (canon x) < s) : need m canon (m.start (canon x)) + 1 ≤ need m canon s := by
  conv => rhs; rw [need]; simp only [dite_eq_ite]
  have hm : x ∈ (List.range (m.gsize s)).flatMap fun k => (m.d (s + k)).refs :=
    List.mem_flatMap.mpr ⟨k, List.mem_range.mpr hk, hx⟩
  have := (foldl_guard_max _ (fun x => x < s ∧ m.start (canon x) < s) (fun x => need m canon (m.start (canon x)) + 1) 0).2
    x hm ⟨h1, h2⟩
  exact this

/-- Comparisons that find equal canonical groups equal, when they need at most `d` levels. -/
def CompleteG (c : Ctx) (g : Memo → Nat → Nat → Bool × Memo) (d : Nat) : Prop :=
  ∀ memo sa sb, CStart c.ma c.ca sa → CStart c.mb c.cb sb → need c.ma c.ca sa ≤ d →
    closeGroup c.ma sa = closeGroup c.mb sb → (g memo sa sb).1 = true

/-- Nested groups can be compared for an outer reference `x` from the group at `sa`. -/
def NestFor (c : Ctx) (nest : Nest) (x : Nat) : Prop :=
  ∃ g d, nest = some g ∧ CompleteG c g d ∧ need c.ma c.ca (c.ma.start (c.ca x)) ≤ d

theorem cref_out (m : M) (hw : WF m) (s x : Nat) (n : Bool) (hs : s < m.n ∧ m.start s = s)
    (hx : x < s + m.gsize s) (h : ¬(s ≤ x ∧ x < s + m.gsize s)) :
    cref m s (.ref x n) = .dt (dtOf m x).1 (dtOf m x).2 n := by
  have hg := hw.group s hs.1; rw [hs.2] at hg
  have := (hw.group x (by omega)).1
  simp only [cref, h, ite_false, show m.start x < s by omega, dite_true]; rfl

theorem refIn_complete (c : Ctx) (hc : Good c) (nest : Nest) (sa sb : Nat)
    (hsa : CStart c.ma c.ca sa) (hsb : CStart c.mb c.cb sb) (memo : Memo)
    (a b : VT) (ha : RefOK c.ma sa a) (hb : RefOK c.mb sb b) (e : cref c.ma sa a = cref c.mb sb b)
    (hn : ∀ x n, a = .ref x n → x < sa → NestFor c nest x) : (refIn c nest sa sb memo a b).1 = true := by
  have ga := hc.wa.group sa hsa.1; rw [hsa.2.1] at ga
  have gb := hc.wb.group sb hsb.1; rw [hsb.2.1] at gb
  cases a with
  | atom a => cases b with
    | atom b => simp only [cref, CT.atom.injEq] at e; simp [refIn, e]
    | ref y ny =>
      simp only [RefOK] at hb
      by_cases hy : sb ≤ y ∧ y < sb + c.mb.gsize sb
      · simp [cref, hy] at e
      · rw [cref_out c.mb hc.wb sb y ny ⟨hsb.1, hsb.2.1⟩ hb hy] at e; simp [cref] at e
  | ref x nx => cases b with
    | atom b =>
      simp only [RefOK] at ha
      by_cases hx : sa ≤ x ∧ x < sa + c.ma.gsize sa
      · simp [cref, hx] at e
      · rw [cref_out c.ma hc.wa sa x nx ⟨hsa.1, hsa.2.1⟩ ha hx] at e; simp [cref] at e
    | ref y ny =>
      simp only [RefOK] at ha hb
      have ia := in_canon c.ma c.ca hc.wa hc.la sa x hsa ha
      have ib := in_canon c.mb c.cb hc.wb hc.lb sb y hsb hb
      by_cases hx : sa ≤ x ∧ x < sa + c.ma.gsize sa <;> by_cases hy : sb ≤ y ∧ y < sb + c.mb.gsize sb
      · simp only [cref, hx, hy, and_self, ite_true, CT.inner.injEq] at e
        obtain ⟨e1, rfl⟩ := e
        have cx := canon_member c.ma c.ca hc.wa hc.la sa (x - sa) hsa (by omega)
        have cy := canon_member c.mb c.cb hc.wb hc.lb sb (y - sb) hsb (by omega)
        rw [show sa + (x - sa) = x by omega] at cx; rw [show sb + (y - sb) = y by omega] at cy
        simp only [refIn, bne_self_eq_false, Bool.false_eq_true, ite_false, cx, cy]
        simp [hx, hy, e1]
      · rw [cref_out c.mb hc.wb sb y ny ⟨hsb.1, hsb.2.1⟩ hb hy] at e; simp [cref, hx] at e
      · rw [cref_out c.ma hc.wa sa x nx ⟨hsa.1, hsa.2.1⟩ ha hx] at e; simp [cref, hy] at e
      · rw [cref_out c.ma hc.wa sa x nx ⟨hsa.1, hsa.2.1⟩ ha hx,
          cref_out c.mb hc.wb sb y ny ⟨hsb.1, hsb.2.1⟩ hb hy] at e
        simp only [CT.dt.injEq] at e
        obtain ⟨e1, e2, rfl⟩ := e
        have ed : dtOf c.ma x = dtOf c.mb y := Prod.ext e1 e2
        have hxn : x < c.ma.n := by omega
        have hyn : y < c.mb.n := by omega
        have lx := hc.la x hxn; have ly := hc.lb y hyn
        have ec : dtOf c.ma (c.ca x) = dtOf c.mb (c.cb y) := by rw [lx.2.1, ly.2.1, ed]
        have e1' : decide (sa ≤ c.ca x ∧ c.ca x < sa + c.ma.gsize sa) = false := by
          simp only [decide_eq_false_iff_not]; exact fun h => hx (ia.mp h)
        have e2' : decide (sb ≤ c.cb y ∧ c.cb y < sb + c.mb.gsize sb) = false := by
          simp only [decide_eq_false_iff_not]; exact fun h => hy (ib.mp h)
        unfold refIn
        simp only [bne_self_eq_false, Bool.false_eq_true, ite_false, e1', e2', Bool.and_false, Bool.or_false]
        split
        · rfl
        · obtain ⟨g, d, rfl, hg, hd⟩ := hn x nx rfl (by omega)
          simp only
          have hcx : c.ca x < c.ma.n := by have := lx.1; omega
          have hcy : c.cb y < c.mb.n := by have := ly.1; omega
          unfold dtOf at ec; simp only [Prod.mk.injEq] at ec
          have hsz : c.ma.gsize (c.ca x) = c.mb.gsize (c.cb y) := by
            have := congrArg List.length ec.1
            simp only [closeGroup_eq, List.length_map, List.length_range] at this
            rw [gsize_start c.ma hc.wa _ hcx, gsize_start c.mb hc.wb _ hcy] at this; exact this
          simp only [hsz, ec.2, bne_self_eq_false, Bool.or_self, Bool.false_eq_true, ite_false]
          split
          · rfl
          · have := hg memo _ _ (canon_start c.ma c.ca hc.wa hc.la x hxn) (canon_start c.mb c.cb hc.wb hc.lb y hyn)
              hd ec.1
            simp [this]

theorem fieldsEq_complete (c : Ctx) (hc : Good c) (nest : Nest) (sa sb : Nat)
    (hsa : CStart c.ma c.ca sa) (hsb : CStart c.mb c.cb sb) :
    ∀ (memo : Memo) (fs gs : List (VT × Bool)),
    (∀ f ∈ fs, RefOK c.ma sa f.1) → (∀ g ∈ gs, RefOK c.mb sb g.1) →
    (∀ f ∈ fs, ∀ x n, f.1 = .ref x n → x < sa → NestFor c nest x) →
    fs.map (fun f => (cref c.ma sa f.1, f.2)) = gs.map (fun f => (cref c.mb sb f.1, f.2)) →
    (fieldsEq c nest sa sb memo fs gs).1 = true
  | memo, [], [], _, _, _, _ => rfl
  | memo, [], _ :: _, _, _, _, e => by simp at e
  | memo, _ :: _, [], _, _, _, e => by simp at e
  | memo, f :: fs, g :: gs, hf, hg, hn, e => by
    simp only [List.map_cons, List.cons.injEq, Prod.mk.injEq] at e
    have r := refIn_complete c hc nest sa sb hsa hsb memo f.1 g.1 (hf f (by simp)) (hg g (by simp)) e.1.1
      (fun x n h => hn f (by simp) x n h)
    unfold fieldsEq; dsimp only
    simp only [r, Bool.not_true, Bool.false_eq_true, ite_false, e.1.2, bne_self_eq_false]
    exact fieldsEq_complete c hc nest sa sb hsa hsb _ fs gs (fun f' h => hf f' (by simp [h]))
      (fun g' h => hg g' (by simp [h])) (fun f' h => hn f' (by simp [h])) e.2

/-- Outer references of all types of the group at `s` can be compared when needed. -/
def NestOK (c : Ctx) (nest : Nest) (d : Nat) : Prop :=
  ∀ s, CStart c.ma c.ca s → need c.ma c.ca s ≤ d →
    ∀ k < c.ma.gsize s, ∀ x ∈ (c.ma.d (s + k)).refs, x < s → NestFor c nest x

theorem typeEq_complete (c : Ctx) (hc : Good c) (nest : Nest) (d : Nat) (hn : NestOK c nest d) (sa sb : Nat)
    (hsa : CStart c.ma c.ca sa) (hsb : CStart c.mb c.cb sb) (hd : need c.ma c.ca sa ≤ d) (memo : Memo)
    (k : Nat) (hk : k < c.ma.gsize sa) (hkb : k < c.mb.gsize sb)
    (e : closeSub c.ma sa (sa + k) = closeSub c.mb sb (sb + k)) :
    (typeEq c nest sa sb memo (sa + k) (sb + k)).1 = true := by
  obtain ⟨ha, hsa', _⟩ := start_in c.ma hc.wa sa k hsa.1 hsa.2.1 hk
  obtain ⟨hb, hsb', _⟩ := start_in c.mb hc.wb sb k hsb.1 hsb.2.1 hkb
  have fa := field_refOK c.ma hc.wa _ ha; have fb := field_refOK c.mb hc.wb _ hb
  have pa := super_refOK c.ma hc.wa _ ha; have pb := super_refOK c.mb hc.wb _ hb
  rw [hsa'] at fa pa; rw [hsb'] at fb pb
  have hnk := hn sa hsa hd k hk
  unfold closeSub at e
  simp only [CT.sub.injEq] at e
  obtain ⟨efin, ekind, esup, ecnt, efld⟩ := e
  unfold typeEq; dsimp only
  simp only [ekind, efin, bne_self_eq_false, Bool.or_self, Bool.false_eq_true, ite_false, ecnt]
  have hs : (match (c.ma.d (sa + k)).super, (c.mb.d (sb + k)).super with
      | none, none => (true, memo)
      | some x, some y => refIn c nest sa sb memo (.ref x false) (.ref y false)
      | _, _ => (false, memo)).1 = true := by
    have hsr : ∀ x, (c.ma.d (sa + k)).super = some x → x ∈ (c.ma.d (sa + k)).refs := by
      intro x h; simp [TD.refs, h]
    generalize hxa : (c.ma.d (sa + k)).super = xa at esup pa hsr
    generalize hxb : (c.mb.d (sb + k)).super = xb at esup pb
    cases xa <;> cases xb <;> simp at esup
    · rfl
    · rename_i x y
      exact refIn_complete c hc nest sa sb hsa hsb memo _ _ (pa x rfl) (pb y rfl) esup
        (fun x' n h hlt => by cases h; exact hnk x (hsr x rfl) hlt)
  generalize (match (c.ma.d (sa + k)).super, (c.mb.d (sb + k)).super with
      | none, none => (true, memo)
      | some x, some y => refIn c nest sa sb memo (.ref x false) (.ref y false)
      | _, _ => (false, memo)) = rs at hs
  simp only [hs, Bool.not_true, Bool.false_eq_true, ite_false]
  refine fieldsEq_complete c hc nest sa sb hsa hsb _ _ _ fa fb (fun f hf x n h hlt => ?_) efld
  apply hnk x _ hlt
  simp only [TD.refs, List.mem_append, List.mem_filterMap]
  right; exact ⟨f, hf, by rw [h]⟩

theorem groupLoop_complete (c : Ctx) (hc : Good c) (nest : Nest) (d : Nat) (hn : NestOK c nest d) (sa sb : Nat)
    (hsa : CStart c.ma c.ca sa) (hsb : CStart c.mb c.cb sb) (hd : need c.ma c.ca sa ≤ d)
    (hsz : c.ma.gsize sa = c.mb.gsize sb) :
    ∀ (ks : List Nat) (memo : Memo), (∀ k ∈ ks, k < c.ma.gsize sa) →
    (∀ k ∈ ks, closeSub c.ma sa (sa + k) = closeSub c.mb sb (sb + k)) →
    (groupLoop c nest sa sb memo ks).1 = true
  | [], _, _, _ => rfl
  | k :: ks, memo, hk, he => by
    have r := typeEq_complete c hc nest d hn sa sb hsa hsb hd memo k (hk k (by simp)) (by have := hk k (by simp); omega)
      (he k (by simp))
    unfold groupLoop; dsimp only
    simp only [r, Bool.not_true, Bool.false_eq_true, ite_false]
    exact groupLoop_complete c hc nest d hn sa sb hsa hsb hd hsz ks _ (fun k' h => hk k' (by simp [h]))
      (fun k' h => he k' (by simp [h]))

theorem levelEq_complete (c : Ctx) (hc : Good c) (nest : Nest) (d : Nat) (hn : NestOK c nest d) :
    CompleteG c (levelEq c nest) d := by
  intro memo sa sb hsa hsb hd e
  have e' := e
  rw [closeGroup_eq, closeGroup_eq, map_range_eq_iff] at e'
  obtain ⟨hsz, hk⟩ := e'
  unfold levelEq
  simp only [hsz, bne_self_eq_false, Bool.false_eq_true, ite_false]
  split
  · rfl
  · rw [← hsz]
    exact groupLoop_complete c hc nest d hn sa sb hsa hsb hd hsz _ memo (fun k h => by simpa using h)
      (fun k h => hk k (by simpa using h))

theorem grpEq_complete (c : Ctx) (hc : Good c) : ∀ r, CompleteG c (grpEq c r) r
  | 0 => levelEq_complete c hc none 0 (fun s hs hd k hk x hx hlt => by
      have hcx := (hc.la x (by have := hc.wa.group s hs.1; rw [hs.2.1] at this; omega)).1
      have := (hc.wa.group (c.ca x) (by have := hc.wa.group s hs.1; rw [hs.2.1] at this; omega)).1
      have := need_ref c.ma c.ca s k x hk hx hlt (by omega)
      omega)
  | r + 1 => levelEq_complete c hc _ (r + 1) (fun s hs hd k hk x hx hlt => by
      have hcx := (hc.la x (by have := hc.wa.group s hs.1; rw [hs.2.1] at this; omega)).1
      have := (hc.wa.group (c.ca x) (by have := hc.wa.group s hs.1; rw [hs.2.1] at this; omega)).1
      have := need_ref c.ma c.ca s k x hk hx hlt (by omega)
      exact ⟨grpEq c r, r, rfl, grpEq_complete c hc r, by omega⟩)

/-- `wah_cross_module_type_ref_eq` finds equivalent types equal, unless rec groups are nested more deeply than
`WAH_MAX_TYPE_RECURSION_DEPTH`. -/
theorem typeRefEq_complete (c : Ctx) (hc : Good c) (a b : VT) (ha : ∀ x n, a = .ref x n → x < c.ma.n)
    (hb : ∀ y n, b = .ref y n → y < c.mb.n)
    (hd : ∀ x n, a = .ref x n → need c.ma c.ca (c.ma.start (c.ca x)) ≤ maxDepth)
    (h : SpecEq c a b) : typeRefEq c a b = true := by
  cases a with
  | atom a => cases b with
    | atom b => simp only [SpecEq] at h; simp [typeRefEq, h]
    | ref => simp [SpecEq] at h
  | ref x nx => cases b with
    | atom => simp [SpecEq] at h
    | ref y ny =>
      simp only [SpecEq] at h
      obtain ⟨rfl, ed⟩ := h
      have hxn := ha x nx rfl; have hyn := hb y nx rfl
      have lx := hc.la x hxn; have ly := hc.lb y hyn
      have ec : dtOf c.ma (c.ca x) = dtOf c.mb (c.cb y) := by rw [lx.2.1, ly.2.1, ed]
      have hcx : c.ca x < c.ma.n := by have := lx.1; omega
      have hcy : c.cb y < c.mb.n := by have := ly.1; omega
      unfold dtOf at ec; simp only [Prod.mk.injEq] at ec
      have hsz : c.ma.gsize (c.ca x) = c.mb.gsize (c.cb y) := by
        have := congrArg List.length ec.1
        simp only [closeGroup_eq, List.length_map, List.length_range] at this
        rw [gsize_start c.ma hc.wa _ hcx, gsize_start c.mb hc.wb _ hcy] at this; exact this
      unfold typeRefEq
      simp only [bne_self_eq_false, Bool.false_eq_true, ite_false, hsz, ec.2, Bool.or_self]
      split
      · rfl
      · exact grpEq_complete c hc maxDepth [] _ _ (canon_start c.ma c.ca hc.wa hc.la x hxn)
          (canon_start c.mb c.cb hc.wb hc.lb y hyn) (hd x nx rfl) ec.1

end Wah.Cross
