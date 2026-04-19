(module definition binary
  "\00\61\73\6d\01\00\00\00\05\83\80\80\80\00\01\04"
  "\01\0b\86\80\80\80\00\01\01\03\66\6f\6f"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8c\80\80\80\00\02\60"
  "\03\7e\7f\7e\00\60\01\7e\01\7f\03\83\80\80\80\00"
  "\02\00\01\05\83\80\80\80\00\01\04\01\07\92\80\80"
  "\80\00\02\04\66\69\6c\6c\00\00\07\6c\6f\61\64\38"
  "\5f\75\00\01\0a\9d\80\80\80\00\02\8b\80\80\80\00"
  "\00\20\00\20\01\20\02\fc\0b\00\0b\87\80\80\80\00"
  "\00\20\00\2d\00\00\0b"
)
(module instance)
(invoke "fill" (i64.const 0x1) (i32.const 0xff) (i64.const 0x3))
(assert_return (invoke "load8_u" (i64.const 0x0)) (i32.const 0x0))
(assert_return (invoke "load8_u" (i64.const 0x1)) (i32.const 0xff))
(assert_return (invoke "load8_u" (i64.const 0x2)) (i32.const 0xff))
(assert_return (invoke "load8_u" (i64.const 0x3)) (i32.const 0xff))
(assert_return (invoke "load8_u" (i64.const 0x4)) (i32.const 0x0))
(invoke "fill" (i64.const 0x0) (i32.const 0xbbaa) (i64.const 0x2))
(assert_return (invoke "load8_u" (i64.const 0x0)) (i32.const 0xaa))
(assert_return (invoke "load8_u" (i64.const 0x1)) (i32.const 0xaa))
(invoke "fill" (i64.const 0x0) (i32.const 0x0) (i64.const 0x1_0000))
(invoke "fill" (i64.const 0x1_0000) (i32.const 0x0) (i64.const 0x0))
(assert_trap
  (invoke "fill" (i64.const 0x1_0001) (i32.const 0x0) (i64.const 0x0))
  "out of bounds"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8c\80\80\80\00\02\60"
  "\03\7e\7e\7e\00\60\01\7e\01\7f\03\83\80\80\80\00"
  "\02\00\01\05\84\80\80\80\00\01\05\01\01\07\92\80"
  "\80\80\00\02\04\63\6f\70\79\00\00\07\6c\6f\61\64"
  "\38\5f\75\00\01\0a\9e\80\80\80\00\02\8c\80\80\80"
  "\00\00\20\00\20\01\20\02\fc\0a\00\00\0b\87\80\80"
  "\80\00\00\20\00\2d\00\00\0b\0b\8a\80\80\80\00\01"
  "\00\42\00\0b\04\aa\bb\cc\dd"
)
(module instance)
(invoke "copy" (i64.const 0xa) (i64.const 0x0) (i64.const 0x4))
(assert_return (invoke "load8_u" (i64.const 0x9)) (i32.const 0x0))
(assert_return (invoke "load8_u" (i64.const 0xa)) (i32.const 0xaa))
(assert_return (invoke "load8_u" (i64.const 0xb)) (i32.const 0xbb))
(assert_return (invoke "load8_u" (i64.const 0xc)) (i32.const 0xcc))
(assert_return (invoke "load8_u" (i64.const 0xd)) (i32.const 0xdd))
(assert_return (invoke "load8_u" (i64.const 0xe)) (i32.const 0x0))
(invoke "copy" (i64.const 0x8) (i64.const 0xa) (i64.const 0x4))
(assert_return (invoke "load8_u" (i64.const 0x8)) (i32.const 0xaa))
(assert_return (invoke "load8_u" (i64.const 0x9)) (i32.const 0xbb))
(assert_return (invoke "load8_u" (i64.const 0xa)) (i32.const 0xcc))
(assert_return (invoke "load8_u" (i64.const 0xb)) (i32.const 0xdd))
(assert_return (invoke "load8_u" (i64.const 0xc)) (i32.const 0xcc))
(assert_return (invoke "load8_u" (i64.const 0xd)) (i32.const 0xdd))
(invoke "copy" (i64.const 0xa) (i64.const 0x7) (i64.const 0x6))
(assert_return (invoke "load8_u" (i64.const 0xa)) (i32.const 0x0))
(assert_return (invoke "load8_u" (i64.const 0xb)) (i32.const 0xaa))
(assert_return (invoke "load8_u" (i64.const 0xc)) (i32.const 0xbb))
(assert_return (invoke "load8_u" (i64.const 0xd)) (i32.const 0xcc))
(assert_return (invoke "load8_u" (i64.const 0xe)) (i32.const 0xdd))
(assert_return (invoke "load8_u" (i64.const 0xf)) (i32.const 0xcc))
(assert_return (invoke "load8_u" (i64.const 0x10)) (i32.const 0x0))
(assert_trap
  (invoke "copy"
    (i64.const 0xd)
    (i64.const 0xb)
    (i64.const 0xffff_ffff_ffff_ffff)
  )
  "out of bounds"
)
(assert_return (invoke "load8_u" (i64.const 0xa)) (i32.const 0x0))
(assert_return (invoke "load8_u" (i64.const 0xb)) (i32.const 0xaa))
(assert_return (invoke "load8_u" (i64.const 0xc)) (i32.const 0xbb))
(assert_return (invoke "load8_u" (i64.const 0xd)) (i32.const 0xcc))
(assert_return (invoke "load8_u" (i64.const 0xe)) (i32.const 0xdd))
(assert_return (invoke "load8_u" (i64.const 0xf)) (i32.const 0xcc))
(assert_return (invoke "load8_u" (i64.const 0x10)) (i32.const 0x0))
(invoke "copy" (i64.const 0xff00) (i64.const 0x0) (i64.const 0x100))
(invoke "copy" (i64.const 0xfe00) (i64.const 0xff00) (i64.const 0x100))
(invoke "copy" (i64.const 0x1_0000) (i64.const 0x0) (i64.const 0x0))
(invoke "copy" (i64.const 0x0) (i64.const 0x1_0000) (i64.const 0x0))
(assert_trap
  (invoke "copy" (i64.const 0x1_0001) (i64.const 0x0) (i64.const 0x0))
  "out of bounds"
)
(assert_trap
  (invoke "copy" (i64.const 0x0) (i64.const 0x1_0001) (i64.const 0x0))
  "out of bounds"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8c\80\80\80\00\02\60"
  "\03\7e\7f\7f\00\60\01\7e\01\7f\03\83\80\80\80\00"
  "\02\00\01\05\83\80\80\80\00\01\04\01\07\92\80\80"
  "\80\00\02\04\69\6e\69\74\00\00\07\6c\6f\61\64\38"
  "\5f\75\00\01\0c\81\80\80\80\00\01\0a\9e\80\80\80"
  "\00\02\8c\80\80\80\00\00\20\00\20\01\20\02\fc\08"
  "\00\00\0b\87\80\80\80\00\00\20\00\2d\00\00\0b\0b"
  "\87\80\80\80\00\01\01\04\aa\bb\cc\dd"
)
(module instance)
(invoke "init" (i64.const 0x0) (i32.const 0x1) (i32.const 0x2))
(assert_return (invoke "load8_u" (i64.const 0x0)) (i32.const 0xbb))
(assert_return (invoke "load8_u" (i64.const 0x1)) (i32.const 0xcc))
(assert_return (invoke "load8_u" (i64.const 0x2)) (i32.const 0x0))
(invoke "init" (i64.const 0xfffc) (i32.const 0x0) (i32.const 0x4))
(assert_trap
  (invoke "init" (i64.const 0xfffe) (i32.const 0x0) (i32.const 0x3))
  "out of bounds"
)
(assert_return (invoke "load8_u" (i64.const 0xfffe)) (i32.const 0xcc))
(assert_return (invoke "load8_u" (i64.const 0xffff)) (i32.const 0xdd))
(invoke "init" (i64.const 0x1_0000) (i32.const 0x0) (i32.const 0x0))
(invoke "init" (i64.const 0x0) (i32.const 0x4) (i32.const 0x0))
(assert_trap
  (invoke "init" (i64.const 0x1_0001) (i32.const 0x0) (i32.const 0x0))
  "out of bounds"
)
(assert_trap
  (invoke "init" (i64.const 0x0) (i32.const 0x5) (i32.const 0x0))
  "out of bounds"
)
(invoke "init" (i64.const 0x0) (i32.const 0x0) (i32.const 0x0))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\03\85\80\80\80\00\04\00\00\00\00\05\83\80"
  "\80\80\00\01\04\01\07\bb\80\80\80\00\04\0c\64\72"
  "\6f\70\5f\70\61\73\73\69\76\65\00\00\0c\69\6e\69"
  "\74\5f\70\61\73\73\69\76\65\00\01\0b\64\72\6f\70"
  "\5f\61\63\74\69\76\65\00\02\0b\69\6e\69\74\5f\61"
  "\63\74\69\76\65\00\03\0c\81\80\80\80\00\02\0a\b7"
  "\80\80\80\00\04\85\80\80\80\00\00\fc\09\00\0b\8c"
  "\80\80\80\00\00\42\00\41\00\41\00\fc\08\00\00\0b"
  "\85\80\80\80\00\00\fc\09\01\0b\8c\80\80\80\00\00"
  "\42\00\41\00\41\00\fc\08\01\00\0b\0b\88\80\80\80"
  "\00\02\01\00\00\42\00\0b\00"
)
(module instance)
(invoke "init_passive")
(invoke "drop_passive")
(invoke "drop_passive")
(invoke "drop_active")
