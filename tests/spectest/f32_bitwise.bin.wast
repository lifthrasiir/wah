(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8c\80\80\80\00\02\60"
  "\01\7d\01\7d\60\02\7d\7d\01\7d\03\84\80\80\80\00"
  "\03\00\00\01\07\98\80\80\80\00\03\03\61\62\73\00"
  "\00\03\6e\65\67\00\01\08\63\6f\70\79\73\69\67\6e"
  "\00\02\0a\a1\80\80\80\00\03\85\80\80\80\00\00\20"
  "\00\8b\0b\85\80\80\80\00\00\20\00\8c\0b\87\80\80"
  "\80\00\00\20\00\20\01\98\0b"
)
(module instance)
(assert_return
  (invoke "copysign" (f32.const -0x0p+0) (f32.const -0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x0p+0) (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x0p+0) (f32.const -0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x0p+0) (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x0p+0) (f32.const -0x1p-149))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x0p+0) (f32.const 0x1p-149))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x0p+0) (f32.const -0x1p-149))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x0p+0) (f32.const 0x1p-149))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x0p+0) (f32.const -0x1p-126))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x0p+0) (f32.const 0x1p-126))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x0p+0) (f32.const -0x1p-126))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x0p+0) (f32.const 0x1p-126))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x0p+0) (f32.const -0x1p-1))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x0p+0) (f32.const 0x1p-1))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x0p+0) (f32.const -0x1p-1))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x0p+0) (f32.const 0x1p-1))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x0p+0) (f32.const -0x1p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x0p+0) (f32.const 0x1p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x0p+0) (f32.const -0x1p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x0p+0) (f32.const 0x1p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x0p+0) (f32.const -0x1.921f_b6p+2))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x0p+0) (f32.const 0x1.921f_b6p+2))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x0p+0) (f32.const -0x1.921f_b6p+2))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x0p+0) (f32.const 0x1.921f_b6p+2))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x0p+0) (f32.const -0x1.ffff_fep+127))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x0p+0) (f32.const 0x1.ffff_fep+127))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x0p+0) (f32.const -0x1.ffff_fep+127))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x0p+0) (f32.const 0x1.ffff_fep+127))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x0p+0) (f32.const -inf))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x0p+0) (f32.const inf))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x0p+0) (f32.const -inf))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x0p+0) (f32.const inf))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x0p+0) (f32.const -nan:0x40_0000))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x0p+0) (f32.const nan:0x40_0000))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x0p+0) (f32.const -nan:0x40_0000))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x0p+0) (f32.const nan:0x40_0000))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-149) (f32.const -0x0p+0))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-149) (f32.const 0x0p+0))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-149) (f32.const -0x0p+0))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-149) (f32.const 0x0p+0))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-149) (f32.const -0x1p-149))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-149) (f32.const 0x1p-149))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-149) (f32.const -0x1p-149))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-149) (f32.const 0x1p-149))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-149) (f32.const -0x1p-126))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-149) (f32.const 0x1p-126))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-149) (f32.const -0x1p-126))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-149) (f32.const 0x1p-126))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-149) (f32.const -0x1p-1))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-149) (f32.const 0x1p-1))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-149) (f32.const -0x1p-1))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-149) (f32.const 0x1p-1))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-149) (f32.const -0x1p+0))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-149) (f32.const 0x1p+0))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-149) (f32.const -0x1p+0))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-149) (f32.const 0x1p+0))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-149) (f32.const -0x1.921f_b6p+2))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-149) (f32.const 0x1.921f_b6p+2))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-149) (f32.const -0x1.921f_b6p+2))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-149) (f32.const 0x1.921f_b6p+2))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-149) (f32.const -0x1.ffff_fep+127))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-149) (f32.const 0x1.ffff_fep+127))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-149) (f32.const -0x1.ffff_fep+127))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-149) (f32.const 0x1.ffff_fep+127))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-149) (f32.const -inf))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-149) (f32.const inf))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-149) (f32.const -inf))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-149) (f32.const inf))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-149) (f32.const -nan:0x40_0000))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-149) (f32.const nan:0x40_0000))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-149) (f32.const -nan:0x40_0000))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-149) (f32.const nan:0x40_0000))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-126) (f32.const -0x0p+0))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-126) (f32.const 0x0p+0))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-126) (f32.const -0x0p+0))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-126) (f32.const 0x0p+0))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-126) (f32.const -0x1p-149))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-126) (f32.const 0x1p-149))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-126) (f32.const -0x1p-149))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-126) (f32.const 0x1p-149))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-126) (f32.const -0x1p-126))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-126) (f32.const 0x1p-126))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-126) (f32.const -0x1p-126))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-126) (f32.const 0x1p-126))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-126) (f32.const -0x1p-1))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-126) (f32.const 0x1p-1))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-126) (f32.const -0x1p-1))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-126) (f32.const 0x1p-1))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-126) (f32.const -0x1p+0))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-126) (f32.const 0x1p+0))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-126) (f32.const -0x1p+0))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-126) (f32.const 0x1p+0))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-126) (f32.const -0x1.921f_b6p+2))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-126) (f32.const 0x1.921f_b6p+2))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-126) (f32.const -0x1.921f_b6p+2))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-126) (f32.const 0x1.921f_b6p+2))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-126) (f32.const -0x1.ffff_fep+127))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-126) (f32.const 0x1.ffff_fep+127))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-126) (f32.const -0x1.ffff_fep+127))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-126) (f32.const 0x1.ffff_fep+127))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-126) (f32.const -inf))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-126) (f32.const inf))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-126) (f32.const -inf))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-126) (f32.const inf))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-126) (f32.const -nan:0x40_0000))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-126) (f32.const nan:0x40_0000))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-126) (f32.const -nan:0x40_0000))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-126) (f32.const nan:0x40_0000))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-1) (f32.const -0x0p+0))
  (f32.const -0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-1) (f32.const 0x0p+0))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-1) (f32.const -0x0p+0))
  (f32.const -0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-1) (f32.const 0x0p+0))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-1) (f32.const -0x1p-149))
  (f32.const -0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-1) (f32.const 0x1p-149))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-1) (f32.const -0x1p-149))
  (f32.const -0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-1) (f32.const 0x1p-149))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-1) (f32.const -0x1p-126))
  (f32.const -0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-1) (f32.const 0x1p-126))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-1) (f32.const -0x1p-126))
  (f32.const -0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-1) (f32.const 0x1p-126))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-1) (f32.const -0x1p-1))
  (f32.const -0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-1) (f32.const 0x1p-1))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-1) (f32.const -0x1p-1))
  (f32.const -0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-1) (f32.const 0x1p-1))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-1) (f32.const -0x1p+0))
  (f32.const -0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-1) (f32.const 0x1p+0))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-1) (f32.const -0x1p+0))
  (f32.const -0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-1) (f32.const 0x1p+0))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-1) (f32.const -0x1.921f_b6p+2))
  (f32.const -0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-1) (f32.const 0x1.921f_b6p+2))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-1) (f32.const -0x1.921f_b6p+2))
  (f32.const -0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-1) (f32.const 0x1.921f_b6p+2))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-1) (f32.const -0x1.ffff_fep+127))
  (f32.const -0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-1) (f32.const 0x1.ffff_fep+127))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-1) (f32.const -0x1.ffff_fep+127))
  (f32.const -0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-1) (f32.const 0x1.ffff_fep+127))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-1) (f32.const -inf))
  (f32.const -0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-1) (f32.const inf))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-1) (f32.const -inf))
  (f32.const -0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-1) (f32.const inf))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-1) (f32.const -nan:0x40_0000))
  (f32.const -0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p-1) (f32.const nan:0x40_0000))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-1) (f32.const -nan:0x40_0000))
  (f32.const -0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p-1) (f32.const nan:0x40_0000))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p+0) (f32.const -0x0p+0))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p+0) (f32.const 0x0p+0))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p+0) (f32.const -0x0p+0))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p+0) (f32.const 0x0p+0))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p+0) (f32.const -0x1p-149))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p+0) (f32.const 0x1p-149))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p+0) (f32.const -0x1p-149))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p+0) (f32.const 0x1p-149))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p+0) (f32.const -0x1p-126))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p+0) (f32.const 0x1p-126))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p+0) (f32.const -0x1p-126))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p+0) (f32.const 0x1p-126))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p+0) (f32.const -0x1p-1))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p+0) (f32.const 0x1p-1))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p+0) (f32.const -0x1p-1))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p+0) (f32.const 0x1p-1))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p+0) (f32.const -0x1p+0))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p+0) (f32.const 0x1p+0))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p+0) (f32.const -0x1p+0))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p+0) (f32.const 0x1p+0))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p+0) (f32.const -0x1.921f_b6p+2))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p+0) (f32.const 0x1.921f_b6p+2))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p+0) (f32.const -0x1.921f_b6p+2))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p+0) (f32.const 0x1.921f_b6p+2))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p+0) (f32.const -0x1.ffff_fep+127))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p+0) (f32.const 0x1.ffff_fep+127))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p+0) (f32.const -0x1.ffff_fep+127))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p+0) (f32.const 0x1.ffff_fep+127))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p+0) (f32.const -inf))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p+0) (f32.const inf))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p+0) (f32.const -inf))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p+0) (f32.const inf))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p+0) (f32.const -nan:0x40_0000))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1p+0) (f32.const nan:0x40_0000))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p+0) (f32.const -nan:0x40_0000))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const 0x1p+0) (f32.const nan:0x40_0000))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.921f_b6p+2) (f32.const -0x0p+0))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.921f_b6p+2) (f32.const 0x0p+0))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.921f_b6p+2) (f32.const -0x0p+0))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.921f_b6p+2) (f32.const 0x0p+0))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.921f_b6p+2) (f32.const -0x1p-149))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.921f_b6p+2) (f32.const 0x1p-149))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.921f_b6p+2) (f32.const -0x1p-149))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.921f_b6p+2) (f32.const 0x1p-149))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.921f_b6p+2) (f32.const -0x1p-126))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.921f_b6p+2) (f32.const 0x1p-126))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.921f_b6p+2) (f32.const -0x1p-126))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.921f_b6p+2) (f32.const 0x1p-126))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.921f_b6p+2) (f32.const -0x1p-1))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.921f_b6p+2) (f32.const 0x1p-1))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.921f_b6p+2) (f32.const -0x1p-1))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.921f_b6p+2) (f32.const 0x1p-1))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.921f_b6p+2) (f32.const -0x1p+0))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.921f_b6p+2) (f32.const 0x1p+0))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.921f_b6p+2) (f32.const -0x1p+0))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.921f_b6p+2) (f32.const 0x1p+0))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.921f_b6p+2) (f32.const -0x1.921f_b6p+2))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.921f_b6p+2) (f32.const 0x1.921f_b6p+2))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.921f_b6p+2) (f32.const -0x1.921f_b6p+2))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.921f_b6p+2) (f32.const 0x1.921f_b6p+2))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.921f_b6p+2) (f32.const -0x1.ffff_fep+127))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.921f_b6p+2) (f32.const 0x1.ffff_fep+127))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.921f_b6p+2) (f32.const -0x1.ffff_fep+127))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.921f_b6p+2) (f32.const 0x1.ffff_fep+127))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.921f_b6p+2) (f32.const -inf))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.921f_b6p+2) (f32.const inf))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.921f_b6p+2) (f32.const -inf))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.921f_b6p+2) (f32.const inf))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.921f_b6p+2) (f32.const -nan:0x40_0000))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.921f_b6p+2) (f32.const nan:0x40_0000))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.921f_b6p+2) (f32.const -nan:0x40_0000))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.921f_b6p+2) (f32.const nan:0x40_0000))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.ffff_fep+127) (f32.const -0x0p+0))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.ffff_fep+127) (f32.const 0x0p+0))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.ffff_fep+127) (f32.const -0x0p+0))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.ffff_fep+127) (f32.const 0x0p+0))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.ffff_fep+127) (f32.const -0x1p-149))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.ffff_fep+127) (f32.const 0x1p-149))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.ffff_fep+127) (f32.const -0x1p-149))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.ffff_fep+127) (f32.const 0x1p-149))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.ffff_fep+127) (f32.const -0x1p-126))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.ffff_fep+127) (f32.const 0x1p-126))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.ffff_fep+127) (f32.const -0x1p-126))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.ffff_fep+127) (f32.const 0x1p-126))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.ffff_fep+127) (f32.const -0x1p-1))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.ffff_fep+127) (f32.const 0x1p-1))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.ffff_fep+127) (f32.const -0x1p-1))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.ffff_fep+127) (f32.const 0x1p-1))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.ffff_fep+127) (f32.const -0x1p+0))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.ffff_fep+127) (f32.const 0x1p+0))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.ffff_fep+127) (f32.const -0x1p+0))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.ffff_fep+127) (f32.const 0x1p+0))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.ffff_fep+127) (f32.const -0x1.921f_b6p+2))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.ffff_fep+127) (f32.const 0x1.921f_b6p+2))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.ffff_fep+127) (f32.const -0x1.921f_b6p+2))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.ffff_fep+127) (f32.const 0x1.921f_b6p+2))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign"
    (f32.const -0x1.ffff_fep+127)
    (f32.const -0x1.ffff_fep+127)
  )
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.ffff_fep+127) (f32.const 0x1.ffff_fep+127))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.ffff_fep+127) (f32.const -0x1.ffff_fep+127))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.ffff_fep+127) (f32.const 0x1.ffff_fep+127))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.ffff_fep+127) (f32.const -inf))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.ffff_fep+127) (f32.const inf))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.ffff_fep+127) (f32.const -inf))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.ffff_fep+127) (f32.const inf))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.ffff_fep+127) (f32.const -nan:0x40_0000))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const -0x1.ffff_fep+127) (f32.const nan:0x40_0000))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.ffff_fep+127) (f32.const -nan:0x40_0000))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const 0x1.ffff_fep+127) (f32.const nan:0x40_0000))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "copysign" (f32.const -inf) (f32.const -0x0p+0))
  (f32.const -inf)
)
(assert_return
  (invoke "copysign" (f32.const -inf) (f32.const 0x0p+0))
  (f32.const inf)
)
(assert_return
  (invoke "copysign" (f32.const inf) (f32.const -0x0p+0))
  (f32.const -inf)
)
(assert_return
  (invoke "copysign" (f32.const inf) (f32.const 0x0p+0))
  (f32.const inf)
)
(assert_return
  (invoke "copysign" (f32.const -inf) (f32.const -0x1p-149))
  (f32.const -inf)
)
(assert_return
  (invoke "copysign" (f32.const -inf) (f32.const 0x1p-149))
  (f32.const inf)
)
(assert_return
  (invoke "copysign" (f32.const inf) (f32.const -0x1p-149))
  (f32.const -inf)
)
(assert_return
  (invoke "copysign" (f32.const inf) (f32.const 0x1p-149))
  (f32.const inf)
)
(assert_return
  (invoke "copysign" (f32.const -inf) (f32.const -0x1p-126))
  (f32.const -inf)
)
(assert_return
  (invoke "copysign" (f32.const -inf) (f32.const 0x1p-126))
  (f32.const inf)
)
(assert_return
  (invoke "copysign" (f32.const inf) (f32.const -0x1p-126))
  (f32.const -inf)
)
(assert_return
  (invoke "copysign" (f32.const inf) (f32.const 0x1p-126))
  (f32.const inf)
)
(assert_return
  (invoke "copysign" (f32.const -inf) (f32.const -0x1p-1))
  (f32.const -inf)
)
(assert_return
  (invoke "copysign" (f32.const -inf) (f32.const 0x1p-1))
  (f32.const inf)
)
(assert_return
  (invoke "copysign" (f32.const inf) (f32.const -0x1p-1))
  (f32.const -inf)
)
(assert_return
  (invoke "copysign" (f32.const inf) (f32.const 0x1p-1))
  (f32.const inf)
)
(assert_return
  (invoke "copysign" (f32.const -inf) (f32.const -0x1p+0))
  (f32.const -inf)
)
(assert_return
  (invoke "copysign" (f32.const -inf) (f32.const 0x1p+0))
  (f32.const inf)
)
(assert_return
  (invoke "copysign" (f32.const inf) (f32.const -0x1p+0))
  (f32.const -inf)
)
(assert_return
  (invoke "copysign" (f32.const inf) (f32.const 0x1p+0))
  (f32.const inf)
)
(assert_return
  (invoke "copysign" (f32.const -inf) (f32.const -0x1.921f_b6p+2))
  (f32.const -inf)
)
(assert_return
  (invoke "copysign" (f32.const -inf) (f32.const 0x1.921f_b6p+2))
  (f32.const inf)
)
(assert_return
  (invoke "copysign" (f32.const inf) (f32.const -0x1.921f_b6p+2))
  (f32.const -inf)
)
(assert_return
  (invoke "copysign" (f32.const inf) (f32.const 0x1.921f_b6p+2))
  (f32.const inf)
)
(assert_return
  (invoke "copysign" (f32.const -inf) (f32.const -0x1.ffff_fep+127))
  (f32.const -inf)
)
(assert_return
  (invoke "copysign" (f32.const -inf) (f32.const 0x1.ffff_fep+127))
  (f32.const inf)
)
(assert_return
  (invoke "copysign" (f32.const inf) (f32.const -0x1.ffff_fep+127))
  (f32.const -inf)
)
(assert_return
  (invoke "copysign" (f32.const inf) (f32.const 0x1.ffff_fep+127))
  (f32.const inf)
)
(assert_return
  (invoke "copysign" (f32.const -inf) (f32.const -inf))
  (f32.const -inf)
)
(assert_return
  (invoke "copysign" (f32.const -inf) (f32.const inf))
  (f32.const inf)
)
(assert_return
  (invoke "copysign" (f32.const inf) (f32.const -inf))
  (f32.const -inf)
)
(assert_return
  (invoke "copysign" (f32.const inf) (f32.const inf))
  (f32.const inf)
)
(assert_return
  (invoke "copysign" (f32.const -inf) (f32.const -nan:0x40_0000))
  (f32.const -inf)
)
(assert_return
  (invoke "copysign" (f32.const -inf) (f32.const nan:0x40_0000))
  (f32.const inf)
)
(assert_return
  (invoke "copysign" (f32.const inf) (f32.const -nan:0x40_0000))
  (f32.const -inf)
)
(assert_return
  (invoke "copysign" (f32.const inf) (f32.const nan:0x40_0000))
  (f32.const inf)
)
(assert_return
  (invoke "copysign" (f32.const -nan:0x40_0000) (f32.const -0x0p+0))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const -nan:0x40_0000) (f32.const 0x0p+0))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const nan:0x40_0000) (f32.const -0x0p+0))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const nan:0x40_0000) (f32.const 0x0p+0))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const -nan:0x40_0000) (f32.const -0x1p-149))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const -nan:0x40_0000) (f32.const 0x1p-149))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const nan:0x40_0000) (f32.const -0x1p-149))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const nan:0x40_0000) (f32.const 0x1p-149))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const -nan:0x40_0000) (f32.const -0x1p-126))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const -nan:0x40_0000) (f32.const 0x1p-126))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const nan:0x40_0000) (f32.const -0x1p-126))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const nan:0x40_0000) (f32.const 0x1p-126))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const -nan:0x40_0000) (f32.const -0x1p-1))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const -nan:0x40_0000) (f32.const 0x1p-1))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const nan:0x40_0000) (f32.const -0x1p-1))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const nan:0x40_0000) (f32.const 0x1p-1))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const -nan:0x40_0000) (f32.const -0x1p+0))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const -nan:0x40_0000) (f32.const 0x1p+0))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const nan:0x40_0000) (f32.const -0x1p+0))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const nan:0x40_0000) (f32.const 0x1p+0))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const -nan:0x40_0000) (f32.const -0x1.921f_b6p+2))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const -nan:0x40_0000) (f32.const 0x1.921f_b6p+2))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const nan:0x40_0000) (f32.const -0x1.921f_b6p+2))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const nan:0x40_0000) (f32.const 0x1.921f_b6p+2))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const -nan:0x40_0000) (f32.const -0x1.ffff_fep+127))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const -nan:0x40_0000) (f32.const 0x1.ffff_fep+127))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const nan:0x40_0000) (f32.const -0x1.ffff_fep+127))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const nan:0x40_0000) (f32.const 0x1.ffff_fep+127))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const -nan:0x40_0000) (f32.const -inf))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const -nan:0x40_0000) (f32.const inf))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const nan:0x40_0000) (f32.const -inf))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const nan:0x40_0000) (f32.const inf))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const -nan:0x40_0000) (f32.const -nan:0x40_0000))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const -nan:0x40_0000) (f32.const nan:0x40_0000))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const nan:0x40_0000) (f32.const -nan:0x40_0000))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "copysign" (f32.const nan:0x40_0000) (f32.const nan:0x40_0000))
  (f32.const nan:0x40_0000)
)
(assert_return (invoke "abs" (f32.const -0x0p+0)) (f32.const 0x0p+0))
(assert_return (invoke "abs" (f32.const 0x0p+0)) (f32.const 0x0p+0))
(assert_return (invoke "abs" (f32.const -0x1p-149)) (f32.const 0x1p-149))
(assert_return (invoke "abs" (f32.const 0x1p-149)) (f32.const 0x1p-149))
(assert_return (invoke "abs" (f32.const -0x1p-126)) (f32.const 0x1p-126))
(assert_return (invoke "abs" (f32.const 0x1p-126)) (f32.const 0x1p-126))
(assert_return (invoke "abs" (f32.const -0x1p-1)) (f32.const 0x1p-1))
(assert_return (invoke "abs" (f32.const 0x1p-1)) (f32.const 0x1p-1))
(assert_return (invoke "abs" (f32.const -0x1p+0)) (f32.const 0x1p+0))
(assert_return (invoke "abs" (f32.const 0x1p+0)) (f32.const 0x1p+0))
(assert_return
  (invoke "abs" (f32.const -0x1.921f_b6p+2))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "abs" (f32.const 0x1.921f_b6p+2))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "abs" (f32.const -0x1.ffff_fep+127))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "abs" (f32.const 0x1.ffff_fep+127))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return (invoke "abs" (f32.const -inf)) (f32.const inf))
