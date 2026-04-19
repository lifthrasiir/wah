(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\01\60"
  "\03\7b\7b\7b\01\7b\03\89\80\80\80\00\08\00\00\00"
  "\00\00\00\00\00\07\bd\81\80\80\00\08\12\66\33\32"
  "\78\34\2e\72\65\6c\61\78\65\64\5f\6d\61\64\64\00"
  "\00\13\66\33\32\78\34\2e\72\65\6c\61\78\65\64\5f"
  "\6e\6d\61\64\64\00\01\13\66\36\34\78\32\2e\72\65"
  "\6c\61\78\65\64\5f\6e\6d\61\64\64\00\02\12\66\36"
  "\34\78\32\2e\72\65\6c\61\78\65\64\5f\6d\61\64\64"
  "\00\03\16\66\33\32\78\34\2e\72\65\6c\61\78\65\64"
  "\5f\6d\61\64\64\5f\63\6d\70\00\04\17\66\33\32\78"
  "\34\2e\72\65\6c\61\78\65\64\5f\6e\6d\61\64\64\5f"
  "\63\6d\70\00\05\17\66\36\34\78\32\2e\72\65\6c\61"
  "\78\65\64\5f\6e\6d\61\64\64\5f\63\6d\70\00\06\16"
  "\66\36\34\78\32\2e\72\65\6c\61\78\65\64\5f\6d\61"
  "\64\64\5f\63\6d\70\00\07\0a\ad\81\80\80\00\08\8b"
  "\80\80\80\00\00\20\00\20\01\20\02\fd\85\02\0b\8b"
  "\80\80\80\00\00\20\00\20\01\20\02\fd\86\02\0b\8b"
  "\80\80\80\00\00\20\00\20\01\20\02\fd\88\02\0b\8b"
  "\80\80\80\00\00\20\00\20\01\20\02\fd\87\02\0b\96"
  "\80\80\80\00\00\20\00\20\01\20\02\fd\85\02\20\00"
  "\20\01\20\02\fd\85\02\fd\41\0b\96\80\80\80\00\00"
  "\20\00\20\01\20\02\fd\86\02\20\00\20\01\20\02\fd"
  "\86\02\fd\41\0b\96\80\80\80\00\00\20\00\20\01\20"
  "\02\fd\88\02\20\00\20\01\20\02\fd\88\02\fd\47\0b"
  "\96\80\80\80\00\00\20\00\20\01\20\02\fd\87\02\20"
  "\00\20\01\20\02\fd\87\02\fd\47\0b"
)
(module instance)
(assert_return
  (invoke "f32x4.relaxed_madd"
    (v128.const i32x4 0x7f7f_ffff 0x7f7f_ffff 0x7f7f_ffff 0x7f7f_ffff)
    (v128.const i32x4 0x4000_0000 0x4000_0000 0x4000_0000 0x4000_0000)
    (v128.const i32x4 0xff7f_ffff 0xff7f_ffff 0xff7f_ffff 0xff7f_ffff)
  )
  (either
    (v128.const f32x4
      0x1.ffff_fep+127
      0x1.ffff_fep+127
      0x1.ffff_fep+127
      0x1.ffff_fep+127
    )
    (v128.const f32x4 inf inf inf inf)
  )
)
(assert_return
  (invoke "f32x4.relaxed_madd"
    (v128.const i32x4 0x3f80_0002 0x3f80_0002 0x3f80_0002 0x3f80_0002)
    (v128.const i32x4 0x3f80_0100 0x3f80_0100 0x3f80_0100 0x3f80_0100)
    (v128.const i32x4 0xbf80_0102 0xbf80_0102 0xbf80_0102 0xbf80_0102)
  )
  (either
    (v128.const f32x4 0x1p-37 0x1p-37 0x1p-37 0x1p-37)
    (v128.const f32x4 0x0p+0 0x0p+0 0x0p+0 0x0p+0)
  )
)
(assert_return
  (invoke "f32x4.relaxed_nmadd"
    (v128.const i32x4 0xbf80_0002 0xbf80_0002 0xbf80_0002 0xbf80_0002)
    (v128.const i32x4 0x3f80_0100 0x3f80_0100 0x3f80_0100 0x3f80_0100)
    (v128.const i32x4 0xbf80_0102 0xbf80_0102 0xbf80_0102 0xbf80_0102)
  )
  (either
    (v128.const f32x4 0x1p-37 0x1p-37 0x1p-37 0x1p-37)
    (v128.const f32x4 0x0p+0 0x0p+0 0x0p+0 0x0p+0)
  )
)
(assert_return
  (invoke "f32x4.relaxed_nmadd"
    (v128.const i32x4 0x3f80_0002 0x3f80_0002 0x3f80_0002 0x3f80_0002)
    (v128.const i32x4 0xbf80_0100 0xbf80_0100 0xbf80_0100 0xbf80_0100)
    (v128.const i32x4 0xbf80_0102 0xbf80_0102 0xbf80_0102 0xbf80_0102)
  )
  (either
    (v128.const f32x4 0x1p-37 0x1p-37 0x1p-37 0x1p-37)
    (v128.const f32x4 0x0p+0 0x0p+0 0x0p+0 0x0p+0)
  )
)
(assert_return
  (invoke "f64x2.relaxed_madd"
    (v128.const i32x4 0xffff_ffff 0x7fef_ffff 0xffff_ffff 0x7fef_ffff)
    (v128.const i32x4 0x0 0x4000_0000 0x0 0x4000_0000)
    (v128.const i32x4 0xffff_ffff 0xffef_ffff 0xffff_ffff 0xffef_ffff)
  )
  (either
    (v128.const f64x2 0x1.ffff_ffff_ffff_fp+1023 0x1.ffff_ffff_ffff_fp+1023)
    (v128.const f64x2 inf inf)
  )
)
(assert_return
  (invoke "f64x2.relaxed_madd"
    (v128.const i32x4 0x40_0000 0x3ff0_0000 0x40_0000 0x3ff0_0000)
    (v128.const i32x4 0x2000_0000 0x3ff0_0000 0x2000_0000 0x3ff0_0000)
    (v128.const i32x4 0x2040_0000 0xbff0_0000 0x2040_0000 0xbff0_0000)
  )
  (either (v128.const f64x2 0x1p-53 0x1p-53) (v128.const f64x2 0x0p+0 0x0p+0))
)
(assert_return
  (invoke "f64x2.relaxed_nmadd"
    (v128.const i32x4 0x40_0000 0xbff0_0000 0x40_0000 0xbff0_0000)
    (v128.const i32x4 0x2000_0000 0x3ff0_0000 0x2000_0000 0x3ff0_0000)
    (v128.const i32x4 0x2040_0000 0xbff0_0000 0x2040_0000 0xbff0_0000)
  )
  (either (v128.const f64x2 0x1p-53 0x1p-53) (v128.const f64x2 0x0p+0 0x0p+0))
)
(assert_return
  (invoke "f64x2.relaxed_nmadd"
    (v128.const i32x4 0x40_0000 0x3ff0_0000 0x40_0000 0x3ff0_0000)
    (v128.const i32x4 0x2000_0000 0xbff0_0000 0x2000_0000 0xbff0_0000)
    (v128.const i32x4 0x2040_0000 0xbff0_0000 0x2040_0000 0xbff0_0000)
  )
  (either (v128.const f64x2 0x1p-53 0x1p-53) (v128.const f64x2 0x0p+0 0x0p+0))
)
(assert_return
  (invoke "f32x4.relaxed_madd_cmp"
    (v128.const i32x4 0x7f7f_ffff 0x7f7f_ffff 0x7f7f_ffff 0x7f7f_ffff)
    (v128.const i32x4 0x4000_0000 0x4000_0000 0x4000_0000 0x4000_0000)
    (v128.const i32x4 0xff7f_ffff 0xff7f_ffff 0xff7f_ffff 0xff7f_ffff)
  )
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "f32x4.relaxed_madd_cmp"
    (v128.const i32x4 0x3f80_0002 0x3f80_0002 0x3f80_0002 0x3f80_0002)
    (v128.const i32x4 0x3f80_0100 0x3f80_0100 0x3f80_0100 0x3f80_0100)
    (v128.const i32x4 0xbf80_0102 0xbf80_0102 0xbf80_0102 0xbf80_0102)
  )
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "f32x4.relaxed_nmadd_cmp"
    (v128.const i32x4 0xbf80_0002 0xbf80_0002 0xbf80_0002 0xbf80_0002)
    (v128.const i32x4 0x3f80_0100 0x3f80_0100 0x3f80_0100 0x3f80_0100)
    (v128.const i32x4 0xbf80_0102 0xbf80_0102 0xbf80_0102 0xbf80_0102)
  )
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "f32x4.relaxed_nmadd_cmp"
    (v128.const i32x4 0x3f80_0002 0x3f80_0002 0x3f80_0002 0x3f80_0002)
    (v128.const i32x4 0xbf80_0100 0xbf80_0100 0xbf80_0100 0xbf80_0100)
    (v128.const i32x4 0xbf80_0102 0xbf80_0102 0xbf80_0102 0xbf80_0102)
  )
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "f64x2.relaxed_madd_cmp"
    (v128.const i32x4 0xffff_ffff 0x7fef_ffff 0xffff_ffff 0x7fef_ffff)
    (v128.const i32x4 0x0 0x4000_0000 0x0 0x4000_0000)
    (v128.const i32x4 0xffff_ffff 0xffef_ffff 0xffff_ffff 0xffef_ffff)
  )
  (v128.const i64x2 0xffff_ffff_ffff_ffff 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "f64x2.relaxed_madd_cmp"
    (v128.const i32x4 0x40_0000 0x3ff0_0000 0x40_0000 0x3ff0_0000)
    (v128.const i32x4 0x2000_0000 0x3ff0_0000 0x2000_0000 0x3ff0_0000)
    (v128.const i32x4 0x2040_0000 0xbff0_0000 0x2040_0000 0xbff0_0000)
  )
  (v128.const i64x2 0xffff_ffff_ffff_ffff 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "f64x2.relaxed_nmadd_cmp"
    (v128.const i32x4 0x40_0000 0xbff0_0000 0x40_0000 0xbff0_0000)
    (v128.const i32x4 0x2000_0000 0x3ff0_0000 0x2000_0000 0x3ff0_0000)
    (v128.const i32x4 0x2040_0000 0xbff0_0000 0x2040_0000 0xbff0_0000)
  )
  (v128.const i64x2 0xffff_ffff_ffff_ffff 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "f64x2.relaxed_nmadd_cmp"
    (v128.const i32x4 0x40_0000 0x3ff0_0000 0x40_0000 0x3ff0_0000)
    (v128.const i32x4 0x2000_0000 0xbff0_0000 0x2000_0000 0xbff0_0000)
    (v128.const i32x4 0x2040_0000 0xbff0_0000 0x2040_0000 0xbff0_0000)
  )
  (v128.const i64x2 0xffff_ffff_ffff_ffff 0xffff_ffff_ffff_ffff)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\01\60"
  "\03\7b\7b\7b\01\7b\03\82\80\80\80\00\01\00\07\a2"
  "\80\80\80\00\01\1e\74\65\73\74\2d\63\6f\6e\73\69"
  "\73\74\65\6e\74\2d\6e\6f\6e\64\65\74\65\72\6d\69"
  "\6e\69\73\6d\00\00\0a\cc\80\80\80\00\01\c6\80\80"
  "\80\00\00\fd\0c\ff\ff\7f\7f\ff\ff\7f\7f\ff\ff\7f"
  "\7f\ff\ff\7f\7f\fd\0c\00\00\00\40\00\00\00\40\00"
  "\00\00\40\00\00\00\40\fd\0c\ff\ff\7f\ff\ff\ff\7f"
  "\ff\ff\ff\7f\ff\ff\ff\7f\ff\fd\85\02\20\00\20\01"
  "\20\02\fd\85\02\fd\41\0b"
)
(module instance)
(assert_return
  (invoke "test-consistent-nondeterminism"
    (v128.const i32x4 0x7f7f_ffff 0x7f7f_ffff 0x7f7f_ffff 0x7f7f_ffff)
    (v128.const i32x4 0x4000_0000 0x4000_0000 0x4000_0000 0x4000_0000)
    (v128.const i32x4 0xff7f_ffff 0xff7f_ffff 0xff7f_ffff 0xff7f_ffff)
  )
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
