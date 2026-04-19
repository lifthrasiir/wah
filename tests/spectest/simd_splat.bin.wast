(module definition binary
  "\00\61\73\6d\01\00\00\00\01\95\80\80\80\00\04\60"
  "\01\7f\01\7b\60\01\7d\01\7b\60\01\7e\01\7b\60\01"
  "\7c\01\7b\03\87\80\80\80\00\06\00\00\00\01\02\03"
  "\07\d5\80\80\80\00\06\0b\69\38\78\31\36\2e\73\70"
  "\6c\61\74\00\00\0b\69\31\36\78\38\2e\73\70\6c\61"
  "\74\00\01\0b\69\33\32\78\34\2e\73\70\6c\61\74\00"
  "\02\0b\66\33\32\78\34\2e\73\70\6c\61\74\00\03\0b"
  "\69\36\34\78\32\2e\73\70\6c\61\74\00\04\0b\66\36"
  "\34\78\32\2e\73\70\6c\61\74\00\05\0a\c3\80\80\80"
  "\00\06\86\80\80\80\00\00\20\00\fd\0f\0b\86\80\80"
  "\80\00\00\20\00\fd\10\0b\86\80\80\80\00\00\20\00"
  "\fd\11\0b\86\80\80\80\00\00\20\00\fd\13\0b\86\80"
  "\80\80\00\00\20\00\fd\12\0b\86\80\80\80\00\00\20"
  "\00\fd\14\0b"
)
(module instance)
(assert_return
  (invoke "i8x16.splat" (i32.const 0x0))
  (v128.const i8x16
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
  )
)
(assert_return
  (invoke "i8x16.splat" (i32.const 0x5))
  (v128.const i8x16
    0x5
    0x5
    0x5
    0x5
    0x5
    0x5
    0x5
    0x5
    0x5
    0x5
    0x5
    0x5
    0x5
    0x5
    0x5
    0x5
  )
)
(assert_return
  (invoke "i8x16.splat" (i32.const 0xffff_fffb))
  (v128.const i8x16
    0xfb
    0xfb
    0xfb
    0xfb
    0xfb
    0xfb
    0xfb
    0xfb
    0xfb
    0xfb
    0xfb
    0xfb
    0xfb
    0xfb
    0xfb
    0xfb
  )
)
(assert_return
  (invoke "i8x16.splat" (i32.const 0x101))
  (v128.const i8x16
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
  )
)
(assert_return
  (invoke "i8x16.splat" (i32.const 0xff))
  (v128.const i8x16
    0xff
    0xff
    0xff
    0xff
    0xff
    0xff
    0xff
    0xff
    0xff
    0xff
    0xff
    0xff
    0xff
    0xff
    0xff
    0xff
  )
)
(assert_return
  (invoke "i8x16.splat" (i32.const 0xffff_ff80))
  (v128.const i8x16
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
  )
)
(assert_return
  (invoke "i8x16.splat" (i32.const 0x7f))
  (v128.const i8x16
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
  )
)
(assert_return
  (invoke "i8x16.splat" (i32.const 0xffff_ff7f))
  (v128.const i8x16
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
  )
)
(assert_return
  (invoke "i8x16.splat" (i32.const 0x80))
  (v128.const i8x16
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
  )
)
(assert_return
  (invoke "i8x16.splat" (i32.const 0xff7f))
  (v128.const i8x16
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
  )
)
(assert_return
  (invoke "i8x16.splat" (i32.const 0x80))
  (v128.const i8x16
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
    0x80
  )
)
(assert_return
  (invoke "i8x16.splat" (i32.const 0xab))
  (v128.const i32x4 0xabab_abab 0xabab_abab 0xabab_abab 0xabab_abab)
)
(assert_return
  (invoke "i16x8.splat" (i32.const 0x0))
  (v128.const i16x8 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "i16x8.splat" (i32.const 0x5))
  (v128.const i16x8 0x5 0x5 0x5 0x5 0x5 0x5 0x5 0x5)
)
(assert_return
  (invoke "i16x8.splat" (i32.const 0xffff_fffb))
  (v128.const i16x8 0xfffb 0xfffb 0xfffb 0xfffb 0xfffb 0xfffb 0xfffb 0xfffb)
)
(assert_return
  (invoke "i16x8.splat" (i32.const 0x1_0001))
  (v128.const i16x8 0x1 0x1 0x1 0x1 0x1 0x1 0x1 0x1)
)
(assert_return
  (invoke "i16x8.splat" (i32.const 0xffff))
  (v128.const i16x8 0xffff 0xffff 0xffff 0xffff 0xffff 0xffff 0xffff 0xffff)
)
(assert_return
  (invoke "i16x8.splat" (i32.const 0xffff_8000))
  (v128.const i16x8 0x8000 0x8000 0x8000 0x8000 0x8000 0x8000 0x8000 0x8000)
)
(assert_return
  (invoke "i16x8.splat" (i32.const 0x7fff))
  (v128.const i16x8 0x7fff 0x7fff 0x7fff 0x7fff 0x7fff 0x7fff 0x7fff 0x7fff)
)
(assert_return
  (invoke "i16x8.splat" (i32.const 0xffff_7fff))
  (v128.const i16x8 0x7fff 0x7fff 0x7fff 0x7fff 0x7fff 0x7fff 0x7fff 0x7fff)
)
(assert_return
  (invoke "i16x8.splat" (i32.const 0x8000))
  (v128.const i16x8 0x8000 0x8000 0x8000 0x8000 0x8000 0x8000 0x8000 0x8000)
)
(assert_return
  (invoke "i16x8.splat" (i32.const 0xffff_7fff))
  (v128.const i16x8 0x7fff 0x7fff 0x7fff 0x7fff 0x7fff 0x7fff 0x7fff 0x7fff)
)
(assert_return
  (invoke "i16x8.splat" (i32.const 0x8000))
  (v128.const i16x8 0x8000 0x8000 0x8000 0x8000 0x8000 0x8000 0x8000 0x8000)
)
(assert_return
  (invoke "i16x8.splat" (i32.const 0xabcd))
  (v128.const i32x4 0xabcd_abcd 0xabcd_abcd 0xabcd_abcd 0xabcd_abcd)
)
(assert_return
  (invoke "i16x8.splat" (i32.const 0x3039))
  (v128.const i16x8 0x3039 0x3039 0x3039 0x3039 0x3039 0x3039 0x3039 0x3039)
)
(assert_return
  (invoke "i16x8.splat" (i32.const 0x1234))
  (v128.const i16x8 0x1234 0x1234 0x1234 0x1234 0x1234 0x1234 0x1234 0x1234)
)
(assert_return
  (invoke "i32x4.splat" (i32.const 0x0))
  (v128.const i32x4 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "i32x4.splat" (i32.const 0x5))
  (v128.const i32x4 0x5 0x5 0x5 0x5)
)
(assert_return
  (invoke "i32x4.splat" (i32.const 0xffff_fffb))
  (v128.const i32x4 0xffff_fffb 0xffff_fffb 0xffff_fffb 0xffff_fffb)
)
(assert_return
  (invoke "i32x4.splat" (i32.const 0xffff_ffff))
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "i32x4.splat" (i32.const 0xffff_ffff))
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "i32x4.splat" (i32.const 0x8000_0000))
  (v128.const i32x4 0x8000_0000 0x8000_0000 0x8000_0000 0x8000_0000)
)
(assert_return
  (invoke "i32x4.splat" (i32.const 0x7fff_ffff))
  (v128.const i32x4 0x7fff_ffff 0x7fff_ffff 0x7fff_ffff 0x7fff_ffff)
)
(assert_return
  (invoke "i32x4.splat" (i32.const 0x8000_0000))
  (v128.const i32x4 0x8000_0000 0x8000_0000 0x8000_0000 0x8000_0000)
)
(assert_return
  (invoke "i32x4.splat" (i32.const 0x4996_02d2))
  (v128.const i32x4 0x4996_02d2 0x4996_02d2 0x4996_02d2 0x4996_02d2)
)
(assert_return
  (invoke "i32x4.splat" (i32.const 0x1234_5678))
  (v128.const i32x4 0x1234_5678 0x1234_5678 0x1234_5678 0x1234_5678)
)
(assert_return
  (invoke "f32x4.splat" (f32.const 0x0p+0))
  (v128.const f32x4 0x0p+0 0x0p+0 0x0p+0 0x0p+0)
)
(assert_return
  (invoke "f32x4.splat" (f32.const 0x1.1999_9ap+0))
  (v128.const f32x4 0x1.1999_9ap+0 0x1.1999_9ap+0 0x1.1999_9ap+0 0x1.1999_9ap+0)
)
(assert_return
  (invoke "f32x4.splat" (f32.const -0x1.1999_9ap+0))
  (v128.const f32x4
    -0x1.1999_9ap+0
    -0x1.1999_9ap+0
    -0x1.1999_9ap+0
    -0x1.1999_9ap+0
  )
)
(assert_return
  (invoke "f32x4.splat" (f32.const 0x1.2ced_32p+126))
  (v128.const f32x4
    0x1.2ced_32p+126
    0x1.2ced_32p+126
    0x1.2ced_32p+126
    0x1.2ced_32p+126
  )
)
(assert_return
  (invoke "f32x4.splat" (f32.const -0x1.2ced_32p+126))
  (v128.const f32x4
    -0x1.2ced_32p+126
    -0x1.2ced_32p+126
    -0x1.2ced_32p+126
    -0x1.2ced_32p+126
  )
)
(assert_return
  (invoke "f32x4.splat" (f32.const 0x1.ffff_fep+127))
  (v128.const f32x4
    0x1.ffff_fep+127
    0x1.ffff_fep+127
    0x1.ffff_fep+127
    0x1.ffff_fep+127
  )
)
(assert_return
  (invoke "f32x4.splat" (f32.const -0x1.ffff_fep+127))
  (v128.const f32x4
    -0x1.ffff_fep+127
    -0x1.ffff_fep+127
    -0x1.ffff_fep+127
    -0x1.ffff_fep+127
  )
)
(assert_return
  (invoke "f32x4.splat" (f32.const 0x1p+127))
  (v128.const f32x4 0x1p+127 0x1p+127 0x1p+127 0x1p+127)
)
(assert_return
  (invoke "f32x4.splat" (f32.const -0x1p+127))
  (v128.const f32x4 -0x1p+127 -0x1p+127 -0x1p+127 -0x1p+127)
)
(assert_return
  (invoke "f32x4.splat" (f32.const inf))
  (v128.const f32x4 inf inf inf inf)
)
(assert_return
  (invoke "f32x4.splat" (f32.const -inf))
  (v128.const f32x4 -inf -inf -inf -inf)
)
(assert_return
  (invoke "f32x4.splat" (f32.const nan:0x40_0000))
  (v128.const f32x4 nan:0x40_0000 nan:0x40_0000 nan:0x40_0000 nan:0x40_0000)
)
(assert_return
  (invoke "f32x4.splat" (f32.const nan:0x1))
  (v128.const f32x4 nan:0x1 nan:0x1 nan:0x1 nan:0x1)
)
(assert_return
  (invoke "f32x4.splat" (f32.const nan:0x7f_ffff))
  (v128.const f32x4 nan:0x7f_ffff nan:0x7f_ffff nan:0x7f_ffff nan:0x7f_ffff)
)
(assert_return
  (invoke "f32x4.splat" (f32.const 0x1.d6f3_46p+26))
  (v128.const f32x4
    0x1.d6f3_46p+26
    0x1.d6f3_46p+26
    0x1.d6f3_46p+26
    0x1.d6f3_46p+26
  )
)
(assert_return
  (invoke "f32x4.splat" (f32.const 0x1.d6f3_46p+26))
  (v128.const f32x4
    0x1.d6f3_46p+26
    0x1.d6f3_46p+26
    0x1.d6f3_46p+26
    0x1.d6f3_46p+26
  )
)
(assert_return
  (invoke "f32x4.splat" (f32.const 0x1.2345_68p+56))
  (v128.const f32x4
    0x1.2345_68p+56
    0x1.2345_68p+56
    0x1.2345_68p+56
    0x1.2345_68p+56
  )
)
(assert_return
  (invoke "f32x4.splat" (f32.const 0x1.2345_68p+56))
  (v128.const f32x4
    0x1.2345_68p+56
    0x1.2345_68p+56
    0x1.2345_68p+56
    0x1.2345_68p+56
  )
)
(assert_return
  (invoke "f32x4.splat" (f32.const 0x1.fe9a_f6p+89))
  (v128.const f32x4
    0x1.fe9a_f6p+89
    0x1.fe9a_f6p+89
    0x1.fe9a_f6p+89
    0x1.fe9a_f6p+89
  )
)
(assert_return
  (invoke "f32x4.splat" (f32.const 0x1.fe9a_f6p+89))
  (v128.const f32x4
    0x1.fe9a_f6p+89
    0x1.fe9a_f6p+89
    0x1.fe9a_f6p+89
    0x1.fe9a_f6p+89
  )
)
(assert_return
  (invoke "f32x4.splat" (f32.const 0x1.2345_68p+75))
  (v128.const f32x4
    0x1.2345_68p+75
    0x1.2345_68p+75
    0x1.2345_68p+75
    0x1.2345_68p+75
  )
)
(assert_return
  (invoke "f32x4.splat" (f32.const 0x1.2345_68p+37))
  (v128.const f32x4
    0x1.2345_68p+37
    0x1.2345_68p+37
    0x1.2345_68p+37
    0x1.2345_68p+37
  )
)
(assert_return
  (invoke "i64x2.splat" (i64.const 0x0))
  (v128.const i64x2 0x0 0x0)
)
(assert_return
  (invoke "i64x2.splat" (i64.const 0x0))
  (v128.const i64x2 0x0 0x0)
)
(assert_return
  (invoke "i64x2.splat" (i64.const 0x1))
  (v128.const i64x2 0x1 0x1)
)
(assert_return
  (invoke "i64x2.splat" (i64.const 0xffff_ffff_ffff_ffff))
  (v128.const i64x2 0xffff_ffff_ffff_ffff 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "i64x2.splat" (i64.const 0x8000_0000_0000_0000))
  (v128.const i64x2 0x8000_0000_0000_0000 0x8000_0000_0000_0000)
)
(assert_return
  (invoke "i64x2.splat" (i64.const 0x8000_0000_0000_0000))
  (v128.const i64x2 0x8000_0000_0000_0000 0x8000_0000_0000_0000)
)
(assert_return
  (invoke "i64x2.splat" (i64.const 0x7fff_ffff_ffff_ffff))
  (v128.const i64x2 0x7fff_ffff_ffff_ffff 0x7fff_ffff_ffff_ffff)
)
(assert_return
  (invoke "i64x2.splat" (i64.const 0xffff_ffff_ffff_ffff))
  (v128.const i64x2 0xffff_ffff_ffff_ffff 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "i64x2.splat" (i64.const 0x7fff_ffff_ffff_ffff))
  (v128.const i64x2 0x7fff_ffff_ffff_ffff 0x7fff_ffff_ffff_ffff)
)
(assert_return
  (invoke "i64x2.splat" (i64.const 0xffff_ffff_ffff_ffff))
  (v128.const i64x2 0xffff_ffff_ffff_ffff 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "i64x2.splat" (i64.const 0x8000_0000_0000_0000))
  (v128.const i64x2 0x8000_0000_0000_0000 0x8000_0000_0000_0000)
)
(assert_return
  (invoke "i64x2.splat" (i64.const 0x8000_0000_0000_0000))
  (v128.const i64x2 0x8000_0000_0000_0000 0x8000_0000_0000_0000)
)
(assert_return
  (invoke "i64x2.splat" (i64.const 0x1122_10f4_7de9_8115))
  (v128.const i64x2 0x1122_10f4_7de9_8115 0x1122_10f4_7de9_8115)
)
(assert_return
  (invoke "i64x2.splat" (i64.const 0x1234_5678_90ab_cdef))
  (v128.const i64x2 0x1234_5678_90ab_cdef 0x1234_5678_90ab_cdef)
)
(assert_return
  (invoke "f64x2.splat" (f64.const 0x0p+0))
  (v128.const f64x2 0x0p+0 0x0p+0)
)
(assert_return
  (invoke "f64x2.splat" (f64.const -0x0p+0))
  (v128.const f64x2 -0x0p+0 -0x0p+0)
)
(assert_return
  (invoke "f64x2.splat" (f64.const 0x1.1999_9999_9999_ap+0))
  (v128.const f64x2 0x1.1999_9999_9999_ap+0 0x1.1999_9999_9999_ap+0)
)
(assert_return
  (invoke "f64x2.splat" (f64.const -0x1.1999_9999_9999_ap+0))
  (v128.const f64x2 -0x1.1999_9999_9999_ap+0 -0x1.1999_9999_9999_ap+0)
)
(assert_return
  (invoke "f64x2.splat" (f64.const 0x0.0000_0000_0000_1p-1022))
  (v128.const f64x2 0x0.0000_0000_0000_1p-1022 0x0.0000_0000_0000_1p-1022)
)
(assert_return
  (invoke "f64x2.splat" (f64.const -0x0.0000_0000_0000_1p-1022))
  (v128.const f64x2 -0x0.0000_0000_0000_1p-1022 -0x0.0000_0000_0000_1p-1022)
)
(assert_return
  (invoke "f64x2.splat" (f64.const 0x1p-1022))
  (v128.const f64x2 0x1p-1022 0x1p-1022)
)
(assert_return
  (invoke "f64x2.splat" (f64.const -0x1p-1022))
  (v128.const f64x2 -0x1p-1022 -0x1p-1022)
)
(assert_return
  (invoke "f64x2.splat" (f64.const 0x1p-1))
  (v128.const f64x2 0x1p-1 0x1p-1)
)
(assert_return
  (invoke "f64x2.splat" (f64.const -0x1p-1))
  (v128.const f64x2 -0x1p-1 -0x1p-1)
)
(assert_return
  (invoke "f64x2.splat" (f64.const 0x1p+0))
  (v128.const f64x2 0x1p+0 0x1p+0)
)
(assert_return
  (invoke "f64x2.splat" (f64.const -0x1p+0))
  (v128.const f64x2 -0x1p+0 -0x1p+0)
)
(assert_return
  (invoke "f64x2.splat" (f64.const 0x1.921f_b544_42d1_8p+2))
  (v128.const f64x2 0x1.921f_b544_42d1_8p+2 0x1.921f_b544_42d1_8p+2)
)
(assert_return
  (invoke "f64x2.splat" (f64.const -0x1.921f_b544_42d1_8p+2))
  (v128.const f64x2 -0x1.921f_b544_42d1_8p+2 -0x1.921f_b544_42d1_8p+2)
)
(assert_return
  (invoke "f64x2.splat" (f64.const 0x1.ffff_ffff_ffff_fp+1023))
  (v128.const f64x2 0x1.ffff_ffff_ffff_fp+1023 0x1.ffff_ffff_ffff_fp+1023)
)
(assert_return
  (invoke "f64x2.splat" (f64.const -0x1.ffff_ffff_ffff_fp+1023))
  (v128.const f64x2 -0x1.ffff_ffff_ffff_fp+1023 -0x1.ffff_ffff_ffff_fp+1023)
)
(assert_return
  (invoke "f64x2.splat" (f64.const inf))
  (v128.const f64x2 inf inf)
)
(assert_return
  (invoke "f64x2.splat" (f64.const -inf))
  (v128.const f64x2 -inf -inf)
)
(assert_return
  (invoke "f64x2.splat" (f64.const nan:0x8_0000_0000_0000))
  (v128.const f64x2 nan:0x8_0000_0000_0000 nan:0x8_0000_0000_0000)
)
(assert_return
  (invoke "f64x2.splat" (f64.const -nan:0x8_0000_0000_0000))
  (v128.const f64x2 -nan:0x8_0000_0000_0000 -nan:0x8_0000_0000_0000)
)
(assert_return
  (invoke "f64x2.splat" (f64.const nan:0x4_0000_0000_0000))
  (v128.const f64x2 nan:0x4_0000_0000_0000 nan:0x4_0000_0000_0000)
)
(assert_return
  (invoke "f64x2.splat" (f64.const -nan:0x4_0000_0000_0000))
  (v128.const f64x2 -nan:0x4_0000_0000_0000 -nan:0x4_0000_0000_0000)
)
(assert_return
  (invoke "f64x2.splat" (f64.const 0x1.d6f3_454p+26))
  (v128.const f64x2 0x1.d6f3_454p+26 0x1.d6f3_454p+26)
)
(assert_return
  (invoke "f64x2.splat" (f64.const 0x1.d6f3_454p+26))
  (v128.const f64x2 0x1.d6f3_454p+26 0x1.d6f3_454p+26)
)
(assert_return
  (invoke "f64x2.splat" (f64.const 0x1.2345_6789_abcd_fp+80))
  (v128.const f64x2 0x1.2345_6789_abcd_fp+80 0x1.2345_6789_abcd_fp+80)
)
(assert_return
  (invoke "f64x2.splat" (f64.const 0x1.2345_6789_abcd_fp+80))
  (v128.const f64x2 0x1.2345_6789_abcd_fp+80 0x1.2345_6789_abcd_fp+80)
)
(assert_return
  (invoke "f64x2.splat" (f64.const 0x1.fe9a_f5b5_e16f_ap+89))
  (v128.const f64x2 0x1.fe9a_f5b5_e16f_ap+89 0x1.fe9a_f5b5_e16f_ap+89)
)
(assert_return
  (invoke "f64x2.splat" (f64.const 0x1.fe9a_f5b5_e16f_ap+89))
  (v128.const f64x2 0x1.fe9a_f5b5_e16f_ap+89 0x1.fe9a_f5b5_e16f_ap+89)
)
(assert_return
  (invoke "f64x2.splat" (f64.const 0x1.2345_6789_abcd_fp+99))
  (v128.const f64x2 0x1.2345_6789_abcd_fp+99 0x1.2345_6789_abcd_fp+99)
)
(assert_return
  (invoke "f64x2.splat" (f64.const 0x1.2345_6789_abcd_fp+61))
  (v128.const f64x2 0x1.2345_6789_abcd_fp+61 0x1.2345_6789_abcd_fp+61)
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8c\80\80\80"
    "\00\01\86\80\80\80\00\00\fd\0f\42\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8f\80\80\80"
    "\00\01\89\80\80\80\00\00\fd\0f\43\00\00\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\93\80\80\80"
    "\00\01\8d\80\80\80\00\00\fd\0f\44\00\00\00\00\00"
    "\00\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8c\80\80\80"
    "\00\01\86\80\80\80\00\00\fd\10\42\01\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8f\80\80\80"
    "\00\01\89\80\80\80\00\00\fd\10\43\00\00\80\3f\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\93\80\80\80"
    "\00\01\8d\80\80\80\00\00\fd\10\44\00\00\00\00\00"
    "\00\f0\3f\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8c\80\80\80"
    "\00\01\86\80\80\80\00\00\fd\11\42\02\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8f\80\80\80"
    "\00\01\89\80\80\80\00\00\fd\11\43\00\00\00\40\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\93\80\80\80"
    "\00\01\8d\80\80\80\00\00\fd\11\44\00\00\00\00\00"
    "\00\00\40\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8c\80\80\80"
    "\00\01\86\80\80\80\00\00\fd\13\41\04\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8c\80\80\80"
    "\00\01\86\80\80\80\00\00\fd\13\42\04\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\93\80\80\80"
    "\00\01\8d\80\80\80\00\00\fd\13\44\00\00\00\00\00"
    "\00\10\40\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8c\80\80\80"
    "\00\01\86\80\80\80\00\00\fd\12\41\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\93\80\80\80"
    "\00\01\8d\80\80\80\00\00\fd\12\44\00\00\00\00\00"
    "\00\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8c\80\80\80"
    "\00\01\86\80\80\80\00\00\fd\14\41\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8f\80\80\80"
    "\00\01\89\80\80\80\00\00\fd\14\43\00\00\00\00\0b"
  )
  "type mismatch"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\90\80\80\80\00\03\60"
  "\01\7f\01\7b\60\01\7e\01\7b\60\01\7c\01\7b\03\86"
  "\80\80\80\00\05\00\00\00\01\02\05\83\80\80\80\00"
  "\01\00\01\07\83\81\80\80\00\05\17\61\73\2d\76\31"
  "\32\38\5f\73\74\6f\72\65\2d\6f\70\65\72\61\6e\64"
  "\2d\31\00\00\17\61\73\2d\76\31\32\38\5f\73\74\6f"
  "\72\65\2d\6f\70\65\72\61\6e\64\2d\32\00\01\17\61"
  "\73\2d\76\31\32\38\5f\73\74\6f\72\65\2d\6f\70\65"
  "\72\61\6e\64\2d\33\00\02\17\61\73\2d\76\31\32\38"
  "\5f\73\74\6f\72\65\2d\6f\70\65\72\61\6e\64\2d\34"
  "\00\03\17\61\73\2d\76\31\32\38\5f\73\74\6f\72\65"
  "\2d\6f\70\65\72\61\6e\64\2d\35\00\04\0a\f4\80\80"
  "\80\00\05\92\80\80\80\00\00\41\00\20\00\fd\0f\fd"
  "\0b\04\00\41\00\fd\00\04\00\0b\92\80\80\80\00\00"
  "\41\00\20\00\fd\10\fd\0b\04\00\41\00\fd\00\04\00"
  "\0b\92\80\80\80\00\00\41\00\20\00\fd\11\fd\0b\04"
  "\00\41\00\fd\00\04\00\0b\92\80\80\80\00\00\41\00"
  "\20\00\fd\12\fd\0b\04\00\41\00\fd\00\04\00\0b\92"
  "\80\80\80\00\00\41\00\20\00\fd\14\fd\0b\04\00\41"
  "\00\fd\00\04\00\0b"
)
(module instance)
(assert_return
  (invoke "as-v128_store-operand-1" (i32.const 0x1))
  (v128.const i8x16
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
    0x1
  )
)
(assert_return
  (invoke "as-v128_store-operand-2" (i32.const 0x100))
  (v128.const i16x8 0x100 0x100 0x100 0x100 0x100 0x100 0x100 0x100)
)
(assert_return
  (invoke "as-v128_store-operand-3" (i32.const 0xffff_ffff))
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "as-v128_store-operand-4" (i64.const 0x1))
  (v128.const i64x2 0x1 0x1)
)
(assert_return
  (invoke "as-v128_store-operand-5" (f64.const -0x1p+0))
  (v128.const f64x2 -0x1p+0 -0x1p+0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\db\80\80\80\00\0f\60"
  "\01\7f\01\7f\60\01\7d\01\7d\60\02\7f\7f\01\7b\60"
  "\01\7e\01\7e\60\01\7c\01\7c\60\03\7f\7f\7f\01\7b"
  "\60\04\7f\7f\7f\7f\01\7b\60\04\7e\7e\7e\7e\01\7b"
  "\60\04\7c\7c\7c\7c\01\7b\60\01\7e\01\7f\60\02\7e"
  "\7e\01\7b\60\02\7d\7d\01\7b\60\02\7c\7c\01\7b\60"
  "\01\7d\01\7b\60\01\7f\01\7b\03\ac\80\80\80\00\2b"
  "\00\00\00\00\00\00\01\01\02\03\03\04\04\05\06\06"
  "\07\08\02\02\02\02\02\02\02\02\02\02\00\00\00\09"
  "\02\02\02\0a\0b\0c\0d\0b\0b\0e\0d\07\9f\8a\80\80"
  "\00\2b\25\61\73\2d\69\38\78\31\36\5f\65\78\74\72"
  "\61\63\74\5f\6c\61\6e\65\5f\73\2d\6f\70\65\72\61"
  "\6e\64\2d\66\69\72\73\74\00\00\24\61\73\2d\69\38"
  "\78\31\36\5f\65\78\74\72\61\63\74\5f\6c\61\6e\65"
  "\5f\73\2d\6f\70\65\72\61\6e\64\2d\6c\61\73\74\00"
  "\01\25\61\73\2d\69\31\36\78\38\5f\65\78\74\72\61"
  "\63\74\5f\6c\61\6e\65\5f\73\2d\6f\70\65\72\61\6e"
  "\64\2d\66\69\72\73\74\00\02\24\61\73\2d\69\31\36"
  "\78\38\5f\65\78\74\72\61\63\74\5f\6c\61\6e\65\5f"
  "\73\2d\6f\70\65\72\61\6e\64\2d\6c\61\73\74\00\03"
  "\25\61\73\2d\69\33\32\78\34\5f\65\78\74\72\61\63"
  "\74\5f\6c\61\6e\65\5f\73\2d\6f\70\65\72\61\6e\64"
  "\2d\66\69\72\73\74\00\04\24\61\73\2d\69\33\32\78"
  "\34\5f\65\78\74\72\61\63\74\5f\6c\61\6e\65\5f\73"
  "\2d\6f\70\65\72\61\6e\64\2d\6c\61\73\74\00\05\25"
  "\61\73\2d\66\33\32\78\34\5f\65\78\74\72\61\63\74"
  "\5f\6c\61\6e\65\5f\73\2d\6f\70\65\72\61\6e\64\2d"
  "\66\69\72\73\74\00\06\24\61\73\2d\66\33\32\78\34"
  "\5f\65\78\74\72\61\63\74\5f\6c\61\6e\65\5f\73\2d"
  "\6f\70\65\72\61\6e\64\2d\6c\61\73\74\00\07\19\61"
  "\73\2d\76\38\78\31\36\5f\73\77\69\7a\7a\6c\65\2d"
  "\6f\70\65\72\61\6e\64\73\00\08\23\61\73\2d\69\36"
  "\34\78\32\5f\65\78\74\72\61\63\74\5f\6c\61\6e\65"
  "\2d\6f\70\65\72\61\6e\64\2d\66\69\72\73\74\00\09"
  "\22\61\73\2d\69\36\34\78\32\5f\65\78\74\72\61\63"
  "\74\5f\6c\61\6e\65\2d\6f\70\65\72\61\6e\64\2d\6c"
  "\61\73\74\00\0a\23\61\73\2d\66\36\34\78\32\5f\65"
  "\78\74\72\61\63\74\5f\6c\61\6e\65\2d\6f\70\65\72"
  "\61\6e\64\2d\66\69\72\73\74\00\0b\22\61\73\2d\66"
  "\36\34\78\32\5f\65\78\74\72\61\63\74\5f\6c\61\6e"
  "\65\2d\6f\70\65\72\61\6e\64\2d\6c\61\73\74\00\0c"
  "\19\61\73\2d\69\38\78\31\36\5f\61\64\64\5f\73\75"
  "\62\2d\6f\70\65\72\61\6e\64\73\00\0d\1d\61\73\2d"
  "\69\31\36\78\38\5f\61\64\64\5f\73\75\62\5f\6d\75"
  "\6c\2d\6f\70\65\72\61\6e\64\73\00\0e\1d\61\73\2d"
  "\69\33\32\78\34\5f\61\64\64\5f\73\75\62\5f\6d\75"
  "\6c\2d\6f\70\65\72\61\6e\64\73\00\0f\1d\61\73\2d"
  "\69\36\34\78\32\5f\61\64\64\5f\73\75\62\5f\6d\75"
  "\6c\2d\6f\70\65\72\61\6e\64\73\00\10\1d\61\73\2d"
  "\66\36\34\78\32\5f\61\64\64\5f\73\75\62\5f\6d\75"
  "\6c\2d\6f\70\65\72\61\6e\64\73\00\11\1b\61\73\2d"
  "\69\38\78\31\36\5f\61\64\64\5f\73\61\74\5f\73\2d"
  "\6f\70\65\72\61\6e\64\73\00\12\1b\61\73\2d\69\31"
  "\36\78\38\5f\61\64\64\5f\73\61\74\5f\73\2d\6f\70"
  "\65\72\61\6e\64\73\00\13\1b\61\73\2d\69\38\78\31"
  "\36\5f\73\75\62\5f\73\61\74\5f\75\2d\6f\70\65\72"
  "\61\6e\64\73\00\14\1b\61\73\2d\69\31\36\78\38\5f"
  "\73\75\62\5f\73\61\74\5f\75\2d\6f\70\65\72\61\6e"
  "\64\73\00\15\16\61\73\2d\69\38\78\31\36\5f\73\68"
  "\72\5f\73\2d\6f\70\65\72\61\6e\64\00\16\16\61\73"
  "\2d\69\31\36\78\38\5f\73\68\72\5f\73\2d\6f\70\65"
  "\72\61\6e\64\00\17\16\61\73\2d\69\33\32\78\34\5f"
  "\73\68\72\5f\73\2d\6f\70\65\72\61\6e\64\00\18\14"
  "\61\73\2d\76\31\32\38\5f\61\6e\64\2d\6f\70\65\72"
  "\61\6e\64\73\00\19\13\61\73\2d\76\31\32\38\5f\6f"
  "\72\2d\6f\70\65\72\61\6e\64\73\00\1a\14\61\73\2d"
  "\76\31\32\38\5f\78\6f\72\2d\6f\70\65\72\61\6e\64"
  "\73\00\1b\19\61\73\2d\69\38\78\31\36\5f\61\6c\6c"
  "\5f\74\72\75\65\2d\6f\70\65\72\61\6e\64\00\1c\19"
  "\61\73\2d\69\31\36\78\38\5f\61\6c\6c\5f\74\72\75"
  "\65\2d\6f\70\65\72\61\6e\64\00\1d\1a\61\73\2d\69"
  "\33\32\78\34\5f\61\6c\6c\5f\74\72\75\65\2d\6f\70"
  "\65\72\61\6e\64\31\00\1e\1a\61\73\2d\69\33\32\78"
  "\34\5f\61\6c\6c\5f\74\72\75\65\2d\6f\70\65\72\61"
  "\6e\64\32\00\1f\14\61\73\2d\69\38\78\31\36\5f\65"
  "\71\2d\6f\70\65\72\61\6e\64\73\00\20\14\61\73\2d"
  "\69\31\36\78\38\5f\65\71\2d\6f\70\65\72\61\6e\64"
  "\73\00\21\15\61\73\2d\69\33\32\78\34\5f\65\71\2d"
  "\6f\70\65\72\61\6e\64\73\31\00\22\15\61\73\2d\69"
  "\33\32\78\34\5f\65\71\2d\6f\70\65\72\61\6e\64\73"
  "\32\00\23\14\61\73\2d\66\33\32\78\34\5f\65\71\2d"
  "\6f\70\65\72\61\6e\64\73\00\24\14\61\73\2d\66\36"
  "\34\78\32\5f\65\71\2d\6f\70\65\72\61\6e\64\73\00"
  "\25\14\61\73\2d\66\33\32\78\34\5f\61\62\73\2d\6f"
  "\70\65\72\61\6e\64\00\26\15\61\73\2d\66\33\32\78"
  "\34\5f\6d\69\6e\2d\6f\70\65\72\61\6e\64\73\00\27"
  "\15\61\73\2d\66\33\32\78\34\5f\64\69\76\2d\6f\70"
  "\65\72\61\6e\64\73\00\28\20\61\73\2d\66\33\32\78"
  "\34\5f\63\6f\6e\76\65\72\74\5f\73\5f\69\33\32\78"
  "\34\2d\6f\70\65\72\61\6e\64\00\29\22\61\73\2d\69"
  "\33\32\78\34\5f\74\72\75\6e\63\5f\73\5f\66\33\32"
  "\78\34\5f\73\61\74\2d\6f\70\65\72\61\6e\64\00\2a"
  "\0a\e4\85\80\80\00\2b\89\80\80\80\00\00\20\00\fd"
  "\0f\fd\15\00\0b\89\80\80\80\00\00\20\00\fd\0f\fd"
  "\15\0f\0b\89\80\80\80\00\00\20\00\fd\10\fd\18\00"
  "\0b\89\80\80\80\00\00\20\00\fd\10\fd\18\07\0b\89"
  "\80\80\80\00\00\20\00\fd\11\fd\1b\00\0b\89\80\80"
  "\80\00\00\20\00\fd\11\fd\1b\03\0b\89\80\80\80\00"
  "\00\20\00\fd\13\fd\1f\00\0b\89\80\80\80\00\00\20"
  "\00\fd\13\fd\1f\03\0b\8c\80\80\80\00\00\20\00\fd"
  "\0f\20\01\fd\0f\fd\0e\0b\89\80\80\80\00\00\20\00"
  "\fd\12\fd\1d\00\0b\89\80\80\80\00\00\20\00\fd\12"
  "\fd\1d\01\0b\89\80\80\80\00\00\20\00\fd\14\fd\21"
  "\00\0b\89\80\80\80\00\00\20\00\fd\14\fd\21\01\0b"
  "\92\80\80\80\00\00\20\00\fd\0f\20\01\fd\0f\20\02"
  "\fd\0f\fd\71\fd\6e\0b\9b\80\80\80\00\00\20\00\fd"
  "\10\20\01\fd\10\20\02\fd\10\20\03\fd\10\fd\95\01"
  "\fd\91\01\fd\8e\01\0b\9b\80\80\80\00\00\20\00\fd"
  "\11\20\01\fd\11\20\02\fd\11\20\03\fd\11\fd\b5\01"
  "\fd\b1\01\fd\ae\01\0b\9b\80\80\80\00\00\20\00\fd"
  "\12\20\01\fd\12\20\02\fd\12\20\03\fd\12\fd\d5\01"
  "\fd\d1\01\fd\ce\01\0b\9b\80\80\80\00\00\20\00\fd"
  "\14\20\01\fd\14\20\02\fd\14\20\03\fd\14\fd\f2\01"
  "\fd\f1\01\fd\f0\01\0b\8c\80\80\80\00\00\20\00\fd"
  "\0f\20\01\fd\0f\fd\6f\0b\8d\80\80\80\00\00\20\00"
  "\fd\10\20\01\fd\10\fd\8f\01\0b\8c\80\80\80\00\00"
  "\20\00\fd\0f\20\01\fd\0f\fd\73\0b\8d\80\80\80\00"
  "\00\20\00\fd\10\20\01\fd\10\fd\93\01\0b\8a\80\80"
  "\80\00\00\20\00\fd\0f\20\01\fd\6c\0b\8b\80\80\80"
  "\00\00\20\00\fd\10\20\01\fd\8c\01\0b\8b\80\80\80"
  "\00\00\20\00\fd\11\20\01\fd\ac\01\0b\8c\80\80\80"
  "\00\00\20\00\fd\0f\20\01\fd\0f\fd\4e\0b\8c\80\80"
  "\80\00\00\20\00\fd\10\20\01\fd\10\fd\50\0b\8c\80"
  "\80\80\00\00\20\00\fd\11\20\01\fd\11\fd\51\0b\88"
  "\80\80\80\00\00\20\00\fd\0f\fd\63\0b\89\80\80\80"
  "\00\00\20\00\fd\10\fd\83\01\0b\89\80\80\80\00\00"
  "\20\00\fd\11\fd\a3\01\0b\89\80\80\80\00\00\20\00"
  "\fd\12\fd\a3\01\0b\8c\80\80\80\00\00\20\00\fd\0f"
  "\20\01\fd\0f\fd\23\0b\8c\80\80\80\00\00\20\00\fd"
  "\10\20\01\fd\10\fd\2d\0b\8c\80\80\80\00\00\20\00"
  "\fd\11\20\01\fd\11\fd\37\0b\8c\80\80\80\00\00\20"
  "\00\fd\12\20\01\fd\12\fd\37\0b\8c\80\80\80\00\00"
  "\20\00\fd\13\20\01\fd\13\fd\41\0b\8c\80\80\80\00"
  "\00\20\00\fd\14\20\01\fd\14\fd\47\0b\89\80\80\80"
  "\00\00\20\00\fd\13\fd\e0\01\0b\8d\80\80\80\00\00"
  "\20\00\fd\13\20\01\fd\13\fd\e8\01\0b\8d\80\80\80"
  "\00\00\20\00\fd\13\20\01\fd\13\fd\e7\01\0b\89\80"
  "\80\80\00\00\20\00\fd\11\fd\fa\01\0b\89\80\80\80"
  "\00\00\20\00\fd\13\fd\f8\01\0b"
)
(module instance)
(assert_return
  (invoke "as-i8x16_extract_lane_s-operand-first" (i32.const 0x2a))
  (i32.const 0x2a)
)
(assert_return
  (invoke "as-i8x16_extract_lane_s-operand-last" (i32.const 0xffff_ffd6))
  (i32.const 0xffff_ffd6)
)
(assert_return
  (invoke "as-i16x8_extract_lane_s-operand-first" (i32.const 0xffff_7fff))
  (i32.const 0x7fff)
)
(assert_return
  (invoke "as-i16x8_extract_lane_s-operand-last" (i32.const 0x8000))
  (i32.const 0xffff_8000)
)
(assert_return
  (invoke "as-i32x4_extract_lane_s-operand-first" (i32.const 0x7fff_ffff))
  (i32.const 0x7fff_ffff)
)
(assert_return
  (invoke "as-i32x4_extract_lane_s-operand-last" (i32.const 0x8000_0000))
  (i32.const 0x8000_0000)
)
(assert_return
  (invoke "as-f32x4_extract_lane_s-operand-first" (f32.const 0x1.8p+0))
  (f32.const 0x1.8p+0)
)
(assert_return
  (invoke "as-f32x4_extract_lane_s-operand-last" (f32.const -0x1p-2))
  (f32.const -0x1p-2)
)
(assert_return
  (invoke "as-v8x16_swizzle-operands" (i32.const 0x1) (i32.const 0xffff_ffff))
  (v128.const i8x16
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
  )
)
(assert_return
  (invoke "as-i64x2_extract_lane-operand-last"
    (i64.const 0xffff_ffff_ffff_ffd6)
  )
  (i64.const 0xffff_ffff_ffff_ffd6)
)
(assert_return
  (invoke "as-i64x2_extract_lane-operand-first" (i64.const 0x2a))
  (i64.const 0x2a)
)
(assert_return
  (invoke "as-f64x2_extract_lane-operand-first" (f64.const 0x1.8p+0))
  (f64.const 0x1.8p+0)
)
(assert_return
  (invoke "as-f64x2_extract_lane-operand-last" (f64.const -0x1p+0))
  (f64.const -0x1p+0)
)
(assert_return
  (invoke "as-i8x16_add_sub-operands"
    (i32.const 0x3)
    (i32.const 0x2)
    (i32.const 0x1)
  )
  (v128.const i8x16
    0x4
    0x4
    0x4
    0x4
    0x4
    0x4
    0x4
    0x4
    0x4
    0x4
    0x4
    0x4
    0x4
    0x4
    0x4
    0x4
  )
)
(assert_return
  (invoke "as-i16x8_add_sub_mul-operands"
    (i32.const 0x101)
    (i32.const 0x80)
    (i32.const 0x10)
    (i32.const 0x10)
  )
  (v128.const i16x8 0x81 0x81 0x81 0x81 0x81 0x81 0x81 0x81)
)
(assert_return
  (invoke "as-i32x4_add_sub_mul-operands"
    (i32.const 0xffff)
    (i32.const 0x1_0001)
    (i32.const 0x100)
    (i32.const 0x100)
  )
  (v128.const i32x4 0x1_0000 0x1_0000 0x1_0000 0x1_0000)
)
(assert_return
  (invoke "as-i64x2_add_sub_mul-operands"
    (i64.const 0x7fff_ffff)
    (i64.const 0x1_0000_0001)
    (i64.const 0x1_0000)
    (i64.const 0x1_0000)
  )
  (v128.const i64x2 0x8000_0000 0x8000_0000)
)
(assert_return
  (invoke "as-f64x2_add_sub_mul-operands"
    (f64.const 0x1p-1)
    (f64.const 0x1.8p-1)
    (f64.const 0x1p-1)
    (f64.const 0x1p-1)
  )
  (v128.const f64x2 0x1p+0 0x1p+0)
)
(assert_return
  (invoke "as-i8x16_add_sat_s-operands" (i32.const 0x7f) (i32.const 0x1))
  (v128.const i8x16
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
    0x7f
  )
)
(assert_return
  (invoke "as-i16x8_add_sat_s-operands" (i32.const 0x7fff) (i32.const 0x1))
  (v128.const i16x8 0x7fff 0x7fff 0x7fff 0x7fff 0x7fff 0x7fff 0x7fff 0x7fff)
)
(assert_return
  (invoke "as-i8x16_sub_sat_u-operands" (i32.const 0x7f) (i32.const 0xff))
  (v128.const i8x16
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
  )
)
(assert_return
  (invoke "as-i16x8_sub_sat_u-operands" (i32.const 0x7fff) (i32.const 0xffff))
  (v128.const i16x8 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "as-i8x16_shr_s-operand" (i32.const 0xf0) (i32.const 0x3))
  (v128.const i8x16
    0xfe
    0xfe
    0xfe
    0xfe
    0xfe
    0xfe
    0xfe
    0xfe
    0xfe
    0xfe
    0xfe
    0xfe
    0xfe
    0xfe
    0xfe
    0xfe
  )
)
(assert_return
  (invoke "as-i16x8_shr_s-operand" (i32.const 0x100) (i32.const 0x4))
  (v128.const i16x8 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10)
)
(assert_return
  (invoke "as-i32x4_shr_s-operand" (i32.const 0xffff_ffff) (i32.const 0x10))
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "as-v128_and-operands" (i32.const 0x11) (i32.const 0xff))
  (v128.const i8x16
    0x11
    0x11
    0x11
    0x11
    0x11
    0x11
    0x11
    0x11
    0x11
    0x11
    0x11
    0x11
    0x11
    0x11
    0x11
    0x11
  )
)
(assert_return
  (invoke "as-v128_or-operands" (i32.const 0x0) (i32.const 0xffff))
  (v128.const i16x8 0xffff 0xffff 0xffff 0xffff 0xffff 0xffff 0xffff 0xffff)
)
(assert_return
  (invoke "as-v128_xor-operands"
    (i32.const 0xf0f0_f0f0)
    (i32.const 0xffff_ffff)
  )
  (v128.const i32x4 0xf0f_0f0f 0xf0f_0f0f 0xf0f_0f0f 0xf0f_0f0f)
)
(assert_return
  (invoke "as-i8x16_all_true-operand" (i32.const 0x0))
  (i32.const 0x0)
)
(assert_return
  (invoke "as-i16x8_all_true-operand" (i32.const 0xffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "as-i32x4_all_true-operand1" (i32.const 0xf0f0_f0f0))
  (i32.const 0x1)
)
(assert_return
  (invoke "as-i32x4_all_true-operand2" (i64.const 0xffff_ffff_ffff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "as-i8x16_eq-operands" (i32.const 0x1) (i32.const 0x2))
  (v128.const i8x16
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
    0x0
  )
)
(assert_return
  (invoke "as-i16x8_eq-operands" (i32.const 0xffff_ffff) (i32.const 0xffff))
  (v128.const i16x8 0xffff 0xffff 0xffff 0xffff 0xffff 0xffff 0xffff 0xffff)
)
(assert_return
  (invoke "as-i32x4_eq-operands1"
    (i32.const 0xffff_ffff)
    (i32.const 0xffff_ffff)
  )
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "as-f32x4_eq-operands" (f32.const 0x0p+0) (f32.const -0x0p+0))
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "as-i32x4_eq-operands2" (i64.const 0x1) (i64.const 0x2))
  (v128.const i64x2 0xffff_ffff_0000_0000 0xffff_ffff_0000_0000)
)
(assert_return
  (invoke "as-f64x2_eq-operands" (f64.const 0x0p+0) (f64.const -0x0p+0))
  (v128.const i64x2 0xffff_ffff_ffff_ffff 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "as-f32x4_abs-operand" (f32.const -0x1.2p+0))
  (v128.const f32x4 0x1.2p+0 0x1.2p+0 0x1.2p+0 0x1.2p+0)
)
(assert_return
  (invoke "as-f32x4_min-operands"
    (f32.const 0x1p-2)
    (f32.const 0x1.b38f_b8p-127)
  )
  (v128.const f32x4
    0x1.b38f_b8p-127
    0x1.b38f_b8p-127
    0x1.b38f_b8p-127
    0x1.b38f_b8p-127
  )
)
(assert_return
  (invoke "as-f32x4_div-operands" (f32.const 0x1p+0) (f32.const 0x1p+3))
  (v128.const f32x4 0x1p-3 0x1p-3 0x1p-3 0x1p-3)
)
(assert_return
  (invoke "as-f32x4_convert_s_i32x4-operand" (i32.const 0x3039))
  (v128.const f32x4 0x1.81c8p+13 0x1.81c8p+13 0x1.81c8p+13 0x1.81c8p+13)
)
(assert_return
  (invoke "as-i32x4_trunc_s_f32x4_sat-operand" (f32.const 0x1.1999_9ap+0))
  (v128.const i32x4 0x1 0x1 0x1 0x1)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\95\80\80\80\00\04\60"
  "\01\7f\01\7b\60\01\7d\01\7b\60\01\7e\01\7b\60\01"
  "\7c\01\7b\03\89\80\80\80\00\08\00\00\00\01\02\02"
  "\02\03\06\96\80\80\80\00\01\7b\01\fd\0c\00\00\00"
  "\00\00\00\00\00\00\00\00\00\00\00\00\00\0b\07\9f"
  "\81\80\80\00\08\0c\61\73\2d\62\72\2d\76\61\6c\75"
  "\65\31\00\00\10\61\73\2d\72\65\74\75\72\6e\2d\76"
  "\61\6c\75\65\31\00\01\13\61\73\2d\6c\6f\63\61\6c"
  "\5f\73\65\74\2d\76\61\6c\75\65\31\00\02\14\61\73"
  "\2d\67\6c\6f\62\61\6c\5f\73\65\74\2d\76\61\6c\75"
  "\65\31\00\03\0c\61\73\2d\62\72\2d\76\61\6c\75\65"
  "\32\00\04\10\61\73\2d\72\65\74\75\72\6e\2d\76\61"
  "\6c\75\65\32\00\05\13\61\73\2d\6c\6f\63\61\6c\5f"
  "\73\65\74\2d\76\61\6c\75\65\32\00\06\14\61\73\2d"
  "\67\6c\6f\62\61\6c\5f\73\65\74\2d\76\61\6c\75\65"
  "\32\00\07\0a\fd\80\80\80\00\08\8b\80\80\80\00\00"
  "\02\7b\20\00\fd\0f\0c\00\0b\0b\87\80\80\80\00\00"
  "\20\00\fd\10\0f\0b\8d\80\80\80\00\01\01\7b\20\00"
  "\fd\11\21\01\20\01\0f\0b\8b\80\80\80\00\00\20\00"
  "\fd\13\24\00\23\00\0f\0b\8b\80\80\80\00\00\02\7b"
  "\20\00\fd\12\0c\00\0b\0b\87\80\80\80\00\00\20\00"
  "\fd\12\0f\0b\8d\80\80\80\00\01\01\7b\20\00\fd\12"
  "\21\01\20\01\0f\0b\8b\80\80\80\00\00\20\00\fd\14"
  "\24\00\23\00\0f\0b"
)
(module instance)
(assert_return
  (invoke "as-br-value1" (i32.const 0xab))
  (v128.const i8x16
    0xab
    0xab
    0xab
    0xab
    0xab
    0xab
    0xab
    0xab
    0xab
    0xab
    0xab
    0xab
    0xab
    0xab
    0xab
    0xab
  )
)
(assert_return
  (invoke "as-return-value1" (i32.const 0xabcd))
  (v128.const i16x8 0xabcd 0xabcd 0xabcd 0xabcd 0xabcd 0xabcd 0xabcd 0xabcd)
)
(assert_return
  (invoke "as-local_set-value1" (i32.const 0x1_0000))
  (v128.const i32x4 0x1_0000 0x1_0000 0x1_0000 0x1_0000)
)
(assert_return
  (invoke "as-global_set-value1" (f32.const 0x1p+0))
  (v128.const f32x4 0x1p+0 0x1p+0 0x1p+0 0x1p+0)
)
(assert_return
  (invoke "as-br-value2" (i64.const 0xabcd))
  (v128.const i64x2 0xabcd 0xabcd)
)
(assert_return
  (invoke "as-return-value2" (i64.const 0xabcd))
  (v128.const i64x2 0xabcd 0xabcd)
)
(assert_return
  (invoke "as-local_set-value2" (i64.const 0x1_0000))
  (v128.const i64x2 0x1_0000 0x1_0000)
)
(assert_return
  (invoke "as-global_set-value2" (f64.const 0x1p+0))
  (v128.const f64x2 0x1p+0 0x1p+0)
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8a\80\80\80"
    "\00\01\84\80\80\80\00\00\fd\0f\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8a\80\80\80"
    "\00\01\84\80\80\80\00\00\fd\10\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8a\80\80\80"
    "\00\01\84\80\80\80\00\00\fd\11\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8a\80\80\80"
    "\00\01\84\80\80\80\00\00\fd\13\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8a\80\80\80"
    "\00\01\84\80\80\80\00\00\fd\12\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8a\80\80\80"
    "\00\01\84\80\80\80\00\00\fd\14\0b"
  )
  "type mismatch"
)
