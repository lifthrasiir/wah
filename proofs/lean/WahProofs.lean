/-
Lean proofs of WAH: Lean 4 models of parts of wah.h with proofs of their properties, checked by `make lean` (needs
elan; the toolchain is pinned by lean-toolchain). No Mathlib or other dependencies. `make lean` builds the proofs,
rejects unproven or unsound constructs in the sources, and checks by Audit.lean that every declaration in the `Wah`
namespace only depends on `propext`, `Classical.choice` and `Quot.sound`.

A proof about a model only matters as long as the model agrees with the C code, so every model has a bridge to the
C code that CI checks. When you change a modeled C function, change its model and bridge in the same commit.

- WahProofs/Layout.lean models `wah_layout_place` (`place`) and the loop of `wah_build_struct_repr_info` (`run`,
  `size`). Fields are aligned, disjoint, within the struct size, padded by at most 15 bytes, and never moved by later
  fields. Bridge: proofs/layout.c (`make cbmc_layout.place`) checks that each step agrees with `place` for every state
  satisfying `Inv`; the loop itself is only checked by review and by `wah_validate_struct_repr_info` in WAH_DEBUG.
- WahProofs/Types.lean models `wah_canonicalize_type_ref` (`canonRef`), `wah_canon_ref_eq` (`refEq`),
  `wah_types_structurally_equal` (`typesEq`), `wah_rec_group_hash[_type]` (`hashRef`, `groupHash` for any mixer)
  and the construction of `canonical_map` in `wah_module_build_type_metadata` (`build`). `canonical_map` maps each
  type to the least type equivalent to it in the spec (iso-recursive equivalence of closed rec groups). Bridges:
  proofs/types.c (`make cbmc_types.*`) checks `canonRef`, `refEq`, `typesEq` and `hashRef` with the encoding of
  wah_type_t, and `make lean-diff` compares canonical maps of the spec tests and the fuzz corpus with `build`
  (Oracle.lean, which reads dumps of dump_types.c), also with all hashes colliding.
- WahProofs/Subtype.lean defines the subtyping of the spec over closed forms (`HSub`, `VSub`) and models
  `wah_type_is_subtype` (`wahSub`) and `wah_type_accepts_repr` (`acceptsWalk`), which agree with it for valid modules
  and canonical maps like `canonical_map`. Bridges: proofs/types.c (`make cbmc_types.is_subtype`,
  `make cbmc_types.accepts_repr`), and `make lean-diff` compares subtyping between all types of a module.
- WahProofs/SubDecl.lean models `wah_type_matches_supertype` (`declCheck`), the check of declared supertypes, which
  agrees with the composite type subtyping of the spec (`CompSub`). Bridge: proofs/types.c
  (`make cbmc_types.matches_supertype`).
- WahProofs/LayoutSub.lean proves that structs lay out the fields of their declared supertypes as the supertypes do,
  as storage types of subtypes have the same size (`fieldClass`, which is checked against `wah_field_type_layout` by
  `make cbmc_layout.field_class`). So the offset check of `wah_check_layouts` always holds for valid modules.
- WahProofs/Cross.lean models `wah_cross_module_type_ref_eq` and the functions it calls (`typeRefEq`), with the memo
  and the depth limit, which finds types of two modules equal only when they are equivalent in the spec, and always
  when rec groups are nested at most `WAH_MAX_TYPE_RECURSION_DEPTH` deep. WahProofs/CrossSub.lean models
  `wah_cross_module_subtype` (`crossSub`), which is the subtyping of the spec under the same condition. Bridge:
  `make lean-diff` compares both between all types of all pairs of modules of each spec test and corpus file. The
  cache of `wah_cross_module_subtype_cached` is only checked by review.
-/
import WahProofs.Layout
import WahProofs.Types
import WahProofs.Subtype
import WahProofs.SubDecl
import WahProofs.LayoutSub
import WahProofs.Cross
import WahProofs.CrossSub
