/-
Correspondence of the heap subtyping of WahProofs/Subtype.lean (`HSub` over closed forms, which `wah_type_is_subtype`
follows) with `Heaptype_sub` of the Lean model that SpecTec generates from the Wasm 3.0 spec (SpecTec/Wasm.lean,
fetched by `make lean-spectec`).

Closed forms translate to SpecTec syntax directly: `inner i` is `REC i`, `dt g i` is `_DEF (REC g) i` and `sub` is
`SUB`, with the comptype chosen by the kind. Atoms (numbers, vectors, packed and abstract reference types) are
translated by any map `A` to well-formed storage types.

The generated model leaves `$minus_recs` opaque, as the generator cannot translate it, so the theorems assume that it
is the function of the spec (`MinusRecs`), which is transcribed here as `minusRecs`.
-/
import SpecTec.Wasm
import WahProofs.Subtype

set_option linter.unusedSectionVars false

namespace Wah.SpecTec
open Wah.Types Wah.Subtype

/-- `$minus_recs` of 1.2-syntax.types.spectec, which drops the substitutions of `REC` variables. -/
def minusRecs : List typevar → List typeuse → Option (List typevar × List typeuse)
  | [], [] => some ([], [])
  | .REC _ :: tvs, _ :: tus => minusRecs tvs tus
  | ._IDX x :: tvs, tu :: tus => (minusRecs tvs tus).map fun (a, b) => (._IDX x :: a, tu :: b)
  | _, _ => none

/-- The generated `minus_recs` is the function of the spec. -/
def MinusRecs : Prop := ∀ tvs tus, minus_recs tvs tus = minusRecs tvs tus

/-! ### Translation -/

def trAbs : Abs → heaptype
  | .any => .ANY | .eq => .EQ | .i31 => .I31 | .struct => .STRUCT | .array => .ARRAY | .none => .NONE
  | .func => .FUNC | .nofunc => .NOFUNC | .extern => .EXTERN | .noextern => .NOEXTERN | .exn => .EXN
  | .noexn => .NOEXN

def nullOpt (b : Bool) : Option null := if b then some .NULL else none

def mutOpt (b : Bool) : Option «mut» := if b then some .MUT else none

def finalOpt (b : Bool) : Option final := if b then some .FINAL else none

/-- Value types of function types, which have no packed types. -/
def toVal : storagetype → valtype
  | .I32 => .I32 | .I64 => .I64 | .F32 => .F32 | .F64 => .F64 | .V128 => .V128
  | .REF n h => .REF n h | _ => .BOT

variable (A : Nat → storagetype)

/-- The composite type of a kind, with the fields of `wah_type_def_t`. -/
def trComp (kind : Nat) (counts : Nat × Nat) (fs : List fieldtype) : comptype :=
  if kind = kStruct then .STRUCT (.mk_list fs)
  else if kind = kArray then .ARRAY (fs.headD (.mk_fieldtype none .BOT))
  else .FUNC (.mk_list ((fs.take counts.1).map fun f => match f with | .mk_fieldtype _ t => toVal t))
    (.mk_list ((fs.drop counts.1).map fun f => match f with | .mk_fieldtype _ t => toVal t))

/-- `REC i` of the outermost rec group becomes `_DEF r i` if it is unrolled by `r`. -/
def recRef (u : Option rectype) (i : Nat) : heaptype :=
  match u with
  | some r => ._DEF r i
  | none => .REC i

def recUse (u : Option rectype) (i : Nat) : typeuse :=
  match u with
  | some r => ._DEF r i
  | none => .REC i

mutual
/-- Storage types. -/
def trV (u : Option rectype) : CT → storagetype
  | .atom a => A a
  | .inner i null => .REF (nullOpt null) (recRef u i)
  | .dt g i null => .REF (nullOpt null) (._DEF (.REC (.mk_list (trSs none g))) i)
  | .sub .. => .BOT

/-- Supertypes. -/
def trU (u : Option rectype) : CT → typeuse
  | .inner i _ => recUse u i
  | .dt g i _ => ._DEF (.REC (.mk_list (trSs none g))) i
  | _ => .REC 0

def trFs (u : Option rectype) : List (CT × Bool) → List fieldtype
  | [] => []
  | (c, m) :: fs => .mk_fieldtype (mutOpt m) (trV u c) :: trFs u fs

def trSup (u : Option rectype) : Option CT → List typeuse
  | some s => [trU u s]
  | none => []

def trS (u : Option rectype) : CT → subtype
  | .sub fin kind super counts fields => .SUB (finalOpt fin) (trSup u super) (trComp kind counts (trFs u fields))
  | _ => .SUB none [] (.STRUCT (.mk_list []))

def trSs (u : Option rectype) : List CT → List subtype
  | [] => []
  | c :: cs => trS u c :: trSs u cs
end

/-- Closed rec groups. -/
def trG (g : List CT) : rectype := .REC (.mk_list (trSs A none g))

theorem trFs_eq (u : Option rectype) (fs : List (CT × Bool)) :
    trFs A u fs = fs.map fun f => .mk_fieldtype (mutOpt f.2) (trV A u f.1) := by
  induction fs with
  | nil => simp [trFs]
  | cons f fs ih => obtain ⟨c, m⟩ := f; simp [trFs, ih]

theorem trSs_eq (u : Option rectype) (g : List CT) : trSs A u g = g.map (trS A u) := by
  induction g with
  | nil => simp [trSs]
  | cons c g ih => simp [trSs, ih]

