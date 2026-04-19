(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\01\60"
  "\03\7c\7c\7c\01\7c\03\82\80\80\80\00\01\00\07\96"
  "\80\80\80\00\01\12\66\36\34\2e\6e\6f\5f\63\6f\6e"
  "\74\72\61\63\74\69\6f\6e\00\00\0a\90\80\80\80\00"
  "\01\8a\80\80\80\00\00\20\00\20\01\a2\20\02\a0\0b"
)
(module instance)
(assert_return
  (invoke "f64.no_contraction"
    (f64.const -0x1.9e87_ce14_273a_fp-103)
    (f64.const 0x1.2515_ad31_db63_ep+664)
    (f64.const 0x1.868c_6685_e618_5p+533)
  )
  (f64.const -0x1.da94_885b_1149_3p+561)
)
(assert_return
  (invoke "f64.no_contraction"
    (f64.const 0x1.da21_c460_a6f4_4p+52)
    (f64.const 0x1.6085_9d2e_7714_ap-321)
    (f64.const 0x1.e63f_1b7b_660e_1p-302)
  )
  (f64.const 0x1.4672_f256_d179_4p-268)
)
(assert_return
  (invoke "f64.no_contraction"
    (f64.const -0x1.f3ea_f43f_327cp-594)
    (f64.const 0x1.dfcc_0099_06b5_7p+533)
    (f64.const 0x1.5984_e03c_520a_1p-104)
  )
  (f64.const -0x1.d479_7fb3_db16_6p-60)
)
(assert_return
  (invoke "f64.no_contraction"
    (f64.const 0x1.dab6_c772_cb2e_2p-69)
    (f64.const -0x1.d761_6636_79a8_4p-101)
    (f64.const 0x1.f22f_92c8_4322_6p-218)
  )
  (f64.const -0x1.b50d_72df_cef6_8p-169)
)
(assert_return
  (invoke "f64.no_contraction"
    (f64.const -0x1.87c5_def1_e4d3_dp-950)
    (f64.const -0x1.50cd_5dab_2207_fp+935)
    (f64.const 0x1.e629_bd0d_a8c5_dp-54)
  )
  (f64.const 0x1.01b6_feb4_e78a_7p-14)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\02\60"
  "\03\7d\7d\7d\01\7d\60\03\7c\7c\7c\01\7c\03\83\80"
  "\80\80\00\02\00\01\07\9b\80\80\80\00\02\0a\66\33"
  "\32\2e\6e\6f\5f\66\6d\61\00\00\0a\66\36\34\2e\6e"
  "\6f\5f\66\6d\61\00\01\0a\9f\80\80\80\00\02\8a\80"
  "\80\80\00\00\20\00\20\01\94\20\02\92\0b\8a\80\80"
  "\80\00\00\20\00\20\01\a2\20\02\a0\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fma"
    (f32.const 0x1.a784_02p+124)
    (f32.const 0x1.cf85_48p-23)
    (f32.const 0x1.992a_dap+107)
  )
  (f32.const 0x1.a526_2cp+107)
)
(assert_return
  (invoke "f32.no_fma"
    (f32.const 0x1.ed15_a4p-28)
    (f32.const -0x1.613c_72p-50)
    (f32.const 0x1.4757_bp-88)
  )
  (f32.const -0x1.5406_b8p-77)
)
(assert_return
  (invoke "f32.no_fma"
    (f32.const 0x1.ae63_a2p+37)
    (f32.const 0x1.b3a5_9ap-13)
    (f32.const 0x1.c169_18p+10)
  )
  (f32.const 0x1.6e38_5cp+25)
)
(assert_return
  (invoke "f32.no_fma"
    (f32.const 0x1.2a77_fap-8)
    (f32.const -0x1.bb73_56p+22)
    (f32.const -0x1.32be_2ap+1)
  )
  (f32.const -0x1.0286_d4p+15)
)
(assert_return
  (invoke "f32.no_fma"
    (f32.const 0x1.298f_b6p+126)
    (f32.const -0x1.0308_0cp-70)
    (f32.const -0x1.418d_e6p+34)
  )
  (f32.const -0x1.2d15_c6p+56)
)
(assert_return
  (invoke "f64.no_fma"
    (f64.const 0x1.ac35_7ff4_6eed_4p+557)
    (f64.const 0x1.852c_01a5_e729_7p+430)
    (f64.const -0x1.0599_5704_eda8_ap+987)
  )
  (f64.const 0x1.855d_905d_338ep+987)
)
(assert_return
  (invoke "f64.no_fma"
    (f64.const 0x1.e2fd_6bf3_2010_cp+749)
    (f64.const 0x1.01c2_238d_405e_4p-130)
    (f64.const 0x1.2ecc_0db4_b9f9_4p+573)
  )
  (f64.const 0x1.e64e_b07e_063b_cp+619)
)
(assert_return
  (invoke "f64.no_fma"
    (f64.const 0x1.92b7_c743_9ede_3p-721)
    (f64.const -0x1.6aa9_7586_d3de_6p+1011)
    (f64.const 0x1.8de4_823f_6358_ap+237)
  )
  (f64.const -0x1.1d41_39fd_20ec_dp+291)
)
(assert_return
  (invoke "f64.no_fma"
    (f64.const -0x1.466d_30bd_db45_3p-386)
    (f64.const -0x1.185a_4d73_9c7a_ap+443)
    (f64.const 0x1.5f9c_436f_bfc7_bp+55)
  )
  (f64.const 0x1.bd61_a350_fcc1_ap+57)
)
(assert_return
  (invoke "f64.no_fma"
    (f64.const 0x1.7e2c_4405_8a79_9p+52)
    (f64.const 0x1.c73b_7176_5b8b_2p+685)
    (f64.const -0x1.16c6_41df_0b10_8p+690)
  )
  (f64.const 0x1.53cc_b53d_e0bd_1p+738)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\af\80\80\80\00\02\14\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\61\64\64\5f\7a\65\72\6f\00\00"
  "\14\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\61\64\64"
  "\5f\7a\65\72\6f\00\01\0a\a3\80\80\80\00\02\8a\80"
  "\80\80\00\00\20\00\43\00\00\00\00\92\0b\8e\80\80"
  "\80\00\00\20\00\44\00\00\00\00\00\00\00\00\a0\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_add_zero" (f32.const -0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_add_zero" (f64.const -0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_add_zero" (f32.const nan:0x20_0000))
  (f32.const nan:arithmetic)
)
(assert_return
  (invoke "f64.no_fold_add_zero" (f64.const nan:0x4_0000_0000_0000))
  (f64.const nan:arithmetic)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\af\80\80\80\00\02\14\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\7a\65\72\6f\5f\73\75\62\00\00"
  "\14\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\7a\65\72"
  "\6f\5f\73\75\62\00\01\0a\a3\80\80\80\00\02\8a\80"
  "\80\80\00\00\43\00\00\00\00\20\00\93\0b\8e\80\80"
  "\80\00\00\44\00\00\00\00\00\00\00\00\20\00\a1\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_zero_sub" (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_zero_sub" (f64.const 0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_zero_sub" (f32.const nan:0x20_0000))
  (f32.const nan:arithmetic)
)
(assert_return
  (invoke "f64.no_fold_zero_sub" (f64.const nan:0x4_0000_0000_0000))
  (f64.const nan:arithmetic)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\af\80\80\80\00\02\14\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\73\75\62\5f\7a\65\72\6f\00\00"
  "\14\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\73\75\62"
  "\5f\7a\65\72\6f\00\01\0a\a3\80\80\80\00\02\8a\80"
  "\80\80\00\00\20\00\43\00\00\00\00\93\0b\8e\80\80"
  "\80\00\00\20\00\44\00\00\00\00\00\00\00\00\a1\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_sub_zero" (f32.const nan:0x20_0000))
  (f32.const nan:arithmetic)
)
(assert_return
  (invoke "f64.no_fold_sub_zero" (f64.const nan:0x4_0000_0000_0000))
  (f64.const nan:arithmetic)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\af\80\80\80\00\02\14\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\6d\75\6c\5f\7a\65\72\6f\00\00"
  "\14\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\6d\75\6c"
  "\5f\7a\65\72\6f\00\01\0a\a3\80\80\80\00\02\8a\80"
  "\80\80\00\00\20\00\43\00\00\00\00\94\0b\8e\80\80"
  "\80\00\00\20\00\44\00\00\00\00\00\00\00\00\a2\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_mul_zero" (f32.const -0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_mul_zero" (f32.const -0x1p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_mul_zero" (f32.const -0x1p+1))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_mul_zero" (f32.const nan:0x20_0000))
  (f32.const nan:arithmetic)
)
(assert_return
  (invoke "f64.no_fold_mul_zero" (f64.const -0x0p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_mul_zero" (f64.const -0x1p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_mul_zero" (f64.const -0x1p+1))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_mul_zero" (f64.const nan:0x4_0000_0000_0000))
  (f64.const nan:arithmetic)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\ad\80\80\80\00\02\13\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\6d\75\6c\5f\6f\6e\65\00\00\13"
  "\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\6d\75\6c\5f"
  "\6f\6e\65\00\01\0a\a3\80\80\80\00\02\8a\80\80\80"
  "\00\00\20\00\43\00\00\80\3f\94\0b\8e\80\80\80\00"
  "\00\20\00\44\00\00\00\00\00\00\f0\3f\a2\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_mul_one" (f32.const nan:0x20_0000))
  (f32.const nan:arithmetic)
)
(assert_return
  (invoke "f64.no_fold_mul_one" (f64.const nan:0x4_0000_0000_0000))
  (f64.const nan:arithmetic)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\af\80\80\80\00\02\14\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\7a\65\72\6f\5f\64\69\76\00\00"
  "\14\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\7a\65\72"
  "\6f\5f\64\69\76\00\01\0a\a3\80\80\80\00\02\8a\80"
  "\80\80\00\00\43\00\00\00\00\20\00\95\0b\8e\80\80"
  "\80\00\00\44\00\00\00\00\00\00\00\00\20\00\a3\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_zero_div" (f32.const 0x0p+0))
  (f32.const nan:canonical)
)
(assert_return
  (invoke "f32.no_fold_zero_div" (f32.const -0x0p+0))
  (f32.const nan:canonical)
)
(assert_return
  (invoke "f32.no_fold_zero_div" (f32.const nan:0x40_0000))
  (f32.const nan:canonical)
)
(assert_return
  (invoke "f32.no_fold_zero_div" (f32.const nan:0x20_0000))
  (f32.const nan:arithmetic)
)
(assert_return
  (invoke "f64.no_fold_zero_div" (f64.const 0x0p+0))
  (f64.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_zero_div" (f64.const -0x0p+0))
  (f64.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_zero_div" (f64.const nan:0x8_0000_0000_0000))
  (f64.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_zero_div" (f64.const nan:0x4_0000_0000_0000))
  (f64.const nan:arithmetic)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\ad\80\80\80\00\02\13\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\64\69\76\5f\6f\6e\65\00\00\13"
  "\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\64\69\76\5f"
  "\6f\6e\65\00\01\0a\a3\80\80\80\00\02\8a\80\80\80"
  "\00\00\20\00\43\00\00\80\3f\95\0b\8e\80\80\80\00"
  "\00\20\00\44\00\00\00\00\00\00\f0\3f\a3\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_div_one" (f32.const nan:0x20_0000))
  (f32.const nan:arithmetic)
)
(assert_return
  (invoke "f64.no_fold_div_one" (f64.const nan:0x4_0000_0000_0000))
  (f64.const nan:arithmetic)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\af\80\80\80\00\02\14\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\64\69\76\5f\6e\65\67\31\00\00"
  "\14\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\64\69\76"
  "\5f\6e\65\67\31\00\01\0a\a3\80\80\80\00\02\8a\80"
  "\80\80\00\00\20\00\43\00\00\80\bf\95\0b\8e\80\80"
  "\80\00\00\20\00\44\00\00\00\00\00\00\f0\bf\a3\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_div_neg1" (f32.const nan:0x20_0000))
  (f32.const nan:arithmetic)
)
(assert_return
  (invoke "f64.no_fold_div_neg1" (f64.const nan:0x4_0000_0000_0000))
  (f64.const nan:arithmetic)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\af\80\80\80\00\02\14\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\6e\65\67\30\5f\73\75\62\00\00"
  "\14\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\6e\65\67"
  "\30\5f\73\75\62\00\01\0a\a3\80\80\80\00\02\8a\80"
  "\80\80\00\00\43\00\00\00\80\20\00\93\0b\8e\80\80"
  "\80\00\00\44\00\00\00\00\00\00\00\80\20\00\a1\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_neg0_sub" (f32.const nan:0x20_0000))
  (f32.const nan:arithmetic)
)
(assert_return
  (invoke "f64.no_fold_neg0_sub" (f64.const nan:0x4_0000_0000_0000))
  (f64.const nan:arithmetic)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\af\80\80\80\00\02\14\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\6e\65\67\31\5f\6d\75\6c\00\00"
  "\14\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\6e\65\67"
  "\31\5f\6d\75\6c\00\01\0a\a3\80\80\80\00\02\8a\80"
  "\80\80\00\00\43\00\00\80\bf\20\00\94\0b\8e\80\80"
  "\80\00\00\44\00\00\00\00\00\00\f0\bf\20\00\a2\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_neg1_mul" (f32.const nan:0x20_0000))
  (f32.const nan:arithmetic)
)
(assert_return
  (invoke "f64.no_fold_neg1_mul" (f64.const nan:0x4_0000_0000_0000))
  (f64.const nan:arithmetic)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7f\60\01\7c\01\7f\03\83\80\80\80\00\02"
  "\00\01\07\ad\80\80\80\00\02\13\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\65\71\5f\73\65\6c\66\00\00\13"
  "\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\65\71\5f\73"
  "\65\6c\66\00\01\0a\99\80\80\80\00\02\87\80\80\80"
  "\00\00\20\00\20\00\5b\0b\87\80\80\80\00\00\20\00"
  "\20\00\61\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_eq_self" (f32.const nan:0x40_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "f64.no_fold_eq_self" (f64.const nan:0x8_0000_0000_0000))
  (i32.const 0x0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7f\60\01\7c\01\7f\03\83\80\80\80\00\02"
  "\00\01\07\ad\80\80\80\00\02\13\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\6e\65\5f\73\65\6c\66\00\00\13"
  "\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\6e\65\5f\73"
  "\65\6c\66\00\01\0a\99\80\80\80\00\02\87\80\80\80"
  "\00\00\20\00\20\00\5c\0b\87\80\80\80\00\00\20\00"
  "\20\00\62\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_ne_self" (f32.const nan:0x40_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.no_fold_ne_self" (f64.const nan:0x8_0000_0000_0000))
  (i32.const 0x1)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\af\80\80\80\00\02\14\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\73\75\62\5f\73\65\6c\66\00\00"
  "\14\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\73\75\62"
  "\5f\73\65\6c\66\00\01\0a\99\80\80\80\00\02\87\80"
  "\80\80\00\00\20\00\20\00\93\0b\87\80\80\80\00\00"
  "\20\00\20\00\a1\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_sub_self" (f32.const inf))
  (f32.const nan:canonical)
)
(assert_return
  (invoke "f32.no_fold_sub_self" (f32.const nan:0x40_0000))
  (f32.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_sub_self" (f64.const inf))
  (f64.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_sub_self" (f64.const nan:0x8_0000_0000_0000))
  (f64.const nan:canonical)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\af\80\80\80\00\02\14\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\64\69\76\5f\73\65\6c\66\00\00"
  "\14\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\64\69\76"
  "\5f\73\65\6c\66\00\01\0a\99\80\80\80\00\02\87\80"
  "\80\80\00\00\20\00\20\00\95\0b\87\80\80\80\00\00"
  "\20\00\20\00\a3\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_div_self" (f32.const inf))
  (f32.const nan:canonical)
)
(assert_return
  (invoke "f32.no_fold_div_self" (f32.const nan:0x40_0000))
  (f32.const nan:canonical)
)
(assert_return
  (invoke "f32.no_fold_div_self" (f32.const 0x0p+0))
  (f32.const nan:canonical)
)
(assert_return
  (invoke "f32.no_fold_div_self" (f32.const -0x0p+0))
  (f32.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_div_self" (f64.const inf))
  (f64.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_div_self" (f64.const nan:0x8_0000_0000_0000))
  (f64.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_div_self" (f64.const 0x0p+0))
  (f64.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_div_self" (f64.const -0x0p+0))
  (f64.const nan:canonical)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\a9\80\80\80\00\02\11\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\64\69\76\5f\33\00\00\11\66\36"
  "\34\2e\6e\6f\5f\66\6f\6c\64\5f\64\69\76\5f\33\00"
  "\01\0a\a3\80\80\80\00\02\8a\80\80\80\00\00\20\00"
  "\43\00\00\40\40\95\0b\8e\80\80\80\00\00\20\00\44"
  "\00\00\00\00\00\00\08\40\a3\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_div_3" (f32.const -0x1.359c_26p+50))
  (f32.const -0x1.9cd0_32p+48)
)
(assert_return
  (invoke "f32.no_fold_div_3" (f32.const -0x1.e456_46p+93))
  (f32.const -0x1.42e4_2ep+92)
)
(assert_return
  (invoke "f32.no_fold_div_3" (f32.const -0x1.2a39_16p-83))
  (f32.const -0x1.8da1_72p-85)
)
(assert_return
  (invoke "f32.no_fold_div_3" (f32.const -0x1.1f8b_38p-124))
  (f32.const -0x1.7f64_4ap-126)
)
(assert_return
  (invoke "f32.no_fold_div_3" (f32.const -0x1.d64f_64p-56))
  (f32.const -0x1.398a_42p-57)
)
(assert_return
  (invoke "f64.no_fold_div_3" (f64.const -0x1.a8a8_8d29_e2cc_3p+632))
  (f64.const -0x1.1b1b_08c6_9732_dp+631)
)
(assert_return
  (invoke "f64.no_fold_div_3" (f64.const -0x1.bcf5_2dc9_5097_2p-167))
  (f64.const -0x1.28a3_73db_8b0f_7p-168)
)
(assert_return
  (invoke "f64.no_fold_div_3" (f64.const 0x1.bd3c_0d98_9f7a_4p-874))
  (f64.const 0x1.28d2_b3bb_14fc_3p-875)
)
(assert_return
  (invoke "f64.no_fold_div_3" (f64.const -0x1.0138_bf53_0a53_cp+1007))
  (f64.const -0x1.56f6_546e_b86f_bp+1005)
)
(assert_return
  (invoke "f64.no_fold_div_3" (f64.const 0x1.052b_87f9_d794_dp+415))
  (f64.const 0x1.5c3a_0aa2_74c6_7p+413)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\02\60"
  "\03\7d\7d\7d\01\7d\60\03\7c\7c\7c\01\7c\03\83\80"
  "\80\80\00\02\00\01\07\a1\80\80\80\00\02\0d\66\33"
  "\32\2e\6e\6f\5f\66\61\63\74\6f\72\00\00\0d\66\36"
  "\34\2e\6e\6f\5f\66\61\63\74\6f\72\00\01\0a\a5\80"
  "\80\80\00\02\8d\80\80\80\00\00\20\00\20\02\94\20"
  "\01\20\02\94\92\0b\8d\80\80\80\00\00\20\00\20\02"
  "\a2\20\01\20\02\a2\a0\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_factor"
    (f32.const -0x1.4e23_52p+40)
    (f32.const -0x1.842e_2cp+49)
    (f32.const 0x1.eea6_02p+59)
  )
  (f32.const -0x1.77a7_dp+109)
)
(assert_return
  (invoke "f32.no_factor"
    (f32.const -0x1.b4e7_f6p-6)
    (f32.const 0x1.8c99_0cp-5)
    (f32.const -0x1.70cc_02p-9)
  )
  (f32.const -0x1.00a3_42p-14)
)
(assert_return
  (invoke "f32.no_factor"
    (f32.const -0x1.0672_2ep-41)
    (f32.const 0x1.eed3_cep-64)
    (f32.const 0x1.5c55_58p+123)
  )
  (f32.const -0x1.651a_aep+82)
)
(assert_return
  (invoke "f32.no_factor"
    (f32.const -0x1.f8c6_a4p-64)
    (f32.const 0x1.08c8_06p-83)
    (f32.const 0x1.b5ce_ccp+118)
  )
  (f32.const -0x1.afa1_5p+55)
)
(assert_return
  (invoke "f32.no_factor"
    (f32.const -0x1.3aaa_1ep-84)
    (f32.const 0x1.c6d5_eep-71)
    (f32.const 0x1.8d29_24p+20)
  )
  (f32.const 0x1.60c9_cep-50)
)
(assert_return
  (invoke "f64.no_factor"
    (f64.const 0x1.3ade_da91_4497_7p-424)
    (f64.const 0x1.c15a_f887_049e_1p-462)
    (f64.const -0x1.9051_79c4_c477_8p-225)
  )
  (f64.const -0x1.ec60_6bcb_87b1_ap-649)
)
(assert_return
  (invoke "f64.no_factor"
    (f64.const 0x1.3c84_821c_1d34_8p-662)
    (f64.const -0x1.4ffd_4c77_ad03_7p-1009)
    (f64.const -0x1.dd27_5335_c6f4p-957)
  )
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_factor"
    (f64.const -0x1.074f_3723_4705_1p-334)
    (f64.const -0x1.aaee_f661_f4c9_6p-282)
    (f64.const -0x1.9bd3_4abe_8696_dp+479)
  )
  (f64.const 0x1.5767_0295_93e2p+198)
)
(assert_return
  (invoke "f64.no_factor"
    (f64.const -0x1.c4de_d58a_6f38_9p-289)
    (f64.const 0x1.ba6f_def5_d59c_9p-260)
    (f64.const -0x1.c120_1c04_7020_5p-253)
  )
  (f64.const -0x1.841a_da2e_0f18_4p-512)
)
(assert_return
  (invoke "f64.no_factor"
    (f64.const 0x1.9d36_88f8_e375_ap-608)
    (f64.const 0x1.bf91_3115_8825_6p-579)
    (f64.const -0x1.1605_a6b5_d5ff_8p+489)
  )
  (f64.const -0x1.e611_8ca7_6af5_3p-90)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\02\60"
  "\03\7d\7d\7d\01\7d\60\03\7c\7c\7c\01\7c\03\83\80"
  "\80\80\00\02\00\01\07\a9\80\80\80\00\02\11\66\33"
  "\32\2e\6e\6f\5f\64\69\73\74\72\69\62\75\74\65\00"
  "\00\11\66\36\34\2e\6e\6f\5f\64\69\73\74\72\69\62"
  "\75\74\65\00\01\0a\9f\80\80\80\00\02\8a\80\80\80"
  "\00\00\20\00\20\01\92\20\02\94\0b\8a\80\80\80\00"
  "\00\20\00\20\01\a0\20\02\a2\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_distribute"
    (f32.const -0x1.4e23_52p+40)
    (f32.const -0x1.842e_2cp+49)
    (f32.const 0x1.eea6_02p+59)
  )
  (f32.const -0x1.77a7_d2p+109)
)
(assert_return
  (invoke "f32.no_distribute"
    (f32.const -0x1.b4e7_f6p-6)
    (f32.const 0x1.8c99_0cp-5)
    (f32.const -0x1.70cc_02p-9)
  )
  (f32.const -0x1.00a3_4p-14)
)
(assert_return
  (invoke "f32.no_distribute"
    (f32.const -0x1.0672_2ep-41)
    (f32.const 0x1.eed3_cep-64)
    (f32.const 0x1.5c55_58p+123)
  )
  (f32.const -0x1.651a_bp+82)
)
(assert_return
  (invoke "f32.no_distribute"
    (f32.const -0x1.f8c6_a4p-64)
    (f32.const 0x1.08c8_06p-83)
    (f32.const 0x1.b5ce_ccp+118)
  )
  (f32.const -0x1.afa1_4ep+55)
)
(assert_return
  (invoke "f32.no_distribute"
    (f32.const -0x1.3aaa_1ep-84)
    (f32.const 0x1.c6d5_eep-71)
    (f32.const 0x1.8d29_24p+20)
  )
  (f32.const 0x1.60c9_ccp-50)
)
(assert_return
  (invoke "f64.no_distribute"
    (f64.const 0x1.3ade_da91_4497_7p-424)
    (f64.const 0x1.c15a_f887_049e_1p-462)
    (f64.const -0x1.9051_79c4_c477_8p-225)
  )
  (f64.const -0x1.ec60_6bcb_87b1_bp-649)
)
(assert_return
  (invoke "f64.no_distribute"
    (f64.const 0x1.3c84_821c_1d34_8p-662)
    (f64.const -0x1.4ffd_4c77_ad03_7p-1009)
    (f64.const -0x1.dd27_5335_c6f4p-957)
  )
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_distribute"
    (f64.const -0x1.074f_3723_4705_1p-334)
    (f64.const -0x1.aaee_f661_f4c9_6p-282)
    (f64.const -0x1.9bd3_4abe_8696_dp+479)
  )
  (f64.const 0x1.5767_0295_93e1_fp+198)
)
(assert_return
  (invoke "f64.no_distribute"
    (f64.const -0x1.c4de_d58a_6f38_9p-289)
    (f64.const 0x1.ba6f_def5_d59c_9p-260)
    (f64.const -0x1.c120_1c04_7020_5p-253)
  )
  (f64.const -0x1.841a_da2e_0f18_3p-512)
)
(assert_return
  (invoke "f64.no_distribute"
    (f64.const 0x1.9d36_88f8_e375_ap-608)
    (f64.const 0x1.bf91_3115_8825_6p-579)
    (f64.const -0x1.1605_a6b5_d5ff_8p+489)
  )
  (f64.const -0x1.e611_8ca7_6af5_2p-90)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\02\60"
  "\03\7d\7d\7d\01\7d\60\03\7c\7c\7c\01\7c\03\83\80"
  "\80\80\00\02\00\01\07\b3\80\80\80\00\02\16\66\33"
  "\32\2e\6e\6f\5f\72\65\67\72\6f\75\70\5f\64\69\76"
  "\5f\6d\75\6c\00\00\16\66\36\34\2e\6e\6f\5f\72\65"
  "\67\72\6f\75\70\5f\64\69\76\5f\6d\75\6c\00\01\0a"
  "\9f\80\80\80\00\02\8a\80\80\80\00\00\20\00\20\01"
  "\20\02\95\94\0b\8a\80\80\80\00\00\20\00\20\01\20"
  "\02\a3\a2\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_regroup_div_mul"
    (f32.const -0x1.2d14_a6p-115)
    (f32.const -0x1.575a_6cp-64)
    (f32.const 0x1.5cee_0ep-116)
  )
  (f32.const 0x1.2844_cap-63)
)
(assert_return
  (invoke "f32.no_regroup_div_mul"
    (f32.const -0x1.4547_38p+91)
    (f32.const -0x1.b28a_66p-115)
    (f32.const -0x1.f539_08p+72)
  )
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_regroup_div_mul"
    (f32.const -0x1.6be5_6ep+16)
    (f32.const -0x1.b46f_c6p-21)
    (f32.const -0x1.a51d_f6p-123)
  )
  (f32.const -0x1.7922_58p+118)
)
(assert_return
  (invoke "f32.no_regroup_div_mul"
    (f32.const -0x1.c343_f8p-94)
    (f32.const 0x1.e4d9_06p+73)
    (f32.const 0x1.be69_f8p+68)
  )
  (f32.const -0x1.ea1d_f2p-89)
)
(assert_return
  (invoke "f32.no_regroup_div_mul"
    (f32.const 0x1.c6ae_76p+112)
    (f32.const 0x1.fc95_3cp+24)
    (f32.const -0x1.60b3_e8p+71)
  )
  (f32.const -0x1.47d0_eap+66)
)
(assert_return
  (invoke "f64.no_regroup_div_mul"
    (f64.const 0x1.3c04_b815_e30bp-423)
    (f64.const -0x1.3796_46fd_9812_7p-119)
    (f64.const 0x1.bddb_1585_0603_1p-642)
  )
  (f64.const -0x1.b9b3_301f_2dd2_dp+99)
)
(assert_return
  (invoke "f64.no_regroup_div_mul"
    (f64.const 0x1.46b3_a402_f86d_5p+337)
    (f64.const 0x1.6fbf_1b9e_1798_dp-447)
    (f64.const -0x1.bd97_04a5_a6a0_6p+797)
  )
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_regroup_div_mul"
    (f64.const 0x1.6c97_65bb_4347_fp-479)
    (f64.const 0x1.a4af_42e3_4a14_1p+902)
    (f64.const 0x1.d2dd_e70e_b68f_9p-448)
  )
  (f64.const inf)
)
(assert_return
  (invoke "f64.no_regroup_div_mul"
    (f64.const -0x1.7060_2364_5be7_2p+480)
    (f64.const -0x1.6c22_9f7d_9101_dp+611)
    (f64.const -0x1.4d50_fa68_d3d9_ep+836)
  )
  (f64.const -0x1.926f_a3ca_cc65_1p+255)
)
(assert_return
  (invoke "f64.no_regroup_div_mul"
    (f64.const 0x1.8cc6_3d8c_af4c_7p-599)
    (f64.const 0x1.8671_ac4c_3575_3p-878)
    (f64.const -0x1.ef35_b169_5e65_9p-838)
  )
  (f64.const -0x1.38d5_5f56_406dp-639)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\02\60"
  "\03\7d\7d\7d\01\7d\60\03\7c\7c\7c\01\7c\03\83\80"
  "\80\80\00\02\00\01\07\b3\80\80\80\00\02\16\66\33"
  "\32\2e\6e\6f\5f\72\65\67\72\6f\75\70\5f\6d\75\6c"
  "\5f\64\69\76\00\00\16\66\36\34\2e\6e\6f\5f\72\65"
  "\67\72\6f\75\70\5f\6d\75\6c\5f\64\69\76\00\01\0a"
  "\9f\80\80\80\00\02\8a\80\80\80\00\00\20\00\20\01"
  "\94\20\02\95\0b\8a\80\80\80\00\00\20\00\20\01\a2"
  "\20\02\a3\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_regroup_mul_div"
    (f32.const -0x1.2d14_a6p-115)
    (f32.const -0x1.575a_6cp-64)
    (f32.const 0x1.5cee_0ep-116)
  )
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_regroup_mul_div"
    (f32.const -0x1.4547_38p+91)
    (f32.const -0x1.b28a_66p-115)
    (f32.const -0x1.f539_08p+72)
  )
  (f32.const -0x1.1a00_e8p-96)
)
(assert_return
  (invoke "f32.no_regroup_mul_div"
    (f32.const -0x1.6be5_6ep+16)
    (f32.const -0x1.b46f_c6p-21)
    (f32.const -0x1.a51d_f6p-123)
  )
  (f32.const -0x1.7922_5ap+118)
)
(assert_return
  (invoke "f32.no_regroup_mul_div"
    (f32.const -0x1.c343_f8p-94)
    (f32.const 0x1.e4d9_06p+73)
    (f32.const 0x1.be69_f8p+68)
  )
  (f32.const -0x1.ea1d_f4p-89)
)
(assert_return
  (invoke "f32.no_regroup_mul_div"
    (f32.const 0x1.c6ae_76p+112)
    (f32.const 0x1.fc95_3cp+24)
    (f32.const -0x1.60b3_e8p+71)
  )
  (f32.const -inf)
)
(assert_return
  (invoke "f64.no_regroup_mul_div"
    (f64.const 0x1.3c04_b815_e30bp-423)
    (f64.const -0x1.3796_46fd_9812_7p-119)
    (f64.const 0x1.bddb_1585_0603_1p-642)
  )
  (f64.const -0x1.b9b3_301f_2dd2_ep+99)
)
(assert_return
  (invoke "f64.no_regroup_mul_div"
    (f64.const 0x1.46b3_a402_f86d_5p+337)
    (f64.const 0x1.6fbf_1b9e_1798_dp-447)
    (f64.const -0x1.bd97_04a5_a6a0_6p+797)
  )
  (f64.const -0x1.0da0_b632_8e09p-907)
)
(assert_return
  (invoke "f64.no_regroup_mul_div"
    (f64.const 0x1.6c97_65bb_4347_fp-479)
    (f64.const 0x1.a4af_42e3_4a14_1p+902)
    (f64.const 0x1.d2dd_e70e_b68f_9p-448)
  )
  (f64.const 0x1.4886_b6d9_a9a7_9p+871)
)
(assert_return
  (invoke "f64.no_regroup_mul_div"
    (f64.const -0x1.7060_2364_5be7_2p+480)
    (f64.const -0x1.6c22_9f7d_9101_dp+611)
    (f64.const -0x1.4d50_fa68_d3d9_ep+836)
  )
  (f64.const -inf)
)
(assert_return
  (invoke "f64.no_regroup_mul_div"
    (f64.const 0x1.8cc6_3d8c_af4c_7p-599)
    (f64.const 0x1.8671_ac4c_3575_3p-878)
    (f64.const -0x1.ef35_b169_5e65_9p-838)
  )
  (f64.const -0x0p+0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\91\80\80\80\00\02\60"
  "\04\7d\7d\7d\7d\01\7d\60\04\7c\7c\7c\7c\01\7c\03"
  "\83\80\80\80\00\02\00\01\07\b3\80\80\80\00\02\16"
  "\66\33\32\2e\6e\6f\5f\72\65\61\73\73\6f\63\69\61"
  "\74\65\5f\61\64\64\00\00\16\66\36\34\2e\6e\6f\5f"
  "\72\65\61\73\73\6f\63\69\61\74\65\5f\61\64\64\00"
  "\01\0a\a5\80\80\80\00\02\8d\80\80\80\00\00\20\00"
  "\20\01\92\20\02\92\20\03\92\0b\8d\80\80\80\00\00"
  "\20\00\20\01\a0\20\02\a0\20\03\a0\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_reassociate_add"
    (f32.const -0x1.5f7d_dcp+44)
    (f32.const 0x1.854e_1p+34)
    (f32.const -0x1.b206_8cp+47)
    (f32.const -0x1.2096_92p+41)
  )
  (f32.const -0x1.e26c_76p+47)
)
(assert_return
  (invoke "f32.no_reassociate_add"
    (f32.const 0x1.da3b_78p-9)
    (f32.const -0x1.4312_fap-7)
    (f32.const 0x1.0395_e6p-4)
    (f32.const -0x1.6d5e_a6p-7)
  )
  (f32.const 0x1.78b3_1ap-5)
)
(assert_return
  (invoke "f32.no_reassociate_add"
    (f32.const -0x1.fdb9_3ap+34)
    (f32.const -0x1.b6fc_e6p+41)
    (f32.const 0x1.c131_d8p+44)
    (f32.const 0x1.8835_b6p+38)
  )
  (f32.const 0x1.8ff3_a2p+44)
)
(assert_return
  (invoke "f32.no_reassociate_add"
    (f32.const 0x1.1739_fcp+47)
    (f32.const 0x1.a4b1_86p+49)
    (f32.const -0x1.0c62_3cp+35)
    (f32.const 0x1.16a1_02p+51)
  )
  (f32.const 0x1.913f_f6p+51)
)
(assert_return
  (invoke "f32.no_reassociate_add"
    (f32.const 0x1.733c_fap+108)
    (f32.const -0x1.38d3_0cp+108)
    (f32.const 0x1.2f58_54p+105)
    (f32.const -0x1.ccb0_58p+94)
  )
  (f32.const 0x1.8137_16p+106)
)
(assert_return
  (invoke "f64.no_reassociate_add"
    (f64.const -0x1.697a_4d9f_f19a_6p+841)
    (f64.const 0x1.b305_4662_3839_7p+847)
    (f64.const 0x1.e0b2_d9bf_b4e7_2p+855)
    (f64.const -0x1.6e1f_3ae2_b06b_bp+857)
  )
  (f64.const -0x1.eb0e_5936_f087_ap+856)
)
(assert_return
  (invoke "f64.no_reassociate_add"
    (f64.const 0x1.00ef_6746_b30e_1p-543)
    (f64.const 0x1.cc1c_fafd_f3fe_1p-544)
    (f64.const -0x1.f772_6df3_ecba_6p-543)
    (f64.const -0x1.b266_95f9_9d30_7p-594)
  )
  (f64.const -0x1.0748_92e3_fad7_6p-547)
)
(assert_return
  (invoke "f64.no_reassociate_add"
    (f64.const -0x1.e807_b3bd_6d85_4p+440)
    (f64.const 0x1.ceda_e26c_2c5fp+407)
    (f64.const -0x1.00ab_6e14_4254_1p+437)
    (f64.const 0x1.2853_8a55_997b_dp+397)
  )
  (f64.const -0x1.040e_90bf_871e_bp+441)
)
(assert_return
  (invoke "f64.no_reassociate_add"
    (f64.const -0x1.ba2b_6f35_a240_2p-317)
    (f64.const 0x1.ad1c_3fea_7cd9_ep-307)
    (f64.const -0x1.93aa_ce2b_f126_1p-262)
    (f64.const 0x1.9fdd_be47_2847_ep-260)
  )
  (f64.const 0x1.3af3_0abc_2c01_bp-260)
)
(assert_return
  (invoke "f64.no_reassociate_add"
    (f64.const -0x1.ccb9_c609_2fb1_dp+641)
    (f64.const -0x1.4b7c_28c1_0824_4p+614)
    (f64.const 0x1.8a7c_efef_4bde_1p+646)
    (f64.const -0x1.901b_28b0_8b48_2p+644)
  )
  (f64.const 0x1.1810_5791_9412_6p+646)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\91\80\80\80\00\02\60"
  "\04\7d\7d\7d\7d\01\7d\60\04\7c\7c\7c\7c\01\7c\03"
  "\83\80\80\80\00\02\00\01\07\b3\80\80\80\00\02\16"
  "\66\33\32\2e\6e\6f\5f\72\65\61\73\73\6f\63\69\61"
  "\74\65\5f\6d\75\6c\00\00\16\66\36\34\2e\6e\6f\5f"
  "\72\65\61\73\73\6f\63\69\61\74\65\5f\6d\75\6c\00"
  "\01\0a\a5\80\80\80\00\02\8d\80\80\80\00\00\20\00"
  "\20\01\94\20\02\94\20\03\94\0b\8d\80\80\80\00\00"
  "\20\00\20\01\a2\20\02\a2\20\03\a2\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_reassociate_mul"
    (f32.const 0x1.950b_a8p-116)
    (f32.const 0x1.efda_cep-33)
    (f32.const -0x1.5f9b_cp+102)
    (f32.const 0x1.f045_08p-56)
  )
  (f32.const -0x1.ff35_6ep-101)
)
(assert_return
  (invoke "f32.no_reassociate_mul"
    (f32.const 0x1.5990_aep-56)
    (f32.const -0x1.7dfb_04p+102)
    (f32.const -0x1.4f77_4ap-125)
    (f32.const -0x1.595f_e6p+70)
  )
  (f32.const -0x1.c7c8_fcp-8)
)
(assert_return
  (invoke "f32.no_reassociate_mul"
    (f32.const 0x1.6ad9_a4p-48)
    (f32.const -0x1.9138_aap+55)
    (f32.const -0x1.4a77_4ep-40)
    (f32.const 0x1.1ff0_8p+76)
  )
  (f32.const 0x1.9cd8_ecp+44)
)
(assert_return
  (invoke "f32.no_reassociate_mul"
    (f32.const 0x1.e1ca_ecp-105)
    (f32.const 0x1.af0d_d2p+77)
    (f32.const -0x1.016e_ep+56)
    (f32.const -0x1.ab70_d6p+59)
  )
  (f32.const 0x1.5487_0ep+89)
)
(assert_return
  (invoke "f32.no_reassociate_mul"
    (f32.const -0x1.3b1d_cp-99)
    (f32.const 0x1.4e5a_34p-49)
    (f32.const -0x1.38ba_5ap+3)
    (f32.const 0x1.7fb8_eep+59)
  )
  (f32.const 0x1.5bbf_98p-85)
)
(assert_return
  (invoke "f64.no_reassociate_mul"
    (f64.const -0x1.e784_2ab7_181p-667)
    (f64.const -0x1.fabf_40ce_ecea_fp+990)
    (f64.const -0x1.1a38_a825_ab01_ap-376)
    (f64.const -0x1.27e8_ea46_9b14_fp+664)
  )
  (f64.const 0x1.336e_b428_af4f_3p+613)
)
(assert_return
  (invoke "f64.no_reassociate_mul"
    (f64.const 0x1.4ca2_292a_6acb_cp+454)
    (f64.const 0x1.6ffb_ab85_0089_ap-516)
    (f64.const -0x1.547c_32e1_f5b9_3p-899)
    (f64.const -0x1.c757_1d93_8837_5p+540)
  )
  (f64.const 0x1.1ac7_9695_4fc1p-419)
)
(assert_return
  (invoke "f64.no_reassociate_mul"
    (f64.const 0x1.7388_1a52_e040_1p-501)
    (f64.const -0x1.1b68_dd9e_fb1a_7p+788)
    (f64.const 0x1.d1c5_e6a3_eb27_cp-762)
    (f64.const -0x1.56cb_2fcc_7546_fp+88)
  )
  (f64.const 0x1.f508_db92_c34e_fp-386)
)
(assert_return
  (invoke "f64.no_reassociate_mul"
    (f64.const 0x1.2efa_8785_9987_cp+692)
    (f64.const 0x1.68e4_373e_241p-423)
    (f64.const 0x1.4e2d_0fb3_83a5_7p+223)
    (f64.const -0x1.301d_3265_c737_bp-23)
  )
  (f64.const -0x1.4b2b_6c39_3f30_cp+470)
)
(assert_return
  (invoke "f64.no_reassociate_mul"
    (f64.const 0x1.1013_f749_8b95_fp-234)
    (f64.const 0x1.d2d1_c36f_ff13_8p-792)
    (f64.const -0x1.cbf1_824e_a7bf_dp+728)
    (f64.const -0x1.440d_a9c8_b836_dp-599)
  )
  (f64.const 0x1.1a16_5128_81c9_1p-895)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\a9\80\80\80\00\02\11\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\64\69\76\5f\30\00\00\11\66\36"
  "\34\2e\6e\6f\5f\66\6f\6c\64\5f\64\69\76\5f\30\00"
  "\01\0a\a3\80\80\80\00\02\8a\80\80\80\00\00\20\00"
  "\43\00\00\00\00\95\0b\8e\80\80\80\00\00\20\00\44"
  "\00\00\00\00\00\00\00\00\a3\0b"
)
(module instance)
(assert_return (invoke "f32.no_fold_div_0" (f32.const 0x1p+0)) (f32.const inf))
(assert_return
  (invoke "f32.no_fold_div_0" (f32.const -0x1p+0))
  (f32.const -inf)
)
(assert_return (invoke "f32.no_fold_div_0" (f32.const inf)) (f32.const inf))
(assert_return (invoke "f32.no_fold_div_0" (f32.const -inf)) (f32.const -inf))
(assert_return
  (invoke "f32.no_fold_div_0" (f32.const 0x0p+0))
  (f32.const nan:canonical)
)
(assert_return
  (invoke "f32.no_fold_div_0" (f32.const -0x0p+0))
  (f32.const nan:canonical)
)
(assert_return
  (invoke "f32.no_fold_div_0" (f32.const nan:0x20_0000))
  (f32.const nan:arithmetic)
)
(assert_return
  (invoke "f32.no_fold_div_0" (f32.const nan:0x40_0000))
  (f32.const nan:canonical)
)
(assert_return (invoke "f64.no_fold_div_0" (f64.const 0x1p+0)) (f64.const inf))
(assert_return
  (invoke "f64.no_fold_div_0" (f64.const -0x1p+0))
  (f64.const -inf)
)
(assert_return (invoke "f64.no_fold_div_0" (f64.const inf)) (f64.const inf))
(assert_return (invoke "f64.no_fold_div_0" (f64.const -inf)) (f64.const -inf))
(assert_return
  (invoke "f64.no_fold_div_0" (f64.const 0x0p+0))
  (f64.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_div_0" (f64.const -0x0p+0))
  (f64.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_div_0" (f64.const nan:0x8_0000_0000_0000))
  (f64.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_div_0" (f64.const nan:0x4_0000_0000_0000))
  (f64.const nan:arithmetic)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\af\80\80\80\00\02\14\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\64\69\76\5f\6e\65\67\30\00\00"
  "\14\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\64\69\76"
  "\5f\6e\65\67\30\00\01\0a\a3\80\80\80\00\02\8a\80"
  "\80\80\00\00\20\00\43\00\00\00\80\95\0b\8e\80\80"
  "\80\00\00\20\00\44\00\00\00\00\00\00\00\80\a3\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_div_neg0" (f32.const 0x1p+0))
  (f32.const -inf)
)
(assert_return
  (invoke "f32.no_fold_div_neg0" (f32.const -0x1p+0))
  (f32.const inf)
)
(assert_return (invoke "f32.no_fold_div_neg0" (f32.const inf)) (f32.const -inf))
(assert_return (invoke "f32.no_fold_div_neg0" (f32.const -inf)) (f32.const inf))
(assert_return
  (invoke "f32.no_fold_div_neg0" (f32.const 0x0p+0))
  (f32.const nan:canonical)
)
(assert_return
  (invoke "f32.no_fold_div_neg0" (f32.const -0x0p+0))
  (f32.const nan:canonical)
)
(assert_return
  (invoke "f32.no_fold_div_neg0" (f32.const nan:0x20_0000))
  (f32.const nan:arithmetic)
)
(assert_return
  (invoke "f32.no_fold_div_neg0" (f32.const nan:0x40_0000))
  (f32.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_div_neg0" (f64.const 0x1p+0))
  (f64.const -inf)
)
(assert_return
  (invoke "f64.no_fold_div_neg0" (f64.const -0x1p+0))
  (f64.const inf)
)
(assert_return (invoke "f64.no_fold_div_neg0" (f64.const inf)) (f64.const -inf))
(assert_return (invoke "f64.no_fold_div_neg0" (f64.const -inf)) (f64.const inf))
(assert_return
  (invoke "f64.no_fold_div_neg0" (f64.const 0x0p+0))
  (f64.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_div_neg0" (f64.const -0x0p+0))
  (f64.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_div_neg0" (f64.const nan:0x8_0000_0000_0000))
  (f64.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_div_neg0" (f64.const nan:0x4_0000_0000_0000))
  (f64.const nan:arithmetic)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\83\80\80\80"
  "\00\02\00\01\07\af\80\80\80\00\02\14\66\33\32\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\74\6f\5f\68\79\70\6f\74"
  "\00\00\14\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\74"
  "\6f\5f\68\79\70\6f\74\00\01\0a\a7\80\80\80\00\02"
  "\8e\80\80\80\00\00\20\00\20\00\94\20\01\20\01\94"
  "\92\91\0b\8e\80\80\80\00\00\20\00\20\00\a2\20\01"
  "\20\01\a2\a0\9f\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_to_hypot"
    (f32.const 0x1.c2f3_38p-81)
    (f32.const 0x1.401b_5ep-68)
  )
  (f32.const 0x1.401c_ccp-68)
)
(assert_return
  (invoke "f32.no_fold_to_hypot"
    (f32.const -0x1.c38d_1p-71)
    (f32.const -0x1.359d_dp-107)
  )
  (f32.const 0x1.c36a_62p-71)
)
(assert_return
  (invoke "f32.no_fold_to_hypot"
    (f32.const -0x1.99e0_cap-114)
    (f32.const -0x1.ed0c_6cp-69)
  )
  (f32.const 0x1.ed0e_48p-69)
)
(assert_return
  (invoke "f32.no_fold_to_hypot"
    (f32.const -0x1.1b6c_eap+5)
    (f32.const 0x1.5440_bep+17)
  )
  (f32.const 0x1.5440_cp+17)
)
(assert_return
  (invoke "f32.no_fold_to_hypot"
    (f32.const 0x1.8f01_9ep-76)
    (f32.const -0x1.1823_08p-71)
  )
  (f32.const 0x1.17e2_bcp-71)
)
(assert_return
  (invoke "f64.no_fold_to_hypot"
    (f64.const 0x1.1a0a_c4f7_c871_1p-636)
    (f64.const 0x1.1372_ebaf_ff55_1p-534)
  )
  (f64.const 0x1.1346_3fa3_7014_ep-534)
)
(assert_return
  (invoke "f64.no_fold_to_hypot"
    (f64.const 0x1.b793_5121_6749_9p+395)
    (f64.const -0x1.11cb_c52a_f4c3_6p+410)
  )
  (f64.const 0x1.11cb_c530_783a_2p+410)
)
(assert_return
  (invoke "f64.no_fold_to_hypot"
    (f64.const 0x1.7677_7f44_ff40_bp-536)
    (f64.const -0x1.c389_6e4d_c1fbp-766)
  )
  (f64.const 0x1.8p-536)
)
(assert_return
  (invoke "f64.no_fold_to_hypot"
    (f64.const -0x1.889a_c72c_c6b5_dp-521)
    (f64.const 0x1.8d70_84e6_59f3_bp-733)
  )
  (f64.const 0x1.889a_c72c_a843_ap-521)
)
(assert_return
  (invoke "f64.no_fold_to_hypot"
    (f64.const 0x1.5ee5_88c0_2cb0_8p-670)
    (f64.const -0x1.05ce_2578_8d9e_cp-514)
  )
  (f64.const 0x1.05ce_2578_8d9d_fp-514)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7d\01\7d\03\82\80\80\80\00\01\00\07\a1\80\80"
  "\80\00\01\1d\66\33\32\2e\6e\6f\5f\61\70\70\72\6f"
  "\78\69\6d\61\74\65\5f\72\65\63\69\70\72\6f\63\61"
  "\6c\00\00\0a\90\80\80\80\00\01\8a\80\80\80\00\00"
  "\43\00\00\80\3f\20\00\95\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_approximate_reciprocal" (f32.const -0x1.2900_b6p-10))
  (f32.const -0x1.b950_d4p+9)
)
(assert_return
  (invoke "f32.no_approximate_reciprocal" (f32.const 0x1.e721_2p+127))
  (f32.const 0x1.0d11_f8p-128)
)
(assert_return
  (invoke "f32.no_approximate_reciprocal" (f32.const -0x1.42a4_66p-93))
  (f32.const -0x1.963e_e6p+92)
)
(assert_return
  (invoke "f32.no_approximate_reciprocal" (f32.const 0x1.5d0c_32p+76))
  (f32.const 0x1.7783_62p-77)
)
(assert_return
  (invoke "f32.no_approximate_reciprocal" (f32.const -0x1.601d_e2p-82))
  (f32.const -0x1.743d_7ep+81)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\c4\80\80\80\00\02\22\66\33\32\2e\6e\6f"
  "\5f\61\70\70\72\6f\78\69\6d\61\74\65\5f\72\65\63"
  "\69\70\72\6f\63\61\6c\5f\73\71\72\74\00\00\1b\66"
  "\36\34\2e\6e\6f\5f\66\75\73\65\5f\72\65\63\69\70"
  "\72\6f\63\61\6c\5f\73\71\72\74\00\01\0a\a5\80\80"
  "\80\00\02\8b\80\80\80\00\00\43\00\00\80\3f\20\00"
  "\91\95\0b\8f\80\80\80\00\00\44\00\00\00\00\00\00"
  "\f0\3f\20\00\9f\a3\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_approximate_reciprocal_sqrt" (f32.const 0x1.6af1_2ap-43))
  (f32.const 0x1.300e_d4p+21)
)
(assert_return
  (invoke "f32.no_approximate_reciprocal_sqrt" (f32.const 0x1.e82f_c6p-8))
  (f32.const 0x1.72c3_76p+3)
)
(assert_return
  (invoke "f32.no_approximate_reciprocal_sqrt" (f32.const 0x1.b9fa_9cp-66))
  (f32.const 0x1.85a9_bap+32)
)
(assert_return
  (invoke "f32.no_approximate_reciprocal_sqrt" (f32.const 0x1.f4f5_46p-44))
  (f32.const 0x1.6e01_c2p+21)
)
(assert_return
  (invoke "f32.no_approximate_reciprocal_sqrt" (f32.const 0x1.5da7_aap-86))
  (f32.const 0x1.b618_cap+42)
)
(assert_return
  (invoke "f64.no_fuse_reciprocal_sqrt" (f64.const 0x1.1568_a63b_55fa_3p+889))
  (f64.const 0x1.5bc9_c74c_9952p-445)
)
(assert_return
  (invoke "f64.no_fuse_reciprocal_sqrt" (f64.const 0x1.239f_cd09_39ca_fp+311))
  (f64.const 0x1.5334_a922_b481_8p-156)
)
(assert_return
  (invoke "f64.no_fuse_reciprocal_sqrt" (f64.const 0x1.6e36_a24e_1105_4p+104))
  (f64.const 0x1.ac13_f209_77f2_9p-53)
)
(assert_return
  (invoke "f64.no_fuse_reciprocal_sqrt" (f64.const 0x1.23ee_1732_19f8_3p+668))
  (f64.const 0x1.df75_3e05_5862_dp-335)
)
(assert_return
  (invoke "f64.no_fuse_reciprocal_sqrt" (f64.const 0x1.b30f_74ca_f9ba_bp+146))
  (f64.const 0x1.88bf_c3d1_764a_9p-74)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7d\01\7d\03\82\80\80\80\00\01\00\07\a6\80\80"
  "\80\00\01\22\66\33\32\2e\6e\6f\5f\61\70\70\72\6f"
  "\78\69\6d\61\74\65\5f\73\71\72\74\5f\72\65\63\69"
  "\70\72\6f\63\61\6c\00\00\0a\91\80\80\80\00\01\8b"
  "\80\80\80\00\00\43\00\00\80\3f\20\00\95\91\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_approximate_sqrt_reciprocal" (f32.const 0x1.a4c9_86p+60))
  (f32.const 0x1.8f5a_c6p-31)
)
(assert_return
  (invoke "f32.no_approximate_sqrt_reciprocal" (f32.const 0x1.5051_1ep-9))
  (f32.const 0x1.3bdd_46p+4)
)
(assert_return
  (invoke "f32.no_approximate_sqrt_reciprocal" (f32.const 0x1.125e_c2p+69))
  (f32.const 0x1.5db5_72p-35)
)
(assert_return
  (invoke "f32.no_approximate_sqrt_reciprocal" (f32.const 0x1.ba4c_5p+13))
  (f32.const 0x1.136f_16p-7)
)
(assert_return
  (invoke "f32.no_approximate_sqrt_reciprocal" (f32.const 0x1.4a5b_e2p+104))
  (f32.const 0x1.c2b5_bp-53)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7f\01\7f\60\01\7e\01\7e\03\85\80\80\80\00\04"
  "\00\00\01\01\07\d1\80\80\80\00\04\11\69\33\32\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\66\33\32\5f\73\00\00\11"
  "\69\33\32\2e\6e\6f\5f\66\6f\6c\64\5f\66\33\32\5f"
  "\75\00\01\11\69\36\34\2e\6e\6f\5f\66\6f\6c\64\5f"
  "\66\36\34\5f\73\00\02\11\69\36\34\2e\6e\6f\5f\66"
  "\6f\6c\64\5f\66\36\34\5f\75\00\03\0a\ad\80\80\80"
  "\00\04\86\80\80\80\00\00\20\00\b2\a8\0b\86\80\80"
  "\80\00\00\20\00\b3\a9\0b\86\80\80\80\00\00\20\00"
  "\b9\b0\0b\86\80\80\80\00\00\20\00\ba\b1\0b"
)
(module instance)
(assert_return
  (invoke "i32.no_fold_f32_s" (i32.const 0x100_0000))
  (i32.const 0x100_0000)
)
(assert_return
  (invoke "i32.no_fold_f32_s" (i32.const 0x100_0001))
  (i32.const 0x100_0000)
)
(assert_return
  (invoke "i32.no_fold_f32_s" (i32.const 0xf000_0010))
  (i32.const 0xf000_0010)
)
(assert_return
  (invoke "i32.no_fold_f32_u" (i32.const 0x100_0000))
  (i32.const 0x100_0000)
)
(assert_return
  (invoke "i32.no_fold_f32_u" (i32.const 0x100_0001))
  (i32.const 0x100_0000)
)
(assert_return
  (invoke "i32.no_fold_f32_u" (i32.const 0xf000_0010))
  (i32.const 0xf000_0000)
)
(assert_return
  (invoke "i64.no_fold_f64_s" (i64.const 0x20_0000_0000_0000))
  (i64.const 0x20_0000_0000_0000)
)
(assert_return
  (invoke "i64.no_fold_f64_s" (i64.const 0x20_0000_0000_0001))
  (i64.const 0x20_0000_0000_0000)
)
(assert_return
  (invoke "i64.no_fold_f64_s" (i64.const 0xf000_0000_0000_0400))
  (i64.const 0xf000_0000_0000_0400)
)
(assert_return
  (invoke "i64.no_fold_f64_u" (i64.const 0x20_0000_0000_0000))
  (i64.const 0x20_0000_0000_0000)
)
(assert_return
  (invoke "i64.no_fold_f64_u" (i64.const 0x20_0000_0000_0001))
  (i64.const 0x20_0000_0000_0000)
)
(assert_return
  (invoke "i64.no_fold_f64_u" (i64.const 0xf000_0000_0000_0400))
  (i64.const 0xf000_0000_0000_0000)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\83\80\80\80"
  "\00\02\00\01\07\ad\80\80\80\00\02\13\66\33\32\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\61\64\64\5f\73\75\62\00"
  "\00\13\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\61\64"
  "\64\5f\73\75\62\00\01\0a\9f\80\80\80\00\02\8a\80"
  "\80\80\00\00\20\00\20\01\92\20\01\93\0b\8a\80\80"
  "\80\00\00\20\00\20\01\a0\20\01\a1\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_add_sub"
    (f32.const 0x1.b553_e4p-47)
    (f32.const -0x1.67db_2cp-26)
  )
  (f32.const 0x1.cp-47)
)
(assert_return
  (invoke "f32.no_fold_add_sub"
    (f32.const -0x1.a884_dp-23)
    (f32.const 0x1.f2ae_1ep-19)
  )
  (f32.const -0x1.a884_ep-23)
)
(assert_return
  (invoke "f32.no_fold_add_sub"
    (f32.const -0x1.fc04_fp+82)
    (f32.const -0x1.6540_3ap+101)
  )
  (f32.const -0x1p+83)
)
(assert_return
  (invoke "f32.no_fold_add_sub"
    (f32.const 0x1.870f_a2p-78)
    (f32.const 0x1.c549_16p-56)
  )
  (f32.const 0x1.8p-78)
)
(assert_return
  (invoke "f32.no_fold_add_sub"
    (f32.const -0x1.17e9_66p-108)
    (f32.const -0x1.5fa6_1ap-84)
  )
  (f32.const -0x1p-107)
)
(assert_return
  (invoke "f64.no_fold_add_sub"
    (f64.const -0x1.1053_ea17_2dba_8p-874)
    (f64.const 0x1.113c_4134_08ac_8p-857)
  )
  (f64.const -0x1.1053_ea17_2p-874)
)
(assert_return
  (invoke "f64.no_fold_add_sub"
    (f64.const 0x1.e377_d548_0797_2p-546)
    (f64.const 0x1.040a_0a4d_1ff7p-526)
  )
  (f64.const 0x1.e377_d548p-546)
)
(assert_return
  (invoke "f64.no_fold_add_sub"
    (f64.const -0x1.75f5_3cd9_26b6_2p-30)
    (f64.const -0x1.66b1_76e6_02bb_5p-3)
  )
  (f64.const -0x1.75f5_3dp-30)
)
(assert_return
  (invoke "f64.no_fold_add_sub"
    (f64.const -0x1.c450_ff28_332ap-341)
    (f64.const 0x1.15a5_8550_23ba_ep-305)
  )
  (f64.const -0x1.c451p-341)
)
(assert_return
  (invoke "f64.no_fold_add_sub"
    (f64.const -0x1.1ad4_a596_d3ea_8p-619)
    (f64.const -0x1.17d8_1a41_c0ea_8p-588)
  )
  (f64.const -0x1.1ad4_a8p-619)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\83\80\80\80"
  "\00\02\00\01\07\ad\80\80\80\00\02\13\66\33\32\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\73\75\62\5f\61\64\64\00"
  "\00\13\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\73\75"
  "\62\5f\61\64\64\00\01\0a\9f\80\80\80\00\02\8a\80"
  "\80\80\00\00\20\00\20\01\93\20\01\92\0b\8a\80\80"
  "\80\00\00\20\00\20\01\a1\20\01\a0\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_sub_add"
    (f32.const -0x1.523c_b8p+9)
    (f32.const 0x1.9309_6cp+8)
  )
  (f32.const -0x1.523c_bap+9)
)
(assert_return
  (invoke "f32.no_fold_sub_add"
    (f32.const -0x1.a31a_1p-111)
    (f32.const 0x1.745e_fp-95)
  )
  (f32.const -0x1.a4p-111)
)
(assert_return
  (invoke "f32.no_fold_sub_add"
    (f32.const 0x1.3d53_28p+26)
    (f32.const 0x1.5856_7p+35)
  )
  (f32.const 0x1.3d54p+26)
)
(assert_return
  (invoke "f32.no_fold_sub_add"
    (f32.const 0x1.374e_26p-39)
    (f32.const -0x1.66a5p-27)
  )
  (f32.const 0x1.374p-39)
)
(assert_return
  (invoke "f32.no_fold_sub_add"
    (f32.const 0x1.320f_acp-3)
    (f32.const -0x1.ac06_9ap+14)
  )
  (f32.const 0x1.34p-3)
)
(assert_return
  (invoke "f64.no_fold_sub_add"
    (f64.const 0x1.8f92_aad2_c9b8_dp+255)
    (f64.const -0x1.08cd_4992_266c_bp+259)
  )
  (f64.const 0x1.8f92_aad2_c9b9p+255)
)
(assert_return
  (invoke "f64.no_fold_sub_add"
    (f64.const 0x1.5aaf_f557_42c8_bp-666)
    (f64.const 0x1.8f5f_4718_1f46_dp-647)
  )
  (f64.const 0x1.5aaf_f557_8p-666)
)
(assert_return
  (invoke "f64.no_fold_sub_add"
    (f64.const 0x1.21bc_5296_7a98_dp+251)
    (f64.const -0x1.fcff_aa32_d088_4p+300)
  )
  (f64.const 0x1.2p+251)
)
(assert_return
  (invoke "f64.no_fold_sub_add"
    (f64.const 0x1.9c78_361f_4737_4p-26)
    (f64.const -0x1.69d6_9f4e_dc61_cp-13)
  )
  (f64.const 0x1.9c78_361f_48p-26)
)
(assert_return
  (invoke "f64.no_fold_sub_add"
    (f64.const 0x1.4dbe_68e4_afab_2p-367)
    (f64.const -0x1.dc24_e5b3_9cd0_2p-361)
  )
  (f64.const 0x1.4dbe_68e4_afacp-367)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\83\80\80\80"
  "\00\02\00\01\07\ad\80\80\80\00\02\13\66\33\32\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\6d\75\6c\5f\64\69\76\00"
  "\00\13\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\6d\75"
  "\6c\5f\64\69\76\00\01\0a\9f\80\80\80\00\02\8a\80"
  "\80\80\00\00\20\00\20\01\94\20\01\95\0b\8a\80\80"
  "\80\00\00\20\00\20\01\a2\20\01\a3\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_mul_div"
    (f32.const -0x1.cd85_9ap+54)
    (f32.const 0x1.6ca9_36p-47)
  )
  (f32.const -0x1.cd85_98p+54)
)
(assert_return
  (invoke "f32.no_fold_mul_div"
    (f32.const -0x1.0b56_b8p-26)
    (f32.const 0x1.4826_4cp-106)
  )
  (f32.const -0x1.0b56_a4p-26)
)
(assert_return
  (invoke "f32.no_fold_mul_div"
    (f32.const -0x1.e755_5cp-48)
    (f32.const -0x1.9161_cp+48)
  )
  (f32.const -0x1.e755_5ap-48)
)
(assert_return
  (invoke "f32.no_fold_mul_div"
    (f32.const 0x1.aaa5_0ep+52)
    (f32.const -0x1.dfb3_9ep+60)
  )
  (f32.const 0x1.aaa5_0cp+52)
)
(assert_return
  (invoke "f32.no_fold_mul_div"
    (f32.const -0x1.2b7d_fap-92)
    (f32.const -0x1.7c4c_a6p-37)
  )
  (f32.const -0x1.2b7d_fep-92)
)
(assert_return
  (invoke "f64.no_fold_mul_div"
    (f64.const -0x1.3d79_ff41_18a1_ap-837)
    (f64.const -0x1.b8b5_dda3_1808_cp-205)
  )
  (f64.const -0x1.3d79_ff41_2263_ep-837)
)
(assert_return
  (invoke "f64.no_fold_mul_div"
    (f64.const 0x1.f894_d1ee_6b3a_4p+384)
    (f64.const 0x1.8c26_06d0_3d58_ap+585)
  )
  (f64.const 0x1.f894_d1ee_6b3a_5p+384)
)
(assert_return
  (invoke "f64.no_fold_mul_div"
    (f64.const -0x1.a022_260a_cc99_3p+238)
    (f64.const -0x1.5fbc_128f_c8e3_cp-552)
  )
  (f64.const -0x1.a022_260a_cc99_2p+238)
)
(assert_return
  (invoke "f64.no_fold_mul_div"
    (f64.const 0x1.9d4b_8ed1_74f5_4p-166)
    (f64.const 0x1.ee3d_467a_eeac_6p-906)
  )
  (f64.const 0x1.8dcc_95a0_53b2_bp-166)
)
(assert_return
  (invoke "f64.no_fold_mul_div"
    (f64.const -0x1.e95e_a897_cdcd_4p+660)
    (f64.const -0x1.854d_5df0_85f2_ep-327)
  )
  (f64.const -0x1.e95e_a897_cdcd_5p+660)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\83\80\80\80"
  "\00\02\00\01\07\ad\80\80\80\00\02\13\66\33\32\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\64\69\76\5f\6d\75\6c\00"
  "\00\13\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\64\69"
  "\76\5f\6d\75\6c\00\01\0a\9f\80\80\80\00\02\8a\80"
  "\80\80\00\00\20\00\20\01\95\20\01\94\0b\8a\80\80"
  "\80\00\00\20\00\20\01\a3\20\01\a2\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_div_mul"
    (f32.const -0x1.dc63_64p+38)
    (f32.const 0x1.d630_ecp+29)
  )
  (f32.const -0x1.dc63_62p+38)
)
(assert_return
  (invoke "f32.no_fold_div_mul"
    (f32.const -0x1.1f98_36p-52)
    (f32.const -0x1.16c4_e4p-18)
  )
  (f32.const -0x1.1f98_38p-52)
)
(assert_return
  (invoke "f32.no_fold_div_mul"
    (f32.const 0x1.c597_2cp-126)
    (f32.const -0x1.d665_9ep+7)
  )
  (f32.const 0x1.c598_0ep-126)
)
(assert_return
  (invoke "f32.no_fold_div_mul"
    (f32.const -0x1.2e3a_9ep-74)
    (f32.const -0x1.3539_94p+59)
  )
  (f32.const -0x1.2e3a_4p-74)
)
(assert_return
  (invoke "f32.no_fold_div_mul"
    (f32.const 0x1.d96b_82p-98)
    (f32.const 0x1.95d9_08p+27)
  )
  (f32.const 0x1.d96b_84p-98)
)
(assert_return
  (invoke "f64.no_fold_div_mul"
    (f64.const 0x1.d01f_913a_5248_1p-876)
    (f64.const -0x1.2cd0_668b_2834_4p+184)
  )
  (f64.const 0x1.d020_daf7_1cdcp-876)
)
(assert_return
  (invoke "f64.no_fold_div_mul"
    (f64.const -0x1.81cb_7d40_0918_dp-714)
    (f64.const 0x1.7caa_6435_86d6_ep-53)
  )
  (f64.const -0x1.81cb_7d40_0918_ep-714)
)
(assert_return
  (invoke "f64.no_fold_div_mul"
    (f64.const -0x1.6690_4c97_b5c8_ep-145)
    (f64.const 0x1.5c34_8159_2ad4_cp+428)
  )
  (f64.const -0x1.6690_4c97_b5c8_dp-145)
)
(assert_return
  (invoke "f64.no_fold_div_mul"
    (f64.const -0x1.e758_59d2_f076_5p-278)
    (f64.const -0x1.5f19_b6ab_497f_9p+283)
  )
  (f64.const -0x1.e758_59d2_f076_4p-278)
)
(assert_return
  (invoke "f64.no_fold_div_mul"
    (f64.const -0x1.515f_e9c3_b5f5p+620)
    (f64.const 0x1.36be_869c_99f7_ap+989)
  )
  (f64.const -0x1.515f_e9c3_b5f4_fp+620)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\b1\80\80\80\00\02\15\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\64\69\76\32\5f\6d\75\6c\32\00"
  "\00\15\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\64\69"
  "\76\32\5f\6d\75\6c\32\00\01\0a\b3\80\80\80\00\02"
  "\90\80\80\80\00\00\20\00\43\00\00\00\40\95\43\00"
  "\00\00\40\94\0b\98\80\80\80\00\00\20\00\44\00\00"
  "\00\00\00\00\00\40\a3\44\00\00\00\00\00\00\00\40"
  "\a2\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_div2_mul2" (f32.const 0x1.ffff_fep-126))
  (f32.const 0x1p-125)
)
(assert_return
  (invoke "f64.no_fold_div2_mul2" (f64.const 0x1.ffff_ffff_ffff_fp-1022))
  (f64.const 0x1p-1021)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7c\01\7c\03\82\80\80\80\00\01\00\07\9a\80\80"
  "\80\00\01\16\6e\6f\5f\66\6f\6c\64\5f\64\65\6d\6f"
  "\74\65\5f\70\72\6f\6d\6f\74\65\00\00\0a\8c\80\80"
  "\80\00\01\86\80\80\80\00\00\20\00\b6\bb\0b"
)
(module instance)
(assert_return
  (invoke "no_fold_demote_promote" (f64.const -0x1.dece_2723_90f5_dp-133))
  (f64.const -0x1.decep-133)
)
(assert_return
  (invoke "no_fold_demote_promote" (f64.const -0x1.19e6_c799_38a6_fp-85))
  (f64.const -0x1.19e6_c8p-85)
)
(assert_return
  (invoke "no_fold_demote_promote" (f64.const 0x1.49b2_97ec_44dc_1p+107))
  (f64.const 0x1.49b2_98p+107)
)
(assert_return
  (invoke "no_fold_demote_promote" (f64.const -0x1.74f5_bd86_5163p-88))
  (f64.const -0x1.74f5_bep-88)
)
(assert_return
  (invoke "no_fold_demote_promote" (f64.const 0x1.26d6_7566_2367_ep+104))
  (f64.const 0x1.26d6_76p+104)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7d\01\7d\03\82\80\80\80\00\01\00\07\9a\80\80"
  "\80\00\01\16\6e\6f\5f\66\6f\6c\64\5f\70\72\6f\6d"
  "\6f\74\65\5f\64\65\6d\6f\74\65\00\00\0a\8c\80\80"
  "\80\00\01\86\80\80\80\00\00\20\00\bb\b6\0b"
)
(module instance)
(assert_return
  (invoke "no_fold_promote_demote" (f32.const nan:0x20_0000))
  (f32.const nan:arithmetic)
)
(assert_return
  (invoke "no_fold_promote_demote" (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "no_fold_promote_demote" (f32.const -0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "no_fold_promote_demote" (f32.const 0x1p-149))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "no_fold_promote_demote" (f32.const -0x1p-149))
  (f32.const -0x1p-149)
)
(assert_return
  (invoke "no_fold_promote_demote" (f32.const 0x1.ffff_fcp-127))
  (f32.const 0x1.ffff_fcp-127)
)
(assert_return
  (invoke "no_fold_promote_demote" (f32.const -0x1.ffff_fcp-127))
  (f32.const -0x1.ffff_fcp-127)
)
(assert_return
  (invoke "no_fold_promote_demote" (f32.const 0x1p-126))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "no_fold_promote_demote" (f32.const -0x1p-126))
  (f32.const -0x1p-126)
)
(assert_return
  (invoke "no_fold_promote_demote" (f32.const 0x1.ffff_fep+127))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "no_fold_promote_demote" (f32.const -0x1.ffff_fep+127))
  (f32.const -0x1.ffff_fep+127)
)
(assert_return
  (invoke "no_fold_promote_demote" (f32.const inf))
  (f32.const inf)
)
(assert_return
  (invoke "no_fold_promote_demote" (f32.const -inf))
  (f32.const -inf)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7c\7d\01\7d\60\02\7d\7c\01\7d\03\83\80\80\80"
  "\00\02\00\01\07\b6\80\80\80\00\02\13\6e\6f\5f\64"
  "\65\6d\6f\74\65\5f\6d\69\78\65\64\5f\61\64\64\00"
  "\00\1c\6e\6f\5f\64\65\6d\6f\74\65\5f\6d\69\78\65"
  "\64\5f\61\64\64\5f\63\6f\6d\6d\75\74\65\64\00\01"
  "\0a\9d\80\80\80\00\02\89\80\80\80\00\00\20\00\20"
  "\01\bb\a0\b6\0b\89\80\80\80\00\00\20\00\bb\20\01"
  "\a0\b6\0b"
)
(module instance)
(assert_return
  (invoke "no_demote_mixed_add"
    (f64.const 0x1.f51a_9d04_854f_9p-95)
    (f32.const 0x1.3f4e_9cp-119)
  )
  (f32.const 0x1.f51a_9ep-95)
)
(assert_return
  (invoke "no_demote_mixed_add"
    (f64.const 0x1.065b_3d81_ad8dp+37)
    (f32.const 0x1.758c_d8p+38)
  )
  (f32.const 0x1.f8ba_76p+38)
)
(assert_return
  (invoke "no_demote_mixed_add"
    (f64.const 0x1.626c_8096_3bd1_7p-119)
    (f32.const -0x1.9bbf_86p-121)
  )
  (f32.const 0x1.f6f9_3ep-120)
)
(assert_return
  (invoke "no_demote_mixed_add"
    (f64.const -0x1.0d51_10e3_385b_bp-20)
    (f32.const 0x1.096f_4ap-29)
  )
  (f32.const -0x1.0ccc_5ap-20)
)
(assert_return
  (invoke "no_demote_mixed_add"
    (f64.const -0x1.7385_2db4_e507_5p-20)
    (f32.const -0x1.24e4_74p-41)
  )
  (f32.const -0x1.7385_36p-20)
)
(assert_return
  (invoke "no_demote_mixed_add_commuted"
    (f32.const 0x1.3f4e_9cp-119)
    (f64.const 0x1.f51a_9d04_854f_9p-95)
  )
  (f32.const 0x1.f51a_9ep-95)
)
(assert_return
  (invoke "no_demote_mixed_add_commuted"
    (f32.const 0x1.758c_d8p+38)
    (f64.const 0x1.065b_3d81_ad8dp+37)
  )
  (f32.const 0x1.f8ba_76p+38)
)
(assert_return
  (invoke "no_demote_mixed_add_commuted"
    (f32.const -0x1.9bbf_86p-121)
    (f64.const 0x1.626c_8096_3bd1_7p-119)
  )
  (f32.const 0x1.f6f9_3ep-120)
)
(assert_return
  (invoke "no_demote_mixed_add_commuted"
    (f32.const 0x1.096f_4ap-29)
    (f64.const -0x1.0d51_10e3_385b_bp-20)
  )
  (f32.const -0x1.0ccc_5ap-20)
)
(assert_return
  (invoke "no_demote_mixed_add_commuted"
    (f32.const -0x1.24e4_74p-41)
    (f64.const -0x1.7385_2db4_e507_5p-20)
  )
  (f32.const -0x1.7385_36p-20)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7c\7d\01\7d\60\02\7d\7c\01\7d\03\83\80\80\80"
  "\00\02\00\01\07\b6\80\80\80\00\02\13\6e\6f\5f\64"
  "\65\6d\6f\74\65\5f\6d\69\78\65\64\5f\73\75\62\00"
  "\00\1c\6e\6f\5f\64\65\6d\6f\74\65\5f\6d\69\78\65"
  "\64\5f\73\75\62\5f\63\6f\6d\6d\75\74\65\64\00\01"
  "\0a\9d\80\80\80\00\02\89\80\80\80\00\00\20\00\20"
  "\01\bb\a1\b6\0b\89\80\80\80\00\00\20\00\bb\20\01"
  "\a1\b6\0b"
)
(module instance)
(assert_return
  (invoke "no_demote_mixed_sub"
    (f64.const 0x1.a0a1_8322_0e9b_1p+82)
    (f32.const 0x1.c5ac_f8p+61)
  )
  (f32.const 0x1.a0a1_74p+82)
)
(assert_return
  (invoke "no_demote_mixed_sub"
    (f64.const -0x1.6e2c_5ac3_9f63_ep+30)
    (f32.const 0x1.d48c_a4p+17)
  )
  (f32.const -0x1.6e3bp+30)
)
(assert_return
  (invoke "no_demote_mixed_sub"
    (f64.const -0x1.98c7_4350_dde6_ap+6)
    (f32.const 0x1.9d69_bcp-12)
  )
  (f32.const -0x1.98c7_aap+6)
)
(assert_return
  (invoke "no_demote_mixed_sub"
    (f64.const 0x1.0459_f340_91db_fp-54)
    (f32.const 0x1.61ad_08p-71)
  )
  (f32.const 0x1.0459_42p-54)
)
(assert_return
  (invoke "no_demote_mixed_sub"
    (f64.const 0x1.a749_8dca_3fdb_7p+14)
    (f32.const 0x1.ed21_c8p+15)
  )
  (f32.const -0x1.197d_02p+15)
)
(assert_return
  (invoke "no_demote_mixed_sub_commuted"
    (f32.const 0x1.c5ac_f8p+61)
    (f64.const 0x1.a0a1_8322_0e9b_1p+82)
  )
  (f32.const -0x1.a0a1_74p+82)
)
(assert_return
  (invoke "no_demote_mixed_sub_commuted"
    (f32.const 0x1.d48c_a4p+17)
    (f64.const -0x1.6e2c_5ac3_9f63_ep+30)
  )
  (f32.const 0x1.6e3bp+30)
)
(assert_return
  (invoke "no_demote_mixed_sub_commuted"
    (f32.const 0x1.9d69_bcp-12)
    (f64.const -0x1.98c7_4350_dde6_ap+6)
  )
  (f32.const 0x1.98c7_aap+6)
)
(assert_return
  (invoke "no_demote_mixed_sub_commuted"
    (f32.const 0x1.61ad_08p-71)
    (f64.const 0x1.0459_f340_91db_fp-54)
  )
  (f32.const -0x1.0459_42p-54)
)
(assert_return
  (invoke "no_demote_mixed_sub_commuted"
    (f32.const 0x1.ed21_c8p+15)
    (f64.const 0x1.a749_8dca_3fdb_7p+14)
  )
  (f32.const 0x1.197d_02p+15)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7c\7d\01\7d\60\02\7d\7c\01\7d\03\83\80\80\80"
  "\00\02\00\01\07\b6\80\80\80\00\02\13\6e\6f\5f\64"
  "\65\6d\6f\74\65\5f\6d\69\78\65\64\5f\6d\75\6c\00"
  "\00\1c\6e\6f\5f\64\65\6d\6f\74\65\5f\6d\69\78\65"
  "\64\5f\6d\75\6c\5f\63\6f\6d\6d\75\74\65\64\00\01"
  "\0a\9d\80\80\80\00\02\89\80\80\80\00\00\20\00\20"
  "\01\bb\a2\b6\0b\89\80\80\80\00\00\20\00\bb\20\01"
  "\a2\b6\0b"
)
(module instance)
(assert_return
  (invoke "no_demote_mixed_mul"
    (f64.const 0x1.a197_89e5_aa47_5p-202)
    (f32.const 0x1.858c_bep+113)
  )
  (f32.const 0x1.3db8_6cp-88)
)
(assert_return
  (invoke "no_demote_mixed_mul"
    (f64.const 0x1.8f0e_6a5a_53f1_5p+140)
    (f32.const 0x1.2ef8_26p-107)
  )
  (f32.const 0x1.d845_d2p+33)
)
(assert_return
  (invoke "no_demote_mixed_mul"
    (f64.const 0x1.f03a_a769_e296_cp+176)
    (f32.const 0x1.a925_5p-57)
  )
  (f32.const 0x1.9c0c_dap+120)
)
(assert_return
  (invoke "no_demote_mixed_mul"
    (f64.const 0x1.9cd7_0b63_6bc5_2p+221)
    (f32.const 0x1.3f3a_c6p-122)
  )
  (f32.const 0x1.0167_6p+100)
)
(assert_return
  (invoke "no_demote_mixed_mul"
    (f64.const 0x1.c56b_4c29_91a3_cp-170)
    (f32.const 0x1.1ad2_42p+48)
  )
  (f32.const 0x1.f4ec_98p-122)
)
(assert_return
  (invoke "no_demote_mixed_mul_commuted"
    (f32.const 0x1.858c_bep+113)
    (f64.const 0x1.a197_89e5_aa47_5p-202)
  )
  (f32.const 0x1.3db8_6cp-88)
)
(assert_return
  (invoke "no_demote_mixed_mul_commuted"
    (f32.const 0x1.2ef8_26p-107)
    (f64.const 0x1.8f0e_6a5a_53f1_5p+140)
  )
  (f32.const 0x1.d845_d2p+33)
)
(assert_return
  (invoke "no_demote_mixed_mul_commuted"
    (f32.const 0x1.a925_5p-57)
    (f64.const 0x1.f03a_a769_e296_cp+176)
  )
  (f32.const 0x1.9c0c_dap+120)
)
(assert_return
  (invoke "no_demote_mixed_mul_commuted"
    (f32.const 0x1.3f3a_c6p-122)
    (f64.const 0x1.9cd7_0b63_6bc5_2p+221)
  )
  (f32.const 0x1.0167_6p+100)
)
(assert_return
  (invoke "no_demote_mixed_mul_commuted"
    (f32.const 0x1.1ad2_42p+48)
    (f64.const 0x1.c56b_4c29_91a3_cp-170)
  )
  (f32.const 0x1.f4ec_98p-122)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7c\7d\01\7d\60\02\7d\7c\01\7d\03\83\80\80\80"
  "\00\02\00\01\07\b6\80\80\80\00\02\13\6e\6f\5f\64"
  "\65\6d\6f\74\65\5f\6d\69\78\65\64\5f\64\69\76\00"
  "\00\1c\6e\6f\5f\64\65\6d\6f\74\65\5f\6d\69\78\65"
  "\64\5f\64\69\76\5f\63\6f\6d\6d\75\74\65\64\00\01"
  "\0a\9d\80\80\80\00\02\89\80\80\80\00\00\20\00\20"
  "\01\bb\a3\b6\0b\89\80\80\80\00\00\20\00\bb\20\01"
  "\a3\b6\0b"
)
(module instance)
(assert_return
  (invoke "no_demote_mixed_div"
    (f64.const 0x1.40d0_b55d_4cee_1p+150)
    (f32.const 0x1.6c74_96p+103)
  )
  (f32.const 0x1.c2b1_58p+46)
)
(assert_return
  (invoke "no_demote_mixed_div"
    (f64.const 0x1.4027_50f3_4cd9_8p-153)
    (f32.const 0x1.3db8_ep-82)
  )
  (f32.const 0x1.01f5_86p-71)
)
(assert_return
  (invoke "no_demote_mixed_div"
    (f64.const 0x1.3f7e_ce1a_790a_7p-37)
    (f32.const 0x1.a565_2p-128)
  )
  (f32.const 0x1.8430_dp+90)
)
(assert_return
  (invoke "no_demote_mixed_div"
    (f64.const 0x1.5171_328e_1688_5p-138)
    (f32.const 0x1.1063_6ap-88)
  )
  (f32.const 0x1.3d23_cep-50)
)
(assert_return
  (invoke "no_demote_mixed_div"
    (f64.const 0x1.d3a3_80fc_986c_cp+74)
    (f32.const 0x1.f095_b6p+88)
  )
  (f32.const 0x1.e227_c4p-15)
)
(assert_return
  (invoke "no_demote_mixed_div_commuted"
    (f32.const 0x1.d78d_dcp-74)
    (f64.const 0x1.2c57_e125_069e_2p-42)
  )
  (f32.const 0x1.91ee_d6p-32)
)
(assert_return
  (invoke "no_demote_mixed_div_commuted"
    (f32.const 0x1.7db2_24p+26)
    (f64.const 0x1.1c29_1ec6_09ed_4p+159)
  )
  (f32.const 0x1.57dfp-133)
)
(assert_return
  (invoke "no_demote_mixed_div_commuted"
    (f32.const 0x1.e7a8_24p-40)
    (f64.const 0x1.f4bd_b25f_f00f_cp-137)
  )
  (f32.const 0x1.f29f_22p+96)
)
(assert_return
  (invoke "no_demote_mixed_div_commuted"
    (f32.const 0x1.730b_8p+80)
    (f64.const 0x1.880f_b331_a64c_ap+210)
  )
  (f32.const 0x1.e48ep-131)
)
(assert_return
  (invoke "no_demote_mixed_div_commuted"
    (f32.const 0x1.7715_fcp-73)
    (f64.const 0x1.6feb_1fa6_6f11_bp-198)
  )
  (f32.const 0x1.04fc_b6p+125)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\91\80\80\80\00\10"
  "\00\00\00\00\01\01\01\01\00\00\00\00\01\01\01\01"
  "\07\c1\84\80\80\00\10\21\66\33\32\2e\69\33\32\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\74\72\75\6e\63\5f\73\5f"
  "\63\6f\6e\76\65\72\74\5f\73\00\00\21\66\33\32\2e"
  "\69\33\32\2e\6e\6f\5f\66\6f\6c\64\5f\74\72\75\6e"
  "\63\5f\75\5f\63\6f\6e\76\65\72\74\5f\73\00\01\21"
  "\66\33\32\2e\69\33\32\2e\6e\6f\5f\66\6f\6c\64\5f"
  "\74\72\75\6e\63\5f\73\5f\63\6f\6e\76\65\72\74\5f"
  "\75\00\02\21\66\33\32\2e\69\33\32\2e\6e\6f\5f\66"
  "\6f\6c\64\5f\74\72\75\6e\63\5f\75\5f\63\6f\6e\76"
  "\65\72\74\5f\75\00\03\21\66\36\34\2e\69\33\32\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\74\72\75\6e\63\5f\73\5f"
  "\63\6f\6e\76\65\72\74\5f\73\00\04\21\66\36\34\2e"
  "\69\33\32\2e\6e\6f\5f\66\6f\6c\64\5f\74\72\75\6e"
  "\63\5f\75\5f\63\6f\6e\76\65\72\74\5f\73\00\05\21"
  "\66\36\34\2e\69\33\32\2e\6e\6f\5f\66\6f\6c\64\5f"
  "\74\72\75\6e\63\5f\73\5f\63\6f\6e\76\65\72\74\5f"
  "\75\00\06\21\66\36\34\2e\69\33\32\2e\6e\6f\5f\66"
  "\6f\6c\64\5f\74\72\75\6e\63\5f\75\5f\63\6f\6e\76"
  "\65\72\74\5f\75\00\07\21\66\33\32\2e\69\36\34\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\74\72\75\6e\63\5f\73\5f"
  "\63\6f\6e\76\65\72\74\5f\73\00\08\21\66\33\32\2e"
  "\69\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\74\72\75\6e"
  "\63\5f\75\5f\63\6f\6e\76\65\72\74\5f\73\00\09\21"
  "\66\33\32\2e\69\36\34\2e\6e\6f\5f\66\6f\6c\64\5f"
  "\74\72\75\6e\63\5f\73\5f\63\6f\6e\76\65\72\74\5f"
  "\75\00\0a\21\66\33\32\2e\69\36\34\2e\6e\6f\5f\66"
  "\6f\6c\64\5f\74\72\75\6e\63\5f\75\5f\63\6f\6e\76"
  "\65\72\74\5f\75\00\0b\21\66\36\34\2e\69\36\34\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\74\72\75\6e\63\5f\73\5f"
  "\63\6f\6e\76\65\72\74\5f\73\00\0c\21\66\36\34\2e"
  "\69\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\74\72\75\6e"
  "\63\5f\75\5f\63\6f\6e\76\65\72\74\5f\73\00\0d\21"
  "\66\36\34\2e\69\36\34\2e\6e\6f\5f\66\6f\6c\64\5f"
  "\74\72\75\6e\63\5f\73\5f\63\6f\6e\76\65\72\74\5f"
  "\75\00\0e\21\66\36\34\2e\69\36\34\2e\6e\6f\5f\66"
  "\6f\6c\64\5f\74\72\75\6e\63\5f\75\5f\63\6f\6e\76"
  "\65\72\74\5f\75\00\0f\0a\b1\81\80\80\00\10\86\80"
  "\80\80\00\00\20\00\a8\b2\0b\86\80\80\80\00\00\20"
  "\00\a9\b2\0b\86\80\80\80\00\00\20\00\a8\b3\0b\86"
  "\80\80\80\00\00\20\00\a9\b3\0b\86\80\80\80\00\00"
  "\20\00\aa\b7\0b\86\80\80\80\00\00\20\00\ab\b7\0b"
  "\86\80\80\80\00\00\20\00\aa\b8\0b\86\80\80\80\00"
  "\00\20\00\ab\b8\0b\86\80\80\80\00\00\20\00\ae\b4"
  "\0b\86\80\80\80\00\00\20\00\af\b4\0b\86\80\80\80"
  "\00\00\20\00\ae\b5\0b\86\80\80\80\00\00\20\00\af"
  "\b5\0b\86\80\80\80\00\00\20\00\b0\b9\0b\86\80\80"
  "\80\00\00\20\00\b1\b9\0b\86\80\80\80\00\00\20\00"
  "\b0\ba\0b\86\80\80\80\00\00\20\00\b1\ba\0b"
)
(module instance)
(assert_return
  (invoke "f32.i32.no_fold_trunc_s_convert_s" (f32.const 0x1.8p+0))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "f32.i32.no_fold_trunc_s_convert_s" (f32.const -0x1.8p+0))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "f32.i32.no_fold_trunc_u_convert_s" (f32.const 0x1.8p+0))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "f32.i32.no_fold_trunc_u_convert_s" (f32.const -0x1p-1))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.i32.no_fold_trunc_s_convert_u" (f32.const 0x1.8p+0))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "f32.i32.no_fold_trunc_s_convert_u" (f32.const -0x1.8p+0))
  (f32.const 0x1p+32)
)
(assert_return
  (invoke "f32.i32.no_fold_trunc_u_convert_u" (f32.const 0x1.8p+0))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "f32.i32.no_fold_trunc_u_convert_u" (f32.const -0x1p-1))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f64.i32.no_fold_trunc_s_convert_s" (f64.const 0x1.8p+0))
  (f64.const 0x1p+0)
)
(assert_return
  (invoke "f64.i32.no_fold_trunc_s_convert_s" (f64.const -0x1.8p+0))
  (f64.const -0x1p+0)
)
(assert_return
  (invoke "f64.i32.no_fold_trunc_u_convert_s" (f64.const 0x1.8p+0))
  (f64.const 0x1p+0)
)
(assert_return
  (invoke "f64.i32.no_fold_trunc_u_convert_s" (f64.const -0x1p-1))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.i32.no_fold_trunc_s_convert_u" (f64.const 0x1.8p+0))
  (f64.const 0x1p+0)
)
(assert_return
  (invoke "f64.i32.no_fold_trunc_s_convert_u" (f64.const -0x1.8p+0))
  (f64.const 0x1.ffff_fffep+31)
)
(assert_return
  (invoke "f64.i32.no_fold_trunc_u_convert_u" (f64.const 0x1.8p+0))
  (f64.const 0x1p+0)
)
(assert_return
  (invoke "f64.i32.no_fold_trunc_u_convert_u" (f64.const -0x1p-1))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f32.i64.no_fold_trunc_s_convert_s" (f32.const 0x1.8p+0))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "f32.i64.no_fold_trunc_s_convert_s" (f32.const -0x1.8p+0))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "f32.i64.no_fold_trunc_u_convert_s" (f32.const 0x1.8p+0))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "f32.i64.no_fold_trunc_u_convert_s" (f32.const -0x1p-1))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.i64.no_fold_trunc_s_convert_u" (f32.const 0x1.8p+0))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "f32.i64.no_fold_trunc_s_convert_u" (f32.const -0x1.8p+0))
  (f32.const 0x1p+64)
)
(assert_return
  (invoke "f32.i64.no_fold_trunc_u_convert_u" (f32.const 0x1.8p+0))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "f32.i64.no_fold_trunc_u_convert_u" (f32.const -0x1p-1))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f64.i64.no_fold_trunc_s_convert_s" (f64.const 0x1.8p+0))
  (f64.const 0x1p+0)
)
(assert_return
  (invoke "f64.i64.no_fold_trunc_s_convert_s" (f64.const -0x1.8p+0))
  (f64.const -0x1p+0)
)
(assert_return
  (invoke "f64.i64.no_fold_trunc_u_convert_s" (f64.const 0x1.8p+0))
  (f64.const 0x1p+0)
)
(assert_return
  (invoke "f64.i64.no_fold_trunc_u_convert_s" (f64.const -0x1p-1))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.i64.no_fold_trunc_s_convert_u" (f64.const 0x1.8p+0))
  (f64.const 0x1p+0)
)
(assert_return
  (invoke "f64.i64.no_fold_trunc_s_convert_u" (f64.const -0x1.8p+0))
  (f64.const 0x1p+64)
)
(assert_return
  (invoke "f64.i64.no_fold_trunc_u_convert_u" (f64.const 0x1.8p+0))
  (f64.const 0x1p+0)
)
(assert_return
  (invoke "f64.i64.no_fold_trunc_u_convert_u" (f64.const -0x1p-1))
  (f64.const 0x0p+0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\02\7f\7d\00\60\01\7f\01\7d\03\84\80\80\80\00\03"
  "\00\00\01\05\84\80\80\80\00\01\01\01\01\07\96\80"
  "\80\80\00\03\04\69\6e\69\74\00\00\03\72\75\6e\00"
  "\01\05\63\68\65\63\6b\00\02\0a\c5\80\80\80\00\03"
  "\89\80\80\80\00\00\20\00\20\01\38\02\00\0b\a5\80"
  "\80\80\00\01\01\7f\02\40\03\40\20\02\20\02\2a\02"
  "\00\20\01\95\38\02\00\20\02\41\04\6a\21\02\20\02"
  "\20\00\49\0d\00\0b\0b\0b\87\80\80\80\00\00\20\00"
  "\2a\02\00\0b"
)
(module instance)
(invoke "init" (i32.const 0x0) (f32.const 0x1.e333_34p+3))
(invoke "init" (i32.const 0x4) (f32.const 0x1.e666_66p+3))
(invoke "init" (i32.const 0x8) (f32.const 0x1.e999_9ap+3))
(invoke "init" (i32.const 0xc) (f32.const 0x1.eccc_ccp+3))
(assert_return (invoke "check" (i32.const 0x0)) (f32.const 0x1.e333_34p+3))
(assert_return (invoke "check" (i32.const 0x4)) (f32.const 0x1.e666_66p+3))
(assert_return (invoke "check" (i32.const 0x8)) (f32.const 0x1.e999_9ap+3))
(assert_return (invoke "check" (i32.const 0xc)) (f32.const 0x1.eccc_ccp+3))
(invoke "run" (i32.const 0x10) (f32.const 0x1.8p+1))
(assert_return (invoke "check" (i32.const 0x0)) (f32.const 0x1.4222_22p+2))
(assert_return (invoke "check" (i32.const 0x4)) (f32.const 0x1.4444_44p+2))
(assert_return (invoke "check" (i32.const 0x8)) (f32.const 0x1.4666_66p+2))
(assert_return (invoke "check" (i32.const 0xc)) (f32.const 0x1.4888_88p+2))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\02\7f\7c\00\60\01\7f\01\7c\03\84\80\80\80\00\03"
  "\00\00\01\05\84\80\80\80\00\01\01\01\01\07\96\80"
  "\80\80\00\03\04\69\6e\69\74\00\00\03\72\75\6e\00"
  "\01\05\63\68\65\63\6b\00\02\0a\c5\80\80\80\00\03"
  "\89\80\80\80\00\00\20\00\20\01\39\03\00\0b\a5\80"
  "\80\80\00\01\01\7f\02\40\03\40\20\02\20\02\2b\03"
  "\00\20\01\a3\39\03\00\20\02\41\08\6a\21\02\20\02"
  "\20\00\49\0d\00\0b\0b\0b\87\80\80\80\00\00\20\00"
  "\2b\03\00\0b"
)
(module instance)
(invoke "init" (i32.const 0x0) (f64.const 0x1.e333_3333_3333_3p+3))
(invoke "init" (i32.const 0x8) (f64.const 0x1.e666_6666_6666_6p+3))
(invoke "init" (i32.const 0x10) (f64.const 0x1.e999_9999_9999_ap+3))
(invoke "init" (i32.const 0x18) (f64.const 0x1.eccc_cccc_cccc_dp+3))
(assert_return
  (invoke "check" (i32.const 0x0))
  (f64.const 0x1.e333_3333_3333_3p+3)
)
(assert_return
  (invoke "check" (i32.const 0x8))
  (f64.const 0x1.e666_6666_6666_6p+3)
)
(assert_return
  (invoke "check" (i32.const 0x10))
  (f64.const 0x1.e999_9999_9999_ap+3)
)
(assert_return
  (invoke "check" (i32.const 0x18))
  (f64.const 0x1.eccc_cccc_cccc_dp+3)
)
(invoke "run" (i32.const 0x20) (f64.const 0x1.8p+1))
(assert_return
  (invoke "check" (i32.const 0x0))
  (f64.const 0x1.4222_2222_2222_2p+2)
)
(assert_return
  (invoke "check" (i32.const 0x8))
  (f64.const 0x1.4444_4444_4444_4p+2)
)
(assert_return
  (invoke "check" (i32.const 0x10))
  (f64.const 0x1.4666_6666_6666_7p+2)
)
(assert_return
  (invoke "check" (i32.const 0x18))
  (f64.const 0x1.4888_8888_8888_9p+2)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7f\60\02\7c\7c\01\7f\03\89\80\80\80"
  "\00\08\00\00\00\00\01\01\01\01\07\d1\80\80\80\00"
  "\08\07\66\33\32\2e\75\6c\74\00\00\07\66\33\32\2e"
  "\75\6c\65\00\01\07\66\33\32\2e\75\67\74\00\02\07"
  "\66\33\32\2e\75\67\65\00\03\07\66\36\34\2e\75\6c"
  "\74\00\04\07\66\36\34\2e\75\6c\65\00\05\07\66\36"
  "\34\2e\75\67\74\00\06\07\66\36\34\2e\75\67\65\00"
  "\07\0a\e9\80\80\80\00\08\88\80\80\80\00\00\20\00"
  "\20\01\60\45\0b\88\80\80\80\00\00\20\00\20\01\5e"
  "\45\0b\88\80\80\80\00\00\20\00\20\01\5f\45\0b\88"
  "\80\80\80\00\00\20\00\20\01\5d\45\0b\88\80\80\80"
  "\00\00\20\00\20\01\66\45\0b\88\80\80\80\00\00\20"
  "\00\20\01\64\45\0b\88\80\80\80\00\00\20\00\20\01"
  "\65\45\0b\88\80\80\80\00\00\20\00\20\01\63\45\0b"
)
(module instance)
(assert_return
  (invoke "f32.ult" (f32.const 0x1.8p+1) (f32.const 0x1p+1))
  (i32.const 0x0)
)
(assert_return
  (invoke "f32.ult" (f32.const 0x1p+1) (f32.const 0x1p+1))
  (i32.const 0x0)
)
(assert_return
  (invoke "f32.ult" (f32.const 0x1p+1) (f32.const 0x1.8p+1))
  (i32.const 0x1)
)
(assert_return
  (invoke "f32.ult" (f32.const 0x1p+1) (f32.const nan:0x40_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "f32.ule" (f32.const 0x1.8p+1) (f32.const 0x1p+1))
  (i32.const 0x0)
)
(assert_return
  (invoke "f32.ule" (f32.const 0x1p+1) (f32.const 0x1p+1))
  (i32.const 0x1)
)
(assert_return
  (invoke "f32.ule" (f32.const 0x1p+1) (f32.const 0x1.8p+1))
  (i32.const 0x1)
)
(assert_return
  (invoke "f32.ule" (f32.const 0x1p+1) (f32.const nan:0x40_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "f32.ugt" (f32.const 0x1.8p+1) (f32.const 0x1p+1))
  (i32.const 0x1)
)
(assert_return
  (invoke "f32.ugt" (f32.const 0x1p+1) (f32.const 0x1p+1))
  (i32.const 0x0)
)
(assert_return
  (invoke "f32.ugt" (f32.const 0x1p+1) (f32.const 0x1.8p+1))
  (i32.const 0x0)
)
(assert_return
  (invoke "f32.ugt" (f32.const 0x1p+1) (f32.const nan:0x40_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "f32.uge" (f32.const 0x1.8p+1) (f32.const 0x1p+1))
  (i32.const 0x1)
)
(assert_return
  (invoke "f32.uge" (f32.const 0x1p+1) (f32.const 0x1p+1))
  (i32.const 0x1)
)
(assert_return
  (invoke "f32.uge" (f32.const 0x1p+1) (f32.const 0x1.8p+1))
  (i32.const 0x0)
)
(assert_return
  (invoke "f32.uge" (f32.const 0x1p+1) (f32.const nan:0x40_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.ult" (f64.const 0x1.8p+1) (f64.const 0x1p+1))
  (i32.const 0x0)
)
(assert_return
  (invoke "f64.ult" (f64.const 0x1p+1) (f64.const 0x1p+1))
  (i32.const 0x0)
)
(assert_return
  (invoke "f64.ult" (f64.const 0x1p+1) (f64.const 0x1.8p+1))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.ult" (f64.const 0x1p+1) (f64.const nan:0x8_0000_0000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.ule" (f64.const 0x1.8p+1) (f64.const 0x1p+1))
  (i32.const 0x0)
)
(assert_return
  (invoke "f64.ule" (f64.const 0x1p+1) (f64.const 0x1p+1))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.ule" (f64.const 0x1p+1) (f64.const 0x1.8p+1))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.ule" (f64.const 0x1p+1) (f64.const nan:0x8_0000_0000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.ugt" (f64.const 0x1.8p+1) (f64.const 0x1p+1))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.ugt" (f64.const 0x1p+1) (f64.const 0x1p+1))
  (i32.const 0x0)
)
(assert_return
  (invoke "f64.ugt" (f64.const 0x1p+1) (f64.const 0x1.8p+1))
  (i32.const 0x0)
)
(assert_return
  (invoke "f64.ugt" (f64.const 0x1p+1) (f64.const nan:0x8_0000_0000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.uge" (f64.const 0x1.8p+1) (f64.const 0x1p+1))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.uge" (f64.const 0x1p+1) (f64.const 0x1p+1))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.uge" (f64.const 0x1p+1) (f64.const 0x1.8p+1))
  (i32.const 0x0)
)
(assert_return
  (invoke "f64.uge" (f64.const 0x1p+1) (f64.const nan:0x8_0000_0000_0000))
  (i32.const 0x1)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\89\80\80\80"
  "\00\08\00\00\00\00\01\01\01\01\07\c1\81\80\80\00"
  "\08\15\66\33\32\2e\6e\6f\5f\66\6f\6c\64\5f\6c\74"
  "\5f\73\65\6c\65\63\74\00\00\15\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\6c\65\5f\73\65\6c\65\63\74\00"
  "\01\15\66\33\32\2e\6e\6f\5f\66\6f\6c\64\5f\67\74"
  "\5f\73\65\6c\65\63\74\00\02\15\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\67\65\5f\73\65\6c\65\63\74\00"
  "\03\15\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\6c\74"
  "\5f\73\65\6c\65\63\74\00\04\15\66\36\34\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\6c\65\5f\73\65\6c\65\63\74\00"
  "\05\15\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\67\74"
  "\5f\73\65\6c\65\63\74\00\06\15\66\36\34\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\67\65\5f\73\65\6c\65\63\74\00"
  "\07\0a\89\81\80\80\00\08\8c\80\80\80\00\00\20\00"
  "\20\01\20\00\20\01\5d\1b\0b\8c\80\80\80\00\00\20"
  "\00\20\01\20\00\20\01\5f\1b\0b\8c\80\80\80\00\00"
  "\20\00\20\01\20\00\20\01\5e\1b\0b\8c\80\80\80\00"
  "\00\20\00\20\01\20\00\20\01\60\1b\0b\8c\80\80\80"
  "\00\00\20\00\20\01\20\00\20\01\63\1b\0b\8c\80\80"
  "\80\00\00\20\00\20\01\20\00\20\01\65\1b\0b\8c\80"
  "\80\80\00\00\20\00\20\01\20\00\20\01\64\1b\0b\8c"
  "\80\80\80\00\00\20\00\20\01\20\00\20\01\66\1b\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_lt_select" (f32.const 0x0p+0) (f32.const nan:0x40_0000))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "f32.no_fold_lt_select" (f32.const nan:0x40_0000) (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_lt_select" (f32.const 0x0p+0) (f32.const -0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_lt_select" (f32.const -0x0p+0) (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_le_select" (f32.const 0x0p+0) (f32.const nan:0x40_0000))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "f32.no_fold_le_select" (f32.const nan:0x40_0000) (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_le_select" (f32.const 0x0p+0) (f32.const -0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_le_select" (f32.const -0x0p+0) (f32.const 0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_gt_select" (f32.const 0x0p+0) (f32.const nan:0x40_0000))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "f32.no_fold_gt_select" (f32.const nan:0x40_0000) (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_gt_select" (f32.const 0x0p+0) (f32.const -0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_gt_select" (f32.const -0x0p+0) (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_ge_select" (f32.const 0x0p+0) (f32.const nan:0x40_0000))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "f32.no_fold_ge_select" (f32.const nan:0x40_0000) (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_ge_select" (f32.const 0x0p+0) (f32.const -0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_ge_select" (f32.const -0x0p+0) (f32.const 0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_lt_select"
    (f64.const 0x0p+0)
    (f64.const nan:0x8_0000_0000_0000)
  )
  (f64.const nan:0x8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_lt_select"
    (f64.const nan:0x8_0000_0000_0000)
    (f64.const 0x0p+0)
  )
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_lt_select" (f64.const 0x0p+0) (f64.const -0x0p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_lt_select" (f64.const -0x0p+0) (f64.const 0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_le_select"
    (f64.const 0x0p+0)
    (f64.const nan:0x8_0000_0000_0000)
  )
  (f64.const nan:0x8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_le_select"
    (f64.const nan:0x8_0000_0000_0000)
    (f64.const 0x0p+0)
  )
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_le_select" (f64.const 0x0p+0) (f64.const -0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_le_select" (f64.const -0x0p+0) (f64.const 0x0p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_gt_select"
    (f64.const 0x0p+0)
    (f64.const nan:0x8_0000_0000_0000)
  )
  (f64.const nan:0x8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_gt_select"
    (f64.const nan:0x8_0000_0000_0000)
    (f64.const 0x0p+0)
  )
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_gt_select" (f64.const 0x0p+0) (f64.const -0x0p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_gt_select" (f64.const -0x0p+0) (f64.const 0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_ge_select"
    (f64.const 0x0p+0)
    (f64.const nan:0x8_0000_0000_0000)
  )
  (f64.const nan:0x8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_ge_select"
    (f64.const nan:0x8_0000_0000_0000)
    (f64.const 0x0p+0)
  )
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_ge_select" (f64.const 0x0p+0) (f64.const -0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_ge_select" (f64.const -0x0p+0) (f64.const 0x0p+0))
  (f64.const -0x0p+0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\89\80\80\80"
  "\00\08\00\00\00\00\01\01\01\01\07\a1\81\80\80\00"
  "\08\11\66\33\32\2e\6e\6f\5f\66\6f\6c\64\5f\6c\74"
  "\5f\69\66\00\00\11\66\33\32\2e\6e\6f\5f\66\6f\6c"
  "\64\5f\6c\65\5f\69\66\00\01\11\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\67\74\5f\69\66\00\02\11\66\33"
  "\32\2e\6e\6f\5f\66\6f\6c\64\5f\67\65\5f\69\66\00"
  "\03\11\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\6c\74"
  "\5f\69\66\00\04\11\66\36\34\2e\6e\6f\5f\66\6f\6c"
  "\64\5f\6c\65\5f\69\66\00\05\11\66\36\34\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\67\74\5f\69\66\00\06\11\66\36"
  "\34\2e\6e\6f\5f\66\6f\6c\64\5f\67\65\5f\69\66\00"
  "\07\0a\a1\81\80\80\00\08\8f\80\80\80\00\00\20\00"
  "\20\01\5d\04\7d\20\00\05\20\01\0b\0b\8f\80\80\80"
  "\00\00\20\00\20\01\5f\04\7d\20\00\05\20\01\0b\0b"
  "\8f\80\80\80\00\00\20\00\20\01\5e\04\7d\20\00\05"
  "\20\01\0b\0b\8f\80\80\80\00\00\20\00\20\01\60\04"
  "\7d\20\00\05\20\01\0b\0b\8f\80\80\80\00\00\20\00"
  "\20\01\63\04\7c\20\00\05\20\01\0b\0b\8f\80\80\80"
  "\00\00\20\00\20\01\65\04\7c\20\00\05\20\01\0b\0b"
  "\8f\80\80\80\00\00\20\00\20\01\64\04\7c\20\00\05"
  "\20\01\0b\0b\8f\80\80\80\00\00\20\00\20\01\66\04"
  "\7c\20\00\05\20\01\0b\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_lt_if" (f32.const 0x0p+0) (f32.const nan:0x40_0000))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "f32.no_fold_lt_if" (f32.const nan:0x40_0000) (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_lt_if" (f32.const 0x0p+0) (f32.const -0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_lt_if" (f32.const -0x0p+0) (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_le_if" (f32.const 0x0p+0) (f32.const nan:0x40_0000))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "f32.no_fold_le_if" (f32.const nan:0x40_0000) (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_le_if" (f32.const 0x0p+0) (f32.const -0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_le_if" (f32.const -0x0p+0) (f32.const 0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_gt_if" (f32.const 0x0p+0) (f32.const nan:0x40_0000))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "f32.no_fold_gt_if" (f32.const nan:0x40_0000) (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_gt_if" (f32.const 0x0p+0) (f32.const -0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_gt_if" (f32.const -0x0p+0) (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_ge_if" (f32.const 0x0p+0) (f32.const nan:0x40_0000))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "f32.no_fold_ge_if" (f32.const nan:0x40_0000) (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_ge_if" (f32.const 0x0p+0) (f32.const -0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_ge_if" (f32.const -0x0p+0) (f32.const 0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_lt_if"
    (f64.const 0x0p+0)
    (f64.const nan:0x8_0000_0000_0000)
  )
  (f64.const nan:0x8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_lt_if"
    (f64.const nan:0x8_0000_0000_0000)
    (f64.const 0x0p+0)
  )
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_lt_if" (f64.const 0x0p+0) (f64.const -0x0p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_lt_if" (f64.const -0x0p+0) (f64.const 0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_le_if"
    (f64.const 0x0p+0)
    (f64.const nan:0x8_0000_0000_0000)
  )
  (f64.const nan:0x8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_le_if"
    (f64.const nan:0x8_0000_0000_0000)
    (f64.const 0x0p+0)
  )
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_le_if" (f64.const 0x0p+0) (f64.const -0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_le_if" (f64.const -0x0p+0) (f64.const 0x0p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_gt_if"
    (f64.const 0x0p+0)
    (f64.const nan:0x8_0000_0000_0000)
  )
  (f64.const nan:0x8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_gt_if"
    (f64.const nan:0x8_0000_0000_0000)
    (f64.const 0x0p+0)
  )
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_gt_if" (f64.const 0x0p+0) (f64.const -0x0p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_gt_if" (f64.const -0x0p+0) (f64.const 0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_ge_if"
    (f64.const 0x0p+0)
    (f64.const nan:0x8_0000_0000_0000)
  )
  (f64.const nan:0x8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_ge_if"
    (f64.const nan:0x8_0000_0000_0000)
    (f64.const 0x0p+0)
  )
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_ge_if" (f64.const 0x0p+0) (f64.const -0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_ge_if" (f64.const -0x0p+0) (f64.const 0x0p+0))
  (f64.const -0x0p+0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\89\80\80\80\00\08"
  "\00\00\00\00\01\01\01\01\07\f9\81\80\80\00\08\1c"
  "\66\33\32\2e\6e\6f\5f\66\6f\6c\64\5f\6c\74\5f\73"
  "\65\6c\65\63\74\5f\74\6f\5f\61\62\73\00\00\1c\66"
  "\33\32\2e\6e\6f\5f\66\6f\6c\64\5f\6c\65\5f\73\65"
  "\6c\65\63\74\5f\74\6f\5f\61\62\73\00\01\1c\66\33"
  "\32\2e\6e\6f\5f\66\6f\6c\64\5f\67\74\5f\73\65\6c"
  "\65\63\74\5f\74\6f\5f\61\62\73\00\02\1c\66\33\32"
  "\2e\6e\6f\5f\66\6f\6c\64\5f\67\65\5f\73\65\6c\65"
  "\63\74\5f\74\6f\5f\61\62\73\00\03\1c\66\36\34\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\6c\74\5f\73\65\6c\65\63"
  "\74\5f\74\6f\5f\61\62\73\00\04\1c\66\36\34\2e\6e"
  "\6f\5f\66\6f\6c\64\5f\6c\65\5f\73\65\6c\65\63\74"
  "\5f\74\6f\5f\61\62\73\00\05\1c\66\36\34\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\67\74\5f\73\65\6c\65\63\74\5f"
  "\74\6f\5f\61\62\73\00\06\1c\66\36\34\2e\6e\6f\5f"
  "\66\6f\6c\64\5f\67\65\5f\73\65\6c\65\63\74\5f\74"
  "\6f\5f\61\62\73\00\07\0a\b9\81\80\80\00\08\90\80"
  "\80\80\00\00\20\00\8c\20\00\20\00\43\00\00\00\00"
  "\5d\1b\0b\90\80\80\80\00\00\20\00\8c\20\00\20\00"
  "\43\00\00\00\80\5f\1b\0b\90\80\80\80\00\00\20\00"
  "\20\00\8c\20\00\43\00\00\00\80\5e\1b\0b\90\80\80"
  "\80\00\00\20\00\20\00\8c\20\00\43\00\00\00\00\60"
  "\1b\0b\94\80\80\80\00\00\20\00\9a\20\00\20\00\44"
  "\00\00\00\00\00\00\00\00\63\1b\0b\94\80\80\80\00"
  "\00\20\00\9a\20\00\20\00\44\00\00\00\00\00\00\00"
  "\80\65\1b\0b\94\80\80\80\00\00\20\00\20\00\9a\20"
  "\00\44\00\00\00\00\00\00\00\80\64\1b\0b\94\80\80"
  "\80\00\00\20\00\20\00\9a\20\00\44\00\00\00\00\00"
  "\00\00\00\66\1b\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_lt_select_to_abs" (f32.const nan:0x20_0000))
  (f32.const nan:0x20_0000)
)
(assert_return
  (invoke "f32.no_fold_lt_select_to_abs" (f32.const -nan:0x40_0000))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "f32.no_fold_lt_select_to_abs" (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_lt_select_to_abs" (f32.const -0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_le_select_to_abs" (f32.const nan:0x20_0000))
  (f32.const nan:0x20_0000)
)
(assert_return
  (invoke "f32.no_fold_le_select_to_abs" (f32.const -nan:0x40_0000))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "f32.no_fold_le_select_to_abs" (f32.const 0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_le_select_to_abs" (f32.const -0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_gt_select_to_abs" (f32.const nan:0x20_0000))
  (f32.const -nan:0x20_0000)
)
(assert_return
  (invoke "f32.no_fold_gt_select_to_abs" (f32.const -nan:0x40_0000))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "f32.no_fold_gt_select_to_abs" (f32.const 0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_gt_select_to_abs" (f32.const -0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_ge_select_to_abs" (f32.const nan:0x20_0000))
  (f32.const -nan:0x20_0000)
)
(assert_return
  (invoke "f32.no_fold_ge_select_to_abs" (f32.const -nan:0x40_0000))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "f32.no_fold_ge_select_to_abs" (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_ge_select_to_abs" (f32.const -0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_lt_select_to_abs" (f64.const nan:0x4_0000_0000_0000))
  (f64.const nan:0x4_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_lt_select_to_abs" (f64.const -nan:0x8_0000_0000_0000))
  (f64.const -nan:0x8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_lt_select_to_abs" (f64.const 0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_lt_select_to_abs" (f64.const -0x0p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_le_select_to_abs" (f64.const nan:0x4_0000_0000_0000))
  (f64.const nan:0x4_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_le_select_to_abs" (f64.const -nan:0x8_0000_0000_0000))
  (f64.const -nan:0x8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_le_select_to_abs" (f64.const 0x0p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_le_select_to_abs" (f64.const -0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_gt_select_to_abs" (f64.const nan:0x4_0000_0000_0000))
  (f64.const -nan:0x4_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_gt_select_to_abs" (f64.const -nan:0x8_0000_0000_0000))
  (f64.const nan:0x8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_gt_select_to_abs" (f64.const 0x0p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_gt_select_to_abs" (f64.const -0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_ge_select_to_abs" (f64.const nan:0x4_0000_0000_0000))
  (f64.const -nan:0x4_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_ge_select_to_abs" (f64.const -nan:0x8_0000_0000_0000))
  (f64.const nan:0x8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_ge_select_to_abs" (f64.const 0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_ge_select_to_abs" (f64.const -0x0p+0))
  (f64.const -0x0p+0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\89\80\80\80\00\08"
  "\00\00\00\00\01\01\01\01\07\d9\81\80\80\00\08\18"
  "\66\33\32\2e\6e\6f\5f\66\6f\6c\64\5f\6c\74\5f\69"
  "\66\5f\74\6f\5f\61\62\73\00\00\18\66\33\32\2e\6e"
  "\6f\5f\66\6f\6c\64\5f\6c\65\5f\69\66\5f\74\6f\5f"
  "\61\62\73\00\01\18\66\33\32\2e\6e\6f\5f\66\6f\6c"
  "\64\5f\67\74\5f\69\66\5f\74\6f\5f\61\62\73\00\02"
  "\18\66\33\32\2e\6e\6f\5f\66\6f\6c\64\5f\67\65\5f"
  "\69\66\5f\74\6f\5f\61\62\73\00\03\18\66\36\34\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\6c\74\5f\69\66\5f\74\6f"
  "\5f\61\62\73\00\04\18\66\36\34\2e\6e\6f\5f\66\6f"
  "\6c\64\5f\6c\65\5f\69\66\5f\74\6f\5f\61\62\73\00"
  "\05\18\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\67\74"
  "\5f\69\66\5f\74\6f\5f\61\62\73\00\06\18\66\36\34"
  "\2e\6e\6f\5f\66\6f\6c\64\5f\67\65\5f\69\66\5f\74"
  "\6f\5f\61\62\73\00\07\0a\d1\81\80\80\00\08\93\80"
  "\80\80\00\00\20\00\43\00\00\00\00\5d\04\7d\20\00"
  "\8c\05\20\00\0b\0b\93\80\80\80\00\00\20\00\43\00"
  "\00\00\80\5f\04\7d\20\00\8c\05\20\00\0b\0b\93\80"
  "\80\80\00\00\20\00\43\00\00\00\80\5e\04\7d\20\00"
  "\05\20\00\8c\0b\0b\93\80\80\80\00\00\20\00\43\00"
  "\00\00\00\60\04\7d\20\00\05\20\00\8c\0b\0b\97\80"
  "\80\80\00\00\20\00\44\00\00\00\00\00\00\00\00\63"
  "\04\7c\20\00\9a\05\20\00\0b\0b\97\80\80\80\00\00"
  "\20\00\44\00\00\00\00\00\00\00\80\65\04\7c\20\00"
  "\9a\05\20\00\0b\0b\97\80\80\80\00\00\20\00\44\00"
  "\00\00\00\00\00\00\80\64\04\7c\20\00\05\20\00\9a"
  "\0b\0b\97\80\80\80\00\00\20\00\44\00\00\00\00\00"
  "\00\00\00\66\04\7c\20\00\05\20\00\9a\0b\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_lt_if_to_abs" (f32.const nan:0x20_0000))
  (f32.const nan:0x20_0000)
)
(assert_return
  (invoke "f32.no_fold_lt_if_to_abs" (f32.const -nan:0x40_0000))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "f32.no_fold_lt_if_to_abs" (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_lt_if_to_abs" (f32.const -0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_le_if_to_abs" (f32.const nan:0x20_0000))
  (f32.const nan:0x20_0000)
)
(assert_return
  (invoke "f32.no_fold_le_if_to_abs" (f32.const -nan:0x40_0000))
  (f32.const -nan:0x40_0000)
)
(assert_return
  (invoke "f32.no_fold_le_if_to_abs" (f32.const 0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_le_if_to_abs" (f32.const -0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_gt_if_to_abs" (f32.const nan:0x20_0000))
  (f32.const -nan:0x20_0000)
)
(assert_return
  (invoke "f32.no_fold_gt_if_to_abs" (f32.const -nan:0x40_0000))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "f32.no_fold_gt_if_to_abs" (f32.const 0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_gt_if_to_abs" (f32.const -0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_ge_if_to_abs" (f32.const nan:0x20_0000))
  (f32.const -nan:0x20_0000)
)
(assert_return
  (invoke "f32.no_fold_ge_if_to_abs" (f32.const -nan:0x40_0000))
  (f32.const nan:0x40_0000)
)
(assert_return
  (invoke "f32.no_fold_ge_if_to_abs" (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_ge_if_to_abs" (f32.const -0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_lt_if_to_abs" (f64.const nan:0x4_0000_0000_0000))
  (f64.const nan:0x4_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_lt_if_to_abs" (f64.const -nan:0x8_0000_0000_0000))
  (f64.const -nan:0x8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_lt_if_to_abs" (f64.const 0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_lt_if_to_abs" (f64.const -0x0p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_le_if_to_abs" (f64.const nan:0x4_0000_0000_0000))
  (f64.const nan:0x4_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_le_if_to_abs" (f64.const -nan:0x8_0000_0000_0000))
  (f64.const -nan:0x8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_le_if_to_abs" (f64.const 0x0p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_le_if_to_abs" (f64.const -0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_gt_if_to_abs" (f64.const nan:0x4_0000_0000_0000))
  (f64.const -nan:0x4_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_gt_if_to_abs" (f64.const -nan:0x8_0000_0000_0000))
  (f64.const nan:0x8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_gt_if_to_abs" (f64.const 0x0p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_gt_if_to_abs" (f64.const -0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_ge_if_to_abs" (f64.const nan:0x4_0000_0000_0000))
  (f64.const -nan:0x4_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_ge_if_to_abs" (f64.const -nan:0x8_0000_0000_0000))
  (f64.const nan:0x8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_ge_if_to_abs" (f64.const 0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_ge_if_to_abs" (f64.const -0x0p+0))
  (f64.const -0x0p+0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\60"
  "\00\01\7d\60\00\01\7c\03\83\80\80\80\00\02\00\01"
  "\07\b7\80\80\80\00\02\18\66\33\32\2e\69\6e\63\6f"
  "\72\72\65\63\74\5f\63\6f\72\72\65\63\74\69\6f\6e"
  "\00\00\18\66\36\34\2e\69\6e\63\6f\72\72\65\63\74"
  "\5f\63\6f\72\72\65\63\74\69\6f\6e\00\01\0a\cd\80"
  "\80\80\00\02\99\80\80\80\00\00\43\be\9f\aa\3f\43"
  "\cd\cc\9c\3f\92\43\be\9f\aa\3f\93\43\cd\cc\9c\3f"
  "\93\0b\a9\80\80\80\00\00\44\87\16\d9\ce\f7\53\f5"
  "\3f\44\9a\99\99\99\99\99\f3\3f\a0\44\87\16\d9\ce"
  "\f7\53\f5\3f\a1\44\9a\99\99\99\99\99\f3\3f\a1\0b"
)
(module instance)
(assert_return (invoke "f32.incorrect_correction") (f32.const 0x1p-23))
(assert_return (invoke "f64.incorrect_correction") (f64.const -0x1p-52))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\00\01\7d\03\82\80\80\80\00\01\00\07\8d\80\80\80"
  "\00\01\09\63\61\6c\63\75\6c\61\74\65\00\00\0a\ce"
  "\80\80\80\00\01\c8\80\80\80\00\01\05\7d\43\00\40"
  "\1c\43\21\00\43\55\55\50\43\21\01\43\15\c7\e3\3f"
  "\21\02\20\01\8c\20\00\94\20\00\20\02\94\20\01\93"
  "\95\21\03\20\01\8c\20\00\94\20\00\20\02\94\20\01"
  "\93\95\21\04\02\40\20\03\20\04\5b\0d\00\00\0b\20"
  "\04\0b"
)
(module instance)
(assert_return (invoke "calculate") (f32.const -0x1.d2ed_46p+8))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\00\01\7c\03\82\80\80\80\00\01\00\07\8d\80\80\80"
  "\00\01\09\63\61\6c\63\75\6c\61\74\65\00\00\0a\da"
  "\80\80\80\00\01\d4\80\80\80\00\01\05\7c\44\00\00"
  "\00\00\00\88\63\40\21\00\44\4b\06\ab\aa\aa\0a\6a"
  "\40\21\01\44\96\d0\e3\aa\e2\78\fc\3f\21\02\20\01"
  "\9a\20\00\a2\20\00\20\02\a2\20\01\a1\a3\21\03\20"
  "\01\9a\20\00\a2\20\00\20\02\a2\20\01\a1\a3\21\04"
  "\02\40\20\03\20\04\61\0d\00\00\0b\20\04\0b"
)
(module instance)
(assert_return (invoke "calculate") (f64.const -0x1.d2ed_4d02_18c9_3p+8))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7d\01\7d\03\82\80\80\80\00\01\00\07\90\80\80"
  "\80\00\01\0c\6c\6c\76\6d\5f\70\72\32\36\37\34\36"
  "\00\00\0a\96\80\80\80\00\01\90\80\80\80\00\00\43"
  "\00\00\00\00\43\00\00\00\80\20\00\93\93\0b"
)
(module instance)
(assert_return (invoke "llvm_pr26746" (f32.const -0x0p+0)) (f32.const 0x0p+0))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7f\01\7d\03\82\80\80\80\00\01\00\07\90\80\80"
  "\80\00\01\0c\6c\6c\76\6d\5f\70\72\32\37\31\35\33"
  "\00\00\0a\98\80\80\80\00\01\92\80\80\80\00\00\20"
  "\00\41\ff\ff\ff\ff\00\71\b2\43\00\00\00\cb\92\0b"
)
(module instance)
(assert_return
  (invoke "llvm_pr27153" (i32.const 0x200_0002))
  (f32.const 0x1.8p+24)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\87\80\80\80\00\01\60"
  "\02\7f\7f\01\7d\03\82\80\80\80\00\01\00\07\90\80"
  "\80\80\00\01\0c\6c\6c\76\6d\5f\70\72\32\37\30\33"
  "\36\00\00\0a\9b\80\80\80\00\01\95\80\80\80\00\00"
  "\20\00\41\cb\ff\87\74\72\b2\20\01\41\80\80\90\07"
  "\71\b2\92\0b"
)
(module instance)
(assert_return
  (invoke "llvm_pr27036" (i32.const 0xfe81_ffcb) (i32.const 0xe4_0000))
  (f32.const -0x1.3400_68p+23)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\97\80\80\80\00\03\60"
  "\04\7c\7c\7c\7c\01\7c\60\03\7c\7c\7c\01\7c\60\03"
  "\7d\7d\7d\01\7d\03\84\80\80\80\00\03\00\01\02\07"
  "\a2\80\80\80\00\03\08\74\68\65\70\61\73\74\30\00"
  "\00\08\74\68\65\70\61\73\74\31\00\01\08\74\68\65"
  "\70\61\73\74\32\00\02\0a\b1\80\80\80\00\03\8d\80"
  "\80\80\00\00\20\00\20\01\a2\20\02\20\03\a2\a3\0b"
  "\8a\80\80\80\00\00\20\00\20\01\a2\20\02\a1\0b\8a"
  "\80\80\80\00\00\20\00\20\01\94\20\02\94\0b"
)
(module instance)
(assert_return
  (invoke "thepast0"
    (f64.const 0x1p-1021)
    (f64.const 0x1.ffff_ffff_ffff_fp-1)
    (f64.const 0x1p+1)
    (f64.const 0x1p-1)
  )
  (f64.const 0x1.ffff_ffff_ffff_fp-1022)
)
(assert_return
  (invoke "thepast1"
    (f64.const 0x1p-54)
    (f64.const 0x1.ffff_ffff_ffff_fp-1)
    (f64.const 0x1p-54)
  )
  (f64.const -0x1p-107)
)
(assert_return
  (invoke "thepast2" (f32.const 0x1p-125) (f32.const 0x1p-1) (f32.const 0x1p+0))
  (f32.const 0x1p-126)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7d\01\7d\03\82\80\80\80\00\01\00\07\8b\80\80"
  "\80\00\01\07\69\6e\76\65\72\73\65\00\00\0a\90\80"
  "\80\80\00\01\8a\80\80\80\00\00\43\00\00\80\3f\20"
  "\00\95\0b"
)
(module instance)
(assert_return
  (invoke "inverse" (f32.const 0x1.8p+6))
  (f32.const 0x1.5555_56p-7)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\a7\80\80\80\00\02\10\66\33\32\5f\73\71"
  "\72\74\5f\6d\69\6e\75\73\5f\32\00\00\10\66\36\34"
  "\5f\73\71\72\74\5f\6d\69\6e\75\73\5f\32\00\01\0a"
  "\a5\80\80\80\00\02\8b\80\80\80\00\00\20\00\91\43"
  "\00\00\00\40\93\0b\8f\80\80\80\00\00\20\00\9f\44"
  "\00\00\00\00\00\00\00\40\a1\0b"
)
(module instance)
(assert_return
  (invoke "f32_sqrt_minus_2" (f32.const 0x1p+2))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f64_sqrt_minus_2" (f64.const 0x1p+2))
  (f64.const 0x0p+0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\b5\80\80\80\00\02\17\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\72\65\63\69\70\5f\72\65\63\69"
  "\70\00\00\17\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f"
  "\72\65\63\69\70\5f\72\65\63\69\70\00\01\0a\b3\80"
  "\80\80\00\02\90\80\80\80\00\00\43\00\00\80\3f\43"
  "\00\00\80\3f\20\00\95\95\0b\98\80\80\80\00\00\44"
  "\00\00\00\00\00\00\f0\3f\44\00\00\00\00\00\00\f0"
  "\3f\20\00\a3\a3\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_recip_recip" (f32.const -0x1.e8bf_18p+65))
  (f32.const -0x1.e8bf_16p+65)
)
(assert_return
  (invoke "f32.no_fold_recip_recip" (f32.const 0x1.e242_48p-77))
  (f32.const 0x1.e242_46p-77)
)
(assert_return
  (invoke "f32.no_fold_recip_recip" (f32.const 0x1.caf0_e8p-64))
  (f32.const 0x1.caf0_eap-64)
)
(assert_return
  (invoke "f32.no_fold_recip_recip" (f32.const -0x1.e669_82p+4))
  (f32.const -0x1.e669_84p+4)
)
(assert_return
  (invoke "f32.no_fold_recip_recip" (f32.const 0x1.f999_16p+70))
  (f32.const 0x1.f999_14p+70)
)
(assert_return
  (invoke "f32.no_fold_recip_recip" (f32.const -0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_recip_recip" (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_recip_recip" (f32.const -inf))
  (f32.const -inf)
)
(assert_return
  (invoke "f32.no_fold_recip_recip" (f32.const inf))
  (f32.const inf)
)
(assert_return
  (invoke "f64.no_fold_recip_recip" (f64.const -0x1.d812_48dd_a63dp+148))
  (f64.const -0x1.d812_48dd_a63d_1p+148)
)
(assert_return
  (invoke "f64.no_fold_recip_recip" (f64.const -0x1.f475_0312_039e_3p+66))
  (f64.const -0x1.f475_0312_039e_2p+66)
)
(assert_return
  (invoke "f64.no_fold_recip_recip" (f64.const 0x1.fa50_630e_ec7f_6p+166))
  (f64.const 0x1.fa50_630e_ec7f_5p+166)
)
(assert_return
  (invoke "f64.no_fold_recip_recip" (f64.const 0x1.db05_9861_7ba9_2p-686))
  (f64.const 0x1.db05_9861_7ba9_1p-686)
)
(assert_return
  (invoke "f64.no_fold_recip_recip" (f64.const 0x1.85f1_638a_0c82_bp+902))
  (f64.const 0x1.85f1_638a_0c82_ap+902)
)
(assert_return
  (invoke "f64.no_fold_recip_recip" (f64.const -0x0p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_recip_recip" (f64.const 0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_recip_recip" (f64.const -inf))
  (f64.const -inf)
)
(assert_return
  (invoke "f64.no_fold_recip_recip" (f64.const inf))
  (f64.const inf)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\83\80\80\80"
  "\00\02\00\01\07\bb\80\80\80\00\02\1a\66\33\32\2e"
  "\6e\6f\5f\61\6c\67\65\62\72\61\69\63\5f\66\61\63"
  "\74\6f\72\69\6e\67\00\00\1a\66\36\34\2e\6e\6f\5f"
  "\61\6c\67\65\62\72\61\69\63\5f\66\61\63\74\6f\72"
  "\69\6e\67\00\01\0a\a5\80\80\80\00\02\8d\80\80\80"
  "\00\00\20\00\20\01\92\20\00\20\01\93\94\0b\8d\80"
  "\80\80\00\00\20\00\20\01\a0\20\00\20\01\a1\a2\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_algebraic_factoring"
    (f32.const -0x1.ef67_8ep-55)
    (f32.const 0x1.c160_b8p-54)
  )
  (f32.const -0x1.1294_02p-107)
)
(assert_return
  (invoke "f32.no_algebraic_factoring"
    (f32.const -0x1.2d76_bcp+24)
    (f32.const 0x1.f408_9cp+24)
  )
  (f32.const -0x1.36d8_9ap+49)
)
(assert_return
  (invoke "f32.no_algebraic_factoring"
    (f32.const 0x1.7ca2_b2p+45)
    (f32.const -0x1.0851_3cp+47)
  )
  (f32.const -0x1.db10_dep+93)
)
(assert_return
  (invoke "f32.no_algebraic_factoring"
    (f32.const 0x1.7d5e_3p+17)
    (f32.const -0x1.c783_b4p+7)
  )
  (f32.const 0x1.1c10_a6p+35)
)
(assert_return
  (invoke "f32.no_algebraic_factoring"
    (f32.const -0x1.daf9_6p+7)
    (f32.const -0x1.dac6_bp+19)
  )
  (f32.const -0x1.b842_2ep+39)
)
(assert_return
  (invoke "f64.no_algebraic_factoring"
    (f64.const 0x1.e17c_0a02_ac6b_5p-476)
    (f64.const 0x1.e8f1_3f1f_cdc1_4p-463)
  )
  (f64.const -0x1.d2ec_518f_6286_3p-925)
)
(assert_return
  (invoke "f64.no_algebraic_factoring"
    (f64.const 0x1.971b_55a5_7e3a_3p-377)
    (f64.const 0x1.edeb_4233_c1b2_7p-399)
  )
  (f64.const 0x1.43b3_f69f_b258_bp-753)
)
(assert_return
  (invoke "f64.no_algebraic_factoring"
    (f64.const -0x1.c3b9_dc02_472f_ap-378)
    (f64.const -0x1.74e9_faeb_aff1_4p-369)
  )
  (f64.const -0x1.0f9c_07e8_caa2_5p-737)
)
(assert_return
  (invoke "f64.no_algebraic_factoring"
    (f64.const -0x1.afaf_4688_ed01_9p+179)
    (f64.const 0x1.b071_71cb_49e9_4p+188)
  )
  (f64.const -0x1.6d3f_2e2b_ebcf_7p+377)
)
(assert_return
  (invoke "f64.no_algebraic_factoring"
    (f64.const 0x1.4377_a989_48f1_2p+114)
    (f64.const -0x1.500c_05bd_24c9_7p+90)
  )
  (f64.const 0x1.98b7_2dbf_7bf7_2p+228)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\83\80\80\80"
  "\00\02\00\01\07\bb\80\80\80\00\02\1a\66\33\32\2e"
  "\6e\6f\5f\61\6c\67\65\62\72\61\69\63\5f\66\61\63"
  "\74\6f\72\69\6e\67\00\00\1a\66\36\34\2e\6e\6f\5f"
  "\61\6c\67\65\62\72\61\69\63\5f\66\61\63\74\6f\72"
  "\69\6e\67\00\01\0a\a5\80\80\80\00\02\8d\80\80\80"
  "\00\00\20\00\20\00\94\20\01\20\01\94\93\0b\8d\80"
  "\80\80\00\00\20\00\20\00\a2\20\01\20\01\a2\a1\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_algebraic_factoring"
    (f32.const 0x1.8e2c_14p-46)
    (f32.const 0x1.bad5_9ap-39)
  )
  (f32.const -0x1.7efe_5p-77)
)
(assert_return
  (invoke "f32.no_algebraic_factoring"
    (f32.const -0x1.7ef1_92p+41)
    (f32.const -0x1.db18_4ap+33)
  )
  (f32.const 0x1.1e69_32p+83)
)
(assert_return
  (invoke "f32.no_algebraic_factoring"
    (f32.const 0x1.7eb4_58p-12)
    (f32.const -0x1.52c4_98p-13)
  )
  (f32.const 0x1.cc0b_c6p-24)
)
(assert_return
  (invoke "f32.no_algebraic_factoring"
    (f32.const 0x1.2675_c6p-44)
    (f32.const -0x1.edd3_1ap-46)
  )
  (f32.const 0x1.1729_4cp-88)
)
(assert_return
  (invoke "f32.no_algebraic_factoring"
    (f32.const 0x1.9a5f_92p+51)
    (f32.const -0x1.2b00_98p+52)
  )
  (f32.const -0x1.7189_a6p+103)
)
(assert_return
  (invoke "f64.no_algebraic_factoring"
    (f64.const 0x1.749a_128f_18f6_9p+356)
    (f64.const -0x1.0bc9_7ee1_354e_1p+337)
  )
  (f64.const 0x1.0f28_1155_18d7_4p+713)
)
(assert_return
  (invoke "f64.no_algebraic_factoring"
    (f64.const -0x1.2dab_01b2_215e_ap+309)
    (f64.const -0x1.e12b_288b_ff2b_dp+331)
  )
  (f64.const -0x1.c431_9ad2_5d20_1p+663)
)
(assert_return
  (invoke "f64.no_algebraic_factoring"
    (f64.const 0x1.3ed8_9843_1e10_2p+42)
    (f64.const -0x1.c409_183f_a92e_6p+39)
  )
  (f64.const 0x1.80a6_1110_3c71_dp+84)
)
(assert_return
  (invoke "f64.no_algebraic_factoring"
    (f64.const -0x1.be66_3e4c_0e4b_2p+182)
    (f64.const -0x1.da85_7037_60d2_5p+166)
  )
  (f64.const 0x1.8534_34f1_a2ff_ep+365)
)
(assert_return
  (invoke "f64.no_algebraic_factoring"
    (f64.const -0x1.230e_0995_2df1_cp-236)
    (f64.const -0x1.fa27_52ad_fadc_9p-237)
  )
  (f64.const 0x1.42e4_3156_bd1b_8p-474)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8c\80\80\80\00\02\60"
  "\03\7f\7f\7f\00\60\01\7f\01\7d\03\83\80\80\80\00"
  "\02\00\01\05\84\80\80\80\00\01\01\01\01\07\a0\80"
  "\80\80\00\02\11\66\33\32\2e\73\69\6d\70\6c\65\5f"
  "\78\34\5f\73\75\6d\00\00\08\66\33\32\2e\6c\6f\61"
  "\64\00\01\0a\f6\80\80\80\00\02\e4\80\80\80\00\01"
  "\08\7d\20\00\2a\02\00\21\03\20\00\2a\02\04\21\04"
  "\20\00\2a\02\08\21\05\20\00\2a\02\0c\21\06\20\01"
  "\2a\02\00\21\07\20\01\2a\02\04\21\08\20\01\2a\02"
  "\08\21\09\20\01\2a\02\0c\21\0a\20\02\20\03\20\07"
  "\92\38\02\00\20\02\20\04\20\08\92\38\02\04\20\02"
  "\20\05\20\09\92\38\02\08\20\02\20\06\20\0a\92\38"
  "\02\0c\0b\87\80\80\80\00\00\20\00\2a\02\00\0b\0b"
  "\b6\80\80\80\00\01\00\41\00\0b\30\01\00\00\00\01"
  "\00\00\80\01\00\00\00\01\00\00\80\01\00\00\00\01"
  "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\00\00\00\00\00\00\00"
)
(module instance)
(assert_return
  (invoke "f32.simple_x4_sum" (i32.const 0x0) (i32.const 0x10) (i32.const 0x20))
)
(assert_return (invoke "f32.load" (i32.const 0x20)) (f32.const 0x1p-148))
(assert_return (invoke "f32.load" (i32.const 0x24)) (f32.const 0x0p+0))
(assert_return (invoke "f32.load" (i32.const 0x28)) (f32.const 0x1p-149))
(assert_return (invoke "f32.load" (i32.const 0x2c)) (f32.const -0x1p-149))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8c\80\80\80\00\02\60"
  "\03\7f\7f\7f\00\60\01\7f\01\7c\03\83\80\80\80\00"
  "\02\00\01\05\84\80\80\80\00\01\01\01\01\07\a0\80"
  "\80\80\00\02\11\66\36\34\2e\73\69\6d\70\6c\65\5f"
  "\78\34\5f\73\75\6d\00\00\08\66\36\34\2e\6c\6f\61"
  "\64\00\01\0a\f6\80\80\80\00\02\e4\80\80\80\00\01"
  "\08\7c\20\00\2b\03\00\21\03\20\00\2b\03\08\21\04"
  "\20\00\2b\03\10\21\05\20\00\2b\03\18\21\06\20\01"
  "\2b\03\00\21\07\20\01\2b\03\08\21\08\20\01\2b\03"
  "\10\21\09\20\01\2b\03\18\21\0a\20\02\20\03\20\07"
  "\a0\39\03\00\20\02\20\04\20\08\a0\39\03\08\20\02"
  "\20\05\20\09\a0\39\03\10\20\02\20\06\20\0a\a0\39"
  "\03\18\0b\87\80\80\80\00\00\20\00\2b\03\00\0b\0b"
  "\e6\80\80\80\00\01\00\41\00\0b\60\01\00\00\00\00"
  "\00\00\00\01\00\00\00\00\00\00\80\01\00\00\00\00"
  "\00\00\00\01\00\00\00\00\00\00\80\01\00\00\00\00"
  "\00\00\00\01\00\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\00\00\00\00\00\00\00"
)
(module instance)
(assert_return
  (invoke "f64.simple_x4_sum" (i32.const 0x0) (i32.const 0x20) (i32.const 0x40))
)
(assert_return
  (invoke "f64.load" (i32.const 0x40))
  (f64.const 0x0.0000_0000_0000_2p-1022)
)
(assert_return (invoke "f64.load" (i32.const 0x48)) (f64.const 0x0p+0))
(assert_return
  (invoke "f64.load" (i32.const 0x50))
  (f64.const 0x0.0000_0000_0000_1p-1022)
)
(assert_return
  (invoke "f64.load" (i32.const 0x58))
  (f64.const -0x0.0000_0000_0000_1p-1022)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\87\80\80\80\00\01\60"
  "\02\7f\7f\01\7d\03\83\80\80\80\00\02\00\00\05\84"
  "\80\80\80\00\01\01\01\01\07\a1\80\80\80\00\02\0d"
  "\66\33\32\2e\6b\61\68\61\6e\5f\73\75\6d\00\00\0d"
  "\66\33\32\2e\70\6c\61\69\6e\5f\73\75\6d\00\01\0a"
  "\ea\80\80\80\00\02\b7\80\80\80\00\01\03\7d\02\40"
  "\03\40\20\03\20\00\2a\02\00\20\04\93\22\04\92\22"
  "\02\20\03\93\20\04\93\21\04\20\00\41\04\6a\21\00"
  "\20\02\21\03\20\01\41\7f\6a\22\01\0d\00\0b\0b\20"
  "\02\0b\a8\80\80\80\00\01\01\7d\02\40\03\40\20\02"
  "\20\00\2a\02\00\92\21\02\20\00\41\04\6a\21\00\20"
  "\01\41\7f\6a\21\01\20\01\0d\00\0b\0b\20\02\0b\0b"
  "\87\88\80\80\00\01\00\41\00\0b\80\08\c4\c5\57\24"
  "\a5\84\c8\0b\6d\b8\4b\2e\f2\76\17\1c\ca\4a\56\1e"
  "\1b\6e\71\22\5d\17\1e\6e\bf\cd\14\5c\c7\21\55\51"
  "\39\9c\1f\b2\51\f0\a3\93\d7\c1\2c\ae\7e\a8\28\3a"
  "\01\21\f4\0a\58\93\f8\42\77\9f\83\39\6a\5f\ba\f7"
  "\0a\d8\51\6a\34\ca\ad\c6\34\0e\d8\26\dc\4c\33\1c"
  "\ed\29\90\a8\78\0f\d1\ce\76\31\23\83\b8\35\e8\f2"
  "\44\b0\d3\a1\fc\bb\32\e1\b0\ba\69\44\09\d6\d9\7d"
  "\ff\2e\c0\5a\36\14\33\14\3e\a9\fa\87\6d\8b\bc\ce"
  "\9d\a7\fd\c4\e9\85\3f\dd\d7\e1\18\a6\50\26\72\6e"
  "\3f\73\0f\f8\12\93\23\34\61\76\12\48\c0\9b\05\93"
  "\eb\ac\86\de\94\3e\55\e8\8c\e8\dd\e4\fc\95\47\be"
  "\56\03\21\20\4c\e6\bf\7b\f6\7f\d5\ba\73\1c\c1\14"
  "\8f\c4\27\96\b3\bd\33\ff\78\41\5f\c0\5a\ce\f6\67"
  "\6e\73\9a\17\66\70\03\f8\ce\27\a3\52\b2\9f\3b\bf"
  "\fb\ae\ed\d3\5a\f8\37\57\f0\f5\6e\ef\b1\4d\70\3d"
  "\54\a7\01\9a\85\08\48\91\f5\9d\0c\60\87\5b\d9\54"
  "\1e\51\6d\88\8e\08\8c\a5\71\3a\56\08\67\46\8f\8f"
  "\13\2a\2c\ec\2c\1f\b4\62\2b\6f\41\0a\c4\65\42\a2"
  "\31\6b\2c\7d\3e\bb\75\ac\86\97\30\d9\48\cd\9a\1f"
  "\56\c4\c6\e4\12\c0\9d\fb\ee\02\8c\ce\1c\f2\1e\a1"
  "\78\23\db\c4\1e\49\03\d3\71\cc\08\50\c5\d8\5c\ed"
  "\d5\b5\65\ac\b5\c9\21\d2\c9\29\76\de\f0\30\1a\5b"
  "\3c\f2\3b\db\3a\39\82\3a\16\08\6f\a8\f1\be\69\69"
  "\99\71\a6\05\d3\14\93\2a\16\f2\2f\11\c7\7e\20\bb"
  "\91\44\ee\f8\e4\01\53\c0\b9\7f\f0\bf\f0\03\9c\6d"
  "\b1\df\a2\44\01\6d\6b\71\2b\5c\b3\21\19\46\5e\8f"
  "\db\91\d3\7c\78\6b\b7\12\00\8f\eb\bd\8a\f5\d4\2e"
  "\c4\c1\1e\df\73\63\59\47\49\03\0a\b7\cf\24\cf\9c"
  "\0e\44\7a\9e\14\fb\42\bf\9d\39\30\9e\a0\ab\2f\d1"
  "\ae\9e\6a\83\43\e3\55\7d\85\bf\63\8a\f8\96\10\1f"
  "\fe\6d\e7\22\1b\e1\69\46\8a\44\c8\c8\f9\0c\2b\19"
  "\07\a5\02\3e\f2\30\10\9a\85\8a\5f\ef\81\45\a0\77"
  "\b1\03\10\73\4b\ae\98\9d\47\bf\9a\2d\3a\d5\0f\03"
  "\66\e3\3d\53\d9\40\ce\1f\6f\32\2f\21\2b\23\21\6c"
  "\62\d4\a7\3e\a8\ce\28\31\2d\00\3d\67\5e\af\a0\cf"
  "\2e\d2\b9\6b\84\eb\69\08\3c\62\36\be\12\fd\36\7f"
  "\88\3e\ad\bc\0b\c0\41\c4\50\b6\e3\50\31\e8\ce\e2"
  "\96\65\55\9c\16\46\e6\b0\2d\3a\e8\81\05\b0\bf\34"
  "\f7\bc\10\1c\fb\cc\3c\f1\85\97\42\9f\eb\14\8d\3c"
  "\bf\d7\17\88\49\9d\8b\2b\b2\3a\83\d1\4f\04\9e\a1"
  "\0f\ad\08\9d\54\af\d1\82\c3\ec\32\2f\02\8f\05\21"
  "\2d\a2\b7\e4\f4\6f\2e\81\2b\0b\9c\fc\cb\fe\74\02"
  "\f9\db\f4\f3\ea\00\a8\ec\d1\99\74\26\dd\d6\34\d5"
  "\25\b1\46\dd\9c\aa\71\f5\60\b0\88\c8\e0\0b\59\5a"
  "\25\4f\29\66\f9\e3\2e\fe\e9\da\e5\18\4f\27\62\f4"
  "\ce\a4\21\95\74\c7\57\64\27\9a\4c\fd\54\7d\61\ce"
  "\c3\ac\87\46\9c\fa\ff\09\ca\79\97\67\24\74\ca\d4"
  "\21\83\26\25\19\12\37\64\19\e5\65\e0\74\75\8e\dd"
  "\c8\ef\74\c7\d8\21\2b\79\04\51\46\65\60\03\5d\fa"
  "\d8\f4\65\a4\9e\5d\23\da\d7\8a\92\80\a4\de\78\3c"
  "\f1\57\42\6d\cd\c9\2f\d5\a4\9e\ab\40\f4\cb\1b\d7"
  "\a3\ca\fc\eb\a7\01\b2\9a\69\4e\46\9b\18\4e\dd\79"
  "\a7\aa\a6\52\39\1e\ef\30\cc\9b\bd\5b\ee\4c\21\6d"
  "\30\00\72\b0\46\5f\08\cf\c5\b9\e0\3e\c2\b3\0c\dc"
  "\8e\64\de\19\42\79\cf\43\ea\43\5d\8e\88\f7\ab\15"
  "\dc\3f\c8\67\20\db\b8\64\b1\47\1f\de\f2\cb\3f\59"
  "\9f\d8\46\90\dc\ae\2f\22\f9\e2\31\89\d9\9c\1c\4c"
  "\d3\a9\4a\57\84\9c\9f\ea\2c\3c\ae\3c\c3\1e\8b\e5"
  "\4e\17\01\25\db\34\46\5f\15\ea\05\0c\7c\d9\45\8c"
  "\19\d0\73\8a\96\16\dd\44\f9\05\b7\5b\71\b0\e6\21"
  "\36\5f\75\89\91\73\75\ab\7d\ae\d3\73\ec\37\c6\ea"
  "\55\75\ef\ea\ab\8b\7b\11\dc\6d\1a\b2\6a\c4\25\cf"
  "\aa\e3\9f\49\49\89\cb\37\9b\0a\a7\01\60\70\dc\b7"
  "\c8\83\e1\42\f5\be\ad\62\94\ad\8d\a1"
)
(module instance)
(assert_return
  (invoke "f32.kahan_sum" (i32.const 0x0) (i32.const 0x100))
  (f32.const -0x1.101a_1ap+104)
)
(assert_return
  (invoke "f32.plain_sum" (i32.const 0x0) (i32.const 0x100))
  (f32.const -0x1.a034_3ap+103)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\87\80\80\80\00\01\60"
  "\02\7f\7f\01\7c\03\83\80\80\80\00\02\00\00\05\84"
  "\80\80\80\00\01\01\01\01\07\a1\80\80\80\00\02\0d"
  "\66\36\34\2e\6b\61\68\61\6e\5f\73\75\6d\00\00\0d"
  "\66\36\34\2e\70\6c\61\69\6e\5f\73\75\6d\00\01\0a"
  "\ea\80\80\80\00\02\b7\80\80\80\00\01\03\7c\02\40"
  "\03\40\20\03\20\00\2b\03\00\20\04\a1\22\04\a0\22"
  "\02\20\03\a1\20\04\a1\21\04\20\00\41\08\6a\21\00"
  "\20\02\21\03\20\01\41\7f\6a\22\01\0d\00\0b\0b\20"
  "\02\0b\a8\80\80\80\00\01\01\7c\02\40\03\40\20\02"
  "\20\00\2b\03\00\a0\21\02\20\00\41\08\6a\21\00\20"
  "\01\41\7f\6a\21\01\20\01\0d\00\0b\0b\20\02\0b\0b"
  "\87\90\80\80\00\01\00\41\00\0b\80\10\13\05\84\42"
  "\5d\a2\2c\c6\43\db\55\a9\cd\da\55\e3\73\fc\58\d6"
  "\ba\d5\00\fd\83\35\42\88\8b\13\5d\38\4a\47\0d\72"
  "\73\a1\1a\ef\c4\45\17\57\d8\c9\46\e0\8d\6c\e1\37"
  "\70\c8\83\5b\55\5e\5a\2d\73\1e\56\c8\e1\6d\69\14"
  "\78\0a\8a\5a\64\3a\09\c7\a8\87\c5\f0\d3\5d\e6\03"
  "\fc\93\be\26\ca\d6\a9\91\60\bd\b0\ed\ae\f7\30\7e"
  "\92\3a\6f\a7\59\8e\aa\7d\bf\67\58\2a\54\f8\4e\fe"
  "\ed\35\58\a6\51\bf\42\e5\4b\66\27\24\6d\7f\42\2d"
  "\28\92\18\ec\08\ae\e7\55\da\b1\a6\65\a5\72\50\47"
  "\1b\b8\a9\54\d7\a6\06\5b\0f\42\58\83\8a\17\82\c6"
  "\10\43\a0\c0\2e\6d\bc\5a\85\53\72\7f\ad\44\bc\30"
  "\3c\55\b2\24\9a\74\3a\9e\e1\d8\0f\70\fc\a9\3a\cd"
  "\93\4b\ec\e3\7e\dd\5d\27\cd\f8\a0\9d\1c\11\c0\57"
  "\2e\fd\c8\13\32\cc\3a\1a\7d\a3\41\55\ed\c3\82\49"
  "\2a\04\1e\ef\73\b9\2e\2e\e3\5f\f4\df\e6\b2\33\0c"
  "\39\3f\6f\44\6a\03\c1\42\b9\fa\b1\c8\ed\a5\58\99"
  "\7f\ed\b4\72\9e\79\eb\fb\43\82\45\aa\bb\95\d2\ff"
  "\28\9e\f6\a1\ad\95\d6\55\95\0d\6f\60\11\c7\78\3e"
  "\49\f2\7e\48\f4\a2\71\d0\13\8e\b3\de\99\52\e3\45"
  "\74\ea\76\0e\1b\2a\c8\ee\14\01\c4\50\5b\36\3c\ef"
  "\ba\72\a2\a6\08\f8\7b\36\9d\f9\ef\0b\c7\56\2d\5c"
  "\f0\9d\5d\de\fc\b8\ad\0f\64\0e\97\15\32\26\c2\31"
  "\e6\05\1e\ef\cb\17\1b\6d\15\0b\74\5d\d3\2e\f8\6b"
  "\86\b4\ba\73\52\53\99\a9\76\20\45\c9\40\80\6b\14"
  "\ed\a1\fa\80\46\e6\26\d2\e6\98\c4\57\bf\c4\1c\a4"
  "\90\7a\36\94\14\ba\15\89\6e\e6\9c\37\8c\f4\de\12"
  "\22\5d\a1\79\50\67\0d\3d\7a\e9\d4\aa\2e\7f\2a\7a"
  "\30\3d\ea\5d\12\48\fe\e1\18\cd\a4\57\a2\87\3e\b6"
  "\9a\8b\db\da\9d\78\9c\cf\8d\b1\4f\90\b4\34\e0\9d"
  "\f6\ca\fe\4c\3b\78\6d\0a\5c\18\9f\61\b9\dd\b4\e0"
  "\0f\76\e0\1b\69\0d\5e\58\73\70\5e\0e\2d\a1\7d\ff"
  "\20\eb\91\34\92\ac\38\72\2a\1f\8e\71\2e\6a\f1\af"
  "\c7\27\70\d9\c4\57\f7\d2\3c\1d\b8\f0\f0\64\cf\dc"
  "\ae\be\a3\cc\3e\22\7d\4e\69\21\63\17\ed\03\02\54"
  "\9a\0f\50\4e\13\5a\35\a1\22\a4\df\86\c2\74\79\16"
  "\b8\69\69\a0\52\5d\11\64\bd\5b\93\fc\69\a0\f4\13"
  "\d0\81\51\dd\fa\0c\15\c3\7a\c9\62\7a\a9\1d\c9\e6"
  "\5a\b3\5b\97\02\3c\64\22\12\3c\22\90\64\2d\30\54"
  "\4c\b4\a1\22\09\57\22\5e\8e\38\2b\02\a8\ae\f6\be"
  "\0d\2b\f2\03\ad\fa\10\01\71\77\2a\30\02\95\f6\00"
  "\3e\d0\c4\8d\34\19\50\21\0a\bc\50\da\3c\30\d6\3a"
  "\31\94\8d\3a\fe\ef\14\57\9d\4b\93\00\96\24\0c\6f"
  "\fd\bc\23\76\02\6c\eb\52\72\80\11\7e\80\3a\13\12"
  "\38\1d\38\49\95\40\27\8a\44\7b\e8\dc\6d\8c\8c\8e"
  "\3c\b5\b3\18\0e\f6\08\1a\84\41\35\ff\8b\b8\93\40"
  "\ea\e1\51\1d\89\a5\8d\42\68\29\ea\2f\c1\7a\52\eb"
  "\90\5d\4d\d6\80\e3\d7\75\48\ce\ed\d3\01\1c\8d\5b"
  "\a5\94\0d\78\cf\f1\06\13\2f\98\02\a4\6d\2e\6c\f2"
  "\d5\74\29\89\4c\f9\03\f5\c7\18\ad\7a\f0\68\f8\5c"
  "\d6\59\87\6e\d6\3f\06\be\86\20\e3\41\91\22\f3\6e"
  "\8b\f0\68\1c\57\a7\fc\b0\7c\9e\99\0b\96\1a\89\5f"
  "\e6\0d\7c\08\51\a0\a2\67\9a\47\00\93\6b\f9\28\f0"
  "\68\db\62\f1\e0\65\2c\53\33\e0\a7\ca\11\42\30\f6"
  "\af\01\c1\65\3d\32\01\6f\ab\2e\be\d3\8b\be\14\c3"
  "\ff\ec\fb\f0\f9\c5\0c\05\6f\01\09\6b\e3\34\31\0c"
  "\1f\66\a6\42\bc\1a\87\49\16\16\8c\b0\90\0d\34\8c"
  "\0a\e1\09\5e\10\a4\6b\56\cc\f0\c9\bb\dc\b8\5c\ce"
  "\f6\cc\8d\75\7e\b3\07\88\04\2f\b4\5e\c9\e3\4a\23"
  "\73\19\62\6c\9a\03\76\44\86\9c\60\fc\db\72\8f\27"
  "\a0\dd\b3\c5\da\ff\f9\ec\6a\b1\7b\d3\cf\50\37\c9"
  "\7a\78\0c\e4\3a\b6\f5\e6\f4\98\6e\42\7d\35\73\8b"
  "\45\c0\56\97\cd\6d\ce\cf\ad\31\b3\c3\54\fa\ef\d5"
  "\c0\f4\6a\5f\54\e7\49\3e\33\0a\30\38\fd\d9\05\ff"
  "\a5\3f\57\46\14\b5\91\17\ca\6b\98\23\7a\65\b3\6c"
  "\02\b4\cc\79\5d\58\d8\b3\d5\94\ae\f4\6d\75\65\f7"
  "\92\bf\7e\47\4c\3c\ee\db\ac\f1\32\5d\fb\6f\41\1c"
  "\34\c8\83\4f\c2\58\01\be\05\3e\66\16\a6\04\6d\5d"
  "\4f\86\09\27\82\25\12\cd\3a\cd\ce\6b\bc\ca\ac\28"
  "\9b\ee\6a\25\86\9e\45\70\c6\d2\bd\3b\7d\42\e5\27"
  "\af\c7\1d\f4\81\c8\b3\76\8a\a8\36\a3\ae\2a\e6\18"
  "\e1\36\22\ad\f6\25\72\b0\39\8b\01\9a\22\7b\84\c3"
  "\2d\5f\72\a4\98\ac\15\70\e7\d4\18\e2\7d\d2\30\7c"
  "\33\08\cd\ca\c4\22\85\88\75\81\c6\4a\74\58\8d\e0"
  "\e8\ac\c5\ab\75\5a\f4\28\12\f0\18\45\52\f2\97\b2"
  "\93\41\6f\8d\7f\db\70\fb\a3\5d\1f\a7\8d\98\20\2b"
  "\22\9f\3a\01\b5\8b\1b\d2\cb\14\03\0e\14\14\d2\19"
  "\5a\1f\ce\5e\cd\81\79\15\01\ca\de\73\74\8c\56\20"
  "\9f\77\2d\25\16\f6\61\51\1d\a4\8e\9b\98\a5\c6\ec"
  "\a8\45\57\82\59\78\0d\90\b4\df\51\b0\c3\82\94\cc"
  "\b3\53\09\15\6d\96\6c\3a\40\47\b7\4a\7a\05\2f\a1"
  "\1e\8c\9d\a0\20\88\fb\52\b7\9f\f3\f3\bb\5f\e7\8a"
  "\61\a7\21\b1\ac\fa\09\aa\a4\6c\bc\24\80\ba\2a\e9"
  "\65\ff\70\ff\cc\fa\65\87\76\f3\c5\15\ce\cb\e8\42"
  "\31\00\0c\91\57\d9\e0\9d\35\54\24\ad\a4\d8\f9\08"
  "\67\63\c8\cf\81\dd\90\a2\d7\c4\07\4a\e6\10\6f\67"
  "\e7\27\d4\23\59\18\f2\a8\9d\5f\d8\94\30\aa\54\86"
  "\4f\87\9d\82\b5\26\ca\a6\96\bf\cf\55\f9\9d\37\01"
  "\19\48\43\c5\94\6c\f3\74\97\58\4c\3c\9d\08\e8\04"
  "\c2\58\30\76\e1\a0\f8\ea\e9\c5\ae\cf\78\9e\a9\0c"
  "\ac\b3\44\42\e0\bc\5d\1b\9c\49\58\4a\1c\19\49\c1"
  "\3a\ea\f5\eb\3b\81\a9\4b\70\0c\cc\9e\1a\d3\2f\b7"
  "\52\2f\20\3b\eb\64\51\1d\a0\2d\b2\3e\be\13\85\48"
  "\92\32\2e\db\5c\a1\e7\8c\45\91\35\01\0a\93\c2\eb"
  "\09\ce\f3\d2\22\24\d0\8c\cc\1d\9d\38\c8\4d\e3\82"
  "\cc\64\15\06\2d\e7\01\2f\ab\bb\b5\04\4c\92\1c\7a"
  "\d6\3f\e8\5f\31\15\0c\dc\e4\31\b4\c4\25\3e\2a\aa"
  "\00\9e\c8\e5\21\7a\7f\29\f1\c0\af\1d\5e\e8\63\39"
  "\ad\f8\7e\6c\c8\c5\7f\c2\a8\97\27\0a\d9\f4\21\6a"
  "\ea\03\09\fb\f7\96\3b\83\79\5f\7c\4b\30\9f\56\35"
  "\de\b4\73\d4\95\f0\14\c3\74\2f\0d\a3\1d\4e\8d\31"
  "\24\b3\1a\84\85\62\5a\7b\3c\14\39\17\e6\6d\eb\37"
  "\c2\00\58\5b\0b\e3\3c\8a\62\e1\f8\35\4b\56\e2\87"
  "\60\8b\be\a7\38\91\77\54\a9\5a\24\25\90\9f\a5\42"
  "\77\f3\5c\39\df\ff\74\07\76\a1\cd\1f\62\0b\81\81"
  "\68\af\05\c1\c0\7f\26\ee\c0\91\a3\6a\7d\29\61\45"
  "\27\e5\57\88\dc\0d\97\04\1a\33\a9\44\8a\da\02\10"
  "\45\3f\8e\55\a6\76\8c\4d\e3\f1\89\83\c8\d0\f8\9b"
  "\50\77\9f\47\df\4c\9c\66\0d\aa\18\b8\5f\4f\c4\01"
  "\ce\dc\84\ac\46\9e\69\e1\76\45\6b\61\89\e4\5d\94"
  "\bb\11\83\9f\78\d8\0a\d2\f5\7e\5d\43\ea\bc\10\f1"
  "\3a\c9\e2\64\fb\53\65\d0\c7\b4\a7\fb\d4\05\53\25"
  "\d0\cd\29\88\00\56\25\24\7d\5d\b4\f3\41\9f\e9\b5"
  "\f7\ae\64\2c\e3\c9\6d\d5\84\3a\72\12\b8\7a\d9\1b"
  "\09\e8\38\da\26\4f\04\ce\03\71\6e\8a\44\7b\5c\81"
  "\59\9c\d2\e4\c3\ba\59\a6\e5\28\a7\8f\9a\e4\d5\4e"
  "\b9\ca\7f\cb\75\b8\2b\43\3e\b3\15\46\b1\a5\bc\9d"
  "\9e\38\15\f1\bd\1b\21\aa\f1\82\00\95\fc\a7\77\47"
  "\39\a7\33\43\92\d7\52\40\4b\06\81\8a\a0\bd\f1\6b"
  "\99\84\42\5b\e2\3b\c5\5e\12\5c\28\4d\b6\0e\4e\c8"
  "\5c\e8\01\8a\c5\e7\e4\9d\42\ee\5d\9c\c4\eb\eb\68"
  "\09\27\92\95\9a\11\54\73\c4\12\80\fb\7d\fe\c5\08"
  "\60\7f\36\41\e0\10\ba\d6\2b\6c\f1\b4\17\fe\26\34"
  "\e3\4b\f8\a8\e3\91\be\4f\2a\fc\da\81\b8\e7\fe\d5"
  "\26\50\47\f3\1a\65\32\81\e0\05\b8\4f\32\31\26\00"
  "\4a\53\97\c2\c3\0e\2e\a1\26\54\ab\05\8e\56\2f\7d"
  "\af\22\84\68\a5\8b\97\f6\a4\fd\a8\cc\75\41\96\86"
  "\fd\27\3d\29\86\8d\7f\4c\d4\8e\73\41\f4\1e\e2\dd"
  "\58\27\97\ce\9c\94\cf\7a\04\2f\dc\ed"
)
(module instance)
(assert_return
  (invoke "f64.kahan_sum" (i32.const 0x0) (i32.const 0x100))
  (f64.const 0x1.dd7c_b2a5_ffc8_8p+998)
)
(assert_return
  (invoke "f64.plain_sum" (i32.const 0x0) (i32.const 0x100))
  (f64.const 0x1.dd7c_b2a6_3fc8_7p+998)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\83\80\80\80"
  "\00\02\00\01\07\ad\80\80\80\00\02\13\66\33\32\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\6e\65\67\5f\73\75\62\00"
  "\00\13\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\6e\65"
  "\67\5f\73\75\62\00\01\0a\9b\80\80\80\00\02\88\80"
  "\80\80\00\00\20\00\20\01\93\8c\0b\88\80\80\80\00"
  "\00\20\00\20\01\a1\9a\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_neg_sub" (f32.const -0x0p+0) (f32.const -0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_neg_sub" (f32.const 0x0p+0) (f32.const -0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_neg_sub" (f32.const -0x0p+0) (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_neg_sub" (f32.const 0x0p+0) (f32.const 0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_neg_sub" (f64.const -0x0p+0) (f64.const -0x0p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_neg_sub" (f64.const 0x0p+0) (f64.const -0x0p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_neg_sub" (f64.const -0x0p+0) (f64.const 0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_neg_sub" (f64.const 0x0p+0) (f64.const 0x0p+0))
  (f64.const -0x0p+0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\83\80\80\80"
  "\00\02\00\01\07\ad\80\80\80\00\02\13\66\33\32\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\6e\65\67\5f\61\64\64\00"
  "\00\13\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\6e\65"
  "\67\5f\61\64\64\00\01\0a\9b\80\80\80\00\02\88\80"
  "\80\80\00\00\20\00\20\01\92\8c\0b\88\80\80\80\00"
  "\00\20\00\20\01\a0\9a\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_neg_add" (f32.const -0x0p+0) (f32.const -0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_neg_add" (f32.const 0x0p+0) (f32.const -0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_neg_add" (f32.const -0x0p+0) (f32.const 0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_neg_add" (f32.const 0x0p+0) (f32.const 0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_neg_add" (f64.const -0x0p+0) (f64.const -0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_neg_add" (f64.const 0x0p+0) (f64.const -0x0p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_neg_add" (f64.const -0x0p+0) (f64.const 0x0p+0))
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_neg_add" (f64.const 0x0p+0) (f64.const 0x0p+0))
  (f64.const -0x0p+0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\83\80\80\80"
  "\00\02\00\01\07\b5\80\80\80\00\02\17\66\33\32\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\61\64\64\5f\6e\65\67\5f"
  "\6e\65\67\00\00\17\66\36\34\2e\6e\6f\5f\66\6f\6c"
  "\64\5f\61\64\64\5f\6e\65\67\5f\6e\65\67\00\01\0a"
  "\9d\80\80\80\00\02\89\80\80\80\00\00\20\00\8c\20"
  "\01\8c\92\0b\89\80\80\80\00\00\20\00\9a\20\01\9a"
  "\a0\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_add_neg_neg" (f32.const -0x0p+0) (f32.const -0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_add_neg_neg" (f32.const 0x0p+0) (f32.const -0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_add_neg_neg" (f32.const -0x0p+0) (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_add_neg_neg" (f32.const 0x0p+0) (f32.const 0x0p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_add_neg_neg" (f64.const -0x0p+0) (f64.const -0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_add_neg_neg" (f64.const 0x0p+0) (f64.const -0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_add_neg_neg" (f64.const -0x0p+0) (f64.const 0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_add_neg_neg" (f64.const 0x0p+0) (f64.const 0x0p+0))
  (f64.const -0x0p+0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\ad\80\80\80\00\02\13\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\61\64\64\5f\6e\65\67\00\00\13"
  "\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\61\64\64\5f"
  "\6e\65\67\00\01\0a\9b\80\80\80\00\02\88\80\80\80"
  "\00\00\20\00\8c\20\00\92\0b\88\80\80\80\00\00\20"
  "\00\9a\20\00\a0\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_add_neg" (f32.const 0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_add_neg" (f32.const -0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_add_neg" (f32.const inf))
  (f32.const nan:canonical)
)
(assert_return
  (invoke "f32.no_fold_add_neg" (f32.const -inf))
  (f32.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_add_neg" (f64.const 0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_add_neg" (f64.const -0x0p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_add_neg" (f64.const inf))
  (f64.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_add_neg" (f64.const -inf))
  (f64.const nan:canonical)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\b3\80\80\80\00\02\16\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\36\78\5f\76\69\61\5f\61\64\64"
  "\00\00\16\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\36"
  "\78\5f\76\69\61\5f\61\64\64\00\01\0a\b1\80\80\80"
  "\00\02\93\80\80\80\00\00\20\00\20\00\92\20\00\92"
  "\20\00\92\20\00\92\20\00\92\0b\93\80\80\80\00\00"
  "\20\00\20\00\a0\20\00\a0\20\00\a0\20\00\a0\20\00"
  "\a0\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_6x_via_add" (f32.const -0x1.598a_0cp+99))
  (f32.const -0x1.0327_8ap+102)
)
(assert_return
  (invoke "f32.no_fold_6x_via_add" (f32.const -0x1.d3e7_acp-77))
  (f32.const -0x1.5eed_c2p-74)
)
(assert_return
  (invoke "f32.no_fold_6x_via_add" (f32.const 0x1.00fa_02p-77))
  (f32.const 0x1.8177_02p-75)
)
(assert_return
  (invoke "f32.no_fold_6x_via_add" (f32.const -0x1.51f4_34p-31))
  (f32.const -0x1.faee_4cp-29)
)
(assert_return
  (invoke "f32.no_fold_6x_via_add" (f32.const -0x1.0032_8ap+80))
  (f32.const -0x1.804b_cep+82)
)
(assert_return
  (invoke "f64.no_fold_6x_via_add" (f64.const -0x1.310e_15ac_affe_6p+68))
  (f64.const -0x1.c995_2083_07fd_ap+70)
)
(assert_return
  (invoke "f64.no_fold_6x_via_add" (f64.const -0x1.aad6_2c78_fa9b_4p-535))
  (f64.const -0x1.4020_a15a_bbf4_6p-532)
)
(assert_return
  (invoke "f64.no_fold_6x_via_add" (f64.const -0x1.f8fb_fa94_f6ab_2p+271))
  (f64.const -0x1.7abc_fbef_b900_5p+274)
)
(assert_return
  (invoke "f64.no_fold_6x_via_add" (f64.const 0x1.756c_cc28_30a8_ep+751))
  (f64.const 0x1.1811_991e_247e_bp+754)
)
(assert_return
  (invoke "f64.no_fold_6x_via_add" (f64.const -0x1.8fd1_ab1d_2402_ap+234))
  (f64.const -0x1.2bdd_4055_db01_fp+237)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\02\60"
  "\03\7d\7d\7d\01\7d\60\03\7c\7c\7c\01\7c\03\83\80"
  "\80\80\00\02\00\01\07\ad\80\80\80\00\02\13\66\33"
  "\32\2e\6e\6f\5f\66\6f\6c\64\5f\64\69\76\5f\64\69"
  "\76\00\00\13\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f"
  "\64\69\76\5f\64\69\76\00\01\0a\9f\80\80\80\00\02"
  "\8a\80\80\80\00\00\20\00\20\01\95\20\02\95\0b\8a"
  "\80\80\80\00\00\20\00\20\01\a3\20\02\a3\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_div_div"
    (f32.const -0x1.f702_28p+78)
    (f32.const -0x1.fbc6_12p-16)
    (f32.const -0x1.8c37_9p+10)
  )
  (f32.const -0x1.47b4_3cp+83)
)
(assert_return
  (invoke "f32.no_fold_div_div"
    (f32.const 0x1.d29d_2ep-70)
    (f32.const 0x1.f3a1_7ep+110)
    (f32.const -0x1.64d4_1p-112)
  )
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_div_div"
    (f32.const 0x1.867f_98p+43)
    (f32.const 0x1.30ac_fcp-105)
    (f32.const 0x1.e210_d8p+105)
  )
  (f32.const inf)
)
(assert_return
  (invoke "f32.no_fold_div_div"
    (f32.const -0x1.c400_1ap-14)
    (f32.const -0x1.9beb_6cp+124)
    (f32.const -0x1.74f3_4cp-43)
  )
  (f32.const -0x1.8198_74p-96)
)
(assert_return
  (invoke "f32.no_fold_div_div"
    (f32.const 0x1.db0e_6ep+46)
    (f32.const 0x1.55ee_a2p+56)
    (f32.const -0x1.f313_4p+124)
  )
  (f32.const -0x1.6cep-135)
)
(assert_return
  (invoke "f64.no_fold_div_div"
    (f64.const 0x1.b4dc_8ec3_c777_7p+337)
    (f64.const 0x1.9f95_ac2d_1863p+584)
    (f64.const -0x1.d431_8abb_a341_ep-782)
  )
  (f64.const -0x1.2649_159d_87e0_2p+534)
)
(assert_return
  (invoke "f64.no_fold_div_div"
    (f64.const -0x1.ac53_af5e_b445_fp+791)
    (f64.const 0x1.8549_c0a4_ceb1_3p-29)
    (f64.const 0x1.64e3_8400_3c80_1p+316)
  )
  (f64.const -0x1.9417_cdcc_bae9_1p+503)
)
(assert_return
  (invoke "f64.no_fold_div_div"
    (f64.const -0x1.d268_5afb_2732_7p+2)
    (f64.const -0x1.abb1_eeed_3dbe_bp+880)
    (f64.const 0x1.a543_e2e6_968a_3p+170)
  )
  (f64.const 0x0.0000_002a_69a5_fp-1022)
)
(assert_return
  (invoke "f64.no_fold_div_div"
    (f64.const -0x1.47dd_ede7_8ad1_cp+825)
    (f64.const 0x1.6d93_2d07_0a36_7p-821)
    (f64.const 0x1.79cf_18cc_64fp+961)
  )
  (f64.const -inf)
)
(assert_return
  (invoke "f64.no_fold_div_div"
    (f64.const -0x1.f73d_4979_a937_9p-888)
    (f64.const 0x1.4d83_b53e_9778_8p-596)
    (f64.const -0x1.f8f8_6c96_03b5_bp-139)
  )
  (f64.const 0x1.87a7_bd89_c586_cp-154)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\91\80\80\80\00\02\60"
  "\04\7d\7d\7d\7d\01\7d\60\04\7c\7c\7c\7c\01\7c\03"
  "\83\80\80\80\00\02\00\01\07\af\80\80\80\00\02\14"
  "\66\33\32\2e\6e\6f\5f\66\6f\6c\64\5f\6d\75\6c\5f"
  "\64\69\76\73\00\00\14\66\36\34\2e\6e\6f\5f\66\6f"
  "\6c\64\5f\6d\75\6c\5f\64\69\76\73\00\01\0a\a5\80"
  "\80\80\00\02\8d\80\80\80\00\00\20\00\20\01\95\20"
  "\02\20\03\95\94\0b\8d\80\80\80\00\00\20\00\20\01"
  "\a3\20\02\20\03\a3\a2\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_mul_divs"
    (f32.const -0x1.c483_bep-109)
    (f32.const 0x1.ee1c_3cp-92)
    (f32.const 0x1.8007_56p-88)
    (f32.const -0x1.95b9_72p+4)
  )
  (f32.const 0x1.bbd3_0cp-110)
)
(assert_return
  (invoke "f32.no_fold_mul_divs"
    (f32.const -0x1.0f42_62p+102)
    (f32.const 0x1.2484_98p+25)
    (f32.const 0x1.f66a_7cp-17)
    (f32.const 0x1.897f_c8p-3)
  )
  (f32.const -0x1.2f1a_a4p+63)
)
(assert_return
  (invoke "f32.no_fold_mul_divs"
    (f32.const -0x1.df5f_22p+33)
    (f32.const -0x1.fcee_3ep+39)
    (f32.const -0x1.9ea9_14p+29)
    (f32.const -0x1.2c4d_3p+10)
  )
  (f32.const 0x1.4cf5_1cp+13)
)
(assert_return
  (invoke "f32.no_fold_mul_divs"
    (f32.const -0x1.f568_bcp+109)
    (f32.const 0x1.d996_3p-34)
    (f32.const 0x1.37a8_7ap-16)
    (f32.const 0x1.a152_4ap+78)
  )
  (f32.const -inf)
)
(assert_return
  (invoke "f32.no_fold_mul_divs"
    (f32.const 0x1.3dd5_92p-53)
    (f32.const -0x1.332c_22p-64)
    (f32.const 0x1.b010_64p-91)
    (f32.const 0x1.92bb_3ap-36)
  )
  (f32.const -0x1.1c2d_bp-44)
)
(assert_return
  (invoke "f64.no_fold_mul_divs"
    (f64.const -0x1.363d_6764_f7b1_2p-819)
    (f64.const -0x1.ed54_71f6_60b5_fp-464)
    (f64.const -0x1.671b_0a7f_3a42p+547)
    (f64.const 0x1.0633_be34_ba1f_2p+186)
  )
  (f64.const -0x1.b8fa_2b76_baee_bp+5)
)
(assert_return
  (invoke "f64.no_fold_mul_divs"
    (f64.const -0x1.3788_0182_e0fa_8p+115)
    (f64.const 0x1.f842_6315_7614_7p-920)
    (f64.const -0x1.9993_7223_1d15_6p+362)
    (f64.const -0x1.d5db_481a_b955_4p+467)
  )
  (f64.const -inf)
)
(assert_return
  (invoke "f64.no_fold_mul_divs"
    (f64.const -0x1.9a74_7c8d_4b54_1p+308)
    (f64.const -0x1.9909_2ad6_bbdc_8p+192)
    (f64.const -0x1.cb23_755c_2010_1p-140)
    (f64.const -0x1.de87_16f6_b0b6_ap+732)
  )
  (f64.const 0x1.ecf5_84c8_466a_5p-757)
)
(assert_return
  (invoke "f64.no_fold_mul_divs"
    (f64.const -0x1.c424_b2ec_e903_dp+129)
    (f64.const -0x1.568c_e281_db37_fp-347)
    (f64.const 0x1.5390_0b99_fd3dp-957)
    (f64.const 0x1.5c33_9522_54da_dp+223)
  )
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_mul_divs"
    (f64.const 0x1.a8ec_2cec_b32a_9p-18)
    (f64.const 0x1.58ac_ab00_5185_1p-277)
    (f64.const 0x1.35e8_7c90_77f7_fp-620)
    (f64.const -0x1.925e_e37f_fb38_6p+352)
  )
  (f64.const -0x1.e628_6970_b31b_fp-714)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\02\60"
  "\03\7d\7d\7d\01\7d\60\03\7c\7c\7c\01\7c\03\83\80"
  "\80\80\00\02\00\01\07\af\80\80\80\00\02\14\66\33"
  "\32\2e\6e\6f\5f\66\6f\6c\64\5f\61\64\64\5f\64\69"
  "\76\73\00\00\14\66\36\34\2e\6e\6f\5f\66\6f\6c\64"
  "\5f\61\64\64\5f\64\69\76\73\00\01\0a\a5\80\80\80"
  "\00\02\8d\80\80\80\00\00\20\00\20\02\95\20\01\20"
  "\02\95\92\0b\8d\80\80\80\00\00\20\00\20\02\a3\20"
  "\01\20\02\a3\a0\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_add_divs"
    (f32.const 0x1.795e_7p+8)
    (f32.const -0x1.48a5_eep-5)
    (f32.const -0x1.9a24_4cp+126)
  )
  (f32.const -0x1.d709_b6p-119)
)
(assert_return
  (invoke "f32.no_fold_add_divs"
    (f32.const -0x1.ae89_e8p-63)
    (f32.const -0x1.e990_3ep-49)
    (f32.const -0x1.370a_8cp+47)
  )
  (f32.const 0x1.92f3_f6p-96)
)
(assert_return
  (invoke "f32.no_fold_add_divs"
    (f32.const -0x1.6264_08p-46)
    (f32.const 0x1.2ee5_b2p-64)
    (f32.const -0x1.ecef_aap+48)
  )
  (f32.const 0x1.7018_64p-95)
)
(assert_return
  (invoke "f32.no_fold_add_divs"
    (f32.const -0x1.061d_3p-101)
    (f32.const 0x1.3834_92p-98)
    (f32.const -0x1.1d92_d2p+88)
  )
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_add_divs"
    (f32.const 0x1.1ea3_9ep-10)
    (f32.const 0x1.a7ff_fep-3)
    (f32.const 0x1.6fc5_74p-123)
  )
  (f32.const 0x1.28b2_dep+120)
)
(assert_return
  (invoke "f64.no_fold_add_divs"
    (f64.const -0x1.c5fc_c327_3b13_6p+430)
    (f64.const 0x1.892a_09ee_d8f6_fp+434)
    (f64.const 0x1.8258_b71e_6439_7p+911)
  )
  (f64.const 0x1.e36e_b970_6ad8_2p-478)
)
(assert_return
  (invoke "f64.no_fold_add_divs"
    (f64.const -0x1.2215_d406_1b5b_3p+53)
    (f64.const 0x1.fb61_84d9_7f27_cp+5)
    (f64.const -0x1.f3bb_59da_cc0e_bp-957)
  )
  (f64.const 0x1.2934_eb01_18be_3p+1009)
)
(assert_return
  (invoke "f64.no_fold_add_divs"
    (f64.const -0x1.e7a4_5337_41d8_ep-967)
    (f64.const 0x1.a519_bb7f_eb80_2p-976)
    (f64.const 0x1.1f8a_4345_4e51_ap+504)
  )
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_add_divs"
    (f64.const 0x1.991c_6cf9_3e2b_4p+313)
    (f64.const -0x1.f2f7_4326_98d1_1p+329)
    (f64.const 0x1.0d8c_1b24_5361_7p-126)
  )
  (f64.const -0x1.d9e1_d84d_dd1d_4p+455)
)
(assert_return
  (invoke "f64.no_fold_add_divs"
    (f64.const -0x1.d436_849d_c127_1p-728)
    (f64.const 0x1.19d1_c145_0e52_dp-755)
    (f64.const 0x1.fa1b_e69e_a06f_ep-70)
  )
  (f64.const -0x1.d9a9_b1c2_f562_3p-659)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\b5\80\80\80\00\02\17\66\33\32\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\73\71\72\74\5f\73\71\75\61\72"
  "\65\00\00\17\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f"
  "\73\71\72\74\5f\73\71\75\61\72\65\00\01\0a\9b\80"
  "\80\80\00\02\88\80\80\80\00\00\20\00\20\00\94\91"
  "\0b\88\80\80\80\00\00\20\00\20\00\a2\9f\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_sqrt_square" (f32.const -0x1.5cb3_16p-66))
  (f32.const 0x1.5cb3_22p-66)
)
(assert_return
  (invoke "f32.no_fold_sqrt_square" (f32.const -0x1.b0f9_e4p-73))
  (f32.const 0x1.b211_b2p-73)
)
(assert_return
  (invoke "f32.no_fold_sqrt_square" (f32.const -0x1.de41_7cp-71))
  (f32.const 0x1.de65_b8p-71)
)
(assert_return
  (invoke "f32.no_fold_sqrt_square" (f32.const 0x1.64c8_72p-86))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_sqrt_square" (f32.const 0x1.e199_e4p+108))
  (f32.const inf)
)
(assert_return
  (invoke "f64.no_fold_sqrt_square" (f64.const 0x1.1759_d657_203f_dp-529))
  (f64.const 0x1.1759_dd57_545f_3p-529)
)
(assert_return
  (invoke "f64.no_fold_sqrt_square" (f64.const -0x1.4c68_de1c_78d8_3p-514))
  (f64.const 0x1.4c68_de1c_78d8_1p-514)
)
(assert_return
  (invoke "f64.no_fold_sqrt_square" (f64.const -0x1.2147_36ed_b6e1_ep-521))
  (f64.const 0x1.2147_36ed_9cf8_dp-521)
)
(assert_return
  (invoke "f64.no_fold_sqrt_square" (f64.const -0x1.0864_b9f6_8457p-616))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_sqrt_square" (f64.const 0x1.b2a9_8559_95ab_ap+856))
  (f64.const inf)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\83\80\80\80"
  "\00\02\00\01\07\b1\80\80\80\00\02\15\66\33\32\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\6d\75\6c\5f\73\71\72\74"
  "\73\00\00\15\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f"
  "\6d\75\6c\5f\73\71\72\74\73\00\01\0a\9d\80\80\80"
  "\00\02\89\80\80\80\00\00\20\00\91\20\01\91\94\0b"
  "\89\80\80\80\00\00\20\00\9f\20\01\9f\a2\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_mul_sqrts"
    (f32.const 0x1.dddd_a8p-125)
    (f32.const -0x1.25d2_2ap-83)
  )
  (f32.const nan:canonical)
)
(assert_return
  (invoke "f32.no_fold_mul_sqrts"
    (f32.const 0x1.418d_14p-92)
    (f32.const 0x1.c653_5cp-32)
  )
  (f32.const 0x1.7e37_3ap-62)
)
(assert_return
  (invoke "f32.no_fold_mul_sqrts"
    (f32.const 0x1.4de7_ep-88)
    (f32.const 0x1.84ff_18p+6)
  )
  (f32.const 0x1.6866_68p-41)
)
(assert_return
  (invoke "f32.no_fold_mul_sqrts"
    (f32.const 0x1.7809_1ep+101)
    (f32.const 0x1.81fe_b8p-9)
  )
  (f32.const 0x1.7cfb_98p+46)
)
(assert_return
  (invoke "f32.no_fold_mul_sqrts"
    (f32.const 0x1.583ap-56)
    (f32.const 0x1.14ba_2ap-9)
  )
  (f32.const 0x1.b47a_8ep-33)
)
(assert_return
  (invoke "f64.no_fold_mul_sqrts"
    (f64.const -0x1.d114_4cc2_8cdb_ep-635)
    (f64.const -0x1.bf9b_c373_d3b6_ap-8)
  )
  (f64.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_mul_sqrts"
    (f64.const 0x1.5a7e_b976_bebc_9p-643)
    (f64.const 0x1.f30c_b886_5a4c_ap-404)
  )
  (f64.const 0x1.260a_1032_d6e7_6p-523)
)
(assert_return
  (invoke "f64.no_fold_mul_sqrts"
    (f64.const 0x1.711a_0c17_0793_5p-89)
    (f64.const 0x1.6fb5_de51_a20d_3p-913)
  )
  (f64.const 0x1.7067_ca28_e31e_cp-501)
)
(assert_return
  (invoke "f64.no_fold_mul_sqrts"
    (f64.const 0x1.fb0b_bea3_3b07_6p-363)
    (f64.const 0x1.d963_b348_9415_8p-573)
  )
  (f64.const 0x1.e9ed_c1fa_624a_fp-468)
)
(assert_return
  (invoke "f64.no_fold_mul_sqrts"
    (f64.const 0x1.8676_eab7_a4d0_dp+24)
    (f64.const 0x1.75a5_8231_ba7a_5p+513)
  )
  (f64.const 0x1.0e16_aebe_203b_3p+269)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\83\80\80\80"
  "\00\02\00\01\07\b1\80\80\80\00\02\15\66\33\32\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\64\69\76\5f\73\71\72\74"
  "\73\00\00\15\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f"
  "\64\69\76\5f\73\71\72\74\73\00\01\0a\9d\80\80\80"
  "\00\02\89\80\80\80\00\00\20\00\91\20\01\91\95\0b"
  "\89\80\80\80\00\00\20\00\9f\20\01\9f\a3\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_div_sqrts"
    (f32.const -0x1.bea9_bap+25)
    (f32.const -0x1.db77_6ep-58)
  )
  (f32.const nan:canonical)
)
(assert_return
  (invoke "f32.no_fold_div_sqrts"
    (f32.const 0x1.b983_b6p+32)
    (f32.const 0x1.901f_1ep+27)
  )
  (f32.const 0x1.7c4d_f6p+2)
)
(assert_return
  (invoke "f32.no_fold_div_sqrts"
    (f32.const 0x1.d45e_72p-120)
    (f32.const 0x1.ab49_ccp+15)
  )
  (f32.const 0x1.7b0b_04p-68)
)
(assert_return
  (invoke "f32.no_fold_div_sqrts"
    (f32.const 0x1.b2e4_44p+59)
    (f32.const 0x1.5b8b_16p-30)
  )
  (f32.const 0x1.94fc_a8p+44)
)
(assert_return
  (invoke "f32.no_fold_div_sqrts"
    (f32.const 0x1.835a_a6p-112)
    (f32.const 0x1.d171_28p-103)
  )
  (f32.const 0x1.4a46_8p-5)
)
(assert_return
  (invoke "f64.no_fold_div_sqrts"
    (f64.const -0x1.509f_c164_1116_7p-711)
    (f64.const -0x1.9c42_55f5_d651_7p-187)
  )
  (f64.const nan:canonical)
)
(assert_return
  (invoke "f64.no_fold_div_sqrts"
    (f64.const 0x1.b689_7bdd_ac76p-587)
    (f64.const 0x1.1045_78b4_c91f_3p+541)
  )
  (f64.const 0x1.44e4_f21f_26cc_9p-564)
)
(assert_return
  (invoke "f64.no_fold_div_sqrts"
    (f64.const 0x1.ac83_451b_0898_9p+523)
    (f64.const 0x1.8da5_75c6_d12b_8p-109)
  )
  (f64.const 0x1.09c0_0399_1ce1_7p+316)
)
(assert_return
  (invoke "f64.no_fold_div_sqrts"
    (f64.const 0x1.bab7_8364_5641_7p-810)
    (f64.const 0x1.1ff6_0d03_ba60_7p+291)
  )
  (f64.const 0x1.c0e6_c833_bf65_7p-551)
)
(assert_return
  (invoke "f64.no_fold_div_sqrts"
    (f64.const 0x1.a957_816a_d951_5p-789)
    (f64.const 0x1.8c18_a3a2_22ab_1p+945)
  )
  (f64.const 0x1.0948_5397_81e9_2p-867)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\83\80\80\80"
  "\00\02\00\01\07\b7\80\80\80\00\02\18\66\33\32\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\6d\75\6c\5f\73\71\72\74"
  "\5f\64\69\76\00\00\18\66\36\34\2e\6e\6f\5f\66\6f"
  "\6c\64\5f\6d\75\6c\5f\73\71\72\74\5f\64\69\76\00"
  "\01\0a\a1\80\80\80\00\02\8b\80\80\80\00\00\20\00"
  "\20\01\91\94\20\01\95\0b\8b\80\80\80\00\00\20\00"
  "\20\01\9f\a2\20\01\a3\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_mul_sqrt_div"
    (f32.const -0x1.f4a7_cap+81)
    (f32.const 0x1.c09a_dep+92)
  )
  (f32.const -inf)
)
(assert_return
  (invoke "f32.no_fold_mul_sqrt_div"
    (f32.const -0x1.90bf_1cp-120)
    (f32.const 0x1.8dbe_88p-97)
  )
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f32.no_fold_mul_sqrt_div"
    (f32.const 0x1.8570_e8p+29)
    (f32.const 0x1.217d_3p-128)
  )
  (f32.const 0x1.6e39_1ap+93)
)
(assert_return
  (invoke "f32.no_fold_mul_sqrt_div"
    (f32.const -0x1.5b46_52p+43)
    (f32.const 0x1.a9d7_1cp+112)
  )
  (f32.const -0x1.0d42_3ap-13)
)
(assert_return
  (invoke "f32.no_fold_mul_sqrt_div"
    (f32.const -0x1.9106_04p+8)
    (f32.const 0x1.0ca9_12p+7)
  )
  (f32.const -0x1.14cd_ecp+5)
)
(assert_return
  (invoke "f64.no_fold_mul_sqrt_div"
    (f64.const 0x1.1dcd_eb85_7305_fp+698)
    (f64.const 0x1.a066_171c_40eb_9p+758)
  )
  (f64.const inf)
)
(assert_return
  (invoke "f64.no_fold_mul_sqrt_div"
    (f64.const -0x1.8b4f_1c21_8e2a_bp-827)
    (f64.const 0x1.5e1e_e659_53b0_bp-669)
  )
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_mul_sqrt_div"
    (f64.const 0x1.74ee_531d_dba3_8p-425)
    (f64.const 0x1.f370_f758_857f_3p+560)
  )
  (f64.const 0x1.0aff_3426_9583_ep-705)
)
(assert_return
  (invoke "f64.no_fold_mul_sqrt_div"
    (f64.const -0x1.27f2_16b0_da6c_5p+352)
    (f64.const 0x1.8e0b_4e0b_9fd7_ep-483)
  )
  (f64.const -0x1.4fa5_58aa_d514_ep+593)
)
(assert_return
  (invoke "f64.no_fold_mul_sqrt_div"
    (f64.const 0x1.4c69_55df_9912_bp+104)
    (f64.const 0x1.0cca_42c9_d371_ep+842)
  )
  (f64.const 0x1.4468_072f_5429_4p-317)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\02\60"
  "\03\7d\7d\7d\01\7d\60\03\7c\7c\7c\01\7c\03\83\80"
  "\80\80\00\02\00\01\07\cd\80\80\80\00\02\23\66\33"
  "\32\2e\6e\6f\5f\66\6c\75\73\68\5f\69\6e\74\65\72"
  "\6d\65\64\69\61\74\65\5f\73\75\62\6e\6f\72\6d\61"
  "\6c\00\00\23\66\36\34\2e\6e\6f\5f\66\6c\75\73\68"
  "\5f\69\6e\74\65\72\6d\65\64\69\61\74\65\5f\73\75"
  "\62\6e\6f\72\6d\61\6c\00\01\0a\9f\80\80\80\00\02"
  "\8a\80\80\80\00\00\20\00\20\01\94\20\02\94\0b\8a"
  "\80\80\80\00\00\20\00\20\01\a2\20\02\a2\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_flush_intermediate_subnormal"
    (f32.const 0x1p-126)
    (f32.const 0x1p-23)
    (f32.const 0x1p+23)
  )
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "f64.no_flush_intermediate_subnormal"
    (f64.const 0x1p-1022)
    (f64.const 0x1p-52)
    (f64.const 0x1p+52)
  )
  (f64.const 0x1p-1022)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\93\80\80\80\00\03\60"
  "\02\7d\7d\01\7f\60\02\7c\7c\01\7f\60\02\7c\7d\01"
  "\7d\03\88\80\80\80\00\07\00\00\00\01\01\01\02\07"
  "\ff\80\80\80\00\07\0f\66\33\32\2e\72\65\63\6f\64"
  "\69\6e\67\5f\65\71\00\00\0f\66\33\32\2e\72\65\63"
  "\6f\64\69\6e\67\5f\6c\65\00\01\0f\66\33\32\2e\72"
  "\65\63\6f\64\69\6e\67\5f\6c\74\00\02\0f\66\36\34"
  "\2e\72\65\63\6f\64\69\6e\67\5f\65\71\00\03\0f\66"
  "\36\34\2e\72\65\63\6f\64\69\6e\67\5f\6c\65\00\04"
  "\0f\66\36\34\2e\72\65\63\6f\64\69\6e\67\5f\6c\74"
  "\00\05\0f\72\65\63\6f\64\69\6e\67\5f\64\65\6d\6f"
  "\74\65\00\06\0a\e8\80\80\80\00\07\8a\80\80\80\00"
  "\00\20\00\20\01\94\20\00\5b\0b\8a\80\80\80\00\00"
  "\20\00\20\01\94\20\00\5f\0b\8a\80\80\80\00\00\20"
  "\00\20\01\94\20\00\5d\0b\8a\80\80\80\00\00\20\00"
  "\20\01\a2\20\00\61\0b\8a\80\80\80\00\00\20\00\20"
  "\01\a2\20\00\65\0b\8a\80\80\80\00\00\20\00\20\01"
  "\a2\20\00\63\0b\88\80\80\80\00\00\20\00\b6\20\01"
  "\94\0b"
)
(module instance)
(assert_return
  (invoke "f32.recoding_eq" (f32.const -inf) (f32.const 0x1.8p+1))
  (i32.const 0x1)
)
(assert_return
  (invoke "f32.recoding_le" (f32.const -inf) (f32.const 0x1.8p+1))
  (i32.const 0x1)
)
(assert_return
  (invoke "f32.recoding_lt" (f32.const -inf) (f32.const 0x1.8p+1))
  (i32.const 0x0)
)
(assert_return
  (invoke "f32.recoding_eq" (f32.const 0x0p+0) (f32.const 0x1p+0))
  (i32.const 0x1)
)
(assert_return
  (invoke "f32.recoding_le" (f32.const 0x0p+0) (f32.const 0x1p+0))
  (i32.const 0x1)
)
(assert_return
  (invoke "f32.recoding_lt" (f32.const 0x0p+0) (f32.const 0x1p+0))
  (i32.const 0x0)
)
(assert_return
  (invoke "f64.recoding_eq" (f64.const -inf) (f64.const 0x1.8p+1))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.recoding_le" (f64.const -inf) (f64.const 0x1.8p+1))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.recoding_lt" (f64.const -inf) (f64.const 0x1.8p+1))
  (i32.const 0x0)
)
(assert_return
  (invoke "f64.recoding_eq" (f64.const 0x0p+0) (f64.const 0x1p+0))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.recoding_le" (f64.const 0x0p+0) (f64.const 0x1p+0))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.recoding_lt" (f64.const 0x0p+0) (f64.const 0x1p+0))
  (i32.const 0x0)
)
(assert_return
  (invoke "recoding_demote" (f64.const 0x1.4c8f_8p-132) (f32.const 0x1.314p+10))
  (f32.const 0x1.8c8a_1cp-122)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\02\60"
  "\03\7d\7d\7d\01\7f\60\03\7c\7c\7c\01\7f\03\83\80"
  "\80\80\00\02\00\01\07\c1\80\80\80\00\02\1d\66\33"
  "\32\2e\6e\6f\5f\65\78\74\65\6e\64\65\64\5f\70\72"
  "\65\63\69\73\69\6f\6e\5f\64\69\76\00\00\1d\66\36"
  "\34\2e\6e\6f\5f\65\78\74\65\6e\64\65\64\5f\70\72"
  "\65\63\69\73\69\6f\6e\5f\64\69\76\00\01\0a\9f\80"
  "\80\80\00\02\8a\80\80\80\00\00\20\00\20\01\95\20"
  "\02\5b\0b\8a\80\80\80\00\00\20\00\20\01\a3\20\02"
  "\61\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_extended_precision_div"
    (f32.const 0x1.8p+1)
    (f32.const 0x1.cp+2)
    (f32.const 0x1.b6db_6ep-2)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.no_extended_precision_div"
    (f64.const 0x1.8p+1)
    (f64.const 0x1.cp+2)
    (f64.const 0x1.b6db_6db6_db6d_bp-2)
  )
  (i32.const 0x1)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\b5\80\80\80\00\02\17\66\33\32\2e\6e\6f"
  "\5f\64\69\73\74\72\69\62\75\74\65\5f\65\78\61\63"
  "\74\00\00\17\66\36\34\2e\6e\6f\5f\64\69\73\74\72"
  "\69\62\75\74\65\5f\65\78\61\63\74\00\01\0a\b9\80"
  "\80\80\00\02\93\80\80\80\00\00\43\00\00\00\c1\20"
  "\00\94\43\00\00\00\41\20\00\94\92\0b\9b\80\80\80"
  "\00\00\44\00\00\00\00\00\00\20\c0\20\00\a2\44\00"
  "\00\00\00\00\00\20\40\20\00\a2\a0\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_distribute_exact" (f32.const -0x0p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_distribute_exact" (f64.const -0x0p+0))
  (f64.const 0x0p+0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\a9\80\80\80\00\06\60"
  "\01\7d\01\7d\60\04\7d\7d\7d\7d\01\7d\60\03\7d\7d"
  "\7d\01\7d\60\01\7c\01\7c\60\04\7c\7c\7c\7c\01\7c"
  "\60\03\7c\7c\7c\01\7c\03\8b\80\80\80\00\0a\00\01"
  "\02\02\01\03\04\05\05\04\07\b5\81\80\80\00\0a\08"
  "\66\33\32\2e\73\71\72\74\00\00\0f\66\33\32\2e\78"
  "\6b\63\64\5f\73\71\72\74\5f\32\00\01\0f\66\33\32"
  "\2e\78\6b\63\64\5f\73\71\72\74\5f\33\00\02\0f\66"
  "\33\32\2e\78\6b\63\64\5f\73\71\72\74\5f\35\00\03"
  "\16\66\33\32\2e\78\6b\63\64\5f\62\65\74\74\65\72"
  "\5f\73\71\72\74\5f\35\00\04\08\66\36\34\2e\73\71"
  "\72\74\00\05\0f\66\36\34\2e\78\6b\63\64\5f\73\71"
  "\72\74\5f\32\00\06\0f\66\36\34\2e\78\6b\63\64\5f"
  "\73\71\72\74\5f\33\00\07\0f\66\36\34\2e\78\6b\63"
  "\64\5f\73\71\72\74\5f\35\00\08\16\66\36\34\2e\78"
  "\6b\63\64\5f\62\65\74\74\65\72\5f\73\71\72\74\5f"
  "\35\00\09\0a\b1\81\80\80\00\0a\85\80\80\80\00\00"
  "\20\00\91\0b\90\80\80\80\00\00\20\00\20\01\95\20"
  "\02\20\03\20\02\93\95\92\0b\8a\80\80\80\00\00\20"
  "\00\20\01\94\20\02\95\0b\8d\80\80\80\00\00\20\00"
  "\20\01\95\20\02\20\00\95\92\0b\93\80\80\80\00\00"
  "\20\00\20\01\20\02\94\92\20\03\20\01\20\02\94\93"
  "\95\0b\85\80\80\80\00\00\20\00\9f\0b\90\80\80\80"
  "\00\00\20\00\20\01\a3\20\02\20\03\20\02\a1\a3\a0"
  "\0b\8a\80\80\80\00\00\20\00\20\01\a2\20\02\a3\0b"
  "\8d\80\80\80\00\00\20\00\20\01\a3\20\02\20\00\a3"
  "\a0\0b\93\80\80\80\00\00\20\00\20\01\20\02\a2\a0"
  "\20\03\20\01\20\02\a2\a1\a3\0b"
)
(module instance)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1p+1))
  (f32.const 0x1.6a09_e6p+0)
)
(assert_return
  (invoke "f32.xkcd_sqrt_2"
    (f32.const 0x1.8p+1)
    (f32.const 0x1.4p+2)
    (f32.const 0x1.921f_b6p+1)
    (f32.const 0x1.cp+2)
  )
  (f32.const 0x1.6a0a_54p+0)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.8p+1))
  (f32.const 0x1.bb67_aep+0)
)
(assert_return
  (invoke "f32.xkcd_sqrt_3"
    (f32.const 0x1p+1)
    (f32.const 0x1.5bf0_a8p+1)
    (f32.const 0x1.921f_b6p+1)
  )
  (f32.const 0x1.bb02_d4p+0)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.4p+2))
  (f32.const 0x1.1e37_7ap+1)
)
(assert_return
  (invoke "f32.xkcd_sqrt_5"
    (f32.const 0x1p+1)
    (f32.const 0x1.5bf0_a8p+1)
    (f32.const 0x1.8p+1)
  )
  (f32.const 0x1.1e2d_58p+1)
)
(assert_return
  (invoke "f32.xkcd_better_sqrt_5"
    (f32.const 0x1.ap+3)
    (f32.const 0x1p+2)
    (f32.const 0x1.921f_b6p+1)
    (f32.const 0x1.8p+4)
  )
  (f32.const 0x1.1e37_7ap+1)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1p+1))
  (f64.const 0x1.6a09_e667_f3bc_dp+0)
)
(assert_return
  (invoke "f64.xkcd_sqrt_2"
    (f64.const 0x1.8p+1)
    (f64.const 0x1.4p+2)
    (f64.const 0x1.921f_b544_42d1_8p+1)
    (f64.const 0x1.cp+2)
  )
  (f64.const 0x1.6a0a_5362_b055_fp+0)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.8p+1))
  (f64.const 0x1.bb67_ae85_84ca_ap+0)
)
(assert_return
  (invoke "f64.xkcd_sqrt_3"
    (f64.const 0x1p+1)
    (f64.const 0x1.5bf0_a8b1_4576_9p+1)
    (f64.const 0x1.921f_b544_42d1_8p+1)
  )
  (f64.const 0x1.bb02_d4ec_a8f9_5p+0)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.4p+2))
  (f64.const 0x1.1e37_79b9_7f4a_8p+1)
)
(assert_return
  (invoke "f64.xkcd_sqrt_5"
    (f64.const 0x1p+1)
    (f64.const 0x1.5bf0_a8b1_4576_9p+1)
    (f64.const 0x1.8p+1)
  )
  (f64.const 0x1.1e2d_58d8_b3bc_ep+1)
)
(assert_return
  (invoke "f64.xkcd_better_sqrt_5"
    (f64.const 0x1.ap+3)
    (f64.const 0x1p+2)
    (f64.const 0x1.921f_b544_42d1_8p+1)
    (f64.const 0x1.8p+4)
  )
  (f64.const 0x1.1e37_7850_9a5a_3p+1)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\83\80\80\80"
  "\00\02\00\01\07\a9\80\80\80\00\02\11\66\33\32\2e"
  "\63\6f\6d\70\75\74\65\5f\72\61\64\69\78\00\00\11"
  "\66\36\34\2e\63\6f\6d\70\75\74\65\5f\72\61\64\69"
  "\78\00\01\0a\a5\81\80\80\00\02\c3\80\80\80\00\00"
  "\03\40\20\00\20\00\92\22\00\43\00\00\80\3f\92\20"
  "\00\93\43\00\00\80\bf\92\43\00\00\00\00\5b\0d\00"
  "\0b\03\40\20\00\20\01\43\00\00\80\3f\92\22\01\92"
  "\20\00\93\20\01\93\43\00\00\00\00\5c\0d\00\0b\20"
  "\01\0b\d7\80\80\80\00\00\03\40\20\00\20\00\a0\22"
  "\00\44\00\00\00\00\00\00\f0\3f\a0\20\00\a1\44\00"
  "\00\00\00\00\00\f0\bf\a0\44\00\00\00\00\00\00\00"
  "\00\61\0d\00\0b\03\40\20\00\20\01\44\00\00\00\00"
  "\00\00\f0\3f\a0\22\01\a0\20\00\a1\20\01\a1\44\00"
  "\00\00\00\00\00\00\00\62\0d\00\0b\20\01\0b"
)
(module instance)
(assert_return
  (invoke "f32.compute_radix" (f32.const 0x1p+0) (f32.const 0x1p+0))
  (f32.const 0x1p+1)
)
(assert_return
  (invoke "f64.compute_radix" (f64.const 0x1p+0) (f64.const 0x1p+0))
  (f64.const 0x1p+1)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\83\80\80\80"
  "\00\02\00\01\07\b7\80\80\80\00\02\18\66\33\32\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\73\75\62\31\5f\6d\75\6c"
  "\5f\61\64\64\00\00\18\66\36\34\2e\6e\6f\5f\66\6f"
  "\6c\64\5f\73\75\62\31\5f\6d\75\6c\5f\61\64\64\00"
  "\01\0a\af\80\80\80\00\02\90\80\80\80\00\00\20\00"
  "\43\00\00\80\3f\93\20\01\94\20\01\92\0b\94\80\80"
  "\80\00\00\20\00\44\00\00\00\00\00\00\f0\3f\a1\20"
  "\01\a2\20\01\a0\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_sub1_mul_add" (f32.const 0x1p-32) (f32.const 0x1p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_sub1_mul_add" (f64.const 0x1p-64) (f64.const 0x1p+0))
  (f64.const 0x0p+0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\02\60"
  "\03\7d\7d\7d\01\7f\60\03\7c\7c\7c\01\7f\03\85\80"
  "\80\80\00\04\00\00\01\01\07\89\81\80\80\00\04\1f"
  "\66\33\32\2e\6e\6f\5f\66\6f\6c\64\5f\61\64\64\5f"
  "\6c\65\5f\6d\6f\6e\6f\74\6f\6e\69\63\69\74\79\00"
  "\00\1f\66\33\32\2e\6e\6f\5f\66\6f\6c\64\5f\61\64"
  "\64\5f\67\65\5f\6d\6f\6e\6f\74\6f\6e\69\63\69\74"
  "\79\00\01\1f\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f"
  "\61\64\64\5f\6c\65\5f\6d\6f\6e\6f\74\6f\6e\69\63"
  "\69\74\79\00\02\1f\66\36\34\2e\6e\6f\5f\66\6f\6c"
  "\64\5f\61\64\64\5f\67\65\5f\6d\6f\6e\6f\74\6f\6e"
  "\69\63\69\74\79\00\03\0a\c9\80\80\80\00\04\8d\80"
  "\80\80\00\00\20\00\20\02\92\20\01\20\02\92\5f\0b"
  "\8d\80\80\80\00\00\20\00\20\02\92\20\01\20\02\92"
  "\60\0b\8d\80\80\80\00\00\20\00\20\02\a0\20\01\20"
  "\02\a0\65\0b\8d\80\80\80\00\00\20\00\20\02\a0\20"
  "\01\20\02\a0\66\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_add_le_monotonicity"
    (f32.const 0x0p+0)
    (f32.const 0x0p+0)
    (f32.const nan:0x40_0000)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "f32.no_fold_add_le_monotonicity"
    (f32.const inf)
    (f32.const -inf)
    (f32.const inf)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "f64.no_fold_add_le_monotonicity"
    (f64.const 0x0p+0)
    (f64.const 0x0p+0)
    (f64.const nan:0x8_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "f64.no_fold_add_le_monotonicity"
    (f64.const inf)
    (f64.const -inf)
    (f64.const inf)
  )
  (i32.const 0x0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7f\60\02\7c\7c\01\7f\03\89\80\80\80"
  "\00\08\00\00\00\00\01\01\01\01\07\e9\80\80\80\00"
  "\08\0a\66\33\32\2e\6e\6f\74\5f\6c\74\00\00\0a\66"
  "\33\32\2e\6e\6f\74\5f\6c\65\00\01\0a\66\33\32\2e"
  "\6e\6f\74\5f\67\74\00\02\0a\66\33\32\2e\6e\6f\74"
  "\5f\67\65\00\03\0a\66\36\34\2e\6e\6f\74\5f\6c\74"
  "\00\04\0a\66\36\34\2e\6e\6f\74\5f\6c\65\00\05\0a"
  "\66\36\34\2e\6e\6f\74\5f\67\74\00\06\0a\66\36\34"
  "\2e\6e\6f\74\5f\67\65\00\07\0a\e9\80\80\80\00\08"
  "\88\80\80\80\00\00\20\00\20\01\5d\45\0b\88\80\80"
  "\80\00\00\20\00\20\01\5f\45\0b\88\80\80\80\00\00"
  "\20\00\20\01\5e\45\0b\88\80\80\80\00\00\20\00\20"
  "\01\60\45\0b\88\80\80\80\00\00\20\00\20\01\63\45"
  "\0b\88\80\80\80\00\00\20\00\20\01\65\45\0b\88\80"
  "\80\80\00\00\20\00\20\01\64\45\0b\88\80\80\80\00"
  "\00\20\00\20\01\66\45\0b"
)
(module instance)
(assert_return
  (invoke "f32.not_lt" (f32.const nan:0x40_0000) (f32.const 0x0p+0))
  (i32.const 0x1)
)
(assert_return
  (invoke "f32.not_le" (f32.const nan:0x40_0000) (f32.const 0x0p+0))
  (i32.const 0x1)
)
(assert_return
  (invoke "f32.not_gt" (f32.const nan:0x40_0000) (f32.const 0x0p+0))
  (i32.const 0x1)
)
(assert_return
  (invoke "f32.not_ge" (f32.const nan:0x40_0000) (f32.const 0x0p+0))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.not_lt" (f64.const nan:0x8_0000_0000_0000) (f64.const 0x0p+0))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.not_le" (f64.const nan:0x8_0000_0000_0000) (f64.const 0x0p+0))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.not_gt" (f64.const nan:0x8_0000_0000_0000) (f64.const 0x0p+0))
  (i32.const 0x1)
)
(assert_return
  (invoke "f64.not_ge" (f64.const nan:0x8_0000_0000_0000) (f64.const 0x0p+0))
  (i32.const 0x1)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\60"
  "\00\01\7d\60\00\01\7c\03\83\80\80\80\00\02\00\01"
  "\07\9d\80\80\80\00\02\0b\66\33\32\2e\65\70\73\69"
  "\6c\6f\6e\00\00\0b\66\36\34\2e\65\70\73\69\6c\6f"
  "\6e\00\01\0a\dd\80\80\80\00\02\9f\80\80\80\00\00"
  "\43\00\00\80\3f\43\00\00\40\40\43\00\00\80\40\43"
  "\00\00\40\40\95\43\00\00\80\3f\93\94\93\0b\b3\80"
  "\80\80\00\00\44\00\00\00\00\00\00\f0\3f\44\00\00"
  "\00\00\00\00\08\40\44\00\00\00\00\00\00\10\40\44"
  "\00\00\00\00\00\00\08\40\a3\44\00\00\00\00\00\00"
  "\f0\3f\a1\a2\a1\0b"
)
(module instance)
(assert_return (invoke "f32.epsilon") (f32.const -0x1p-23))
(assert_return (invoke "f64.epsilon") (f64.const 0x1p-52))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\60"
  "\00\01\7d\60\00\01\7c\03\83\80\80\80\00\02\00\01"
  "\07\9d\80\80\80\00\02\0b\66\33\32\2e\65\70\73\69"
  "\6c\6f\6e\00\00\0b\66\36\34\2e\65\70\73\69\6c\6f"
  "\6e\00\01\0a\ef\80\80\80\00\02\aa\80\80\80\00\01"
  "\02\7d\43\00\00\80\3f\21\00\03\40\20\00\22\01\43"
  "\00\00\00\3f\94\22\00\43\00\00\80\3f\92\43\00\00"
  "\80\3f\5e\0d\00\0b\20\01\0b\ba\80\80\80\00\01\02"
  "\7c\44\00\00\00\00\00\00\f0\3f\21\00\03\40\20\00"
  "\22\01\44\00\00\00\00\00\00\e0\3f\a2\22\00\44\00"
  "\00\00\00\00\00\f0\3f\a0\44\00\00\00\00\00\00\f0"
  "\3f\64\0d\00\0b\20\01\0b"
)
(module instance)
(assert_return (invoke "f32.epsilon") (f32.const 0x1p-23))
(assert_return (invoke "f64.epsilon") (f64.const 0x1p-52))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7f\60\02\7c\7c\01\7f\03\89\80\80\80"
  "\00\08\00\00\00\00\01\01\01\01\07\b9\81\80\80\00"
  "\08\14\66\33\32\2e\6e\6f\5f\74\72\69\63\68\6f\74"
  "\6f\6d\79\5f\6c\74\00\00\14\66\33\32\2e\6e\6f\5f"
  "\74\72\69\63\68\6f\74\6f\6d\79\5f\6c\65\00\01\14"
  "\66\33\32\2e\6e\6f\5f\74\72\69\63\68\6f\74\6f\6d"
  "\79\5f\67\74\00\02\14\66\33\32\2e\6e\6f\5f\74\72"
  "\69\63\68\6f\74\6f\6d\79\5f\67\65\00\03\14\66\36"
  "\34\2e\6e\6f\5f\74\72\69\63\68\6f\74\6f\6d\79\5f"
  "\6c\74\00\04\14\66\36\34\2e\6e\6f\5f\74\72\69\63"
  "\68\6f\74\6f\6d\79\5f\6c\65\00\05\14\66\36\34\2e"
  "\6e\6f\5f\74\72\69\63\68\6f\74\6f\6d\79\5f\67\74"
  "\00\06\14\66\36\34\2e\6e\6f\5f\74\72\69\63\68\6f"
  "\74\6f\6d\79\5f\67\65\00\07\0a\91\81\80\80\00\08"
  "\8d\80\80\80\00\00\20\00\20\01\5d\20\00\20\01\60"
  "\72\0b\8d\80\80\80\00\00\20\00\20\01\5f\20\00\20"
  "\01\5e\72\0b\8d\80\80\80\00\00\20\00\20\01\5e\20"
  "\00\20\01\5f\72\0b\8d\80\80\80\00\00\20\00\20\01"
  "\60\20\00\20\01\5d\72\0b\8d\80\80\80\00\00\20\00"
  "\20\01\63\20\00\20\01\66\72\0b\8d\80\80\80\00\00"
  "\20\00\20\01\65\20\00\20\01\64\72\0b\8d\80\80\80"
  "\00\00\20\00\20\01\64\20\00\20\01\65\72\0b\8d\80"
  "\80\80\00\00\20\00\20\01\66\20\00\20\01\63\72\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_trichotomy_lt" (f32.const 0x0p+0) (f32.const nan:0x40_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "f32.no_trichotomy_le" (f32.const 0x0p+0) (f32.const nan:0x40_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "f32.no_trichotomy_gt" (f32.const 0x0p+0) (f32.const nan:0x40_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "f32.no_trichotomy_ge" (f32.const 0x0p+0) (f32.const nan:0x40_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "f64.no_trichotomy_lt"
    (f64.const 0x0p+0)
    (f64.const nan:0x8_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "f64.no_trichotomy_le"
    (f64.const 0x0p+0)
    (f64.const nan:0x8_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "f64.no_trichotomy_gt"
    (f64.const 0x0p+0)
    (f64.const nan:0x8_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "f64.no_trichotomy_ge"
    (f64.const 0x0p+0)
    (f64.const nan:0x8_0000_0000_0000)
  )
  (i32.const 0x0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\97\80\80\80\00\04\60"
  "\02\7f\7f\01\7f\60\01\7f\01\7f\60\02\7e\7e\01\7e"
  "\60\01\7e\01\7e\03\94\80\80\80\00\13\00\00\01\02"
  "\02\03\01\01\01\01\01\01\03\03\03\03\03\03\01\07"
  "\ee\83\80\80\00\13\1d\66\33\32\2e\61\72\69\74\68"
  "\6d\65\74\69\63\5f\6e\61\6e\5f\62\69\74\70\61\74"
  "\74\65\72\6e\00\00\1c\66\33\32\2e\63\61\6e\6f\6e"
  "\69\63\61\6c\5f\6e\61\6e\5f\62\69\74\70\61\74\74"
  "\65\72\6e\00\01\20\66\33\32\2e\6e\6f\6e\61\72\69"
  "\74\68\6d\65\74\69\63\5f\6e\61\6e\5f\62\69\74\70"
  "\61\74\74\65\72\6e\00\02\1d\66\36\34\2e\61\72\69"
  "\74\68\6d\65\74\69\63\5f\6e\61\6e\5f\62\69\74\70"
  "\61\74\74\65\72\6e\00\03\1c\66\36\34\2e\63\61\6e"
  "\6f\6e\69\63\61\6c\5f\6e\61\6e\5f\62\69\74\70\61"
  "\74\74\65\72\6e\00\04\20\66\36\34\2e\6e\6f\6e\61"
  "\72\69\74\68\6d\65\74\69\63\5f\6e\61\6e\5f\62\69"
  "\74\70\61\74\74\65\72\6e\00\05\14\66\33\32\2e\6e"
  "\6f\5f\66\6f\6c\64\5f\73\75\62\5f\7a\65\72\6f\00"
  "\06\14\66\33\32\2e\6e\6f\5f\66\6f\6c\64\5f\6e\65"
  "\67\30\5f\73\75\62\00\07\13\66\33\32\2e\6e\6f\5f"
  "\66\6f\6c\64\5f\6d\75\6c\5f\6f\6e\65\00\08\14\66"
  "\33\32\2e\6e\6f\5f\66\6f\6c\64\5f\6e\65\67\31\5f"
  "\6d\75\6c\00\09\13\66\33\32\2e\6e\6f\5f\66\6f\6c"
  "\64\5f\64\69\76\5f\6f\6e\65\00\0a\14\66\33\32\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\64\69\76\5f\6e\65\67\31"
  "\00\0b\14\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\73"
  "\75\62\5f\7a\65\72\6f\00\0c\14\66\36\34\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\6e\65\67\30\5f\73\75\62\00\0d"
  "\13\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f\6d\75\6c"
  "\5f\6f\6e\65\00\0e\14\66\36\34\2e\6e\6f\5f\66\6f"
  "\6c\64\5f\6e\65\67\31\5f\6d\75\6c\00\0f\13\66\36"
  "\34\2e\6e\6f\5f\66\6f\6c\64\5f\64\69\76\5f\6f\6e"
  "\65\00\10\14\66\36\34\2e\6e\6f\5f\66\6f\6c\64\5f"
  "\64\69\76\5f\6e\65\67\31\00\11\16\6e\6f\5f\66\6f"
  "\6c\64\5f\70\72\6f\6d\6f\74\65\5f\64\65\6d\6f\74"
  "\65\00\12\0a\e5\83\80\80\00\13\91\80\80\80\00\00"
  "\20\00\be\20\01\be\95\bc\41\80\80\80\fe\07\71\0b"
  "\91\80\80\80\00\00\20\00\be\20\01\be\95\bc\41\ff"
  "\ff\ff\ff\07\71\0b\87\80\80\80\00\00\20\00\be\8c"
  "\bc\0b\96\80\80\80\00\00\20\00\bf\20\01\bf\a3\bd"
  "\42\80\80\80\80\80\80\80\fc\ff\00\83\0b\96\80\80"
  "\80\00\00\20\00\bf\20\01\bf\a3\bd\42\ff\ff\ff\ff"
  "\ff\ff\ff\ff\ff\00\83\0b\87\80\80\80\00\00\20\00"
  "\bf\9a\bd\0b\93\80\80\80\00\00\20\00\be\43\00\00"
  "\00\00\93\bc\41\80\80\80\fe\07\71\0b\93\80\80\80"
  "\00\00\43\00\00\00\80\20\00\be\93\bc\41\80\80\80"
  "\fe\07\71\0b\93\80\80\80\00\00\20\00\be\43\00\00"
  "\80\3f\94\bc\41\80\80\80\fe\07\71\0b\93\80\80\80"
  "\00\00\43\00\00\80\bf\20\00\be\94\bc\41\80\80\80"
  "\fe\07\71\0b\93\80\80\80\00\00\20\00\be\43\00\00"
  "\80\3f\95\bc\41\80\80\80\fe\07\71\0b\93\80\80\80"
  "\00\00\20\00\be\43\00\00\80\bf\95\bc\41\80\80\80"
  "\fe\07\71\0b\9c\80\80\80\00\00\20\00\bf\44\00\00"
  "\00\00\00\00\00\00\a1\bd\42\80\80\80\80\80\80\80"
  "\fc\ff\00\83\0b\9c\80\80\80\00\00\44\00\00\00\00"
  "\00\00\00\80\20\00\bf\a1\bd\42\80\80\80\80\80\80"
  "\80\fc\ff\00\83\0b\9c\80\80\80\00\00\20\00\bf\44"
  "\00\00\00\00\00\00\f0\3f\a2\bd\42\80\80\80\80\80"
  "\80\80\fc\ff\00\83\0b\9c\80\80\80\00\00\44\00\00"
  "\00\00\00\00\f0\bf\20\00\bf\a2\bd\42\80\80\80\80"
  "\80\80\80\fc\ff\00\83\0b\9c\80\80\80\00\00\20\00"
  "\bf\44\00\00\00\00\00\00\f0\3f\a3\bd\42\80\80\80"
  "\80\80\80\80\fc\ff\00\83\0b\9c\80\80\80\00\00\20"
  "\00\bf\44\00\00\00\00\00\00\f0\bf\a3\bd\42\80\80"
  "\80\80\80\80\80\fc\ff\00\83\0b\8f\80\80\80\00\00"
  "\20\00\be\bb\b6\bc\41\80\80\80\fe\07\71\0b"
)
(module instance)
(assert_return
  (invoke "f32.arithmetic_nan_bitpattern"
    (i32.const 0x7f80_3210)
    (i32.const 0x7f80_3210)
  )
  (i32.const 0x7fc0_0000)
)
(assert_return
  (invoke "f32.canonical_nan_bitpattern" (i32.const 0x0) (i32.const 0x0))
  (i32.const 0x7fc0_0000)
)
(assert_return
  (invoke "f32.canonical_nan_bitpattern"
    (i32.const 0x7fc0_0000)
    (i32.const 0x7fc0_0000)
  )
  (i32.const 0x7fc0_0000)
)
(assert_return
  (invoke "f32.canonical_nan_bitpattern"
    (i32.const 0xffc0_0000)
    (i32.const 0x7fc0_0000)
  )
  (i32.const 0x7fc0_0000)
)
(assert_return
  (invoke "f32.canonical_nan_bitpattern"
    (i32.const 0x7fc0_0000)
    (i32.const 0xffc0_0000)
  )
  (i32.const 0x7fc0_0000)
)
(assert_return
  (invoke "f32.canonical_nan_bitpattern"
    (i32.const 0xffc0_0000)
    (i32.const 0xffc0_0000)
  )
  (i32.const 0x7fc0_0000)
)
(assert_return
  (invoke "f32.nonarithmetic_nan_bitpattern" (i32.const 0x7fc0_3210))
  (i32.const 0xffc0_3210)
)
(assert_return
  (invoke "f32.nonarithmetic_nan_bitpattern" (i32.const 0xffc0_3210))
  (i32.const 0x7fc0_3210)
)
(assert_return
  (invoke "f32.nonarithmetic_nan_bitpattern" (i32.const 0x7f80_3210))
  (i32.const 0xff80_3210)
)
(assert_return
  (invoke "f32.nonarithmetic_nan_bitpattern" (i32.const 0xff80_3210))
  (i32.const 0x7f80_3210)
)
(assert_return
  (invoke "f64.arithmetic_nan_bitpattern"
    (i64.const 0x7ff0_0000_0000_3210)
    (i64.const 0x7ff0_0000_0000_3210)
  )
  (i64.const 0x7ff8_0000_0000_0000)
)
(assert_return
  (invoke "f64.canonical_nan_bitpattern" (i64.const 0x0) (i64.const 0x0))
  (i64.const 0x7ff8_0000_0000_0000)
)
(assert_return
  (invoke "f64.canonical_nan_bitpattern"
    (i64.const 0x7ff8_0000_0000_0000)
    (i64.const 0x7ff8_0000_0000_0000)
  )
  (i64.const 0x7ff8_0000_0000_0000)
)
(assert_return
  (invoke "f64.canonical_nan_bitpattern"
    (i64.const 0xfff8_0000_0000_0000)
    (i64.const 0x7ff8_0000_0000_0000)
  )
  (i64.const 0x7ff8_0000_0000_0000)
)
(assert_return
  (invoke "f64.canonical_nan_bitpattern"
    (i64.const 0x7ff8_0000_0000_0000)
    (i64.const 0xfff8_0000_0000_0000)
  )
  (i64.const 0x7ff8_0000_0000_0000)
)
(assert_return
  (invoke "f64.canonical_nan_bitpattern"
    (i64.const 0xfff8_0000_0000_0000)
    (i64.const 0xfff8_0000_0000_0000)
  )
  (i64.const 0x7ff8_0000_0000_0000)
)
(assert_return
  (invoke "f64.nonarithmetic_nan_bitpattern" (i64.const 0x7ff8_0000_0000_3210))
  (i64.const 0xfff8_0000_0000_3210)
)
(assert_return
  (invoke "f64.nonarithmetic_nan_bitpattern" (i64.const 0xfff8_0000_0000_3210))
  (i64.const 0x7ff8_0000_0000_3210)
)
(assert_return
  (invoke "f64.nonarithmetic_nan_bitpattern" (i64.const 0x7ff0_0000_0000_3210))
  (i64.const 0xfff0_0000_0000_3210)
)
(assert_return
  (invoke "f64.nonarithmetic_nan_bitpattern" (i64.const 0xfff0_0000_0000_3210))
  (i64.const 0x7ff0_0000_0000_3210)
)
(assert_return
  (invoke "f32.no_fold_sub_zero" (i32.const 0x7fa0_0000))
  (i32.const 0x7fc0_0000)
)
(assert_return
  (invoke "f32.no_fold_neg0_sub" (i32.const 0x7fa0_0000))
  (i32.const 0x7fc0_0000)
)
(assert_return
  (invoke "f32.no_fold_mul_one" (i32.const 0x7fa0_0000))
  (i32.const 0x7fc0_0000)
)
(assert_return
  (invoke "f32.no_fold_neg1_mul" (i32.const 0x7fa0_0000))
  (i32.const 0x7fc0_0000)
)
(assert_return
  (invoke "f32.no_fold_div_one" (i32.const 0x7fa0_0000))
  (i32.const 0x7fc0_0000)
)
(assert_return
  (invoke "f32.no_fold_div_neg1" (i32.const 0x7fa0_0000))
  (i32.const 0x7fc0_0000)
)
(assert_return
  (invoke "f64.no_fold_sub_zero" (i64.const 0x7ff4_0000_0000_0000))
  (i64.const 0x7ff8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_neg0_sub" (i64.const 0x7ff4_0000_0000_0000))
  (i64.const 0x7ff8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_mul_one" (i64.const 0x7ff4_0000_0000_0000))
  (i64.const 0x7ff8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_neg1_mul" (i64.const 0x7ff4_0000_0000_0000))
  (i64.const 0x7ff8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_div_one" (i64.const 0x7ff4_0000_0000_0000))
  (i64.const 0x7ff8_0000_0000_0000)
)
(assert_return
  (invoke "f64.no_fold_div_neg1" (i64.const 0x7ff4_0000_0000_0000))
  (i64.const 0x7ff8_0000_0000_0000)
)
(assert_return
  (invoke "no_fold_promote_demote" (i32.const 0x7fa0_0000))
  (i32.const 0x7fc0_0000)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\01\60"
  "\08\7c\7c\7c\7c\7c\7c\7c\7c\01\7c\03\83\80\80\80"
  "\00\02\00\00\07\b2\80\80\80\00\02\13\64\6f\74\5f"
  "\70\72\6f\64\75\63\74\5f\65\78\61\6d\70\6c\65\00"
  "\00\18\77\69\74\68\5f\62\69\6e\61\72\79\5f\73\75"
  "\6d\5f\63\6f\6c\6c\61\70\73\65\00\01\0a\bd\80\80"
  "\80\00\02\99\80\80\80\00\00\20\00\20\04\a2\20\01"
  "\20\05\a2\a0\20\02\20\06\a2\a0\20\03\20\07\a2\a0"
  "\0b\99\80\80\80\00\00\20\00\20\04\a2\20\01\20\05"
  "\a2\a0\20\02\20\06\a2\20\03\20\07\a2\a0\a0\0b"
)
(module instance)
(assert_return
  (invoke "dot_product_example"
    (f64.const 0x1.e848p+24)
    (f64.const 0x1p+0)
    (f64.const -0x1p+0)
    (f64.const 0x1.312dp+26)
    (f64.const 0x1.312dp+25)
    (f64.const 0x1p+0)
    (f64.const -0x1p+0)
    (f64.const -0x1.e848p+23)
  )
  (f64.const 0x1p+1)
)
(assert_return
  (invoke "with_binary_sum_collapse"
    (f64.const 0x1.e848p+24)
    (f64.const 0x1p+0)
    (f64.const -0x1p+0)
    (f64.const 0x1.312dp+26)
    (f64.const 0x1.312dp+25)
    (f64.const 0x1p+0)
    (f64.const -0x1p+0)
    (f64.const -0x1.e848p+23)
  )
  (f64.const 0x1p+1)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\83\80\80\80"
  "\00\02\00\01\07\a7\80\80\80\00\02\10\66\33\32\2e"
  "\63\6f\6e\74\72\61\63\74\32\66\6d\61\00\00\10\66"
  "\36\34\2e\63\6f\6e\74\72\61\63\74\32\66\6d\61\00"
  "\01\0a\a7\80\80\80\00\02\8e\80\80\80\00\00\20\00"
  "\20\00\94\20\01\20\01\94\93\91\0b\8e\80\80\80\00"
  "\00\20\00\20\00\a2\20\01\20\01\a2\a1\9f\0b"
)
(module instance)
(assert_return
  (invoke "f32.contract2fma" (f32.const 0x1p+0) (f32.const 0x1p+0))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.contract2fma"
    (f32.const 0x1.1999_9ap+0)
    (f32.const 0x1.1999_9ap+0)
  )
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.contract2fma"
    (f32.const 0x1.3333_32p+0)
    (f32.const 0x1.3333_32p+0)
  )
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f64.contract2fma" (f64.const 0x1p+0) (f64.const 0x1p+0))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.contract2fma"
    (f64.const 0x1.1999_9999_9999_ap+0)
    (f64.const 0x1.1999_9999_9999_ap+0)
  )
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.contract2fma"
    (f64.const 0x1.3333_3333_3333_3p+0)
    (f64.const 0x1.3333_3333_3333_3p+0)
  )
  (f64.const 0x0p+0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\02\60"
  "\03\7d\7d\7d\01\7d\60\03\7c\7c\7c\01\7c\03\83\80"
  "\80\80\00\02\00\01\07\bf\80\80\80\00\02\1c\66\33"
  "\32\2e\64\69\76\69\73\69\6f\6e\5f\62\79\5f\73\6d"
  "\61\6c\6c\5f\6e\75\6d\62\65\72\00\00\1c\66\36\34"
  "\2e\64\69\76\69\73\69\6f\6e\5f\62\79\5f\73\6d\61"
  "\6c\6c\5f\6e\75\6d\62\65\72\00\01\0a\9f\80\80\80"
  "\00\02\8a\80\80\80\00\00\20\00\20\01\20\02\95\93"
  "\0b\8a\80\80\80\00\00\20\00\20\01\20\02\a3\a1\0b"
)
(module instance)
(assert_return
  (invoke "f32.division_by_small_number"
    (f32.const 0x1.ab3fp+26)
    (f32.const 0x1.86ap+16)
    (f32.const 0x1.d7db_f4p-11)
  )
  (f32.const 0x1.b207p+19)
)
(assert_return
  (invoke "f64.division_by_small_number"
    (f64.const 0x1.ab3fp+26)
    (f64.const 0x1.86ap+16)
    (f64.const 0x1.d7db_f487_fcb9_2p-11)
  )
  (f64.const 0x1.b207_1c71_c718p+19)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\02\60"
  "\03\7d\7d\7d\01\7d\60\03\7c\7c\7c\01\7c\03\83\80"
  "\80\80\00\02\00\01\07\a7\80\80\80\00\02\10\66\33"
  "\32\2e\67\6f\6c\64\65\6e\5f\72\61\74\69\6f\00\00"
  "\10\66\36\34\2e\67\6f\6c\64\65\6e\5f\72\61\74\69"
  "\6f\00\01\0a\a1\80\80\80\00\02\8b\80\80\80\00\00"
  "\20\00\20\01\20\02\91\92\94\0b\8b\80\80\80\00\00"
  "\20\00\20\01\20\02\9f\a0\a2\0b"
)
(module instance)
(assert_return
  (invoke "f32.golden_ratio"
    (f32.const 0x1p-1)
    (f32.const 0x1p+0)
    (f32.const 0x1.4p+2)
  )
  (f32.const 0x1.9e37_7ap+0)
)
(assert_return
  (invoke "f64.golden_ratio"
    (f64.const 0x1p-1)
    (f64.const 0x1p+0)
    (f64.const 0x1.4p+2)
  )
  (f64.const 0x1.9e37_79b9_7f4a_8p+0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\60"
  "\01\7d\01\7d\60\01\7c\01\7c\03\83\80\80\80\00\02"
  "\00\01\07\a7\80\80\80\00\02\10\66\33\32\2e\73\69"
  "\6c\76\65\72\5f\6d\65\61\6e\73\00\00\10\66\36\34"
  "\2e\73\69\6c\76\65\72\5f\6d\65\61\6e\73\00\01\0a"
  "\c1\80\80\80\00\02\97\80\80\80\00\00\43\00\00\00"
  "\3f\20\00\20\00\20\00\94\43\00\00\80\40\92\91\92"
  "\94\0b\9f\80\80\80\00\00\44\00\00\00\00\00\00\e0"
  "\3f\20\00\20\00\20\00\a2\44\00\00\00\00\00\00\10"
  "\40\a0\9f\a0\a2\0b"
)
(module instance)
(assert_return
  (invoke "f32.silver_means" (f32.const 0x0p+0))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "f32.silver_means" (f32.const 0x1p+0))
  (f32.const 0x1.9e37_7ap+0)
)
(assert_return
  (invoke "f32.silver_means" (f32.const 0x1p+1))
  (f32.const 0x1.3504_f4p+1)
)
(assert_return
  (invoke "f32.silver_means" (f32.const 0x1.8p+1))
  (f32.const 0x1.a6c1_5ap+1)
)
(assert_return
  (invoke "f32.silver_means" (f32.const 0x1p+2))
  (f32.const 0x1.0f1b_bcp+2)
)
(assert_return
  (invoke "f32.silver_means" (f32.const 0x1.4p+2))
  (f32.const 0x1.4c53_44p+2)
)
(assert_return
  (invoke "f64.silver_means" (f64.const 0x0p+0))
  (f64.const 0x1p+0)
)
(assert_return
  (invoke "f64.silver_means" (f64.const 0x1p+0))
  (f64.const 0x1.9e37_79b9_7f4a_8p+0)
)
(assert_return
  (invoke "f64.silver_means" (f64.const 0x1p+1))
  (f64.const 0x1.3504_f333_f9de_6p+1)
)
(assert_return
  (invoke "f64.silver_means" (f64.const 0x1.8p+1))
  (f64.const 0x1.a6c1_5a23_0acf_ap+1)
)
(assert_return
  (invoke "f64.silver_means" (f64.const 0x1p+2))
  (f64.const 0x1.0f1b_bcdc_bfa5_4p+2)
)
(assert_return
  (invoke "f64.silver_means" (f64.const 0x1.4p+2))
  (f64.const 0x1.4c53_4525_46cf_ap+2)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\87\80\80\80\00\01\60"
  "\02\7c\7c\01\7f\03\82\80\80\80\00\01\00\07\8e\80"
  "\80\80\00\01\0a\70\6f\69\6e\74\5f\66\6f\75\72\00"
  "\00\0a\97\80\80\80\00\01\91\80\80\80\00\00\20\00"
  "\20\01\a3\44\9a\99\99\99\99\99\d9\3f\63\0b"
)
(module instance)
(assert_return
  (invoke "point_four" (f64.const 0x1p+2) (f64.const 0x1.4p+3))
  (i32.const 0x0)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7f\01\7c\03\82\80\80\80\00\01\00\07\87\80\80"
  "\80\00\01\03\74\61\75\00\00\0a\ce\81\80\80\00\01"
  "\c8\81\80\80\00\01\04\7c\44\00\00\00\00\00\00\00"
  "\00\21\01\02\40\20\00\41\01\48\0d\00\44\00\00\00"
  "\00\00\00\f0\3f\21\02\44\00\00\00\00\00\00\00\00"
  "\21\03\03\40\20\01\20\02\44\00\00\00\00\00\00\20"
  "\40\20\03\44\00\00\00\00\00\00\20\40\a2\22\04\44"
  "\00\00\00\00\00\00\f0\3f\a0\a3\44\00\00\00\00\00"
  "\00\10\40\20\04\44\00\00\00\00\00\00\10\40\a0\a3"
  "\a1\44\00\00\00\00\00\00\00\40\20\04\44\00\00\00"
  "\00\00\00\14\40\a0\a3\a1\44\00\00\00\00\00\00\00"
  "\40\20\04\44\00\00\00\00\00\00\18\40\a0\a3\a1\a2"
  "\a0\21\01\20\03\44\00\00\00\00\00\00\f0\3f\a0\21"
  "\03\20\02\44\00\00\00\00\00\00\b0\3f\a2\21\02\20"
  "\00\41\7f\6a\22\00\0d\00\0b\0b\20\01\0b"
)
(module instance)
(assert_return
  (invoke "tau" (i32.const 0xa))
  (f64.const 0x1.921f_b544_42d1_4p+2)
)
(assert_return
  (invoke "tau" (i32.const 0xb))
  (f64.const 0x1.921f_b544_42d1_8p+2)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\02\60"
  "\02\7d\7d\01\7d\60\02\7c\7c\01\7c\03\83\80\80\80"
  "\00\02\00\01\07\bd\80\80\80\00\02\1b\66\33\32\2e"
  "\6e\6f\5f\66\6f\6c\64\5f\63\6f\6e\64\69\74\69\6f"
  "\6e\61\6c\5f\69\6e\63\00\00\1b\66\36\34\2e\6e\6f"
  "\5f\66\6f\6c\64\5f\63\6f\6e\64\69\74\69\6f\6e\61"
  "\6c\5f\69\6e\63\00\01\0a\bd\80\80\80\00\02\95\80"
  "\80\80\00\00\20\00\20\00\43\00\00\80\3f\92\20\01"
  "\43\00\00\00\00\5d\1b\0b\9d\80\80\80\00\00\20\00"
  "\20\00\44\00\00\00\00\00\00\f0\3f\a0\20\01\44\00"
  "\00\00\00\00\00\00\00\63\1b\0b"
)
(module instance)
(assert_return
  (invoke "f32.no_fold_conditional_inc" (f32.const -0x0p+0) (f32.const -0x1p+0))
  (f32.const -0x0p+0)
)
(assert_return
  (invoke "f64.no_fold_conditional_inc" (f64.const -0x0p+0) (f64.const -0x1p+0))
  (f64.const -0x0p+0)
)
