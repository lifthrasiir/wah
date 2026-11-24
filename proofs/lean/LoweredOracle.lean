/- Checks verdicts of `wah_check_lowered_code` on functions dumped by dump_lowered.c (and their mutants) against
`check` of WahProofs/Lowered.lean (`make lean-diff`), with the subtyping of the dumped table, which must pass
`tableOK` so that `check_sound` applies. Instructions, one per line, with types as wah_type_t, signatures as
`<count> <type>*` for what is popped (bottom first) and then for what is pushed, and ref maps as -1 (malformed) or
`<count> <position>*`:
  o sig sig              op                 y map sig sig   op with a ref map
  k sig sig              call               r sig sig       return_call
  d / s                  drop / select      z / a           ref.is_null / ref.as_non_null
  g i / x i / t i        local.get/set/tee
  b t k d / B t k d      br / br_if         N t k d / O t k d   br_on_null / br_on_non_null
  T k n (t d)*           br_table           i t / e t / m t     if / else / meter
  p map / q map          poll / tick        n / R / u / X       end / return / unreachable / bad -/
import WahProofs.Lowered
import Std.Data.HashSet
open Wah.Lowered

def types : List Int → Option (List VT × List Int)
  | n :: rest => if 0 ≤ n ∧ n.toNat ≤ rest.length then some (rest.take n.toNat, rest.drop n.toNat) else none
  | [] => none

def map : List Int → Option (Option (List Nat) × List Int)
  | -1 :: rest => some (none, rest)
  | n :: rest => do
    let (ps, rest) ← types (n :: rest)
    if ps.all (0 ≤ ·) then some (some (ps.map Int.toNat), rest) else none
  | [] => none

def sig (ns : List Int) : Option (List VT × List VT) := do
  let (a, rest) ← types ns
  let (r, rest) ← types rest
  if rest.isEmpty then some (a, r) else none

def pairs : List Int → Option (List (Nat × Nat))
  | [] => some []
  | a :: b :: rest => do return (a.toNat, b.toNat) :: (← pairs rest)
  | [_] => none

def parseInstr (op : String) (ns : List Int) : Option Instr := do
  let n := fun (i : Int) => if 0 ≤ i then some i.toNat else none
  match op, ns with
  | "o", _ => let (a, r) ← sig ns; return .op a r
  | "y", _ => let (m, rest) ← map ns; let (a, r) ← sig rest; return .opMap m a r
  | "k", _ => let (a, r) ← sig ns; return .call a r
  | "r", _ => let (a, r) ← sig ns; return .retCall a r
  | "d", [] => return .drop
  | "s", [] => return .select
  | "z", [] => return .refIsNull
  | "a", [] => return .refAsNonNull
  | "g", [i] => return .localGet (← n i)
  | "x", [i] => return .localSet (← n i)
  | "t", [i] => return .localTee (← n i)
  | "b", [t, k, d] => return .br (← n t) (← n k) (← n d)
  | "B", [t, k, d] => return .brIf (← n t) (← n k) (← n d)
  | "N", [t, k, d] => return .brOnNull (← n t) (← n k) (← n d)
  | "O", [t, k, d] => return .brOnNonNull (← n t) (← n k) (← n d)
  | "T", k :: c :: rest => do
    let recs ← pairs rest
    if recs.length == c.toNat then return .brTable (← n k) recs else none
  | "i", [t] => return .if_ (← n t)
  | "e", [t] => return .else_ (← n t)
  | "m", [t] => return .meter (← n t)
  | "p", _ => let (m, rest) ← map ns; if rest.isEmpty then return .poll m else none
  | "q", _ => let (m, rest) ← map ns; if rest.isEmpty then return .tick m else none
  | "n", [] => return .end_
  | "R", [] => return .ret
  | "u", [] => return .unreachable
  | "X", [] => return .bad
  | _, _ => none

def words (l : String) : Option (String × List Int) := do
  let ws := l.splitOn " "
  return (← ws.head?, ← ws.tail.mapM String.toInt?)

def parseHeader (ns : List Int) : Option (Nat × List VT × List VT × Bool) := do
  match ns with
  | d :: rest =>
    let (locals, rest) ← types rest
    let (results, rest) ← types rest
    match rest with
    | [v] => some (d.toNat, locals, results, v != 0)
    | _ => none
  | [] => none

/-- The table of a line `S <n> <type>* <bits>`: pairs of distinct types that are subtypes. -/
def parseTable (l : String) : Option (List (VT × VT)) := do
  let ws := l.splitOn " "
  let n ← (ws[1]?.bind String.toNat?)
  let ts ← (ws.drop 2 |>.take n).mapM String.toInt?
  let bits := (ws[n + 2]?.getD "").toList
  let ps := ts.flatMap fun a => (ts.filter (· != a)).map fun b => (a, b)
  if ts.length != n || bits.length != ps.length then none
  return (ps.zip bits).filterMap fun (p, c) => if c == '1' then some p else none

partial def loop (h : IO.FS.Stream) (n bad accepted : Nat) : IO (Nat × Nat × Nat) := do
  let l := (← h.getLine).trimAsciiEnd.toString
  if l.isEmpty then return (n, bad, accepted)
  let some ("F", ns) := words l | throw (IO.userError s!"unexpected line: {l}")
  let some (d, locals, results, verdict) := parseHeader ns | throw (IO.userError s!"malformed header: {l}")
  let mut code : Array Instr := #[]
  let mut lines : Array String := #[]
  let mut maps : List (Nat × List VT) := []
  let mut tbl : List (VT × VT) := []
  repeat
    let l := (← h.getLine).trimAsciiEnd.toString
    if l == "E" then break
    if l.isEmpty then throw (IO.userError "unexpected end of input")
    if l.startsWith "S " then
      let some t := parseTable l | throw (IO.userError s!"malformed table: {l}")
      tbl := t
      continue
    let some (op, ns) := words l | throw (IO.userError s!"unexpected line: {l}")
    if op == "M" then
      let some (i :: rest) := some ns | throw (IO.userError s!"malformed stack map: {l}")
      let some (ts, []) := types rest | throw (IO.userError s!"malformed stack map: {l}")
      maps := maps ++ [(i.toNat, ts)]
      continue
    let some i := parseInstr op ns | throw (IO.userError s!"unexpected instruction: {l}")
    code := code.push i
    lines := lines.push l
  let set : Std.HashSet (VT × VT) := Std.HashSet.ofList tbl
  let F : Fn := ⟨code, locals, results, d, maps, fun a b => a == b || set.contains (a, b)⟩
  let mut bad := bad
  if !tableOK tbl then
    bad := bad + 1
    IO.eprintln s!"subtyping is not a preorder like that of the spec, or misses types: {tbl}"
  let model := check F
  if model != verdict then
    bad := bad + 1
    if bad ≤ 10 then
      IO.eprintln s!"wah_check_lowered_code {if verdict then "accepts" else "rejects"} but the model doesn't: {l}"
      for (x, k) in lines.zipIdx do IO.eprintln s!"  {k}: {x}"
      IO.eprintln s!"  maps {maps}, subtypes {tbl}"
  loop h (n + 1) bad (accepted + if verdict then 1 else 0)

def main : IO UInt32 := do
  let (n, bad, accepted) ← loop (← IO.getStdin) 0 0 0
  IO.println s!"{n} functions ({accepted} accepted), {bad} failures"
  return if bad == 0 then 0 else 1
