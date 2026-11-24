/-
Completeness of the heap subtyping of WahProofs/Subtype.lean for `Heaptype_sub` of the SpecTec model: subtypes in
the spec are subtypes in `HSub`, for closed types in the empty context.

The proof goes through an algorithmic relation `SAlgo` on SpecTec heap types, which `Heaptype_sub` implies for
heap types whose supertype chains keep their kinds (`Good`), as valid types (`Heaptype_ok`) do. The middle types of
transitivity are valid, so the induction stays within `Good` types.
-/
import WahSpecTec.Subtype

set_option linter.unusedSectionVars false

namespace Wah.SpecTec
open Wah.Types Wah.Subtype

/-! ### The empty substitution -/

theorem forall2_eq {α : Type} :
    ∀ (l1 l2 : List α), l1.length = l2.length → Forall₂ (fun a b => a = b) l1 l2 → l1 = l2
  | [], [], _, _ => rfl
  | a :: l1, b :: l2, hl, h => by
    have e : a = b := h a b (by simp)
    have r : l1 = l2 := forall2_eq l1 l2 (by simpa using hl) (fun x y hm => h x y (by simp [hm]))
    rw [e, r]
  | [], _ :: _, hl, _ | _ :: _, [], hl, _ => by simp at hl

theorem subst_typevar_nil {tv : typevar} {tvs tus r} (h : fun_subst_typevar tv tvs tus r) (h1 : tvs = [])
    (h2 : tus = []) : r = some (typeuse_typevar tv) := by
  subst h1 h2
  cases h
  · rfl
  · rename_i hb
    exact absurd (fun_subst_typevar_before_fun_subst_typevar_case_2.fun_subst_typevar_case_0 tv) hb

theorem forall2_id {α : Type} {R : α → α → Prop} (hR : ∀ {x y}, R x y → x = y) {l1 l2 : List α}
    (hl : l1.length = l2.length) (h : Forall₂ R l1 l2) : l1 = l2 :=
  forall2_eq l1 l2 hl fun x y hm => hR (h x y hm)

/-! ### Shapes of closed forms -/

/-- Atoms of well-shaped closed forms: no other atom translates to the same type, which is neither a concrete
reference nor bottom (where other atoms may go). -/
def ShapeAtom (A : Nat → storagetype) (a : Nat) : Prop :=
  (∀ b, A b = A a → b = a) ∧ (∀ n i, A a ≠ .REF n (.REC i)) ∧ (∀ n rt i, A a ≠ .REF n (._DEF rt i)) ∧
    A a ≠ .BOT

/-- Fields of function types are value types. -/
def NotPacked (A : Nat → storagetype) : CT → Prop
  | .atom a => A a ≠ .I8 ∧ A a ≠ .I16
  | _ => True

def FuncFields (A : Nat → storagetype) (fs : List (CT × Bool)) : Prop :=
  ∀ f ∈ fs, f.2 = false ∧ NotPacked A f.1

mutual
/-- The shapes of `wah_type_def_t`: structs and arrays have `(field_count, 0)` as counts, arrays one field, and
functions immutable value types as fields, split by the counts. -/
def ShapeV (A : Nat → storagetype) : CT → Prop
  | .atom a => ShapeAtom A a
  | .inner .. => True
  | .dt g _ _ => ShapeL A g
  | .sub .. => False

def ShapeSup (A : Nat → storagetype) : Option CT → Prop
  | none => True
  | some (.inner _ null) => null = false
  | some (.dt g _ null) => null = false ∧ ShapeL A g
  | some _ => False

def ShapeFs (A : Nat → storagetype) : List (CT × Bool) → Prop
  | [] => True
  | (c, _) :: fs => ShapeV A c ∧ ShapeFs A fs

def ShapeS (A : Nat → storagetype) : CT → Prop
  | .sub _ kind super counts fields => ShapeSup A super ∧ ShapeFs A fields ∧
    (kind = kStruct ∧ counts = (fields.length, 0) ∨ kind = kArray ∧ fields.length = 1 ∧ counts = (1, 0) ∨
     kind = kFunc ∧ counts.1 + counts.2 = fields.length ∧ FuncFields A fields)
  | _ => False

def ShapeL (A : Nat → storagetype) : List CT → Prop
  | [] => True
  | c :: cs => ShapeS A c ∧ ShapeL A cs
end

theorem nullOpt_inj {a b : Bool} (h : nullOpt a = nullOpt b) : a = b := by
  cases a <;> cases b <;> simp_all [nullOpt]

theorem mutOpt_inj {a b : Bool} (h : mutOpt a = mutOpt b) : a = b := by
  cases a <;> cases b <;> simp_all [mutOpt]

theorem finalOpt_inj {a b : Bool} (h : finalOpt a = finalOpt b) : a = b := by
  cases a <;> cases b <;> simp_all [finalOpt]

/-- Storage types that function types can have, on which `toVal` is injective. -/
def ValLike : storagetype → Prop
  | .I8 | .I16 | .BOT => False
  | _ => True

theorem toVal_inj {t1 t2 : storagetype} (h1 : ValLike t1) (h2 : ValLike t2) (h : toVal t1 = toVal t2) : t1 = t2 := by
  cases t1 <;> cases t2 <;> simp_all [ValLike, toVal]

theorem valLike_trV {A : Nat → storagetype} {c : CT} (hp : NotPacked A c) (hs : ShapeV A c) :
    ValLike (trV A none c) := by
  cases c with
  | atom a =>
    simp only [trV]; simp only [NotPacked] at hp
    have := hs.2.2.2
    revert hp this; cases A a <;> simp [ValLike]
  | inner _ _ => simp [trV, ValLike]
  | dt _ _ _ => simp [trV, ValLike]
  | sub _ _ _ _ _ => simp [ShapeV] at hs

mutual
theorem inj_trV {A : Nat → storagetype} :
    ∀ {c1 c2 : CT}, ShapeV A c1 → ShapeV A c2 → trV A none c1 = trV A none c2 → c1 = c2
  | .atom a, .atom b, s1, _, h => by simp only [trV] at h; rw [s1.1 b h.symm]
  | .atom a, .inner i n, s1, _, h => by simp only [trV, recRef] at h; exact absurd h (s1.2.1 _ i)
  | .atom a, .dt g i n, s1, _, h => by simp only [trV] at h; exact absurd h (s1.2.2.1 _ _ i)
  | .inner i n, .atom a, _, s2, h => by simp only [trV, recRef] at h; exact absurd h.symm (s2.2.1 _ i)
  | .dt g i n, .atom a, _, s2, h => by simp only [trV] at h; exact absurd h.symm (s2.2.2.1 _ _ i)
  | .inner i n, .inner j m, _, _, h => by
    simp only [trV, recRef, storagetype.REF.injEq, heaptype.REC.injEq] at h
    rw [nullOpt_inj h.1, h.2]
  | .inner i n, .dt g j m, _, _, h => by simp [trV, recRef] at h
  | .dt g j m, .inner i n, _, _, h => by simp [trV, recRef] at h
  | .dt g i n, .dt g' j m, s1, s2, h => by
    simp only [trV, storagetype.REF.injEq, heaptype._DEF.injEq, rectype.REC.injEq, list.mk_list.injEq] at h
    simp only [ShapeV] at s1 s2
    rw [nullOpt_inj h.1, inj_trSs s1 s2 h.2.1, h.2.2]
  | .sub .., _, s1, _, _ => by simp [ShapeV] at s1
  | .atom _, .sub .., _, s2, _ | .inner .., .sub .., _, s2, _ | .dt .., .sub .., _, s2, _ => by simp [ShapeV] at s2
termination_by structural c1 _ _ _ _ => c1

theorem inj_trU {A : Nat → storagetype} :
    ∀ {s1 s2 : CT}, ShapeSup A (some s1) → ShapeSup A (some s2) → trU A none s1 = trU A none s2 → s1 = s2
  | .inner i n, .inner j m, h1, h2, h => by
    simp only [trU, recUse, typeuse.REC.injEq] at h
    simp only [ShapeSup] at h1 h2
    rw [h, h1, h2]
  | .inner i n, .dt g j m, _, _, h => by simp [trU, recUse] at h
  | .dt g j m, .inner i n, _, _, h => by simp [trU, recUse] at h
  | .dt g i n, .dt g' j m, h1, h2, h => by
    simp only [trU, typeuse._DEF.injEq, rectype.REC.injEq, list.mk_list.injEq] at h
    simp only [ShapeSup] at h1 h2
    rw [inj_trSs h1.2 h2.2 h.1, h.2, h1.1, h2.1]
  | .atom _, _, h1, _, _ | .sub .., _, h1, _, _ => by simp [ShapeSup] at h1
  | .inner .., .atom _, _, h2, _ | .inner .., .sub .., _, h2, _ | .dt .., .atom _, _, h2, _
  | .dt .., .sub .., _, h2, _ => by simp [ShapeSup] at h2
