(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\03\60"
  "\01\7f\01\7f\60\01\7e\01\7e\60\00\01\7f\03\85\80"
  "\80\80\00\04\00\01\00\02\07\9e\80\80\80\00\04\04"
  "\73\74\6d\74\00\00\04\65\78\70\72\00\01\03\61\72"
  "\67\00\02\06\63\6f\72\6e\65\72\00\03\0a\ee\81\80"
  "\80\00\04\d7\80\80\80\00\01\01\7f\41\e4\00\21\01"
  "\02\40\02\40\02\40\02\40\02\40\02\40\02\40\02\40"
  "\02\40\02\40\20\00\0e\08\00\01\02\03\04\05\06\08"
  "\07\0b\20\00\0f\0b\01\0b\0b\41\00\20\00\6b\21\01"
  "\0c\05\0b\0c\04\0b\41\e5\00\21\01\0c\03\0b\41\e5"
  "\00\21\01\0b\41\e6\00\21\01\0b\0b\20\01\0f\0b\cc"
  "\80\80\80\00\01\01\7e\42\e4\00\21\01\02\7e\02\40"
  "\02\40\02\40\02\40\02\40\02\40\02\40\02\40\02\40"
  "\20\00\a7\0e\08\00\01\02\03\06\05\04\08\07\0b\20"
  "\00\0f\0b\01\0b\0b\42\00\20\00\7d\0c\05\0b\42\e5"
  "\00\21\01\0b\0b\0b\20\01\0c\01\0b\42\7b\0b\0f\0b"
  "\aa\80\80\80\00\00\02\7f\41\0a\02\7f\41\e4\00\02"
  "\7f\41\e8\07\02\7f\41\02\20\00\6c\41\03\20\00\71"
  "\0e\03\01\02\03\00\0b\6a\0b\6a\0b\6a\0b\0f\0b\8c"
  "\80\80\80\00\00\02\40\41\00\0e\00\00\0b\41\01\0b"
)
(module instance)
(assert_return (invoke "stmt" (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "stmt" (i32.const 0x1)) (i32.const 0xffff_ffff))
(assert_return (invoke "stmt" (i32.const 0x2)) (i32.const 0xffff_fffe))
(assert_return (invoke "stmt" (i32.const 0x3)) (i32.const 0xffff_fffd))
(assert_return (invoke "stmt" (i32.const 0x4)) (i32.const 0x64))
(assert_return (invoke "stmt" (i32.const 0x5)) (i32.const 0x65))
(assert_return (invoke "stmt" (i32.const 0x6)) (i32.const 0x66))
(assert_return (invoke "stmt" (i32.const 0x7)) (i32.const 0x64))
(assert_return (invoke "stmt" (i32.const 0xffff_fff6)) (i32.const 0x66))
(assert_return (invoke "expr" (i64.const 0x0)) (i64.const 0x0))
(assert_return
  (invoke "expr" (i64.const 0x1))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "expr" (i64.const 0x2))
  (i64.const 0xffff_ffff_ffff_fffe)
)
(assert_return
  (invoke "expr" (i64.const 0x3))
  (i64.const 0xffff_ffff_ffff_fffd)
)
(assert_return (invoke "expr" (i64.const 0x6)) (i64.const 0x65))
(assert_return
  (invoke "expr" (i64.const 0x7))
  (i64.const 0xffff_ffff_ffff_fffb)
)
(assert_return
  (invoke "expr" (i64.const 0xffff_ffff_ffff_fff6))
  (i64.const 0x64)
)
(assert_return (invoke "arg" (i32.const 0x0)) (i32.const 0x6e))
(assert_return (invoke "arg" (i32.const 0x1)) (i32.const 0xc))
(assert_return (invoke "arg" (i32.const 0x2)) (i32.const 0x4))
(assert_return (invoke "arg" (i32.const 0x3)) (i32.const 0x45c))
(assert_return (invoke "arg" (i32.const 0x4)) (i32.const 0x76))
(assert_return (invoke "arg" (i32.const 0x5)) (i32.const 0x14))
(assert_return (invoke "arg" (i32.const 0x6)) (i32.const 0xc))
(assert_return (invoke "arg" (i32.const 0x7)) (i32.const 0x464))
(assert_return (invoke "arg" (i32.const 0x8)) (i32.const 0x7e))
(assert_return (invoke "corner") (i32.const 0x1))
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\8d\80\80\80\00"
    "\01\87\80\80\80\00\00\41\00\0e\00\03\0b"
  )
  "unknown label"
)
