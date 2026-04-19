(module definition binary
  "\00\61\73\6d\01\00\00\00\01\97\80\80\80\00\04\60"
  "\02\7e\7e\01\7e\60\01\7e\01\7e\60\01\7e\01\7f\60"
  "\02\7e\7e\01\7f\03\a1\80\80\80\00\20\00\00\00\00"
  "\00\00\00\00\00\00\00\00\00\00\00\01\01\01\01\01"
  "\01\02\03\03\03\03\03\03\03\03\03\03\07\eb\81\80"
  "\80\00\20\03\61\64\64\00\00\03\73\75\62\00\01\03"
  "\6d\75\6c\00\02\05\64\69\76\5f\73\00\03\05\64\69"
  "\76\5f\75\00\04\05\72\65\6d\5f\73\00\05\05\72\65"
  "\6d\5f\75\00\06\03\61\6e\64\00\07\02\6f\72\00\08"
  "\03\78\6f\72\00\09\03\73\68\6c\00\0a\05\73\68\72"
  "\5f\73\00\0b\05\73\68\72\5f\75\00\0c\04\72\6f\74"
  "\6c\00\0d\04\72\6f\74\72\00\0e\03\63\6c\7a\00\0f"
  "\03\63\74\7a\00\10\06\70\6f\70\63\6e\74\00\11\09"
  "\65\78\74\65\6e\64\38\5f\73\00\12\0a\65\78\74\65"
  "\6e\64\31\36\5f\73\00\13\0a\65\78\74\65\6e\64\33"
  "\32\5f\73\00\14\03\65\71\7a\00\15\02\65\71\00\16"
  "\02\6e\65\00\17\04\6c\74\5f\73\00\18\04\6c\74\5f"
  "\75\00\19\04\6c\65\5f\73\00\1a\04\6c\65\5f\75\00"
  "\1b\04\67\74\5f\73\00\1c\04\67\74\5f\75\00\1d\04"
  "\67\65\5f\73\00\1e\04\67\65\5f\75\00\1f\0a\f3\82"
  "\80\80\00\20\87\80\80\80\00\00\20\00\20\01\7c\0b"
  "\87\80\80\80\00\00\20\00\20\01\7d\0b\87\80\80\80"
  "\00\00\20\00\20\01\7e\0b\87\80\80\80\00\00\20\00"
  "\20\01\7f\0b\87\80\80\80\00\00\20\00\20\01\80\0b"
  "\87\80\80\80\00\00\20\00\20\01\81\0b\87\80\80\80"
  "\00\00\20\00\20\01\82\0b\87\80\80\80\00\00\20\00"
  "\20\01\83\0b\87\80\80\80\00\00\20\00\20\01\84\0b"
  "\87\80\80\80\00\00\20\00\20\01\85\0b\87\80\80\80"
  "\00\00\20\00\20\01\86\0b\87\80\80\80\00\00\20\00"
  "\20\01\87\0b\87\80\80\80\00\00\20\00\20\01\88\0b"
  "\87\80\80\80\00\00\20\00\20\01\89\0b\87\80\80\80"
  "\00\00\20\00\20\01\8a\0b\85\80\80\80\00\00\20\00"
  "\79\0b\85\80\80\80\00\00\20\00\7a\0b\85\80\80\80"
  "\00\00\20\00\7b\0b\85\80\80\80\00\00\20\00\c2\0b"
  "\85\80\80\80\00\00\20\00\c3\0b\85\80\80\80\00\00"
  "\20\00\c4\0b\85\80\80\80\00\00\20\00\50\0b\87\80"
  "\80\80\00\00\20\00\20\01\51\0b\87\80\80\80\00\00"
  "\20\00\20\01\52\0b\87\80\80\80\00\00\20\00\20\01"
  "\53\0b\87\80\80\80\00\00\20\00\20\01\54\0b\87\80"
  "\80\80\00\00\20\00\20\01\57\0b\87\80\80\80\00\00"
  "\20\00\20\01\58\0b\87\80\80\80\00\00\20\00\20\01"
  "\55\0b\87\80\80\80\00\00\20\00\20\01\56\0b\87\80"
  "\80\80\00\00\20\00\20\01\59\0b\87\80\80\80\00\00"
  "\20\00\20\01\5a\0b"
)
(module instance)
(assert_return (invoke "add" (i64.const 0x1) (i64.const 0x1)) (i64.const 0x2))
(assert_return (invoke "add" (i64.const 0x1) (i64.const 0x0)) (i64.const 0x1))
(assert_return
  (invoke "add"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0xffff_ffff_ffff_fffe)
)
(assert_return
  (invoke "add" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x1))
  (i64.const 0x0)
)
(assert_return
  (invoke "add" (i64.const 0x7fff_ffff_ffff_ffff) (i64.const 0x1))
  (i64.const 0x8000_0000_0000_0000)
)
(assert_return
  (invoke "add"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0x7fff_ffff_ffff_ffff)
)
(assert_return
  (invoke "add"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i64.const 0x0)
)
(assert_return
  (invoke "add" (i64.const 0x3fff_ffff) (i64.const 0x1))
  (i64.const 0x4000_0000)
)
(assert_return (invoke "sub" (i64.const 0x1) (i64.const 0x1)) (i64.const 0x0))
(assert_return (invoke "sub" (i64.const 0x1) (i64.const 0x0)) (i64.const 0x1))
(assert_return
  (invoke "sub"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0x0)
)
(assert_return
  (invoke "sub"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0x8000_0000_0000_0000)
)
(assert_return
  (invoke "sub" (i64.const 0x8000_0000_0000_0000) (i64.const 0x1))
  (i64.const 0x7fff_ffff_ffff_ffff)
)
(assert_return
  (invoke "sub"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i64.const 0x0)
)
(assert_return
  (invoke "sub" (i64.const 0x3fff_ffff) (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0x4000_0000)
)
(assert_return (invoke "mul" (i64.const 0x1) (i64.const 0x1)) (i64.const 0x1))
(assert_return (invoke "mul" (i64.const 0x1) (i64.const 0x0)) (i64.const 0x0))
(assert_return
  (invoke "mul"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0x1)
)
(assert_return
  (invoke "mul" (i64.const 0x1000_0000_0000_0000) (i64.const 0x1000))
  (i64.const 0x0)
)
(assert_return
  (invoke "mul" (i64.const 0x8000_0000_0000_0000) (i64.const 0x0))
  (i64.const 0x0)
)
(assert_return
  (invoke "mul"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0x8000_0000_0000_0000)
)
(assert_return
  (invoke "mul"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0x8000_0000_0000_0001)
)
(assert_return
  (invoke "mul"
    (i64.const 0x123_4567_89ab_cdef)
    (i64.const 0xfedc_ba98_7654_3210)
  )
  (i64.const 0x2236_d88f_e561_8cf0)
)
(assert_return
  (invoke "mul"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i64.const 0x1)
)
(assert_trap
  (invoke "div_s" (i64.const 0x1) (i64.const 0x0))
  "integer divide by zero"
)
(assert_trap
  (invoke "div_s" (i64.const 0x0) (i64.const 0x0))
  "integer divide by zero"
)
(assert_trap
  (invoke "div_s"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  "integer overflow"
)
(assert_trap
  (invoke "div_s" (i64.const 0x8000_0000_0000_0000) (i64.const 0x0))
  "integer divide by zero"
)
(assert_return (invoke "div_s" (i64.const 0x1) (i64.const 0x1)) (i64.const 0x1))
(assert_return (invoke "div_s" (i64.const 0x0) (i64.const 0x1)) (i64.const 0x0))
(assert_return
  (invoke "div_s" (i64.const 0x0) (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0x0)
)
(assert_return
  (invoke "div_s"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0x1)
)
(assert_return
  (invoke "div_s" (i64.const 0x8000_0000_0000_0000) (i64.const 0x2))
  (i64.const 0xc000_0000_0000_0000)
)
(assert_return
  (invoke "div_s" (i64.const 0x8000_0000_0000_0001) (i64.const 0x3e8))
  (i64.const 0xffdf_3b64_5a1c_ac09)
)
(assert_return (invoke "div_s" (i64.const 0x5) (i64.const 0x2)) (i64.const 0x2))
(assert_return
  (invoke "div_s" (i64.const 0xffff_ffff_ffff_fffb) (i64.const 0x2))
  (i64.const 0xffff_ffff_ffff_fffe)
)
(assert_return
  (invoke "div_s" (i64.const 0x5) (i64.const 0xffff_ffff_ffff_fffe))
  (i64.const 0xffff_ffff_ffff_fffe)
)
(assert_return
  (invoke "div_s"
    (i64.const 0xffff_ffff_ffff_fffb)
    (i64.const 0xffff_ffff_ffff_fffe)
  )
  (i64.const 0x2)
)
(assert_return (invoke "div_s" (i64.const 0x7) (i64.const 0x3)) (i64.const 0x2))
(assert_return
  (invoke "div_s" (i64.const 0xffff_ffff_ffff_fff9) (i64.const 0x3))
  (i64.const 0xffff_ffff_ffff_fffe)
)
(assert_return
  (invoke "div_s" (i64.const 0x7) (i64.const 0xffff_ffff_ffff_fffd))
  (i64.const 0xffff_ffff_ffff_fffe)
)
(assert_return
  (invoke "div_s"
    (i64.const 0xffff_ffff_ffff_fff9)
    (i64.const 0xffff_ffff_ffff_fffd)
  )
  (i64.const 0x2)
)
(assert_return (invoke "div_s" (i64.const 0xb) (i64.const 0x5)) (i64.const 0x2))
(assert_return
  (invoke "div_s" (i64.const 0x11) (i64.const 0x7))
  (i64.const 0x2)
)
(assert_trap
  (invoke "div_u" (i64.const 0x1) (i64.const 0x0))
  "integer divide by zero"
)
(assert_trap
  (invoke "div_u" (i64.const 0x0) (i64.const 0x0))
  "integer divide by zero"
)
(assert_return (invoke "div_u" (i64.const 0x1) (i64.const 0x1)) (i64.const 0x1))
(assert_return (invoke "div_u" (i64.const 0x0) (i64.const 0x1)) (i64.const 0x0))
(assert_return
  (invoke "div_u"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0x1)
)
(assert_return
  (invoke "div_u"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0x0)
)
(assert_return
  (invoke "div_u" (i64.const 0x8000_0000_0000_0000) (i64.const 0x2))
  (i64.const 0x4000_0000_0000_0000)
)
(assert_return
  (invoke "div_u" (i64.const 0x8ff0_0ff0_0ff0_0ff0) (i64.const 0x1_0000_0001))
  (i64.const 0x8ff0_0fef)
)
(assert_return
  (invoke "div_u" (i64.const 0x8000_0000_0000_0001) (i64.const 0x3e8))
  (i64.const 0x20_c49b_a5e3_53f7)
)
(assert_return (invoke "div_u" (i64.const 0x5) (i64.const 0x2)) (i64.const 0x2))
(assert_return
  (invoke "div_u" (i64.const 0xffff_ffff_ffff_fffb) (i64.const 0x2))
  (i64.const 0x7fff_ffff_ffff_fffd)
)
(assert_return
  (invoke "div_u" (i64.const 0x5) (i64.const 0xffff_ffff_ffff_fffe))
  (i64.const 0x0)
)
(assert_return
  (invoke "div_u"
    (i64.const 0xffff_ffff_ffff_fffb)
    (i64.const 0xffff_ffff_ffff_fffe)
  )
  (i64.const 0x0)
)
(assert_return (invoke "div_u" (i64.const 0x7) (i64.const 0x3)) (i64.const 0x2))
(assert_return (invoke "div_u" (i64.const 0xb) (i64.const 0x5)) (i64.const 0x2))
(assert_return
  (invoke "div_u" (i64.const 0x11) (i64.const 0x7))
  (i64.const 0x2)
)
(assert_trap
  (invoke "rem_s" (i64.const 0x1) (i64.const 0x0))
  "integer divide by zero"
)
(assert_trap
  (invoke "rem_s" (i64.const 0x0) (i64.const 0x0))
  "integer divide by zero"
)
(assert_return
  (invoke "rem_s"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0x0)
)
(assert_return (invoke "rem_s" (i64.const 0x1) (i64.const 0x1)) (i64.const 0x0))
(assert_return (invoke "rem_s" (i64.const 0x0) (i64.const 0x1)) (i64.const 0x0))
(assert_return
  (invoke "rem_s" (i64.const 0x0) (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0x0)
)
(assert_return
  (invoke "rem_s"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0x0)
)
(assert_return
  (invoke "rem_s"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0x0)
)
(assert_return
  (invoke "rem_s" (i64.const 0x8000_0000_0000_0000) (i64.const 0x2))
  (i64.const 0x0)
)
(assert_return
  (invoke "rem_s" (i64.const 0x8000_0000_0000_0001) (i64.const 0x3e8))
  (i64.const 0xffff_ffff_ffff_fcd9)
)
(assert_return (invoke "rem_s" (i64.const 0x5) (i64.const 0x2)) (i64.const 0x1))
(assert_return
  (invoke "rem_s" (i64.const 0xffff_ffff_ffff_fffb) (i64.const 0x2))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "rem_s" (i64.const 0x5) (i64.const 0xffff_ffff_ffff_fffe))
  (i64.const 0x1)
)
(assert_return
  (invoke "rem_s"
    (i64.const 0xffff_ffff_ffff_fffb)
    (i64.const 0xffff_ffff_ffff_fffe)
  )
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return (invoke "rem_s" (i64.const 0x7) (i64.const 0x3)) (i64.const 0x1))
(assert_return
  (invoke "rem_s" (i64.const 0xffff_ffff_ffff_fff9) (i64.const 0x3))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "rem_s" (i64.const 0x7) (i64.const 0xffff_ffff_ffff_fffd))
  (i64.const 0x1)
)
(assert_return
  (invoke "rem_s"
    (i64.const 0xffff_ffff_ffff_fff9)
    (i64.const 0xffff_ffff_ffff_fffd)
  )
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return (invoke "rem_s" (i64.const 0xb) (i64.const 0x5)) (i64.const 0x1))
(assert_return
  (invoke "rem_s" (i64.const 0x11) (i64.const 0x7))
  (i64.const 0x3)
)
(assert_trap
  (invoke "rem_u" (i64.const 0x1) (i64.const 0x0))
  "integer divide by zero"
)
(assert_trap
  (invoke "rem_u" (i64.const 0x0) (i64.const 0x0))
  "integer divide by zero"
)
(assert_return (invoke "rem_u" (i64.const 0x1) (i64.const 0x1)) (i64.const 0x0))
(assert_return (invoke "rem_u" (i64.const 0x0) (i64.const 0x1)) (i64.const 0x0))
(assert_return
  (invoke "rem_u"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0x0)
)
(assert_return
  (invoke "rem_u"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0x8000_0000_0000_0000)
)
(assert_return
  (invoke "rem_u" (i64.const 0x8000_0000_0000_0000) (i64.const 0x2))
  (i64.const 0x0)
)
(assert_return
  (invoke "rem_u" (i64.const 0x8ff0_0ff0_0ff0_0ff0) (i64.const 0x1_0000_0001))
  (i64.const 0x8000_0001)
)
(assert_return
  (invoke "rem_u" (i64.const 0x8000_0000_0000_0001) (i64.const 0x3e8))
  (i64.const 0x329)
)
(assert_return (invoke "rem_u" (i64.const 0x5) (i64.const 0x2)) (i64.const 0x1))
(assert_return
  (invoke "rem_u" (i64.const 0xffff_ffff_ffff_fffb) (i64.const 0x2))
  (i64.const 0x1)
)
(assert_return
  (invoke "rem_u" (i64.const 0x5) (i64.const 0xffff_ffff_ffff_fffe))
  (i64.const 0x5)
)
(assert_return
  (invoke "rem_u"
    (i64.const 0xffff_ffff_ffff_fffb)
    (i64.const 0xffff_ffff_ffff_fffe)
  )
  (i64.const 0xffff_ffff_ffff_fffb)
)
(assert_return (invoke "rem_u" (i64.const 0x7) (i64.const 0x3)) (i64.const 0x1))
(assert_return (invoke "rem_u" (i64.const 0xb) (i64.const 0x5)) (i64.const 0x1))
(assert_return
  (invoke "rem_u" (i64.const 0x11) (i64.const 0x7))
  (i64.const 0x3)
)
(assert_return (invoke "and" (i64.const 0x1) (i64.const 0x0)) (i64.const 0x0))
(assert_return (invoke "and" (i64.const 0x0) (i64.const 0x1)) (i64.const 0x0))
(assert_return (invoke "and" (i64.const 0x1) (i64.const 0x1)) (i64.const 0x1))
(assert_return (invoke "and" (i64.const 0x0) (i64.const 0x0)) (i64.const 0x0))
(assert_return
  (invoke "and"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i64.const 0x0)
)
(assert_return
  (invoke "and"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0x7fff_ffff_ffff_ffff)
)
(assert_return
  (invoke "and" (i64.const 0xf0f0_ffff) (i64.const 0xffff_f0f0))
  (i64.const 0xf0f0_f0f0)
)
(assert_return
  (invoke "and"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return (invoke "or" (i64.const 0x1) (i64.const 0x0)) (i64.const 0x1))
(assert_return (invoke "or" (i64.const 0x0) (i64.const 0x1)) (i64.const 0x1))
(assert_return (invoke "or" (i64.const 0x1) (i64.const 0x1)) (i64.const 0x1))
(assert_return (invoke "or" (i64.const 0x0) (i64.const 0x0)) (i64.const 0x0))
(assert_return
  (invoke "or"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "or" (i64.const 0x8000_0000_0000_0000) (i64.const 0x0))
  (i64.const 0x8000_0000_0000_0000)
)
(assert_return
  (invoke "or" (i64.const 0xf0f0_ffff) (i64.const 0xffff_f0f0))
  (i64.const 0xffff_ffff)
)
(assert_return
  (invoke "or"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return (invoke "xor" (i64.const 0x1) (i64.const 0x0)) (i64.const 0x1))
(assert_return (invoke "xor" (i64.const 0x0) (i64.const 0x1)) (i64.const 0x1))
(assert_return (invoke "xor" (i64.const 0x1) (i64.const 0x1)) (i64.const 0x0))
(assert_return (invoke "xor" (i64.const 0x0) (i64.const 0x0)) (i64.const 0x0))
(assert_return
  (invoke "xor"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "xor" (i64.const 0x8000_0000_0000_0000) (i64.const 0x0))
  (i64.const 0x8000_0000_0000_0000)
)
(assert_return
  (invoke "xor"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i64.const 0x7fff_ffff_ffff_ffff)
)
(assert_return
  (invoke "xor"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i64.const 0x8000_0000_0000_0000)
)
(assert_return
  (invoke "xor" (i64.const 0xf0f0_ffff) (i64.const 0xffff_f0f0))
  (i64.const 0xf0f_0f0f)
)
(assert_return
  (invoke "xor"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0x0)
)
(assert_return (invoke "shl" (i64.const 0x1) (i64.const 0x1)) (i64.const 0x2))
(assert_return (invoke "shl" (i64.const 0x1) (i64.const 0x0)) (i64.const 0x1))
(assert_return
  (invoke "shl" (i64.const 0x7fff_ffff_ffff_ffff) (i64.const 0x1))
  (i64.const 0xffff_ffff_ffff_fffe)
)
(assert_return
  (invoke "shl" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x1))
  (i64.const 0xffff_ffff_ffff_fffe)
)
(assert_return
  (invoke "shl" (i64.const 0x8000_0000_0000_0000) (i64.const 0x1))
  (i64.const 0x0)
)
(assert_return
  (invoke "shl" (i64.const 0x4000_0000_0000_0000) (i64.const 0x1))
  (i64.const 0x8000_0000_0000_0000)
)
(assert_return
  (invoke "shl" (i64.const 0x1) (i64.const 0x3f))
  (i64.const 0x8000_0000_0000_0000)
)
(assert_return (invoke "shl" (i64.const 0x1) (i64.const 0x40)) (i64.const 0x1))
(assert_return (invoke "shl" (i64.const 0x1) (i64.const 0x41)) (i64.const 0x2))
(assert_return
  (invoke "shl" (i64.const 0x1) (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0x8000_0000_0000_0000)
)
(assert_return
  (invoke "shl" (i64.const 0x1) (i64.const 0x7fff_ffff_ffff_ffff))
  (i64.const 0x8000_0000_0000_0000)
)
(assert_return (invoke "shr_s" (i64.const 0x1) (i64.const 0x1)) (i64.const 0x0))
(assert_return (invoke "shr_s" (i64.const 0x1) (i64.const 0x0)) (i64.const 0x1))
(assert_return
  (invoke "shr_s" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x1))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "shr_s" (i64.const 0x7fff_ffff_ffff_ffff) (i64.const 0x1))
  (i64.const 0x3fff_ffff_ffff_ffff)
)
(assert_return
  (invoke "shr_s" (i64.const 0x8000_0000_0000_0000) (i64.const 0x1))
  (i64.const 0xc000_0000_0000_0000)
)
(assert_return
  (invoke "shr_s" (i64.const 0x4000_0000_0000_0000) (i64.const 0x1))
  (i64.const 0x2000_0000_0000_0000)
)
(assert_return
  (invoke "shr_s" (i64.const 0x1) (i64.const 0x40))
  (i64.const 0x1)
)
(assert_return
  (invoke "shr_s" (i64.const 0x1) (i64.const 0x41))
  (i64.const 0x0)
)
(assert_return
  (invoke "shr_s" (i64.const 0x1) (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0x0)
)
(assert_return
  (invoke "shr_s" (i64.const 0x1) (i64.const 0x7fff_ffff_ffff_ffff))
  (i64.const 0x0)
)
(assert_return
  (invoke "shr_s" (i64.const 0x1) (i64.const 0x8000_0000_0000_0000))
  (i64.const 0x1)
)
(assert_return
  (invoke "shr_s" (i64.const 0x8000_0000_0000_0000) (i64.const 0x3f))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "shr_s" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x40))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "shr_s" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x41))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "shr_s"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "shr_s"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "shr_s"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return (invoke "shr_u" (i64.const 0x1) (i64.const 0x1)) (i64.const 0x0))