termination_by structural s1 _ _ _ _ => s1

theorem inj_trFs {A : Nat → storagetype} :
    ∀ {fs1 fs2 : List (CT × Bool)}, ShapeFs A fs1 → ShapeFs A fs2 → trFs A none fs1 = trFs A none fs2 →
      fs1 = fs2
  | [], [], _, _, _ => rfl
  | (c, m) :: fs, (c', m') :: fs', s1, s2, h => by
    simp only [trFs, List.cons.injEq, fieldtype.mk_fieldtype.injEq] at h
    simp only [ShapeFs] at s1 s2
    rw [mutOpt_inj h.1.1, inj_trV s1.1 s2.1 h.1.2, inj_trFs s1.2 s2.2 h.2]
  | [], _ :: _, _, _, h | _ :: _, [], _, _, h => by simp [trFs] at h
termination_by structural fs1 _ _ _ _ => fs1

theorem inj_fn {A : Nat → storagetype} :
    ∀ {fs1 fs2 : List (CT × Bool)}, ShapeFs A fs1 → ShapeFs A fs2 → FuncFields A fs1 → FuncFields A fs2 →
      (trFs A none fs1).map fval = (trFs A none fs2).map fval → fs1 = fs2
  | [], [], _, _, _, _, _ => rfl
  | (c, m) :: fs, (c', m') :: fs', s1, s2, f1, f2, h => by
    simp only [trFs, List.map_cons, List.cons.injEq, fval] at h
    simp only [ShapeFs] at s1 s2
    have hm : m = false := (f1 (c, m) (by simp)).1
    have hm' : m' = false := (f2 (c', m') (by simp)).1
    have hv := toVal_inj (valLike_trV (f1 (c, m) (by simp)).2 s1.1) (valLike_trV (f2 (c', m') (by simp)).2 s2.1) h.1
    rw [inj_trV s1.1 s2.1 hv, hm, hm',
      inj_fn s1.2 s2.2 (fun f hf => f1 f (by simp [hf])) (fun f hf => f2 f (by simp [hf])) h.2]
  | [], _ :: _, _, _, _, _, h | _ :: _, [], _, _, _, _, h => by simp [trFs] at h
termination_by structural fs1 _ _ _ _ _ _ => fs1

theorem inj_trS {A : Nat → storagetype} :
    ∀ {c1 c2 : CT}, ShapeS A c1 → ShapeS A c2 → trS A none c1 = trS A none c2 → c1 = c2
  | .sub f1 k1 s1 n1 fs1, .sub f2 k2 s2 n2 fs2, h1, h2, h => by
    rw [trS.eq_1, trS.eq_1] at h
    simp only [subtype.SUB.injEq] at h
    obtain ⟨hf, hs, hc⟩ := h
    simp only [ShapeS] at h1 h2
    have es : s1 = s2 := by
      cases s1 with
      | none => cases s2 with
        | none => rfl
        | some _ => simp [trSup] at hs
      | some a => cases s2 with
        | none => simp [trSup] at hs
        | some b =>
          simp only [trSup, List.cons.injEq, and_true] at hs
          rw [inj_trU h1.1 h2.1 hs]
    rw [finalOpt_inj hf, es]
    rcases h1.2.2 with ⟨hk1, hn1⟩ | ⟨hk1, hl1, hn1⟩ | ⟨hk1, hn1, hfn1⟩ <;>
    rcases h2.2.2 with ⟨hk2, hn2⟩ | ⟨hk2, hl2, hn2⟩ | ⟨hk2, hn2, hfn2⟩ <;>
    subst hk1 hk2 <;> simp [trComp] at hc
    · have e := inj_trFs h1.2.1 h2.2.1 hc
      subst e; rw [hn1, hn2]
    · match fs1, fs2, hl1, hl2 with
      | [x1], [x2], _, _ =>
        have e := inj_trFs h1.2.1 h2.2.1 (by simp [trFs] at hc ⊢; exact hc)
        rw [e, hn1, hn2]
    · have ha : (trFs A none fs1).map fval = (trFs A none fs2).map fval := by
        rw [← List.take_append_drop n1.1 (List.map fval (trFs A none fs1)),
          ← List.take_append_drop n2.1 (List.map fval (trFs A none fs2)), hc.1, hc.2]
      have e := inj_fn h1.2.1 h2.2.1 hfn1 hfn2 ha
      subst e
      have l1 := congrArg List.length hc.1
      simp [trFs_eq] at l1
      have : n1.1 = n2.1 := by omega
      have : n1 = n2 := Prod.ext this (by omega)
      rw [this]
  | .atom _, _, h1, _, _ | .inner .., _, h1, _, _ | .dt .., _, h1, _, _ => by simp [ShapeS] at h1
  | .sub .., .atom _, _, h2, _ | .sub .., .inner .., _, h2, _ | .sub .., .dt .., _, h2, _ => by simp [ShapeS] at h2
termination_by structural c1 _ _ _ _ => c1

theorem inj_trSs {A : Nat → storagetype} :
    ∀ {l1 l2 : List CT}, ShapeL A l1 → ShapeL A l2 → trSs A none l1 = trSs A none l2 → l1 = l2
  | [], [], _, _, _ => rfl
  | c :: cs, c' :: cs', s1, s2, h => by
    simp only [trSs, List.cons.injEq] at h
    simp only [ShapeL] at s1 s2
    rw [inj_trS s1.1 s2.1 h.1, inj_trSs s1.2 s2.2 h.2]
  | [], _ :: _, _, _, h | _ :: _, [], _, _, h => by simp [trSs] at h
termination_by structural l1 _ _ _ _ => l1
end

variable (hMR : MinusRecs)
include hMR

theorem minus_nil : Option.get! (minus_recs [] []) = ([], []) := by rw [hMR]; rfl

mutual
theorem id_typeuse : ∀ {x tvs tus y}, fun_subst_typeuse x tvs tus y → tvs = [] → tus = [] → y = x
  | _, _, _, _, .fun_subst_typeuse_case_0 n _ _ _ h, e1, e2 => by rw [subst_typevar_nil h e1 e2]; rfl
  | _, _, _, _, .fun_subst_typeuse_case_1 x _ _ _ h, e1, e2 => by rw [subst_typevar_nil h e1 e2]; rfl
  | _, _, _, _, .fun_subst_typeuse_case_2 _ _ _ _ _ h, e1, e2 => by rw [id_deftype h e1 e2]; rfl

theorem id_heaptype : ∀ {x tvs tus y}, fun_subst_heaptype x tvs tus y → tvs = [] → tus = [] → y = x
  | _, _, _, _, .fun_subst_heaptype_case_0 n _ _ _ h, e1, e2 => by rw [subst_typevar_nil h e1 e2]; rfl
  | _, _, _, _, .fun_subst_heaptype_case_1 x _ _ _ h, e1, e2 => by rw [subst_typevar_nil h e1 e2]; rfl
  | _, _, _, _, .fun_subst_heaptype_case_2 _ _ _ _ _ h, e1, e2 => by rw [id_deftype h e1 e2]; rfl
  | _, _, _, _, .fun_subst_heaptype_case_3 _ _ _ _, _, _ => rfl

theorem id_reftype : ∀ {x tvs tus y}, fun_subst_reftype x tvs tus y → tvs = [] → tus = [] → y = x
  | _, _, _, _, .fun_subst_reftype_case_0 _ _ _ _ _ h, e1, e2 => by rw [id_heaptype h e1 e2]

theorem id_valtype : ∀ {x tvs tus y}, fun_subst_valtype x tvs tus y → tvs = [] → tus = [] → y = x
  | _, _, _, _, .fun_subst_valtype_case_5 _ _ _ _ _ h, e1, e2 => by rw [id_reftype h e1 e2]; rfl
  | _, _, _, _, .fun_subst_valtype_case_0 .., _, _ | _, _, _, _, .fun_subst_valtype_case_1 .., _, _
  | _, _, _, _, .fun_subst_valtype_case_2 .., _, _ | _, _, _, _, .fun_subst_valtype_case_3 .., _, _
  | _, _, _, _, .fun_subst_valtype_case_4 .., _, _ | _, _, _, _, .fun_subst_valtype_case_6 .., _, _ => rfl

