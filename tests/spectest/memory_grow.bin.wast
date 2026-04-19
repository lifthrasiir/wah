(module definition binary
  "\00\61\73\6d\01\00\00\00\05\86\80\80\80\00\02\01"
  "\02\05\00\00\07\8f\80\80\80\00\02\04\6d\65\6d\31"
  "\02\00\04\6d\65\6d\32\02\01"
)
(module instance)
(register "M")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8a\80\80\80\00\02\60"
  "\00\01\7f\60\01\7f\01\7f\02\96\80\80\80\00\02\01"
  "\4d\04\6d\65\6d\31\02\01\01\06\01\4d\04\6d\65\6d"
  "\32\02\00\00\03\89\80\80\80\00\08\00\00\00\00\01"
  "\01\01\01\05\86\80\80\80\00\02\00\03\01\04\05\07"
  "\c1\80\80\80\00\08\05\73\69\7a\65\31\00\00\05\73"
  "\69\7a\65\32\00\01\05\73\69\7a\65\33\00\02\05\73"
  "\69\7a\65\34\00\03\05\67\72\6f\77\31\00\04\05\67"
  "\72\6f\77\32\00\05\05\67\72\6f\77\33\00\06\05\67"
  "\72\6f\77\34\00\07\0a\d1\80\80\80\00\08\84\80\80"
  "\80\00\00\3f\00\0b\84\80\80\80\00\00\3f\01\0b\84"
  "\80\80\80\00\00\3f\02\0b\84\80\80\80\00\00\3f\03"
  "\0b\86\80\80\80\00\00\20\00\40\00\0b\86\80\80\80"
  "\00\00\20\00\40\01\0b\86\80\80\80\00\00\20\00\40"
  "\02\0b\86\80\80\80\00\00\20\00\40\03\0b"
)
(module instance)
(assert_return (invoke "size1") (i32.const 0x2))
(assert_return (invoke "size2") (i32.const 0x0))
(assert_return (invoke "size3") (i32.const 0x3))
(assert_return (invoke "size4") (i32.const 0x4))
(assert_return (invoke "grow1" (i32.const 0x1)) (i32.const 0x2))
(assert_return (invoke "size1") (i32.const 0x3))
(assert_return (invoke "size2") (i32.const 0x0))
(assert_return (invoke "size3") (i32.const 0x3))
(assert_return (invoke "size4") (i32.const 0x4))
(assert_return (invoke "grow1" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "size1") (i32.const 0x5))
(assert_return (invoke "size2") (i32.const 0x0))
(assert_return (invoke "size3") (i32.const 0x3))
(assert_return (invoke "size4") (i32.const 0x4))
(assert_return (invoke "grow1" (i32.const 0x1)) (i32.const 0xffff_ffff))
(assert_return (invoke "size1") (i32.const 0x5))
(assert_return (invoke "size2") (i32.const 0x0))
(assert_return (invoke "size3") (i32.const 0x3))
(assert_return (invoke "size4") (i32.const 0x4))
(assert_return (invoke "grow2" (i32.const 0xa)) (i32.const 0x0))
(assert_return (invoke "size1") (i32.const 0x5))
(assert_return (invoke "size2") (i32.const 0xa))
(assert_return (invoke "size3") (i32.const 0x3))
(assert_return (invoke "size4") (i32.const 0x4))
(assert_return (invoke "grow3" (i32.const 0x1000_0000)) (i32.const 0xffff_ffff))
(assert_return (invoke "size1") (i32.const 0x5))
(assert_return (invoke "size2") (i32.const 0xa))
(assert_return (invoke "size3") (i32.const 0x3))
(assert_return (invoke "size4") (i32.const 0x4))
(assert_return (invoke "grow3" (i32.const 0x3)) (i32.const 0x3))
(assert_return (invoke "size1") (i32.const 0x5))
(assert_return (invoke "size2") (i32.const 0xa))
(assert_return (invoke "size3") (i32.const 0x6))
(assert_return (invoke "size4") (i32.const 0x4))
(assert_return (invoke "grow4" (i32.const 0x1)) (i32.const 0x4))
(assert_return (invoke "grow4" (i32.const 0x1)) (i32.const 0xffff_ffff))
(assert_return (invoke "size1") (i32.const 0x5))
(assert_return (invoke "size2") (i32.const 0xa))
(assert_return (invoke "size3") (i32.const 0x6))
(assert_return (invoke "size4") (i32.const 0x5))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8a\80\80\80\00\02\60"
  "\01\7f\01\7f\60\00\01\7f\03\85\80\80\80\00\04\00"
  "\00\01\01\05\85\80\80\80\00\02\00\01\00\02\07\a1"
  "\80\80\80\00\04\05\67\72\6f\77\31\00\00\05\67\72"
  "\6f\77\32\00\01\05\73\69\7a\65\31\00\02\05\73\69"
  "\7a\65\32\00\03\0a\a9\80\80\80\00\04\86\80\80\80"
  "\00\00\20\00\40\00\0b\86\80\80\80\00\00\20\00\40"
  "\01\0b\84\80\80\80\00\00\3f\00\0b\84\80\80\80\00"
  "\00\3f\01\0b"
)
(module instance)
(assert_return (invoke "size1") (i32.const 0x1))
(assert_return (invoke "size2") (i32.const 0x2))
(assert_return (invoke "grow1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "grow1" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "grow1" (i32.const 0x1)) (i32.const 0x8))
(assert_return (invoke "grow2" (i32.const 0x1)) (i32.const 0x2))
(assert_return (invoke "grow2" (i32.const 0x1)) (i32.const 0x3))
