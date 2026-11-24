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
-/
import WahProofs.Layout
import WahProofs.Types
