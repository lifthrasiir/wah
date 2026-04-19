(module definition binary
  "\00\61\73\6d\01\00\00\00\05\83\80\80\80\00\01\00"
  "\02\07\87\80\80\80\00\01\03\6d\65\6d\02\00"
)
(module instance)
(register "M")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\60"
  "\01\7f\01\7f\60\00\00\02\8a\80\80\80\00\01\01\4d"
  "\03\6d\65\6d\02\00\02\03\85\80\80\80\00\04\00\00"
  "\01\01\05\83\80\80\80\00\01\00\03\07\ad\80\80\80"
  "\00\04\05\72\65\61\64\31\00\00\05\72\65\61\64\32"
  "\00\01\0b\63\6f\70\79\2d\31\2d\74\6f\2d\32\00\02"
  "\0b\63\6f\70\79\2d\32\2d\74\6f\2d\31\00\03\0a\f0"
  "\80\80\80\00\04\87\80\80\80\00\00\20\00\2d\00\00"
  "\0b\88\80\80\80\00\00\20\00\2d\40\01\00\0b\a6\80"
  "\80\80\00\01\01\7f\41\14\21\00\03\40\20\00\41\17"
  "\46\0d\01\20\00\20\00\2d\00\00\3a\40\01\00\20\00"
  "\41\01\6a\21\00\0c\00\0b\0b\a6\80\80\80\00\01\01"
  "\7f\41\32\21\00\03\40\20\00\41\36\46\0d\01\20\00"
  "\20\00\2d\40\01\00\3a\00\00\20\00\41\01\6a\21\00"
  "\0c\00\0b\0b\0b\96\80\80\80\00\02\00\41\14\0b\05"
  "\01\02\03\04\05\02\01\41\32\0b\05\0a\0b\0c\0d\0e"
)
(module instance)
(assert_return (invoke "read2" (i32.const 0x14)) (i32.const 0x0))
(assert_return (invoke "read2" (i32.const 0x15)) (i32.const 0x0))
(assert_return (invoke "read2" (i32.const 0x16)) (i32.const 0x0))
(assert_return (invoke "read2" (i32.const 0x17)) (i32.const 0x0))
(assert_return (invoke "read2" (i32.const 0x18)) (i32.const 0x0))
(invoke "copy-1-to-2")
(assert_return (invoke "read2" (i32.const 0x14)) (i32.const 0x1))
(assert_return (invoke "read2" (i32.const 0x15)) (i32.const 0x2))
(assert_return (invoke "read2" (i32.const 0x16)) (i32.const 0x3))
(assert_return (invoke "read2" (i32.const 0x17)) (i32.const 0x0))
(assert_return (invoke "read2" (i32.const 0x18)) (i32.const 0x0))
(assert_return (invoke "read1" (i32.const 0x32)) (i32.const 0x0))
(assert_return (invoke "read1" (i32.const 0x33)) (i32.const 0x0))
(assert_return (invoke "read1" (i32.const 0x34)) (i32.const 0x0))
(assert_return (invoke "read1" (i32.const 0x35)) (i32.const 0x0))
(assert_return (invoke "read1" (i32.const 0x36)) (i32.const 0x0))
(invoke "copy-2-to-1")
(assert_return (invoke "read1" (i32.const 0x32)) (i32.const 0xa))
(assert_return (invoke "read1" (i32.const 0x33)) (i32.const 0xb))
(assert_return (invoke "read1" (i32.const 0x34)) (i32.const 0xc))
(assert_return (invoke "read1" (i32.const 0x35)) (i32.const 0xd))
(assert_return (invoke "read1" (i32.const 0x36)) (i32.const 0x0))
