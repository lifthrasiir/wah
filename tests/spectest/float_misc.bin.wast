(module definition binary
  "\00\61\73\6d\01\00\00\00\01\97\80\80\80\00\04\60"
  "\02\7d\7d\01\7d\60\01\7d\01\7d\60\02\7c\7c\01\7c"
  "\60\01\7c\01\7c\03\9d\80\80\80\00\1c\00\00\00\00"
  "\01\01\01\00\01\01\01\01\00\00\02\02\02\02\03\03"
  "\03\02\03\03\03\03\02\02\07\b7\82\80\80\00\1c\07"
  "\66\33\32\2e\61\64\64\00\00\07\66\33\32\2e\73\75"
  "\62\00\01\07\66\33\32\2e\6d\75\6c\00\02\07\66\33"
  "\32\2e\64\69\76\00\03\08\66\33\32\2e\73\71\72\74"
  "\00\04\07\66\33\32\2e\61\62\73\00\05\07\66\33\32"
  "\2e\6e\65\67\00\06\0c\66\33\32\2e\63\6f\70\79\73"
  "\69\67\6e\00\07\08\66\33\32\2e\63\65\69\6c\00\08"
  "\09\66\33\32\2e\66\6c\6f\6f\72\00\09\09\66\33\32"
  "\2e\74\72\75\6e\63\00\0a\0b\66\33\32\2e\6e\65\61"
  "\72\65\73\74\00\0b\07\66\33\32\2e\6d\69\6e\00\0c"
  "\07\66\33\32\2e\6d\61\78\00\0d\07\66\36\34\2e\61"
  "\64\64\00\0e\07\66\36\34\2e\73\75\62\00\0f\07\66"
  "\36\34\2e\6d\75\6c\00\10\07\66\36\34\2e\64\69\76"
  "\00\11\08\66\36\34\2e\73\71\72\74\00\12\07\66\36"
  "\34\2e\61\62\73\00\13\07\66\36\34\2e\6e\65\67\00"
  "\14\0c\66\36\34\2e\63\6f\70\79\73\69\67\6e\00\15"
  "\08\66\36\34\2e\63\65\69\6c\00\16\09\66\36\34\2e"
  "\66\6c\6f\6f\72\00\17\09\66\36\34\2e\74\72\75\6e"
  "\63\00\18\0b\66\36\34\2e\6e\65\61\72\65\73\74\00"
  "\19\07\66\36\34\2e\6d\69\6e\00\1a\07\66\36\34\2e"
  "\6d\61\78\00\1b\0a\b5\82\80\80\00\1c\87\80\80\80"
  "\00\00\20\00\20\01\92\0b\87\80\80\80\00\00\20\00"
  "\20\01\93\0b\87\80\80\80\00\00\20\00\20\01\94\0b"
  "\87\80\80\80\00\00\20\00\20\01\95\0b\85\80\80\80"
  "\00\00\20\00\91\0b\85\80\80\80\00\00\20\00\8b\0b"
  "\85\80\80\80\00\00\20\00\8c\0b\87\80\80\80\00\00"
  "\20\00\20\01\98\0b\85\80\80\80\00\00\20\00\8d\0b"
  "\85\80\80\80\00\00\20\00\8e\0b\85\80\80\80\00\00"
  "\20\00\8f\0b\85\80\80\80\00\00\20\00\90\0b\87\80"
  "\80\80\00\00\20\00\20\01\96\0b\87\80\80\80\00\00"
  "\20\00\20\01\97\0b\87\80\80\80\00\00\20\00\20\01"
  "\a0\0b\87\80\80\80\00\00\20\00\20\01\a1\0b\87\80"
  "\80\80\00\00\20\00\20\01\a2\0b\87\80\80\80\00\00"
  "\20\00\20\01\a3\0b\85\80\80\80\00\00\20\00\9f\0b"
  "\85\80\80\80\00\00\20\00\99\0b\85\80\80\80\00\00"
  "\20\00\9a\0b\87\80\80\80\00\00\20\00\20\01\a6\0b"
  "\85\80\80\80\00\00\20\00\9b\0b\85\80\80\80\00\00"
  "\20\00\9c\0b\85\80\80\80\00\00\20\00\9d\0b\85\80"
  "\80\80\00\00\20\00\9e\0b\87\80\80\80\00\00\20\00"
  "\20\01\a4\0b\87\80\80\80\00\00\20\00\20\01\a5\0b"
)
(module instance)
(assert_return
  (invoke "f32.add" (f32.const 0x1.1f9a_dep+0) (f32.const 0x1.0f78_2cp-33))
  (f32.const 0x1.1f9a_dep+0)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.1f9a_dd37_3963_6p+0)
    (f64.const 0x1.0f78_2bf7_cc83_fp-33)
  )
  (f64.const 0x1.1f9a_dd37_c11f_7p+0)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1p+0) (f32.const 0x1p-24))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1p+0) (f32.const 0x1.0000_02p-24))
  (f32.const 0x1.0000_02p+0)
)
(assert_return
  (invoke "f64.add" (f64.const 0x1p+0) (f64.const 0x1p-53))
  (f64.const 0x1p+0)
)
(assert_return
  (invoke "f64.add" (f64.const 0x1p+0) (f64.const 0x1.0000_0000_0000_1p-53))
  (f64.const 0x1.0000_0000_0000_1p+0)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1p-149) (f32.const 0x1.ffff_fcp-127))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x0.0000_0000_0000_1p-1022)
    (f64.const 0x0.ffff_ffff_ffff_fp-1022)
  )
  (f64.const 0x1p-1022)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1p+31) (f32.const 0x1.001p+10))
  (f32.const 0x1.0000_08p+31)
)
(assert_return
  (invoke "f64.add" (f64.const 0x1p+63) (f64.const 0x1.001p+10))
  (f64.const 0x1.0000_0000_0000_1p+63)
)
(assert_return
  (invoke "f64.add"
    (f64.const -0x1p-1008)
    (f64.const 0x0.0000_0000_0171_6p-1022)
  )
  (f64.const -0x1.ffff_ffff_ffff_fp-1009)
)
(assert_return
  (invoke "f64.add" (f64.const 0x1p+53) (f64.const 0x1.0000_a7c5_ac47_2p+0))
  (f64.const 0x1.0000_0000_0000_1p+53)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.0000_0000_0000_1p+53)
    (f64.const 0x1.fffep-1)
  )
  (f64.const 0x1.0000_0000_0000_1p+53)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1p+23) (f32.const 0x1p-1))
  (f32.const 0x1p+23)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1.0000_02p+23) (f32.const 0x1p-1))
  (f32.const 0x1.0000_04p+23)
)
(assert_return
  (invoke "f64.add" (f64.const 0x1p+52) (f64.const 0x1p-1))
  (f64.const 0x1p+52)
)
(assert_return
  (invoke "f64.add" (f64.const 0x1.0000_0000_0000_1p+52) (f64.const 0x1p-1))
  (f64.const 0x1.0000_0000_0000_2p+52)
)
(assert_return
  (invoke "f32.add" (f32.const -0x1.3967_5ap+102) (f32.const 0x1.76c9_4cp-99))
  (f32.const -0x1.3967_5ap+102)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1.6c0f_24p+67) (f32.const -0x1.2b92_dp+52))
  (f32.const 0x1.6c0c_ccp+67)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1.e623_18p-83) (f32.const 0x1.f74a_bep-125))
  (f32.const 0x1.e623_18p-83)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1.2a71_d4p+39) (f32.const -0x1.c9f1_0cp+55))
  (f32.const -0x1.c9ef_e2p+55)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1.f8f7_36p-15) (f32.const 0x1.7bd4_5ep+106))
  (f32.const 0x1.7bd4_5ep+106)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.f33e_1fbc_a27a_ap-413)
    (f64.const -0x1.6b19_2891_ed61p+249)
  )
  (f64.const -0x1.6b19_2891_ed61p+249)
)
(assert_return
  (invoke "f64.add"
    (f64.const -0x1.46f7_5d13_0eeb_1p+76)
    (f64.const 0x1.2527_5d6f_7a4a_cp-184)
  )
  (f64.const -0x1.46f7_5d13_0eeb_1p+76)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.04de_c926_5a73_1p-148)
    (f64.const -0x1.11ee_d4e8_c127_cp-12)
  )
  (f64.const -0x1.11ee_d4e8_c127_cp-12)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.0577_3b71_66b0_ap+497)
    (f64.const 0x1.1340_22f2_da37_bp+66)
  )
  (f64.const 0x1.0577_3b71_66b0_ap+497)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.ef4f_7942_82a8_2p+321)
    (f64.const 0x1.14a8_2266_badep+394)
  )
  (f64.const 0x1.14a8_2266_badep+394)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1.1bf9_76p+72) (f32.const -0x1.7f58_68p+20))
  (f32.const 0x1.1bf9_76p+72)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1.7f9c_6cp-45) (f32.const -0x1.b9bb_0ep-78))
  (f32.const 0x1.7f9c_6cp-45)
)
(assert_return
  (invoke "f32.add" (f32.const -0x1.32d1_bcp-42) (f32.const 0x1.f7d2_14p+125))
  (f32.const 0x1.f7d2_14p+125)
)
(assert_return
  (invoke "f32.add" (f32.const -0x1.8e5c_0ep-44) (f32.const -0x1.3afa_4cp-106))
  (f32.const -0x1.8e5c_0ep-44)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1.13cd_78p-10) (f32.const -0x1.3af3_16p-107))
  (f32.const 0x1.13cd_78p-10)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.f8dd_15ca_97d4_ap+179)
    (f64.const -0x1.3673_17d1_fe8b_fp-527)
  )
  (f64.const 0x1.f8dd_15ca_97d4_ap+179)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.5db0_8d73_9228_cp+155)
    (f64.const -0x1.fb31_6fa1_47dc_bp-61)
  )
  (f64.const 0x1.5db0_8d73_9228_cp+155)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.bbb4_03cb_85c0_7p-404)
    (f64.const -0x1.7e44_046b_8bbf_3p-979)
  )
  (f64.const 0x1.bbb4_03cb_85c0_7p-404)
)
(assert_return
  (invoke "f64.add"
    (f64.const -0x1.34d3_8af2_9183_1p+147)
    (f64.const -0x1.9890_b474_3995_3p+139)
  )
  (f64.const -0x1.366c_1ba7_05bc_ap+147)
)
(assert_return
  (invoke "f64.add"
    (f64.const -0x1.b61d_edf4_e030_6p+3)
    (f64.const 0x1.09e2_f317_73c4_ap+290)
  )
  (f64.const 0x1.09e2_f317_73c4_ap+290)
)
(assert_return
  (invoke "f32.add" (f32.const -0x1.129b_d8p-117) (f32.const 0x1.c750_12p-43))
  (f32.const 0x1.c750_12p-43)
)
(assert_return
  (invoke "f32.add" (f32.const -0x1.c204_a2p-16) (f32.const 0x1.80b1_32p-27))
  (f32.const -0x1.c1d4_8cp-16)
)
(assert_return
  (invoke "f32.add" (f32.const -0x1.decc_1cp+36) (f32.const 0x1.c688_dap-109))
  (f32.const -0x1.decc_1cp+36)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1.61ce_6ap-118) (f32.const -0x1.7728_92p+30))
  (f32.const -0x1.7728_92p+30)
)
(assert_return
  (invoke "f32.add" (f32.const -0x1.3dc8_26p-120) (f32.const 0x1.fc3f_66p+95))
  (f32.const 0x1.fc3f_66p+95)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.bf68_acc2_63a0_fp-777)
    (f64.const -0x1.5f93_5296_5e5a_6p+1004)
  )
  (f64.const -0x1.5f93_5296_5e5a_6p+1004)
)
(assert_return
  (invoke "f64.add"
    (f64.const -0x1.76ea_a709_11f5_1p+516)
    (f64.const -0x1.2d74_6324_ce47_ap+493)
  )
  (f64.const -0x1.76ea_a963_fabb_6p+516)
)
(assert_return
  (invoke "f64.add"
    (f64.const -0x1.b637_d82c_15a7_ap-967)
    (f64.const 0x1.cc65_4cca_b415_2p-283)
  )
  (f64.const 0x1.cc65_4cca_b415_2p-283)
)
(assert_return
  (invoke "f64.add"
    (f64.const -0x1.a5b1_fb66_e846_ep-509)
    (f64.const 0x1.4bdd_36f0_bb5c_cp-860)
  )
  (f64.const -0x1.a5b1_fb66_e846_ep-509)
)
(assert_return
  (invoke "f64.add"
    (f64.const -0x1.1410_8da8_80f9_ep+966)
    (f64.const 0x1.417f_3570_1e89_fp+800)
  )
  (f64.const -0x1.1410_8da8_80f9_ep+966)
)
(assert_return
  (invoke "f64.add"
    (f64.const -0x1.fa0c_af21_ffeb_cp+804)
    (f64.const 0x1.4ca8_fdcf_f89f_9p+826)
  )
  (f64.const 0x1.4ca8_f5e7_c5e3_1p+826)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.016f_1fcb_dfd3_8p+784)
    (f64.const 0x1.375d_ffcb_c9a2_cp+746)
  )
  (f64.const 0x1.016f_1fcb_e4b0_fp+784)
)
(assert_return
  (invoke "f64.add"
    (f64.const -0x1.dffd_a6d5_bff3_ap+624)
    (f64.const 0x1.f9e8_cc2d_ff78_2p+674)
  )
  (f64.const 0x1.f9e8_cc2d_ff77_bp+674)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.fff4_b436_87df_bp+463)
    (f64.const 0x1.0fd5_617c_4a80_9p+517)
  )
  (f64.const 0x1.0fd5_617c_4a80_9p+517)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.535d_3800_35da_2p-995)
    (f64.const 0x1.cce3_7ddd_bb73_bp-963)
  )
  (f64.const 0x1.cce3_7ddf_0ed0_fp-963)
)
(assert_return
  (invoke "f64.add"
    (f64.const -0x1.d91c_d3fc_0c66_fp+752)
    (f64.const -0x1.4e18_c802_2973_4p+952)
  )
  (f64.const -0x1.4e18_c802_2973_4p+952)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.afc7_0fd3_6e37_2p+193)
    (f64.const -0x1.bd10_a9b3_77b4_6p+273)
  )
  (f64.const -0x1.bd10_a9b3_77b4_6p+273)
)
(assert_return
  (invoke "f64.add"
    (f64.const -0x1.2abd_570b_078b_2p+302)
    (f64.const 0x1.b3c1_ad75_9cb5_bp-423)
  )
  (f64.const -0x1.2abd_570b_078b_2p+302)
)
(assert_return
  (invoke "f64.add"
    (f64.const -0x1.5b2a_e84c_0686_cp-317)
    (f64.const -0x1.dba7_a1c0_2282_3p+466)
  )
  (f64.const -0x1.dba7_a1c0_2282_3p+466)
)
(assert_return
  (invoke "f64.add"
    (f64.const -0x1.ac62_7bd7_cbf3_8p-198)
    (f64.const 0x1.2312_e265_b8d5_9p-990)
  )
  (f64.const -0x1.ac62_7bd7_cbf3_8p-198)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1.2b91_ap+116) (f32.const 0x1.cbcd_52p+127))
  (f32.const 0x1.cbf2_c4p+127)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1.96f3_92p+127) (f32.const -0x1.6b3f_ecp+107))
  (f32.const 0x1.96f3_7cp+127)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1.132f_1cp+118) (f32.const -0x1.63d6_32p+127))
  (f32.const -0x1.634c_9ap+127)
)
(assert_return
  (invoke "f32.add" (f32.const -0x1.1dda_64p+120) (f32.const -0x1.ef02_ep+127))
  (f32.const -0x1.f13e_94p+127)
)
(assert_return
  (invoke "f32.add" (f32.const -0x1.4ad8_dap+127) (f32.const -0x1.eae0_82p+125))
  (f32.const -0x1.c590_fap+127)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.0170_99f2_a4b8_bp+1023)
    (f64.const 0x1.1f63_b28f_0545_4p+981)
  )
  (f64.const 0x1.0170_99f2_a500_9p+1023)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.d88b_6c74_984e_fp+1023)
    (f64.const 0x1.33b4_4477_5eab_cp+990)
  )
  (f64.const 0x1.d88b_6c75_3229_1p+1023)
)
(assert_return
  (invoke "f64.add"
    (f64.const -0x1.8457_6422_fdf5p+1023)
    (f64.const 0x1.60ee_6aa1_2fb9_cp+1012)
  )
  (f64.const -0x1.842b_4655_a9cf_1p+1023)
)
(assert_return
  (invoke "f64.add"
    (f64.const -0x1.9aaa_ce3e_79f7_dp+1001)
    (f64.const 0x1.e406_8af2_95cb_6p+1023)
  )
  (f64.const 0x1.e406_8487_ea92_6p+1023)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.06cd_ae79_f27b_9p+1023)
    (f64.const -0x1.e05c_b0c9_6f97_5p+991)
  )
  (f64.const 0x1.06cd_ae78_121e_ep+1023)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1.6a1a_2p-127) (f32.const 0x1.378p-140))
  (f32.const 0x1.6a23_dcp-127)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1.28p-144) (f32.const -0x1p-148))
  (f32.const 0x1.18p-144)
)
(assert_return
  (invoke "f32.add" (f32.const -0x1p-146) (f32.const 0x1.c3cap-128))
  (f32.const 0x1.c3c9_cp-128)
)
(assert_return
  (invoke "f32.add" (f32.const -0x1.4p-145) (f32.const 0x1.4240_52p-122))
  (f32.const 0x1.4240_5p-122)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1.c5p-141) (f32.const -0x1.72f8p-135))
  (f32.const -0x1.6be4p-135)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.4774_c681_d1e2_1p-1022)
    (f64.const -0x1.271e_58e9_f58c_ap-1021)
  )
  (f64.const -0x1.06c7_eb52_1937_3p-1022)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.10b3_a75e_3191_6p-1021)
    (f64.const -0x1.ffb8_2b0e_868a_7p-1021)
  )
  (f64.const -0x1.de09_0760_a9f2_2p-1022)
)
(assert_return
  (invoke "f64.add"
    (f64.const -0x0.6b58_448b_8098_ap-1022)
    (f64.const -0x1.5797_96ed_04cb_ep-1022)
  )
  (f64.const -0x1.c2ef_db78_8564_8p-1022)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.9eb9_e7ba_ae8d_1p-1020)
    (f64.const -0x1.d58e_136f_8c6e_ep-1020)
  )
  (f64.const -0x0.db50_aed3_7787_4p-1022)
)
(assert_return
  (invoke "f64.add"
    (f64.const -0x1.f111_5dee_afa0_bp-1022)
    (f64.const 0x1.221b_1c87_dca2_9p-1022)
  )
  (f64.const -0x0.cef6_4166_d2fe_2p-1022)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1.ffff_fcp+127) (f32.const 0x1p+104))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "f64.add" (f64.const 0x1.ffff_ffff_ffff_ep+1023) (f64.const 0x1p+971))
  (f64.const 0x1.ffff_ffff_ffff_fp+1023)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1p+1) (f32.const 0x1p+1))
  (f32.const 0x1p+2)
)
(assert_return
  (invoke "f64.add" (f64.const 0x1p+1) (f64.const 0x1p+1))
  (f64.const 0x1p+2)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1.ffff_fep+127) (f32.const 0x1.ffff_fep+102))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "f32.add" (f32.const 0x1.ffff_fep+127) (f32.const 0x1p+103))
  (f32.const inf)
)
(assert_return
  (invoke "f64.add"
    (f64.const 0x1.ffff_ffff_ffff_fp+1023)
    (f64.const 0x1.ffff_ffff_ffff_fp+969)
  )
  (f64.const 0x1.ffff_ffff_ffff_fp+1023)
)
(assert_return
  (invoke "f64.add" (f64.const 0x1.ffff_ffff_ffff_fp+1023) (f64.const 0x1p+970))
  (f64.const inf)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1p+16) (f32.const 0x1p-37))
  (f32.const 0x1p+16)
)
(assert_return
  (invoke "f64.sub" (f64.const 0x1p+16) (f64.const 0x1p-37))
  (f64.const 0x1.ffff_ffff_ffff_fp+15)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1p+0) (f32.const 0x1p-25))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1p+0) (f32.const 0x1.0000_02p-25))
  (f32.const 0x1.ffff_fep-1)
)
(assert_return
  (invoke "f64.sub" (f64.const 0x1p+0) (f64.const 0x1p-54))
  (f64.const 0x1p+0)
)
(assert_return
  (invoke "f64.sub" (f64.const 0x1p+0) (f64.const 0x1.0000_0000_0000_1p-54))
  (f64.const 0x1.ffff_ffff_ffff_fp-1)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1.ee24_66p-106) (f32.const -0x1.1627_7ep+119))
  (f32.const 0x1.1627_7ep+119)
)
(assert_return
  (invoke "f32.sub" (f32.const -0x1.446f_9ep+119) (f32.const -0x1.4396_a4p+43))
  (f32.const -0x1.446f_9ep+119)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1.7477_3cp+0) (f32.const -0x1.a255_12p-82))
  (f32.const 0x1.7477_3cp+0)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1.9345_c4p-117) (f32.const 0x1.6792_c2p-76))
  (f32.const -0x1.6792_c2p-76)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1.9ecf_a4p-18) (f32.const -0x1.864b_44p-107))
  (f32.const 0x1.9ecf_a4p-18)
)
(assert_return
  (invoke "f64.sub"
    (f64.const -0x1.5b79_8875_e784_5p-333)
    (f64.const -0x1.b514_7117_452f_ep-903)
  )
  (f64.const -0x1.5b79_8875_e784_5p-333)
)
(assert_return
  (invoke "f64.sub"
    (f64.const -0x1.6c87_baeb_6d72_dp+552)
    (f64.const -0x1.64fb_35d4_b557_1p-158)
  )
  (f64.const -0x1.6c87_baeb_6d72_dp+552)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.b3d3_69fc_f74bp-461)
    (f64.const -0x1.ea16_68c0_dec9_3p-837)
  )
  (f64.const 0x1.b3d3_69fc_f74bp-461)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.0abd_4493_53ea_dp-1005)
    (f64.const -0x1.0422_ea3e_82ee_9p+154)
  )
  (f64.const 0x1.0422_ea3e_82ee_9p+154)
)
(assert_return
  (invoke "f64.sub"
    (f64.const -0x1.aadb_c6b4_3cc3_dp-143)
    (f64.const -0x1.e7f9_22ef_1ee5_8p-539)
  )
  (f64.const -0x1.aadb_c6b4_3cc3_dp-143)
)
(assert_return
  (invoke "f32.sub" (f32.const -0x1.61e2_62p+108) (f32.const -0x1.baf3_e4p+112))
  (f32.const 0x1.a4d5_bep+112)
)
(assert_return
  (invoke "f32.sub" (f32.const -0x1.62c2_f6p+109) (f32.const 0x1.6e51_4ap+6))
  (f32.const -0x1.62c2_f6p+109)
)
(assert_return
  (invoke "f32.sub" (f32.const -0x1.287c_94p-83) (f32.const 0x1.0f2f_9cp-24))
  (f32.const -0x1.0f2f_9cp-24)
)
(assert_return
  (invoke "f32.sub" (f32.const -0x1.c882_5cp-77) (f32.const -0x1.4aea_d6p-12))
  (f32.const 0x1.4aea_d6p-12)
)
(assert_return
  (invoke "f32.sub" (f32.const -0x1.2976_a4p+99) (f32.const 0x1.c6e3_b8p-59))
  (f32.const -0x1.2976_a4p+99)
)
(assert_return
  (invoke "f64.sub"
    (f64.const -0x1.76cb_28ae_6c04_5p+202)
    (f64.const -0x1.0611_f2af_4e9b_9p+901)
  )
  (f64.const 0x1.0611_f2af_4e9b_9p+901)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.baf3_5eff_22e9_ep-368)
    (f64.const 0x1.5c3e_08ec_f73e_cp-451)
  )
  (f64.const 0x1.baf3_5eff_22e9_ep-368)
)
(assert_return
  (invoke "f64.sub"
    (f64.const -0x1.8fd3_54b3_76f1_fp-200)
    (f64.const 0x1.513c_860f_386f_fp-508)
  )
  (f64.const -0x1.8fd3_54b3_76f1_fp-200)
)
(assert_return
  (invoke "f64.sub"
    (f64.const -0x1.760d_4472_30ae_6p-992)
    (f64.const -0x1.16f7_8843_8ae3_ep-328)
  )
  (f64.const 0x1.16f7_8843_8ae3_ep-328)
)
(assert_return
  (invoke "f64.sub"
    (f64.const -0x1.73aa_b4fc_fc7ap+112)
    (f64.const 0x1.7c58_9f99_0b88_4p+171)
  )
  (f64.const -0x1.7c58_9f99_0b88_4p+171)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1.ea26_4cp+95) (f32.const 0x1.8529_88p-15))
  (f32.const 0x1.ea26_4cp+95)
)
(assert_return
  (invoke "f32.sub" (f32.const -0x1.14ec_7cp+19) (f32.const -0x1.0ad3_fep-35))
  (f32.const -0x1.14ec_7cp+19)
)
(assert_return
  (invoke "f32.sub" (f32.const -0x1.3251_dap-36) (f32.const -0x1.49c9_7ep-56))
  (f32.const -0x1.3251_c6p-36)
)
(assert_return
  (invoke "f32.sub" (f32.const -0x1.1356_5ep-14) (f32.const 0x1.2f89_a8p-13))
  (f32.const -0x1.b934_d8p-13)
)
(assert_return
  (invoke "f32.sub" (f32.const -0x1.6032_b6p-33) (f32.const -0x1.bb51_96p-104))
  (f32.const -0x1.6032_b6p-33)
)
(assert_return
  (invoke "f64.sub"
    (f64.const -0x1.b5b0_797a_f491p-157)
    (f64.const -0x1.694b_8348_189e_8p+722)
  )
  (f64.const 0x1.694b_8348_189e_8p+722)
)
(assert_return
  (invoke "f64.sub"
    (f64.const -0x1.72b1_4282_6ed7_3p+759)
    (f64.const -0x1.0104_77bc_9afb_dp+903)
  )
  (f64.const 0x1.0104_77bc_9afb_dp+903)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.8327_3b6b_b94c_fp-796)
    (f64.const 0x1.1a93_f948_a2ab_bp+181)
  )
  (f64.const -0x1.1a93_f948_a2ab_bp+181)
)
(assert_return
  (invoke "f64.sub"
    (f64.const -0x1.207e_7156_cbf2p-573)
    (f64.const 0x1.cf3f_12fd_3814_dp-544)
  )
  (f64.const -0x1.cf3f_1306_3c08_6p-544)
)
(assert_return
  (invoke "f64.sub"
    (f64.const -0x1.837e_6844_f171_8p-559)
    (f64.const -0x1.1c29_b757_f98a_bp-14)
  )
  (f64.const 0x1.1c29_b757_f98a_bp-14)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.c211_51a7_09b6_cp-78)
    (f64.const 0x1.0a12_fff8_910f_6p-115)
  )
  (f64.const 0x1.c211_51a7_0166_3p-78)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.c579_12aa_e2f6_4p-982)
    (f64.const 0x1.dbfb_d480_0b7c_fp-1010)
  )
  (f64.const 0x1.c579_128d_2338_fp-982)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.ffef_4399_af9c_6p-254)
    (f64.const 0x1.edb9_6dfa_ea8b_1p-200)
  )
  (f64.const -0x1.edb9_6dfa_ea8b_1p-200)
)
(assert_return
  (invoke "f64.sub"
    (f64.const -0x1.363e_ee39_1cde_2p-39)
    (f64.const -0x1.a654_6200_0265_fp-69)
  )
  (f64.const -0x1.363e_ee32_838c_9p-39)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.5901_6dba_002a_1p-25)
    (f64.const 0x1.5d43_74f1_24cc_cp-3)
  )
  (f64.const -0x1.5d43_6f8d_1f15_dp-3)
)
(assert_return
  (invoke "f64.sub"
    (f64.const -0x1.1819_6bca_005c_fp-814)
    (f64.const -0x1.db7b_01ce_3f52_fp-766)
  )
  (f64.const 0x1.db7b_01ce_3f51_dp-766)
)
(assert_return
  (invoke "f64.sub"
    (f64.const -0x1.d17b_3528_d219p+33)
    (f64.const 0x1.fd73_9d4e_a220_ap+367)
  )
  (f64.const -0x1.fd73_9d4e_a220_ap+367)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.dea4_6994_de31_9p+114)
    (f64.const 0x1.b5b1_9cd5_5c7d_3p-590)
  )
  (f64.const 0x1.dea4_6994_de31_9p+114)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.b60f_9b2f_bd9e_cp-489)
    (f64.const -0x1.6f81_c59e_c5b8_ep-694)
  )
  (f64.const 0x1.b60f_9b2f_bd9e_cp-489)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.5e42_3fe8_571f_4p-57)
    (f64.const 0x1.9624_ed7c_162d_fp-618)
  )
  (f64.const 0x1.5e42_3fe8_571f_4p-57)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1.7240_46p+4) (f32.const 0x1.921f_b6p+1))
  (f32.const 0x1.3ffc_5p+4)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.7240_46eb_0933_ap+4)
    (f64.const 0x1.921f_b544_42d1_8p+1)
  )
  (f64.const 0x1.3ffc_5042_80d9_7p+4)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1.6e35_f8p+21) (f32.const 0x1.6e35_fp+21))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1.e847_fp+20) (f32.const 0x1.e847_bp+20))
  (f32.const 0x1p+2)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1.e847_fp+20) (f32.const 0x1.e847_9p+20))
  (f32.const 0x1.8p+2)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1.86a0_8p+18) (f32.const 0x1.86a0_4p+18))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1.86a0_8p+18) (f32.const 0x1.86ap+18))
  (f32.const 0x1p+1)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.550f_7dca_6fff_ep+51)
    (f64.const 0x1.550f_7dca_6fff_cp+51)
  )
  (f64.const 0x1p+0)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.c6bf_5263_3fff_cp+50)
    (f64.const 0x1.c6bf_5263_3ffe_cp+50)
  )
  (f64.const 0x1p+2)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.c6bf_5263_3fff_cp+50)
    (f64.const 0x1.c6bf_5263_3ffe_4p+50)
  )
  (f64.const 0x1.8p+2)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.6bcc_41e9_0002p+48)
    (f64.const 0x1.6bcc_41e9_0001p+48)
  )
  (f64.const 0x1p+0)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.6bcc_41e9_0002p+48)
    (f64.const 0x1.6bcc_41e9p+48)
  )
  (f64.const 0x1p+1)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1p-126) (f32.const 0x1.ffff_fcp-127))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1p-1022)
    (f64.const 0x0.ffff_ffff_ffff_fp-1022)
  )
  (f64.const 0x0.0000_0000_0000_1p-1022)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1.0000_02p+0) (f32.const 0x1.ffff_fep-1))
  (f32.const 0x1.8p-23)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1.0000_02p+0) (f32.const 0x1p+0))
  (f32.const 0x1p-23)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1p+0) (f32.const 0x1.ffff_fep-1))
  (f32.const 0x1p-24)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.0000_0000_0000_1p+0)
    (f64.const 0x1.ffff_ffff_ffff_fp-1)
  )
  (f64.const 0x1.8p-52)
)
(assert_return
  (invoke "f64.sub" (f64.const 0x1.0000_0000_0000_1p+0) (f64.const 0x1p+0))
  (f64.const 0x1p-52)
)
(assert_return
  (invoke "f64.sub" (f64.const 0x1p+0) (f64.const 0x1.ffff_ffff_ffff_fp-1))
  (f64.const 0x1p-53)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1.ffff_fep+127) (f32.const 0x1.ffff_fep+102))
  (f32.const 0x1.ffff_fep+127)
)
(assert_return
  (invoke "f32.sub" (f32.const 0x1.ffff_fep+127) (f32.const 0x1p+103))
  (f32.const 0x1.ffff_fcp+127)
)
(assert_return
  (invoke "f64.sub"
    (f64.const 0x1.ffff_ffff_ffff_fp+1023)
    (f64.const 0x1.ffff_ffff_ffff_fp+969)
  )
  (f64.const 0x1.ffff_ffff_ffff_fp+1023)
)
(assert_return
  (invoke "f64.sub" (f64.const 0x1.ffff_ffff_ffff_fp+1023) (f64.const 0x1p+970))
  (f64.const 0x1.ffff_ffff_ffff_ep+1023)
)
(assert_return
  (invoke "f32.mul" (f32.const 0x1.c6bf_52p+49) (f32.const 0x1.c6bf_52p+49))
  (f32.const 0x1.93e5_92p+99)
)
(assert_return
  (invoke "f32.mul" (f32.const 0x1.5af1_d8p+66) (f32.const 0x1.5af1_d8p+66))
  (f32.const inf)
)
(assert_return
  (invoke "f32.mul" (f32.const 0x1.08b2_a2p+83) (f32.const 0x1.08b2_a2p+83))
  (f32.const inf)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.c6bf_5263_4p+49)
    (f64.const 0x1.c6bf_5263_4p+49)
  )
  (f64.const 0x1.93e5_939a_08ce_ap+99)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.5af1_d78b_58c4p+66)
    (f64.const 0x1.5af1_d78b_58c4p+66)
  )
  (f64.const 0x1.d632_9f1c_35ca_5p+132)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.08b2_a2c2_8029_1p+83)
    (f64.const 0x1.08b2_a2c2_8029_1p+83)
  )
  (f64.const 0x1.11b0_ec57_e649_bp+166)
)
(assert_return
  (invoke "f32.mul" (f32.const 0x1.b8ce_5ep+30) (f32.const 0x1.2958_9ep+34))
  (f32.const 0x1.0000_02p+65)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.b8ce_5d7cp+30)
    (f64.const 0x1.2958_9d27_cp+34)
  )
  (f64.const 0x1.0000_0000_0000_1p+65)
)
(assert_return
  (invoke "f32.mul" (f32.const 0x1.3466_66p+6) (f32.const 0x1.a9p+9))
  (f32.const 0x1.fffep+15)
)
(assert_return
  (invoke "f64.mul" (f64.const 0x1.3466_6666_6666_6p+6) (f64.const 0x1.a9p+9))
  (f64.const 0x1.fffd_ffff_ffff_fp+15)
)
(assert_return
  (invoke "f32.mul" (f32.const -0x1.14df_2ep+61) (f32.const 0x1.7488_78p-36))
  (f32.const -0x1.92e7_e8p+25)
)
(assert_return
  (invoke "f32.mul" (f32.const -0x1.5629_e2p+102) (f32.const -0x1.c330_12p-102))
  (f32.const 0x1.2d86_04p+1)
)
(assert_return
  (invoke "f32.mul" (f32.const -0x1.b176_94p+92) (f32.const -0x1.e4b5_6ap-97))
  (f32.const 0x1.9a5b_aep-4)
)
(assert_return
  (invoke "f32.mul" (f32.const -0x1.1626_a6p+79) (f32.const -0x1.c57d_7p-75))
  (f32.const 0x1.ecba_aep+4)
)
(assert_return
  (invoke "f32.mul" (f32.const 0x1.7acf_72p+53) (f32.const 0x1.6c89_acp+5))
  (f32.const 0x1.0db5_56p+59)
)
(assert_return
  (invoke "f64.mul"
    (f64.const -0x1.25c2_93f6_f37e_4p+425)
    (f64.const 0x1.f5fd_4fa4_1c6d_8p+945)
  )
  (f64.const -inf)
)
(assert_return
  (invoke "f64.mul"
    (f64.const -0x1.cc1a_e79f_ffc5_bp-986)
    (f64.const -0x1.c36c_cc28_61ca_6p-219)
  )
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.c023_2b3e_64b5_6p+606)
    (f64.const -0x1.f693_9cf3_affa_ap+106)
  )
  (f64.const -0x1.b7e3_aedf_190d_3p+713)
)
(assert_return
  (invoke "f64.mul"
    (f64.const -0x1.60f2_8996_6b27_1p-313)
    (f64.const 0x1.28a5_497f_0c25_9p+583)
  )
  (f64.const -0x1.98fc_50bc_ec25_9p+270)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.37da_b12d_3afa_2p+795)
    (f64.const 0x1.81e1_56bd_393f_1p-858)
  )
  (f64.const 0x1.d612_6554_b829_8p-63)
)
(assert_return
  (invoke "f32.mul" (f32.const -0x1.3f57_a2p-89) (f32.const -0x1.041d_68p+92))
  (f32.const 0x1.4479_bp+3)
)
(assert_return
  (invoke "f32.mul" (f32.const 0x1.4d05_82p+73) (f32.const 0x1.6e04_3ap+19))
  (f32.const 0x1.dc23_6p+92)
)
(assert_return
  (invoke "f32.mul" (f32.const -0x1.2fdap-32) (f32.const -0x1.e173_1cp+74))
  (f32.const 0x1.1db8_9ep+43)
)
(assert_return
  (invoke "f32.mul" (f32.const 0x1.7bc8_fep+67) (f32.const -0x1.3ad5_92p+15))
  (f32.const -0x1.d311_5ep+82)
)
(assert_return
  (invoke "f32.mul" (f32.const 0x1.9367_42p+30) (f32.const -0x1.a7a1_9p+66))
  (f32.const -0x1.4dc7_1ap+97)
)
(assert_return
  (invoke "f64.mul"
    (f64.const -0x1.ba73_7b4c_a3b1_3p-639)
    (f64.const 0x1.8923_3098_5743_8p-314)
  )
  (f64.const -0x1.53bc_0d07_baa3_7p-952)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.7c19_32e6_1021_9p-276)
    (f64.const -0x1.2605_db64_6489_fp-635)
  )
  (f64.const -0x1.b48d_a2b0_d2ae_3p-911)
)
(assert_return
  (invoke "f64.mul"
    (f64.const -0x1.e43c_df3b_2108p+329)
    (f64.const -0x1.99d9_6abb_d61d_1p+835)
  )
  (f64.const inf)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.4c19_4665_51da_3p+947)
    (f64.const 0x1.0bdc_d6c7_646e_9p-439)
  )
  (f64.const 0x1.5b7c_d8c3_f638_ap+508)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.ff1d_a172_6e3d_fp+339)
    (f64.const -0x1.043c_44f5_2b15_8p+169)
  )
  (f64.const -0x1.03c9_364b_b585_cp+509)
)
(assert_return
  (invoke "f32.mul" (f32.const -0x1.907e_8ap+46) (f32.const -0x1.5d36_68p+95))
  (f32.const inf)
)
(assert_return
  (invoke "f32.mul" (f32.const -0x1.8c9f_74p-3) (f32.const 0x1.e2b4_52p-99))
  (f32.const -0x1.75ed_ccp-101)
)
(assert_return
  (invoke "f32.mul" (f32.const -0x1.cc60_5ap-19) (f32.const 0x1.ec32_1ap+105))
  (f32.const -0x1.ba91_a4p+87)
)
(assert_return
  (invoke "f32.mul" (f32.const -0x1.5fbb_7ap+56) (f32.const 0x1.a896_5ep-96))
  (f32.const -0x1.23ae_8ep-39)
)
(assert_return
  (invoke "f32.mul" (f32.const -0x1.fb7f_12p+16) (f32.const 0x1.3a70_1ap-119))
  (f32.const -0x1.37ac_0cp-102)
)
(assert_return
  (invoke "f64.mul"
    (f64.const -0x1.5b02_6645_4c26_bp-496)
    (f64.const -0x1.af57_87e3_e039_9p+433)
  )
  (f64.const 0x1.2457_d819_49e0_bp-62)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.0d54_a823_93d4_5p+478)
    (f64.const -0x1.4257_6080_7cea_ep-764)
  )
  (f64.const -0x1.5320_68c8_d0d5_dp-286)
)
(assert_return
  (invoke "f64.mul"
    (f64.const -0x1.b532_af98_1786p+172)
    (f64.const 0x1.ada9_5085_ba36_fp+359)
  )
  (f64.const -0x1.6ee3_8c1e_0186_4p+532)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.e132_f4d4_9d1c_ep+768)
    (f64.const -0x1.a75a_fe9a_7d86_4p+374)
  )
  (f64.const -inf)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.68bb_f1cf_ff90_ap+81)
    (f64.const 0x1.09cd_17d6_52c5p+70)
  )
  (f64.const 0x1.768b_8d67_d794p+151)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.f99f_b602_c89b_7p-341)
    (f64.const 0x1.6caa_b46a_31a2_ep-575)
  )
  (f64.const 0x1.6820_1f98_6e9d_7p-915)
)
(assert_return
  (invoke "f64.mul"
    (f64.const -0x1.8699_9c5e_ee37_9p-9)
    (f64.const 0x1.6e3b_9e0d_53e0_dp+723)
  )
  (f64.const -0x1.1765_4a0e_f35f_5p+715)
)
(assert_return
  (invoke "f64.mul"
    (f64.const -0x1.0695_71b1_76f9p+367)
    (f64.const -0x1.e248_b6ab_0a0e_3p-652)
  )
  (f64.const 0x1.eeaf_f575_cae1_dp-285)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.c217_6457_77dd_2p+775)
    (f64.const 0x1.d93f_5715_dd64_6p+60)
  )
  (f64.const 0x1.a006_4aa1_d920_dp+836)
)
(assert_return
  (invoke "f64.mul"
    (f64.const -0x1.8489_81b6_e694_ap-276)
    (f64.const 0x1.f5aa_cb64_a0d1_9p+896)
  )
  (f64.const -0x1.7cb2_296e_6c2e_5p+621)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.db3b_d2a2_8694_4p-599)
    (f64.const 0x1.ce91_0af1_d55c_ap-425)
  )
  (f64.const 0x0.d6ac_cdd5_38a3_9p-1022)
)
(assert_return
  (invoke "f64.mul"
    (f64.const -0x1.aca2_2391_6012p-57)
    (f64.const -0x1.2b2b_4958_dd22_8p-966)
  )
  (f64.const 0x0.fa74_ecca_e561_5p-1022)
)
(assert_return
  (invoke "f64.mul"
    (f64.const -0x1.bd06_2def_16cf_fp-488)
    (f64.const -0x1.7ddd_91a0_c4c0_ep-536)
  )
  (f64.const 0x0.a5f4_d776_9d90_dp-1022)
)
(assert_return
  (invoke "f64.mul"
    (f64.const -0x1.c6a5_6169_e9cep-772)
    (f64.const 0x1.517d_55a4_7412_2p-255)
  )
  (f64.const -0x0.12ba_f260_afb7_7p-1022)
)
(assert_return
  (invoke "f64.mul"
    (f64.const -0x1.0895_1b0b_4170_5p-516)
    (f64.const -0x1.102d_c271_68d0_9p-507)
  )
  (f64.const 0x0.8ca6_dbf3_f592_bp-1022)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.8d0d_ea50_c8c9_bp+852)
    (f64.const 0x1.21ca_c31d_87a2_4p-881)
  )
  (f64.const 0x1.c177_311f_7cd7_3p-29)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.9804_9118_e306_3p-7)
    (f64.const 0x1.6362_5251_51b5_8p-149)
  )
  (f64.const 0x1.1b35_8514_103f_9p-155)
)
(assert_return
  (invoke "f64.mul"
    (f64.const -0x1.ea65_cb06_3132_3p+1)
    (f64.const 0x1.fce6_8320_1a19_bp-41)
  )
  (f64.const -0x1.e76d_c8c2_2366_7p-39)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.e4d2_3596_1d54_3p-373)
    (f64.const 0x1.bc56_f20e_f9a4_8p-205)
  )
  (f64.const 0x1.a4c0_9efc_b71d_6p-577)
)
(assert_return
  (invoke "f64.mul"
    (f64.const -0x1.b961_2e66_faba_8p+77)
    (f64.const 0x1.e2bc_6aa7_8227_3p-348)
  )
  (f64.const -0x1.a026_ea4f_81db_1p-270)
)
(assert_return
  (invoke "f32.mul" (f32.const 0x1p-75) (f32.const 0x1p-75))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.mul" (f32.const 0x1.0000_02p-75) (f32.const 0x1.0000_02p-75))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.6a09_e667_f3bc_cp-538)
    (f64.const 0x1.6a09_e667_f3bc_cp-538)
  )
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.6a09_e667_f3bc_dp-538)
    (f64.const 0x1.6a09_e667_f3bc_dp-538)
  )
  (f64.const 0x0.0000_0000_0000_1p-1022)
)
(assert_return
  (invoke "f32.mul" (f32.const 0x1.ffff_fep+63) (f32.const 0x1.ffff_fep+63))
  (f32.const 0x1.ffff_fcp+127)
)
(assert_return
  (invoke "f32.mul" (f32.const 0x1p+64) (f32.const 0x1p+64))
  (f32.const inf)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.ffff_ffff_ffff_fp+511)
    (f64.const 0x1.ffff_ffff_ffff_fp+511)
  )
  (f64.const 0x1.ffff_ffff_ffff_ep+1023)
)
(assert_return
  (invoke "f64.mul" (f64.const 0x1p+512) (f64.const 0x1p+512))
  (f64.const inf)
)
(assert_return
  (invoke "f32.mul" (f32.const 0x1.0000_02p+0) (f32.const 0x1.0000_02p+0))
  (f32.const 0x1.0000_04p+0)
)
(assert_return
  (invoke "f32.mul" (f32.const 0x1.ffff_fep-1) (f32.const 0x1.ffff_fep-1))
  (f32.const 0x1.ffff_fcp-1)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.0000_0000_0000_1p+0)
    (f64.const 0x1.0000_0000_0000_1p+0)
  )
  (f64.const 0x1.0000_0000_0000_2p+0)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.ffff_ffff_ffff_fp-1)
    (f64.const 0x1.ffff_ffff_ffff_fp-1)
  )
  (f64.const 0x1.ffff_ffff_ffff_ep-1)
)
(assert_return
  (invoke "f32.mul" (f32.const 0x1.0000_02p+0) (f32.const 0x1.ffff_fep-1))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "f32.mul" (f32.const 0x1.0000_04p+0) (f32.const 0x1.ffff_fcp-1))
  (f32.const 0x1.0000_02p+0)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.0000_0000_0000_1p+0)
    (f64.const 0x1.ffff_ffff_ffff_fp-1)
  )
  (f64.const 0x1p+0)
)
(assert_return
  (invoke "f64.mul"
    (f64.const 0x1.0000_0000_0000_2p+0)
    (f64.const 0x1.ffff_ffff_ffff_ep-1)
  )
  (f64.const 0x1.0000_0000_0000_1p+0)
)
(assert_return
  (invoke "f32.mul" (f32.const 0x1p-126) (f32.const 0x1p-23))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "f64.mul" (f64.const 0x1p-1022) (f64.const 0x1p-52))
  (f64.const 0x0.0000_0000_0000_1p-1022)
)
(assert_return
  (invoke "f32.mul" (f32.const -0x1.0006p+4) (f32.const 0x1.ap-132))
  (f32.const -0x1.a009_cp-128)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.1f9a_dep+0) (f32.const 0x1.9p+6))
  (f32.const 0x1.7022_64p-7)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.0017_e6p+23) (f32.const 0x1.7fff_f2p+23))
  (f32.const 0x1.5575_4p-1)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1p+16) (f32.const 0x1p-37))
  (f32.const 0x1p+53)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.dcbf_6ap+0) (f32.const 0x1.ffff_fep+127))
  (f32.const 0x1.dcbf_68p-128)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1p+2) (f32.const 0x1.8p+1))
  (f32.const 0x1.5555_56p+0)
)
(assert_return
  (invoke "f64.div" (f64.const 0x1.1f9a_dd37_3963_6p+0) (f64.const 0x1.9p+6))
  (f64.const 0x1.7022_62d6_0c04_5p-7)
)
(assert_return
  (invoke "f64.div" (f64.const 0x1.0017_e6p+23) (f64.const 0x1.7fff_f2p+23))
  (f64.const 0x1.5575_3f1d_9ba2_7p-1)
)
(assert_return
  (invoke "f64.div" (f64.const 0x1p+16) (f64.const 0x1p-37))
  (f64.const 0x1p+53)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.dcbf_6ap+0)
    (f64.const 0x1.ffff_ffff_ffff_fp+1023)
  )
  (f64.const 0x0.772f_da8p-1022)
)
(assert_return
  (invoke "f64.div" (f64.const 0x1p+2) (f64.const 0x1.8p+1))
  (f64.const 0x1.5555_5555_5555_5p+0)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.0017_ecp+22) (f32.const 0x1.7fff_f8p+21))
  (f32.const 0x1.5575_42p+0)
)
(assert_return
  (invoke "f64.div" (f64.const 0x1.0017_ecp+22) (f64.const 0x1.7fff_f8p+21))
  (f64.const 0x1.5575_41c7_c6b4_3p+0)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.6a6c_5ap-48) (f32.const 0x1.fa0b_7p+127))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.616f_b2p-87) (f32.const 0x1.3321_72p+68))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.div" (f32.const -0x1.96e7_78p+16) (f32.const 0x1.eb0c_56p-80))
  (f32.const -0x1.a844_0ap+95)
)
(assert_return
  (invoke "f32.div" (f32.const -0x1.e262_4p-76) (f32.const -0x1.ed23_6ep-122))
  (f32.const 0x1.f4d5_84p+45)
)
(assert_return
  (invoke "f32.div" (f32.const -0x1.e237_4ep+41) (f32.const 0x1.71fc_dcp-80))
  (f32.const -0x1.4da7_06p+121)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.163c_09d0_c38c_1p+147)
    (f64.const 0x1.e04c_c737_348e_6p+223)
  )
  (f64.const 0x1.2899_21ca_eed2_3p-77)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.d686_7e74_1e0a_9p-626)
    (f64.const 0x1.335e_b19a_9aae_4p-972)
  )
  (f64.const 0x1.87e3_42d1_1f51_9p+346)
)
(assert_return
  (invoke "f64.div"
    (f64.const -0x1.d5ed_f648_aeb9_8p+298)
    (f64.const 0x1.0dda_15b0_7935_5p+640)
  )
  (f64.const -0x1.bdce_af97_34b5_cp-342)
)
(assert_return
  (invoke "f64.div"
    (f64.const -0x1.b683_e393_4aed_ap+691)
    (f64.const 0x1.c364_e1df_00df_fp+246)
  )
  (f64.const -0x1.f164_56e7_afe3_bp+444)
)
(assert_return
  (invoke "f64.div"
    (f64.const -0x1.44ca_7539_cc85_1p+540)
    (f64.const 0x1.5850_1bcc_c58f_ep+453)
  )
  (f64.const -0x1.e2f8_657e_0924_ep+86)
)
(assert_return
  (invoke "f32.div" (f32.const -0x1.c2c5_4ap+69) (f32.const -0x1.00d1_42p-86))
  (f32.const inf)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.e35a_bep-46) (f32.const 0x1.c69d_fp+44))
  (f32.const 0x1.102e_b4p-90)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.45ff_2ap+0) (f32.const -0x1.1e87_54p+89))
  (f32.const -0x1.2343_4ep-89)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.8db1_8ap-51) (f32.const 0x1.47c6_78p-128))
  (f32.const 0x1.369b_96p+77)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.7859_9p+90) (f32.const 0x1.5341_44p+87))
  (f32.const 0x1.1bfd_dcp+3)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x0.f331_c4f4_7eb5_1p-1022)
    (f64.const -0x1.c7ff_45bf_6f03_ap+362)
  )
  (f64.const -0x0p+0)
)
(assert_return
  (invoke "f64.div"
    (f64.const -0x1.0fc8_707b_9d19_cp-987)
    (f64.const 0x1.7752_4d5f_4a56_3p-536)
  )
  (f64.const -0x1.72c1_a937_d231p-452)
)
(assert_return
  (invoke "f64.div"
    (f64.const -0x1.edb3_aa64_bb33_8p-403)
    (f64.const -0x1.1c7c_1643_20e4p+45)
  )
  (f64.const 0x1.bc44_cc1c_5ae6_3p-448)
)
(assert_return
  (invoke "f64.div"
    (f64.const -0x1.6534_b34e_8686_bp+80)
    (f64.const 0x1.c34a_7fc5_9e3c_3p-791)
  )
  (f64.const -0x1.9542_1bf2_91b6_6p+870)
)
(assert_return
  (invoke "f64.div"
    (f64.const -0x1.91f5_8d7e_d123_7p+236)
    (f64.const -0x1.f190_d808_383c_8p+55)
  )
  (f64.const 0x1.9d9e_b083_6f90_6p+180)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.64b2_a4p+26) (f32.const 0x1.e957_52p-119))
  (f32.const inf)
)
(assert_return
  (invoke "f32.div" (f32.const -0x1.53c9_b6p+77) (f32.const 0x1.d689_ap+27))
  (f32.const -0x1.71ba_a4p+49)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.664a_8ap+38) (f32.const -0x1.59db_a2p+96))
  (f32.const -0x1.0933_f4p-58)
)
(assert_return
  (invoke "f32.div" (f32.const -0x1.99e0_fap+111) (f32.const -0x1.c2b5_a8p+9))
  (f32.const 0x1.d19d_e6p+101)
)
(assert_return
  (invoke "f32.div" (f32.const -0x1.5a81_5ap+92) (f32.const -0x1.b582_0ap+13))
  (f32.const 0x1.9580_b8p+78)
)
(assert_return
  (invoke "f64.div"
    (f64.const -0x1.81fd_1e2a_f7be_bp-655)
    (f64.const 0x1.edef_c4ea_e536_cp-691)
  )
  (f64.const -0x1.901a_bdd9_1b66_1p+35)
)
(assert_return
  (invoke "f64.div"
    (f64.const -0x1.47cf_9329_53c4_3p+782)
    (f64.const -0x1.bc40_496b_1f2a_1p-553)
  )
  (f64.const inf)
)
(assert_return
  (invoke "f64.div"
    (f64.const -0x1.2bd2_e8fb_dcad_7p-746)
    (f64.const 0x1.b115_674c_c476_ep-65)
  )
  (f64.const -0x1.6275_2bf1_9fa8_1p-682)
)
(assert_return
  (invoke "f64.div"
    (f64.const -0x1.f923_e3fe_a9ef_ep+317)
    (f64.const -0x1.8044_c74d_27a3_9p-588)
  )
  (f64.const 0x1.5086_518c_c718_6p+905)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.516e_d205_1d6b_bp+181)
    (f64.const -0x1.c9f4_55eb_9c2e_ep+214)
  )
  (f64.const -0x1.7941_4d67_f288_9p-34)
)
(assert_return
  (invoke "f64.div"
    (f64.const -0x1.9c52_726a_ed36_6p+585)
    (f64.const -0x1.7d05_68c7_5660_fp+195)
  )
  (f64.const 0x1.1507_ca2a_65f2_3p+390)
)
(assert_return
  (invoke "f64.div"
    (f64.const -0x1.5226_72f4_6166_7p+546)
    (f64.const -0x1.36d3_6572_c9f7_1p+330)
  )
  (f64.const 0x1.1681_3693_7061_9p+216)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.0105_1b4e_8cd6_1p+185)
    (f64.const -0x1.2cbb_5ca3_d33e_bp+965)
  )
  (f64.const -0x1.b594_7159_8a2f_3p-781)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.5f93_bb80_fc2c_bp+217)
    (f64.const 0x1.7e05_1aae_9f0e_dp+427)
  )
  (f64.const 0x1.d732_fa92_6ba4_fp-211)
)
(assert_return
  (invoke "f64.div"
    (f64.const -0x1.e251_d762_163c_cp+825)
    (f64.const 0x1.3ee6_3581_e179_6p+349)
  )
  (f64.const -0x1.8330_077d_90a0_7p+476)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.dcbf_69f1_0006_dp+0)
    (f64.const 0x1.ffff_ffff_ffff_fp+1023)
  )
  (f64.const 0x0.772f_da7c_4001_bp-1022)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.e141_6944_2fbc_ap-1011)
    (f64.const 0x1.5054_51d6_2ff7_dp+12)
  )
  (f64.const 0x0.b727_e85f_38b3_9p-1022)
)
(assert_return
  (invoke "f64.div"
    (f64.const -0x1.d3eb_e726_ec96_4p-144)
    (f64.const -0x1.4a7b_fc0b_8360_8p+880)
  )
  (f64.const 0x0.5a9d_8c50_cbf8_7p-1022)
)
(assert_return
  (invoke "f64.div"
    (f64.const -0x1.6c3d_ef77_0aee_1p-393)
    (f64.const -0x1.8b84_7243_4759_8p+631)
  )
  (f64.const 0x0.3af0_707f_cd0c_7p-1022)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.16ab_da1b_b3cb_3p-856)
    (f64.const 0x1.6c9c_7198_eb1e_6p+166)
  )
  (f64.const 0x0.c3a8_fd67_4164_9p-1022)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.7057_d6ab_553c_ap-1005)
    (f64.const -0x1.2abf_1e98_660e_bp+23)
  )
  (f64.const -0x0.04ee_8d8e_c01c_dp-1022)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.ada9_aap+89) (f32.const 0x1.6988_4cp+42))
  (f32.const 0x1.303e_2ep+47)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.8281_c8p+90) (f32.const -0x1.6288_3cp+106))
  (f32.const -0x1.1716_9cp-16)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.5c6b_e2p+81) (f32.const 0x1.d01d_fep-1))
  (f32.const 0x1.805e_32p+81)
)
(assert_return
  (invoke "f32.div" (f32.const -0x1.bbd2_52p+19) (f32.const -0x1.fba9_5p+33))
  (f32.const 0x1.bf9d_56p-15)
)
(assert_return
  (invoke "f32.div" (f32.const -0x1.0f41_d6p-42) (f32.const -0x1.3f2d_bep+56))
  (f32.const 0x1.b320_d8p-99)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.b234_8a1c_8189_9p+61)
    (f64.const -0x1.4a58_aad9_03dd_3p-861)
  )
  (f64.const -0x1.507c_1e2a_41b3_5p+922)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.23fa_5137_a918_ap-130)
    (f64.const -0x1.7268_db19_5126_3p-521)
  )
  (f64.const -0x1.9396_5e0d_896b_ep+390)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.dcb3_915d_82de_ep+669)
    (f64.const 0x1.50ca_aa1d_c6b1_9p+638)
  )
  (f64.const 0x1.6a58_ec81_4b09_dp+31)
)
(assert_return
  (invoke "f64.div"
    (f64.const -0x1.046e_378c_0cc4_6p+182)
    (f64.const 0x1.ac92_5009_a922_bp+773)
  )
  (f64.const -0x1.3720_aa94_dab1_8p-592)
)
(assert_return
  (invoke "f64.div"
    (f64.const -0x1.8945_fd69_d8e1_1p-871)
    (f64.const -0x1.0a37_870a_f809_ap-646)
  )
  (f64.const 0x1.7a2e_286c_6238_2p-225)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.8200_2af0_ea1f_3p-57)
    (f64.const 0x1.d0a9_b0c2_fa33_9p+0)
  )
  (f64.const 0x1.a952_fbd1_fc17_cp-58)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.1e12_b515_db47_1p-102)
    (f64.const -0x1.41fc_3c94_fba5p-42)
  )
  (f64.const -0x1.c6e5_0ccc_b7cb_6p-61)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.aba5_adcd_6f58_3p-41)
    (f64.const 0x1.17df_ac63_9ce0_fp-112)
  )
  (f64.const 0x1.872b_0a00_8c32_6p+71)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.cf82_510d_0ae6_bp+89)
    (f64.const 0x1.0207_d864_9805_3p+97)
  )
  (f64.const 0x1.cbdc_804e_2cf1_4p-8)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.4c82_cbb5_08e2_1p-11)
    (f64.const -0x1.6b57_208c_2d5d_5p+52)
  )
  (f64.const -0x1.d48e_8b36_9129_ap-64)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1p-126) (f32.const 0x1.ffff_fcp-127))
  (f32.const 0x1.0000_02p+0)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.ffff_fcp-127) (f32.const 0x1p-126))
  (f32.const 0x1.ffff_fcp-1)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1p-1022)
    (f64.const 0x0.ffff_ffff_ffff_fp-1022)
  )
  (f64.const 0x1.0000_0000_0000_1p+0)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x0.ffff_ffff_ffff_fp-1022)
    (f64.const 0x1p-1022)
  )
  (f64.const 0x1.ffff_ffff_ffff_ep-1)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.ffff_fep-23) (f32.const 0x1.ffff_fep+127))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1p-22) (f32.const 0x1.ffff_fep+127))
  (f32.const 0x1p-149)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.ffff_ffff_ffff_fp-52)
    (f64.const 0x1.ffff_ffff_ffff_fp+1023)
  )
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.div" (f64.const 0x1p-51) (f64.const 0x1.ffff_ffff_ffff_fp+1023))
  (f64.const 0x0.0000_0000_0000_1p-1022)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1p+0) (f32.const 0x1p-128))
  (f32.const inf)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1p+0) (f32.const 0x1.0000_08p-128))
  (f32.const 0x1.ffff_fp+127)
)
(assert_return
  (invoke "f64.div" (f64.const 0x1p+0) (f64.const 0x0.4p-1022))
  (f64.const inf)
)
(assert_return
  (invoke "f64.div" (f64.const 0x1p+0) (f64.const 0x0.4000_0000_0000_1p-1022))
  (f64.const 0x1.ffff_ffff_ffff_8p+1023)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1p+0) (f32.const 0x1.0000_02p+126))
  (f32.const 0x1.ffff_fcp-127)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1p+0) (f32.const 0x1p+126))
  (f32.const 0x1p-126)
)
(assert_return
  (invoke "f64.div" (f64.const 0x1p+0) (f64.const 0x1.0000_0000_0000_1p+1022))
  (f64.const 0x0.ffff_ffff_ffff_fp-1022)
)
(assert_return
  (invoke "f64.div" (f64.const 0x1p+0) (f64.const 0x1p+1022))
  (f64.const 0x1p-1022)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1p+0) (f32.const 0x1.8p+1))
  (f32.const 0x1.5555_56p-2)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.8p+1) (f32.const 0x1.2p+3))
  (f32.const 0x1.5555_56p-2)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.2p+3) (f32.const 0x1.bp+4))
  (f32.const 0x1.5555_56p-2)
)
(assert_return
  (invoke "f64.div" (f64.const 0x1p+0) (f64.const 0x1.8p+1))
  (f64.const 0x1.5555_5555_5555_5p-2)
)
(assert_return
  (invoke "f64.div" (f64.const 0x1.8p+1) (f64.const 0x1.2p+3))
  (f64.const 0x1.5555_5555_5555_5p-2)
)
(assert_return
  (invoke "f64.div" (f64.const 0x1.2p+3) (f64.const 0x1.bp+4))
  (f64.const 0x1.5555_5555_5555_5p-2)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.0000_02p+0) (f32.const 0x1.ffff_fep-1))
  (f32.const 0x1.0000_04p+0)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1.ffff_fep-1) (f32.const 0x1.0000_02p+0))
  (f32.const 0x1.ffff_fap-1)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1p+0) (f32.const 0x1.ffff_fep-1))
  (f32.const 0x1.0000_02p+0)
)
(assert_return
  (invoke "f32.div" (f32.const 0x1p+0) (f32.const 0x1.0000_02p+0))
  (f32.const 0x1.ffff_fcp-1)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.0000_0000_0000_1p+0)
    (f64.const 0x1.ffff_ffff_ffff_fp-1)
  )
  (f64.const 0x1.0000_0000_0000_2p+0)
)
(assert_return
  (invoke "f64.div"
    (f64.const 0x1.ffff_ffff_ffff_fp-1)
    (f64.const 0x1.0000_0000_0000_1p+0)
  )
  (f64.const 0x1.ffff_ffff_ffff_dp-1)
)
(assert_return
  (invoke "f64.div" (f64.const 0x1p+0) (f64.const 0x1.ffff_ffff_ffff_fp-1))
  (f64.const 0x1.0000_0000_0000_1p+0)
)
(assert_return
  (invoke "f64.div" (f64.const 0x1p+0) (f64.const 0x1.0000_0000_0000_1p+0))
  (f64.const 0x1.ffff_ffff_ffff_ep-1)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.56p+7))
  (f32.const 0x1.a274_4cp+3)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.594d_fcp-23))
  (f32.const 0x1.a478_9cp-12)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.56p+7))
  (f64.const 0x1.a274_4ce9_674f_5p+3)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.594d_fc70_aa10_5p-23))
  (f64.const 0x1.a478_9c0e_37f9_9p-12)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.0263_fcc9_4f25_9p-164))
  (f64.const 0x1.0131_485d_e579_fp-82)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.352d_fa27_8c43_dp+338))
  (f64.const 0x1.1956_07da_c541_7p+169)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.b15d_aa23_924f_ap+402))
  (f64.const 0x1.4d14_3db5_6149_3p+201)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.518c_8e68_cb75_3p-37))
  (f64.const 0x1.9fb8_ef1a_d5bf_dp-19)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.86d8_b651_8078_ep-370))
  (f64.const 0x1.3c51_42a4_8fca_dp-185)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.ffff_ffff_ffff_fp-1))
  (f64.const 0x1.ffff_ffff_ffff_fp-1)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.0980_64p-3))
  (f32.const 0x1.70b2_3p-2)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.d9be_fp+100))
  (f32.const 0x1.5c40_52p+50)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.42b5_b6p-4))
  (f32.const 0x1.1f6d_0ep-2)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.3684_dp-71))
  (f32.const 0x1.8eba_e2p-36)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.d8bc_4ep-11))
  (f32.const 0x1.ebf9_eap-6)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.5c39_f220_d570_4p-924))
  (f64.const 0x1.2a92_bc24_ceae_9p-462)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.5352_1a63_5745_cp+727))
  (f64.const 0x1.a0cf_dc4e_f8ff_1p+363)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.dfd5_bbc9_f467_8p+385))
  (f64.const 0x1.efa8_1711_7c94_cp+192)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.33f9_6408_11cd_4p+105))
  (f64.const 0x1.8d17_c924_3baa_3p+52)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.6c0e_f026_7ff4_5p+999))
  (f64.const 0x1.afbc_fae3_f2b4p+499)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.26a6_2ep+27))
  (f32.const 0x1.8468_5p+13)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.1660_02p-113))
  (f32.const 0x1.7987_62p-57)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.3dfb_5p-15))
  (f32.const 0x1.937e_38p-8)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.30eb_2cp-120))
  (f32.const 0x1.1764_06p-60)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.cb70_5cp-123))
  (f32.const 0x1.e502_0ap-62)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.edae_8aea_0543p+695))
  (f64.const 0x1.f6c1_ea4f_c8dd_2p+347)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.f7ee_4bda_5c9c_3p-763))
  (f64.const 0x1.fbf3_0bda_f11c_5p-382)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.a48f_3482_66ad_1p-30))
  (f64.const 0x1.481e_e754_0baf_7p-15)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.feb5_a1ce_3ed9_cp-242))
  (f64.const 0x1.6995_060c_20d4_6p-121)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.957d_9796_e383_4p+930))
  (f64.const 0x1.4230_5213_157b_ap+465)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.6578_7cp+118))
  (f32.const 0x1.2e82_a4p+59)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.7360_44p+15))
  (f32.const 0x1.b40e_4p+7)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.a00e_dp-1))
  (f32.const 0x1.cd8a_ecp-1)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.7a4c_8p-87))
  (f32.const 0x1.b819_e4p-44)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.5d24_d4p-94))
  (f32.const 0x1.2af7_5ep-47)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.a008_948e_ad27_4p+738))
  (f64.const 0x1.4659_b37c_39b1_9p+369)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.70f6_199e_d21f_5p-381))
  (f64.const 0x1.b2a2_bddf_3300_dp-191)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.35c1_d49f_2a35_2p+965))
  (f64.const 0x1.8e3d_9f01_a971_6p+482)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.3fbd_cfb2_b2a1_5p-45))
  (f64.const 0x1.949b_a4fe_ca42_ap-23)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.c201_b947_5714_5p-492))
  (f64.const 0x1.5369_ee6b_f296_7p-246)
)
(assert_return
  (invoke "f64.sqrt" (f64.const -0x1.360e_8d00_32adp-963))
  (f64.const nan:canonical)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.d9a6_f5ee_f050_3p+103))
  (f64.const 0x1.ec73_f56c_166f_6p+51)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.aa05_1a5c_4ec2_7p-760))
  (f64.const 0x1.4a3e_771f_f514_9p-380)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.e552_2a74_1bab_ep-276))
  (f64.const 0x1.607a_e2b6_feb7_dp-138)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.4832_badc_0c06_1p+567))
  (f64.const 0x1.99ec_7934_139b_2p+283)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.0000_02p+0))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.0000_04p+0))
  (f32.const 0x1.0000_02p+0)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.0000_0000_0000_1p+0))
  (f64.const 0x1p+0)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.0000_0000_0000_2p+0))
  (f64.const 0x1.0000_0000_0000_1p+0)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.ffff_fcp-1))
  (f32.const 0x1.ffff_fep-1)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.ffff_fap-1))
  (f32.const 0x1.ffff_fcp-1)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.ffff_ffff_ffff_ep-1))
  (f64.const 0x1.ffff_ffff_ffff_fp-1)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.ffff_ffff_ffff_dp-1))
  (f64.const 0x1.ffff_ffff_ffff_ep-1)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.ffff_fep-3))
  (f32.const 0x1.ffff_fep-2)
)
(assert_return (invoke "f32.sqrt" (f32.const 0x1p-2)) (f32.const 0x1p-1))
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.0000_02p-2))
  (f32.const 0x1p-1)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.0000_04p-2))
  (f32.const 0x1.0000_02p-1)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.ffff_ffff_ffff_fp-3))
  (f64.const 0x1.ffff_ffff_ffff_fp-2)
)
(assert_return (invoke "f64.sqrt" (f64.const 0x1p-2)) (f64.const 0x1p-1))
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.0000_0000_0000_1p-2))
  (f64.const 0x1p-1)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.0000_0000_0000_2p-2))
  (f64.const 0x1.0000_0000_0000_1p-1)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.fb41_d4p+37))
  (f32.const 0x1.fd9f_8p+18)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.fb41_d442_eeb1_bp+37))
  (f64.const 0x1.fd9f_808a_0b68_dp+18)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.3c61_b2p+33))
  (f32.const 0x1.927ap+16)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.56bd_4ep+51))
  (f32.const 0x1.a2e8_0cp+25)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.65f0_2cp+44))
  (f32.const 0x1.2eb5_44p+22)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.2658_0cp+30))
  (f32.const 0x1.1280_d6p+15)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.3c61_b112p+33))
  (f64.const 0x1.927ap+16)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.56bd_4e65_c854_8p+51))
  (f64.const 0x1.a2e8_0dp+25)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.65f0_2cc9_3a1p+44))
  (f64.const 0x1.2eb5_44p+22)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.2658_0b4cp+30))
  (f64.const 0x1.1280_d62b_818c_fp+15)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.0817_fcp-1))
  (f32.const 0x1.6fb7_9ep-1)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.017b_98p-1))
  (f32.const 0x1.6b15_eep-1)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.0817_fcp-1))
  (f64.const 0x1.6fb7_9d0d_faff_ap-1)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.017b_98p-1))
  (f64.const 0x1.6b15_ed00_71b9_5p-1)
)
(assert_return (invoke "f32.sqrt" (f32.const 0x1.2p+3)) (f32.const 0x1.8p+1))
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.12p+7))
  (f32.const 0x1.768c_e6p+3)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.c615_dep+0))
  (f32.const 0x1.54f2_dp+0)
)
(assert_return
  (invoke "f32.sqrt" (f32.const 0x1.f4p+9))
  (f32.const 0x1.f9f6_e4p+4)
)
(assert_return (invoke "f64.sqrt" (f64.const 0x1.2p+3)) (f64.const 0x1.8p+1))
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.12p+7))
  (f64.const 0x1.768c_e6d3_c11ep+3)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.c615_df07_a57d_3p+0))
  (f64.const 0x1.54f2_d015_acf0_9p+0)
)
(assert_return
  (invoke "f64.sqrt" (f64.const 0x1.f4p+9))
  (f64.const 0x1.f9f6_e499_0f22_7p+4)
)
(assert_return (invoke "f32.abs" (f32.const nan:0xf1e2)) (f32.const nan:0xf1e2))
(assert_return
  (invoke "f32.abs" (f32.const -nan:0xf1e2))
  (f32.const nan:0xf1e2)
)
(assert_return
  (invoke "f64.abs" (f64.const nan:0xf1e2_7a6b))
  (f64.const nan:0xf1e2_7a6b)
)
(assert_return
  (invoke "f64.abs" (f64.const -nan:0xf1e2_7a6b))
  (f64.const nan:0xf1e2_7a6b)
)
(assert_return
  (invoke "f32.neg" (f32.const nan:0xf1e2))
  (f32.const -nan:0xf1e2)
)
(assert_return
  (invoke "f32.neg" (f32.const -nan:0xf1e2))
  (f32.const nan:0xf1e2)
)
(assert_return
  (invoke "f64.neg" (f64.const nan:0xf1e2_7a6b))
  (f64.const -nan:0xf1e2_7a6b)
)
(assert_return
  (invoke "f64.neg" (f64.const -nan:0xf1e2_7a6b))
  (f64.const nan:0xf1e2_7a6b)
)
(assert_return
  (invoke "f32.copysign" (f32.const nan:0xf1e2) (f32.const nan:0x40_0000))
  (f32.const nan:0xf1e2)
)
(assert_return
  (invoke "f32.copysign" (f32.const nan:0xf1e2) (f32.const -nan:0x40_0000))
  (f32.const -nan:0xf1e2)
)
(assert_return
  (invoke "f32.copysign" (f32.const -nan:0xf1e2) (f32.const nan:0x40_0000))
  (f32.const nan:0xf1e2)
)
(assert_return
  (invoke "f32.copysign" (f32.const -nan:0xf1e2) (f32.const -nan:0x40_0000))
  (f32.const -nan:0xf1e2)
)
(assert_return
  (invoke "f64.copysign"
    (f64.const nan:0xf1e2_7a6b)
    (f64.const nan:0x8_0000_0000_0000)
  )
  (f64.const nan:0xf1e2_7a6b)
)
(assert_return
  (invoke "f64.copysign"
    (f64.const nan:0xf1e2_7a6b)
    (f64.const -nan:0x8_0000_0000_0000)
  )
  (f64.const -nan:0xf1e2_7a6b)
)
(assert_return
  (invoke "f64.copysign"
    (f64.const -nan:0xf1e2_7a6b)
    (f64.const nan:0x8_0000_0000_0000)
  )
  (f64.const nan:0xf1e2_7a6b)
)
(assert_return
  (invoke "f64.copysign"
    (f64.const -nan:0xf1e2_7a6b)
    (f64.const -nan:0x8_0000_0000_0000)
  )
  (f64.const -nan:0xf1e2_7a6b)
)
(assert_return
  (invoke "f32.ceil" (f32.const 0x1.ffff_fep-1))
  (f32.const 0x1p+0)
)
(assert_return
  (invoke "f32.ceil" (f32.const 0x1.0000_02p+0))
  (f32.const 0x1p+1)
)
(assert_return
  (invoke "f64.ceil" (f64.const 0x1.ffff_ffff_ffff_fp-1))
  (f64.const 0x1p+0)
)
(assert_return
  (invoke "f64.ceil" (f64.const 0x1.0000_0000_0000_1p+0))
  (f64.const 0x1p+1)
)
(assert_return
  (invoke "f32.ceil" (f32.const 0x1.ffff_fep+22))
  (f32.const 0x1p+23)
)
(assert_return
  (invoke "f32.ceil" (f32.const -0x1.ffff_fep+22))
  (f32.const -0x1.ffff_fcp+22)
)
(assert_return
  (invoke "f64.ceil" (f64.const 0x1.ffff_ffff_ffff_fp+51))
  (f64.const 0x1p+52)
)
(assert_return
  (invoke "f64.ceil" (f64.const -0x1.ffff_ffff_ffff_fp+51))
  (f64.const -0x1.ffff_ffff_ffff_ep+51)
)
(assert_return
  (invoke "f32.ceil" (f32.const 0x1.ffff_fep+23))
  (f32.const 0x1.ffff_fep+23)
)
(assert_return
  (invoke "f32.ceil" (f32.const -0x1.ffff_fep+23))
  (f32.const -0x1.ffff_fep+23)
)
(assert_return
  (invoke "f64.ceil" (f64.const 0x1.ffff_ffff_ffff_fp+52))
  (f64.const 0x1.ffff_ffff_ffff_fp+52)
)
(assert_return
  (invoke "f64.ceil" (f64.const -0x1.ffff_ffff_ffff_fp+52))
  (f64.const -0x1.ffff_ffff_ffff_fp+52)
)
(assert_return
  (invoke "f32.floor" (f32.const -0x1.ffff_fep-1))
  (f32.const -0x1p+0)
)
(assert_return
  (invoke "f32.floor" (f32.const -0x1.0000_02p+0))
  (f32.const -0x1p+1)
)
(assert_return
  (invoke "f64.floor" (f64.const -0x1.ffff_ffff_ffff_fp-1))
  (f64.const -0x1p+0)
)
(assert_return
  (invoke "f64.floor" (f64.const -0x1.0000_0000_0000_1p+0))
  (f64.const -0x1p+1)
)
(assert_return
  (invoke "f32.floor" (f32.const -0x1.ffff_fep+22))
  (f32.const -0x1p+23)
)
(assert_return
  (invoke "f32.floor" (f32.const 0x1.ffff_fep+22))
  (f32.const 0x1.ffff_fcp+22)
)
(assert_return
  (invoke "f64.floor" (f64.const -0x1.ffff_ffff_ffff_fp+51))
  (f64.const -0x1p+52)
)
(assert_return
  (invoke "f64.floor" (f64.const 0x1.ffff_ffff_ffff_fp+51))
  (f64.const 0x1.ffff_ffff_ffff_ep+51)
)
(assert_return
  (invoke "f32.floor" (f32.const 0x1.5a1fp+16))
  (f32.const 0x1.5a1fp+16)
)
(assert_return
  (invoke "f64.floor" (f64.const 0x1.5a1fp+16))
  (f64.const 0x1.5a1fp+16)
)
(assert_return
  (invoke "f32.trunc" (f32.const -0x1.ffff_fep+22))
  (f32.const -0x1.ffff_fcp+22)
)
(assert_return
  (invoke "f32.trunc" (f32.const 0x1.ffff_fep+22))
  (f32.const 0x1.ffff_fcp+22)
)
(assert_return
  (invoke "f64.trunc" (f64.const -0x1.ffff_ffff_ffff_fp+51))
  (f64.const -0x1.ffff_ffff_ffff_ep+51)
)
(assert_return
  (invoke "f64.trunc" (f64.const 0x1.ffff_ffff_ffff_fp+51))
  (f64.const 0x1.ffff_ffff_ffff_ep+51)
)
(assert_return
  (invoke "f32.nearest" (f32.const 0x1.0000_02p+23))
  (f32.const 0x1.0000_02p+23)
)
(assert_return
  (invoke "f32.nearest" (f32.const 0x1.0000_04p+23))
  (f32.const 0x1.0000_04p+23)
)
(assert_return
  (invoke "f32.nearest" (f32.const 0x1.ffff_fep-2))
  (f32.const 0x0p+0)
)
(assert_return
  (invoke "f32.nearest" (f32.const 0x1.ffff_fep+47))
  (f32.const 0x1.ffff_fep+47)
)
(assert_return
  (invoke "f64.nearest" (f64.const 0x1.0000_0000_0000_1p+52))
  (f64.const 0x1.0000_0000_0000_1p+52)
)
(assert_return
  (invoke "f64.nearest" (f64.const 0x1.0000_0000_0000_2p+52))
  (f64.const 0x1.0000_0000_0000_2p+52)
)
(assert_return
  (invoke "f64.nearest" (f64.const 0x1.ffff_ffff_ffff_fp-2))
  (f64.const 0x0p+0)
)
(assert_return
  (invoke "f64.nearest" (f64.const 0x1.ffff_ffff_ffff_fp+105))
  (f64.const 0x1.ffff_ffff_ffff_fp+105)
)
(assert_return (invoke "f32.nearest" (f32.const 0x1.2p+2)) (f32.const 0x1p+2))
(assert_return (invoke "f32.nearest" (f32.const -0x1.2p+2)) (f32.const -0x1p+2))
(assert_return (invoke "f32.nearest" (f32.const -0x1.cp+1)) (f32.const -0x1p+2))
(assert_return (invoke "f64.nearest" (f64.const 0x1.2p+2)) (f64.const 0x1p+2))
(assert_return (invoke "f64.nearest" (f64.const -0x1.2p+2)) (f64.const -0x1p+2))
(assert_return (invoke "f64.nearest" (f64.const -0x1.cp+1)) (f64.const -0x1p+2))
(assert_return
  (invoke "f32.nearest" (f32.const -0x1.ffff_fep+22))
  (f32.const -0x1p+23)
)
(assert_return
  (invoke "f32.nearest" (f32.const 0x1.ffff_fep+22))
  (f32.const 0x1p+23)
)
(assert_return
  (invoke "f64.nearest" (f64.const -0x1.ffff_ffff_ffff_fp+51))
  (f64.const -0x1p+52)
)
(assert_return
  (invoke "f64.nearest" (f64.const 0x1.ffff_ffff_ffff_fp+51))
  (f64.const 0x1p+52)
)
