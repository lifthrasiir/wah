/-
Subtyping of heap and value types over the closed forms of WahProofs/Types.lean, and its algorithmic form, which
`wah_type_is_subtype` follows.

The spec relation `HSub` has the rules of the spec, including transitivity through valid types. Validity is weakened
to `KindsOK` (supertypes have the same kind along the chain), which is all that the proofs need and which validity
of types implies.
-/
import WahProofs.Types

namespace Wah.Subtype
open Wah.Types

/-- Abstract heap types. -/
inductive Abs where
  | any | eq | i31 | struct | array | none | func | nofunc | extern | noextern | exn | noexn
  deriving DecidableEq, Repr

/-- Defined types: a closed rec group and an index into it (`dtOf`). -/
abbrev DT := List CT × Nat

/-- Closed heap types. -/
inductive CH where
  | abs (h : Abs)
  | defd (d : DT)

/-- `wah_comp_type_kind_t` -/
@[simp] def kFunc : Nat := 0x60
@[simp] def kStruct : Nat := 0x5F
@[simp] def kArray : Nat := 0x5E

def kindOf (d : DT) : Nat :=
  match d.1[d.2]? with
  | some (.sub _ k _ _ _) => k
  | _ => 0

/-- The declared supertype of a defined type, unrolled. -/
def superOf (d : DT) : Option DT :=
  match d.1[d.2]? with
  | some (.sub _ _ (some (.inner k _)) _ _) => some (d.1, k)
  | some (.sub _ _ (some (.dt g k _)) _ _) => some (g, k)
  | _ => none