(assert_return (invoke "shr_u" (i64.const 0x1) (i64.const 0x0)) (i64.const 0x1))
(assert_return
  (invoke "shr_u" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x1))
  (i64.const 0x7fff_ffff_ffff_ffff)
)
(assert_return
  (invoke "shr_u" (i64.const 0x7fff_ffff_ffff_ffff) (i64.const 0x1))
  (i64.const 0x3fff_ffff_ffff_ffff)
)
(assert_return
  (invoke "shr_u" (i64.const 0x8000_0000_0000_0000) (i64.const 0x1))
  (i64.const 0x4000_0000_0000_0000)
)
(assert_return
  (invoke "shr_u" (i64.const 0x4000_0000_0000_0000) (i64.const 0x1))
  (i64.const 0x2000_0000_0000_0000)
)
(assert_return
  (invoke "shr_u" (i64.const 0x1) (i64.const 0x40))
  (i64.const 0x1)
)
(assert_return
  (invoke "shr_u" (i64.const 0x1) (i64.const 0x41))
  (i64.const 0x0)
)
(assert_return
  (invoke "shr_u" (i64.const 0x1) (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0x0)
)
(assert_return
  (invoke "shr_u" (i64.const 0x1) (i64.const 0x7fff_ffff_ffff_ffff))
  (i64.const 0x0)
)
(assert_return
  (invoke "shr_u" (i64.const 0x1) (i64.const 0x8000_0000_0000_0000))
  (i64.const 0x1)
)
(assert_return
  (invoke "shr_u" (i64.const 0x8000_0000_0000_0000) (i64.const 0x3f))
  (i64.const 0x1)
)
(assert_return
  (invoke "shr_u" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x40))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "shr_u" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x41))
  (i64.const 0x7fff_ffff_ffff_ffff)
)
(assert_return
  (invoke "shr_u"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i64.const 0x1)
)
(assert_return
  (invoke "shr_u"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i64.const 0x1)
)
(assert_return
  (invoke "shr_u"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return (invoke "rotl" (i64.const 0x1) (i64.const 0x1)) (i64.const 0x2))
(assert_return (invoke "rotl" (i64.const 0x1) (i64.const 0x0)) (i64.const 0x1))
(assert_return
  (invoke "rotl" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x1))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return (invoke "rotl" (i64.const 0x1) (i64.const 0x40)) (i64.const 0x1))