theorem id_storagetype : ∀ {x tvs tus y}, fun_subst_storagetype x tvs tus y → tvs = [] → tus = [] → y = x
  | _, _, _, _, .fun_subst_storagetype_case_0 _ _ _ h, e1, e2 => by rw [id_valtype h e1 e2]; rfl
  | _, _, _, _, .fun_subst_storagetype_case_1 _ _ _ _ _ h, e1, e2 => by rw [id_valtype h e1 e2]; rfl
  | _, _, _, _, .fun_subst_storagetype_case_2 _ _ _ h, e1, e2 => by rw [id_valtype h e1 e2]; rfl
  | _, _, _, _, .fun_subst_storagetype_case_3 _ _ _ h, e1, e2 => by rw [id_valtype h e1 e2]; rfl
  | _, _, _, _, .fun_subst_storagetype_case_4 _ _ _ h, e1, e2 => by rw [id_valtype h e1 e2]; rfl
  | _, _, _, _, .fun_subst_storagetype_case_5 _ _ _ h, e1, e2 => by rw [id_valtype h e1 e2]; rfl
  | _, _, _, _, .fun_subst_storagetype_case_6 _ _ _ h, e1, e2 => by rw [id_valtype h e1 e2]; rfl
  | _, _, _, _, .fun_subst_storagetype_case_7 .., _, _ | _, _, _, _, .fun_subst_storagetype_case_8 .., _, _ => rfl

theorem id_fieldtype : ∀ {x tvs tus y}, fun_subst_fieldtype x tvs tus y → tvs = [] → tus = [] → y = x
  | _, _, _, _, .fun_subst_fieldtype_case_0 _ _ _ _ _ h, e1, e2 => by rw [id_storagetype h e1 e2]

theorem id_comptype : ∀ {x tvs tus y}, fun_subst_comptype x tvs tus y → tvs = [] → tus = [] → y = x
  | _, _, _, _, .fun_subst_comptype_case_0 _ _ _ _ h hl, e1, e2 => by
    rw [forall2_eq _ _ hl (fun x y hm => id_fieldtype (h x y hm) e1 e2)]
  | _, _, _, _, .fun_subst_comptype_case_1 _ _ _ _ h, e1, e2 => by rw [id_fieldtype h e1 e2]
  | _, _, _, _, .fun_subst_comptype_case_2 _ _ _ _ _ _ h2 h1 l2 l1, e1, e2 => by
    rw [forall2_eq _ _ l1 (fun x y hm => id_valtype (h1 x y hm) e1 e2),
      forall2_eq _ _ l2 (fun x y hm => id_valtype (h2 x y hm) e1 e2)]

theorem id_subtype : ∀ {x tvs tus y}, fun_subst_subtype x tvs tus y → tvs = [] → tus = [] → y = x
  | _, _, _, _, .fun_subst_subtype_case_0 _ _ _ _ _ _ _ hc h hl, e1, e2 => by
    rw [id_comptype hc e1 e2, forall2_eq _ _ hl (fun x y hm => id_typeuse (h x y hm) e1 e2)]

theorem id_rectype : ∀ {x tvs tus y}, fun_subst_rectype x tvs tus y → tvs = [] → tus = [] → y = x
  | _, _, _, _, .fun_subst_rectype_case_0 _ _ _ tv' tu' _ _ _ h0 he _ _ _ _ l0, e1, e2 => by
    have ht : tv' = [] ∧ tu' = [] := by subst e1 e2; rw [minus_nil hMR] at he; exact Prod.mk.inj he
    rw [forall2_eq _ _ l0 (fun x y hm => id_subtype (h0 x y hm) ht.1 ht.2)]

theorem id_deftype : ∀ {x tvs tus y}, fun_subst_deftype x tvs tus y → tvs = [] → tus = [] → y = x
  | _, _, _, _, .fun_subst_deftype_case_0 _ _ _ _ _ _ _ h0 _, e1, e2 => by rw [id_rectype h0 e1 e2]
end

/-- Nested rec groups are left alone by substitutions without free variables left for them. -/
theorem rectype_fixed {rt tvs tus rt'} (h : fun_subst_rectype rt tvs tus rt')
    (hm : Option.get! (minus_recs tvs tus) = ([], [])) : rt' = rt := by
  cases h with
  | fun_subst_rectype_case_0 _ _ _ tv' tu' _ _ _ h0 he _ _ _ _ l0 =>
    rw [hm] at he
    obtain ⟨e1, e2⟩ := Prod.mk.inj he
    rw [forall2_eq _ _ l0 (fun x y hm => id_subtype hMR (h0 x y hm) e1 e2)]

theorem deftype_fixed {d tvs tus d'} (h : fun_subst_deftype d tvs tus d')
    (hm : Option.get! (minus_recs tvs tus) = ([], [])) : d' = d := by
  cases h with
  | fun_subst_deftype_case_0 _ _ _ _ _ _ _ h0 _ => rw [rectype_fixed hMR h0 hm]

omit hMR in
/-- What a substitution makes of a type variable. -/
def lookupTV (tv : typevar) : List typevar → List typeuse → typeuse
  | [], [] => typeuse_typevar tv
  | a :: as, b :: bs => if tv == a then b else lookupTV tv as bs
  | _, _ => default

omit hMR in
theorem subst_typevar_det {tv tvs tus r} (h : fun_subst_typevar tv tvs tus r) (hl : tvs.length = tus.length) :
    r = some (lookupTV tv tvs tus) := by
  induction h with
  | fun_subst_typevar_case_0 => rfl
  | fun_subst_typevar_case_1 tv1 tvs tu1 tus v _ ih =>
    rw [ih (by simpa using hl)]
    simp [lookupTV]
  | fun_subst_typevar_case_2 tvs tus hb =>
    exfalso; apply hb
    cases tvs <;> cases tus <;> simp at hl
    · exact .fun_subst_typevar_case_0 _
    · exact .fun_subst_typevar_case_1 _ _ _ _ _ none

omit hMR in
theorem lookup_rec (rt : rectype) (k : Nat) : ∀ l : List Nat,
    lookupTV (.REC k) (l.map fun j => (.REC j : typevar)) (l.map fun j => (._DEF rt j : typeuse)) =
      if k ∈ l then ._DEF rt k else .REC k
  | [] => rfl
  | j :: l => by
    simp only [List.map_cons, lookupTV, lookup_rec rt k l]
    by_cases h : k = j <;> simp [h]

omit hMR in
theorem lookup_idx (rt : rectype) (x : uN) : ∀ l : List Nat,
    lookupTV (._IDX x) (l.map fun j => (.REC j : typevar)) (l.map fun j => (._DEF rt j : typeuse)) = ._IDX x
  | [] => rfl
  | j :: l => by simp only [List.map_cons, lookupTV, lookup_idx rt x l]; simp

/-! ### Unrolling any defined type -/

omit hMR in
def rtSubs : rectype → List subtype
  | .REC (.mk_list l) => l

omit hMR in
/-- The subtype of a defined type as it is in its rec group. -/
def rolled : deftype → Option subtype
  | ._DEF rt i => (rtSubs rt)[i]?

omit hMR in
/-- The kind of a composite type, as `wah_comp_type_kind_t`. -/
def ckind : comptype → Nat
  | .STRUCT _ => kStruct
  | .ARRAY _ => kArray
  | .FUNC _ _ => kFunc

omit hMR in
/-- What unrolling makes of a supertype. -/
def unUse (rt : rectype) : typeuse → typeuse
  | .REC k => if k < (rtSubs rt).length then ._DEF rt k else .REC k
  | t => t

omit hMR in
theorem forall2_get {α β : Type} {R : α → β → Prop} {l1 : List α} {l2 : List β} (h : Forall₂ R l1 l2)
    (hl : l1.length = l2.length) {i : Nat} (hi : i < l2.length) : R (l1[i]'(by omega)) l2[i] :=
  h _ _ (by
    have : (l1.zip l2)[i]'(by simp; omega) = (l1[i]'(by omega), l2[i]) := by simp
    rw [← this]; exact List.getElem_mem _)