/-- `d'` is `d` or one of its supertypes. -/
inductive Chain : DT → DT → Prop where
  | refl (d : DT) : Chain d d
  | step {d e d' : DT} : superOf d = some e → Chain e d' → Chain d d'

/-- Supertypes keep the kind along the chain. -/
def KindsOK (d : DT) : Prop := ∀ e e', Chain d e → superOf e = some e' → kindOf e' = kindOf e

def ValidH : CH → Prop
  | .abs _ => True
  | .defd d => KindsOK d

/-- Heap subtyping of the spec. -/
inductive HSub : CH → CH → Prop where
  | refl (h : CH) : HSub h h
  | trans {a b c : CH} : ValidH b → HSub a b → HSub b c → HSub a c
  | eq_any : HSub (.abs .eq) (.abs .any)
  | i31_eq : HSub (.abs .i31) (.abs .eq)
  | struct_eq : HSub (.abs .struct) (.abs .eq)
  | array_eq : HSub (.abs .array) (.abs .eq)
  | def_struct {d : DT} : kindOf d = kStruct → HSub (.defd d) (.abs .struct)
  | def_array {d : DT} : kindOf d = kArray → HSub (.defd d) (.abs .array)
  | def_func {d : DT} : kindOf d = kFunc → HSub (.defd d) (.abs .func)
  | def_super {d e : DT} : superOf d = some e → HSub (.defd d) (.defd e)
  | none_ {h : CH} : ValidH h → HSub h (.abs .any) → HSub (.abs .none) h
  | nofunc_ {h : CH} : ValidH h → HSub h (.abs .func) → HSub (.abs .nofunc) h
  | noextern_ {h : CH} : ValidH h → HSub h (.abs .extern) → HSub (.abs .noextern) h
  | noexn_ {h : CH} : ValidH h → HSub h (.abs .exn) → HSub (.abs .noexn) h

/-! ### Algorithmic subtyping, as in `wah_type_is_subtype` -/

/-- Between abstract heap types. -/
def absSub (a b : Abs) : Bool :=
  a == b || match b with
    | .any => a == .eq || a == .i31 || a == .struct || a == .array || a == .none
    | .eq => a == .i31 || a == .struct || a == .array || a == .none
    | .struct | .array | .i31 => a == .none
    | .func => a == .nofunc
    | .extern => a == .noextern
    | .exn => a == .noexn
    | _ => false

/-- From a defined type of kind `k` to an abstract heap type. -/
def defAbs (k : Nat) : Abs → Bool
  | .any | .eq => k == kStruct || k == kArray
  | .struct => k == kStruct
  | .array => k == kArray
  | .func => k == kFunc
  | _ => false

/-- From an abstract heap type to a defined type of kind `k`. -/
def absDef (k : Nat) : Abs → Bool
  | .none => k == kStruct || k == kArray
  | .nofunc => k == kFunc
  | _ => false

def Algo : CH → CH → Prop
  | .abs a, .abs b => absSub a b = true
  | .defd d, .abs b => defAbs (kindOf d) b = true
  | .abs a, .defd d => absDef (kindOf d) a = true
  | .defd d, .defd d' => Chain d d'

/-! ### Chains -/

theorem chain_trans {a b c : DT} (h1 : Chain a b) (h2 : Chain b c) : Chain a c := by
  induction h1 with
  | refl => exact h2
  | step e _ ih => exact .step e (ih h2)

theorem kindsOK_chain {d e : DT} (h : KindsOK d) (c : Chain d e) : KindsOK e :=
  fun x y cx sx => h x y (chain_trans c cx) sx

theorem kind_chain {d e : DT} (h : KindsOK d) (c : Chain d e) : kindOf e = kindOf d := by
  induction c with
  | refl => rfl
  | @step d e d' s _ ih =>
    rw [ih (kindsOK_chain h (.step s (.refl _))), h d e (.refl _) s]

/-! ### Equivalence -/

theorem absSub_trans (a b c : Abs) : absSub a b = true → absSub b c = true → absSub a c = true := by
  cases a <;> cases b <;> cases c <;> decide

theorem algo_refl (h : CH) : Algo h h := by
  cases h with
  | abs a => cases a <;> simp [Algo, absSub]
  | defd d => exact .refl d

theorem algo_trans {a b c : CH} (ha : ValidH a) (hb : ValidH b) (h1 : Algo a b) (h2 : Algo b c) : Algo a c := by
  cases a <;> cases b <;> cases c <;> simp only [Algo, ValidH] at *
  · exact absSub_trans _ _ _ h1 h2
  · rename_i a b d; cases a <;> cases b <;> simp [absSub, absDef] at h1 h2 ⊢ <;> simp [h2]
  · rename_i a d c; cases a <;> cases c <;> simp [absDef, defAbs, absSub] at h1 h2 ⊢ <;> omega
  · rename_i a d d'; rw [kind_chain hb h2]; exact h1
  · rename_i d a b; cases a <;> cases b <;> simp [defAbs, absSub] at h1 h2 ⊢ <;> simp [h1]
  · rename_i d a d'; cases a <;> simp [defAbs, absDef] at h1 h2 <;> omega
  · rename_i d d' c; rw [← kind_chain ha h1]; exact h2
  · exact chain_trans h1 h2

/-- Spec subtyping implies algorithmic subtyping. -/
theorem algo_of_hsub {a b : CH} (h : HSub a b) (ha : ValidH a) : Algo a b := by
  induction h with
  | refl => exact algo_refl _
  | trans hb _ _ ih1 ih2 => exact algo_trans ha hb (ih1 ha) (ih2 hb)
  | eq_any | i31_eq | struct_eq | array_eq => simp [Algo, absSub]
  | def_struct k | def_array k | def_func k => simp [Algo, defAbs, k]
  | def_super s => exact .step s (.refl _)
  | @none_ t hv _ ih =>
    have := ih hv
    cases t with
    | abs b => cases b <;> simp_all [Algo, absSub]
    | defd d => simp only [Algo, defAbs, absDef] at this ⊢; exact this
  | @nofunc_ t hv _ ih =>
    have := ih hv
    cases t with
    | abs b => cases b <;> simp_all [Algo, absSub]
    | defd d => simp only [Algo, defAbs, absDef] at this ⊢; exact this
  | @noextern_ t hv _ ih =>
    have := ih hv
    cases t with
    | abs b => cases b <;> simp_all [Algo, absSub]
    | defd d => simp [Algo, defAbs] at this
  | @noexn_ t hv _ ih =>
    have := ih hv
    cases t with
    | abs b => cases b <;> simp_all [Algo, absSub]
    | defd d => simp [Algo, defAbs] at this

theorem hsub_abs (a b : Abs) (h : absSub a b = true) : HSub (.abs a) (.abs b) := by
  have tr := fun {x y z : Abs} (h1 : HSub (.abs x) (.abs y)) (h2 : HSub (.abs y) (.abs z)) =>
    HSub.trans (b := .abs y) trivial h1 h2
  have nb : ∀ x, HSub (.abs x) (.abs .any) → HSub (.abs .none) (.abs x) := fun x => HSub.none_ trivial
  cases a <;> cases b <;> simp [absSub] at h
  all_goals first
    | exact .refl _ | exact .eq_any | exact .i31_eq | exact .struct_eq | exact .array_eq
    | exact tr .i31_eq .eq_any | exact tr .struct_eq .eq_any | exact tr .array_eq .eq_any
    | exact nb _ .eq_any | exact nb _ (.refl _) | exact nb _ (tr .i31_eq .eq_any)
    | exact nb _ (tr .struct_eq .eq_any) | exact nb _ (tr .array_eq .eq_any)
    | exact .nofunc_ trivial (.refl _) | exact .noextern_ trivial (.refl _) | exact .noexn_ trivial (.refl _)

theorem hsub_def_abs (d : DT) (b : Abs) (h : defAbs (kindOf d) b = true) : HSub (.defd d) (.abs b) := by
  have tr := fun {y z : Abs} (h1 : HSub (.defd d) (.abs y)) (h2 : HSub (.abs y) (.abs z)) =>
    HSub.trans (b := .abs y) trivial h1 h2
  cases b <;> simp [defAbs] at h
  · rcases h with h | h
    · exact tr (tr (.def_struct h) .struct_eq) .eq_any
    · exact tr (tr (.def_array h) .array_eq) .eq_any
  · rcases h with h | h
    · exact tr (.def_struct h) .struct_eq
    · exact tr (.def_array h) .array_eq
  · exact .def_struct h
  · exact .def_array h
  · exact .def_func h

theorem hsub_chain {d e : DT} (hv : KindsOK d) (c : Chain d e) : HSub (.defd d) (.defd e) := by
  induction c with
  | refl => exact .refl _
  | @step d e d' s c ih =>
    have he : KindsOK e := kindsOK_chain hv (.step s (.refl _))
    exact .trans (b := .defd e) he (.def_super s) (ih he)

/-- Algorithmic subtyping implies spec subtyping. -/
theorem hsub_of_algo {a b : CH} (ha : ValidH a) (hb : ValidH b) (h : Algo a b) : HSub a b := by
  cases a <;> cases b <;> simp only [Algo, ValidH] at *
  · exact hsub_abs _ _ h
  · rename_i a d
    cases a <;> simp [absDef] at h
    · exact .none_ hb (hsub_def_abs d .any (by simp [defAbs]; exact h))
    · exact .nofunc_ hb (hsub_def_abs d .func (by simp [defAbs]; exact h))
  · exact hsub_def_abs _ _ h
  · exact hsub_chain ha h

theorem hsub_iff {a b : CH} (ha : ValidH a) (hb : ValidH b) : HSub a b ↔ Algo a b :=
  ⟨fun h => algo_of_hsub h ha, hsub_of_algo ha hb⟩

/-! ### Closed forms of module types -/

theorem closeGroup_get (m : M) (hw : WF m) (t : Nat) (ht : t < m.n) :
    (closeGroup m (m.start t))[t - m.start t]? = some (closeSub m (m.start t) t) := by
  have := hw.group t ht
  rw [closeGroup_eq, List.getElem?_map, List.getElem?_range (by rw [gsize_start m hw t ht]; omega)]
  simp; congr 1; omega

theorem kindOf_dtOf (m : M) (hw : WF m) (t : Nat) (ht : t < m.n) : kindOf (dtOf m t) = (m.d t).kind := by
  unfold kindOf dtOf; simp only; rw [closeGroup_get m hw t ht]; rfl

theorem superOf_dtOf (m : M) (hw : WF m) (t : Nat) (ht : t < m.n) :
    superOf (dtOf m t) = (m.d t).super.map (dtOf m) := by
  unfold superOf dtOf; simp only; rw [closeGroup_get m hw t ht]
  simp only [closeSub]
  cases hs : (m.d t).super with
  | none => rfl
  | some x =>
    have hx := super_refOK m hw t ht x hs; simp only [RefOK] at hx
    have hg := hw.group t ht; have := gsize_start m hw t ht
    simp only [Option.map_some, cref]
    by_cases hin : m.start t ≤ x ∧ x < m.start t + m.gsize (m.start t)
    · simp only [hin, and_self, ite_true]
      have hxs := (in_group_iff m hw (m.start t) x (by omega) (start_start m hw t ht) (by omega)).mp hin
      simp [hxs]
    · have hxs : m.start x < m.start t := by have := (hw.group x (by omega)).1; omega
      simp [hin, hxs]

/-- Valid modules: supertypes are earlier types of the same kind (checked when the type section is read). -/
structure SubWF (m : M) : Prop extends WF m where
  super_lt : ∀ t < m.n, ∀ s, (m.d t).super = some s → s < t
  super_kind : ∀ t < m.n, ∀ s, (m.d t).super = some s → (m.d s).kind = (m.d t).kind

/-- `t'` is `t` or one of its supertypes, by index. -/
inductive IChain (m : M) : Nat → Nat → Prop where
  | refl (t : Nat) : IChain m t t
  | step {t s t' : Nat} : (m.d t).super = some s → IChain m s t' → IChain m t t'

theorem ichain_lt {m : M} (hw : SubWF m) {t t' : Nat} (c : IChain m t t') (ht : t < m.n) : t' < m.n := by
  induction c with
  | refl => exact ht
  | step s _ ih => exact ih (by have := hw.super_lt _ ht _ s; omega)

theorem chain_dtOf (m : M) (hw : SubWF m) (t : Nat) (ht : t < m.n) (d : DT) :
    Chain (dtOf m t) d ↔ ∃ t', IChain m t t' ∧ dtOf m t' = d := by
  constructor
  · intro c
    generalize he : dtOf m t = e at c
    induction c generalizing t with
    | refl => exact ⟨t, .refl t, he⟩
    | @step e e' d s c ih =>
      rw [← he, superOf_dtOf m hw.toWF t ht] at s
      cases hs : (m.d t).super with
      | none => simp [hs] at s
      | some x =>
        simp [hs] at s
        obtain ⟨t', c', e'⟩ := ih x (by have := hw.super_lt t ht x hs; omega) s
        exact ⟨t', .step hs c', e'⟩
  · rintro ⟨t', c, rfl⟩
    induction c with
    | refl => exact .refl _
    | @step t s t' hs _ ih =>
      have hsn : s < m.n := by have := hw.super_lt t ht s hs; omega
      exact .step (by rw [superOf_dtOf m hw.toWF t ht, hs]; rfl) (ih hsn)

theorem kindsOK_dtOf (m : M) (hw : SubWF m) (t : Nat) (ht : t < m.n) : KindsOK (dtOf m t) := by
  intro e e' c s
  obtain ⟨u, cu, rfl⟩ := (chain_dtOf m hw t ht e).mp c
  have hu := ichain_lt hw cu ht
  rw [superOf_dtOf m hw.toWF u hu] at s
  cases hs : (m.d u).super with
  | none => simp [hs] at s
  | some x =>
    simp [hs] at s; subst s
    have hx : x < m.n := by have := hw.super_lt u hu x hs; omega
    rw [kindOf_dtOf m hw.toWF x hx, kindOf_dtOf m hw.toWF u hu, hw.super_kind u hu x hs]

/-! ### Value types -/

/-- Closed value types: references, the bottom type of validation, and anything else. -/
inductive CV where
  | other (t : Int)
  | ref (null : Bool) (h : CH)
  | bot

/-- Value subtyping of the spec. -/
inductive VSub : CV → CV → Prop where
  | refl (v : CV) : VSub v v
  | bot (v : CV) : VSub .bot v
  | ref {n1 n2 : Bool} {h1 h2 : CH} : HSub h1 h2 → (n1 = true → n2 = true) → VSub (.ref n1 h1) (.ref n2 h2)

def VAlgo : CV → CV → Prop
  | .bot, _ => True
  | .other a, v => v = .other a
  | .ref n1 h1, .ref n2 h2 => (n1 = true → n2 = true) ∧ Algo h1 h2
  | .ref _ _, _ => False

def ValidV : CV → Prop
  | .ref _ h => ValidH h
  | _ => True

theorem vsub_iff {v w : CV} (hv : ValidV v) (hw : ValidV w) : VSub v w ↔ VAlgo v w := by
  constructor
  · intro h; cases h with
    | refl v => cases v with
      | other => rfl
      | ref n h => exact ⟨id, algo_refl h⟩
      | bot => trivial
    | bot => trivial
    | ref h n => exact ⟨n, algo_of_hsub h hv⟩
  · intro h; cases v <;> cases w <;> simp only [VAlgo, ValidV] at h hv hw
    all_goals first
      | (subst h; exact .refl _) | (rw [h]; exact .refl _) | exact .bot _ | (cases h; done)
      | exact .ref (hsub_of_algo hv hw h.2) h.1
      | simp at h

/-! ### `wah_type_is_subtype` -/

/-- Built-in abstract heap types as non-nullable wah_type_t. -/
def absCode : Abs → Int
  | .func => -32 | .extern => -34 | .any => -36 | .eq => -38 | .i31 => -40 | .struct => -42 | .array => -44
  | .exn => -46 | .nofunc => -48 | .noextern => -50 | .none => -52 | .noexn => -54

def allAbs : List Abs := [.func, .extern, .any, .eq, .i31, .struct, .array, .exn, .nofunc, .noextern, .none, .noexn]

def absOfCode (t : Int) : Option Abs := allAbs.find? (absCode · == t)

/-- wah_type_t without type indices, closed. -/
def closeNeg (t : Int) : CV :=
  if t = -99 then .bot
  else match absOfCode (t - t % 2) with
    | some h => .ref (t % 2 == 1) (.abs h)
    | none => .other t

/-- wah_type_t of a module, closed. -/
def closeV (m : M) (t : Int) : CV :=
  if 0 ≤ t then .ref (t % 2 == 1) (.defd (dtOf m (t / 2).toNat)) else closeNeg t

/-- wah_type_t values that validation lets through: numbers, vectors, packed types, references and bottom. -/
def builtinCodes : List Int :=
  [-2, -4, -6, -8, -10, -16, -18, -99, -31, -32, -33, -34, -35, -36, -37, -38, -39, -40, -41, -42, -43, -44, -45, -46,
   -47, -48, -49, -50, -51, -52, -53, -54]

def ValidCode (m : M) (t : Int) : Prop := t ∈ builtinCodes ∨ (0 ≤ t ∧ (t / 2).toNat < m.n)

def isRefCode (t : Int) : Bool := (-54 ≤ t && t ≤ -31) || 0 ≤ t

/-- The last switch of `wah_type_is_subtype`, between different non-nullable built-in types. -/
def absTable (sub sup : Int) : Bool :=
  if !isRefCode sub || !isRefCode sup then false
  else if sup = -36 then !(sub = -34 || sub = -46 || sub = -50 || sub = -54 || sub = -32 || sub = -48)
  else if sup = -38 then sub = -40 || sub = -42 || sub = -44 || sub = -52
  else if sup = -42 || sup = -44 || sup = -40 then sub = -52
  else if sup = -32 then sub = -48
  else if sup = -34 then sub = -50
  else if sup = -46 then sub = -54
  else false

/-- The supertype walk of `wah_type_is_subtype` from `t`, for `sup` = `pi`. -/
def walk (m : M) (canon : Nat → Nat) (pi : Nat) : Nat → Nat → Bool
  | 0, _ => false
  | f + 1, t => match (m.d t).super with
    | none => false
    | some st => st == pi || canon st == canon pi || walk m canon pi f st

def wahKind (m : M) (t : Int) : Nat := if 0 ≤ t ∧ (t / 2).toNat < m.n then (m.d (t / 2).toNat).kind else 0

/-- `wah_type_is_subtype` with `canonical_map` = `canon`. -/
def wahSub (m : M) (canon : Nat → Nat) (sub sup : Int) : Bool :=
  if sub = sup ∨ sub = -99 then true
  else if sub % 2 = 1 ∧ sup % 2 ≠ 1 then false
  else
    let sub := sub - sub % 2
    let sup := sup - sup % 2
    if sub = sup then true
    else if 0 ≤ sub ∧ 0 ≤ sup then
      let si := (sub / 2).toNat
      let pi := (sup / 2).toNat
      if si < m.n ∧ pi < m.n then canon si == canon pi || walk m canon pi m.n si else false
    else if 0 ≤ sub then
      let k := wahKind m sub
      if sup = -36 ∨ sup = -38 then k == kStruct || k == kArray
      else if sup = -42 then k == kStruct
      else if sup = -44 then k == kArray
      else if sup = -32 then k == kFunc
      else false
    else if 0 ≤ sup then
      let k := wahKind m sup
      if sub = -52 then k == kStruct || k == kArray
      else if sub = -48 then k == kFunc
      else false
    else absTable sub sup

/-! ### `wah_type_is_subtype` agrees with the spec -/

/-- `VAlgo` between closed built-in types. -/
def vAbsB : CV → CV → Bool
  | .bot, _ => true
  | .other a, .other b => a == b
  | .ref n1 (.abs h1), .ref n2 (.abs h2) => (!n1 || n2) && absSub h1 h2
  | _, _ => false

/-- `wah_type_is_subtype` between built-in types. -/
def wahSubNeg (sub sup : Int) : Bool :=
  if sub = sup ∨ sub = -99 then true
  else if sub % 2 = 1 ∧ sup % 2 ≠ 1 then false
  else if sub - sub % 2 = sup - sup % 2 then true
  else absTable (sub - sub % 2) (sup - sup % 2)

theorem builtin_neg : ∀ t ∈ builtinCodes, t < 0 := by decide

/-- The whole table of built-in types, checked by evaluation. -/
theorem neg_table : builtinCodes.all (fun a => builtinCodes.all fun b => wahSubNeg a b == vAbsB (closeNeg a) (closeNeg b)) = true := by
  decide

theorem wahSub_neg (m : M) (canon : Nat → Nat) (sub sup : Int) (h1 : sub < 0) (h2 : sup < 0) :
    wahSub m canon sub sup = wahSubNeg sub sup := by
  unfold wahSub wahSubNeg
  have a1 : ¬(0 ≤ sub - sub % 2) := by omega
  have a2 : ¬(0 ≤ sup - sup % 2) := by omega
  simp only [a1, a2, false_and, ite_false]

theorem closeNeg_algo (a b : Int) :
    VAlgo (closeNeg a) (closeNeg b) ↔ vAbsB (closeNeg a) (closeNeg b) = true := by
  unfold closeNeg
  split
  · simp [VAlgo, vAbsB]
  · split <;> split <;> (try split) <;> simp_all [VAlgo, vAbsB, Algo]
    all_goals first | exact fun _ => Decidable.imp_iff_not_or | exact eq_comm

/-- From a type index to a built-in type, after the nullability check. -/
def kindTable (k : Nat) (sup : Int) : Bool :=
  if sup = -36 ∨ sup = -38 then k == kStruct || k == kArray
  else if sup = -42 then k == kStruct
  else if sup = -44 then k == kArray
  else if sup = -32 then k == kFunc
  else false

/-- From a built-in type to a type index, after the nullability check. -/
def kindTable' (k : Nat) (sub : Int) : Bool :=
  if sub = -52 then k == kStruct || k == kArray
  else if sub = -48 then k == kFunc
  else false

theorem wahSub_idx_neg (m : M) (canon : Nat → Nat) (sub sup : Int) (h1 : 0 ≤ sub) (h2 : sup < 0) :
    wahSub m canon sub sup =
      (if sub % 2 = 1 ∧ sup % 2 ≠ 1 then false else kindTable (wahKind m (sub - sub % 2)) (sup - sup % 2)) := by
  unfold wahSub kindTable
  have a0 : ¬(sub = sup ∨ sub = -99) := by omega
  have a1 : ¬(sub - sub % 2 = sup - sup % 2) := by omega
  have a2 : 0 ≤ sub - sub % 2 := by omega
  have a3 : ¬(0 ≤ sup - sup % 2) := by omega
  simp only [a0, a1, a2, a3, and_false, ite_false, ite_true]

theorem wahSub_neg_idx (m : M) (canon : Nat → Nat) (sub sup : Int) (h1 : sub < 0) (h2 : 0 ≤ sup) :
    wahSub m canon sub sup =
      (if sub = -99 then true else if sub % 2 = 1 ∧ sup % 2 ≠ 1 then false
       else kindTable' (wahKind m (sup - sup % 2)) (sub - sub % 2)) := by
  unfold wahSub kindTable'
  have a0 : sub ≠ sup := by omega
  have a1 : ¬(sub - sub % 2 = sup - sup % 2) := by omega
  have a2 : ¬(0 ≤ sub - sub % 2) := by omega
  have a3 : 0 ≤ sup - sup % 2 := by omega
  by_cases hb : sub = -99
  · simp [hb]
  · simp only [a0, hb, or_self, a1, a2, a3, false_and, ite_false, ite_true]

theorem wahSub_idx_idx (m : M) (canon : Nat → Nat) (sub sup : Int) (h1 : 0 ≤ sub) (h2 : 0 ≤ sup) :
    wahSub m canon sub sup =
      (if sub = sup then true else if sub % 2 = 1 ∧ sup % 2 ≠ 1 then false
       else if sub / 2 = sup / 2 then true
       else if (sub / 2).toNat < m.n ∧ (sup / 2).toNat < m.n then
         canon (sub / 2).toNat == canon (sup / 2).toNat || walk m canon (sup / 2).toNat m.n (sub / 2).toNat
       else false) := by
  unfold wahSub
  have e1 : (sub - sub % 2) / 2 = sub / 2 := by omega
  have e2 : (sup - sup % 2) / 2 = sup / 2 := by omega
  have e3 : (sub - sub % 2 = sup - sup % 2) = (sub / 2 = sup / 2) := by apply propext; omega
  have a2 : 0 ≤ sub - sub % 2 ∧ 0 ≤ sup - sup % 2 := by omega
  have hb : sub ≠ -99 := by omega
  simp only [hb, or_false, e1, e2, e3, a2, and_self, ite_true]

theorem wahKind_eq (m : M) (hw : WF m) (t : Int) (h0 : 0 ≤ t) (hn : (t / 2).toNat < m.n) :
    wahKind m (t - t % 2) = kindOf (dtOf m (t / 2).toNat) := by
  have e : (t - t % 2) / 2 = t / 2 := by omega
  unfold wahKind; rw [e]; split
  · rw [kindOf_dtOf m hw _ hn]
  · omega

theorem ichain_cases {m : M} {s t' : Nat} (c : IChain m s t') :
    t' = s ∨ ∃ s', (m.d s).super = some s' ∧ IChain m s' t' := by
  cases c with
  | refl => left; rfl
  | step h c => right; exact ⟨_, h, c⟩

theorem walk_iff (m : M) (canon : Nat → Nat) (hw : SubWF m) (pi : Nat) :
    ∀ f t, t < f → t < m.n → (walk m canon pi f t = true ↔
      ∃ s, (m.d t).super = some s ∧ ∃ t', IChain m s t' ∧ canon t' = canon pi)
  | 0, _, h, _ => absurd h (Nat.not_lt_zero _)
  | f + 1, t, hf, ht => by
    unfold walk
    cases hs : (m.d t).super with
    | none => simp
    | some st =>
      have hst := hw.super_lt t ht st hs
      have ih := walk_iff m canon hw pi f st (by omega) (by omega)
      simp only [Bool.or_eq_true, beq_iff_eq, ih, Option.some.injEq, exists_eq_left']
      constructor
      · rintro ((rfl | e) | ⟨s', hs', t', c, e⟩)
        · exact ⟨st, .refl _, rfl⟩
        · exact ⟨st, .refl _, e⟩
        · exact ⟨t', .step hs' c, e⟩
      · rintro ⟨t', c, e⟩
        rcases ichain_cases c with rfl | ⟨s', hs', c'⟩
        · exact .inl (.inr e)
        · exact .inr ⟨s', hs', t', c', e⟩

theorem idx_idx (m : M) (canon : Nat → Nat) (hw : SubWF m)
    (hc : ∀ x y, x < m.n → y < m.n → (canon x = canon y ↔ dtOf m x = dtOf m y)) (si pi : Nat)
    (hsi : si < m.n) (hpi : pi < m.n) :
    (canon si == canon pi || walk m canon pi m.n si) = true ↔ Chain (dtOf m si) (dtOf m pi) := by
  rw [chain_dtOf m hw si hsi, Bool.or_eq_true, beq_iff_eq, walk_iff m canon hw pi m.n si hsi hsi]
  constructor
  · rintro (e | ⟨s, hs, t', c, e⟩)
    · exact ⟨si, .refl _, (hc si pi hsi hpi).mp e⟩
    · have := ichain_lt hw (.step hs c) hsi
      exact ⟨t', .step hs c, (hc t' pi this hpi).mp e⟩
  · rintro ⟨t', c, e⟩
    have ht' := ichain_lt hw c hsi
    rcases ichain_cases c with rfl | ⟨s', hs', c'⟩
    · exact .inl ((hc _ pi hsi hpi).mpr e)
    · exact .inr ⟨s', hs', t', c', (hc t' pi ht' hpi).mpr e⟩

theorem validV_closeV (m : M) (hw : SubWF m) (t : Int) (h : ValidCode m t) : ValidV (closeV m t) := by
  unfold closeV
  split
  · rcases h with h | ⟨_, hn⟩
    · have := builtin_neg t h; omega
    · exact kindsOK_dtOf m hw _ hn
  · unfold closeNeg; split
    · trivial
    · split <;> trivial

/-- `wah_type_is_subtype` is the subtyping of the spec, given a canonical map like `canonical_map`. -/
theorem wahSub_iff (m : M) (canon : Nat → Nat) (hw : SubWF m)
    (hc : ∀ x y, x < m.n → y < m.n → (canon x = canon y ↔ dtOf m x = dtOf m y))
    (sub sup : Int) (hs : ValidCode m sub) (hp : ValidCode m sup) :
    wahSub m canon sub sup = true ↔ VSub (closeV m sub) (closeV m sup) := by
  rw [vsub_iff (validV_closeV m hw sub hs) (validV_closeV m hw sup hp)]
  by_cases h1 : 0 ≤ sub <;> by_cases h2 : 0 ≤ sup
  · -- Both type indices
    have hsn : (sub / 2).toNat < m.n := by rcases hs with h | h; exact absurd (builtin_neg _ h) (by omega); exact h.2
    have hpn : (sup / 2).toNat < m.n := by rcases hp with h | h; exact absurd (builtin_neg _ h) (by omega); exact h.2
    have e1 : (sub - sub % 2) / 2 = sub / 2 := by omega
    have e2 : (sup - sup % 2) / 2 = sup / 2 := by omega
    have hii := idx_idx m canon hw hc _ _ hsn hpn
    rw [wahSub_idx_idx m canon sub sup h1 h2]
    unfold closeV; simp only [h1, h2, ite_true, VAlgo, Algo]
    have hn1 : (sub % 2 == 1) = true ↔ sub % 2 = 1 := by simp
    have hn2 : (sup % 2 == 1) = true ↔ sup % 2 = 1 := by simp
    rw [hn1, hn2]
    split
    · rename_i he; subst he; simp only [true_iff]; exact ⟨id, .refl _⟩
    · split
      · rename_i hn; simp only [Bool.false_eq_true, false_iff, not_and]; intro h; exact absurd (h hn.1) hn.2
      · rename_i hn
        have hn' : sub % 2 = 1 → sup % 2 = 1 := fun h => by omega
        split
        · rename_i he; have : (sub / 2).toNat = (sup / 2).toNat := by omega
          rw [this]; simp only [true_iff]; exact ⟨hn', .refl _⟩
        · simp only [hsn, hpn, and_self, ite_true]; rw [hii]; exact ⟨fun h => ⟨hn', h⟩, fun h => h.2⟩
  · -- Type index to built-in type
    have hsn : (sub / 2).toNat < m.n := by rcases hs with h | h; exact absurd (builtin_neg _ h) (by omega); exact h.2
    have hpc : sup ∈ builtinCodes := by rcases hp with h | h; exact h; omega
    rw [wahSub_idx_neg m canon sub sup h1 (by omega), wahKind_eq m hw.toWF sub h1 hsn]
    unfold closeV; simp only [h1, h2, ite_true, ite_false]
    have hb : sub % 2 = 0 ∨ sub % 2 = 1 := by omega
    simp [builtinCodes] at hpc
    rcases hpc with rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl |
      rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl
    all_goals rcases hb with hb | hb <;> simp [hb, closeNeg, absOfCode, allAbs, absCode, kindTable, VAlgo, Algo, defAbs]
  · -- Built-in type to type index
    have hpn : (sup / 2).toNat < m.n := by rcases hp with h | h; exact absurd (builtin_neg _ h) (by omega); exact h.2
    have hsc : sub ∈ builtinCodes := by rcases hs with h | h; exact h; omega
    rw [wahSub_neg_idx m canon sub sup (by omega) h2, wahKind_eq m hw.toWF sup h2 hpn]
    unfold closeV; simp only [h1, h2, ite_true, ite_false]
    have hb : sup % 2 = 0 ∨ sup % 2 = 1 := by omega
    simp [builtinCodes] at hsc
    rcases hsc with rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl |
      rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl | rfl
    all_goals rcases hb with hb | hb <;> simp [hb, closeNeg, absOfCode, allAbs, absCode, kindTable', VAlgo, Algo, absDef]
  · -- Built-in types
    have hsc : sub ∈ builtinCodes := by rcases hs with h | h; exact h; omega
    have hpc : sup ∈ builtinCodes := by rcases hp with h | h; exact h; omega
    rw [wahSub_neg m canon sub sup (by omega) (by omega)]
    unfold closeV; simp only [h1, h2, ite_false]
    rw [closeNeg_algo sub sup]
    have := neg_table
    simp only [List.all_eq_true, beq_iff_eq] at this
    rw [this sub hsc sup hpc]

/-- `wah_type_accepts_repr`: the supertype walk from the type `t` of an object, for the cast target `target`. -/
def acceptsWalk (m : M) (canon : Nat → Nat) (target : Nat) : Nat → Nat → Bool
  | 0, _ => false
  | f + 1, t => canon t == canon target || match (m.d t).super with
    | none => false
    | some s => acceptsWalk m canon target f s

theorem acceptsWalk_iff (m : M) (canon : Nat → Nat) (hw : SubWF m) (target : Nat) :
    ∀ f t, t < f → t < m.n → (acceptsWalk m canon target f t = true ↔ ∃ t', IChain m t t' ∧ canon t' = canon target)
  | 0, _, h, _ => absurd h (Nat.not_lt_zero _)
  | f + 1, t, hf, ht => by
    unfold acceptsWalk
    rw [Bool.or_eq_true, beq_iff_eq]
    cases hs : (m.d t).super with
    | none =>
      simp only [Bool.false_eq_true, or_false]
      constructor
      · intro e; exact ⟨t, .refl _, e⟩
      · rintro ⟨t', c, e⟩
        rcases ichain_cases c with rfl | ⟨s', hs', _⟩
        · exact e
        · rw [hs] at hs'; cases hs'
    | some st =>
      have hst := hw.super_lt t ht st hs
      simp only [acceptsWalk_iff m canon hw target f st (by omega) (by omega)]
      constructor
      · rintro (e | ⟨t', c, e⟩)
        · exact ⟨t, .refl _, e⟩
        · exact ⟨t', .step hs c, e⟩
      · rintro ⟨t', c, e⟩
        rcases ichain_cases c with rfl | ⟨s', hs', c'⟩
        · exact .inl e
        · rw [hs] at hs'; cases hs'; exact .inr ⟨t', c', e⟩

/-- Casts of objects of type `t` to `target` succeed exactly when `t` is a subtype of `target` in the spec. -/
theorem acceptsWalk_spec (m : M) (canon : Nat → Nat) (hw : SubWF m)
    (hc : ∀ x y, x < m.n → y < m.n → (canon x = canon y ↔ dtOf m x = dtOf m y)) (t target : Nat)
    (ht : t < m.n) (hg : target < m.n) :
    acceptsWalk m canon target m.n t = true ↔ HSub (.defd (dtOf m t)) (.defd (dtOf m target)) := by
  rw [hsub_iff (a := .defd _) (b := .defd _) (kindsOK_dtOf m hw t ht) (kindsOK_dtOf m hw target hg), Algo,
    chain_dtOf m hw t ht,
    acceptsWalk_iff m canon hw target m.n t ht ht]
  constructor
  · rintro ⟨t', c, e⟩; exact ⟨t', c, (hc t' target (ichain_lt hw c ht) hg).mp e⟩
  · rintro ⟨t', c, e⟩; exact ⟨t', c, (hc t' target (ichain_lt hw c ht) hg).mpr e⟩

/-- `wah_type_is_subtype` with the `canonical_map` that WAH builds is the subtyping of the spec. -/
theorem wah_type_is_subtype_spec (m : M) (mix : Nat → Nat → Nat) (funcKind : Nat) (hw : SubWF m)
    (sub sup : Int) (hs : ValidCode m sub) (hp : ValidCode m sup) :
    wahSub m (canonicalMap m (ghash m mix funcKind)) sub sup = true ↔ VSub (closeV m sub) (closeV m sup) :=
  wahSub_iff m _ hw (fun x y hx hy => canonicalMap_iff m _ hw.toWF (ghash_ok m mix funcKind hw.toWF) x y hx hy)
    sub sup hs hp

end Wah.Subtype
