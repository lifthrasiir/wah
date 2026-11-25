/-
Lean proofs relating the models of WahProofs to the Lean model of Wasm 3.0 that SpecTec generates from the spec
(`SpecTec.Wasm`, `SpecTecWP/Wasm.lean` of sgraf812/spectec-wp at a pinned commit, with spectec/model.patch, which
is what the generator fixed by spectec/generator.patch generates: optional atoms such as `NULL?` in rules were always
present, so that `(ref ht) <: (ref null ht)` was missing), checked by `make lean-spectec`,
which fetches the model (about 6 minutes to elaborate once), builds these proofs, rejects unproven constructs and
audits axioms like `make lean`. They are not part of `make lean`, as the model is large and generated elsewhere.

- WahSpecTec/Subtype.lean: heap subtyping of WahProofs/Subtype.lean (`HSub`, which `wah_type_is_subtype` follows)
  implies `Heaptype_sub` of the spec for closed types in any context (`hsub_sound`), with closed forms
  translated to SpecTec syntax, assuming that `$minus_recs` (opaque in the generated model) is the function of the
  spec.
- WahSpecTec/SubtypeComplete.lean: the converse for closed forms with the shapes of `wah_type_def_t`
  (`hsub_complete`), so that they agree (`hsub_iff_spec`), through an algorithmic `Heaptype_sub` on SpecTec types
  that valid types (`Heaptype_ok`, which transitivity needs) keep kinds along. Both hold for the atoms of WAH
  (`wahAtom_ok`, `shapeAtom_wah`). The converse is for the empty context, as transitivity through type indices and
  recursive types of other contexts is not covered.
- WahSpecTec/Valtype.lean: value subtyping (`VSub`) implies `Valtype_sub` in any context (`vsub_sound`), and the
  converse in the empty context (`vsub_iff_spec`).
- WahSpecTec/Closed.lean: the closed forms of the types of a valid module (`dtOf` of WahProofs/Types.lean) are what
  the spec makes of them: `$rollrt` rolls each rec group uniquely, and `$clos_deftypes` closes the types with their
  groups rolled to the translations of `dtOf`, uniquely (`dtOf_spec`). Closed groups of valid modules translate to
  well-formed types (`okG_closeGroup`), so the subtyping theorems apply to them. The generated `fun_rolldt` lacks the
  length of its result, so the theorems go through `fun_rollrt` and the definition of `$rolldt` instead.
- WahSpecTec/Context.lean: between types of a module, heap subtyping implies `Heaptype_sub` of the spec between their
  type indices in contexts with the types of the module (`idx_hsub_sound`), as the spec checks instructions.
  Supertype chains go through the rolled types of the context and end with `Deftype_sub.refl` on closed forms.
-/
import WahSpecTec.Subtype
import WahSpecTec.SubtypeComplete
import WahSpecTec.Valtype
import WahSpecTec.Closed
import WahSpecTec.Context