(assert_return
  (invoke "rotl" (i64.const 0xabcd_9876_0246_8ace) (i64.const 0x1))
  (i64.const 0x579b_30ec_048d_159d)
)
(assert_return
  (invoke "rotl" (i64.const 0xfe00_0000_dc00_0000) (i64.const 0x4))
  (i64.const 0xe000_000d_c000_000f)
)
(assert_return
  (invoke "rotl" (i64.const 0xabcd_1234_ef56_7809) (i64.const 0x35))
  (i64.const 0x135_79a2_469d_eacf)
)
(assert_return
  (invoke "rotl" (i64.const 0xabd1_234e_f567_809c) (i64.const 0x3f))
  (i64.const 0x55e8_91a7_7ab3_c04e)
)
(assert_return
  (invoke "rotl" (i64.const 0xabcd_1234_ef56_7809) (i64.const 0xf5))
  (i64.const 0x135_79a2_469d_eacf)
)
(assert_return
  (invoke "rotl"
    (i64.const 0xabcd_7294_ef56_7809)
    (i64.const 0xffff_ffff_ffff_ffed)
  )
  (i64.const 0xcf01_3579_ae52_9dea)
)
(assert_return
  (invoke "rotl"
    (i64.const 0xabd1_234e_f567_809c)
    (i64.const 0x8000_0000_0000_003f)
  )
  (i64.const 0x55e8_91a7_7ab3_c04e)
)
(assert_return
  (invoke "rotl" (i64.const 0x1) (i64.const 0x3f))
  (i64.const 0x8000_0000_0000_0000)
)
(assert_return
  (invoke "rotl" (i64.const 0x8000_0000_0000_0000) (i64.const 0x1))
  (i64.const 0x1)
)
(assert_return
  (invoke "rotr" (i64.const 0x1) (i64.const 0x1))
  (i64.const 0x8000_0000_0000_0000)
)
(assert_return (invoke "rotr" (i64.const 0x1) (i64.const 0x0)) (i64.const 0x1))
(assert_return
  (invoke "rotr" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x1))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return (invoke "rotr" (i64.const 0x1) (i64.const 0x40)) (i64.const 0x1))
