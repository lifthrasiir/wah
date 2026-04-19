(module definition binary
  "\00\61\73\6d\01\00\00\00\01\a2\80\80\80\00\07\60"
  "\00\00\60\01\7e\01\6f\60\01\7e\01\70\60\02\7e\6f"
  "\00\60\02\7e\70\00\60\02\7e\7e\00\60\01\7e\01\7f"
  "\03\88\80\80\80\00\07\00\01\02\03\04\05\06\04\87"
  "\80\80\80\00\02\6f\04\01\70\04\02\07\e2\80\80\80"
  "\00\06\0d\67\65\74\2d\65\78\74\65\72\6e\72\65\66"
  "\00\01\0b\67\65\74\2d\66\75\6e\63\72\65\66\00\02"
  "\0d\73\65\74\2d\65\78\74\65\72\6e\72\65\66\00\03"
  "\0b\73\65\74\2d\66\75\6e\63\72\65\66\00\04\10\73"
  "\65\74\2d\66\75\6e\63\72\65\66\2d\66\72\6f\6d\00"
  "\05\0f\69\73\5f\6e\75\6c\6c\2d\66\75\6e\63\72\65"
  "\66\00\06\09\89\80\80\80\00\01\02\01\42\01\0b\00"
  "\01\00\0a\d3\80\80\80\00\07\82\80\80\80\00\00\0b"
  "\86\80\80\80\00\00\20\00\25\00\0b\86\80\80\80\00"
  "\00\20\00\25\01\0b\88\80\80\80\00\00\20\00\20\01"
  "\26\00\0b\88\80\80\80\00\00\20\00\20\01\26\01\0b"
  "\8a\80\80\80\00\00\20\00\20\01\25\01\26\01\0b\87"
  "\80\80\80\00\00\20\00\10\02\d1\0b"
)
(module instance)
(assert_return (invoke "get-externref" (i64.const 0x0)) (ref.null extern))
(assert_return (invoke "set-externref" (i64.const 0x0) (ref.extern 1)))
(assert_return (invoke "get-externref" (i64.const 0x0)) (ref.extern 1))
(assert_return (invoke "set-externref" (i64.const 0x0) (ref.null extern)))
(assert_return (invoke "get-externref" (i64.const 0x0)) (ref.null extern))
(assert_return (invoke "get-funcref" (i64.const 0x0)) (ref.null func))
(assert_return (invoke "set-funcref-from" (i64.const 0x0) (i64.const 0x1)))
(assert_return (invoke "is_null-funcref" (i64.const 0x0)) (i32.const 0x0))
(assert_return (invoke "set-funcref" (i64.const 0x0) (ref.null func)))
(assert_return (invoke "get-funcref" (i64.const 0x0)) (ref.null func))
(assert_trap
  (invoke "set-externref" (i64.const 0x2) (ref.null extern))
  "out of bounds table access"
)
(assert_trap
  (invoke "set-funcref" (i64.const 0x3) (ref.null func))
  "out of bounds table access"
)
(assert_trap
  (invoke "set-externref" (i64.const 0xffff_ffff_ffff_ffff) (ref.null extern))
  "out of bounds table access"
)
(assert_trap
  (invoke "set-funcref" (i64.const 0xffff_ffff_ffff_ffff) (ref.null func))
  "out of bounds table access"
)
(assert_trap
  (invoke "set-externref" (i64.const 0x2) (ref.extern 0))
  "out of bounds table access"
)
(assert_trap
  (invoke "set-funcref-from" (i64.const 0x3) (i64.const 0x1))
  "out of bounds table access"
)
(assert_trap
  (invoke "set-externref" (i64.const 0xffff_ffff_ffff_ffff) (ref.extern 0))
  "out of bounds table access"
)
(assert_trap
  (invoke "set-funcref-from" (i64.const 0xffff_ffff_ffff_ffff) (i64.const 0x1))
  "out of bounds table access"
)
