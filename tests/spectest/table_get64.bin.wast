(module definition binary
  "\00\61\73\6d\01\00\00\00\01\97\80\80\80\00\05\60"
  "\00\00\60\01\6f\00\60\01\7e\01\6f\60\01\7e\01\70"
  "\60\01\7e\01\7f\03\86\80\80\80\00\05\00\01\02\03"
  "\04\04\87\80\80\80\00\02\6f\04\02\70\04\03\07\b8"
  "\80\80\80\00\04\04\69\6e\69\74\00\01\0d\67\65\74"
  "\2d\65\78\74\65\72\6e\72\65\66\00\02\0b\67\65\74"
  "\2d\66\75\6e\63\72\65\66\00\03\0f\69\73\5f\6e\75"
  "\6c\6c\2d\66\75\6e\63\72\65\66\00\04\09\89\80\80"
  "\80\00\01\02\01\42\01\0b\00\01\00\0a\bf\80\80\80"
  "\00\05\82\80\80\80\00\00\0b\90\80\80\80\00\00\42"
  "\01\20\00\26\00\42\02\42\01\25\01\26\01\0b\86\80"
  "\80\80\00\00\20\00\25\00\0b\86\80\80\80\00\00\20"
  "\00\25\01\0b\87\80\80\80\00\00\20\00\10\03\d1\0b"
)
(module instance)
(invoke "init" (ref.extern 1))
(assert_return (invoke "get-externref" (i64.const 0x0)) (ref.null extern))
(assert_return (invoke "get-externref" (i64.const 0x1)) (ref.extern 1))
(assert_return (invoke "get-funcref" (i64.const 0x0)) (ref.null func))
(assert_return (invoke "is_null-funcref" (i64.const 0x1)) (i32.const 0x0))
(assert_return (invoke "is_null-funcref" (i64.const 0x2)) (i32.const 0x0))
(assert_trap
  (invoke "get-externref" (i64.const 0x2))
  "out of bounds table access"
)
(assert_trap
  (invoke "get-funcref" (i64.const 0x3))
  "out of bounds table access"
)
(assert_trap
  (invoke "get-externref" (i64.const 0xffff_ffff_ffff_ffff))
  "out of bounds table access"
)
(assert_trap
  (invoke "get-funcref" (i64.const 0xffff_ffff_ffff_ffff))
  "out of bounds table access"
)