omit hMR in
theorem forall2_map_eq {α β : Type} {R : α → β → Prop} (g : β → α) (hg : ∀ {x y}, R x y → x = g y) :
    ∀ {l1 : List α} {l2 : List β}, l1.length = l2.length → Forall₂ R l1 l2 → l1 = l2.map g
  | [], [], _, _ => rfl
  | a :: l1, b :: l2, hl, h => by
    rw [List.map_cons, hg (h a b (by simp)),
      forall2_map_eq g hg (l1 := l1) (l2 := l2) (by simpa using hl) (fun x y hm => h x y (by simp [hm]))]
  | [], _ :: _, hl, _ | _ :: _, [], hl, _ => by simp at hl

theorem subst_unroll_use (rt : rectype) {tu v} (h : fun_subst_typeuse tu
    ((List.range (rtSubs rt).length).map fun j => (.REC j : typevar))
    ((List.range (rtSubs rt).length).map fun j => (._DEF rt j : typeuse)) v) : v = unUse rt tu := by
  have hm : Option.get! (minus_recs ((List.range (rtSubs rt).length).map fun j => (.REC j : typevar))
      ((List.range (rtSubs rt).length).map fun j => (._DEF rt j : typeuse))) = ([], []) := by
    rw [hMR, minusRecs_rec _ _ (by simp)]; rfl
  cases h with
  | fun_subst_typeuse_case_0 k _ _ r hr =>
    rw [subst_typevar_det hr (by simp), lookup_rec]; simp [unUse]
  | fun_subst_typeuse_case_1 x _ _ r hr =>
    rw [subst_typevar_det hr (by simp), lookup_idx]; rfl
  | fun_subst_typeuse_case_2 rt' k _ _ d hd => rw [deftype_fixed hMR hd hm]; rfl

omit hMR in
theorem subst_ckind {ct tvs tus ct'} (h : fun_subst_comptype ct tvs tus ct') : ckind ct' = ckind ct := by
  cases h <;> rfl

/-- Unrolling keeps finality and the kind, and unrolls the supertypes. -/
theorem unroll_shape {rt : rectype} {i : Nat} {s : subtype} (h : fun_unrolldt (._DEF rt i) s) {f tus ct}
    (hr : rolled (._DEF rt i) = some (.SUB f tus ct)) :
    ∃ ct', s = .SUB f (tus.map (unUse rt)) ct' ∧ ckind ct' = ckind ct := by
  cases h with
  | fun_unrolldt_case_0 _ _ sl v hrt he _ =>
    cases hrt with
    | fun_unrollrt_case_0 _ n stl v1 v0 _ F0 hrt' _ _ _ l2 l0 =>
      subst hrt'
      cases he
      simp only [rolled, rtSubs] at hr
      have hi : i < stl.length := by
        rcases Nat.lt_or_ge i stl.length with h | h
        · exact h
        · simp [List.getElem?_eq_none h] at hr
      have hs := forall2_get F0 (by omega) hi
      rw [List.getElem?_eq_getElem hi] at hr
      rw [Option.some.inj hr] at hs
      rw [List.getElem!_eq_getElem?_getD, List.getElem?_eq_getElem (by omega)]
      simp only [Option.getD_some]
      revert hs
      generalize sl[i]'_ = x
      intro hs
      rw [← l2] at hs
      cases hs with
      | fun_subst_subtype_case_0 _ _ _ _ _ c' us hc hus hl =>
        refine ⟨c', ?_, subst_ckind hc⟩
        congr 1
        exact forall2_map_eq (unUse _) (fun hx => subst_unroll_use hMR _ hx) hl hus

/-! ### Kinds and supertypes of defined types -/

omit hMR in
def kindS (d : deftype) : Nat :=
  match rolled d with
  | some (.SUB _ _ ct) => ckind ct
  | none => 0

omit hMR in
def rtOf : deftype → rectype
  | ._DEF rt _ => rt

omit hMR in
/-- The declared supertype, unrolled. -/
def superS (d : deftype) : Option deftype :=
  match rolled d with
  | some (.SUB _ [tu] _) => match unUse (rtOf d) tu with
    | ._DEF rt k => some (._DEF rt k)
    | _ => none
  | _ => none

omit hMR in
/-- Defined types with at most one supertype, which is a defined type when unrolled. -/
def NiceD (d : deftype) : Prop :=
  ∃ f tus ct, rolled d = some (.SUB f tus ct) ∧ (tus = [] ∨ ∃ tu, tus = [tu] ∧ (superS d).isSome)

omit hMR in
inductive ChainS : deftype → deftype → Prop where
  | refl (d : deftype) : ChainS d d
  | step {d e d' : deftype} : superS d = some e → ChainS e d' → ChainS d d'

omit hMR in
/-- Defined types whose supertype chains keep the kind. -/
def GoodD (d : deftype) : Prop := ∀ e, ChainS d e → NiceD e ∧ ∀ e', superS e = some e' → kindS e' = kindS e

omit hMR in
theorem chainS_trans {a b c : deftype} (h1 : ChainS a b) (h2 : ChainS b c) : ChainS a c := by
  induction h1 with
  | refl => exact h2
  | step s _ ih => exact .step s (ih h2)

omit hMR in
theorem goodD_super {d e : deftype} (h : GoodD d) (s : superS d = some e) : GoodD e :=
  fun x c => h x (.step s c)

omit hMR in
theorem goodD_chain {d e : deftype} (h : GoodD d) (c : ChainS d e) : GoodD e :=
  fun x c' => h x (chainS_trans c c')

omit hMR in
theorem kindS_chain {d e : deftype} (h : GoodD d) (c : ChainS d e) : kindS e = kindS d := by
  induction c with
  | refl => rfl
  | @step d e d' s _ ih => rw [ih (goodD_super h s), (h d (.refl _)).2 e s]

/-! ### Valid defined types are good -/

omit hMR in
theorem rec_ok_get : ∀ {C : context} {l : List subtype} {j : Nat}, Rectype_ok2 C (.REC (.mk_list l)) (.OK j) →
    ∀ k (hk : k < l.length), Subtype_ok2 C l[k] (.OK (j + k))
  | _, [], _, _, k, hk => by simp at hk
  | _, s :: l, j, h, k, hk => by
    cases h with
    | cons _ _ _ _ hs hr _ _ _ =>
      cases k with
      | zero => exact hs
      | succ k =>
        have := rec_ok_get hr k (by simpa using hk)
        rw [show j + 1 + k = j + (k + 1) by omega] at this
        exact this

omit hMR in
theorem comptype_sub_ckind {C ct ct'} (h : Comptype_sub C ct ct') : ckind ct = ckind ct' := by cases h <;> rfl

omit hMR in
/-- The supertype of a valid subtype: unrolled, it is not final and has the same kind. -/
theorem subtype_ok_super {C f tus ct i} (h : Subtype_ok2 C (.SUB f tus ct) (.OK i)) :
    tus = [] ∨ ∃ tu, tus = [tu] ∧ Typeuse_ok C tu ∧ before tu i = true ∧
      ∃ tus'' ct'', fun_unrollht_ C (heaptype_typeuse tu) (.SUB none tus'' ct'') ∧ ckind ct'' = ckind ct := by
  cases h with
  | mk_Subtype_ok2 _ _ _ _ cs uss _ vs F2 hl hok hbf l1 l2 F3 _ hsub _ _ _ _ l3 l4 l5 _ =>
    match tus, vs, cs, uss, hl, l1, l2, l3 with
    | [], _, _, _, _, _, _, _ => exact .inl rfl
    | [tu], [v], [c], [u], _, _, _, _ =>
      refine .inr ⟨tu, rfl, hok tu (by simp), hbf tu (by simp), u, c, ?_, ?_⟩
      · have hv : v = .SUB none u c := F3 v c u (by simp)
        rw [← hv]; exact F2 v tu (by simp)
      · exact (comptype_sub_ckind (hsub c (by simp))).symm
    | _ :: _ :: _, _, _, _, hl, _, _, _ => simp at hl
    | [_], [], _, _, _, _, _, l3 | [_], _ :: _ :: _, _, _, _, _, _, l3 => simp_all
    | [_], [_], [], _, _, l1, _, _ | [_], [_], _ :: _ :: _, _, _, l1, _, _ => simp_all
    | [_], [_], [_], [], _, _, l2, _ | [_], [_], [_], _ :: _ :: _, _, _, l2, _ => simp_all

