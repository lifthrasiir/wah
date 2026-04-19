(module definition binary
  "\00\61\73\6d\01\00\00\00\01\af\80\80\80\00\09\50"
  "\00\5f\00\50\00\5f\01\7f\00\5e\78\00\50\01\00\5f"
  "\00\50\01\00\5f\00\50\01\01\5f\01\7f\00\50\01\01"
  "\5f\01\7f\00\60\00\00\60\02\7f\7f\01\7f\03\83\80"
  "\80\80\00\02\07\08\04\84\80\80\80\00\01\6d\00\14"
  "\07\8d\80\80\80\00\02\04\69\6e\69\74\00\00\02\65"
  "\71\00\01\0a\dc\80\80\80\00\02\c6\80\80\80\00\00"
  "\41\00\d0\6d\26\00\41\01\d0\6c\26\00\41\02\41\07"
  "\fb\1c\26\00\41\03\41\07\fb\1c\26\00\41\04\41\08"
  "\fb\1c\26\00\41\05\fb\01\00\26\00\41\06\fb\01\00"
  "\26\00\41\07\41\00\fb\07\02\26\00\41\08\41\00\fb"
  "\07\02\26\00\0b\8b\80\80\80\00\00\20\00\25\00\20"
  "\01\25\00\d3\0b"
)
(module instance)
(invoke "init")
(assert_return (invoke "eq" (i32.const 0x0) (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "eq" (i32.const 0x0) (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "eq" (i32.const 0x0) (i32.const 0x2)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x0) (i32.const 0x3)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x0) (i32.const 0x4)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x0) (i32.const 0x5)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x0) (i32.const 0x6)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x0) (i32.const 0x7)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x0) (i32.const 0x8)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "eq" (i32.const 0x1) (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "eq" (i32.const 0x1) (i32.const 0x2)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x1) (i32.const 0x3)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x1) (i32.const 0x4)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x1) (i32.const 0x5)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x1) (i32.const 0x6)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x1) (i32.const 0x7)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x1) (i32.const 0x8)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x2) (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x2) (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x2) (i32.const 0x2)) (i32.const 0x1))
(assert_return (invoke "eq" (i32.const 0x2) (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "eq" (i32.const 0x2) (i32.const 0x4)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x2) (i32.const 0x5)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x2) (i32.const 0x6)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x2) (i32.const 0x7)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x2) (i32.const 0x8)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x3) (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x3) (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x3) (i32.const 0x2)) (i32.const 0x1))
(assert_return (invoke "eq" (i32.const 0x3) (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "eq" (i32.const 0x3) (i32.const 0x4)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x3) (i32.const 0x5)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x3) (i32.const 0x6)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x3) (i32.const 0x7)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x3) (i32.const 0x8)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x4) (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x4) (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x4) (i32.const 0x2)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x4) (i32.const 0x3)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x4) (i32.const 0x4)) (i32.const 0x1))
(assert_return (invoke "eq" (i32.const 0x4) (i32.const 0x5)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x4) (i32.const 0x6)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x4) (i32.const 0x7)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x4) (i32.const 0x8)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x5) (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x5) (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x5) (i32.const 0x2)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x5) (i32.const 0x3)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x5) (i32.const 0x4)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x5) (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "eq" (i32.const 0x5) (i32.const 0x6)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x5) (i32.const 0x7)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x5) (i32.const 0x8)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x6) (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x6) (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x6) (i32.const 0x2)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x6) (i32.const 0x3)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x6) (i32.const 0x4)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x6) (i32.const 0x5)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x6) (i32.const 0x6)) (i32.const 0x1))
(assert_return (invoke "eq" (i32.const 0x6) (i32.const 0x7)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x6) (i32.const 0x8)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x7) (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x7) (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x7) (i32.const 0x2)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x7) (i32.const 0x3)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x7) (i32.const 0x4)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x7) (i32.const 0x5)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x7) (i32.const 0x6)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x7) (i32.const 0x7)) (i32.const 0x1))
(assert_return (invoke "eq" (i32.const 0x7) (i32.const 0x8)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x8) (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x8) (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x8) (i32.const 0x2)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x8) (i32.const 0x3)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x8) (i32.const 0x4)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x8) (i32.const 0x5)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x8) (i32.const 0x6)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x8) (i32.const 0x7)) (i32.const 0x0))
(assert_return (invoke "eq" (i32.const 0x8) (i32.const 0x8)) (i32.const 0x1))
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\87\80\80\80\00\01\60"
    "\01\64\6e\01\7f\03\82\80\80\80\00\01\00\07\86\80"
    "\80\80\00\01\02\65\71\00\00\0a\8d\80\80\80\00\01"
    "\87\80\80\80\00\00\20\00\20\00\d3\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
    "\01\6e\01\7f\03\82\80\80\80\00\01\00\07\86\80\80"
    "\80\00\01\02\65\71\00\00\0a\8d\80\80\80\00\01\87"
    "\80\80\80\00\00\20\00\20\00\d3\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\87\80\80\80\00\01\60"
    "\01\64\70\01\7f\03\82\80\80\80\00\01\00\07\86\80"
    "\80\80\00\01\02\65\71\00\00\0a\8d\80\80\80\00\01"
    "\87\80\80\80\00\00\20\00\20\00\d3\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
    "\01\70\01\7f\03\82\80\80\80\00\01\00\07\86\80\80"
    "\80\00\01\02\65\71\00\00\0a\8d\80\80\80\00\01\87"
    "\80\80\80\00\00\20\00\20\00\d3\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\87\80\80\80\00\01\60"
    "\01\64\6f\01\7f\03\82\80\80\80\00\01\00\07\86\80"
    "\80\80\00\01\02\65\71\00\00\0a\8d\80\80\80\00\01"
    "\87\80\80\80\00\00\20\00\20\00\d3\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
    "\01\6f\01\7f\03\82\80\80\80\00\01\00\07\86\80\80"
    "\80\00\01\02\65\71\00\00\0a\8d\80\80\80\00\01\87"
    "\80\80\80\00\00\20\00\20\00\d3\0b"
  )
  "type mismatch"
)