(assert_return
  (invoke "rotr" (i64.const 0xabcd_9876_0246_8ace) (i64.const 0x1))
  (i64.const 0x55e6_cc3b_0123_4567)
)
(assert_return
  (invoke "rotr" (i64.const 0xfe00_0000_dc00_0000) (i64.const 0x4))
  (i64.const 0xfe0_0000_0dc0_0000)
)
(assert_return
  (invoke "rotr" (i64.const 0xabcd_1234_ef56_7809) (i64.const 0x35))
  (i64.const 0x6891_a77a_b3c0_4d5e)
)
(assert_return
  (invoke "rotr" (i64.const 0xabd1_234e_f567_809c) (i64.const 0x3f))
  (i64.const 0x57a2_469d_eacf_0139)
)
(assert_return
  (invoke "rotr" (i64.const 0xabcd_1234_ef56_7809) (i64.const 0xf5))
  (i64.const 0x6891_a77a_b3c0_4d5e)
)
(assert_return
  (invoke "rotr"
    (i64.const 0xabcd_7294_ef56_7809)
    (i64.const 0xffff_ffff_ffff_ffed)
  )
  (i64.const 0x94a7_7ab3_c04d_5e6b)
)
(assert_return
  (invoke "rotr"
    (i64.const 0xabd1_234e_f567_809c)
    (i64.const 0x8000_0000_0000_003f)
  )
  (i64.const 0x57a2_469d_eacf_0139)
)
(assert_return (invoke "rotr" (i64.const 0x1) (i64.const 0x3f)) (i64.const 0x2))
(assert_return
  (invoke "rotr" (i64.const 0x8000_0000_0000_0000) (i64.const 0x3f))
  (i64.const 0x1)
)
(assert_return (invoke "clz" (i64.const 0xffff_ffff_ffff_ffff)) (i64.const 0x0))
(assert_return (invoke "clz" (i64.const 0x0)) (i64.const 0x40))
(assert_return (invoke "clz" (i64.const 0x8000)) (i64.const 0x30))
(assert_return (invoke "clz" (i64.const 0xff)) (i64.const 0x38))
(assert_return (invoke "clz" (i64.const 0x8000_0000_0000_0000)) (i64.const 0x0))
(assert_return (invoke "clz" (i64.const 0x1)) (i64.const 0x3f))
(assert_return (invoke "clz" (i64.const 0x2)) (i64.const 0x3e))
(assert_return (invoke "clz" (i64.const 0x7fff_ffff_ffff_ffff)) (i64.const 0x1))
(assert_return (invoke "ctz" (i64.const 0xffff_ffff_ffff_ffff)) (i64.const 0x0))
(assert_return (invoke "ctz" (i64.const 0x0)) (i64.const 0x40))
(assert_return (invoke "ctz" (i64.const 0x8000)) (i64.const 0xf))
(assert_return (invoke "ctz" (i64.const 0x1_0000)) (i64.const 0x10))
(assert_return
  (invoke "ctz" (i64.const 0x8000_0000_0000_0000))
  (i64.const 0x3f)
)
(assert_return (invoke "ctz" (i64.const 0x7fff_ffff_ffff_ffff)) (i64.const 0x0))
(assert_return
  (invoke "popcnt" (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0x40)
)
(assert_return (invoke "popcnt" (i64.const 0x0)) (i64.const 0x0))
(assert_return (invoke "popcnt" (i64.const 0x8000)) (i64.const 0x1))
(assert_return
  (invoke "popcnt" (i64.const 0x8000_8000_8000_8000))
  (i64.const 0x4)
)
(assert_return
  (invoke "popcnt" (i64.const 0x7fff_ffff_ffff_ffff))
  (i64.const 0x3f)
)
(assert_return
  (invoke "popcnt" (i64.const 0xaaaa_aaaa_5555_5555))
  (i64.const 0x20)
)
(assert_return
  (invoke "popcnt" (i64.const 0x9999_9999_aaaa_aaaa))
  (i64.const 0x20)
)
(assert_return
  (invoke "popcnt" (i64.const 0xdead_beef_dead_beef))
  (i64.const 0x30)
)
(assert_return (invoke "extend8_s" (i64.const 0x0)) (i64.const 0x0))
(assert_return (invoke "extend8_s" (i64.const 0x7f)) (i64.const 0x7f))
(assert_return
  (invoke "extend8_s" (i64.const 0x80))
  (i64.const 0xffff_ffff_ffff_ff80)
)
(assert_return
  (invoke "extend8_s" (i64.const 0xff))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "extend8_s" (i64.const 0x123_4567_89ab_cd00))
  (i64.const 0x0)
)
(assert_return
  (invoke "extend8_s" (i64.const 0xfedc_ba98_7654_3280))
  (i64.const 0xffff_ffff_ffff_ff80)
)
(assert_return
  (invoke "extend8_s" (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return (invoke "extend16_s" (i64.const 0x0)) (i64.const 0x0))
(assert_return (invoke "extend16_s" (i64.const 0x7fff)) (i64.const 0x7fff))
(assert_return
  (invoke "extend16_s" (i64.const 0x8000))
  (i64.const 0xffff_ffff_ffff_8000)
)
(assert_return
  (invoke "extend16_s" (i64.const 0xffff))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "extend16_s" (i64.const 0x1234_5678_9abc_0000))
  (i64.const 0x0)
)
(assert_return
  (invoke "extend16_s" (i64.const 0xfedc_ba98_7654_8000))
  (i64.const 0xffff_ffff_ffff_8000)
)
(assert_return
  (invoke "extend16_s" (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return (invoke "extend32_s" (i64.const 0x0)) (i64.const 0x0))
(assert_return (invoke "extend32_s" (i64.const 0x7fff)) (i64.const 0x7fff))
(assert_return (invoke "extend32_s" (i64.const 0x8000)) (i64.const 0x8000))
(assert_return (invoke "extend32_s" (i64.const 0xffff)) (i64.const 0xffff))
(assert_return
  (invoke "extend32_s" (i64.const 0x7fff_ffff))
  (i64.const 0x7fff_ffff)
)
(assert_return
  (invoke "extend32_s" (i64.const 0x8000_0000))
  (i64.const 0xffff_ffff_8000_0000)
)
(assert_return
  (invoke "extend32_s" (i64.const 0xffff_ffff))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "extend32_s" (i64.const 0x123_4567_0000_0000))
  (i64.const 0x0)
)
(assert_return
  (invoke "extend32_s" (i64.const 0xfedc_ba98_8000_0000))
  (i64.const 0xffff_ffff_8000_0000)
)
(assert_return
  (invoke "extend32_s" (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return (invoke "eqz" (i64.const 0x0)) (i32.const 0x1))
(assert_return (invoke "eqz" (i64.const 0x1)) (i32.const 0x0))
(assert_return (invoke "eqz" (i64.const 0x8000_0000_0000_0000)) (i32.const 0x0))
(assert_return (invoke "eqz" (i64.const 0x7fff_ffff_ffff_ffff)) (i32.const 0x0))
(assert_return (invoke "eqz" (i64.const 0xffff_ffff_ffff_ffff)) (i32.const 0x0))
(assert_return (invoke "eq" (i64.const 0x0) (i64.const 0x0)) (i32.const 0x1))
(assert_return (invoke "eq" (i64.const 0x1) (i64.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "eq" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x1))
  (i32.const 0x0)
)
(assert_return
  (invoke "eq"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "eq"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "eq"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return (invoke "eq" (i64.const 0x1) (i64.const 0x0)) (i32.const 0x0))
(assert_return (invoke "eq" (i64.const 0x0) (i64.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "eq" (i64.const 0x8000_0000_0000_0000) (i64.const 0x0))
  (i32.const 0x0)
)
(assert_return
  (invoke "eq" (i64.const 0x0) (i64.const 0x8000_0000_0000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "eq"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "eq"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "eq"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "eq"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_return (invoke "ne" (i64.const 0x0) (i64.const 0x0)) (i32.const 0x0))
(assert_return (invoke "ne" (i64.const 0x1) (i64.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "ne" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x1))
  (i32.const 0x1)
)
(assert_return
  (invoke "ne"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "ne"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "ne"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return (invoke "ne" (i64.const 0x1) (i64.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ne" (i64.const 0x0) (i64.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "ne" (i64.const 0x8000_0000_0000_0000) (i64.const 0x0))
  (i32.const 0x1)
)
(assert_return
  (invoke "ne" (i64.const 0x0) (i64.const 0x8000_0000_0000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "ne"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "ne"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "ne"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "ne"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x1)
)
(assert_return (invoke "lt_s" (i64.const 0x0) (i64.const 0x0)) (i32.const 0x0))
(assert_return (invoke "lt_s" (i64.const 0x1) (i64.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "lt_s" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x1))
  (i32.const 0x1)
)
(assert_return
  (invoke "lt_s"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_s"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_s"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return (invoke "lt_s" (i64.const 0x1) (i64.const 0x0)) (i32.const 0x0))
(assert_return (invoke "lt_s" (i64.const 0x0) (i64.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "lt_s" (i64.const 0x8000_0000_0000_0000) (i64.const 0x0))
  (i32.const 0x1)
)
(assert_return
  (invoke "lt_s" (i64.const 0x0) (i64.const 0x8000_0000_0000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_s"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "lt_s"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_s"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "lt_s"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_return (invoke "lt_u" (i64.const 0x0) (i64.const 0x0)) (i32.const 0x0))
(assert_return (invoke "lt_u" (i64.const 0x1) (i64.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "lt_u" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x1))
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_u"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_u"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_u"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return (invoke "lt_u" (i64.const 0x1) (i64.const 0x0)) (i32.const 0x0))
(assert_return (invoke "lt_u" (i64.const 0x0) (i64.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "lt_u" (i64.const 0x8000_0000_0000_0000) (i64.const 0x0))
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_u" (i64.const 0x0) (i64.const 0x8000_0000_0000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "lt_u"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "lt_u"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_u"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_u"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x1)
)
(assert_return (invoke "le_s" (i64.const 0x0) (i64.const 0x0)) (i32.const 0x1))
(assert_return (invoke "le_s" (i64.const 0x1) (i64.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "le_s" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x1))
  (i32.const 0x1)
)
(assert_return
  (invoke "le_s"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "le_s"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "le_s"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return (invoke "le_s" (i64.const 0x1) (i64.const 0x0)) (i32.const 0x0))
(assert_return (invoke "le_s" (i64.const 0x0) (i64.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "le_s" (i64.const 0x8000_0000_0000_0000) (i64.const 0x0))
  (i32.const 0x1)
)
(assert_return
  (invoke "le_s" (i64.const 0x0) (i64.const 0x8000_0000_0000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "le_s"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "le_s"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "le_s"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "le_s"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_return (invoke "le_u" (i64.const 0x0) (i64.const 0x0)) (i32.const 0x1))
(assert_return (invoke "le_u" (i64.const 0x1) (i64.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "le_u" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x1))
  (i32.const 0x0)
)
(assert_return
  (invoke "le_u"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "le_u"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "le_u"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return (invoke "le_u" (i64.const 0x1) (i64.const 0x0)) (i32.const 0x0))
(assert_return (invoke "le_u" (i64.const 0x0) (i64.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "le_u" (i64.const 0x8000_0000_0000_0000) (i64.const 0x0))
  (i32.const 0x0)
)
(assert_return
  (invoke "le_u" (i64.const 0x0) (i64.const 0x8000_0000_0000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "le_u"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "le_u"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "le_u"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "le_u"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x1)
)
(assert_return (invoke "gt_s" (i64.const 0x0) (i64.const 0x0)) (i32.const 0x0))
(assert_return (invoke "gt_s" (i64.const 0x1) (i64.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "gt_s" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x1))
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_s"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_s"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_s"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return (invoke "gt_s" (i64.const 0x1) (i64.const 0x0)) (i32.const 0x1))
(assert_return (invoke "gt_s" (i64.const 0x0) (i64.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "gt_s" (i64.const 0x8000_0000_0000_0000) (i64.const 0x0))
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_s" (i64.const 0x0) (i64.const 0x8000_0000_0000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "gt_s"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_s"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "gt_s"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_s"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x1)
)
(assert_return (invoke "gt_u" (i64.const 0x0) (i64.const 0x0)) (i32.const 0x0))
(assert_return (invoke "gt_u" (i64.const 0x1) (i64.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "gt_u" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x1))
  (i32.const 0x1)
)
(assert_return
  (invoke "gt_u"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_u"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_u"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return (invoke "gt_u" (i64.const 0x1) (i64.const 0x0)) (i32.const 0x1))
(assert_return (invoke "gt_u" (i64.const 0x0) (i64.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "gt_u" (i64.const 0x8000_0000_0000_0000) (i64.const 0x0))
  (i32.const 0x1)
)
(assert_return
  (invoke "gt_u" (i64.const 0x0) (i64.const 0x8000_0000_0000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_u"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_u"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "gt_u"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "gt_u"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_return (invoke "ge_s" (i64.const 0x0) (i64.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ge_s" (i64.const 0x1) (i64.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "ge_s" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x1))
  (i32.const 0x0)
)
(assert_return
  (invoke "ge_s"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_s"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_s"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return (invoke "ge_s" (i64.const 0x1) (i64.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ge_s" (i64.const 0x0) (i64.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "ge_s" (i64.const 0x8000_0000_0000_0000) (i64.const 0x0))
  (i32.const 0x0)
)
(assert_return
  (invoke "ge_s" (i64.const 0x0) (i64.const 0x8000_0000_0000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_s"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "ge_s"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_s"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "ge_s"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x1)
)
(assert_return (invoke "ge_u" (i64.const 0x0) (i64.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ge_u" (i64.const 0x1) (i64.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "ge_u" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x1))
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_u"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_u"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_u"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return (invoke "ge_u" (i64.const 0x1) (i64.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ge_u" (i64.const 0x0) (i64.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "ge_u" (i64.const 0x8000_0000_0000_0000) (i64.const 0x0))
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_u" (i64.const 0x0) (i64.const 0x8000_0000_0000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "ge_u"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  (i32.const 0x0)
)
(assert_return
  (invoke "ge_u"
    (i64.const 0xffff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_u"
    (i64.const 0x8000_0000_0000_0000)
    (i64.const 0x7fff_ffff_ffff_ffff)
  )
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_u"
    (i64.const 0x7fff_ffff_ffff_ffff)
    (i64.const 0x8000_0000_0000_0000)
  )
  (i32.const 0x0)
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\7c"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\83"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\7f"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\80"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\7e"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\84"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\81"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\82"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\89"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\8a"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\86"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\87"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\88"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\7d"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\85"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\8b\80\80\80"
    "\00\01\85\80\80\80\00\00\41\00\50\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\8b\80\80\80"
    "\00\01\85\80\80\80\00\00\41\00\79\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\8b\80\80\80"
    "\00\01\85\80\80\80\00\00\41\00\7a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\8b\80\80\80"
    "\00\01\85\80\80\80\00\00\41\00\7b\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\51"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\59"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\5a"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\55"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\56"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\57"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\58"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\53"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\54"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7e\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\41\00\43\00\00\00\00\52"
    "\0b"
  )
  "type mismatch"
)
