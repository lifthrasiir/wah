/- Checks that every declaration in the `Wah` namespace of WahSpecTec depends only on the standard axioms
(`make lean-spectec`), as Audit.lean does for WahProofs. -/
import WahSpecTec
import Lean
open Lean Elab Command

#eval show CommandElabM Unit from do
  let allowed := [``propext, ``Classical.choice, ``Quot.sound]
  let mut bad : Array (Name × Name) := #[]
  let mut n : Nat := 0
  for (name, _) in (← getEnv).constants.map₁.toList do
    if (`Wah.SpecTec).isPrefixOf name && !name.isInternal then
      n := n + 1
      for a in ← liftCoreM (collectAxioms name) do
        if !allowed.contains a then bad := bad.push (name, a)
  if bad.isEmpty then logInfo m!"{n} declarations use only standard axioms"
  else throwError m!"declarations with disallowed axioms: {bad}"