(assert_return (invoke "abs" (f32.const inf)) (f32.const inf))
(assert_return
  (invoke "abs" (f32.const -nan:0x40_0000))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "abs" (f32.const nan:0x40_0000))
  (f32.const nan:0x40_0000)
)
(assert_return (invoke "neg" (f32.const -0x0p+0)) (f32.const 0x0p+0))
(assert_return (invoke "neg" (f32.const 0x0p+0)) (f32.const -0x0p+0))
(assert_return (invoke "neg" (f32.const -0x1p-149)) (f32.const 0x1p-149))
(assert_return (invoke "neg" (f32.const 0x1p-149)) (f32.const -0x1p-149))
(assert_return (invoke "neg" (f32.const -0x1p-126)) (f32.const 0x1p-126))
(assert_return (invoke "neg" (f32.const 0x1p-126)) (f32.const -0x1p-126))
(assert_return (invoke "neg" (f32.const -0x1p-1)) (f32.const 0x1p-1))
(assert_return (invoke "neg" (f32.const 0x1p-1)) (f32.const -0x1p-1))
(assert_return (invoke "neg" (f32.const -0x1p+0)) (f32.const 0x1p+0))
(assert_return (invoke "neg" (f32.const 0x1p+0)) (f32.const -0x1p+0))
(assert_return
  (invoke "neg" (f32.const -0x1.921f_b6p+2))
  (f32.const 0x1.921f_b6p+2)
)
(assert_return
  (invoke "neg" (f32.const 0x1.921f_b6p+2))
  (f32.const -0x1.921f_b6p+2)
)
(assert_return
  (invoke "neg" (f32.const -0x1.ffff_fep+127))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "neg" (f32.const 0x1.ffff_fep+127))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return (invoke "neg" (f32.const -inf)) (f32.const inf))
(assert_return (invoke "neg" (f32.const inf)) (f32.const -inf))
(assert_return
  (invoke "neg" (f32.const -nan:0x40_0000))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "neg" (f32.const nan:0x40_0000))
  (f32.const -nan:0x40_0000)
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7d\03\82\80\80\80\00\01\00\0a\94\80\80\80"
    "\00\01\8e\80\80\80\00\00\42\00\44\00\00\00\00\00"
    "\00\00\00\98\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7d\03\82\80\80\80\00\01\00\0a\8b\80\80\80"
    "\00\01\85\80\80\80\00\00\42\00\8b\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7d\03\82\80\80\80\00\01\00\0a\8b\80\80\80"
    "\00\01\85\80\80\80\00\00\42\00\8c\0b"
  )
  "type mismatch"
)
