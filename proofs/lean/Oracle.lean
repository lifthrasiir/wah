/- Checks canonical maps, subtyping and equivalence across modules dumped by dump_types.c against `canonicalMap` of
WahProofs/Types.lean, `wahSub` of WahProofs/Subtype.lean, `typeRefEq` of WahProofs/Cross.lean and `crossSub` of WahProofs/CrossSub.lean
(`make lean-diff`). With a constant hash, `HashOK` holds trivially, so the model
maps types to the least equivalent type (`canonicalMap_least`), and so must the C code. -/
import WahProofs.Subtype
import WahProofs.CrossSub
open Wah.Types Wah.Subtype Wah.Cross Wah.CrossSub

/-- wah_type_t as in proofs/types.c: negative values are atoms, others are references of index t / 2. -/
def decodeVT (t : Int) : VT := if t < 0 then .atom (t + 2 ^ 32).toNat else .ref (t.toNat / 2) (t.toNat % 2 == 1)

structure Module where
  n : Nat
  starts : Array Nat
  sizes : Array Nat
  tds : Array TD
  canon : Array Nat
  sub : String

def Module.toM (md : Module) : M :=
  { n := md.n, d := fun i => md.tds.getD i default, start := fun i => md.starts.getD i 0,
    gsize := fun i => md.sizes.getD i 0 }

/-- `WF` as a check; a failure means that WAH accepted a module outside the model. -/
def wfCheck (m : M) : Bool :=
  (List.range m.n).all fun t =>
    m.start t ≤ t && t < m.start t + m.gsize t && m.start t + m.gsize t ≤ m.n &&
    ((List.range (m.gsize t)).all fun k => m.start (m.start t + k) == m.start t && m.gsize (m.start t + k) == m.gsize t) &&
    (m.d t).refs.all fun x => x < m.start t + m.gsize t

/-- `SubWF` beyond `WF`: supertypes are earlier types of the same kind. -/
def subCheck (m : M) : Bool :=
  (List.range m.n).all fun t => match (m.d t).super with
    | none => true
    | some s => s < t && (m.d s).kind == (m.d t).kind

def parseInts (ws : List String) : Option (List Int) := ws.mapM String.toInt?

def parseType (ws : List Int) : Option (Nat × Nat × TD) := do
  match ws with
  | s :: g :: kind :: fin :: sup :: c1 :: c2 :: nf :: rest =>
    let rec fields : Nat → List Int → Option (List (VT × Bool))
      | 0, [] => some []
      | k + 1, t :: mu :: rest => do return (decodeVT t, mu != 0) :: (← fields k rest)
      | _, _ => none
    let fs ← fields nf.toNat rest
    let super : Option Nat := if sup < 0 then none else some sup.toNat
    let td : TD := ⟨fin != 0, kind.toNat, super, (c1.toNat, c2.toNat), fs⟩
    some (s.toNat, g.toNat, td)
  | _ => none

/-- Equivalence of types of two modules: their numbers, whether they are the same module, and the results. -/
structure Pair where
  a : Nat
  b : Nat
  same : Bool
  eqs : String
  subs : String

partial def parseModules (lines : List String) (acc : Array Module) (pairs : Array Pair) :
    Except String (Array Module × Array Pair) :=
  match lines with
  | [] => .ok (acc, pairs)
  | l :: rest =>
    match l.splitOn " " with
    | ["X", a, b, same, eqs, subs] =>
      -- Numbered backwards from the last module
      parseModules rest acc (pairs.push ⟨acc.size - 1 - a.toNat!, acc.size - 1 - b.toNat!, same == "1", eqs, subs⟩)
    | ["M", n] =>
      let n := n.toNat!
      let tlines := rest.take n
      let cline := (rest.drop n).head?.getD ""
      let sline := (rest.drop (n + 1)).head?.getD ""
      let types := tlines.filterMap fun t => match t.splitOn " " with
        | "T" :: ws => (parseInts ws).bind parseType
        | _ => none
      match cline.splitOn " " with
      | "C" :: cs =>
        if types.length != n || cs.length != n || !sline.startsWith "S " || sline.length != n * n + 2 then
          .error s!"malformed module at: {l}" else
        let md : Module := ⟨n, (types.map (·.1)).toArray, (types.map (·.2.1)).toArray,
          (types.map (·.2.2)).toArray, (cs.map String.toNat!).toArray, (sline.drop 2).toString⟩
        parseModules (rest.drop (n + 2)) (acc.push md) pairs
      | _ => .error s!"missing canonical map after: {l}"
    | [""] => parseModules rest acc pairs
    | _ => .error s!"unexpected line: {l}"

def main : IO UInt32 := do
  let input ← (← IO.getStdin).readToEnd
  match parseModules (input.splitOn "\n") #[] #[] with
  | .error e => IO.eprintln e; return 2
  | .ok (mods, pairs) =>
    let mut bad := 0
    let mut types := 0
    for md in mods do
      let m := md.toM
      types := types + m.n
      if !wfCheck m || !subCheck m then
        bad := bad + 1; IO.eprintln s!"module outside the model (not WF), canonical map {md.canon}"; continue
      let c := (build m (fun _ => 0) m.n).get -- canonicalMap, computed once
      let model := (List.range m.n).map c
      if model != md.canon.toList then
        bad := bad + 1; IO.eprintln s!"canonical map {md.canon} differs from the model {model}"
      let subs := String.ofList <| (List.range m.n).flatMap fun i => (List.range m.n).map fun j =>
        if wahSub m c (2 * Int.ofNat i) (2 * Int.ofNat j) then '1' else '0'
      if subs != md.sub then
        bad := bad + 1; IO.eprintln s!"subtyping {md.sub} differs from the model {subs} (canonical map {md.canon})"
    let canons := mods.map fun md => (build md.toM (fun _ => 0) md.n).get
    let mut eqs := 0
    for p in pairs do
      match mods[p.a]?, mods[p.b]? with
      | some ma, some mb =>
        let c : Ctx := ⟨ma.toM, mb.toM, canons[p.a]!, canons[p.b]!, p.same⟩
        let model := String.ofList <| (List.range ma.n).flatMap fun i => (List.range mb.n).map fun j =>
          if typeRefEq c (.ref i false) (.ref j false) then '1' else '0'
        eqs := eqs + model.length
        if model != p.eqs then
          bad := bad + 1; IO.eprintln s!"equivalence of modules {p.a} and {p.b} {p.eqs} differs from the model {model}"
        let msub := String.ofList <| (List.range ma.n).flatMap fun i => (List.range mb.n).map fun j =>
          if crossSub c (2 * Int.ofNat i) (2 * Int.ofNat j) then '1' else '0'
        if msub != p.subs then
          bad := bad + 1; IO.eprintln s!"subtyping of modules {p.a} and {p.b} {p.subs} differs from the model {msub}"
      | _, _ => bad := bad + 1; IO.eprintln s!"unknown modules {p.a} and {p.b}"
    IO.println s!"{mods.size} modules with {types} types, {pairs.size} pairs with {eqs} type pairs, {bad} failures"
    return if bad == 0 then 0 else 1
