(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8e\80\80\80\00\03\60"
  "\01\7f\01\7b\60\00\01\7b\60\01\7f\00\03\8d\80\80"
  "\80\00\0c\00\00\00\00\00\01\01\01\01\01\01\02\05"
  "\83\80\80\80\00\01\00\01\07\b0\81\80\80\00\0c\0b"
  "\6c\6f\61\64\5f\64\61\74\61\5f\31\00\00\0b\6c\6f"
  "\61\64\5f\64\61\74\61\5f\32\00\01\0b\6c\6f\61\64"
  "\5f\64\61\74\61\5f\33\00\02\0b\6c\6f\61\64\5f\64"
  "\61\74\61\5f\34\00\03\0b\6c\6f\61\64\5f\64\61\74"
  "\61\5f\35\00\04\0c\73\74\6f\72\65\5f\64\61\74\61"
  "\5f\30\00\05\0c\73\74\6f\72\65\5f\64\61\74\61\5f"
  "\31\00\06\0c\73\74\6f\72\65\5f\64\61\74\61\5f\32"
  "\00\07\0c\73\74\6f\72\65\5f\64\61\74\61\5f\33\00"
  "\08\0c\73\74\6f\72\65\5f\64\61\74\61\5f\34\00\09"
  "\0c\73\74\6f\72\65\5f\64\61\74\61\5f\35\00\0a\0c"
  "\73\74\6f\72\65\5f\64\61\74\61\5f\36\00\0b\0a\c3"
  "\82\80\80\00\0c\88\80\80\80\00\00\20\00\fd\00\04"
  "\00\0b\88\80\80\80\00\00\20\00\fd\00\00\00\0b\88"
  "\80\80\80\00\00\20\00\fd\00\00\01\0b\88\80\80\80"
  "\00\00\20\00\fd\00\00\02\0b\88\80\80\80\00\00\20"
  "\00\fd\00\00\0f\0b\a0\80\80\80\00\00\41\00\fd\0c"
  "\00\00\00\00\00\00\80\3f\00\00\00\40\00\00\40\40"
  "\fd\0b\04\00\41\00\fd\00\04\00\0b\a0\80\80\80\00"
  "\00\41\00\fd\0c\00\00\00\00\01\00\00\00\02\00\00"
  "\00\03\00\00\00\fd\0b\00\00\41\00\fd\00\00\00\0b"
  "\a0\80\80\80\00\00\41\00\fd\0c\00\00\01\00\02\00"
  "\03\00\04\00\05\00\06\00\07\00\fd\0b\00\01\41\00"
  "\fd\00\00\01\0b\a0\80\80\80\00\00\41\00\fd\0c\00"
  "\01\02\03\04\05\06\07\08\09\0a\0b\0c\0d\0e\0f\fd"
  "\0b\00\02\41\00\fd\00\00\02\0b\a0\80\80\80\00\00"
  "\41\00\fd\0c\00\00\00\00\01\00\00\00\02\00\00\00"
  "\03\00\00\00\fd\0b\00\0f\41\00\fd\00\04\0f\0b\a4"
  "\80\80\80\00\00\41\00\fd\0c\00\00\00\00\01\00\00"
  "\00\02\00\00\00\03\00\00\00\fd\0b\00\f0\ff\03\41"
  "\00\fd\00\04\f0\ff\03\0b\9a\80\80\80\00\00\20\00"
  "\fd\0c\00\00\00\00\01\00\00\00\02\00\00\00\03\00"
  "\00\00\fd\0b\00\01\0b\0b\ad\80\80\80\00\02\00\41"
  "\00\0b\10\00\01\02\03\04\05\06\07\08\09\10\11\12"
  "\13\14\15\00\41\e1\ff\03\0b\10\16\17\18\19\20\21"
  "\22\23\24\25\26\27\28\29\30\31"
)
(module instance)
(assert_return
  (invoke "load_data_1" (i32.const 0x0))
  (v128.const i32x4 0x302_0100 0x706_0504 0x1110_0908 0x1514_1312)
)
(assert_return
  (invoke "load_data_2" (i32.const 0x0))
  (v128.const i32x4 0x302_0100 0x706_0504 0x1110_0908 0x1514_1312)
)
(assert_return
  (invoke "load_data_3" (i32.const 0x0))
  (v128.const i32x4 0x403_0201 0x807_0605 0x1211_1009 0x15_1413)
)
(assert_return
  (invoke "load_data_4" (i32.const 0x0))
  (v128.const i32x4 0x504_0302 0x908_0706 0x1312_1110 0x1514)
)
(assert_return
  (invoke "load_data_5" (i32.const 0x0))
  (v128.const i32x4 0x15 0x0 0x0 0x0)
)
(assert_return
  (invoke "load_data_1" (i32.const 0x0))
  (v128.const i16x8 0x100 0x302 0x504 0x706 0x908 0x1110 0x1312 0x1514)
)
(assert_return
  (invoke "load_data_2" (i32.const 0x0))
  (v128.const i16x8 0x100 0x302 0x504 0x706 0x908 0x1110 0x1312 0x1514)
)
(assert_return
  (invoke "load_data_3" (i32.const 0x0))
  (v128.const i16x8 0x201 0x403 0x605 0x807 0x1009 0x1211 0x1413 0x15)
)
(assert_return
  (invoke "load_data_4" (i32.const 0x0))
  (v128.const i16x8 0x302 0x504 0x706 0x908 0x1110 0x1312 0x1514 0x0)
)
(assert_return
  (invoke "load_data_5" (i32.const 0x0))
  (v128.const i16x8 0x15 0x0 0x0 0x0 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "load_data_1" (i32.const 0x0))
  (v128.const i8x16
    0x0
    0x1
    0x2
    0x3
    0x4
    0x5
    0x6
    0x7
    0x8
    0x9
    0x10
    0x11
    0x12
    0x13
    0x14
    0x15
  )
)
(assert_return
  (invoke "load_data_2" (i32.const 0x0))
  (v128.const i8x16
    0x0
    0x1
    0x2
    0x3
    0x4
    0x5
    0x6
    0x7
    0x8
    0x9
    0x10
    0x11
    0x12
    0x13
    0x14
    0x15
  )
)
(assert_return
  (invoke "load_data_3" (i32.const 0x0))
  (v128.const i8x16
    0x1
    0x2
    0x3
    0x4
    0x5
    0x6
    0x7
    0x8
    0x9
    0x10
    0x11
    0x12
    0x13
    0x14
    0x15
    0x0
  )
)
(assert_return
  (invoke "load_data_4" (i32.const 0x0))
  (v128.const i8x16
    0x2
    0x3
    0x4
    0x5
    0x6
    0x7
    0x8
    0x9
    0x10
    0x11
    0x12
    0x13
    0x14
    0x15
    0x0
    0x0
  )
)
(assert_return
  (invoke "load_data_5" (i32.const 0x0))
  (v128.const i8x16
    0x15
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
  (invoke "load_data_1" (i32.const 0xffe1))
  (v128.const i32x4 0x1918_1716 0x2322_2120 0x2726_2524 0x3130_2928)
)
(assert_return
  (invoke "load_data_2" (i32.const 0xffe1))
  (v128.const i32x4 0x1918_1716 0x2322_2120 0x2726_2524 0x3130_2928)
)
(assert_return
  (invoke "load_data_3" (i32.const 0xffe1))
  (v128.const i32x4 0x2019_1817 0x2423_2221 0x2827_2625 0x31_3029)
)
(assert_return
  (invoke "load_data_4" (i32.const 0xffe1))
  (v128.const i32x4 0x2120_1918 0x2524_2322 0x2928_2726 0x3130)
)
(assert_return
  (invoke "load_data_5" (i32.const 0xffe1))
  (v128.const i32x4 0x31 0x0 0x0 0x0)
)
(assert_return
  (invoke "load_data_1" (i32.const 0xffe1))
  (v128.const i16x8 0x1716 0x1918 0x2120 0x2322 0x2524 0x2726 0x2928 0x3130)
)
(assert_return
  (invoke "load_data_2" (i32.const 0xffe1))
  (v128.const i16x8 0x1716 0x1918 0x2120 0x2322 0x2524 0x2726 0x2928 0x3130)
)
(assert_return
  (invoke "load_data_3" (i32.const 0xffe1))
  (v128.const i16x8 0x1817 0x2019 0x2221 0x2423 0x2625 0x2827 0x3029 0x31)
)
(assert_return
  (invoke "load_data_4" (i32.const 0xffe1))
  (v128.const i16x8 0x1918 0x2120 0x2322 0x2524 0x2726 0x2928 0x3130 0x0)
)
(assert_return
  (invoke "load_data_5" (i32.const 0xffe1))
  (v128.const i16x8 0x31 0x0 0x0 0x0 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "load_data_1" (i32.const 0xffe1))
  (v128.const i8x16
    0x16
    0x17
    0x18
    0x19
    0x20
    0x21
    0x22
    0x23
    0x24
    0x25
    0x26
    0x27
    0x28
    0x29
    0x30
    0x31
  )
)
(assert_return
  (invoke "load_data_2" (i32.const 0xffe1))
  (v128.const i8x16
    0x16
    0x17
    0x18
    0x19
    0x20
    0x21
    0x22
    0x23
    0x24
    0x25
    0x26
    0x27
    0x28
    0x29
    0x30
    0x31
  )
)
(assert_return
  (invoke "load_data_3" (i32.const 0xffe1))
  (v128.const i8x16
    0x17
    0x18
    0x19
    0x20
    0x21
    0x22
    0x23
    0x24
    0x25
    0x26
    0x27
    0x28
    0x29
    0x30
    0x31
    0x0
  )
)
(assert_return
  (invoke "load_data_4" (i32.const 0xffe1))
  (v128.const i8x16
    0x18
    0x19
    0x20
    0x21
    0x22
    0x23
    0x24
    0x25
    0x26
    0x27
    0x28
    0x29
    0x30
    0x31
    0x0
    0x0
  )
)
(assert_return
  (invoke "load_data_5" (i32.const 0xffe1))
  (v128.const i8x16
    0x31
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
(assert_trap
  (invoke "load_data_3" (i32.const 0xffff_ffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "load_data_5" (i32.const 0xffe2))
  "out of bounds memory access"
)
(assert_return
  (invoke "store_data_0")
  (v128.const f32x4 0x0p+0 0x1p+0 0x1p+1 0x1.8p+1)
)
(assert_return (invoke "store_data_1") (v128.const i32x4 0x0 0x1 0x2 0x3))
(assert_return
  (invoke "store_data_2")
  (v128.const i16x8 0x0 0x1 0x2 0x3 0x4 0x5 0x6 0x7)
)
(assert_return
  (invoke "store_data_3")
  (v128.const i8x16
    0x0
    0x1
    0x2
    0x3
    0x4
    0x5
    0x6
    0x7
    0x8
    0x9
    0xa
    0xb
    0xc
    0xd
    0xe
    0xf
  )
)
(assert_return (invoke "store_data_4") (v128.const i32x4 0x0 0x1 0x2 0x3))
(assert_return (invoke "store_data_5") (v128.const i32x4 0x0 0x1 0x2 0x3))
(assert_trap
  (invoke "store_data_6" (i32.const 0xffff_ffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "store_data_6" (i32.const 0xffff))
  "out of bounds memory access"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\03\82\80\80\80\00\01\00\05\83\80\80\80\00"
  "\01\00\01\07\9a\80\80\80\00\01\16\76\31\32\38\2e"
  "\6c\6f\61\64\5f\6f\66\66\73\65\74\5f\36\35\35\32"
  "\31\00\00\0a\91\80\80\80\00\01\8b\80\80\80\00\00"
  "\41\00\fd\00\04\f1\ff\03\1a\0b"
)
(module instance)
(assert_trap (invoke "v128.load_offset_65521") "out of bounds memory access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\03\82\80\80\80\00\01\00\05\83\80\80\80\00"
  "\01\00\01\07\9b\80\80\80\00\01\17\76\31\32\38\2e"
  "\73\74\6f\72\65\5f\6f\66\66\73\65\74\5f\36\35\35"
  "\32\31\00\00\0a\a2\80\80\80\00\01\9c\80\80\80\00"
  "\00\41\00\fd\0c\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\00\fd\0b\04\f1\ff\03\0b"
)
(module instance)
(assert_trap (invoke "v128.store_offset_65521") "out of bounds memory access")
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\05\83\80\80\80\00"
    "\01\00\01\0a\93\80\80\80\00\01\8d\80\80\80\00\00"
    "\41\00\fd\00\04\80\80\80\80\10\1a\0b"
  )
  "offset out of range"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\05\83\80\80\80\00"
    "\01\00\01\0a\a4\80\80\80\00\01\9e\80\80\80\00\00"
    "\41\00\fd\0c\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\fd\0b\04\80\80\80\80\10\0b"
  )
  "offset out of range"
)
