(module definition binary
  "\00\61\73\6d\01\00\00\00\01\93\80\80\80\00\03\60"
  "\01\7b\01\7b\60\02\7b\7b\01\7b\60\03\7b\7b\7b\01"
  "\7b\03\87\80\80\80\00\06\00\01\01\01\02\01\07\ad"
  "\80\80\80\00\06\03\6e\6f\74\00\00\03\61\6e\64\00"
  "\01\02\6f\72\00\02\03\78\6f\72\00\03\09\62\69\74"
  "\73\65\6c\65\63\74\00\04\06\61\6e\64\6e\6f\74\00"
  "\05\0a\cf\80\80\80\00\06\86\80\80\80\00\00\20\00"
  "\fd\4d\0b\88\80\80\80\00\00\20\00\20\01\fd\4e\0b"
  "\88\80\80\80\00\00\20\00\20\01\fd\50\0b\88\80\80"
  "\80\00\00\20\00\20\01\fd\51\0b\8a\80\80\80\00\00"
  "\20\00\20\01\20\02\fd\52\0b\88\80\80\80\00\00\20"
  "\00\20\01\fd\4f\0b"
)
(module instance)
(assert_return
  (invoke "not" (v128.const i32x4 0x0 0x0 0x0 0x0))
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "not"
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
  )
  (v128.const i32x4 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "not" (v128.const i32x4 0xffff_ffff 0x0 0xffff_ffff 0x0))
  (v128.const i32x4 0x0 0xffff_ffff 0x0 0xffff_ffff)
)
(assert_return
  (invoke "not" (v128.const i32x4 0x0 0xffff_ffff 0x0 0xffff_ffff))
  (v128.const i32x4 0xffff_ffff 0x0 0xffff_ffff 0x0)
)
(assert_return
  (invoke "not"
    (v128.const i32x4 0x5555_5555 0x5555_5555 0x5555_5555 0x5555_5555)
  )
  (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
)
(assert_return
  (invoke "not"
    (v128.const i32x4 0xcccc_cccc 0xcccc_cccc 0xcccc_cccc 0xcccc_cccc)
  )
  (v128.const i32x4 0x3333_3333 0x3333_3333 0x3333_3333 0x3333_3333)
)
(assert_return
  (invoke "not"
    (v128.const i32x4 0x4996_02d2 0x4996_02d2 0x4996_02d2 0x4996_02d2)
  )
  (v128.const i32x4 0xb669_fd2d 0xb669_fd2d 0xb669_fd2d 0xb669_fd2d)
)
(assert_return
  (invoke "not"
    (v128.const i32x4 0x1234_5678 0x1234_5678 0x1234_5678 0x1234_5678)
  )
  (v128.const i32x4 0xedcb_a987 0xedcb_a987 0xedcb_a987 0xedcb_a987)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0x0 0x0 0xffff_ffff 0xffff_ffff)
    (v128.const i32x4 0x0 0xffff_ffff 0x0 0xffff_ffff)
  )
  (v128.const i32x4 0x0 0x0 0x0 0xffff_ffff)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0x0 0x0 0x0 0x0)
    (v128.const i32x4 0x0 0x0 0x0 0x0)
  )
  (v128.const i32x4 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0x0 0x0 0x0 0x0)
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
  )
  (v128.const i32x4 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0x0 0x0 0x0 0x0)
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
  )
  (v128.const i32x4 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0x1 0x1 0x1 0x1)
    (v128.const i32x4 0x1 0x1 0x1 0x1)
  )
  (v128.const i32x4 0x1 0x1 0x1 0x1)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0xff 0xff 0xff 0xff)
    (v128.const i32x4 0x55 0x55 0x55 0x55)
  )
  (v128.const i32x4 0x55 0x55 0x55 0x55)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0xff 0xff 0xff 0xff)
    (v128.const i32x4 0x80 0x80 0x80 0x80)
  )
  (v128.const i32x4 0x80 0x80 0x80 0x80)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
    (v128.const i32x4 0xa 0x80 0x5 0xa5)
  )
  (v128.const i32x4 0xa 0x80 0x0 0xa0)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
    (v128.const i32x4 0x5555_5555 0x5555_5555 0x5555_5555 0x5555_5555)
  )
  (v128.const i32x4 0x5555_5555 0x5555_5555 0x5555_5555 0x5555_5555)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
    (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
  )
  (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
    (v128.const i32x4 0x0 0x0 0x0 0x0)
  )
  (v128.const i32x4 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0x5555_5555 0x5555_5555 0x5555_5555 0x5555_5555)
    (v128.const i32x4 0x5555 0xffff 0x55ff 0x5fff)
  )
  (v128.const i32x4 0x5555 0x5555 0x5555 0x5555)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0x4996_02d2 0x4996_02d2 0x4996_02d2 0x4996_02d2)
    (v128.const i32x4 0x4996_02d2 0x4996_02d2 0x4996_02d2 0x4996_02d2)
  )
  (v128.const i32x4 0x4996_02d2 0x4996_02d2 0x4996_02d2 0x4996_02d2)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0x1234_5678 0x1234_5678 0x1234_5678 0x1234_5678)
    (v128.const i32x4 0x90ab_cdef 0x90ab_cdef 0x90ab_cdef 0x90ab_cdef)
  )
  (v128.const i32x4 0x1020_4468 0x1020_4468 0x1020_4468 0x1020_4468)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0x0 0x0 0xffff_ffff 0xffff_ffff)
    (v128.const i32x4 0x0 0xffff_ffff 0x0 0xffff_ffff)
  )
  (v128.const i32x4 0x0 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0x0 0x0 0x0 0x0)
    (v128.const i32x4 0x0 0x0 0x0 0x0)
  )
  (v128.const i32x4 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0x0 0x0 0x0 0x0)
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
  )
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0x0 0x0 0x0 0x0)
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
  )
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0x1 0x1 0x1 0x1)
    (v128.const i32x4 0x1 0x1 0x1 0x1)
  )
  (v128.const i32x4 0x1 0x1 0x1 0x1)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0xff 0xff 0xff 0xff)
    (v128.const i32x4 0x55 0x55 0x55 0x55)
  )
  (v128.const i32x4 0xff 0xff 0xff 0xff)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0xff 0xff 0xff 0xff)
    (v128.const i32x4 0x80 0x80 0x80 0x80)
  )
  (v128.const i32x4 0xff 0xff 0xff 0xff)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
    (v128.const i32x4 0xa 0x80 0x5 0xa5)
  )
  (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaf 0xaaaa_aaaf)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
    (v128.const i32x4 0x5555_5555 0x5555_5555 0x5555_5555 0x5555_5555)
  )
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
    (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
  )
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
    (v128.const i32x4 0x0 0x0 0x0 0x0)
  )
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0x5555_5555 0x5555_5555 0x5555_5555 0x5555_5555)
    (v128.const i32x4 0x5555 0xffff 0x55ff 0x5fff)
  )
  (v128.const i32x4 0x5555_5555 0x5555_ffff 0x5555_55ff 0x5555_5fff)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0x4996_02d2 0x4996_02d2 0x4996_02d2 0x4996_02d2)
    (v128.const i32x4 0x4996_02d2 0x4996_02d2 0x4996_02d2 0x4996_02d2)
  )
  (v128.const i32x4 0x4996_02d2 0x4996_02d2 0x4996_02d2 0x4996_02d2)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0x1234_5678 0x1234_5678 0x1234_5678 0x1234_5678)
    (v128.const i32x4 0x90ab_cdef 0x90ab_cdef 0x90ab_cdef 0x90ab_cdef)
  )
  (v128.const i32x4 0x92bf_dfff 0x92bf_dfff 0x92bf_dfff 0x92bf_dfff)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0x0 0x0 0xffff_ffff 0xffff_ffff)
    (v128.const i32x4 0x0 0xffff_ffff 0x0 0xffff_ffff)
  )
  (v128.const i32x4 0x0 0xffff_ffff 0xffff_ffff 0x0)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0x0 0x0 0x0 0x0)
    (v128.const i32x4 0x0 0x0 0x0 0x0)
  )
  (v128.const i32x4 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0x0 0x0 0x0 0x0)
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
  )
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0x0 0x0 0x0 0x0)
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
  )
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0x1 0x1 0x1 0x1)
    (v128.const i32x4 0x1 0x1 0x1 0x1)
  )
  (v128.const i32x4 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0xff 0xff 0xff 0xff)
    (v128.const i32x4 0x55 0x55 0x55 0x55)
  )
  (v128.const i32x4 0xaa 0xaa 0xaa 0xaa)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0xff 0xff 0xff 0xff)
    (v128.const i32x4 0x80 0x80 0x80 0x80)
  )
  (v128.const i32x4 0x7f 0x7f 0x7f 0x7f)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
    (v128.const i32x4 0xa 0x80 0x5 0xa5)
  )
  (v128.const i32x4 0xaaaa_aaa0 0xaaaa_aa2a 0xaaaa_aaaf 0xaaaa_aa0f)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
    (v128.const i32x4 0x5555_5555 0x5555_5555 0x5555_5555 0x5555_5555)
  )
  (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
    (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
  )
  (v128.const i32x4 0x5555_5555 0x5555_5555 0x5555_5555 0x5555_5555)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
    (v128.const i32x4 0x0 0x0 0x0 0x0)
  )
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0x5555_5555 0x5555_5555 0x5555_5555 0x5555_5555)
    (v128.const i32x4 0x5555 0xffff 0x55ff 0x5fff)
  )
  (v128.const i32x4 0x5555_0000 0x5555_aaaa 0x5555_00aa 0x5555_0aaa)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0x4996_02d2 0x4996_02d2 0x4996_02d2 0x4996_02d2)
    (v128.const i32x4 0x4996_02d2 0x4996_02d2 0x4996_02d2 0x4996_02d2)
  )
  (v128.const i32x4 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0x1234_5678 0x1234_5678 0x1234_5678 0x1234_5678)
    (v128.const i32x4 0x90ab_cdef 0x90ab_cdef 0x90ab_cdef 0x90ab_cdef)
  )
  (v128.const i32x4 0x829f_9b97 0x829f_9b97 0x829f_9b97 0x829f_9b97)
)
(assert_return
  (invoke "bitselect"
    (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
    (v128.const i32x4 0xbbbb_bbbb 0xbbbb_bbbb 0xbbbb_bbbb 0xbbbb_bbbb)
    (v128.const i32x4 0x11_2345 0xf00f_ffff 0x1011_2021 0xbbaa_bbaa)
  )
  (v128.const i32x4 0xbbaa_baba 0xabba_aaaa 0xabaa_bbba 0xaabb_aabb)
)
(assert_return
  (invoke "bitselect"
    (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
    (v128.const i32x4 0xbbbb_bbbb 0xbbbb_bbbb 0xbbbb_bbbb 0xbbbb_bbbb)
    (v128.const i32x4 0x0 0x0 0x0 0x0)
  )
  (v128.const i32x4 0xbbbb_bbbb 0xbbbb_bbbb 0xbbbb_bbbb 0xbbbb_bbbb)
)
(assert_return
  (invoke "bitselect"
    (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
    (v128.const i32x4 0xbbbb_bbbb 0xbbbb_bbbb 0xbbbb_bbbb 0xbbbb_bbbb)
    (v128.const i32x4 0x1111_1111 0x1111_1111 0x1111_1111 0x1111_1111)
  )
  (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
)
(assert_return
  (invoke "bitselect"
    (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
    (v128.const i32x4 0xbbbb_bbbb 0xbbbb_bbbb 0xbbbb_bbbb 0xbbbb_bbbb)
    (v128.const i32x4 0x123_4567 0x89ab_cdef 0xfedc_ba98 0x7654_3210)
  )
  (v128.const i32x4 0xbaba_baba 0xbaba_baba 0xabab_abab 0xabab_abab)
)
(assert_return
  (invoke "bitselect"
    (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
    (v128.const i32x4 0x5555_5555 0x5555_5555 0x5555_5555 0x5555_5555)
    (v128.const i32x4 0x123_4567 0x89ab_cdef 0xfedc_ba98 0x7654_3210)
  )
  (v128.const i32x4 0x5476_1032 0xdcfe_98ba 0xab89_efcd 0x2301_6745)
)
(assert_return
  (invoke "bitselect"
    (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
    (v128.const i32x4 0x5555_5555 0x5555_5555 0x5555_5555 0x5555_5555)
    (v128.const i32x4 0x5555_5555 0xaaaa_aaaa 0x0 0xffff_ffff)
  )
  (v128.const i32x4 0x0 0xffff_ffff 0x5555_5555 0xaaaa_aaaa)
)
(assert_return
  (invoke "bitselect"
    (v128.const i32x4 0x4996_02d2 0x4996_02d2 0x4996_02d2 0x4996_02d2)
    (v128.const i32x4 0xb669_fd2e 0xb669_fd2e 0xb669_fd2e 0xb669_fd2e)
    (v128.const i32x4 0xcdef_cdef 0xcdef_cdef 0xcdef_cdef 0xcdef_cdef)
  )
  (v128.const i32x4 0x7b86_30c2 0x7b86_30c2 0x7b86_30c2 0x7b86_30c2)
)
(assert_return
  (invoke "bitselect"
    (v128.const i32x4 0x1234_5678 0x1234_5678 0x1234_5678 0x1234_5678)
    (v128.const i32x4 0x90ab_cdef 0x90ab_cdef 0x90ab_cdef 0x90ab_cdef)
    (v128.const i32x4 0xcdef_cdef 0xcdef_cdef 0xcdef_cdef 0xcdef_cdef)
  )
  (v128.const i32x4 0x1024_4468 0x1024_4468 0x1024_4468 0x1024_4468)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0x0 0x0 0xffff_ffff 0xffff_ffff)
    (v128.const i32x4 0x0 0xffff_ffff 0x0 0xffff_ffff)
  )
  (v128.const i32x4 0x0 0x0 0xffff_ffff 0x0)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0x0 0x0 0x0 0x0)
    (v128.const i32x4 0x0 0x0 0x0 0x0)
  )
  (v128.const i32x4 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0x0 0x0 0x0 0x0)
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
  )
  (v128.const i32x4 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0x0 0x0 0x0 0x0)
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
  )
  (v128.const i32x4 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0x1 0x1 0x1 0x1)
    (v128.const i32x4 0x1 0x1 0x1 0x1)
  )
  (v128.const i32x4 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0xff 0xff 0xff 0xff)
    (v128.const i32x4 0x55 0x55 0x55 0x55)
  )
  (v128.const i32x4 0xaa 0xaa 0xaa 0xaa)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0xff 0xff 0xff 0xff)
    (v128.const i32x4 0x80 0x80 0x80 0x80)
  )
  (v128.const i32x4 0x7f 0x7f 0x7f 0x7f)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
    (v128.const i32x4 0xa 0x80 0x5 0xa5)
  )
  (v128.const i32x4 0xaaaa_aaa0 0xaaaa_aa2a 0xaaaa_aaaa 0xaaaa_aa0a)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
    (v128.const i32x4 0x5555_5555 0x5555_5555 0x5555_5555 0x5555_5555)
  )
  (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
    (v128.const i32x4 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa 0xaaaa_aaaa)
  )
  (v128.const i32x4 0x5555_5555 0x5555_5555 0x5555_5555 0x5555_5555)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
    (v128.const i32x4 0x0 0x0 0x0 0x0)
  )
  (v128.const i32x4 0xffff_ffff 0xffff_ffff 0xffff_ffff 0xffff_ffff)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0x5555_5555 0x5555_5555 0x5555_5555 0x5555_5555)
    (v128.const i32x4 0x5555 0xffff 0x55ff 0x5fff)
  )
  (v128.const i32x4 0x5555_0000 0x5555_0000 0x5555_0000 0x5555_0000)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0x4996_02d2 0x4996_02d2 0x4996_02d2 0x4996_02d2)
    (v128.const i32x4 0x4996_02d2 0x4996_02d2 0x4996_02d2 0x4996_02d2)
  )
  (v128.const i32x4 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0x1234_5678 0x1234_5678 0x1234_5678 0x1234_5678)
    (v128.const i32x4 0x90ab_cdef 0x90ab_cdef 0x90ab_cdef 0x90ab_cdef)
  )
  (v128.const i32x4 0x214_1210 0x214_1210 0x214_1210 0x214_1210)
)
(assert_return
  (invoke "not"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
  )
  (v128.const f32x4
    0x1.ffff_f8p-128
    0x1.ffff_f8p-128
    0x1.ffff_f8p-128
    0x1.ffff_f8p-128
  )
)
(assert_return
  (invoke "not"
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
  )
  (v128.const f32x4
    -0x1.ffff_f8p-128
    -0x1.ffff_f8p-128
    -0x1.ffff_f8p-128
    -0x1.ffff_f8p-128
  )
)
(assert_return
  (invoke "not"
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
  )
  (v128.const i32x4 0x7f_ffff 0x7f_ffff 0x7f_ffff 0x7f_ffff)
)
(assert_return
  (invoke "not"
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
  )
  (v128.const i32x4 0x807f_ffff 0x807f_ffff 0x807f_ffff 0x807f_ffff)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
  )
  (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
  )
  (v128.const f32x4 nan:0x40_0000 nan:0x40_0000 nan:0x40_0000 nan:0x40_0000)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
  )
  (v128.const f32x4 -inf -inf -inf -inf)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
  )
  (v128.const f32x4 inf inf inf inf)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
  )
  (v128.const f32x4 nan:0x40_0000 nan:0x40_0000 nan:0x40_0000 nan:0x40_0000)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
  )
  (v128.const f32x4 inf inf inf inf)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
  )
  (v128.const f32x4 inf inf inf inf)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
  )
  (v128.const f32x4 -inf -inf -inf -inf)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
  )
  (v128.const f32x4 inf inf inf inf)
)
(assert_return
  (invoke "and"
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
  )
  (v128.const f32x4 inf inf inf inf)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
  )
  (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
  )
  (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
  )
  (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
  )
  (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
  )
  (v128.const f32x4 nan:0x40_0000 nan:0x40_0000 nan:0x40_0000 nan:0x40_0000)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
  )
  (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
  )
  (v128.const f32x4 nan:0x40_0000 nan:0x40_0000 nan:0x40_0000 nan:0x40_0000)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
  )
  (v128.const f32x4 -inf -inf -inf -inf)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
  )
  (v128.const f32x4 -inf -inf -inf -inf)
)
(assert_return
  (invoke "or"
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
  )
  (v128.const f32x4 inf inf inf inf)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
  )
  (v128.const f32x4 0x0p+0 0x0p+0 0x0p+0 0x0p+0)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
  )
  (v128.const f32x4 -0x0p+0 -0x0p+0 -0x0p+0 -0x0p+0)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
  )
  (v128.const i32x4 0x40_0000 0x40_0000 0x40_0000 0x40_0000)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
  )
  (v128.const i32x4 0x8040_0000 0x8040_0000 0x8040_0000 0x8040_0000)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
  )
  (v128.const f32x4 0x0p+0 0x0p+0 0x0p+0 0x0p+0)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
  )
  (v128.const i32x4 0x8040_0000 0x8040_0000 0x8040_0000 0x8040_0000)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
  )
  (v128.const i32x4 0x40_0000 0x40_0000 0x40_0000 0x40_0000)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
  )
  (v128.const f32x4 0x0p+0 0x0p+0 0x0p+0 0x0p+0)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
  )
  (v128.const i32x4 0x8000_0000 0x8000_0000 0x8000_0000 0x8000_0000)
)
(assert_return
  (invoke "xor"
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
  )
  (v128.const f32x4 0x0p+0 0x0p+0 0x0p+0 0x0p+0)
)
(assert_return
  (invoke "bitselect"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6)
  )
  (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
)
(assert_return
  (invoke "bitselect"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
    (v128.const i32x4 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6)
  )
  (v128.const f32x4 nan:0x40_0000 nan:0x40_0000 nan:0x40_0000 nan:0x40_0000)
)
(assert_return
  (invoke "bitselect"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
    (v128.const i32x4 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6)
  )
  (v128.const f32x4 -inf -inf -inf -inf)
)
(assert_return
  (invoke "bitselect"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
    (v128.const i32x4 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6)
  )
  (v128.const f32x4 inf inf inf inf)
)
(assert_return
  (invoke "bitselect"
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
    (v128.const i32x4 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6)
  )
  (v128.const f32x4 nan:0x40_0000 nan:0x40_0000 nan:0x40_0000 nan:0x40_0000)
)
(assert_return
  (invoke "bitselect"
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
    (v128.const i32x4 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6)
  )
  (v128.const f32x4 -inf -inf -inf -inf)
)
(assert_return
  (invoke "bitselect"
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
    (v128.const i32x4 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6)
  )
  (v128.const f32x4 inf inf inf inf)
)
(assert_return
  (invoke "bitselect"
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
    (v128.const i32x4 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6)
  )
  (v128.const f32x4 -inf -inf -inf -inf)
)
(assert_return
  (invoke "bitselect"
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
    (v128.const i32x4 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6)
  )
  (v128.const f32x4 inf inf inf inf)
)
(assert_return
  (invoke "bitselect"
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
    (v128.const i32x4 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6 0x4f25_a5a6)
  )
  (v128.const f32x4 inf inf inf inf)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
  )
  (v128.const i32x4 0x0 0x0 0x0 0x0)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
  )
  (v128.const f32x4 -0x0p+0 -0x0p+0 -0x0p+0 -0x0p+0)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
  )
  (v128.const i32x4 0x40_0000 0x40_0000 0x40_0000 0x40_0000)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0xffc0_0000 0xffc0_0000 0xffc0_0000 0xffc0_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
  )
  (v128.const i32x4 0x8040_0000 0x8040_0000 0x8040_0000 0x8040_0000)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
  )
  (v128.const f32x4 0x0p+0 0x0p+0 0x0p+0 0x0p+0)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
  )
  (v128.const i32x4 0x40_0000 0x40_0000 0x40_0000 0x40_0000)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000 0x7fc0_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
  )
  (v128.const i32x4 0x40_0000 0x40_0000 0x40_0000 0x40_0000)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
  )
  (v128.const f32x4 0x0p+0 0x0p+0 0x0p+0 0x0p+0)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0xff80_0000 0xff80_0000 0xff80_0000 0xff80_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
  )
  (v128.const i32x4 0x8000_0000 0x8000_0000 0x8000_0000 0x8000_0000)
)
(assert_return
  (invoke "andnot"
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
    (v128.const i32x4 0x7f80_0000 0x7f80_0000 0x7f80_0000 0x7f80_0000)
  )
  (v128.const i32x4 0x0 0x0 0x0 0x0)
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8c\80\80\80"
    "\00\01\86\80\80\80\00\00\41\00\fd\4d\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\9e\80\80\80"
    "\00\01\98\80\80\80\00\00\41\00\fd\0c\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\fd\4e\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\9e\80\80\80"
    "\00\01\98\80\80\80\00\00\fd\0c\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\41\00\fd\4e\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8e\80\80\80"
    "\00\01\88\80\80\80\00\00\41\00\41\00\fd\4e\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\9e\80\80\80"
    "\00\01\98\80\80\80\00\00\41\00\fd\0c\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\fd\50\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\9e\80\80\80"
    "\00\01\98\80\80\80\00\00\fd\0c\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\41\00\fd\50\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8e\80\80\80"
    "\00\01\88\80\80\80\00\00\41\00\41\00\fd\50\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\9e\80\80\80"
    "\00\01\98\80\80\80\00\00\41\00\fd\0c\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\fd\51\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\9e\80\80\80"
    "\00\01\98\80\80\80\00\00\fd\0c\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\41\00\fd\51\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8e\80\80\80"
    "\00\01\88\80\80\80\00\00\41\00\41\00\fd\51\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\b0\80\80\80"
    "\00\01\aa\80\80\80\00\00\41\00\fd\0c\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\fd\0c\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\fd\52"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\b0\80\80\80"
    "\00\01\aa\80\80\80\00\00\fd\0c\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\fd\0c\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\41\00\fd\52"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\41\00\41\00\fd\52"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\9e\80\80\80"
    "\00\01\98\80\80\80\00\00\41\00\fd\0c\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\fd\4f\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\9e\80\80\80"
    "\00\01\98\80\80\80\00\00\fd\0c\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\41\00\fd\4f\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8e\80\80\80"
    "\00\01\88\80\80\80\00\00\41\00\41\00\fd\4f\0b"
  )
  "type mismatch"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\03\8e\80\80\80\00\0d\00\00\00\00\00\00\00"
  "\00\00\00\00\00\00\05\83\80\80\80\00\01\00\01\07"
  "\80\82\80\80\00\0d\11\76\31\32\38\2e\6e\6f\74\2d"
  "\69\6e\2d\62\6c\6f\63\6b\00\00\11\76\31\32\38\2e"
  "\61\6e\64\2d\69\6e\2d\62\6c\6f\63\6b\00\01\10\76"
  "\31\32\38\2e\6f\72\2d\69\6e\2d\62\6c\6f\63\6b\00"
  "\02\11\76\31\32\38\2e\78\6f\72\2d\69\6e\2d\62\6c"
  "\6f\63\6b\00\03\17\76\31\32\38\2e\62\69\74\73\65"
  "\6c\65\63\74\2d\69\6e\2d\62\6c\6f\63\6b\00\04\14"
  "\76\31\32\38\2e\61\6e\64\6e\6f\74\2d\69\6e\2d\62"
  "\6c\6f\63\6b\00\05\0f\6e\65\73\74\65\64\2d\76\31"
  "\32\38\2e\6e\6f\74\00\06\0f\6e\65\73\74\65\64\2d"
  "\76\31\32\38\2e\61\6e\64\00\07\0e\6e\65\73\74\65"
  "\64\2d\76\31\32\38\2e\6f\72\00\08\0f\6e\65\73\74"
  "\65\64\2d\76\31\32\38\2e\78\6f\72\00\09\15\6e\65"
  "\73\74\65\64\2d\76\31\32\38\2e\62\69\74\73\65\6c"
  "\65\63\74\00\0a\12\6e\65\73\74\65\64\2d\76\31\32"
  "\38\2e\61\6e\64\6e\6f\74\00\0b\08\61\73\2d\70\61"
  "\72\61\6d\00\0c\0a\fd\85\80\80\00\0d\94\80\80\80"
  "\00\00\02\40\02\7b\02\7b\41\00\fd\00\04\00\0b\fd"
  "\4d\0b\1a\0b\0b\9d\80\80\80\00\00\02\40\02\7b\02"
  "\7b\41\00\fd\00\04\00\0b\02\7b\41\01\fd\00\04\00"
  "\0b\fd\4e\0b\1a\0b\0b\9d\80\80\80\00\00\02\40\02"
  "\7b\02\7b\41\00\fd\00\04\00\0b\02\7b\41\01\fd\00"
  "\04\00\0b\fd\50\0b\1a\0b\0b\9d\80\80\80\00\00\02"
  "\40\02\7b\02\7b\41\00\fd\00\04\00\0b\02\7b\41\01"
  "\fd\00\04\00\0b\fd\51\0b\1a\0b\0b\a6\80\80\80\00"
  "\00\02\40\02\7b\02\7b\41\00\fd\00\04\00\0b\02\7b"
  "\41\01\fd\00\04\00\0b\02\7b\41\02\fd\00\04\00\0b"
  "\fd\52\0b\1a\0b\0b\9d\80\80\80\00\00\02\40\02\7b"
  "\02\7b\41\00\fd\00\04\00\0b\02\7b\41\01\fd\00\04"
  "\00\0b\fd\4f\0b\1a\0b\0b\8f\80\80\80\00\00\41\00"
  "\fd\00\04\00\fd\4d\fd\4d\fd\4d\1a\0b\c1\80\80\80"
  "\00\00\41\00\fd\00\04\00\41\01\fd\00\04\00\fd\4e"
  "\41\00\fd\00\04\00\41\01\fd\00\04\00\fd\4e\fd\4e"
  "\41\00\fd\00\04\00\41\01\fd\00\04\00\fd\4e\41\00"
  "\fd\00\04\00\41\01\fd\00\04\00\fd\4e\fd\4e\fd\4e"
  "\1a\0b\c1\80\80\80\00\00\41\00\fd\00\04\00\41\01"
  "\fd\00\04\00\fd\50\41\00\fd\00\04\00\41\01\fd\00"
  "\04\00\fd\50\fd\50\41\00\fd\00\04\00\41\01\fd\00"
  "\04\00\fd\50\41\00\fd\00\04\00\41\01\fd\00\04\00"
  "\fd\50\fd\50\fd\50\1a\0b\c1\80\80\80\00\00\41\00"
  "\fd\00\04\00\41\01\fd\00\04\00\fd\51\41\00\fd\00"
  "\04\00\41\01\fd\00\04\00\fd\51\fd\51\41\00\fd\00"
  "\04\00\41\01\fd\00\04\00\fd\51\41\00\fd\00\04\00"
  "\41\01\fd\00\04\00\fd\51\fd\51\fd\51\1a\0b\bf\81"
  "\80\80\00\00\41\00\fd\00\04\00\41\01\fd\00\04\00"
  "\41\02\fd\00\04\00\fd\52\41\00\fd\00\04\00\41\01"
  "\fd\00\04\00\41\02\fd\00\04\00\fd\52\41\00\fd\00"
  "\04\00\41\01\fd\00\04\00\41\02\fd\00\04\00\fd\52"
  "\fd\52\41\00\fd\00\04\00\41\01\fd\00\04\00\41\02"
  "\fd\00\04\00\fd\52\41\00\fd\00\04\00\41\01\fd\00"
  "\04\00\41\02\fd\00\04\00\fd\52\41\00\fd\00\04\00"
  "\41\01\fd\00\04\00\41\02\fd\00\04\00\fd\52\fd\52"
  "\41\00\fd\00\04\00\41\01\fd\00\04\00\41\02\fd\00"
  "\04\00\fd\52\41\00\fd\00\04\00\41\01\fd\00\04\00"
  "\41\02\fd\00\04\00\fd\52\41\00\fd\00\04\00\41\01"
  "\fd\00\04\00\41\02\fd\00\04\00\fd\52\fd\52\fd\52"
  "\1a\0b\c1\80\80\80\00\00\41\00\fd\00\04\00\41\01"
  "\fd\00\04\00\fd\4f\41\00\fd\00\04\00\41\01\fd\00"
  "\04\00\fd\4f\fd\4f\41\00\fd\00\04\00\41\01\fd\00"
  "\04\00\fd\4f\41\00\fd\00\04\00\41\01\fd\00\04\00"
  "\fd\4f\fd\4f\fd\4f\1a\0b\bb\80\80\80\00\00\41\00"
  "\fd\00\04\00\fd\4d\41\01\fd\00\04\00\fd\4d\fd\4e"
  "\41\00\fd\00\04\00\41\01\fd\00\04\00\41\02\fd\00"
  "\04\00\fd\52\41\00\fd\00\04\00\41\01\fd\00\04\00"
  "\fd\4f\fd\51\fd\50\1a\0b"
)
(module instance)
(assert_return (invoke "v128.not-in-block"))
(assert_return (invoke "v128.and-in-block"))
(assert_return (invoke "v128.or-in-block"))
(assert_return (invoke "v128.xor-in-block"))
(assert_return (invoke "v128.bitselect-in-block"))
(assert_return (invoke "v128.andnot-in-block"))
(assert_return (invoke "nested-v128.not"))
(assert_return (invoke "nested-v128.and"))
(assert_return (invoke "nested-v128.or"))
(assert_return (invoke "nested-v128.xor"))
(assert_return (invoke "nested-v128.bitselect"))
(assert_return (invoke "nested-v128.andnot"))
(assert_return (invoke "as-param"))
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8a\80\80\80"
    "\00\01\84\80\80\80\00\00\fd\4d\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\9c\80\80\80"
    "\00\01\96\80\80\80\00\00\fd\0c\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\fd\4e\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8a\80\80\80"
    "\00\01\84\80\80\80\00\00\fd\4e\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\9c\80\80\80"
    "\00\01\96\80\80\80\00\00\fd\0c\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\fd\50\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8a\80\80\80"
    "\00\01\84\80\80\80\00\00\fd\50\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\9c\80\80\80"
    "\00\01\96\80\80\80\00\00\fd\0c\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\fd\51\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8a\80\80\80"
    "\00\01\84\80\80\80\00\00\fd\51\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\9c\80\80\80"
    "\00\01\96\80\80\80\00\00\fd\0c\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\fd\4f\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8a\80\80\80"
    "\00\01\84\80\80\80\00\00\fd\4f\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\ae\80\80\80"
    "\00\01\a8\80\80\80\00\00\fd\0c\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\fd\0c\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\fd\52\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\9c\80\80\80"
    "\00\01\96\80\80\80\00\00\fd\0c\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\fd\52\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7b\03\82\80\80\80\00\01\00\0a\8a\80\80\80"
    "\00\01\84\80\80\80\00\00\fd\52\0b"
  )
  "type mismatch"
)
