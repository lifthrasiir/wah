/- Checks verdicts of `wah_check_lowered_code` on functions dumped by dump_lowered.c (and their mutants) against
`check` of WahProofs/Lowered.lean (`make lean-diff`). Instructions, one per line:
  c t                    const              o na a* nr r*   op (types bottom first)
  k na a* nr r*          call               r na a* nr r*   return_call
  d                      drop               s               select
  g i / x i / t i        local.get/set/tee
  b t k d / B t k d      br / br_if         T k n (t d)*    br_table
  i t / e t / m t        if / else / meter  p ok / q ok     poll / tick
  n / R / u / X          end / return / unreachable / bad -/
import WahProofs.Lowered
open Wah.Lowered

def vtOf : Nat → Option VT
  | 0 => some .i32 | 1 => some .i64 | 2 => some .f32 | 3 => some .f64 | 4 => some .v128 | _ => none

def types : List Nat → Option (List VT × List Nat)
  | n :: rest => do
    let ts ← (rest.take n).mapM vtOf
    if ts.length == n then some (ts, rest.drop n) else none
  | [] => none

def pairs : List Nat → Option (List (Nat × Nat))
  | [] => some []
  | a :: b :: rest => do return (a, b) :: (← pairs rest)
  | [_] => none

def parseInstr (l : String) : Option Instr := do
  let ws := l.splitOn " "
  let op ← ws.head?
  let ns ← ws.tail.mapM String.toNat?
  let sig := fun (mk : List VT → List VT → Instr) => do
    let (a, rest) ← types ns
    let (r, rest) ← types rest
    if rest.isEmpty then some (mk a r) else none
  match op, ns with
  | "c", [t] => return .const (← vtOf t)
  | "o", _ => sig .op
  | "k", _ => sig .call
  | "r", _ => sig .retCall
  | "d", [] => return .drop
  | "s", [] => return .select
  | "g", [i] => return .localGet i
  | "x", [i] => return .localSet i
  | "t", [i] => return .localTee i
  | "b", [t, k, d] => return .br t k d
  | "B", [t, k, d] => return .brIf t k d
  | "T", k :: n :: rest => do
    let recs ← pairs rest
    if recs.length == n then return .brTable k recs else none
  | "i", [t] => return .if_ t
  | "e", [t] => return .else_ t
  | "m", [t] => return .meter t
  | "p", [ok] => return .poll (ok != 0)
  | "q", [ok] => return .tick (ok != 0)
  | "n", [] => return .end_
  | "R", [] => return .ret
  | "u", [] => return .unreachable
  | "X", [] => return .bad
  | _, _ => none

def parseHeader (l : String) : Option (Nat × List VT × List VT × Bool) := do
  match l.splitOn " " with
  | "F" :: ws =>
    let ns ← ws.mapM String.toNat?
    match ns with
    | d :: rest =>
      let (locals, rest) ← types rest
      let (results, rest) ← types rest
      match rest with
      | [v] => some (d, locals, results, v != 0)
      | _ => none
    | [] => none
  | _ => none

partial def loop (h : IO.FS.Stream) (n bad accepted : Nat) : IO (Nat × Nat × Nat) := do
  let l := (← h.getLine).trimAsciiEnd.toString
  if l.isEmpty then return (n, bad, accepted)
  let some (d, locals, results, verdict) := parseHeader l
    | throw (IO.userError s!"unexpected line: {l}")
  let mut code : Array Instr := #[]
  let mut lines : Array String := #[]
  repeat
    let l := (← h.getLine).trimAsciiEnd.toString
    if l == "E" then break
    if l.isEmpty then throw (IO.userError "unexpected end of input")
    let some i := parseInstr l | throw (IO.userError s!"unexpected instruction: {l}")
    code := code.push i
    lines := lines.push l
  let F : Fn := ⟨code, locals, results, d, []⟩
  let model := check F
  let mut bad := bad
  if model != verdict then
    bad := bad + 1
    if bad ≤ 10 then
      IO.eprintln s!"wah_check_lowered_code {if verdict then "accepts" else "rejects"} but the model doesn't: {l}"
      for (x, k) in lines.zipIdx do IO.eprintln s!"  {k}: {x}"
  loop h (n + 1) bad (accepted + if verdict then 1 else 0)

def main : IO UInt32 := do
  let (n, bad, accepted) ← loop (← IO.getStdin) 0 0 0
  IO.println s!"{n} functions ({accepted} accepted), {bad} failures"
  return if bad == 0 then 0 else 1
