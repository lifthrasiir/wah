(module definition binary
  "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\60"
  "\00\01\7f\60\01\7f\00\03\89\80\80\80\00\08\00\00"
  "\00\00\01\01\01\01\04\8f\80\80\80\00\04\6f\00\00"
  "\6f\00\01\6f\01\00\02\6f\01\03\08\07\d1\80\80\80"
  "\00\08\07\73\69\7a\65\2d\74\30\00\00\07\73\69\7a"
  "\65\2d\74\31\00\01\07\73\69\7a\65\2d\74\32\00\02"
  "\07\73\69\7a\65\2d\74\33\00\03\07\67\72\6f\77\2d"
  "\74\30\00\04\07\67\72\6f\77\2d\74\31\00\05\07\67"
  "\72\6f\77\2d\74\32\00\06\07\67\72\6f\77\2d\74\33"
  "\00\07\0a\e5\80\80\80\00\08\85\80\80\80\00\00\fc"
  "\10\00\0b\85\80\80\80\00\00\fc\10\01\0b\85\80\80"
  "\80\00\00\fc\10\02\0b\85\80\80\80\00\00\fc\10\03"
  "\0b\8a\80\80\80\00\00\d0\6f\20\00\fc\0f\00\1a\0b"
  "\8a\80\80\80\00\00\d0\6f\20\00\fc\0f\01\1a\0b\8a"
  "\80\80\80\00\00\d0\6f\20\00\fc\0f\02\1a\0b\8a\80"
  "\80\80\00\00\d0\6f\20\00\fc\0f\03\1a\0b"
)
(module instance)
(assert_return (invoke "size-t0") (i32.const 0x0))
(assert_return (invoke "grow-t0" (i32.const 0x1)))
(assert_return (invoke "size-t0") (i32.const 0x1))
(assert_return (invoke "grow-t0" (i32.const 0x4)))
(assert_return (invoke "size-t0") (i32.const 0x5))
(assert_return (invoke "grow-t0" (i32.const 0x0)))
(assert_return (invoke "size-t0") (i32.const 0x5))
(assert_return (invoke "size-t1") (i32.const 0x1))
(assert_return (invoke "grow-t1" (i32.const 0x1)))
(assert_return (invoke "size-t1") (i32.const 0x2))
(assert_return (invoke "grow-t1" (i32.const 0x4)))
(assert_return (invoke "size-t1") (i32.const 0x6))
(assert_return (invoke "grow-t1" (i32.const 0x0)))
(assert_return (invoke "size-t1") (i32.const 0x6))
(assert_return (invoke "size-t2") (i32.const 0x0))
(assert_return (invoke "grow-t2" (i32.const 0x3)))
(assert_return (invoke "size-t2") (i32.const 0x0))
(assert_return (invoke "grow-t2" (i32.const 0x1)))
(assert_return (invoke "size-t2") (i32.const 0x1))
(assert_return (invoke "grow-t2" (i32.const 0x0)))
(assert_return (invoke "size-t2") (i32.const 0x1))
(assert_return (invoke "grow-t2" (i32.const 0x4)))
(assert_return (invoke "size-t2") (i32.const 0x1))
(assert_return (invoke "grow-t2" (i32.const 0x1)))
(assert_return (invoke "size-t2") (i32.const 0x2))
(assert_return (invoke "size-t3") (i32.const 0x3))
(assert_return (invoke "grow-t3" (i32.const 0x1)))
(assert_return (invoke "size-t3") (i32.const 0x4))
(assert_return (invoke "grow-t3" (i32.const 0x3)))
(assert_return (invoke "size-t3") (i32.const 0x7))
(assert_return (invoke "grow-t3" (i32.const 0x0)))
(assert_return (invoke "size-t3") (i32.const 0x7))
(assert_return (invoke "grow-t3" (i32.const 0x2)))
(assert_return (invoke "size-t3") (i32.const 0x7))
(assert_return (invoke "grow-t3" (i32.const 0x1)))
(assert_return (invoke "size-t3") (i32.const 0x8))
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\04\84\80\80\80\00"
    "\01\6f\00\01\0a\8b\80\80\80\00\01\85\80\80\80\00"
    "\00\fc\10\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7d\03\82\80\80\80\00\01\00\04\84\80\80\80"
    "\00\01\6f\00\01\0a\8b\80\80\80\00\01\85\80\80\80"
    "\00\00\fc\10\00\0b"
  )
  "type mismatch"
)
