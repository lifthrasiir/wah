(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8e\80\80\80\00\02\60"
  "\02\7b\7b\01\7b\60\03\7b\7b\7b\01\7b\03\85\80\80"
  "\80\00\04\00\01\00\01\07\99\81\80\80\00\04\1f\69"
  "\31\36\78\38\2e\72\65\6c\61\78\65\64\5f\64\6f\74"
  "\5f\69\38\78\31\36\5f\69\37\78\31\36\5f\73\00\00"
  "\23\69\33\32\78\34\2e\72\65\6c\61\78\65\64\5f\64"
  "\6f\74\5f\69\38\78\31\36\5f\69\37\78\31\36\5f\61"
  "\64\64\5f\73\00\01\23\69\31\36\78\38\2e\72\65\6c"
  "\61\78\65\64\5f\64\6f\74\5f\69\38\78\31\36\5f\69"
  "\37\78\31\36\5f\73\5f\63\6d\70\00\02\27\69\33\32"
  "\78\34\2e\72\65\6c\61\78\65\64\5f\64\6f\74\5f\69"
  "\38\78\31\36\5f\69\37\78\31\36\5f\61\64\64\5f\73"
  "\5f\63\6d\70\00\03\0a\d1\80\80\80\00\04\89\80\80"
  "\80\00\00\20\00\20\01\fd\92\02\0b\8b\80\80\80\00"
  "\00\20\00\20\01\20\02\fd\93\02\0b\92\80\80\80\00"
  "\00\20\00\20\01\fd\92\02\20\00\20\01\fd\92\02\fd"
  "\2d\0b\96\80\80\80\00\00\20\00\20\01\20\02\fd\93"
  "\02\20\00\20\01\20\02\fd\93\02\fd\2d\0b"
)
(module instance)
(assert_return
  (invoke "i16x8.relaxed_dot_i8x16_i7x16_s"
    (v128.const i32x4 0x302_0100 0x706_0504 0xb0a_0908 0xf0e_0d0c)
    (v128.const i32x4 0x302_0100 0x706_0504 0xb0a_0908 0xf0e_0d0c)
  )
  (v128.const i16x8 0x1 0xd 0x29 0x55 0x91 0xdd 0x139 0x1a5)
)
(assert_return
  (invoke "i16x8.relaxed_dot_i8x16_i7x16_s"
    (v128.const i32x4 0x7f7f_8080 0x0 0x0 0x0)
    (v128.const i32x4 0x7f7f_7f7f 0x0 0x0 0x0)
  )
  (v128.const i16x8 0x8100 0x7e02 0x0 0x0 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "i16x8.relaxed_dot_i8x16_i7x16_s"
    (v128.const i32x4 0x8080 0x0 0x0 0x0)
    (v128.const i32x4 0x8181 0x0 0x0 0x0)
  )
  (either
    (v128.const i16x8 0x8000 0x0 0x0 0x0 0x0 0x0 0x0 0x0)
    (v128.const i16x8 0x7f00 0x0 0x0 0x0 0x0 0x0 0x0 0x0)
    (v128.const i16x8 0x8100 0x0 0x0 0x0 0x0 0x0 0x0 0x0)
  )
)
(assert_return
  (invoke "i32x4.relaxed_dot_i8x16_i7x16_add_s"
    (v128.const i32x4 0x302_0100 0x706_0504 0xb0a_0908 0xf0e_0d0c)
    (v128.const i32x4 0x302_0100 0x706_0504 0xb0a_0908 0xf0e_0d0c)
    (v128.const i32x4 0x0 0x1 0x2 0x3)
  )
  (v128.const i32x4 0xe 0x7f 0x170 0x2e1)
)
(assert_return
  (invoke "i32x4.relaxed_dot_i8x16_i7x16_add_s"
    (v128.const i32x4 0x8080_8080 0x7f7f_7f7f 0x0 0x0)
    (v128.const i32x4 0x7f7f_7f7f 0x7f7f_7f7f 0x0 0x0)
    (v128.const i32x4 0x1 0x2 0x3 0x4)
  )
  (v128.const i32x4 0xffff_0201 0xfc06 0x3 0x4)
)
(assert_return
  (invoke "i32x4.relaxed_dot_i8x16_i7x16_add_s"
    (v128.const i32x4 0x8080_8080 0x0 0x0 0x0)
    (v128.const i32x4 0x8181_8181 0x0 0x0 0x0)
    (v128.const i32x4 0x1 0x2 0x3 0x4)
  )
  (either
    (v128.const i32x4 0xfffe_fe01 0x2 0x3 0x4)
    (v128.const i32x4 0xffff_0001 0x2 0x3 0x4)
    (v128.const i32x4 0xfe01 0x2 0x3 0x4)
    (v128.const i32x4 0x1_0201 0x2 0x3 0x4)
  )
)
(assert_return
  (invoke "i16x8.relaxed_dot_i8x16_i7x16_s_cmp"
    (v128.const i32x4 0x7f7f_8080 0x0 0x0 0x0)
    (v128.const i32x4 0x7f7f_7f7f 0x0 0x0 0x0)
  )
  (v128.const i16x8 0xffff 0xffff 0xffff 0xffff 0xffff 0xffff 0xffff 0xffff)
)
(assert_return
  (invoke "i32x4.relaxed_dot_i8x16_i7x16_add_s_cmp"
    (v128.const i32x4 0x8080_8080 0x7f7f_7f7f 0x0 0x0)
    (v128.const i32x4 0x7f7f_7f7f 0x7f7f_7f7f 0x0 0x0)
    (v128.const i32x4 0x1 0x2 0x3 0x4)
  )
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "i16x8.relaxed_dot_i8x16_i7x16_s_cmp"
    (v128.const i32x4 0x8080 0x0 0x0 0x0)
    (v128.const i32x4 0x8181 0x0 0x0 0x0)
  )
  (v128.const i16x8 0xffff 0xffff 0xffff 0xffff 0xffff 0xffff 0xffff 0xffff)
)
(assert_return
  (invoke "i32x4.relaxed_dot_i8x16_i7x16_add_s_cmp"
    (v128.const i32x4 0x8080_8080 0x0 0x0 0x0)
    (v128.const i32x4 0x8181_8181 0x0 0x0 0x0)
    (v128.const i32x4 0x1 0x2 0x3 0x4)
  )
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
