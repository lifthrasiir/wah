(module definition binary
  "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\60"
  "\00\01\7f\60\01\7f\00\03\83\80\80\80\00\02\00\01"
  "\05\83\80\80\80\00\01\00\00\07\8f\80\80\80\00\02"
  "\04\73\69\7a\65\00\00\04\67\72\6f\77\00\01\0a\96"
  "\80\80\80\00\02\84\80\80\80\00\00\3f\00\0b\87\80"
  "\80\80\00\00\20\00\40\00\1a\0b"
)
(module instance)
(assert_return (invoke "size") (i32.const 0x0))
(assert_return (invoke "grow" (i32.const 0x1)))
(assert_return (invoke "size") (i32.const 0x1))
(assert_return (invoke "grow" (i32.const 0x4)))
(assert_return (invoke "size") (i32.const 0x5))
(assert_return (invoke "grow" (i32.const 0x0)))
(assert_return (invoke "size") (i32.const 0x5))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\60"
  "\00\01\7f\60\01\7f\00\03\83\80\80\80\00\02\00\01"
  "\05\83\80\80\80\00\01\00\01\07\8f\80\80\80\00\02"
  "\04\73\69\7a\65\00\00\04\67\72\6f\77\00\01\0a\96"
  "\80\80\80\00\02\84\80\80\80\00\00\3f\00\0b\87\80"
  "\80\80\00\00\20\00\40\00\1a\0b"
)
(module instance)
(assert_return (invoke "size") (i32.const 0x1))
(assert_return (invoke "grow" (i32.const 0x1)))
(assert_return (invoke "size") (i32.const 0x2))
(assert_return (invoke "grow" (i32.const 0x4)))
(assert_return (invoke "size") (i32.const 0x6))
(assert_return (invoke "grow" (i32.const 0x0)))
(assert_return (invoke "size") (i32.const 0x6))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\60"
  "\00\01\7f\60\01\7f\00\03\83\80\80\80\00\02\00\01"
  "\05\84\80\80\80\00\01\01\00\02\07\8f\80\80\80\00"
  "\02\04\73\69\7a\65\00\00\04\67\72\6f\77\00\01\0a"
  "\96\80\80\80\00\02\84\80\80\80\00\00\3f\00\0b\87"
  "\80\80\80\00\00\20\00\40\00\1a\0b"
)
(module instance)
(assert_return (invoke "size") (i32.const 0x0))
(assert_return (invoke "grow" (i32.const 0x3)))
(assert_return (invoke "size") (i32.const 0x0))
(assert_return (invoke "grow" (i32.const 0x1)))
(assert_return (invoke "size") (i32.const 0x1))
(assert_return (invoke "grow" (i32.const 0x0)))
(assert_return (invoke "size") (i32.const 0x1))
(assert_return (invoke "grow" (i32.const 0x4)))
(assert_return (invoke "size") (i32.const 0x1))
(assert_return (invoke "grow" (i32.const 0x1)))
(assert_return (invoke "size") (i32.const 0x2))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\60"
  "\00\01\7f\60\01\7f\00\03\83\80\80\80\00\02\00\01"
  "\05\84\80\80\80\00\01\01\03\08\07\8f\80\80\80\00"
  "\02\04\73\69\7a\65\00\00\04\67\72\6f\77\00\01\0a"
  "\96\80\80\80\00\02\84\80\80\80\00\00\3f\00\0b\87"
  "\80\80\80\00\00\20\00\40\00\1a\0b"
)
(module instance)
(assert_return (invoke "size") (i32.const 0x3))
(assert_return (invoke "grow" (i32.const 0x1)))
(assert_return (invoke "size") (i32.const 0x4))
(assert_return (invoke "grow" (i32.const 0x3)))
(assert_return (invoke "size") (i32.const 0x7))
(assert_return (invoke "grow" (i32.const 0x0)))
(assert_return (invoke "size") (i32.const 0x7))
(assert_return (invoke "grow" (i32.const 0x2)))
(assert_return (invoke "size") (i32.const 0x7))
(assert_return (invoke "grow" (i32.const 0x1)))
(assert_return (invoke "size") (i32.const 0x8))
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\05\83\80\80\80\00"
    "\01\00\01\0a\8a\80\80\80\00\01\84\80\80\80\00\00"
    "\3f\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7d\03\82\80\80\80\00\01\00\05\83\80\80\80"
    "\00\01\00\01\0a\8a\80\80\80\00\01\84\80\80\80\00"
    "\00\3f\00\0b"
  )
  "type mismatch"
)
