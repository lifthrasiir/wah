(module definition binary
  "\00\61\73\6d\01\00\00\00\01\a2\80\80\80\00\07\60"
  "\00\00\60\01\7f\01\6f\60\01\7f\01\70\60\02\7f\6f"
  "\00\60\02\7f\70\00\60\02\7f\7f\00\60\01\7f\01\7f"
  "\03\88\80\80\80\00\07\00\01\02\03\04\05\06\04\87"
  "\80\80\80\00\02\6f\00\01\70\00\02\07\e2\80\80\80"
  "\00\06\0d\67\65\74\2d\65\78\74\65\72\6e\72\65\66"
  "\00\01\0b\67\65\74\2d\66\75\6e\63\72\65\66\00\02"
  "\0d\73\65\74\2d\65\78\74\65\72\6e\72\65\66\00\03"
  "\0b\73\65\74\2d\66\75\6e\63\72\65\66\00\04\10\73"
  "\65\74\2d\66\75\6e\63\72\65\66\2d\66\72\6f\6d\00"
  "\05\0f\69\73\5f\6e\75\6c\6c\2d\66\75\6e\63\72\65"
  "\66\00\06\09\89\80\80\80\00\01\02\01\41\01\0b\00"
  "\01\00\0a\d3\80\80\80\00\07\82\80\80\80\00\00\0b"
  "\86\80\80\80\00\00\20\00\25\00\0b\86\80\80\80\00"
  "\00\20\00\25\01\0b\88\80\80\80\00\00\20\00\20\01"
  "\26\00\0b\88\80\80\80\00\00\20\00\20\01\26\01\0b"
  "\8a\80\80\80\00\00\20\00\20\01\25\01\26\01\0b\87"
  "\80\80\80\00\00\20\00\10\02\d1\0b"
)
(module instance)
(assert_return (invoke "get-externref" (i32.const 0x0)) (ref.null extern))
(assert_return (invoke "set-externref" (i32.const 0x0) (ref.extern 1)))
(assert_return (invoke "get-externref" (i32.const 0x0)) (ref.extern 1))
(assert_return (invoke "set-externref" (i32.const 0x0) (ref.null extern)))
(assert_return (invoke "get-externref" (i32.const 0x0)) (ref.null extern))
(assert_return (invoke "get-funcref" (i32.const 0x0)) (ref.null func))
(assert_return (invoke "set-funcref-from" (i32.const 0x0) (i32.const 0x1)))
(assert_return (invoke "is_null-funcref" (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "set-funcref" (i32.const 0x0) (ref.null func)))
(assert_return (invoke "get-funcref" (i32.const 0x0)) (ref.null func))
(assert_trap
  (invoke "set-externref" (i32.const 0x2) (ref.null extern))
  "out of bounds table access"
)
(assert_trap
  (invoke "set-funcref" (i32.const 0x3) (ref.null func))
  "out of bounds table access"
)
(assert_trap
  (invoke "set-externref" (i32.const 0xffff_ffff) (ref.null extern))
  "out of bounds table access"
)
(assert_trap
  (invoke "set-funcref" (i32.const 0xffff_ffff) (ref.null func))
  "out of bounds table access"
)
(assert_trap
  (invoke "set-externref" (i32.const 0x2) (ref.extern 0))
  "out of bounds table access"
)
(assert_trap
  (invoke "set-funcref-from" (i32.const 0x3) (i32.const 0x1))
  "out of bounds table access"
)
(assert_trap
  (invoke "set-externref" (i32.const 0xffff_ffff) (ref.extern 0))
  "out of bounds table access"
)
(assert_trap
  (invoke "set-funcref-from" (i32.const 0xffff_ffff) (i32.const 0x1))
  "out of bounds table access"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\04\84\80\80\80\00"
    "\01\6f\00\0a\0a\8a\80\80\80\00\01\84\80\80\80\00"
    "\00\26\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\04\84\80\80\80\00"
    "\01\6f\00\0a\0a\8c\80\80\80\00\01\86\80\80\80\00"
    "\00\d0\6f\26\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\04\84\80\80\80\00"
    "\01\6f\00\0a\0a\8c\80\80\80\00\01\86\80\80\80\00"
    "\00\41\01\26\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\04\84\80\80\80\00"
    "\01\6f\00\0a\0a\91\80\80\80\00\01\8b\80\80\80\00"
    "\00\43\00\00\80\3f\d0\6f\26\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\01\6f\00\03\82\80\80\80\00\01\00\04\84\80\80\80"
    "\00\01\70\00\0a\0a\8e\80\80\80\00\01\88\80\80\80"
    "\00\00\41\01\20\00\26\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\01\6f\00\03\82\80\80\80\00\01\00\04\87\80\80\80"
    "\00\02\6f\00\01\70\00\01\0a\8e\80\80\80\00\01\88"
    "\80\80\80\00\00\41\00\20\00\26\01\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\04\84\80\80\80"
    "\00\01\6f\00\0a\0a\8e\80\80\80\00\01\88\80\80\80"
    "\00\00\41\00\d0\6f\26\00\0b"
  )
  "type mismatch"
)
