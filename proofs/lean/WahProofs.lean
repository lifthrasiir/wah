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
-/
import WahProofs.Layout
