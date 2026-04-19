(module definition binary
  "\00\61\73\6d\01\00\00\00\01\a4\80\80\80\00\07\60"
  "\02\7e\7f\00\60\02\7e\7e\00\60\01\7e\01\7f\60\01"
  "\7e\01\7e\60\01\7f\01\7f\60\01\7d\01\7d\60\01\7c"
  "\01\7c\03\98\80\80\80\00\17\00\00\01\02\02\03\04"
  "\04\04\03\03\03\03\03\05\06\04\04\03\03\03\05\06"
  "\05\83\80\80\80\00\01\04\01\07\e1\81\80\80\00\11"
  "\0c\69\33\32\5f\6c\6f\61\64\31\36\5f\73\00\06\0c"
  "\69\33\32\5f\6c\6f\61\64\31\36\5f\75\00\07\08\69"
  "\33\32\5f\6c\6f\61\64\00\08\0c\69\36\34\5f\6c\6f"
  "\61\64\31\36\5f\73\00\09\0c\69\36\34\5f\6c\6f\61"
  "\64\31\36\5f\75\00\0a\0c\69\36\34\5f\6c\6f\61\64"
  "\33\32\5f\73\00\0b\0c\69\36\34\5f\6c\6f\61\64\33"
  "\32\5f\75\00\0c\08\69\36\34\5f\6c\6f\61\64\00\0d"
  "\08\66\33\32\5f\6c\6f\61\64\00\0e\08\66\36\34\5f"
  "\6c\6f\61\64\00\0f\0b\69\33\32\5f\73\74\6f\72\65"
  "\31\36\00\10\09\69\33\32\5f\73\74\6f\72\65\00\11"
  "\0b\69\36\34\5f\73\74\6f\72\65\31\36\00\12\0b\69"
  "\36\34\5f\73\74\6f\72\65\33\32\00\13\09\69\36\34"
  "\5f\73\74\6f\72\65\00\14\09\66\33\32\5f\73\74\6f"
  "\72\65\00\15\09\66\36\34\5f\73\74\6f\72\65\00\16"
  "\0a\d2\83\80\80\00\17\96\80\80\80\00\00\20\00\20"
  "\01\3a\00\00\20\00\42\01\7c\20\01\41\08\76\3a\00"
  "\00\0b\94\80\80\80\00\00\20\00\20\01\10\00\20\00"
  "\42\02\7c\20\01\41\10\76\10\00\0b\96\80\80\80\00"
  "\00\20\00\20\01\a7\10\01\20\00\42\04\7c\20\01\42"
  "\20\88\a7\10\01\0b\93\80\80\80\00\00\20\00\2d\00"
  "\00\20\00\42\01\7c\2d\00\00\41\08\74\72\0b\91\80"
  "\80\80\00\00\20\00\10\03\20\00\42\02\7c\10\03\41"
  "\10\74\72\0b\93\80\80\80\00\00\20\00\10\04\ad\20"
  "\00\42\04\7c\10\04\ad\42\20\86\84\0b\8d\80\80\80"
  "\00\00\42\00\20\00\10\00\42\00\2e\01\00\0b\8d\80"
  "\80\80\00\00\42\00\20\00\10\00\42\00\2f\01\00\0b"
  "\8d\80\80\80\00\00\42\00\20\00\10\01\42\00\28\02"
  "\00\0b\8e\80\80\80\00\00\42\00\20\00\a7\10\00\42"
  "\00\32\01\00\0b\8e\80\80\80\00\00\42\00\20\00\a7"
  "\10\00\42\00\33\01\00\0b\8e\80\80\80\00\00\42\00"
  "\20\00\a7\10\01\42\00\34\02\00\0b\8e\80\80\80\00"
  "\00\42\00\20\00\a7\10\01\42\00\35\02\00\0b\8d\80"
  "\80\80\00\00\42\00\20\00\10\02\42\00\29\03\00\0b"
  "\8e\80\80\80\00\00\42\00\20\00\bc\10\01\42\00\2a"
  "\02\00\0b\8e\80\80\80\00\00\42\00\20\00\bd\10\02"
  "\42\00\2b\03\00\0b\8d\80\80\80\00\00\42\00\20\00"
  "\3b\01\00\42\00\10\03\0b\8d\80\80\80\00\00\42\00"
  "\20\00\36\02\00\42\00\10\04\0b\8e\80\80\80\00\00"
  "\42\00\20\00\3d\01\00\42\00\10\03\ad\0b\8e\80\80"
  "\80\00\00\42\00\20\00\3e\02\00\42\00\10\04\ad\0b"
  "\8d\80\80\80\00\00\42\00\20\00\37\03\00\42\00\10"
  "\05\0b\8e\80\80\80\00\00\42\00\20\00\38\02\00\42"
  "\00\10\04\be\0b\8e\80\80\80\00\00\42\00\20\00\39"
  "\03\00\42\00\10\05\bf\0b"
)
(module instance)
(assert_return
  (invoke "i32_load16_s" (i32.const 0xffff_ffff))
  (i32.const 0xffff_ffff)
)
(assert_return
  (invoke "i32_load16_s" (i32.const 0xffff_ef6e))
  (i32.const 0xffff_ef6e)
)
(assert_return (invoke "i32_load16_s" (i32.const 0x2a)) (i32.const 0x2a))
(assert_return (invoke "i32_load16_s" (i32.const 0x3210)) (i32.const 0x3210))
(assert_return
  (invoke "i32_load16_u" (i32.const 0xffff_ffff))
  (i32.const 0xffff)
)
(assert_return
  (invoke "i32_load16_u" (i32.const 0xffff_ef6e))
  (i32.const 0xef6e)
)
(assert_return (invoke "i32_load16_u" (i32.const 0x2a)) (i32.const 0x2a))
(assert_return (invoke "i32_load16_u" (i32.const 0xcafe)) (i32.const 0xcafe))
(assert_return
  (invoke "i32_load" (i32.const 0xffff_ffff))
  (i32.const 0xffff_ffff)
)
(assert_return
  (invoke "i32_load" (i32.const 0xfd78_a84e))
  (i32.const 0xfd78_a84e)
)
(assert_return
  (invoke "i32_load" (i32.const 0x287_57b2))
  (i32.const 0x287_57b2)
)
(assert_return
  (invoke "i32_load" (i32.const 0xabad_1dea))
  (i32.const 0xabad_1dea)
)
(assert_return
  (invoke "i64_load16_s" (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "i64_load16_s" (i64.const 0xffff_ffff_ffff_ef6e))
  (i64.const 0xffff_ffff_ffff_ef6e)
)
(assert_return (invoke "i64_load16_s" (i64.const 0x2a)) (i64.const 0x2a))
(assert_return (invoke "i64_load16_s" (i64.const 0x3210)) (i64.const 0x3210))
(assert_return
  (invoke "i64_load16_u" (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0xffff)
)
(assert_return
  (invoke "i64_load16_u" (i64.const 0xffff_ffff_ffff_ef6e))
  (i64.const 0xef6e)
)
(assert_return (invoke "i64_load16_u" (i64.const 0x2a)) (i64.const 0x2a))
(assert_return (invoke "i64_load16_u" (i64.const 0xcafe)) (i64.const 0xcafe))
(assert_return
  (invoke "i64_load32_s" (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "i64_load32_s" (i64.const 0xffff_ffff_fd78_a84e))
  (i64.const 0xffff_ffff_fd78_a84e)
)
(assert_return
  (invoke "i64_load32_s" (i64.const 0x287_57b2))
  (i64.const 0x287_57b2)
)
(assert_return
  (invoke "i64_load32_s" (i64.const 0x1234_5678))
  (i64.const 0x1234_5678)
)
(assert_return
  (invoke "i64_load32_u" (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0xffff_ffff)
)
(assert_return
  (invoke "i64_load32_u" (i64.const 0xffff_ffff_fd78_a84e))
  (i64.const 0xfd78_a84e)
)
(assert_return
  (invoke "i64_load32_u" (i64.const 0x287_57b2))
  (i64.const 0x287_57b2)
)
(assert_return
  (invoke "i64_load32_u" (i64.const 0xabad_1dea))
  (i64.const 0xabad_1dea)
)
(assert_return
  (invoke "i64_load" (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "i64_load" (i64.const 0xffff_ffff_fd78_a84e))
  (i64.const 0xffff_ffff_fd78_a84e)
)
(assert_return
  (invoke "i64_load" (i64.const 0xabad_1dea))
  (i64.const 0xabad_1dea)
)
(assert_return
  (invoke "i64_load" (i64.const 0xabad_cafe_dead_1dea))
  (i64.const 0xabad_cafe_dead_1dea)
)
(assert_return (invoke "f32_load" (f32.const -0x1p+0)) (f32.const -0x1p+0))
(assert_return
  (invoke "f32_load" (f32.const 0x1.945b_6cp-7))
  (f32.const 0x1.945b_6cp-7)
)
(assert_return
  (invoke "f32_load" (f32.const 0x1.0926_cap+12))
  (f32.const 0x1.0926_cap+12)
)
(assert_return
  (invoke "f32_load" (f32.const 0x1.ffff_fep+127))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return (invoke "f64_load" (f64.const -0x1p+0)) (f64.const -0x1p+0))
(assert_return
  (invoke "f64_load" (f64.const 0x1.34a4_584f_4c6e_7p+10))
  (f64.const 0x1.34a4_584f_4c6e_7p+10)
)
(assert_return
  (invoke "f64_load" (f64.const 0x1.9e4c_9b26_c7ea_ep+18))
  (f64.const 0x1.9e4c_9b26_c7ea_ep+18)
)
(assert_return
  (invoke "f64_load" (f64.const 0x1.ffff_ffff_ffff_fp+1023))
  (f64.const 0x1.ffff_ffff_ffff_fp+1023)
)
(assert_return
  (invoke "i32_store16" (i32.const 0xffff_ffff))
  (i32.const 0xffff)
)
(assert_return
  (invoke "i32_store16" (i32.const 0xffff_ef6e))
  (i32.const 0xef6e)
)
(assert_return (invoke "i32_store16" (i32.const 0x2a)) (i32.const 0x2a))
(assert_return (invoke "i32_store16" (i32.const 0xcafe)) (i32.const 0xcafe))
(assert_return
  (invoke "i32_store" (i32.const 0xffff_ffff))
  (i32.const 0xffff_ffff)
)
(assert_return
  (invoke "i32_store" (i32.const 0xffff_ef6e))
  (i32.const 0xffff_ef6e)
)
(assert_return
  (invoke "i32_store" (i32.const 0x287_57b2))
  (i32.const 0x287_57b2)
)
(assert_return
  (invoke "i32_store" (i32.const 0xdead_cafe))
  (i32.const 0xdead_cafe)
)
(assert_return
  (invoke "i64_store16" (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0xffff)
)
(assert_return
  (invoke "i64_store16" (i64.const 0xffff_ffff_ffff_ef6e))
  (i64.const 0xef6e)
)
(assert_return (invoke "i64_store16" (i64.const 0x2a)) (i64.const 0x2a))
(assert_return (invoke "i64_store16" (i64.const 0xcafe)) (i64.const 0xcafe))
(assert_return
  (invoke "i64_store32" (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0xffff_ffff)
)
(assert_return
  (invoke "i64_store32" (i64.const 0xffff_ffff_ffff_ef6e))
  (i64.const 0xffff_ef6e)
)
(assert_return
  (invoke "i64_store32" (i64.const 0x287_57b2))
  (i64.const 0x287_57b2)
)
(assert_return
  (invoke "i64_store32" (i64.const 0xdead_cafe))
  (i64.const 0xdead_cafe)
)
(assert_return
  (invoke "i64_store" (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "i64_store" (i64.const 0xffff_ffff_fd78_a84e))
  (i64.const 0xffff_ffff_fd78_a84e)
)
(assert_return
  (invoke "i64_store" (i64.const 0xabad_1dea))
  (i64.const 0xabad_1dea)
)
(assert_return
  (invoke "i64_store" (i64.const 0xabad_cafe_dead_1dea))
  (i64.const 0xabad_cafe_dead_1dea)
)
(assert_return (invoke "f32_store" (f32.const -0x1p+0)) (f32.const -0x1p+0))
(assert_return
  (invoke "f32_store" (f32.const 0x1.945b_6cp-7))
  (f32.const 0x1.945b_6cp-7)
)
(assert_return
  (invoke "f32_store" (f32.const 0x1.0926_cap+12))
  (f32.const 0x1.0926_cap+12)
)
(assert_return
  (invoke "f32_store" (f32.const 0x1.ffff_fep+127))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return (invoke "f64_store" (f64.const -0x1p+0)) (f64.const -0x1p+0))
(assert_return
  (invoke "f64_store" (f64.const 0x1.34a4_584f_4c6e_7p+10))
  (f64.const 0x1.34a4_584f_4c6e_7p+10)
)
(assert_return
  (invoke "f64_store" (f64.const 0x1.9e4c_9b26_c7ea_ep+18))
  (f64.const 0x1.9e4c_9b26_c7ea_ep+18)
)
(assert_return
  (invoke "f64_store" (f64.const 0x1.ffff_ffff_ffff_fp+1023))
  (f64.const 0x1.ffff_ffff_ffff_fp+1023)
)