/-! ### Closed forms that translate to well-formed types

References to the outermost group are below its size `n`, nested groups are `OkG`, subtypes are `sub` with a
reference as the supertype, fields are not `sub`, and lists are shorter than 2^32. -/

mutual
/-- Field types. -/
def OkV (n : Nat) : CT → Prop
  | .atom _ => True
  | .inner i _ => i < n
  | .dt g i _ => g.length < 2 ^ 32 ∧ OkL g.length g ∧ i < g.length
  | .sub .. => False

def OkSuper (n : Nat) : Option CT → Prop
  | none => True
  | some (.inner i _) => i < n
  | some (.dt g i _) => g.length < 2 ^ 32 ∧ OkL g.length g ∧ i < g.length
  | some _ => False

def OkFs (n : Nat) : List (CT × Bool) → Prop
  | [] => True
  | (c, _) :: fs => OkV n c ∧ OkFs n fs

/-- Subtypes. -/
def OkS (n : Nat) : CT → Prop
  | .sub _ _ super _ fields => OkSuper n super ∧ fields.length < 2 ^ 32 ∧ OkFs n fields
  | _ => False

def OkL (n : Nat) : List CT → Prop
  | [] => True
  | c :: cs => OkS n c ∧ OkL n cs
end

def OkG (g : List CT) : Prop := g.length < 2 ^ 32 ∧ OkL g.length g

/-! ### Well-formedness -/

/-- Atoms are well-formed storage types that substitutions leave alone (numbers, vectors, packed types and
abstract references). -/
structure AtomsOK : Prop where
  wf : ∀ a, wf_storagetype (A a)
  subst : ∀ a tvs tus, fun_subst_storagetype (A a) tvs tus (A a)

theorem wf_toVal {t : storagetype} (h : wf_storagetype t) : wf_valtype (toVal t) := by
  cases h <;> first | (constructor; assumption) | constructor

theorem wf_trAbs (h : Abs) : wf_heaptype (trAbs h) := by cases h <;> constructor

