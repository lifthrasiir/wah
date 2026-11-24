/-
Lean proofs relating the models of WahProofs to the Lean model of Wasm 3.0 that SpecTec generates from the spec
(`SpecTec.Wasm`, `SpecTecWP/Wasm.lean` of sgraf812/spectec-wp at a pinned commit, with spectec/model.patch, which
is what the generator fixed by spectec/generator.patch generates: optional atoms such as `NULL?` in rules were always
present, so that `(ref ht) <: (ref null ht)` was missing), checked by `make lean-spectec`,
which fetches the model (about 6 minutes to elaborate once), builds these proofs, rejects unproven constructs and
audits axioms like `make lean`. They are not part of `make lean`, as the model is large and generated elsewhere.

- WahSpecTec/Subtype.lean: heap subtyping of WahProofs/Subtype.lean (`HSub`, which `wah_type_is_subtype` follows)
  implies `Heaptype_sub` of the spec for closed types in the empty context (`hsub_sound`), with closed forms
  translated to SpecTec syntax, assuming that `$minus_recs` (opaque in the generated model) is the function of the
  spec.
- WahSpecTec/SubtypeComplete.lean: the converse for closed forms with the shapes of `wah_type_def_t`
  (`hsub_complete`), so that they agree (`hsub_iff_spec`), through an algorithmic `Heaptype_sub` on SpecTec types
  that valid types (`Heaptype_ok`, which transitivity needs) keep kinds along. Both hold for the atoms of WAH
  (`wahAtom_ok`, `shapeAtom_wah`).
-/
import WahSpecTec.Subtype
import WahSpecTec.SubtypeComplete