omit hMR in
theorem ok_rolled {C d} (h : Deftype_ok C d) : ∃ f tus ct, rolled d = some (.SUB f tus ct) := by
  cases h with
  | mk_Deftype_ok _ rt i n sts _ hrt hi _ _ _ _ hn =>
    subst hrt hn
    simp only [rolled, rtSubs, List.getElem?_eq_getElem hi]
    cases sts[i] with
    | SUB f tus ct => exact ⟨f, tus, ct, rfl⟩

omit hMR in
theorem app_types (X C : context) : (X ++ C).TYPES = X.TYPES ++ C.TYPES := rfl

omit hMR in
theorem app_recs (X C : context) : (X ++ C).RECS = X.RECS ++ C.RECS := rfl

omit hMR in
theorem unrollht_rec {C k s} (h : fun_unrollht_ C (.REC k) s) : s = C.RECS[k]! := by cases h; rfl

omit hMR in
theorem unrollht_def {C rt k s} (h : fun_unrollht_ C (._DEF rt k) s) : fun_unrolldt (._DEF rt k) s := by
  cases h; assumption

omit hMR in
theorem kindS_of {d f tus ct} (h : rolled d = some (.SUB f tus ct)) : kindS d = ckind ct := by simp [kindS, h]

/-- One step up from a valid defined type: the supertype has the same kind and is valid. -/
theorem ok_step {C : context} {d : deftype} (h : Deftype_ok C d) (hT : C.TYPES = []) :
    NiceD d ∧ ∀ e, superS d = some e →
      kindS e = kindS d ∧ ∃ C' : context, C'.TYPES = [] ∧ Deftype_ok C' e := by
  cases h with
  | mk_Deftype_ok _ rt i n sts hR hrt hi wfC _ wfC' wfR hn =>
    subst hrt hn
    have hs := rec_ok_get hR i hi
    simp only [Nat.zero_add] at hs
    have hr : rolled (._DEF (.REC (.mk_list sts)) i) = some sts[i] := by
      simp [rolled, rtSubs, List.getElem?_eq_getElem hi]
    revert hs hr
    cases sts[i] with
    | SUB f tus ct =>
    intro hs hr
    rw [kindS_of hr]
    rcases subtype_ok_super hs with h0 | ⟨tu, rfl, hok, hbf, tus'', ct'', hun, hck⟩
    · subst h0
      refine ⟨⟨f, [], ct, hr, .inl rfl⟩, fun e he => ?_⟩
      simp [superS, hr] at he
    · cases hok with
      | typeidx _ x _ hx _ _ _ => rw [app_types] at hx; simp [hT] at hx
      | rec_ _ k _ _ _ _ _ =>
        simp [before] at hbf
        have hkn : k < sts.length := Nat.lt_trans hbf hi
        have hsup : superS (._DEF (.REC (.mk_list sts)) i) = some (._DEF (.REC (.mk_list sts)) k) := by
          simp [superS, hr, rtOf, unUse, rtSubs, hkn]
        refine ⟨⟨f, [.REC k], ct, hr, .inr ⟨_, rfl, by simp [hsup]⟩⟩, fun e he => ?_⟩
        rw [hsup] at he; cases he
        refine ⟨?_, C, hT, .mk_Deftype_ok _ _ k _ sts hR rfl hkn wfC (.deftype_case_0 _ _ wfR) wfC' wfR rfl⟩
        have e1 := unrollht_rec hun
        rw [app_recs, List.getElem!_eq_getElem?_getD, List.getElem?_append_left hkn,
          List.getElem?_eq_getElem hkn] at e1
        simp only [Option.getD_some] at e1
        have hr' : rolled (._DEF (.REC (.mk_list sts)) k) = some (.SUB none tus'' ct'') := by
          simp [rolled, rtSubs, List.getElem?_eq_getElem hkn, e1]
        rw [kindS_of hr', hck]
      | deftype _ d hd _ _ =>
        cases d with
        | _DEF rt' k =>
        have hsup : superS (._DEF (.REC (.mk_list sts)) i) = some (._DEF rt' k) := by
          simp [superS, hr, unUse, typeuse_deftype]
        refine ⟨⟨f, _, ct, hr, .inr ⟨_, rfl, by simp [hsup]⟩⟩, fun e he => ?_⟩
        rw [hsup] at he; cases he
        refine ⟨?_, _, by rw [app_types, hT]; rfl, hd⟩
        obtain ⟨f', tus', ct', hr'⟩ := ok_rolled hd
        obtain ⟨c, hc, hc'⟩ := unroll_shape hMR (unrollht_def hun) hr'
        cases hc
        rw [kindS_of hr', ← hc', hck]

theorem ok_good {C : context} {d : deftype} (h : Deftype_ok C d) (hT : C.TYPES = []) : GoodD d := by
  intro e c
  induction c generalizing C with
  | refl d => exact ⟨(ok_step hMR h hT).1, fun e' s => ((ok_step hMR h hT).2 e' s).1⟩
  | step s _ ih =>
    obtain ⟨_, C', hT', h'⟩ := (ok_step hMR h hT).2 _ s
    exact ih h' hT'

omit hMR in
/-- Heap types for which `Heaptype_sub` is algorithmic: abstract ones, bottom and good defined types. -/
def GoodH : heaptype → Prop
  | ._DEF rt i => GoodD (._DEF rt i)
  | .REC _ | ._IDX _ => False
  | _ => True

theorem ok_goodH {h : heaptype} (hk : Heaptype_ok C0 h) : GoodH h := by
  cases hk with
  | abs _ x _ => cases x <;> trivial
  | typeuse _ tu ht _ _ =>
    cases ht with
    | typeidx _ _ _ hx _ _ _ => simp [C0] at hx
    | rec_ _ _ _ hx _ _ _ => simp [C0] at hx
    | deftype _ d hd _ _ =>
      cases d with
      | _DEF rt i => exact ok_good hMR hd rfl
  | bot => trivial

/-! ### Algorithmic heap subtyping on SpecTec types -/

omit hMR in
def habs : heaptype → Option Abs
  | .ANY => some .any | .EQ => some .eq | .I31 => some .i31 | .STRUCT => some .struct | .ARRAY => some .array
  | .NONE => some .none | .FUNC => some .func | .NOFUNC => some .nofunc | .EXTERN => some .extern
  | .NOEXTERN => some .noextern | .EXN => some .exn | .NOEXN => some .noexn
  | _ => none

omit hMR in
def SAlgo : heaptype → heaptype → Prop
  | .BOT, _ => True
  | ._DEF rt i, ._DEF rt' i' => ChainS (._DEF rt i) (._DEF rt' i')
  | ._DEF rt i, b => ∃ y, habs b = some y ∧ defAbs (kindS (._DEF rt i)) y = true
  | a, ._DEF rt i => ∃ x, habs a = some x ∧ absDef (kindS (._DEF rt i)) x = true
  | a, b => ∃ x y, habs a = some x ∧ habs b = some y ∧ absSub x y = true

omit hMR in
theorem habs_trAbs (x : Abs) : habs (trAbs x) = some x := by cases x <;> rfl

omit hMR in
/-- The forms of good heap types. -/
inductive View : heaptype → Prop where
  | abs (x : Abs) : View (trAbs x)
  | bot : View .BOT
  | defd (rt : rectype) (i : Nat) : View (._DEF rt i)

omit hMR in
theorem view_of_good {h : heaptype} (hg : GoodH h) : View h := by
  cases h with
  | ANY => exact .abs .any | EQ => exact .abs .eq | I31 => exact .abs .i31 | STRUCT => exact .abs .struct
  | ARRAY => exact .abs .array | NONE => exact .abs .none | FUNC => exact .abs .func
  | NOFUNC => exact .abs .nofunc | EXN => exact .abs .exn | NOEXN => exact .abs .noexn
  | EXTERN => exact .abs .extern | NOEXTERN => exact .abs .noextern | BOT => exact .bot
  | _DEF rt i => exact .defd rt i
  | _IDX _ | REC _ => exact hg.elim

omit hMR in
theorem salgo_abs_abs (x y : Abs) : SAlgo (trAbs x) (trAbs y) ↔ absSub x y = true := by
  cases x <;> cases y <;> simp [SAlgo, trAbs, habs]

omit hMR in
theorem salgo_abs_def (x : Abs) (rt i) :
    SAlgo (trAbs x) (._DEF rt i) ↔ absDef (kindS (._DEF rt i)) x = true := by
  cases x <;> simp [SAlgo, trAbs, habs]

omit hMR in
theorem salgo_def_abs (rt i) (y : Abs) :
    SAlgo (._DEF rt i) (trAbs y) ↔ defAbs (kindS (._DEF rt i)) y = true := by
  cases y <;> simp [SAlgo, trAbs, habs]

omit hMR in
theorem salgo_abs_bot (x : Abs) : ¬ SAlgo (trAbs x) .BOT := by cases x <;> simp [SAlgo, trAbs, habs]

omit hMR in
theorem salgo_def_bot (rt i) : ¬ SAlgo (._DEF rt i) .BOT := by simp [SAlgo, habs]

omit hMR in
theorem salgo_trans {a b c : heaptype} (ha : GoodH a) (hb : GoodH b) (h1 : SAlgo a b) (h2 : SAlgo b c)
    (hc : GoodH c) : SAlgo a c := by
  cases view_of_good ha with
  | bot => trivial
  | abs x =>
    cases view_of_good hb with
    | bot => exact absurd h1 (salgo_abs_bot x)
    | abs y =>
      rw [salgo_abs_abs] at h1
      cases view_of_good hc with
      | bot => exact absurd h2 (salgo_abs_bot y)
      | abs z => rw [salgo_abs_abs] at h2 ⊢; exact absSub_trans x y z h1 h2
      | defd rt i =>
        rw [salgo_abs_def] at h2 ⊢
        cases x <;> cases y <;> simp [absSub, absDef] at h1 h2 ⊢ <;> assumption
    | defd rt i =>
      rw [salgo_abs_def] at h1
      cases view_of_good hc with
      | bot => exact absurd h2 (salgo_def_bot rt i)
      | abs z =>
        rw [salgo_def_abs] at h2; rw [salgo_abs_abs]
        cases x <;> cases z <;> simp [absSub, absDef, defAbs] at h1 h2 ⊢ <;> omega
      | defd rt' i' =>
        rw [salgo_abs_def, kindS_chain hb h2]; exact h1
  | defd rt i =>
    cases view_of_good hb with
    | bot => exact absurd h1 (salgo_def_bot rt i)
    | abs y =>
      rw [salgo_def_abs] at h1
      cases view_of_good hc with
      | bot => exact absurd h2 (salgo_abs_bot y)
      | abs z =>
        rw [salgo_abs_abs] at h2; rw [salgo_def_abs]
        cases y <;> cases z <;> simp [absSub, defAbs] at h1 h2 ⊢ <;> simp [h1]
      | defd rt' i' =>
        rw [salgo_abs_def] at h2
        cases y <;> simp [absDef, defAbs] at h1 h2 <;> omega
    | defd rt' i' =>
      cases view_of_good hc with
      | bot => exact absurd h2 (salgo_def_bot rt' i')
      | abs z =>
        rw [salgo_def_abs] at h2 ⊢
        have h1' : ChainS (._DEF rt i) (._DEF rt' i') := h1
        rw [← kindS_chain ha h1']; exact h2
      | defd rt'' i'' => exact chainS_trans (b := ._DEF rt' i') h1 h2

theorem expand_kind {d ct} (h : Expand d ct) (hg : GoodD d) : ckind ct = kindS d := by
  match h with
  | .mk_Expand _ _ f tus s hun hs _ _ _ =>
    subst hs
    obtain ⟨f', tus0, ct0, hr, _⟩ := (hg d (.refl _)).1
    cases d with
    | _DEF rt i =>
      obtain ⟨c, hc, hc'⟩ := unroll_shape hMR hun hr
      cases hc
      rw [kindS_of hr, hc']

omit hMR in
theorem clos_nil {l v} (h : fun_clos_deftypes l v) (hl : l = []) : v = [] := by
  cases h with
  | fun_clos_deftypes_case_0 => rfl
  | fun_clos_deftypes_case_1 => simp at hl

/-- Closing in the empty context changes nothing. -/
theorem clos_C0 {d d'} (h : fun_clos_deftype C0 d d') : d' = d := by
  match h with
  | .fun_clos_deftype_case_0 _ _ dl _ v1 _ _ hc hs0 he _ _ =>
    have hv : v1 = [] := clos_nil hc rfl
    subst hv he
    match hs0 with
    | .fun_subst_all_deftype_case_0 _ n _ _ _ _ _ h0 _ _ hn =>
      simp at hn; subst hn
      exact id_deftype hMR h0 rfl rfl

omit hMR in
theorem salgo_refl {h : heaptype} (g : GoodH h) : SAlgo h h := by
  cases view_of_good g with
  | abs x => rw [salgo_abs_abs]; cases x <;> rfl
  | bot => trivial
  | defd rt i => exact ChainS.refl _

omit hMR in
/-- The bottom rules (`none`, `nofunc`, `noexn`, `noextern`) are algorithmic. -/
theorem salgo_bottom {h : heaptype} (g : GoodH h) (ne : h ≠ .BOT) (b t : Abs)
    (hbt : ∀ x : Abs, absSub x t = true → absSub b x = true)
    (hk : ∀ k, defAbs k t = true → absDef k b = true) (s : SAlgo h (trAbs t)) : SAlgo (trAbs b) h := by
  cases view_of_good g with
  | abs x => rw [salgo_abs_abs] at s ⊢; exact hbt x s
  | bot => exact absurd rfl ne
  | defd rt i => rw [salgo_def_abs] at s; rw [salgo_abs_def]; exact hk _ s

omit hMR in
theorem superS_use {d f tu ct e} (hr : rolled d = some (.SUB f [tu] ct)) (hs : superS d = some e) :
    heaptype_typeuse (unUse (rtOf d) tu) = heaptype_deftype e := by
  simp only [superS, hr] at hs
  revert hs
  cases unUse (rtOf d) tu with
  | _DEF rt k => intro hs; cases hs; rfl
  | _ => intro hs; cases hs

omit hMR in
theorem goodH_def (d : deftype) : GoodH (heaptype_deftype d) ↔ GoodD d := by cases d; rfl

omit hMR in
theorem salgo_defs (d1 d2 : deftype) : SAlgo (heaptype_deftype d1) (heaptype_deftype d2) ↔ ChainS d1 d2 := by
  cases d1; cases d2; rfl

mutual
theorem sub_salgo : ∀ {C h1 h2}, Heaptype_sub C h1 h2 → C = C0 → GoodH h1 → GoodH h2 → SAlgo h1 h2
  | _, _, _, .refl _ _ _ _, _, g1, _ => salgo_refl g1
  | _, _, _, .trans _ _ _ _ ok s1 s2 _ _ _ _, hC, g1, g2 => by
    have gm := ok_goodH hMR (hC ▸ ok)
    exact salgo_trans g1 gm (sub_salgo s1 hC g1 gm) (sub_salgo s2 hC gm g2) g2
  | _, _, _, .eq_any .., _, _, _ => (salgo_abs_abs .eq .any).mpr rfl
  | _, _, _, .i31_eq .., _, _, _ => (salgo_abs_abs .i31 .eq).mpr rfl
  | _, _, _, .struct_eq .., _, _, _ => (salgo_abs_abs .struct .eq).mpr rfl
  | _, _, _, .array_eq .., _, _, _ => (salgo_abs_abs .array .eq).mpr rfl
  | _, _, _, .struct _ d _ he _ _ _ _, _, g1, _ => by
    cases d with
    | _DEF rt i =>
      have := expand_kind hMR he g1
      exact (salgo_def_abs rt i .struct).mpr (by rw [← this]; rfl)
  | _, _, _, .array _ d _ he _ _ _ _, _, g1, _ => by
    cases d with
    | _DEF rt i =>
      have := expand_kind hMR he g1
      exact (salgo_def_abs rt i .array).mpr (by rw [← this]; rfl)
  | _, _, _, .func _ d _ _ he _ _ _ _, _, g1, _ => by
    cases d with
    | _DEF rt i =>
      have := expand_kind hMR he g1
      exact (salgo_def_abs rt i .func).mpr (by rw [← this]; rfl)
  | _, _, _, .def _ d1 d2 ds _ _ _, hC, g1, g2 =>
    (salgo_defs d1 d2).mpr (dsub_chain ds hC ((goodH_def d1).mp g1) ((goodH_def d2).mp g2))
  | _, _, _, .typeidx_l _ _ _ hx _ _ _ _, hC, _, _ => by subst hC; simp [C0] at hx
  | _, _, _, .typeidx_r _ _ _ hx _ _ _ _, hC, _, _ => by subst hC; simp [C0] at hx
  | _, _, _, .rec_struct _ _ _ _ hx _ _ _ _ _, hC, _, _ => by subst hC; simp [C0] at hx
  | _, _, _, .rec_array _ _ _ _ hx _ _ _ _ _, hC, _, _ => by subst hC; simp [C0] at hx
  | _, _, _, .rec_func _ _ _ _ _ hx _ _ _ _ _, hC, _, _ => by subst hC; simp [C0] at hx
  | _, _, _, .rec_sub _ _ _ _ _ _ _ hx _ _ _ _, hC, _, _ => by subst hC; simp [C0] at hx
  | _, _, _, .none _ _ s ne _ _ _ _ _, hC, _, g2 =>
    salgo_bottom g2 ne .none .any (by intro x; cases x <;> decide) (by intro k; simp [defAbs, absDef])
      (sub_salgo s hC g2 trivial)
  | _, _, _, .nofunc _ _ s ne _ _ _ _ _, hC, _, g2 =>
    salgo_bottom g2 ne .nofunc .func (by intro x; cases x <;> decide) (by intro k; simp [defAbs, absDef])
      (sub_salgo s hC g2 trivial)
  | _, _, _, .noexn _ _ s ne _ _ _ _ _, hC, _, g2 =>
    salgo_bottom g2 ne .noexn .exn (by intro x; cases x <;> decide) (by intro k; simp [defAbs])
      (sub_salgo s hC g2 trivial)
  | _, _, _, .noextern _ _ s ne _ _ _ _ _, hC, _, g2 =>
    salgo_bottom g2 ne .noextern .extern (by intro x; cases x <;> decide) (by intro k; simp [defAbs])
      (sub_salgo s hC g2 trivial)
  | _, _, _, .bot .., _, _, _ => trivial

theorem dsub_chain : ∀ {C d1 d2}, Deftype_sub C d1 d2 → C = C0 → GoodD d1 → GoodD d2 → ChainS d1 d2
  | _, _, _, .refl _ _ _ _ _ c2 c1 e _ _ _ _ _, hC, _, _ => by
    subst hC
    rw [clos_C0 hMR c1, clos_C0 hMR c2] at e
    rw [e]; exact .refl _
  | _, _, _, .super _ d1 d2 f tus ct i v0 hun hv hi hs _ _ _ _ _, hC, g1, g2 => by
    have key : ∃ e, superS d1 = some e ∧ heaptype_typeuse tus[i]! = heaptype_deftype e := by
      obtain ⟨f0, tus0, ct0, hr, hn⟩ := (g1 d1 (.refl _)).1
      cases d1 with
      | _DEF rt k1 =>
      obtain ⟨c', hc, _⟩ := unroll_shape hMR hun hr
      rw [hc] at hv
      obtain ⟨-, htus, -⟩ := subtype.SUB.inj hv
      subst htus
      rcases hn with h0 | ⟨tu, rfl, hsome⟩
      · subst h0; simp at hi
      · obtain ⟨e, he⟩ := Option.isSome_iff_exists.mp hsome
        have hi0 : i = 0 := by simpa using hi
        subst hi0
        exact ⟨e, he, superS_use hr he⟩
    obtain ⟨e, he, hu⟩ := key
    have r := sub_salgo hs hC (by rw [hu]; exact (goodH_def e).mpr (goodD_super g1 he)) ((goodH_def d2).mpr g2)
    rw [hu, salgo_defs] at r
    exact .step he r
end

/-! ### Translated defined types -/

variable {A : Nat → storagetype}

omit hMR in
def tD (A : Nat → storagetype) (d : DT) : deftype := ._DEF (trG A d.1) d.2

omit hMR in
def ShapeH (A : Nat → storagetype) : CH → Prop
  | .abs _ => True
  | .defd d => ShapeL A d.1

omit hMR in
theorem shapeL_get {g : List CT} (h : ShapeL A g) {i : Nat} (hi : i < g.length) : ShapeS A g[i] := by
  induction g generalizing i with
  | nil => simp at hi
  | cons c g ih =>
    simp only [ShapeL] at h
    cases i with
    | zero => exact h.1
    | succ i => exact ih h.2 (by simpa using hi)

omit hMR in
theorem rolled_tD {d : DT} (h : OkD d) {c : CT} (hc : d.1[d.2]? = some c) : rolled (tD A d) = some (trS A none c) := by
  simp [rolled, tD, trG, rtSubs, trSs_eq, hc]

omit hMR in
theorem kindS_tD {d : DT} (h : OkD d) (hs : ShapeL A d.1) : kindS (tD A d) = kindOf d := by
  obtain ⟨fin, kind, super, counts, fields, hc, -⟩ := okD_sub h
  have hsh := shapeL_get hs h.2
  rw [List.getElem?_eq_getElem h.2] at hc
  have hc' := Option.some.inj hc
  rw [hc'] at hsh
  rw [kindS_of (by rw [rolled_tD h (List.getElem?_eq_getElem h.2), hc', trS.eq_1]), kindOf,
    List.getElem?_eq_getElem h.2, hc']
  simp only [ShapeS] at hsh
  rcases hsh.2.2 with ⟨hk, -⟩ | ⟨hk, -⟩ | ⟨hk, -⟩ <;> subst hk <;> rfl

omit hMR in
theorem superS_tD {d : DT} (h : OkD d) : superS (tD A d) = (superOf d).map (tD A) := by
  obtain ⟨fin, kind, super, counts, fields, hc, hs, -⟩ := okD_sub h
  have hr := rolled_tD (A := A) h hc
  rw [trS.eq_1] at hr
  simp only [superS, hr, superOf, hc]
  cases super with
  | none => rfl
  | some s =>
    cases s with
    | inner k n =>
      simp only [OkSuper] at hs
      simp [trSup, trU, recUse, unUse, rtOf, tD, trG, rtSubs, trSs_length, hs]
    | dt g k n => simp [trSup, trU, unUse, tD, trG]
    | atom _ => simp [OkSuper] at hs
    | sub _ _ _ _ _ => simp [OkSuper] at hs

omit hMR in
theorem super_ok {d e : DT} (h : OkD d) (hsh : ShapeL A d.1) (s : superOf d = some e) : OkD e ∧ ShapeL A e.1 := by
  obtain ⟨fin, kind, super, counts, fields, hc, hs, -⟩ := okD_sub h
  have hsh' := shapeL_get hsh h.2
  rw [List.getElem?_eq_getElem h.2] at hc
  rw [Option.some.inj hc] at hsh'
  simp only [ShapeS] at hsh'
  simp only [superOf, List.getElem?_eq_getElem h.2, Option.some.inj hc] at s
  cases super with
  | none => simp at s
  | some t =>
    cases t with
    | inner k n => simp at s; subst s; exact ⟨⟨h.1, hs⟩, hsh⟩
    | dt g k n =>
      simp at s; subst s
      simp only [OkSuper] at hs; simp only [ShapeSup] at hsh'
      exact ⟨⟨⟨hs.1, hs.2.1⟩, hs.2.2⟩, hsh'.1.2⟩
    | atom _ => simp at s
    | sub _ _ _ _ _ => simp at s

omit hMR in
theorem chainS_tD {d : DT} {e : deftype} (c : ChainS (tD A d) e) (h : OkD d) (hs : ShapeL A d.1) :
    ∃ d', Chain d d' ∧ e = tD A d' ∧ OkD d' ∧ ShapeL A d'.1 := by
  generalize hx : tD A d = x at c
  induction c generalizing d with
  | refl => exact ⟨d, .refl d, hx.symm, h, hs⟩
  | step s _ ih =>
    subst hx
    rw [superS_tD h] at s
    obtain ⟨d1, hd1, rfl⟩ := Option.map_eq_some_iff.mp s
    obtain ⟨ok1, sh1⟩ := super_ok h hs hd1
    obtain ⟨d', c', he, ok', sh'⟩ := ih ok1 sh1 rfl
    exact ⟨d', .step hd1 c', he, ok', sh'⟩

omit hMR in
theorem goodD_tD {d : DT} (h : OkD d) (hs : ShapeL A d.1) (hk : KindsOK d) : GoodD (tD A d) := by
  intro e c
  obtain ⟨d', cd, rfl, ok', sh'⟩ := chainS_tD c h hs
  refine ⟨?_, fun e' s => ?_⟩
  · obtain ⟨fin, kind, super, counts, fields, hc, hsup, -⟩ := okD_sub ok'
    have hr := rolled_tD (A := A) ok' hc
    rw [trS.eq_1] at hr
    refine ⟨_, _, _, hr, ?_⟩
    cases super with
    | none => exact .inl rfl
    | some t =>
      refine .inr ⟨_, rfl, ?_⟩
      rw [superS_tD ok']
      cases t with
      | inner k n => simp [superOf, hc]
      | dt g k n => simp [superOf, hc]
      | atom _ => simp [OkSuper] at hsup
      | sub _ _ _ _ _ => simp [OkSuper] at hsup
  · rw [superS_tD ok'] at s
    obtain ⟨d'', hd'', rfl⟩ := Option.map_eq_some_iff.mp s
    obtain ⟨ok'', sh''⟩ := super_ok ok' sh' hd''
    rw [kindS_tD ok'' sh'', kindS_tD ok' sh', hk d' d'' cd hd'']

omit hMR in
theorem tD_inj {d d' : DT} (hs : ShapeL A d.1) (hs' : ShapeL A d'.1) (h : tD A d = tD A d') :
    d = d' := by
  obtain ⟨g, i⟩ := d
  obtain ⟨g', i'⟩ := d'
  simp only [tD, trG, deftype._DEF.injEq, rectype.REC.injEq, list.mk_list.injEq] at h
  rw [inj_trSs (l1 := g) (l2 := g') hs hs' h.1, h.2]

omit hMR in
/-- The algorithmic subtyping on SpecTec types gives the algorithmic one of closed forms. -/
theorem algo_complete {a b : CH} (ha : OkH a) (hb : OkH b) (sa : ShapeH A a) (sb : ShapeH A b)
    (h : SAlgo (trH A a) (trH A b)) : Algo a b := by
  cases a with
  | abs x => cases b with
    | abs y => exact (salgo_abs_abs x y).mp h
    | defd d =>
      have := (salgo_abs_def x (trG A d.1) d.2).mp h
      show absDef (kindOf d) x = true
      rw [← kindS_tD hb sb]; exact this
  | defd d => cases b with
    | abs y =>
      have := (salgo_def_abs (trG A d.1) d.2 y).mp h
      show defAbs (kindOf d) y = true
      rw [← kindS_tD ha sa]; exact this
    | defd d' =>
      have c : ChainS (tD A d) (tD A d') := h
      obtain ⟨d'', cd, he, -, sh''⟩ := chainS_tD c ha sa
      have e := tD_inj sb sh'' he
      subst e
      exact cd

/-- `Heaptype_sub` of the spec implies the heap subtyping of WahProofs/Subtype.lean for closed types. -/
theorem hsub_complete {a b : CH} (ha : OkH a) (hb : OkH b) (sa : ShapeH A a) (sb : ShapeH A b)
    (va : ValidH a) (vb : ValidH b) (h : Heaptype_sub C0 (trH A a) (trH A b)) : HSub a b := by
  have good : ∀ {x : CH}, OkH x → ShapeH A x → ValidH x → GoodH (trH A x) := by
    intro x hx sx vx
    cases x with
    | abs y => cases y <;> trivial
    | defd d => exact goodD_tD hx sx vx
  exact hsub_of_algo va vb (algo_complete ha hb sa sb (sub_salgo hMR h rfl (good ha sa va) (good hb sb vb)))

/-- Both directions. -/
theorem hsub_iff_spec (hA : AtomsOK A) {a b : CH} (ha : OkH a) (hb : OkH b) (sa : ShapeH A a) (sb : ShapeH A b)
    (va : ValidH a) (vb : ValidH b) : HSub a b ↔ Heaptype_sub C0 (trH A a) (trH A b) :=
  ⟨hsub_sound hA wfC0 hMR ha hb va, hsub_complete hMR ha hb sa sb va vb⟩

/-! ### The atoms of WAH

Atoms are `wah_type_t` below 0 as unsigned 32-bit values (Oracle.lean). Numbers, vectors, packed types and
abstract references translate to their types, and anything else to bottom, so that the hypotheses of both
directions hold together. -/

omit hMR in
def wahAtom (a : Nat) : storagetype :=
  let t : Int := a - 2 ^ 32
  if t = -2 then .I32 else if t = -4 then .I64 else if t = -6 then .F32 else if t = -8 then .F64
  else if t = -10 then .V128 else if t = -16 then .I8 else if t = -18 then .I16
  else match absOfCode (t - t % 2) with
    | some h => .REF (nullOpt (t % 2 == 1)) (trAbs h)
    | none => .BOT

omit hMR in
theorem subst_abs_heap (h : Abs) (tvs tus) : fun_subst_heaptype (trAbs h) tvs tus (trAbs h) :=
  .fun_subst_heaptype_case_3 _ _ _ (by cases h <;> intro hb <;> cases hb)

omit hMR in
theorem wahAtom_ok : AtomsOK wahAtom where
  wf a := by
    unfold wahAtom; dsimp only
    repeat' split
    all_goals first | exact .storagetype_case_5 _ _ (wf_trAbs _) | constructor
  subst a tvs tus := by
    unfold wahAtom; dsimp only
    repeat' split
    all_goals first
      | exact .fun_subst_storagetype_case_6 _ _ _ (.fun_subst_valtype_case_0 _ _)
      | exact .fun_subst_storagetype_case_5 _ _ _ (.fun_subst_valtype_case_1 _ _)
      | exact .fun_subst_storagetype_case_4 _ _ _ (.fun_subst_valtype_case_2 _ _)
      | exact .fun_subst_storagetype_case_3 _ _ _ (.fun_subst_valtype_case_3 _ _)
      | exact .fun_subst_storagetype_case_2 _ _ _ (.fun_subst_valtype_case_4 _ _)
      | exact .fun_subst_storagetype_case_7 _ _ | exact .fun_subst_storagetype_case_8 _ _
      | exact subst_ref (subst_abs_heap _ _ _)
      | exact .fun_subst_storagetype_case_0 _ _ _ (.fun_subst_valtype_case_6 _ _)

omit hMR in
/-- The `wah_type_t` of a translated atom. -/
def encAtom : storagetype → Int
  | .I32 => -2 | .I64 => -4 | .F32 => -6 | .F64 => -8 | .V128 => -10 | .I8 => -16 | .I16 => -18
  | .REF n ht => match habs ht with
    | some h => absCode h + if n = some .NULL then 1 else 0
    | none => 0
  | .BOT => 0

omit hMR in
theorem absOfCode_some {t : Int} {h : Abs} (e : absOfCode t = some h) : absCode h = t := by
  have := List.find?_some e
  simpa using this

omit hMR in
theorem enc_wahAtom (a : Nat) (hb : wahAtom a ≠ .BOT) : encAtom (wahAtom a) = (a : Int) - 2 ^ 32 := by
  revert hb
  unfold wahAtom; dsimp only
  repeat' split
  all_goals first | (intro; simp_all [encAtom]; done) | skip
  · rename_i h0 h e
    intro
    simp only [encAtom, habs_trAbs, absOfCode_some e, nullOpt]
    generalize (↑a - 2 ^ 32 : Int) = x
    rcases Int.emod_two_eq x with h2 | h2 <;> simp [h2] <;> omega

omit hMR in
/-- Every atom that does not go to bottom is an atom of well-shaped closed forms. -/
theorem shapeAtom_wah (a : Nat) (hb : wahAtom a ≠ .BOT) : ShapeAtom wahAtom a := by
  refine ⟨fun b e => ?_, fun n i h => ?_, fun n rt i h => ?_, hb⟩
  · have h1 := enc_wahAtom a hb
    have h2 := enc_wahAtom b (by rw [e]; exact hb)
    rw [e] at h2; omega
  all_goals
    revert h; unfold wahAtom; dsimp only
    repeat' split
    all_goals first | (intro h; cases h; done) | (intro h; simp at h; done) | skip
    · rename_i hab _
      intro h; simp only [storagetype.REF.injEq] at h; revert h; cases hab_ : ‹Abs› <;> simp [trAbs]
end Wah.SpecTec
