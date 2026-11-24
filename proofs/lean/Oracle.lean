/- Checks canonical maps dumped by dump_types.c against `canonicalMap` of WahProofs/Types.lean (`make lean-diff`).
With a constant hash, `HashOK` holds trivially, so the model maps types to the least equivalent type
(`canonicalMap_least`), and so must the C code. -/
import WahProofs.Types
open Wah.Types

/-- wah_type_t as in proofs/types.c: negative values are atoms, others are references of index t / 2. -/
def decodeVT (t : Int) : VT := if t < 0 then .atom (t + 2 ^ 32).toNat else .ref (t.toNat / 2) (t.toNat % 2 == 1)

structure Module where
  n : Nat
  starts : Array Nat
  sizes : Array Nat
  tds : Array TD
  canon : Array Nat

def Module.toM (md : Module) : M :=
  { n := md.n, d := fun i => md.tds.getD i default, start := fun i => md.starts.getD i 0,
    gsize := fun i => md.sizes.getD i 0 }

/-- `WF` as a check; a failure means that WAH accepted a module outside the model. -/
def wfCheck (m : M) : Bool :=
  (List.range m.n).all fun t =>
    m.start t ≤ t && t < m.start t + m.gsize t && m.start t + m.gsize t ≤ m.n &&
    ((List.range (m.gsize t)).all fun k => m.start (m.start t + k) == m.start t && m.gsize (m.start t + k) == m.gsize t) &&
    (m.d t).refs.all fun x => x < m.start t + m.gsize t

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

partial def parseModules (lines : List String) (acc : Array Module) : Except String (Array Module) :=
  match lines with
  | [] => .ok acc
  | l :: rest =>
    match l.splitOn " " with
    | ["M", n] =>
      let n := n.toNat!
      let tlines := rest.take n
      let cline := (rest.drop n).head?.getD ""
      let types := tlines.filterMap fun t => match t.splitOn " " with
        | "T" :: ws => (parseInts ws).bind parseType
        | _ => none
      match cline.splitOn " " with
      | "C" :: cs =>
        if types.length != n || cs.length != n then .error s!"malformed module at: {l}" else
        let md : Module := ⟨n, (types.map (·.1)).toArray, (types.map (·.2.1)).toArray,
          (types.map (·.2.2)).toArray, (cs.map String.toNat!).toArray⟩
        parseModules (rest.drop (n + 1)) (acc.push md)
      | _ => .error s!"missing canonical map after: {l}"
    | [""] => parseModules rest acc
    | _ => .error s!"unexpected line: {l}"

def main : IO UInt32 := do
  let input ← (← IO.getStdin).readToEnd
  match parseModules (input.splitOn "\n") #[] with
  | .error e => IO.eprintln e; return 2
  | .ok mods =>
    let mut bad := 0
    let mut types := 0
    for md in mods do
      let m := md.toM
      types := types + m.n
      if !wfCheck m then
        bad := bad + 1; IO.eprintln s!"module outside the model (not WF), canonical map {md.canon}"; continue
      let c := (build m (fun _ => 0) m.n).get -- canonicalMap, computed once
      let model := (List.range m.n).map c
      if model != md.canon.toList then
        bad := bad + 1; IO.eprintln s!"canonical map {md.canon} differs from the model {model}"
    IO.println s!"{mods.size} modules with {types} types, {bad} failures"
    return if bad == 0 then 0 else 1
