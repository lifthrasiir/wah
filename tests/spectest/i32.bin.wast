(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8c\80\80\80\00\02\60"
  "\02\7f\7f\01\7f\60\01\7f\01\7f\03\a0\80\80\80\00"
  "\1f\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
  "\01\01\01\01\01\01\00\00\00\00\00\00\00\00\00\00"
  "\07\de\81\80\80\00\1f\03\61\64\64\00\00\03\73\75"
  "\62\00\01\03\6d\75\6c\00\02\05\64\69\76\5f\73\00"
  "\03\05\64\69\76\5f\75\00\04\05\72\65\6d\5f\73\00"
  "\05\05\72\65\6d\5f\75\00\06\03\61\6e\64\00\07\02"
  "\6f\72\00\08\03\78\6f\72\00\09\03\73\68\6c\00\0a"
  "\05\73\68\72\5f\73\00\0b\05\73\68\72\5f\75\00\0c"
  "\04\72\6f\74\6c\00\0d\04\72\6f\74\72\00\0e\03\63"
  "\6c\7a\00\0f\03\63\74\7a\00\10\06\70\6f\70\63\6e"
  "\74\00\11\09\65\78\74\65\6e\64\38\5f\73\00\12\0a"
  "\65\78\74\65\6e\64\31\36\5f\73\00\13\03\65\71\7a"
  "\00\14\02\65\71\00\15\02\6e\65\00\16\04\6c\74\5f"
  "\73\00\17\04\6c\74\5f\75\00\18\04\6c\65\5f\73\00"
  "\19\04\6c\65\5f\75\00\1a\04\67\74\5f\73\00\1b\04"
  "\67\74\5f\75\00\1c\04\67\65\5f\73\00\1d\04\67\65"
  "\5f\75\00\1e\0a\e9\82\80\80\00\1f\87\80\80\80\00"
  "\00\20\00\20\01\6a\0b\87\80\80\80\00\00\20\00\20"
  "\01\6b\0b\87\80\80\80\00\00\20\00\20\01\6c\0b\87"
  "\80\80\80\00\00\20\00\20\01\6d\0b\87\80\80\80\00"
  "\00\20\00\20\01\6e\0b\87\80\80\80\00\00\20\00\20"
  "\01\6f\0b\87\80\80\80\00\00\20\00\20\01\70\0b\87"
  "\80\80\80\00\00\20\00\20\01\71\0b\87\80\80\80\00"
  "\00\20\00\20\01\72\0b\87\80\80\80\00\00\20\00\20"
  "\01\73\0b\87\80\80\80\00\00\20\00\20\01\74\0b\87"
  "\80\80\80\00\00\20\00\20\01\75\0b\87\80\80\80\00"
  "\00\20\00\20\01\76\0b\87\80\80\80\00\00\20\00\20"
  "\01\77\0b\87\80\80\80\00\00\20\00\20\01\78\0b\85"
  "\80\80\80\00\00\20\00\67\0b\85\80\80\80\00\00\20"
  "\00\68\0b\85\80\80\80\00\00\20\00\69\0b\85\80\80"
  "\80\00\00\20\00\c0\0b\85\80\80\80\00\00\20\00\c1"
  "\0b\85\80\80\80\00\00\20\00\45\0b\87\80\80\80\00"
  "\00\20\00\20\01\46\0b\87\80\80\80\00\00\20\00\20"
  "\01\47\0b\87\80\80\80\00\00\20\00\20\01\48\0b\87"
  "\80\80\80\00\00\20\00\20\01\49\0b\87\80\80\80\00"
  "\00\20\00\20\01\4c\0b\87\80\80\80\00\00\20\00\20"
  "\01\4d\0b\87\80\80\80\00\00\20\00\20\01\4a\0b\87"
  "\80\80\80\00\00\20\00\20\01\4b\0b\87\80\80\80\00"
  "\00\20\00\20\01\4e\0b\87\80\80\80\00\00\20\00\20"
  "\01\4f\0b"
)
(module instance)
(assert_return (invoke "add" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x2))
(assert_return (invoke "add" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x1))
(assert_return
  (invoke "add" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0xffff_fffe)
)
(assert_return
  (invoke "add" (i32.const 0xffff_ffff) (i32.const 0x1))
  (i32.const 0x0)
)
(assert_return
  (invoke "add" (i32.const 0x7fff_ffff) (i32.const 0x1))
  (i32.const 0x8000_0000)
)
(assert_return
  (invoke "add" (i32.const 0x8000_0000) (i32.const 0xffff_ffff))
  (i32.const 0x7fff_ffff)
)
(assert_return
  (invoke "add" (i32.const 0x8000_0000) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "add" (i32.const 0x3fff_ffff) (i32.const 0x1))
  (i32.const 0x4000_0000)
)
(assert_return (invoke "sub" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "sub" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x1))
(assert_return
  (invoke "sub" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "sub" (i32.const 0x7fff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x8000_0000)
)
(assert_return
  (invoke "sub" (i32.const 0x8000_0000) (i32.const 0x1))
  (i32.const 0x7fff_ffff)
)
(assert_return
  (invoke "sub" (i32.const 0x8000_0000) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "sub" (i32.const 0x3fff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x4000_0000)
)
(assert_return (invoke "mul" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "mul" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x0))
(assert_return
  (invoke "mul" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "mul" (i32.const 0x1000_0000) (i32.const 0x1000))
  (i32.const 0x0)
)
(assert_return
  (invoke "mul" (i32.const 0x8000_0000) (i32.const 0x0))
  (i32.const 0x0)
)
(assert_return
  (invoke "mul" (i32.const 0x8000_0000) (i32.const 0xffff_ffff))
  (i32.const 0x8000_0000)
)
(assert_return
  (invoke "mul" (i32.const 0x7fff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x8000_0001)
)
(assert_return
  (invoke "mul" (i32.const 0x123_4567) (i32.const 0x7654_3210))
  (i32.const 0x358e_7470)
)
(assert_return
  (invoke "mul" (i32.const 0x7fff_ffff) (i32.const 0x7fff_ffff))
  (i32.const 0x1)
)
(assert_trap
  (invoke "div_s" (i32.const 0x1) (i32.const 0x0))
  "integer divide by zero"
)
(assert_trap
  (invoke "div_s" (i32.const 0x0) (i32.const 0x0))
  "integer divide by zero"
)
(assert_trap
  (invoke "div_s" (i32.const 0x8000_0000) (i32.const 0xffff_ffff))
  "integer overflow"
)
(assert_trap
  (invoke "div_s" (i32.const 0x8000_0000) (i32.const 0x0))
  "integer divide by zero"
)
(assert_return (invoke "div_s" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "div_s" (i32.const 0x0) (i32.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "div_s" (i32.const 0x0) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "div_s" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "div_s" (i32.const 0x8000_0000) (i32.const 0x2))
  (i32.const 0xc000_0000)
)
(assert_return
  (invoke "div_s" (i32.const 0x8000_0001) (i32.const 0x3e8))
  (i32.const 0xffdf_3b65)
)
(assert_return (invoke "div_s" (i32.const 0x5) (i32.const 0x2)) (i32.const 0x2))
(assert_return
  (invoke "div_s" (i32.const 0xffff_fffb) (i32.const 0x2))
  (i32.const 0xffff_fffe)
)
(assert_return
  (invoke "div_s" (i32.const 0x5) (i32.const 0xffff_fffe))
  (i32.const 0xffff_fffe)
)
(assert_return
  (invoke "div_s" (i32.const 0xffff_fffb) (i32.const 0xffff_fffe))
  (i32.const 0x2)
)
(assert_return (invoke "div_s" (i32.const 0x7) (i32.const 0x3)) (i32.const 0x2))
(assert_return
  (invoke "div_s" (i32.const 0xffff_fff9) (i32.const 0x3))
  (i32.const 0xffff_fffe)
)
(assert_return
  (invoke "div_s" (i32.const 0x7) (i32.const 0xffff_fffd))
  (i32.const 0xffff_fffe)
)
(assert_return
  (invoke "div_s" (i32.const 0xffff_fff9) (i32.const 0xffff_fffd))
  (i32.const 0x2)
)
(assert_return (invoke "div_s" (i32.const 0xb) (i32.const 0x5)) (i32.const 0x2))
(assert_return
  (invoke "div_s" (i32.const 0x11) (i32.const 0x7))
  (i32.const 0x2)
)
(assert_trap
  (invoke "div_u" (i32.const 0x1) (i32.const 0x0))
  "integer divide by zero"
)
(assert_trap
  (invoke "div_u" (i32.const 0x0) (i32.const 0x0))
  "integer divide by zero"
)
(assert_return (invoke "div_u" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "div_u" (i32.const 0x0) (i32.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "div_u" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "div_u" (i32.const 0x8000_0000) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "div_u" (i32.const 0x8000_0000) (i32.const 0x2))
  (i32.const 0x4000_0000)
)
(assert_return
  (invoke "div_u" (i32.const 0x8ff0_0ff0) (i32.const 0x1_0001))
  (i32.const 0x8fef)
)
(assert_return
  (invoke "div_u" (i32.const 0x8000_0001) (i32.const 0x3e8))
  (i32.const 0x20_c49b)
)
(assert_return (invoke "div_u" (i32.const 0x5) (i32.const 0x2)) (i32.const 0x2))
(assert_return
  (invoke "div_u" (i32.const 0xffff_fffb) (i32.const 0x2))
  (i32.const 0x7fff_fffd)
)
(assert_return
  (invoke "div_u" (i32.const 0x5) (i32.const 0xffff_fffe))
  (i32.const 0x0)
)
(assert_return
  (invoke "div_u" (i32.const 0xffff_fffb) (i32.const 0xffff_fffe))
  (i32.const 0x0)
)
(assert_return (invoke "div_u" (i32.const 0x7) (i32.const 0x3)) (i32.const 0x2))
(assert_return (invoke "div_u" (i32.const 0xb) (i32.const 0x5)) (i32.const 0x2))
(assert_return
  (invoke "div_u" (i32.const 0x11) (i32.const 0x7))
  (i32.const 0x2)
)
(assert_trap
  (invoke "rem_s" (i32.const 0x1) (i32.const 0x0))
  "integer divide by zero"
)
(assert_trap
  (invoke "rem_s" (i32.const 0x0) (i32.const 0x0))
  "integer divide by zero"
)
(assert_return
  (invoke "rem_s" (i32.const 0x7fff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return (invoke "rem_s" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "rem_s" (i32.const 0x0) (i32.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "rem_s" (i32.const 0x0) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "rem_s" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "rem_s" (i32.const 0x8000_0000) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "rem_s" (i32.const 0x8000_0000) (i32.const 0x2))
  (i32.const 0x0)
)
(assert_return
  (invoke "rem_s" (i32.const 0x8000_0001) (i32.const 0x3e8))
  (i32.const 0xffff_fd79)
)
(assert_return (invoke "rem_s" (i32.const 0x5) (i32.const 0x2)) (i32.const 0x1))
(assert_return
  (invoke "rem_s" (i32.const 0xffff_fffb) (i32.const 0x2))
  (i32.const 0xffff_ffff)
)
(assert_return
  (invoke "rem_s" (i32.const 0x5) (i32.const 0xffff_fffe))
  (i32.const 0x1)
)
(assert_return
  (invoke "rem_s" (i32.const 0xffff_fffb) (i32.const 0xffff_fffe))
  (i32.const 0xffff_ffff)
)
(assert_return (invoke "rem_s" (i32.const 0x7) (i32.const 0x3)) (i32.const 0x1))
(assert_return
  (invoke "rem_s" (i32.const 0xffff_fff9) (i32.const 0x3))
  (i32.const 0xffff_ffff)
)
(assert_return
  (invoke "rem_s" (i32.const 0x7) (i32.const 0xffff_fffd))
  (i32.const 0x1)
)
(assert_return
  (invoke "rem_s" (i32.const 0xffff_fff9) (i32.const 0xffff_fffd))
  (i32.const 0xffff_ffff)
)
(assert_return (invoke "rem_s" (i32.const 0xb) (i32.const 0x5)) (i32.const 0x1))
(assert_return
  (invoke "rem_s" (i32.const 0x11) (i32.const 0x7))
  (i32.const 0x3)
)
(assert_trap
  (invoke "rem_u" (i32.const 0x1) (i32.const 0x0))
  "integer divide by zero"
)
(assert_trap
  (invoke "rem_u" (i32.const 0x0) (i32.const 0x0))
  "integer divide by zero"
)
(assert_return (invoke "rem_u" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "rem_u" (i32.const 0x0) (i32.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "rem_u" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "rem_u" (i32.const 0x8000_0000) (i32.const 0xffff_ffff))
  (i32.const 0x8000_0000)
)
(assert_return
  (invoke "rem_u" (i32.const 0x8000_0000) (i32.const 0x2))
  (i32.const 0x0)
)
(assert_return
  (invoke "rem_u" (i32.const 0x8ff0_0ff0) (i32.const 0x1_0001))
  (i32.const 0x8001)
)
(assert_return
  (invoke "rem_u" (i32.const 0x8000_0001) (i32.const 0x3e8))
  (i32.const 0x289)
)
(assert_return (invoke "rem_u" (i32.const 0x5) (i32.const 0x2)) (i32.const 0x1))
(assert_return
  (invoke "rem_u" (i32.const 0xffff_fffb) (i32.const 0x2))
  (i32.const 0x1)
)
(assert_return
  (invoke "rem_u" (i32.const 0x5) (i32.const 0xffff_fffe))
  (i32.const 0x5)
)
(assert_return
  (invoke "rem_u" (i32.const 0xffff_fffb) (i32.const 0xffff_fffe))
  (i32.const 0xffff_fffb)
)
(assert_return (invoke "rem_u" (i32.const 0x7) (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "rem_u" (i32.const 0xb) (i32.const 0x5)) (i32.const 0x1))
(assert_return
  (invoke "rem_u" (i32.const 0x11) (i32.const 0x7))
  (i32.const 0x3)
)
(assert_return (invoke "and" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "and" (i32.const 0x0) (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "and" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "and" (i32.const 0x0) (i32.const 0x0)) (i32.const 0x0))
(assert_return
  (invoke "and" (i32.const 0x7fff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "and" (i32.const 0x7fff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x7fff_ffff)
)
(assert_return
  (invoke "and" (i32.const 0xf0f0_ffff) (i32.const 0xffff_f0f0))
  (i32.const 0xf0f0_f0f0)
)
(assert_return
  (invoke "and" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0xffff_ffff)
)
(assert_return (invoke "or" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "or" (i32.const 0x0) (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "or" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "or" (i32.const 0x0) (i32.const 0x0)) (i32.const 0x0))
(assert_return
  (invoke "or" (i32.const 0x7fff_ffff) (i32.const 0x8000_0000))
  (i32.const 0xffff_ffff)
)
(assert_return
  (invoke "or" (i32.const 0x8000_0000) (i32.const 0x0))
  (i32.const 0x8000_0000)
)
(assert_return
  (invoke "or" (i32.const 0xf0f0_ffff) (i32.const 0xffff_f0f0))
  (i32.const 0xffff_ffff)
)
(assert_return
  (invoke "or" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0xffff_ffff)
)
(assert_return (invoke "xor" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "xor" (i32.const 0x0) (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "xor" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "xor" (i32.const 0x0) (i32.const 0x0)) (i32.const 0x0))
(assert_return
  (invoke "xor" (i32.const 0x7fff_ffff) (i32.const 0x8000_0000))
  (i32.const 0xffff_ffff)
)
(assert_return
  (invoke "xor" (i32.const 0x8000_0000) (i32.const 0x0))
  (i32.const 0x8000_0000)
)
(assert_return
  (invoke "xor" (i32.const 0xffff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x7fff_ffff)
)
(assert_return
  (invoke "xor" (i32.const 0xffff_ffff) (i32.const 0x7fff_ffff))
  (i32.const 0x8000_0000)
)
(assert_return
  (invoke "xor" (i32.const 0xf0f0_ffff) (i32.const 0xffff_f0f0))
  (i32.const 0xf0f_0f0f)
)
(assert_return
  (invoke "xor" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return (invoke "shl" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x2))
(assert_return (invoke "shl" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x1))
(assert_return
  (invoke "shl" (i32.const 0x7fff_ffff) (i32.const 0x1))
  (i32.const 0xffff_fffe)
)
(assert_return
  (invoke "shl" (i32.const 0xffff_ffff) (i32.const 0x1))
  (i32.const 0xffff_fffe)
)
(assert_return
  (invoke "shl" (i32.const 0x8000_0000) (i32.const 0x1))
  (i32.const 0x0)
)
(assert_return
  (invoke "shl" (i32.const 0x4000_0000) (i32.const 0x1))
  (i32.const 0x8000_0000)
)
(assert_return
  (invoke "shl" (i32.const 0x1) (i32.const 0x1f))
  (i32.const 0x8000_0000)
)
(assert_return (invoke "shl" (i32.const 0x1) (i32.const 0x20)) (i32.const 0x1))
(assert_return (invoke "shl" (i32.const 0x1) (i32.const 0x21)) (i32.const 0x2))
(assert_return
  (invoke "shl" (i32.const 0x1) (i32.const 0xffff_ffff))
  (i32.const 0x8000_0000)
)
(assert_return
  (invoke "shl" (i32.const 0x1) (i32.const 0x7fff_ffff))
  (i32.const 0x8000_0000)
)
(assert_return (invoke "shr_s" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "shr_s" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x1))
(assert_return
  (invoke "shr_s" (i32.const 0xffff_ffff) (i32.const 0x1))
  (i32.const 0xffff_ffff)
)
(assert_return
  (invoke "shr_s" (i32.const 0x7fff_ffff) (i32.const 0x1))
  (i32.const 0x3fff_ffff)
)
(assert_return
  (invoke "shr_s" (i32.const 0x8000_0000) (i32.const 0x1))
  (i32.const 0xc000_0000)
)
(assert_return
  (invoke "shr_s" (i32.const 0x4000_0000) (i32.const 0x1))
  (i32.const 0x2000_0000)
)
(assert_return
  (invoke "shr_s" (i32.const 0x1) (i32.const 0x20))
  (i32.const 0x1)
)
(assert_return
  (invoke "shr_s" (i32.const 0x1) (i32.const 0x21))
  (i32.const 0x0)
)
(assert_return
  (invoke "shr_s" (i32.const 0x1) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "shr_s" (i32.const 0x1) (i32.const 0x7fff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "shr_s" (i32.const 0x1) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "shr_s" (i32.const 0x8000_0000) (i32.const 0x1f))
  (i32.const 0xffff_ffff)
)
(assert_return
  (invoke "shr_s" (i32.const 0xffff_ffff) (i32.const 0x20))
  (i32.const 0xffff_ffff)
)
(assert_return
  (invoke "shr_s" (i32.const 0xffff_ffff) (i32.const 0x21))
  (i32.const 0xffff_ffff)
)
(assert_return
  (invoke "shr_s" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0xffff_ffff)
)
(assert_return
  (invoke "shr_s" (i32.const 0xffff_ffff) (i32.const 0x7fff_ffff))
  (i32.const 0xffff_ffff)
)
(assert_return
  (invoke "shr_s" (i32.const 0xffff_ffff) (i32.const 0x8000_0000))
  (i32.const 0xffff_ffff)
)
(assert_return (invoke "shr_u" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "shr_u" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x1))
(assert_return
  (invoke "shr_u" (i32.const 0xffff_ffff) (i32.const 0x1))
  (i32.const 0x7fff_ffff)
)
(assert_return
  (invoke "shr_u" (i32.const 0x7fff_ffff) (i32.const 0x1))
  (i32.const 0x3fff_ffff)
)
(assert_return
  (invoke "shr_u" (i32.const 0x8000_0000) (i32.const 0x1))
  (i32.const 0x4000_0000)
)
(assert_return
  (invoke "shr_u" (i32.const 0x4000_0000) (i32.const 0x1))
  (i32.const 0x2000_0000)
)
(assert_return
  (invoke "shr_u" (i32.const 0x1) (i32.const 0x20))
  (i32.const 0x1)
)
(assert_return
  (invoke "shr_u" (i32.const 0x1) (i32.const 0x21))
  (i32.const 0x0)
)
(assert_return
  (invoke "shr_u" (i32.const 0x1) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "shr_u" (i32.const 0x1) (i32.const 0x7fff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "shr_u" (i32.const 0x1) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "shr_u" (i32.const 0x8000_0000) (i32.const 0x1f))
  (i32.const 0x1)
)
(assert_return
  (invoke "shr_u" (i32.const 0xffff_ffff) (i32.const 0x20))
  (i32.const 0xffff_ffff)
)
(assert_return
  (invoke "shr_u" (i32.const 0xffff_ffff) (i32.const 0x21))
  (i32.const 0x7fff_ffff)
)
(assert_return
  (invoke "shr_u" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "shr_u" (i32.const 0xffff_ffff) (i32.const 0x7fff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "shr_u" (i32.const 0xffff_ffff) (i32.const 0x8000_0000))
  (i32.const 0xffff_ffff)
)
(assert_return (invoke "rotl" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x2))
(assert_return (invoke "rotl" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x1))
(assert_return
  (invoke "rotl" (i32.const 0xffff_ffff) (i32.const 0x1))
  (i32.const 0xffff_ffff)
)
(assert_return (invoke "rotl" (i32.const 0x1) (i32.const 0x20)) (i32.const 0x1))
(assert_return
  (invoke "rotl" (i32.const 0xabcd_9876) (i32.const 0x1))
  (i32.const 0x579b_30ed)
)
(assert_return
  (invoke "rotl" (i32.const 0xfe00_dc00) (i32.const 0x4))
  (i32.const 0xe00d_c00f)
)
(assert_return
  (invoke "rotl" (i32.const 0xb0c1_d2e3) (i32.const 0x5))
  (i32.const 0x183a_5c76)
)
(assert_return
  (invoke "rotl" (i32.const 0x8000) (i32.const 0x25))
  (i32.const 0x10_0000)
)
(assert_return
  (invoke "rotl" (i32.const 0xb0c1_d2e3) (i32.const 0xff05))
  (i32.const 0x183a_5c76)
)
(assert_return
  (invoke "rotl" (i32.const 0x769a_bcdf) (i32.const 0xffff_ffed))
  (i32.const 0x579b_eed3)
)
(assert_return
  (invoke "rotl" (i32.const 0x769a_bcdf) (i32.const 0x8000_000d))
  (i32.const 0x579b_eed3)
)
(assert_return
  (invoke "rotl" (i32.const 0x1) (i32.const 0x1f))
  (i32.const 0x8000_0000)
)
(assert_return
  (invoke "rotl" (i32.const 0x8000_0000) (i32.const 0x1))
  (i32.const 0x1)
)
(assert_return
  (invoke "rotr" (i32.const 0x1) (i32.const 0x1))
  (i32.const 0x8000_0000)
)
(assert_return (invoke "rotr" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x1))
(assert_return
  (invoke "rotr" (i32.const 0xffff_ffff) (i32.const 0x1))
  (i32.const 0xffff_ffff)
)
(assert_return (invoke "rotr" (i32.const 0x1) (i32.const 0x20)) (i32.const 0x1))
(assert_return
  (invoke "rotr" (i32.const 0xff00_cc00) (i32.const 0x1))
  (i32.const 0x7f80_6600)
)
(assert_return
  (invoke "rotr" (i32.const 0x8_0000) (i32.const 0x4))
  (i32.const 0x8000)
)
(assert_return
  (invoke "rotr" (i32.const 0xb0c1_d2e3) (i32.const 0x5))
  (i32.const 0x1d86_0e97)
)
(assert_return
  (invoke "rotr" (i32.const 0x8000) (i32.const 0x25))
  (i32.const 0x400)
)
(assert_return
  (invoke "rotr" (i32.const 0xb0c1_d2e3) (i32.const 0xff05))
  (i32.const 0x1d86_0e97)
)
(assert_return
  (invoke "rotr" (i32.const 0x769a_bcdf) (i32.const 0xffff_ffed))
  (i32.const 0xe6fb_b4d5)
)
(assert_return
  (invoke "rotr" (i32.const 0x769a_bcdf) (i32.const 0x8000_000d))
  (i32.const 0xe6fb_b4d5)
)
(assert_return (invoke "rotr" (i32.const 0x1) (i32.const 0x1f)) (i32.const 0x2))
(assert_return
  (invoke "rotr" (i32.const 0x8000_0000) (i32.const 0x1f))
  (i32.const 0x1)
)
(assert_return (invoke "clz" (i32.const 0xffff_ffff)) (i32.const 0x0))
(assert_return (invoke "clz" (i32.const 0x0)) (i32.const 0x20))
(assert_return (invoke "clz" (i32.const 0x8000)) (i32.const 0x10))
(assert_return (invoke "clz" (i32.const 0xff)) (i32.const 0x18))
(assert_return (invoke "clz" (i32.const 0x8000_0000)) (i32.const 0x0))
(assert_return (invoke "clz" (i32.const 0x1)) (i32.const 0x1f))
(assert_return (invoke "clz" (i32.const 0x2)) (i32.const 0x1e))
(assert_return (invoke "clz" (i32.const 0x7fff_ffff)) (i32.const 0x1))
(assert_return (invoke "ctz" (i32.const 0xffff_ffff)) (i32.const 0x0))
(assert_return (invoke "ctz" (i32.const 0x0)) (i32.const 0x20))
(assert_return (invoke "ctz" (i32.const 0x8000)) (i32.const 0xf))
(assert_return (invoke "ctz" (i32.const 0x1_0000)) (i32.const 0x10))
(assert_return (invoke "ctz" (i32.const 0x8000_0000)) (i32.const 0x1f))
(assert_return (invoke "ctz" (i32.const 0x7fff_ffff)) (i32.const 0x0))
(assert_return (invoke "popcnt" (i32.const 0xffff_ffff)) (i32.const 0x20))
(assert_return (invoke "popcnt" (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "popcnt" (i32.const 0x8000)) (i32.const 0x1))
(assert_return (invoke "popcnt" (i32.const 0x8000_8000)) (i32.const 0x2))
(assert_return (invoke "popcnt" (i32.const 0x7fff_ffff)) (i32.const 0x1f))
(assert_return (invoke "popcnt" (i32.const 0xaaaa_aaaa)) (i32.const 0x10))
(assert_return (invoke "popcnt" (i32.const 0x5555_5555)) (i32.const 0x10))
(assert_return (invoke "popcnt" (i32.const 0xdead_beef)) (i32.const 0x18))
(assert_return (invoke "extend8_s" (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "extend8_s" (i32.const 0x7f)) (i32.const 0x7f))
(assert_return (invoke "extend8_s" (i32.const 0x80)) (i32.const 0xffff_ff80))
(assert_return (invoke "extend8_s" (i32.const 0xff)) (i32.const 0xffff_ffff))
(assert_return (invoke "extend8_s" (i32.const 0x123_4500)) (i32.const 0x0))
(assert_return
  (invoke "extend8_s" (i32.const 0xfedc_ba80))
  (i32.const 0xffff_ff80)
)
(assert_return
  (invoke "extend8_s" (i32.const 0xffff_ffff))
  (i32.const 0xffff_ffff)
)
(assert_return (invoke "extend16_s" (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "extend16_s" (i32.const 0x7fff)) (i32.const 0x7fff))
(assert_return (invoke "extend16_s" (i32.const 0x8000)) (i32.const 0xffff_8000))
(assert_return (invoke "extend16_s" (i32.const 0xffff)) (i32.const 0xffff_ffff))
(assert_return (invoke "extend16_s" (i32.const 0x123_0000)) (i32.const 0x0))
(assert_return
  (invoke "extend16_s" (i32.const 0xfedc_8000))
  (i32.const 0xffff_8000)
)
(assert_return
  (invoke "extend16_s" (i32.const 0xffff_ffff))
  (i32.const 0xffff_ffff)
)
(assert_return (invoke "eqz" (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "eqz" (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "eqz" (i32.const 0x8000_0000)) (i32.const 0x0))
(assert_return (invoke "eqz" (i32.const 0x7fff_ffff)) (i32.const 0x0))
(assert_return (invoke "eqz" (i32.const 0xffff_ffff)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x0) (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "eq" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "eq" (i32.const 0xffff_ffff) (i32.const 0x1))
  (i32.const 0x0)
)
(assert_return
  (invoke "eq" (i32.const 0x8000_0000) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "eq" (i32.const 0x7fff_ffff) (i32.const 0x7fff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "eq" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x1)
)
(assert_return (invoke "eq" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x0) (i32.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "eq" (i32.const 0x8000_0000) (i32.const 0x0))
  (i32.const 0x0)
)
(assert_return
  (invoke "eq" (i32.const 0x0) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "eq" (i32.const 0x8000_0000) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "eq" (i32.const 0xffff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "eq" (i32.const 0x8000_0000) (i32.const 0x7fff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "eq" (i32.const 0x7fff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return (invoke "ne" (i32.const 0x0) (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "ne" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "ne" (i32.const 0xffff_ffff) (i32.const 0x1))
  (i32.const 0x1)
)
(assert_return
  (invoke "ne" (i32.const 0x8000_0000) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "ne" (i32.const 0x7fff_ffff) (i32.const 0x7fff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "ne" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return (invoke "ne" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ne" (i32.const 0x0) (i32.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "ne" (i32.const 0x8000_0000) (i32.const 0x0))
  (i32.const 0x1)
)
(assert_return
  (invoke "ne" (i32.const 0x0) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "ne" (i32.const 0x8000_0000) (i32.const 0xffff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "ne" (i32.const 0xffff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "ne" (i32.const 0x8000_0000) (i32.const 0x7fff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "ne" (i32.const 0x7fff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return (invoke "lt_s" (i32.const 0x0) (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "lt_s" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "lt_s" (i32.const 0xffff_ffff) (i32.const 0x1))
  (i32.const 0x1)
)
(assert_return
  (invoke "lt_s" (i32.const 0x8000_0000) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_s" (i32.const 0x7fff_ffff) (i32.const 0x7fff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_s" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return (invoke "lt_s" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "lt_s" (i32.const 0x0) (i32.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "lt_s" (i32.const 0x8000_0000) (i32.const 0x0))
  (i32.const 0x1)
)
(assert_return
  (invoke "lt_s" (i32.const 0x0) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_s" (i32.const 0x8000_0000) (i32.const 0xffff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "lt_s" (i32.const 0xffff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_s" (i32.const 0x8000_0000) (i32.const 0x7fff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "lt_s" (i32.const 0x7fff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return (invoke "lt_u" (i32.const 0x0) (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "lt_u" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "lt_u" (i32.const 0xffff_ffff) (i32.const 0x1))
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_u" (i32.const 0x8000_0000) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_u" (i32.const 0x7fff_ffff) (i32.const 0x7fff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_u" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return (invoke "lt_u" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "lt_u" (i32.const 0x0) (i32.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "lt_u" (i32.const 0x8000_0000) (i32.const 0x0))
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_u" (i32.const 0x0) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "lt_u" (i32.const 0x8000_0000) (i32.const 0xffff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "lt_u" (i32.const 0xffff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_u" (i32.const 0x8000_0000) (i32.const 0x7fff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "lt_u" (i32.const 0x7fff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return (invoke "le_s" (i32.const 0x0) (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "le_s" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "le_s" (i32.const 0xffff_ffff) (i32.const 0x1))
  (i32.const 0x1)
)
(assert_return
  (invoke "le_s" (i32.const 0x8000_0000) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "le_s" (i32.const 0x7fff_ffff) (i32.const 0x7fff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "le_s" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x1)
)
(assert_return (invoke "le_s" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "le_s" (i32.const 0x0) (i32.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "le_s" (i32.const 0x8000_0000) (i32.const 0x0))
  (i32.const 0x1)
)
(assert_return
  (invoke "le_s" (i32.const 0x0) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "le_s" (i32.const 0x8000_0000) (i32.const 0xffff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "le_s" (i32.const 0xffff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "le_s" (i32.const 0x8000_0000) (i32.const 0x7fff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "le_s" (i32.const 0x7fff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return (invoke "le_u" (i32.const 0x0) (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "le_u" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "le_u" (i32.const 0xffff_ffff) (i32.const 0x1))
  (i32.const 0x0)
)
(assert_return
  (invoke "le_u" (i32.const 0x8000_0000) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "le_u" (i32.const 0x7fff_ffff) (i32.const 0x7fff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "le_u" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x1)
)
(assert_return (invoke "le_u" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "le_u" (i32.const 0x0) (i32.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "le_u" (i32.const 0x8000_0000) (i32.const 0x0))
  (i32.const 0x0)
)
(assert_return
  (invoke "le_u" (i32.const 0x0) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "le_u" (i32.const 0x8000_0000) (i32.const 0xffff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "le_u" (i32.const 0xffff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "le_u" (i32.const 0x8000_0000) (i32.const 0x7fff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "le_u" (i32.const 0x7fff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return (invoke "gt_s" (i32.const 0x0) (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "gt_s" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "gt_s" (i32.const 0xffff_ffff) (i32.const 0x1))
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_s" (i32.const 0x8000_0000) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_s" (i32.const 0x7fff_ffff) (i32.const 0x7fff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_s" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return (invoke "gt_s" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "gt_s" (i32.const 0x0) (i32.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "gt_s" (i32.const 0x8000_0000) (i32.const 0x0))
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_s" (i32.const 0x0) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "gt_s" (i32.const 0x8000_0000) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_s" (i32.const 0xffff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "gt_s" (i32.const 0x8000_0000) (i32.const 0x7fff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_s" (i32.const 0x7fff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return (invoke "gt_u" (i32.const 0x0) (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "gt_u" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "gt_u" (i32.const 0xffff_ffff) (i32.const 0x1))
  (i32.const 0x1)
)
(assert_return
  (invoke "gt_u" (i32.const 0x8000_0000) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_u" (i32.const 0x7fff_ffff) (i32.const 0x7fff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_u" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return (invoke "gt_u" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "gt_u" (i32.const 0x0) (i32.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "gt_u" (i32.const 0x8000_0000) (i32.const 0x0))
  (i32.const 0x1)
)
(assert_return
  (invoke "gt_u" (i32.const 0x0) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_u" (i32.const 0x8000_0000) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "gt_u" (i32.const 0xffff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "gt_u" (i32.const 0x8000_0000) (i32.const 0x7fff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "gt_u" (i32.const 0x7fff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return (invoke "ge_s" (i32.const 0x0) (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ge_s" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "ge_s" (i32.const 0xffff_ffff) (i32.const 0x1))
  (i32.const 0x0)
)
(assert_return
  (invoke "ge_s" (i32.const 0x8000_0000) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_s" (i32.const 0x7fff_ffff) (i32.const 0x7fff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_s" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x1)
)
(assert_return (invoke "ge_s" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ge_s" (i32.const 0x0) (i32.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "ge_s" (i32.const 0x8000_0000) (i32.const 0x0))
  (i32.const 0x0)
)
(assert_return
  (invoke "ge_s" (i32.const 0x0) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_s" (i32.const 0x8000_0000) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "ge_s" (i32.const 0xffff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_s" (i32.const 0x8000_0000) (i32.const 0x7fff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "ge_s" (i32.const 0x7fff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return (invoke "ge_u" (i32.const 0x0) (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ge_u" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x1))
(assert_return
  (invoke "ge_u" (i32.const 0xffff_ffff) (i32.const 0x1))
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_u" (i32.const 0x8000_0000) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_u" (i32.const 0x7fff_ffff) (i32.const 0x7fff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_u" (i32.const 0xffff_ffff) (i32.const 0xffff_ffff))
  (i32.const 0x1)
)
(assert_return (invoke "ge_u" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ge_u" (i32.const 0x0) (i32.const 0x1)) (i32.const 0x0))
(assert_return
  (invoke "ge_u" (i32.const 0x8000_0000) (i32.const 0x0))
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_u" (i32.const 0x0) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_return
  (invoke "ge_u" (i32.const 0x8000_0000) (i32.const 0xffff_ffff))
  (i32.const 0x0)
)
(assert_return
  (invoke "ge_u" (i32.const 0xffff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_u" (i32.const 0x8000_0000) (i32.const 0x7fff_ffff))
  (i32.const 0x1)
)
(assert_return
  (invoke "ge_u" (i32.const 0x7fff_ffff) (i32.const 0x8000_0000))
  (i32.const 0x0)
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\8a\80\80\80\00"
    "\01\84\80\80\80\00\00\45\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\8f\80\80\80\00"
    "\01\89\80\80\80\00\00\41\00\02\40\45\1a\0b\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\8f\80\80\80\00"
    "\01\89\80\80\80\00\00\41\00\03\40\45\1a\0b\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\91\80\80\80\00"
    "\01\8b\80\80\80\00\00\41\00\41\00\04\40\45\1a\0b"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\94\80\80\80\00"
    "\01\8e\80\80\80\00\00\41\00\41\00\04\7f\41\00\05"
    "\45\0b\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\91\80\80\80\00"
    "\01\8b\80\80\80\00\00\41\00\02\40\45\0c\00\1a\0b"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\93\80\80\80\00"
    "\01\8d\80\80\80\00\00\41\00\02\40\45\41\01\0d\00"
    "\1a\0b\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\92\80\80\80\00"
    "\01\8c\80\80\80\00\00\41\00\02\40\45\0e\00\00\1a"
    "\0b\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\8b\80\80\80\00"
    "\01\85\80\80\80\00\00\45\0f\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\8f\80\80\80\00"
    "\01\89\80\80\80\00\00\45\41\01\41\02\1b\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\60"
    "\00\00\60\01\7f\01\7f\03\83\80\80\80\00\02\00\01"
    "\0a\95\80\80\80\00\02\86\80\80\80\00\00\45\10\01"
    "\1a\0b\84\80\80\80\00\00\20\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\60"
    "\01\7f\01\7f\60\00\00\03\83\80\80\80\00\02\00\01"
    "\04\85\80\80\80\00\01\70\01\01\01\09\89\80\80\80"
    "\00\01\04\41\00\0b\01\d2\00\0b\0a\9b\80\80\80\00"
    "\02\84\80\80\80\00\00\20\00\0b\8c\80\80\80\00\00"
    "\02\7f\45\41\00\11\00\00\1a\0b\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\90\80\80\80\00"
    "\01\8a\80\80\80\00\01\01\7f\45\21\00\20\00\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\8e\80\80\80\00"
    "\01\88\80\80\80\00\01\01\7f\45\22\00\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\06\86\80\80\80\00"
    "\01\7f\01\41\00\0b\0a\8e\80\80\80\00\01\88\80\80"
    "\80\00\00\45\24\00\23\00\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\05\83\80\80\80\00"
    "\01\00\00\0a\8c\80\80\80\00\01\86\80\80\80\00\00"
    "\45\40\00\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\05\83\80\80\80\00"
    "\01\00\00\0a\8d\80\80\80\00\01\87\80\80\80\00\00"
    "\45\28\02\00\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\05\83\80\80\80\00"
    "\01\00\01\0a\8e\80\80\80\00\01\88\80\80\80\00\00"
    "\45\41\01\36\02\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\8a\80\80\80\00"
    "\01\84\80\80\80\00\00\6a\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\8c\80\80\80\00"
    "\01\86\80\80\80\00\00\41\00\6a\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\91\80\80\80\00"
    "\01\8b\80\80\80\00\00\41\00\41\00\02\40\6a\1a\0b"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\91\80\80\80\00"
    "\01\8b\80\80\80\00\00\41\00\02\40\41\00\6a\1a\0b"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\91\80\80\80\00"
    "\01\8b\80\80\80\00\00\41\00\41\00\03\40\6a\1a\0b"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\91\80\80\80\00"
    "\01\8b\80\80\80\00\00\41\00\03\40\41\00\6a\1a\0b"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\93\80\80\80\00"
    "\01\8d\80\80\80\00\00\41\00\41\00\41\00\6a\04\40"
    "\1a\0b\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\94\80\80\80\00"
    "\01\8e\80\80\80\00\00\41\00\41\00\41\00\04\40\6a"
    "\05\1a\0b\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\99\80\80\80\00"
    "\01\93\80\80\80\00\00\41\00\41\00\41\00\04\7f\41"
    "\00\05\6a\41\00\0b\1a\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\94\80\80\80\00"
    "\01\8e\80\80\80\00\00\41\00\41\00\04\7f\41\00\05"
    "\6a\0b\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\93\80\80\80\00"
    "\01\8d\80\80\80\00\00\41\00\41\00\02\40\6a\0c\00"
    "\1a\0b\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\93\80\80\80\00"
    "\01\8d\80\80\80\00\00\41\00\02\40\41\00\6a\0c\00"
    "\1a\0b\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\95\80\80\80\00"
    "\01\8f\80\80\80\00\00\41\00\41\00\02\40\6a\41\01"
    "\0d\00\1a\0b\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\95\80\80\80\00"
    "\01\8f\80\80\80\00\00\41\00\02\40\41\00\6a\41\01"
    "\0d\00\1a\0b\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\94\80\80\80\00"
    "\01\8e\80\80\80\00\00\41\00\41\00\02\40\6a\0e\00"
    "\00\1a\0b\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\94\80\80\80\00"
    "\01\8e\80\80\80\00\00\41\00\02\40\41\00\6a\0e\00"
    "\00\1a\0b\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\8b\80\80\80\00"
    "\01\85\80\80\80\00\00\6a\0f\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\8d\80\80\80\00"
    "\01\87\80\80\80\00\00\41\00\6a\0f\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\8f\80\80\80\00"
    "\01\89\80\80\80\00\00\6a\41\01\41\02\1b\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\91\80\80\80\00"
    "\01\8b\80\80\80\00\00\41\00\6a\41\01\41\02\1b\1a"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\8a\80\80\80\00\02\60"
    "\00\00\60\02\7f\7f\01\7f\03\83\80\80\80\00\02\00"
    "\01\0a\95\80\80\80\00\02\86\80\80\80\00\00\6a\10"
    "\01\1a\0b\84\80\80\80\00\00\20\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\8a\80\80\80\00\02\60"
    "\00\00\60\02\7f\7f\01\7f\03\83\80\80\80\00\02\00"
    "\01\0a\97\80\80\80\00\02\88\80\80\80\00\00\41\00"
    "\6a\10\01\1a\0b\84\80\80\80\00\00\20\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\60"
    "\01\7f\01\7f\60\00\00\03\83\80\80\80\00\02\00\01"
    "\04\85\80\80\80\00\01\70\01\01\01\09\89\80\80\80"
    "\00\01\04\41\00\0b\01\d2\00\0b\0a\9b\80\80\80\00"
    "\02\84\80\80\80\00\00\20\00\0b\8c\80\80\80\00\00"
    "\02\7f\6a\41\00\11\00\00\1a\0b\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\60"
    "\01\7f\01\7f\60\00\00\03\83\80\80\80\00\02\00\01"
    "\04\85\80\80\80\00\01\70\01\01\01\09\89\80\80\80"
    "\00\01\04\41\00\0b\01\d2\00\0b\0a\9d\80\80\80\00"
    "\02\84\80\80\80\00\00\20\00\0b\8e\80\80\80\00\00"
    "\02\7f\41\00\6a\41\00\11\00\00\1a\0b\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\90\80\80\80\00"
    "\01\8a\80\80\80\00\01\01\7f\6a\21\00\20\00\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\92\80\80\80\00"
    "\01\8c\80\80\80\00\01\01\7f\41\00\6a\21\00\20\00"
    "\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\8e\80\80\80\00"
    "\01\88\80\80\80\00\01\01\7f\6a\22\00\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\90\80\80\80\00"
    "\01\8a\80\80\80\00\01\01\7f\41\00\6a\22\00\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\06\86\80\80\80\00"
    "\01\7f\01\41\00\0b\0a\8e\80\80\80\00\01\88\80\80"
    "\80\00\00\6a\24\00\23\00\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\06\86\80\80\80\00"
    "\01\7f\01\41\00\0b\0a\90\80\80\80\00\01\8a\80\80"
    "\80\00\00\41\00\6a\24\00\23\00\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\05\83\80\80\80\00"
    "\01\00\00\0a\8c\80\80\80\00\01\86\80\80\80\00\00"
    "\6a\40\00\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\05\83\80\80\80\00"
    "\01\00\00\0a\8e\80\80\80\00\01\88\80\80\80\00\00"
    "\41\00\6a\40\00\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\05\83\80\80\80\00"
    "\01\00\00\0a\8d\80\80\80\00\01\87\80\80\80\00\00"
    "\6a\28\02\00\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\05\83\80\80\80\00"
    "\01\00\00\0a\8f\80\80\80\00\01\89\80\80\80\00\00"
    "\41\00\6a\28\02\00\1a\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\05\83\80\80\80\00"
    "\01\00\01\0a\8e\80\80\80\00\01\88\80\80\80\00\00"
    "\6a\41\01\36\02\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\05\83\80\80\80\00"
    "\01\00\01\0a\90\80\80\80\00\01\8a\80\80\80\00\00"
    "\41\01\6a\41\00\36\02\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\6a"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\71"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\6d"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\6e"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\6c"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\72"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\6f"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\70"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\77"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\78"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\74"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\75"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\76"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\6b"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\73"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\8b\80\80\80"
    "\00\01\85\80\80\80\00\00\42\00\45\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\8b\80\80\80"
    "\00\01\85\80\80\80\00\00\42\00\67\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\8b\80\80\80"
    "\00\01\85\80\80\80\00\00\42\00\68\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\8b\80\80\80"
    "\00\01\85\80\80\80\00\00\42\00\69\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\46"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\4e"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\4f"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\4a"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\4b"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\4c"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\4d"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\48"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\49"
    "\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\0a\90\80\80\80"
    "\00\01\8a\80\80\80\00\00\42\00\43\00\00\00\00\47"
    "\0b"
  )
  "type mismatch"
)