theorem wf_comp (kind : Nat) (counts : Nat × Nat) (fs : List fieldtype) (hl : fs.length < 2 ^ 32)
    (hf : ∀ f ∈ fs, wf_fieldtype f) : wf_comptype (trComp kind counts fs) := by
  have hv : ∀ l : List fieldtype, (∀ f ∈ l, wf_fieldtype f) →
      ∀ v ∈ l.map (fun f => match f with | .mk_fieldtype _ t => toVal t), wf_valtype v := by
    intro l hl v hv
    obtain ⟨f, hf, rfl⟩ := List.mem_map.mp hv
    cases hf' : f with
    | mk_fieldtype m t => rw [hf'] at hf; cases hl _ hf; exact wf_toVal (by assumption)
  unfold trComp
  split
  · exact .comptype_case_0 _ (by simpa [proj_list_0] using hl) (by simpa [proj_list_0, Forall] using hf)
  · split
    · constructor
      cases fs with
      | nil => exact .fieldtype_case_0 _ _ .storagetype_case_6
      | cons f _ => exact hf f (by simp)
    · refine .comptype_case_2 _ _ ?_ ?_ ?_ ?_
      · simp [proj_list_0]; omega
      · simpa [proj_list_0, Forall] using hv _ (fun f h => hf f (List.mem_of_mem_take h))
      · simp [proj_list_0]; omega
      · simpa [proj_list_0, Forall] using hv _ (fun f h => hf f (List.mem_of_mem_drop h))

theorem trSs_length (u : Option rectype) (g : List CT) : (trSs A u g).length = g.length := by
  simp [trSs_eq]

variable {A} (hA : AtomsOK A)
include hA

theorem wf_rec_of (g : List CT) (hl : g.length < 2 ^ 32) (h : ∀ s ∈ trSs A none g, wf_subtype s) :
    wf_rectype (trG A g) := by
  rw [trG]
  exact .rectype_case_0 _ (by simpa [proj_list_0, trSs_length] using hl) (by simpa [proj_list_0, Forall] using h)

mutual
theorem wf_trV (u : Option rectype) (hu : ∀ r, u = some r → wf_rectype r) (n : Nat) :
    ∀ c, OkV n c → wf_storagetype (trV A u c)
  | .atom a, _ => by simp only [trV]; exact hA.wf a
  | .inner i null, _ => by
    simp only [trV]
    refine .storagetype_case_5 _ _ ?_
    cases u with
    | none => exact .heaptype_case_15 _
    | some r => exact .heaptype_case_14 _ _ (hu r rfl)
  | .dt g i null, h => by
    simp only [trV, OkV] at h ⊢
    exact .storagetype_case_5 _ _ (.heaptype_case_14 _ _
      (wf_rec_of hA g h.1 (wf_trSs none (fun _ h => nomatch h) g.length g h.2.1)))
  | .sub .., h => by simp [OkV] at h

theorem wf_trU (u : Option rectype) (hu : ∀ r, u = some r → wf_rectype r) (n : Nat) :
    ∀ s, OkSuper n (some s) → wf_typeuse (trU A u s)
  | .inner i _, _ => by
    simp only [trU]
    cases u with
    | none => exact .typeuse_case_2 _
    | some r => exact .typeuse_case_1 _ _ (hu r rfl)
  | .dt g i _, h => by
    simp only [trU, OkSuper] at h ⊢
    exact .typeuse_case_1 _ _ (wf_rec_of hA g h.1 (wf_trSs none (fun _ h => nomatch h) g.length g h.2.1))
  | .atom _, h => by simp [OkSuper] at h
  | .sub .., h => by simp [OkSuper] at h

theorem wf_trFs (u : Option rectype) (hu : ∀ r, u = some r → wf_rectype r) (n : Nat) :
    ∀ fs, OkFs n fs → ∀ f ∈ trFs A u fs, wf_fieldtype f
  | [], _ => by simp [trFs]
  | (c, m) :: fs, h => by
    simp only [OkFs] at h
    simp only [trFs, List.mem_cons]
    rintro f (rfl | hf)
    · exact .fieldtype_case_0 _ _ (wf_trV u hu n c h.1)
    · exact wf_trFs u hu n fs h.2 f hf

theorem wf_trS (u : Option rectype) (hu : ∀ r, u = some r → wf_rectype r) (n : Nat) :
    ∀ c, OkS n c → wf_subtype (trS A u c)
  | .sub fin kind super counts fields, h => by
    simp only [OkS] at h
    rw [trS.eq_1]
    refine .subtype_case_0 _ _ _ ?_ (wf_comp kind counts _ (by simpa [trFs_eq] using h.2.1)
      (wf_trFs u hu n fields h.2.2))
    cases super with
    | none => simp [trSup, Forall]
    | some s => simpa [trSup, Forall] using wf_trU u hu n s h.1
  | .atom _, h | .inner .., h | .dt .., h => by simp [OkS] at h

theorem wf_trSs (u : Option rectype) (hu : ∀ r, u = some r → wf_rectype r) (n : Nat) :
    ∀ cs, OkL n cs → ∀ s ∈ trSs A u cs, wf_subtype s
  | [], _ => by simp [trSs]
  | c :: cs, h => by
    simp only [OkL] at h
    simp only [trSs, List.mem_cons]
    rintro s (rfl | hs)
    · exact wf_trS u hu n c h.1
    · exact wf_trSs u hu n cs h.2 s hs
end

theorem wf_trG (g : List CT) (h : OkG g) : wf_rectype (trG A g) :=
  wf_rec_of hA g h.1 (wf_trSs hA none (fun _ h => nomatch h) g.length g h.2)

/-! ### Substitution -/

omit hA in
theorem toVal_sv (v : valtype) : toVal (storagetype_valtype v) = v := by cases v <;> rfl

omit hA in
theorem subst_toVal {zt zt' : storagetype} {tvs tus} (h : fun_subst_storagetype zt tvs tus zt') :
    fun_subst_valtype (toVal zt) tvs tus (toVal zt') := by
  cases h <;> (try rw [toVal_sv]) <;> first | assumption | exact .fun_subst_valtype_case_6 _ _

/-- The value type of a field of a function type. -/
def fval (f : fieldtype) : valtype := match f with | .mk_fieldtype _ t => toVal t

omit hA in
theorem subst_fval {f f' : fieldtype} {tvs tus} (h : fun_subst_fieldtype f tvs tus f') :
    fun_subst_valtype (fval f) tvs tus (fval f') := by
  cases h; exact subst_toVal (by assumption)

omit hA in
theorem subst_bot_field (tvs tus) :
    fun_subst_fieldtype (.mk_fieldtype none .BOT) tvs tus (.mk_fieldtype none .BOT) :=
  .fun_subst_fieldtype_case_0 _ _ _ _ _ (.fun_subst_storagetype_case_0 _ _ _ (.fun_subst_valtype_case_6 _ _))

omit hA in
theorem subst_comp {α : Type} (kind : Nat) (counts : Nat × Nat) (l : List α) (p q : α → fieldtype) {tvs tus}
    (h : ∀ x ∈ l, fun_subst_fieldtype (p x) tvs tus (q x)) :
    fun_subst_comptype (trComp kind counts (l.map p)) tvs tus (trComp kind counts (l.map q)) := by
  have hv : ∀ l' : List α, (∀ x ∈ l', x ∈ l) → Forall₂ (fun v t => fun_subst_valtype t tvs tus v)
      (l'.map (fval ∘ q)) (l'.map (fval ∘ p)) := by
    intro l' hl x y hxy
    rw [List.zip_map'] at hxy
    obtain ⟨a, ha, he⟩ := List.mem_map.mp hxy
    cases he
    exact subst_fval (h a (hl a ha))
  unfold trComp
  split
  · refine .fun_subst_comptype_case_0 _ _ _ _ ?_ (by simp)
    intro x y hxy
    rw [List.zip_map'] at hxy
    obtain ⟨a, ha, he⟩ := List.mem_map.mp hxy
    cases he
    exact h a ha
  · split
    · refine .fun_subst_comptype_case_1 _ _ _ _ ?_
      cases l with
      | nil => exact subst_bot_field tvs tus
      | cons x _ => exact h x (by simp)
    · have e1 : ∀ r : α → fieldtype, ((l.map r).take counts.1).map (fun f => match f with
          | .mk_fieldtype _ t => toVal t) = (l.take counts.1).map (fval ∘ r) := by
        intro r; rw [← List.map_take, List.map_map]; rfl
      have e2 : ∀ r : α → fieldtype, ((l.map r).drop counts.1).map (fun f => match f with
          | .mk_fieldtype _ t => toVal t) = (l.drop counts.1).map (fval ∘ r) := by
        intro r; rw [← List.map_drop, List.map_map]; rfl
      rw [e1, e1, e2, e2]
      exact .fun_subst_comptype_case_2 _ _ _ _ _ _ (hv _ fun x hx => List.mem_of_mem_drop hx)
        (hv _ fun x hx => List.mem_of_mem_take hx) (by simp) (by simp)

/-- What a substitution does to `REC` variables below `n`: it unrolls them by `u`, and it leaves alone the variables
of nested rec groups. -/
structure SubstOK (tvs : List typevar) (tus : List typeuse) (u : Option rectype) (n : Nat) : Prop where
  recs : ∀ i < n, fun_subst_typevar (.REC i) tvs tus (some (recUse u i))
  minus : minus_recs tvs tus = some ([], [])

omit hA in
theorem heaptype_recUse (u : Option rectype) (i : Nat) : heaptype_typeuse (recUse u i) = recRef u i := by
  cases u <;> rfl

omit hA in
theorem subst_ref {n : Option null} {ht ht' : heaptype} {tvs tus} (h : fun_subst_heaptype ht tvs tus ht') :
    fun_subst_storagetype (.REF n ht) tvs tus (.REF n ht') :=
  .fun_subst_storagetype_case_1 _ _ _ _ _ (.fun_subst_valtype_case_5 _ _ _ _ _ (.fun_subst_reftype_case_0 _ _ _ _ _ h))

omit hA in
theorem substOK_nil (hMR : MinusRecs) (n : Nat) : SubstOK [] [] none n where
  recs i _ := .fun_subst_typevar_case_0 (.REC i)
  minus := by rw [hMR]; rfl

omit hA in
theorem forall2_map {α β : Type} (l : List α) (p q : α → β) (R : β → β → Prop) (h : ∀ x ∈ l, R (q x) (p x)) :
    Forall₂ R (l.map q) (l.map p) := by
  intro x y hxy
  rw [List.zip_map'] at hxy
  obtain ⟨a, ha, he⟩ := List.mem_map.mp hxy
  cases he
  exact h a ha

/-- Nested rec groups are left alone. -/
theorem subst_nested (g : List CT) (hg : OkG g) {tvs tus} (hm : minus_recs tvs tus = some ([], []))
    (h : ∀ c ∈ g, fun_subst_subtype (trS A none c) [] [] (trS A none c)) :
    fun_subst_rectype (trG A g) tvs tus (trG A g) := by
  have h2 : Forall₂ (fun v st => fun_subst_subtype st [] [] v) (trSs A none g) (trSs A none g) := by
    rw [trSs_eq]; exact forall2_map g _ _ _ h
  exact .fun_subst_rectype_case_0 _ _ _ [] [] _ _ h2 h2 (by rw [hm]; rfl) (by rw [hm]; simp [Forall])
    (by rw [hm]; simp [Forall]) (wf_trG hA g hg) rfl rfl

theorem subst_def (g : List CT) (hg : OkG g) (i : Nat) {tvs tus} (hm : minus_recs tvs tus = some ([], []))
    (h : ∀ c ∈ g, fun_subst_subtype (trS A none c) [] [] (trS A none c)) :
    fun_subst_deftype (._DEF (trG A g) i) tvs tus (._DEF (trG A g) i) :=
  .fun_subst_deftype_case_0 _ _ _ _ _ _ (subst_nested hA g hg hm h) (subst_nested hA g hg hm h)
    (.deftype_case_0 _ _ (wf_trG hA g hg))

mutual
theorem subst_trV (hMR : MinusRecs) {tvs tus u n} (hs : SubstOK tvs tus u n) :
    ∀ c, OkV n c → fun_subst_storagetype (trV A none c) tvs tus (trV A u c)
  | .atom a, _ => by simp only [trV]; exact hA.subst a tvs tus
  | .inner i null, h => by
    simp only [trV, OkV] at h ⊢
    rw [← heaptype_recUse u i]
    exact subst_ref (.fun_subst_heaptype_case_0 i tvs tus _ (hs.recs i h))
  | .dt g i null, h => by
    simp only [trV, OkV] at h ⊢
    exact subst_ref (.fun_subst_heaptype_case_2 _ _ _ _ _ (subst_def hA g ⟨h.1, h.2.1⟩ i hs.minus
      (subst_trSs hMR (substOK_nil hMR g.length) g h.2.1)))
  | .sub .., h => by simp [OkV] at h

theorem subst_trU (hMR : MinusRecs) {tvs tus u n} (hs : SubstOK tvs tus u n) :
    ∀ s, OkSuper n (some s) → fun_subst_typeuse (trU A none s) tvs tus (trU A u s)
  | .inner i _, h => by
    simp only [trU, OkSuper] at h ⊢
    exact .fun_subst_typeuse_case_0 i tvs tus _ (hs.recs i h)
  | .dt g i _, h => by
    simp only [trU, OkSuper] at h ⊢
    exact .fun_subst_typeuse_case_2 _ _ _ _ _ (subst_def hA g ⟨h.1, h.2.1⟩ i hs.minus
      (subst_trSs hMR (substOK_nil hMR g.length) g h.2.1))
  | .atom _, h | .sub .., h => by simp [OkSuper] at h

theorem subst_trFs (hMR : MinusRecs) {tvs tus u n} (hs : SubstOK tvs tus u n) :
    ∀ fs, OkFs n fs → ∀ f ∈ fs, fun_subst_fieldtype (.mk_fieldtype (mutOpt f.2) (trV A none f.1)) tvs tus
      (.mk_fieldtype (mutOpt f.2) (trV A u f.1))
  | [], _ => by simp
  | (c, m) :: fs, h => by
    simp only [OkFs] at h
    simp only [List.mem_cons]
    rintro f (rfl | hf)
    · exact .fun_subst_fieldtype_case_0 _ _ _ _ _ (subst_trV hMR hs c h.1)
    · exact subst_trFs hMR hs fs h.2 f hf

theorem subst_trS (hMR : MinusRecs) {tvs tus u n} (hs : SubstOK tvs tus u n) :
    ∀ c, OkS n c → fun_subst_subtype (trS A none c) tvs tus (trS A u c)
  | .sub fin kind super counts fields, h => by
    simp only [OkS] at h
    rw [trS.eq_1, trS.eq_1, trFs_eq, trFs_eq]
    refine .fun_subst_subtype_case_0 _ _ _ _ _ _ _
      (subst_comp kind counts fields _ _ (subst_trFs hMR hs fields h.2.2)) ?_ ?_
    · cases super with
      | none => intro x y hxy; simp [trSup] at hxy
      | some s =>
        intro x y hxy
        simp [trSup] at hxy
        obtain ⟨rfl, rfl⟩ := hxy
        exact subst_trU hMR hs s h.1
    · cases super <;> rfl
  | .atom _, h | .inner .., h | .dt .., h => by simp [OkS] at h

theorem subst_trSs (hMR : MinusRecs) {tvs tus u n} (hs : SubstOK tvs tus u n) :
    ∀ cs, OkL n cs → ∀ c ∈ cs, fun_subst_subtype (trS A none c) tvs tus (trS A u c)
  | [], _ => by simp
  | c :: cs, h => by
    simp only [OkL] at h
    simp only [List.mem_cons]
    rintro x (rfl | hx)
    · exact subst_trS hMR hs x h.1
    · exact subst_trSs hMR hs cs h.2 x hx
end

/-! ### Unrolling -/

omit hA in
theorem subst_rec_list (rt : rectype) : ∀ (l : List Nat) (i : Nat),
    fun_subst_typevar (.REC i) (l.map fun j => (.REC j : typevar)) (l.map fun j => (._DEF rt j : typeuse))
      (some (if i ∈ l then ._DEF rt i else .REC i))
  | [], i => by
    simp only [List.map_nil, List.not_mem_nil, if_false]
    exact fun_subst_typevar.fun_subst_typevar_case_0 (.REC i)
  | j :: l, i => by
    have h := fun_subst_typevar.fun_subst_typevar_case_1 (.REC i) (.REC j) _ (._DEF rt j) _ _ (subst_rec_list rt l i)
    have e : Option.map (fun v => if (typevar.REC i == typevar.REC j) then typeuse._DEF rt j else v)
        (some (if i ∈ l then typeuse._DEF rt i else .REC i)) = some (if i ∈ j :: l then ._DEF rt i else .REC i) := by
      by_cases hij : i = j
      · subst hij; simp
      · have : (typevar.REC i == typevar.REC j) = false := by simp [hij]
        simp [this, hij]
    rw [e] at h
    exact h

omit hA in
theorem minusRecs_rec (l : List Nat) (tus : List typeuse) (h : l.length = tus.length) :
    minusRecs (l.map fun j => (.REC j : typevar)) tus = some ([], []) := by
  induction l generalizing tus with
  | nil => cases tus <;> simp_all [minusRecs]
  | cons j l ih => cases tus with
    | nil => simp at h
    | cons tu tus => simp only [List.map_cons, minusRecs]; exact ih tus (by simpa using h)

omit hA in
theorem substOK_unroll (hMR : MinusRecs) (rt : rectype) (n : Nat) :
    SubstOK ((List.range n).map fun j => (.REC j : typevar)) ((List.range n).map fun j => (._DEF rt j : typeuse))
      (some rt) n where
  recs i hi := by have := subst_rec_list rt (List.range n) i; simp [hi] at this; exact this
  minus := by rw [hMR]; exact minusRecs_rec _ _ (by simp)

theorem wf_trSs_some (g : List CT) (hg : OkG g) (s : subtype) (hs : s ∈ trSs A (some (trG A g)) g) :
    wf_subtype s :=
  wf_trSs hA (some (trG A g)) (fun r h => by cases h; exact wf_trG hA g hg) g.length g hg.2 s hs

/-- A defined type unrolls to its subtype with the references to its group unrolled. -/
theorem unroll_tr (hMR : MinusRecs) (g : List CT) (hg : OkG g) (i : Nat) (c : CT) (hc : g[i]? = some c) :
    fun_unrolldt (._DEF (trG A g) i) (trS A (some (trG A g)) c) := by
  have hr : fun_unrollrt (trG A g) (.REC (.mk_list (trSs A (some (trG A g)) g))) := by
    have F := forall2_map (R := fun v st => fun_subst_subtype st ((List.range g.length).map fun j => (.REC j : typevar))
        ((List.range g.length).map fun j => (._DEF (trG A g) j : typeuse)) v) g (trS A none) (trS A (some (trG A g)))
      (subst_trSs hA hMR (substOK_unroll hMR (trG A g) g.length) g hg.2)
    rw [← trSs_eq, ← trSs_eq] at F
    have hw : wf_rectype (.REC (.mk_list (trSs A (some (trG A g)) g))) :=
      .rectype_case_0 _ (by simpa [proj_list_0, trSs_length] using hg.1)
        (by simpa [proj_list_0, Forall] using wf_trSs_some hA g hg)
    exact .fun_unrollrt_case_0 _ 0 g.length _ _ _ F F rfl hw (wf_trG hA g hg) (by simp [trSs_length])
      (by simp [trSs_length]) (by simp [trSs_length])
  have hw : wf_rectype (.REC (.mk_list (trSs A (some (trG A g)) g))) :=
    .rectype_case_0 _ (by simpa [proj_list_0, trSs_length] using hg.1)
      (by simpa [proj_list_0, Forall] using wf_trSs_some hA g hg)
  have h := fun_unrolldt.fun_unrolldt_case_0 (trG A g) i _ _ hr rfl hw
  have e : (trSs A (some (trG A g)) g)[i]! = trS A (some (trG A g)) c := by
    rw [trSs_eq]
    have hi : i < g.length := by
      rcases Nat.lt_or_ge i g.length with h | h
      · exact h
      · simp [List.getElem?_eq_none h] at hc
    rw [List.getElem!_eq_getElem?_getD, List.getElem?_map, hc]
    rfl
  rw [e] at h
  exact h

/-! ### Subtyping -/

/-- The empty context, in which closed types are checked. -/
def C0 : context :=
  { TYPES := [], TAGS := [], GLOBALS := [], MEMS := [], TABLES := [], FUNCS := [], DATAS := [], ELEMS := [],
    LOCALS := [], LABELS := [], RETURN := none, REFS := [], RECS := [] }

omit hA in
theorem wfC0 : wf_context C0 := by
  unfold C0; constructor <;> simp [Forall]

def absOf : Abs → absheaptype
  | .any => .ANY | .eq => .EQ | .i31 => .I31 | .struct => .STRUCT | .array => .ARRAY | .none => .NONE
  | .func => .FUNC | .nofunc => .NOFUNC | .extern => .EXTERN | .noextern => .NOEXTERN | .exn => .EXN
  | .noexn => .NOEXN

omit hA in
theorem trAbs_eq (h : Abs) : trAbs h = heaptype_absheaptype (absOf h) := by cases h <;> rfl

omit hA in
theorem ok_trAbs (h : Abs) : Heaptype_ok C0 (trAbs h) := by rw [trAbs_eq]; exact .abs C0 _ wfC0

omit hA in
theorem sub_refl (h : Abs) : Heaptype_sub C0 (trAbs h) (trAbs h) := .refl C0 _ wfC0 (wf_trAbs h)

omit hA in
theorem sub_trans {a b c : heaptype} (hb : Heaptype_ok C0 b) (wa : wf_heaptype a) (wb : wf_heaptype b)
    (wc : wf_heaptype c) (h1 : Heaptype_sub C0 a b) (h2 : Heaptype_sub C0 b c) : Heaptype_sub C0 a c :=
  .trans C0 a c b hb h1 h2 wfC0 wa wc wb

omit hA in
theorem sub_eq_any {h : heaptype} (wh : wf_heaptype h) (s : Heaptype_sub C0 h .EQ) : Heaptype_sub C0 h .ANY :=
  sub_trans (ok_trAbs .eq) wh (wf_trAbs .eq) (wf_trAbs .any) s (.eq_any C0 wfC0 .heaptype_case_1 .heaptype_case_0)

omit hA in
theorem abs_sound (a b : Abs) (h : absSub a b = true) : Heaptype_sub C0 (trAbs a) (trAbs b) := by
  have i31 : Heaptype_sub C0 .I31 .EQ := .i31_eq C0 wfC0 .heaptype_case_2 .heaptype_case_1
  have st : Heaptype_sub C0 .STRUCT .EQ := .struct_eq C0 wfC0 .heaptype_case_3 .heaptype_case_1
  have ar : Heaptype_sub C0 .ARRAY .EQ := .array_eq C0 wfC0 .heaptype_case_4 .heaptype_case_1
  have none_ : ∀ x : Abs, Heaptype_sub C0 (trAbs x) .ANY → Heaptype_sub C0 .NONE (trAbs x) := fun x hx =>
    .none C0 _ hx (by cases x <;> simp [trAbs]) wfC0 (wf_trAbs x) .heaptype_case_5 .heaptype_case_0 .heaptype_case_12
  have any : Heaptype_sub C0 .ANY .ANY := sub_refl .any
  cases a <;> cases b <;> simp [absSub] at h <;> first
    | exact sub_refl _
    | exact .eq_any C0 wfC0 .heaptype_case_1 .heaptype_case_0
    | exact i31 | exact st | exact ar
    | exact sub_eq_any (wf_trAbs _) i31 | exact sub_eq_any (wf_trAbs _) st | exact sub_eq_any (wf_trAbs _) ar
    | exact none_ .any any
    | exact none_ .eq (.eq_any C0 wfC0 .heaptype_case_1 .heaptype_case_0)
    | exact none_ .i31 (sub_eq_any (wf_trAbs _) i31) | exact none_ .struct (sub_eq_any (wf_trAbs _) st)
    | exact none_ .array (sub_eq_any (wf_trAbs _) ar)
    | exact .nofunc C0 _ (sub_refl .func) (by simp [trAbs]) wfC0 .heaptype_case_6 .heaptype_case_7
        .heaptype_case_6 .heaptype_case_12
    | exact .noextern C0 _ (sub_refl .extern) (by simp [trAbs]) wfC0 .heaptype_case_10 .heaptype_case_11
        .heaptype_case_10 .heaptype_case_12
    | exact .noexn C0 _ (sub_refl .exn) (by simp [trAbs]) wfC0 .heaptype_case_8 .heaptype_case_9
        .heaptype_case_8 .heaptype_case_12

/-- Defined types that translate to well-formed types. -/
def OkD (d : DT) : Prop := OkG d.1 ∧ d.2 < d.1.length

def trD (A : Nat → storagetype) (d : DT) : heaptype := ._DEF (trG A d.1) d.2

def trH (A : Nat → storagetype) : CH → heaptype
  | .abs h => trAbs h
  | .defd d => trD A d

def OkH : CH → Prop
  | .abs _ => True
  | .defd d => OkD d

omit hA in
theorem okL_get {n : Nat} {g : List CT} (h : OkL n g) {i : Nat} (hi : i < g.length) : OkS n g[i] := by
  induction g generalizing i with
  | nil => simp at hi
  | cons c g ih =>
    simp only [OkL] at h
    cases i with
    | zero => exact h.1
    | succ i => exact ih h.2 (by simpa using hi)

omit hA in
/-- The subtype of a defined type. -/
theorem okD_sub {d : DT} (h : OkD d) : ∃ fin kind super counts fields,
    d.1[d.2]? = some (.sub fin kind super counts fields) ∧ OkSuper d.1.length super ∧ fields.length < 2 ^ 32 ∧
    OkFs d.1.length fields := by
  have hs := okL_get h.1.2 h.2
  rw [List.getElem?_eq_getElem h.2]
  revert hs
  cases d.1[d.2]'h.2 with
  | sub fin kind super counts fields => intro hs; exact ⟨fin, kind, super, counts, fields, rfl, hs⟩
  | _ => intro hs; simp [OkS] at hs

theorem wf_trD {d : DT} (h : OkD d) : wf_heaptype (trD A d) := .heaptype_case_14 _ _ (wf_trG hA _ h.1)

theorem expand_tr (hMR : MinusRecs) {d : DT} (h : OkD d) {fin kind super counts fields}
    (hc : d.1[d.2]? = some (.sub fin kind super counts fields)) (hs : OkSuper d.1.length super)
    (hl : fields.length < 2 ^ 32) (hf : OkFs d.1.length fields) :
    Expand (._DEF (trG A d.1) d.2) (trComp kind counts (trFs A (some (trG A d.1)) fields)) ∧
    wf_comptype (trComp kind counts (trFs A (some (trG A d.1)) fields)) := by
  have hu : ∀ r, some (trG A d.1) = some r → wf_rectype r := fun r e => by cases e; exact wf_trG hA _ h.1
  have hw := wf_trS hA (some (trG A d.1)) hu d.1.length (.sub fin kind super counts fields) ⟨hs, hl, hf⟩
  have hu' := unroll_tr hA hMR d.1 h.1 d.2 _ hc
  rw [trS.eq_1] at hw hu'
  refine ⟨.mk_Expand _ _ _ _ _ hu' rfl (.deftype_case_0 _ _ (wf_trG hA _ h.1)) hw hw, ?_⟩
  cases hw; assumption

theorem def_abs_sound (hMR : MinusRecs) {d : DT} (h : OkD d) {b : Abs} (hb : defAbs (kindOf d) b = true) :
    Heaptype_sub C0 (trD A d) (trAbs b) := by
  obtain ⟨fin, kind, super, counts, fields, hc, hs, hl, hf⟩ := okD_sub h
  obtain ⟨he, hw⟩ := expand_tr hA hMR h hc hs hl hf
  have hk : kindOf d = kind := by simp [kindOf, hc]
  rw [hk] at hb
  have hd := wf_trD hA h
  have toStruct : kind = kStruct → Heaptype_sub C0 (trD A d) .STRUCT := fun e => by
    simp only [trComp, e, ite_true] at he hw
    exact .struct C0 _ _ he wfC0 (.deftype_case_0 _ _ (wf_trG hA _ h.1)) .heaptype_case_3 hw
  have toArray : kind = kArray → Heaptype_sub C0 (trD A d) .ARRAY := fun e => by
    simp only [trComp, e, kArray, kStruct] at he hw
    exact .array C0 _ _ he wfC0 (.deftype_case_0 _ _ (wf_trG hA _ h.1)) .heaptype_case_4 hw
  have toFunc : kind = kFunc → Heaptype_sub C0 (trD A d) .FUNC := fun e => by
    simp only [trComp, e, kArray, kStruct, kFunc] at he hw
    exact .func C0 _ _ _ he wfC0 (.deftype_case_0 _ _ (wf_trG hA _ h.1)) .heaptype_case_6 hw
  have st : Heaptype_sub C0 .STRUCT .EQ := .struct_eq C0 wfC0 .heaptype_case_3 .heaptype_case_1
  have ar : Heaptype_sub C0 .ARRAY .EQ := .array_eq C0 wfC0 .heaptype_case_4 .heaptype_case_1
  have toEq : (kind = kStruct ∨ kind = kArray) → Heaptype_sub C0 (trD A d) .EQ := fun e => by
    rcases e with e | e
    · exact sub_trans (ok_trAbs .struct) hd .heaptype_case_3 .heaptype_case_1 (toStruct e) st
    · exact sub_trans (ok_trAbs .array) hd .heaptype_case_4 .heaptype_case_1 (toArray e) ar
  cases b <;> simp [defAbs] at hb
  · exact sub_eq_any hd (toEq hb)
  · exact toEq hb
  · exact toStruct hb
  · exact toArray hb
  · exact toFunc hb

theorem abs_def_sound (hMR : MinusRecs) {d : DT} (h : OkD d) {a : Abs} (ha : absDef (kindOf d) a = true) :
    Heaptype_sub C0 (trAbs a) (trD A d) := by
  have hd := wf_trD hA h
  have ne : trD A d ≠ .BOT := by simp [trD]
  cases a <;> simp [absDef] at ha
  · have hb : defAbs (kindOf d) .any = true := by simp [defAbs, ha]
    exact .none C0 _ (def_abs_sound hA hMR h hb) ne wfC0 hd .heaptype_case_5 .heaptype_case_0 .heaptype_case_12
  · have hb : defAbs (kindOf d) .func = true := by simp [defAbs, ha]
    exact .nofunc C0 _ (def_abs_sound hA hMR h hb) ne wfC0 hd .heaptype_case_7 .heaptype_case_6 .heaptype_case_12

/-- One step up the declared supertype. -/
theorem super_sound (hMR : MinusRecs) {d e : DT} (h : OkD d) (s : superOf d = some e) :
    OkD e ∧ ∀ d', Heaptype_sub C0 (trD A e) (trD A d') → OkD d' →
      Heaptype_sub C0 (trD A d) (trD A d') := by
  obtain ⟨fin, kind, super, counts, fields, hc, hs, hl, hf⟩ := okD_sub h
  have hu : ∀ r, some (trG A d.1) = some r → wf_rectype r := fun r e => by cases e; exact wf_trG hA _ h.1
  have hw := wf_trS hA (some (trG A d.1)) hu d.1.length (.sub fin kind super counts fields) ⟨hs, hl, hf⟩
  have hun := unroll_tr hA hMR d.1 h.1 d.2 _ hc
  rw [trS.eq_1] at hw hun
  have step : ∀ (t : CT), super = some t → OkD e → trD A e = heaptype_typeuse (trU A (some (trG A d.1)) t) →
      ∀ d', Heaptype_sub C0 (trD A e) (trD A d') → OkD d' → Heaptype_sub C0 (trD A d) (trD A d') := by
    intro t ht he ee d' hs' hd'
    subst ht
    have ds : Deftype_sub C0 (._DEF (trG A d.1) d.2) (._DEF (trG A d'.1) d'.2) :=
      .super C0 _ _ _ _ _ 0 _ hun rfl (by simp [trSup]) (by
        have e0 : ([trU A (some (trG A d.1)) t] : List typeuse)[0]! = trU A (some (trG A d.1)) t := rfl
        simp only [trSup, e0]; rw [← ee]; exact hs') wfC0
        (.deftype_case_0 _ _ (wf_trG hA _ h.1)) (.deftype_case_0 _ _ (wf_trG hA _ hd'.1)) hw hw
    exact .def C0 _ _ ds wfC0 (.deftype_case_0 _ _ (wf_trG hA _ h.1)) (.deftype_case_0 _ _ (wf_trG hA _ hd'.1))
  simp only [superOf, hc] at s
  cases super with
  | none => simp at s
  | some t =>
    cases t with
    | inner k null =>
      simp at s; subst s
      have he : OkD (d.1, k) := ⟨h.1, hs⟩
      exact ⟨he, step _ rfl he rfl⟩
    | dt g k null =>
      simp at s; subst s
      have he : OkD (g, k) := ⟨⟨hs.1, hs.2.1⟩, hs.2.2⟩
      exact ⟨he, step _ rfl he rfl⟩
    | atom _ => simp at s
    | sub _ _ _ _ _ => simp at s

theorem chain_sound (hMR : MinusRecs) {d d' : DT} (c : Chain d d') (h : OkD d) :
    Heaptype_sub C0 (trD A d) (trD A d') ∧ OkD d' := by
  induction c with
  | refl d => exact ⟨.refl C0 _ wfC0 (wf_trD hA h), h⟩
  | step s _ ih =>
    obtain ⟨he, k⟩ := super_sound hA hMR h s
    obtain ⟨ih1, ih2⟩ := ih he
    exact ⟨k _ ih1 ih2, ih2⟩

/-- The algorithmic subtyping, which `wah_type_is_subtype` follows, is sound for the spec. -/
theorem algo_sound (hMR : MinusRecs) {a b : CH} (ha : OkH a) (hb : OkH b) (h : Algo a b) :
    Heaptype_sub C0 (trH A a) (trH A b) := by
  cases a with
  | abs x => cases b with
    | abs y => exact abs_sound x y h
    | defd d => exact abs_def_sound hA hMR hb h
  | defd d => cases b with
    | abs y => exact def_abs_sound hA hMR ha h
    | defd d' => exact (chain_sound hA hMR h ha).1

/-- Heap subtyping of WahProofs/Subtype.lean implies `Heaptype_sub` of the spec for closed types. -/
theorem hsub_sound (hMR : MinusRecs) {a b : CH} (ha : OkH a) (hb : OkH b) (hv : ValidH a) (h : HSub a b) :
    Heaptype_sub C0 (trH A a) (trH A b) :=
  algo_sound hA hMR ha hb (algo_of_hsub h hv)

end Wah.SpecTec
