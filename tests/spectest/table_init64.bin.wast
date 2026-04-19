(module definition binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\00\01\7f\03\86\80\80\80\00\05\00\00\00\00\00\07"
  "\9f\80\80\80\00\05\03\65\66\30\00\00\03\65\66\31"
  "\00\01\03\65\66\32\00\02\03\65\66\33\00\03\03\65"
  "\66\34\00\04\0a\ae\80\80\80\00\05\84\80\80\80\00"
  "\00\41\00\0b\84\80\80\80\00\00\41\01\0b\84\80\80"
  "\80\00\00\41\02\0b\84\80\80\80\00\00\41\03\0b\84"
  "\80\80\80\00\00\41\04\0b"
)
(module instance)
(register "a")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\88\80\80\80\00"
  "\07\00\00\00\00\00\01\02\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1e\1e\07\90\80\80\80\00\02\04\74"
  "\65\73\74\00\0a\05\63\68\65\63\6b\00\0b\09\b5\80"
  "\80\80\00\04\00\41\02\0b\04\03\01\04\01\05\70\04"
  "\d2\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\00\41\0c\0b"
  "\05\07\05\02\03\06\05\70\05\d2\05\0b\d2\09\0b\d2"
  "\02\0b\d2\07\0b\d2\06\0b\0a\cb\80\80\80\00\07\84"
  "\80\80\80\00\00\41\05\0b\84\80\80\80\00\00\41\06"
  "\0b\84\80\80\80\00\00\41\07\0b\84\80\80\80\00\00"
  "\41\08\0b\84\80\80\80\00\00\41\09\0b\8c\80\80\80"
  "\00\00\41\07\41\00\41\04\fc\0c\01\00\0b\87\80\80"
  "\80\00\00\20\00\11\00\00\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check" (i32.const 0x6)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0x7)) (i32.const 0x2))
(assert_return (invoke "check" (i32.const 0x8)) (i32.const 0x7))
(assert_return (invoke "check" (i32.const 0x9)) (i32.const 0x1))
(assert_return (invoke "check" (i32.const 0xa)) (i32.const 0x8))
(assert_trap (invoke "check" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0xc)) (i32.const 0x7))
(assert_return (invoke "check" (i32.const 0xd)) (i32.const 0x5))
(assert_return (invoke "check" (i32.const 0xe)) (i32.const 0x2))
(assert_return (invoke "check" (i32.const 0xf)) (i32.const 0x3))
(assert_return (invoke "check" (i32.const 0x10)) (i32.const 0x6))
(assert_trap (invoke "check" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\88\80\80\80\00"
  "\07\00\00\00\00\00\01\02\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1e\1e\07\90\80\80\80\00\02\04\74"
  "\65\73\74\00\0a\05\63\68\65\63\6b\00\0b\09\b5\80"
  "\80\80\00\04\00\41\02\0b\04\03\01\04\01\05\70\04"
  "\d2\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\00\41\0c\0b"
  "\05\07\05\02\03\06\05\70\05\d2\05\0b\d2\09\0b\d2"
  "\02\0b\d2\07\0b\d2\06\0b\0a\cb\80\80\80\00\07\84"
  "\80\80\80\00\00\41\05\0b\84\80\80\80\00\00\41\06"
  "\0b\84\80\80\80\00\00\41\07\0b\84\80\80\80\00\00"
  "\41\08\0b\84\80\80\80\00\00\41\09\0b\8c\80\80\80"
  "\00\00\41\0f\41\01\41\03\fc\0c\03\00\0b\87\80\80"
  "\80\00\00\20\00\11\00\00\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0xc)) (i32.const 0x7))
(assert_return (invoke "check" (i32.const 0xd)) (i32.const 0x5))
(assert_return (invoke "check" (i32.const 0xe)) (i32.const 0x2))
(assert_return (invoke "check" (i32.const 0xf)) (i32.const 0x9))
(assert_return (invoke "check" (i32.const 0x10)) (i32.const 0x2))
(assert_return (invoke "check" (i32.const 0x11)) (i32.const 0x7))
(assert_trap (invoke "check" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\88\80\80\80\00"
  "\07\00\00\00\00\00\01\02\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1e\1e\07\90\80\80\80\00\02\04\74"
  "\65\73\74\00\0a\05\63\68\65\63\6b\00\0b\09\b5\80"
  "\80\80\00\04\00\41\02\0b\04\03\01\04\01\05\70\04"
  "\d2\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\00\41\0c\0b"
  "\05\07\05\02\03\06\05\70\05\d2\05\0b\d2\09\0b\d2"
  "\02\0b\d2\07\0b\d2\06\0b\0a\8d\81\80\80\00\07\84"
  "\80\80\80\00\00\41\05\0b\84\80\80\80\00\00\41\06"
  "\0b\84\80\80\80\00\00\41\07\0b\84\80\80\80\00\00"
  "\41\08\0b\84\80\80\80\00\00\41\09\0b\ce\80\80\80"
  "\00\00\41\07\41\00\41\04\fc\0c\01\00\fc\0d\01\41"
  "\0f\41\01\41\03\fc\0c\03\00\fc\0d\03\41\14\41\0f"
  "\41\05\fc\0e\00\00\41\15\41\1d\41\01\fc\0e\00\00"
  "\41\18\41\0a\41\01\fc\0e\00\00\41\0d\41\0b\41\04"
  "\fc\0e\00\00\41\13\41\14\41\05\fc\0e\00\00\0b\87"
  "\80\80\80\00\00\20\00\11\00\00\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check" (i32.const 0x6)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0x7)) (i32.const 0x2))
(assert_return (invoke "check" (i32.const 0x8)) (i32.const 0x7))
(assert_return (invoke "check" (i32.const 0x9)) (i32.const 0x1))
(assert_return (invoke "check" (i32.const 0xa)) (i32.const 0x8))
(assert_trap (invoke "check" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0xc)) (i32.const 0x7))
(assert_trap (invoke "check" (i32.const 0xd)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0xe)) (i32.const 0x7))
(assert_return (invoke "check" (i32.const 0xf)) (i32.const 0x5))
(assert_return (invoke "check" (i32.const 0x10)) (i32.const 0x2))
(assert_return (invoke "check" (i32.const 0x11)) (i32.const 0x7))
(assert_trap (invoke "check" (i32.const 0x12)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0x13)) (i32.const 0x9))
(assert_trap (invoke "check" (i32.const 0x14)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0x15)) (i32.const 0x7))
(assert_trap (invoke "check" (i32.const 0x16)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0x17)) (i32.const 0x8))
(assert_return (invoke "check" (i32.const 0x18)) (i32.const 0x8))
(assert_trap (invoke "check" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\88\80\80\80\00"
  "\07\00\00\00\00\00\01\02\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1e\1e\07\90\80\80\80\00\02\04\74"
  "\65\73\74\00\0a\05\63\68\65\63\6b\00\0b\09\b9\80"
  "\80\80\00\04\02\01\41\02\0b\00\04\03\01\04\01\05"
  "\70\04\d2\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\02\01"
  "\41\0c\0b\00\05\07\05\02\03\06\05\70\05\d2\05\0b"
  "\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b\0a\cb\80\80"
  "\80\00\07\84\80\80\80\00\00\41\05\0b\84\80\80\80"
  "\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84\80"
  "\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09\0b"
  "\8c\80\80\80\00\00\41\07\41\00\41\04\fc\0c\01\01"
  "\0b\87\80\80\80\00\00\20\00\11\00\01\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check" (i32.const 0x6)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0x7)) (i32.const 0x2))
(assert_return (invoke "check" (i32.const 0x8)) (i32.const 0x7))
(assert_return (invoke "check" (i32.const 0x9)) (i32.const 0x1))
(assert_return (invoke "check" (i32.const 0xa)) (i32.const 0x8))
(assert_trap (invoke "check" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0xc)) (i32.const 0x7))
(assert_return (invoke "check" (i32.const 0xd)) (i32.const 0x5))
(assert_return (invoke "check" (i32.const 0xe)) (i32.const 0x2))
(assert_return (invoke "check" (i32.const 0xf)) (i32.const 0x3))
(assert_return (invoke "check" (i32.const 0x10)) (i32.const 0x6))
(assert_trap (invoke "check" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\88\80\80\80\00"
  "\07\00\00\00\00\00\01\02\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1e\1e\07\90\80\80\80\00\02\04\74"
  "\65\73\74\00\0a\05\63\68\65\63\6b\00\0b\09\b9\80"
  "\80\80\00\04\02\01\41\02\0b\00\04\03\01\04\01\05"
  "\70\04\d2\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\02\01"
  "\41\0c\0b\00\05\07\05\02\03\06\05\70\05\d2\05\0b"
  "\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b\0a\cb\80\80"
  "\80\00\07\84\80\80\80\00\00\41\05\0b\84\80\80\80"
  "\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84\80"
  "\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09\0b"
  "\8c\80\80\80\00\00\41\0f\41\01\41\03\fc\0c\03\01"
  "\0b\87\80\80\80\00\00\20\00\11\00\01\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0xc)) (i32.const 0x7))
(assert_return (invoke "check" (i32.const 0xd)) (i32.const 0x5))
(assert_return (invoke "check" (i32.const 0xe)) (i32.const 0x2))
(assert_return (invoke "check" (i32.const 0xf)) (i32.const 0x9))
(assert_return (invoke "check" (i32.const 0x10)) (i32.const 0x2))
(assert_return (invoke "check" (i32.const 0x11)) (i32.const 0x7))
(assert_trap (invoke "check" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\88\80\80\80\00"
  "\07\00\00\00\00\00\01\02\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1e\1e\07\90\80\80\80\00\02\04\74"
  "\65\73\74\00\0a\05\63\68\65\63\6b\00\0b\09\b9\80"
  "\80\80\00\04\02\01\41\02\0b\00\04\03\01\04\01\05"
  "\70\04\d2\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\02\01"
  "\41\0c\0b\00\05\07\05\02\03\06\05\70\05\d2\05\0b"
  "\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b\0a\8d\81\80"
  "\80\00\07\84\80\80\80\00\00\41\05\0b\84\80\80\80"
  "\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84\80"
  "\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09\0b"
  "\ce\80\80\80\00\00\41\07\41\00\41\04\fc\0c\01\01"
  "\fc\0d\01\41\0f\41\01\41\03\fc\0c\03\01\fc\0d\03"
  "\41\14\41\0f\41\05\fc\0e\01\01\41\15\41\1d\41\01"
  "\fc\0e\01\01\41\18\41\0a\41\01\fc\0e\01\01\41\0d"
  "\41\0b\41\04\fc\0e\01\01\41\13\41\14\41\05\fc\0e"
  "\01\01\0b\87\80\80\80\00\00\20\00\11\00\01\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check" (i32.const 0x6)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0x7)) (i32.const 0x2))
(assert_return (invoke "check" (i32.const 0x8)) (i32.const 0x7))
(assert_return (invoke "check" (i32.const 0x9)) (i32.const 0x1))
(assert_return (invoke "check" (i32.const 0xa)) (i32.const 0x8))
(assert_trap (invoke "check" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0xc)) (i32.const 0x7))
(assert_trap (invoke "check" (i32.const 0xd)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0xe)) (i32.const 0x7))
(assert_return (invoke "check" (i32.const 0xf)) (i32.const 0x5))
(assert_return (invoke "check" (i32.const 0x10)) (i32.const 0x2))
(assert_return (invoke "check" (i32.const 0x11)) (i32.const 0x7))
(assert_trap (invoke "check" (i32.const 0x12)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0x13)) (i32.const 0x9))
(assert_trap (invoke "check" (i32.const 0x14)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0x15)) (i32.const 0x7))
(assert_trap (invoke "check" (i32.const 0x16)) "uninitialized element")
(assert_return (invoke "check" (i32.const 0x17)) (i32.const 0x8))
(assert_return (invoke "check" (i32.const 0x18)) (i32.const 0x8))
(assert_trap (invoke "check" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7e\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\88\80\80\80\00"
  "\07\00\00\00\00\00\01\02\04\8d\80\80\80\00\03\70"
  "\01\1e\1e\70\01\1e\1e\70\05\1e\1e\07\90\80\80\80"
  "\00\02\04\74\65\73\74\00\0a\05\63\68\65\63\6b\00"
  "\0b\09\b9\80\80\80\00\04\02\02\42\02\0b\00\04\03"
  "\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01\0b\d2"
  "\08\0b\02\02\42\0c\0b\00\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\cb\80\80\80\00\07\84\80\80\80\00\00\41\05\0b"
  "\84\80\80\80\00\00\41\06\0b\84\80\80\80\00\00\41"
  "\07\0b\84\80\80\80\00\00\41\08\0b\84\80\80\80\00"
  "\00\41\09\0b\8c\80\80\80\00\00\42\07\41\00\41\04"
  "\fc\0c\01\02\0b\87\80\80\80\00\00\20\00\11\00\02"
  "\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check" (i64.const 0x0)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x1)) "uninitialized element")
(assert_return (invoke "check" (i64.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check" (i64.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check" (i64.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check" (i64.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check" (i64.const 0x6)) "uninitialized element")
(assert_return (invoke "check" (i64.const 0x7)) (i32.const 0x2))
(assert_return (invoke "check" (i64.const 0x8)) (i32.const 0x7))
(assert_return (invoke "check" (i64.const 0x9)) (i32.const 0x1))
(assert_return (invoke "check" (i64.const 0xa)) (i32.const 0x8))
(assert_trap (invoke "check" (i64.const 0xb)) "uninitialized element")
(assert_return (invoke "check" (i64.const 0xc)) (i32.const 0x7))
(assert_return (invoke "check" (i64.const 0xd)) (i32.const 0x5))
(assert_return (invoke "check" (i64.const 0xe)) (i32.const 0x2))
(assert_return (invoke "check" (i64.const 0xf)) (i32.const 0x3))
(assert_return (invoke "check" (i64.const 0x10)) (i32.const 0x6))
(assert_trap (invoke "check" (i64.const 0x11)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x12)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x13)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x14)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x15)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x16)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x17)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x18)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x19)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7e\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\88\80\80\80\00"
  "\07\00\00\00\00\00\01\02\04\8d\80\80\80\00\03\70"
  "\01\1e\1e\70\01\1e\1e\70\05\1e\1e\07\90\80\80\80"
  "\00\02\04\74\65\73\74\00\0a\05\63\68\65\63\6b\00"
  "\0b\09\b9\80\80\80\00\04\02\02\42\02\0b\00\04\03"
  "\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01\0b\d2"
  "\08\0b\02\02\42\0c\0b\00\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\cb\80\80\80\00\07\84\80\80\80\00\00\41\05\0b"
  "\84\80\80\80\00\00\41\06\0b\84\80\80\80\00\00\41"
  "\07\0b\84\80\80\80\00\00\41\08\0b\84\80\80\80\00"
  "\00\41\09\0b\8c\80\80\80\00\00\42\0f\41\01\41\03"
  "\fc\0c\03\02\0b\87\80\80\80\00\00\20\00\11\00\02"
  "\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check" (i64.const 0x0)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x1)) "uninitialized element")
(assert_return (invoke "check" (i64.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check" (i64.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check" (i64.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check" (i64.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check" (i64.const 0x6)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x7)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x8)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x9)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0xa)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0xb)) "uninitialized element")
(assert_return (invoke "check" (i64.const 0xc)) (i32.const 0x7))
(assert_return (invoke "check" (i64.const 0xd)) (i32.const 0x5))
(assert_return (invoke "check" (i64.const 0xe)) (i32.const 0x2))
(assert_return (invoke "check" (i64.const 0xf)) (i32.const 0x9))
(assert_return (invoke "check" (i64.const 0x10)) (i32.const 0x2))
(assert_return (invoke "check" (i64.const 0x11)) (i32.const 0x7))
(assert_trap (invoke "check" (i64.const 0x12)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x13)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x14)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x15)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x16)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x17)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x18)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x19)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7e\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\88\80\80\80\00"
  "\07\00\00\00\00\00\01\02\04\8d\80\80\80\00\03\70"
  "\01\1e\1e\70\01\1e\1e\70\05\1e\1e\07\90\80\80\80"
  "\00\02\04\74\65\73\74\00\0a\05\63\68\65\63\6b\00"
  "\0b\09\b9\80\80\80\00\04\02\02\42\02\0b\00\04\03"
  "\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01\0b\d2"
  "\08\0b\02\02\42\0c\0b\00\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\8d\81\80\80\00\07\84\80\80\80\00\00\41\05\0b"
  "\84\80\80\80\00\00\41\06\0b\84\80\80\80\00\00\41"
  "\07\0b\84\80\80\80\00\00\41\08\0b\84\80\80\80\00"
  "\00\41\09\0b\ce\80\80\80\00\00\42\07\41\00\41\04"
  "\fc\0c\01\02\fc\0d\01\42\0f\41\01\41\03\fc\0c\03"
  "\02\fc\0d\03\42\14\42\0f\42\05\fc\0e\02\02\42\15"
  "\42\1d\42\01\fc\0e\02\02\42\18\42\0a\42\01\fc\0e"
  "\02\02\42\0d\42\0b\42\04\fc\0e\02\02\42\13\42\14"
  "\42\05\fc\0e\02\02\0b\87\80\80\80\00\00\20\00\11"
  "\00\02\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check" (i64.const 0x0)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x1)) "uninitialized element")
(assert_return (invoke "check" (i64.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check" (i64.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check" (i64.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check" (i64.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check" (i64.const 0x6)) "uninitialized element")
(assert_return (invoke "check" (i64.const 0x7)) (i32.const 0x2))
(assert_return (invoke "check" (i64.const 0x8)) (i32.const 0x7))
(assert_return (invoke "check" (i64.const 0x9)) (i32.const 0x1))
(assert_return (invoke "check" (i64.const 0xa)) (i32.const 0x8))
(assert_trap (invoke "check" (i64.const 0xb)) "uninitialized element")
(assert_return (invoke "check" (i64.const 0xc)) (i32.const 0x7))
(assert_trap (invoke "check" (i64.const 0xd)) "uninitialized element")
(assert_return (invoke "check" (i64.const 0xe)) (i32.const 0x7))
(assert_return (invoke "check" (i64.const 0xf)) (i32.const 0x5))
(assert_return (invoke "check" (i64.const 0x10)) (i32.const 0x2))
(assert_return (invoke "check" (i64.const 0x11)) (i32.const 0x7))
(assert_trap (invoke "check" (i64.const 0x12)) "uninitialized element")
(assert_return (invoke "check" (i64.const 0x13)) (i32.const 0x9))
(assert_trap (invoke "check" (i64.const 0x14)) "uninitialized element")
(assert_return (invoke "check" (i64.const 0x15)) (i32.const 0x7))
(assert_trap (invoke "check" (i64.const 0x16)) "uninitialized element")
(assert_return (invoke "check" (i64.const 0x17)) (i32.const 0x8))
(assert_return (invoke "check" (i64.const 0x18)) (i32.const 0x8))
(assert_trap (invoke "check" (i64.const 0x19)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check" (i64.const 0x1d)) "uninitialized element")
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\07\88\80\80\80\00"
    "\01\04\74\65\73\74\00\00\0a\8b\80\80\80\00\01\85"
    "\80\80\80\00\00\fc\0d\00\0b"
  )
  "unknown elem segment 0"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\07\88\80\80\80\00"
    "\01\04\74\65\73\74\00\00\0a\92\80\80\80\00\01\8c"
    "\80\80\80\00\00\41\0c\41\01\41\01\fc\0c\00\00\0b"
  )
  "unknown table 0"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
    "\00\01\7f\60\00\00\03\83\80\80\80\00\02\00\01\07"
    "\88\80\80\80\00\01\04\74\65\73\74\00\01\09\87\80"
    "\80\80\00\01\05\70\01\d2\00\0b\0a\94\80\80\80\00"
    "\02\84\80\80\80\00\00\41\00\0b\85\80\80\80\00\00"
    "\fc\0d\04\0b"
  )
  "unknown elem segment 4"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
    "\00\01\7f\60\00\00\03\83\80\80\80\00\02\00\01\07"
    "\88\80\80\80\00\01\04\74\65\73\74\00\01\09\87\80"
    "\80\80\00\01\05\70\01\d2\00\0b\0a\9b\80\80\80\00"
    "\02\84\80\80\80\00\00\41\00\0b\8c\80\80\80\00\00"
    "\41\0c\41\01\41\01\fc\0c\04\00\0b"
  )
  "unknown table 0"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\41\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\41\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\e5\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\85\80\80\80\00\00\fc\0d\02\0b"
)
(module instance)
(invoke "test")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\41\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\41\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\41\0c\41\01\41\01\fc\0c\02"
  "\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\41\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\41\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\f6\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\96\80\80\80\00\00\41\0c\41\01\41\01\fc\0c\01"
  "\00\41\15\41\01\41\01\fc\0c\01\00\0b"
)
(module instance)
(invoke "test")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\41\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\41\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\e8\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\88\80\80\80\00\00\fc\0d\01\fc\0d\01\0b"
)
(module instance)
(invoke "test")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\41\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\41\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ef\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8f\80\80\80\00\00\fc\0d\01\41\0c\41\01\41\01"
  "\fc\0c\01\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\41\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\41\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\41\0c\41\00\41\05\fc\0c\01"
  "\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\41\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\41\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\41\0c\41\02\41\03\fc\0c\01"
  "\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\41\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\41\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\41\1c\41\01\41\03\fc\0c\01"
  "\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\41\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\41\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\41\0c\41\04\41\00\fc\0c\01"
  "\00\0b"
)
(module instance)
(invoke "test")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\41\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\41\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\41\0c\41\05\41\00\fc\0c\01"
  "\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\41\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\41\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\41\1e\41\02\41\00\fc\0c\01"
  "\00\0b"
)
(module instance)
(invoke "test")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\41\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\41\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\41\1f\41\02\41\00\fc\0c\01"
  "\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\41\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\41\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\41\1e\41\04\41\00\fc\0c\01"
  "\00\0b"
)
(module instance)
(invoke "test")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\41\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\41\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\41\1f\41\05\41\00\fc\0c\01"
  "\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b9\80\80\80\00\04\02\01\41\02"
  "\0b\00\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b"
  "\d2\01\0b\d2\08\0b\02\01\41\0c\0b\00\05\07\05\02"
  "\03\06\05\70\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07"
  "\0b\d2\06\0b\0a\ec\80\80\80\00\0b\84\80\80\80\00"
  "\00\41\00\0b\84\80\80\80\00\00\41\01\0b\84\80\80"
  "\80\00\00\41\02\0b\84\80\80\80\00\00\41\03\0b\84"
  "\80\80\80\00\00\41\04\0b\84\80\80\80\00\00\41\05"
  "\0b\84\80\80\80\00\00\41\06\0b\84\80\80\80\00\00"
  "\41\07\0b\84\80\80\80\00\00\41\08\0b\84\80\80\80"
  "\00\00\41\09\0b\8c\80\80\80\00\00\41\1a\41\01\41"
  "\03\fc\0c\01\01\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b9\80\80\80\00\04\02\01\41\02"
  "\0b\00\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b"
  "\d2\01\0b\d2\08\0b\02\01\41\0c\0b\00\05\07\05\02"
  "\03\06\05\70\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07"
  "\0b\d2\06\0b\0a\ec\80\80\80\00\0b\84\80\80\80\00"
  "\00\41\00\0b\84\80\80\80\00\00\41\01\0b\84\80\80"
  "\80\00\00\41\02\0b\84\80\80\80\00\00\41\03\0b\84"
  "\80\80\80\00\00\41\04\0b\84\80\80\80\00\00\41\05"
  "\0b\84\80\80\80\00\00\41\06\0b\84\80\80\80\00\00"
  "\41\07\0b\84\80\80\80\00\00\41\08\0b\84\80\80\80"
  "\00\00\41\09\0b\8c\80\80\80\00\00\41\0c\41\04\41"
  "\00\fc\0c\01\01\0b"
)
(module instance)
(invoke "test")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b9\80\80\80\00\04\02\01\41\02"
  "\0b\00\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b"
  "\d2\01\0b\d2\08\0b\02\01\41\0c\0b\00\05\07\05\02"
  "\03\06\05\70\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07"
  "\0b\d2\06\0b\0a\ec\80\80\80\00\0b\84\80\80\80\00"
  "\00\41\00\0b\84\80\80\80\00\00\41\01\0b\84\80\80"
  "\80\00\00\41\02\0b\84\80\80\80\00\00\41\03\0b\84"
  "\80\80\80\00\00\41\04\0b\84\80\80\80\00\00\41\05"
  "\0b\84\80\80\80\00\00\41\06\0b\84\80\80\80\00\00"
  "\41\07\0b\84\80\80\80\00\00\41\08\0b\84\80\80\80"
  "\00\00\41\09\0b\8c\80\80\80\00\00\41\0c\41\05\41"
  "\00\fc\0c\01\01\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b9\80\80\80\00\04\02\01\41\02"
  "\0b\00\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b"
  "\d2\01\0b\d2\08\0b\02\01\41\0c\0b\00\05\07\05\02"
  "\03\06\05\70\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07"
  "\0b\d2\06\0b\0a\ec\80\80\80\00\0b\84\80\80\80\00"
  "\00\41\00\0b\84\80\80\80\00\00\41\01\0b\84\80\80"
  "\80\00\00\41\02\0b\84\80\80\80\00\00\41\03\0b\84"
  "\80\80\80\00\00\41\04\0b\84\80\80\80\00\00\41\05"
  "\0b\84\80\80\80\00\00\41\06\0b\84\80\80\80\00\00"
  "\41\07\0b\84\80\80\80\00\00\41\08\0b\84\80\80\80"
  "\00\00\41\09\0b\8c\80\80\80\00\00\41\1c\41\02\41"
  "\00\fc\0c\01\01\0b"
)
(module instance)
(invoke "test")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b9\80\80\80\00\04\02\01\41\02"
  "\0b\00\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b"
  "\d2\01\0b\d2\08\0b\02\01\41\0c\0b\00\05\07\05\02"
  "\03\06\05\70\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07"
  "\0b\d2\06\0b\0a\ec\80\80\80\00\0b\84\80\80\80\00"
  "\00\41\00\0b\84\80\80\80\00\00\41\01\0b\84\80\80"
  "\80\00\00\41\02\0b\84\80\80\80\00\00\41\03\0b\84"
  "\80\80\80\00\00\41\04\0b\84\80\80\80\00\00\41\05"
  "\0b\84\80\80\80\00\00\41\06\0b\84\80\80\80\00\00"
  "\41\07\0b\84\80\80\80\00\00\41\08\0b\84\80\80\80"
  "\00\00\41\09\0b\8c\80\80\80\00\00\41\1d\41\02\41"
  "\00\fc\0c\01\01\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b9\80\80\80\00\04\02\01\41\02"
  "\0b\00\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b"
  "\d2\01\0b\d2\08\0b\02\01\41\0c\0b\00\05\07\05\02"
  "\03\06\05\70\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07"
  "\0b\d2\06\0b\0a\ec\80\80\80\00\0b\84\80\80\80\00"
  "\00\41\00\0b\84\80\80\80\00\00\41\01\0b\84\80\80"
  "\80\00\00\41\02\0b\84\80\80\80\00\00\41\03\0b\84"
  "\80\80\80\00\00\41\04\0b\84\80\80\80\00\00\41\05"
  "\0b\84\80\80\80\00\00\41\06\0b\84\80\80\80\00\00"
  "\41\07\0b\84\80\80\80\00\00\41\08\0b\84\80\80\80"
  "\00\00\41\09\0b\8c\80\80\80\00\00\41\1c\41\04\41"
  "\00\fc\0c\01\01\0b"
)
(module instance)
(invoke "test")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\01\1e\1e\70\01\1c\1c\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b9\80\80\80\00\04\02\01\41\02"
  "\0b\00\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b"
  "\d2\01\0b\d2\08\0b\02\01\41\0c\0b\00\05\07\05\02"
  "\03\06\05\70\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07"
  "\0b\d2\06\0b\0a\ec\80\80\80\00\0b\84\80\80\80\00"
  "\00\41\00\0b\84\80\80\80\00\00\41\01\0b\84\80\80"
  "\80\00\00\41\02\0b\84\80\80\80\00\00\41\03\0b\84"
  "\80\80\80\00\00\41\04\0b\84\80\80\80\00\00\41\05"
  "\0b\84\80\80\80\00\00\41\06\0b\84\80\80\80\00\00"
  "\41\07\0b\84\80\80\80\00\00\41\08\0b\84\80\80\80"
  "\00\00\41\09\0b\8c\80\80\80\00\00\41\1d\41\05\41"
  "\00\fc\0c\01\01\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\9c\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8f\80\80\80\00\00\41\01\41\01\43\00"
    "\00\80\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\99\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8c\80\80\80\00\00\41\01\41\01\42\01"
    "\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a0\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\93\80\80\80\00\00\41\01\41\01\44\00"
    "\00\00\00\00\00\f0\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\9c\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8f\80\80\80\00\00\41\01\43\00\00\80"
    "\3f\41\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\9f\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\92\80\80\80\00\00\41\01\43\00\00\80"
    "\3f\43\00\00\80\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\9c\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8f\80\80\80\00\00\41\01\43\00\00\80"
    "\3f\42\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a3\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\96\80\80\80\00\00\41\01\43\00\00\80"
    "\3f\44\00\00\00\00\00\00\f0\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\99\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8c\80\80\80\00\00\41\01\42\01\41\01"
    "\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\9c\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8f\80\80\80\00\00\41\01\42\01\43\00"
    "\00\80\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\99\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8c\80\80\80\00\00\41\01\42\01\42\01"
    "\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a0\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\93\80\80\80\00\00\41\01\42\01\44\00"
    "\00\00\00\00\00\f0\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a0\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\93\80\80\80\00\00\41\01\44\00\00\00"
    "\00\00\00\f0\3f\41\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a3\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\96\80\80\80\00\00\41\01\44\00\00\00"
    "\00\00\00\f0\3f\43\00\00\80\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a0\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\93\80\80\80\00\00\41\01\44\00\00\00"
    "\00\00\00\f0\3f\42\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a7\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\9a\80\80\80\00\00\41\01\44\00\00\00"
    "\00\00\00\f0\3f\44\00\00\00\00\00\00\f0\3f\fc\0c"
    "\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\9c\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8f\80\80\80\00\00\43\00\00\80\3f\41"
    "\01\41\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\9f\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\92\80\80\80\00\00\43\00\00\80\3f\41"
    "\01\43\00\00\80\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\9c\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8f\80\80\80\00\00\43\00\00\80\3f\41"
    "\01\42\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a3\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\96\80\80\80\00\00\43\00\00\80\3f\41"
    "\01\44\00\00\00\00\00\00\f0\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\9f\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\92\80\80\80\00\00\43\00\00\80\3f\43"
    "\00\00\80\3f\41\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a2\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\95\80\80\80\00\00\43\00\00\80\3f\43"
    "\00\00\80\3f\43\00\00\80\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\9f\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\92\80\80\80\00\00\43\00\00\80\3f\43"
    "\00\00\80\3f\42\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a6\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\99\80\80\80\00\00\43\00\00\80\3f\43"
    "\00\00\80\3f\44\00\00\00\00\00\00\f0\3f\fc\0c\00"
    "\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\9c\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8f\80\80\80\00\00\43\00\00\80\3f\42"
    "\01\41\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\9f\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\92\80\80\80\00\00\43\00\00\80\3f\42"
    "\01\43\00\00\80\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\9c\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8f\80\80\80\00\00\43\00\00\80\3f\42"
    "\01\42\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a3\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\96\80\80\80\00\00\43\00\00\80\3f\42"
    "\01\44\00\00\00\00\00\00\f0\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a3\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\96\80\80\80\00\00\43\00\00\80\3f\44"
    "\00\00\00\00\00\00\f0\3f\41\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a6\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\99\80\80\80\00\00\43\00\00\80\3f\44"
    "\00\00\00\00\00\00\f0\3f\43\00\00\80\3f\fc\0c\00"
    "\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a3\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\96\80\80\80\00\00\43\00\00\80\3f\44"
    "\00\00\00\00\00\00\f0\3f\42\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\aa\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\9d\80\80\80\00\00\43\00\00\80\3f\44"
    "\00\00\00\00\00\00\f0\3f\44\00\00\00\00\00\00\f0"
    "\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\99\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8c\80\80\80\00\00\42\01\41\01\41\01"
    "\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\9c\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8f\80\80\80\00\00\42\01\41\01\43\00"
    "\00\80\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\99\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8c\80\80\80\00\00\42\01\41\01\42\01"
    "\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a0\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\93\80\80\80\00\00\42\01\41\01\44\00"
    "\00\00\00\00\00\f0\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\9c\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8f\80\80\80\00\00\42\01\43\00\00\80"
    "\3f\41\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\9f\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\92\80\80\80\00\00\42\01\43\00\00\80"
    "\3f\43\00\00\80\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\9c\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8f\80\80\80\00\00\42\01\43\00\00\80"
    "\3f\42\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a3\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\96\80\80\80\00\00\42\01\43\00\00\80"
    "\3f\44\00\00\00\00\00\00\f0\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\99\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8c\80\80\80\00\00\42\01\42\01\41\01"
    "\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\9c\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8f\80\80\80\00\00\42\01\42\01\43\00"
    "\00\80\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\99\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\8c\80\80\80\00\00\42\01\42\01\42\01"
    "\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a0\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\93\80\80\80\00\00\42\01\42\01\44\00"
    "\00\00\00\00\00\f0\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a0\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\93\80\80\80\00\00\42\01\44\00\00\00"
    "\00\00\00\f0\3f\41\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a3\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\96\80\80\80\00\00\42\01\44\00\00\00"
    "\00\00\00\f0\3f\43\00\00\80\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a0\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\93\80\80\80\00\00\42\01\44\00\00\00"
    "\00\00\00\f0\3f\42\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a7\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\9a\80\80\80\00\00\42\01\44\00\00\00"
    "\00\00\00\f0\3f\44\00\00\00\00\00\00\f0\3f\fc\0c"
    "\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a0\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\93\80\80\80\00\00\44\00\00\00\00\00"
    "\00\f0\3f\41\01\41\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a3\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\96\80\80\80\00\00\44\00\00\00\00\00"
    "\00\f0\3f\41\01\43\00\00\80\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a0\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\93\80\80\80\00\00\44\00\00\00\00\00"
    "\00\f0\3f\41\01\42\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a7\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\9a\80\80\80\00\00\44\00\00\00\00\00"
    "\00\f0\3f\41\01\44\00\00\00\00\00\00\f0\3f\fc\0c"
    "\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a3\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\96\80\80\80\00\00\44\00\00\00\00\00"
    "\00\f0\3f\43\00\00\80\3f\41\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a6\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\99\80\80\80\00\00\44\00\00\00\00\00"
    "\00\f0\3f\43\00\00\80\3f\43\00\00\80\3f\fc\0c\00"
    "\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a3\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\96\80\80\80\00\00\44\00\00\00\00\00"
    "\00\f0\3f\43\00\00\80\3f\42\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\aa\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\9d\80\80\80\00\00\44\00\00\00\00\00"
    "\00\f0\3f\43\00\00\80\3f\44\00\00\00\00\00\00\f0"
    "\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a0\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\93\80\80\80\00\00\44\00\00\00\00\00"
    "\00\f0\3f\42\01\41\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a3\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\96\80\80\80\00\00\44\00\00\00\00\00"
    "\00\f0\3f\42\01\43\00\00\80\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a0\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\93\80\80\80\00\00\44\00\00\00\00\00"
    "\00\f0\3f\42\01\42\01\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a7\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\9a\80\80\80\00\00\44\00\00\00\00\00"
    "\00\f0\3f\42\01\44\00\00\00\00\00\00\f0\3f\fc\0c"
    "\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a7\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\9a\80\80\80\00\00\44\00\00\00\00\00"
    "\00\f0\3f\44\00\00\00\00\00\00\f0\3f\41\01\fc\0c"
    "\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\aa\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\9d\80\80\80\00\00\44\00\00\00\00\00"
    "\00\f0\3f\44\00\00\00\00\00\00\f0\3f\43\00\00\80"
    "\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\a7\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\9a\80\80\80\00\00\44\00\00\00\00\00"
    "\00\f0\3f\44\00\00\00\00\00\00\f0\3f\42\01\fc\0c"
    "\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\83\80\80\80\00\02\00\00\04\84\80\80\80"
    "\00\01\70\00\0a\07\88\80\80\80\00\01\04\74\65\73"
    "\74\00\01\09\8d\80\80\80\00\01\05\70\03\d2\00\0b"
    "\d2\00\0b\d2\00\0b\0a\ae\80\80\80\00\02\82\80\80"
    "\80\00\00\0b\a1\80\80\80\00\00\44\00\00\00\00\00"
    "\00\f0\3f\44\00\00\00\00\00\00\f0\3f\44\00\00\00"
    "\00\00\00\f0\3f\fc\0c\00\00\0b"
  )
  "type mismatch"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\03\60"
  "\00\01\7f\60\01\7f\01\7f\60\02\7f\7f\00\03\93\80"
  "\80\80\00\12\00\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\01\02\04\85\80\80\80\00\01\70\01\20"
  "\40\07\e4\80\80\80\00\12\02\66\30\00\00\02\66\31"
  "\00\01\02\66\32\00\02\02\66\33\00\03\02\66\34\00"
  "\04\02\66\35\00\05\02\66\36\00\06\02\66\37\00\07"
  "\02\66\38\00\08\02\66\39\00\09\03\66\31\30\00\0a"
  "\03\66\31\31\00\0b\03\66\31\32\00\0c\03\66\31\33"
  "\00\0d\03\66\31\34\00\0e\03\66\31\35\00\0f\04\74"
  "\65\73\74\00\10\03\72\75\6e\00\11\09\b4\80\80\80"
  "\00\01\05\70\10\d2\00\0b\d2\01\0b\d2\02\0b\d2\03"
  "\0b\d2\04\0b\d2\05\0b\d2\06\0b\d2\07\0b\d2\08\0b"
  "\d2\09\0b\d2\0a\0b\d2\0b\0b\d2\0c\0b\d2\0d\0b\d2"
  "\0e\0b\d2\0f\0b\0a\ae\81\80\80\00\12\84\80\80\80"
  "\00\00\41\00\0b\84\80\80\80\00\00\41\01\0b\84\80"
  "\80\80\00\00\41\02\0b\84\80\80\80\00\00\41\03\0b"
  "\84\80\80\80\00\00\41\04\0b\84\80\80\80\00\00\41"
  "\05\0b\84\80\80\80\00\00\41\06\0b\84\80\80\80\00"
  "\00\41\07\0b\84\80\80\80\00\00\41\08\0b\84\80\80"
  "\80\00\00\41\09\0b\84\80\80\80\00\00\41\0a\0b\84"
  "\80\80\80\00\00\41\0b\0b\84\80\80\80\00\00\41\0c"
  "\0b\84\80\80\80\00\00\41\0d\0b\84\80\80\80\00\00"
  "\41\0e\0b\84\80\80\80\00\00\41\0f\0b\87\80\80\80"
  "\00\00\20\00\11\00\00\0b\8c\80\80\80\00\00\20\00"
  "\41\00\20\01\fc\0c\00\00\0b"
)
(module instance)
(assert_trap
  (invoke "run" (i32.const 0x18) (i32.const 0x10))
  "out of bounds table access"
)
(assert_trap (invoke "test" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x4)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x5)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xb)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xc)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xd)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xe)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xf)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1f)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\03\60"
  "\00\01\7f\60\01\7f\01\7f\60\02\7f\7f\00\03\93\80"
  "\80\80\00\12\00\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\01\02\04\85\80\80\80\00\01\70\01\20"
  "\40\07\e4\80\80\80\00\12\02\66\30\00\00\02\66\31"
  "\00\01\02\66\32\00\02\02\66\33\00\03\02\66\34\00"
  "\04\02\66\35\00\05\02\66\36\00\06\02\66\37\00\07"
  "\02\66\38\00\08\02\66\39\00\09\03\66\31\30\00\0a"
  "\03\66\31\31\00\0b\03\66\31\32\00\0c\03\66\31\33"
  "\00\0d\03\66\31\34\00\0e\03\66\31\35\00\0f\04\74"
  "\65\73\74\00\10\03\72\75\6e\00\11\09\b4\80\80\80"
  "\00\01\05\70\10\d2\00\0b\d2\01\0b\d2\02\0b\d2\03"
  "\0b\d2\04\0b\d2\05\0b\d2\06\0b\d2\07\0b\d2\08\0b"
  "\d2\09\0b\d2\0a\0b\d2\0b\0b\d2\0c\0b\d2\0d\0b\d2"
  "\0e\0b\d2\0f\0b\0a\ae\81\80\80\00\12\84\80\80\80"
  "\00\00\41\00\0b\84\80\80\80\00\00\41\01\0b\84\80"
  "\80\80\00\00\41\02\0b\84\80\80\80\00\00\41\03\0b"
  "\84\80\80\80\00\00\41\04\0b\84\80\80\80\00\00\41"
  "\05\0b\84\80\80\80\00\00\41\06\0b\84\80\80\80\00"
  "\00\41\07\0b\84\80\80\80\00\00\41\08\0b\84\80\80"
  "\80\00\00\41\09\0b\84\80\80\80\00\00\41\0a\0b\84"
  "\80\80\80\00\00\41\0b\0b\84\80\80\80\00\00\41\0c"
  "\0b\84\80\80\80\00\00\41\0d\0b\84\80\80\80\00\00"
  "\41\0e\0b\84\80\80\80\00\00\41\0f\0b\87\80\80\80"
  "\00\00\20\00\11\00\00\0b\8c\80\80\80\00\00\20\00"
  "\41\00\20\01\fc\0c\00\00\0b"
)
(module instance)
(assert_trap
  (invoke "run" (i32.const 0x19) (i32.const 0x10))
  "out of bounds table access"
)
(assert_trap (invoke "test" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x4)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x5)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xb)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xc)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xd)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xe)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xf)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1f)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\03\60"
  "\00\01\7f\60\01\7f\01\7f\60\02\7f\7f\00\03\93\80"
  "\80\80\00\12\00\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\01\02\04\87\80\80\80\00\01\70\01\a0"
  "\01\c0\02\07\e4\80\80\80\00\12\02\66\30\00\00\02"
  "\66\31\00\01\02\66\32\00\02\02\66\33\00\03\02\66"
  "\34\00\04\02\66\35\00\05\02\66\36\00\06\02\66\37"
  "\00\07\02\66\38\00\08\02\66\39\00\09\03\66\31\30"
  "\00\0a\03\66\31\31\00\0b\03\66\31\32\00\0c\03\66"
  "\31\33\00\0d\03\66\31\34\00\0e\03\66\31\35\00\0f"
  "\04\74\65\73\74\00\10\03\72\75\6e\00\11\09\b4\80"
  "\80\80\00\01\05\70\10\d2\00\0b\d2\01\0b\d2\02\0b"
  "\d2\03\0b\d2\04\0b\d2\05\0b\d2\06\0b\d2\07\0b\d2"
  "\08\0b\d2\09\0b\d2\0a\0b\d2\0b\0b\d2\0c\0b\d2\0d"
  "\0b\d2\0e\0b\d2\0f\0b\0a\ae\81\80\80\00\12\84\80"
  "\80\80\00\00\41\00\0b\84\80\80\80\00\00\41\01\0b"
  "\84\80\80\80\00\00\41\02\0b\84\80\80\80\00\00\41"
  "\03\0b\84\80\80\80\00\00\41\04\0b\84\80\80\80\00"
  "\00\41\05\0b\84\80\80\80\00\00\41\06\0b\84\80\80"
  "\80\00\00\41\07\0b\84\80\80\80\00\00\41\08\0b\84"
  "\80\80\80\00\00\41\09\0b\84\80\80\80\00\00\41\0a"
  "\0b\84\80\80\80\00\00\41\0b\0b\84\80\80\80\00\00"
  "\41\0c\0b\84\80\80\80\00\00\41\0d\0b\84\80\80\80"
  "\00\00\41\0e\0b\84\80\80\80\00\00\41\0f\0b\87\80"
  "\80\80\00\00\20\00\11\00\00\0b\8c\80\80\80\00\00"
  "\20\00\41\00\20\01\fc\0c\00\00\0b"
)
(module instance)
(assert_trap
  (invoke "run" (i32.const 0x60) (i32.const 0x20))
  "out of bounds table access"
)
(assert_trap (invoke "test" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x4)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x5)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xb)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xc)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xd)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xe)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xf)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1f)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x20)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x21)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x22)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x23)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x24)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x25)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x26)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x27)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x28)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x29)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2f)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x30)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x31)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x32)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x33)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x34)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x35)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x36)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x37)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x38)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x39)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3f)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x40)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x41)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x42)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x43)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x44)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x45)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x46)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x47)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x48)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x49)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x4a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x4b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x4c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x4d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x4e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x4f)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x50)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x51)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x52)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x53)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x54)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x55)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x56)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x57)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x58)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x59)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x5a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x5b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x5c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x5d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x5e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x5f)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x60)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x61)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x62)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x63)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x64)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x65)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x66)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x67)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x68)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x69)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x6a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x6b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x6c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x6d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x6e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x6f)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x70)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x71)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x72)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x73)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x74)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x75)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x76)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x77)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x78)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x79)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x7a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x7b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x7c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x7d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x7e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x7f)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x80)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x81)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x82)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x83)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x84)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x85)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x86)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x87)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x88)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x89)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x8a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x8b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x8c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x8d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x8e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x8f)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x90)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x91)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x92)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x93)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x94)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x95)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x96)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x97)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x98)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x99)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x9a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x9b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x9c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x9d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x9e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x9f)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\03\60"
  "\00\01\7f\60\01\7f\01\7f\60\02\7f\7f\00\03\93\80"
  "\80\80\00\12\00\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\01\02\04\87\80\80\80\00\01\70\01\a0"
  "\01\c0\02\07\e4\80\80\80\00\12\02\66\30\00\00\02"
  "\66\31\00\01\02\66\32\00\02\02\66\33\00\03\02\66"
  "\34\00\04\02\66\35\00\05\02\66\36\00\06\02\66\37"
  "\00\07\02\66\38\00\08\02\66\39\00\09\03\66\31\30"
  "\00\0a\03\66\31\31\00\0b\03\66\31\32\00\0c\03\66"
  "\31\33\00\0d\03\66\31\34\00\0e\03\66\31\35\00\0f"
  "\04\74\65\73\74\00\10\03\72\75\6e\00\11\09\b4\80"
  "\80\80\00\01\05\70\10\d2\00\0b\d2\01\0b\d2\02\0b"
  "\d2\03\0b\d2\04\0b\d2\05\0b\d2\06\0b\d2\07\0b\d2"
  "\08\0b\d2\09\0b\d2\0a\0b\d2\0b\0b\d2\0c\0b\d2\0d"
  "\0b\d2\0e\0b\d2\0f\0b\0a\ae\81\80\80\00\12\84\80"
  "\80\80\00\00\41\00\0b\84\80\80\80\00\00\41\01\0b"
  "\84\80\80\80\00\00\41\02\0b\84\80\80\80\00\00\41"
  "\03\0b\84\80\80\80\00\00\41\04\0b\84\80\80\80\00"
  "\00\41\05\0b\84\80\80\80\00\00\41\06\0b\84\80\80"
  "\80\00\00\41\07\0b\84\80\80\80\00\00\41\08\0b\84"
  "\80\80\80\00\00\41\09\0b\84\80\80\80\00\00\41\0a"
  "\0b\84\80\80\80\00\00\41\0b\0b\84\80\80\80\00\00"
  "\41\0c\0b\84\80\80\80\00\00\41\0d\0b\84\80\80\80"
  "\00\00\41\0e\0b\84\80\80\80\00\00\41\0f\0b\87\80"
  "\80\80\00\00\20\00\11\00\00\0b\8c\80\80\80\00\00"
  "\20\00\41\00\20\01\fc\0c\00\00\0b"
)
(module instance)
(assert_trap
  (invoke "run" (i32.const 0x61) (i32.const 0x1f))
  "out of bounds table access"
)
(assert_trap (invoke "test" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x4)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x5)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xb)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xc)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xd)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xe)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xf)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1f)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x20)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x21)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x22)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x23)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x24)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x25)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x26)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x27)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x28)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x29)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2f)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x30)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x31)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x32)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x33)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x34)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x35)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x36)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x37)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x38)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x39)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3f)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x40)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x41)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x42)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x43)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x44)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x45)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x46)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x47)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x48)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x49)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x4a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x4b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x4c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x4d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x4e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x4f)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x50)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x51)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x52)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x53)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x54)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x55)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x56)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x57)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x58)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x59)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x5a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x5b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x5c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x5d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x5e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x5f)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x60)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x61)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x62)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x63)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x64)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x65)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x66)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x67)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x68)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x69)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x6a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x6b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x6c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x6d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x6e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x6f)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x70)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x71)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x72)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x73)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x74)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x75)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x76)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x77)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x78)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x79)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x7a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x7b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x7c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x7d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x7e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x7f)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x80)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x81)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x82)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x83)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x84)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x85)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x86)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x87)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x88)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x89)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x8a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x8b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x8c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x8d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x8e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x8f)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x90)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x91)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x92)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x93)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x94)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x95)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x96)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x97)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x98)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x99)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x9a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x9b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x9c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x9d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x9e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x9f)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\03\60"
  "\00\01\7f\60\01\7f\01\7f\60\02\7f\7f\00\03\93\80"
  "\80\80\00\12\00\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\01\02\04\85\80\80\80\00\01\70\01\40"
  "\40\07\e4\80\80\80\00\12\02\66\30\00\00\02\66\31"
  "\00\01\02\66\32\00\02\02\66\33\00\03\02\66\34\00"
  "\04\02\66\35\00\05\02\66\36\00\06\02\66\37\00\07"
  "\02\66\38\00\08\02\66\39\00\09\03\66\31\30\00\0a"
  "\03\66\31\31\00\0b\03\66\31\32\00\0c\03\66\31\33"
  "\00\0d\03\66\31\34\00\0e\03\66\31\35\00\0f\04\74"
  "\65\73\74\00\10\03\72\75\6e\00\11\09\b4\80\80\80"
  "\00\01\05\70\10\d2\00\0b\d2\01\0b\d2\02\0b\d2\03"
  "\0b\d2\04\0b\d2\05\0b\d2\06\0b\d2\07\0b\d2\08\0b"
  "\d2\09\0b\d2\0a\0b\d2\0b\0b\d2\0c\0b\d2\0d\0b\d2"
  "\0e\0b\d2\0f\0b\0a\ae\81\80\80\00\12\84\80\80\80"
  "\00\00\41\00\0b\84\80\80\80\00\00\41\01\0b\84\80"
  "\80\80\00\00\41\02\0b\84\80\80\80\00\00\41\03\0b"
  "\84\80\80\80\00\00\41\04\0b\84\80\80\80\00\00\41"
  "\05\0b\84\80\80\80\00\00\41\06\0b\84\80\80\80\00"
  "\00\41\07\0b\84\80\80\80\00\00\41\08\0b\84\80\80"
  "\80\00\00\41\09\0b\84\80\80\80\00\00\41\0a\0b\84"
  "\80\80\80\00\00\41\0b\0b\84\80\80\80\00\00\41\0c"
  "\0b\84\80\80\80\00\00\41\0d\0b\84\80\80\80\00\00"
  "\41\0e\0b\84\80\80\80\00\00\41\0f\0b\87\80\80\80"
  "\00\00\20\00\11\00\00\0b\8c\80\80\80\00\00\20\00"
  "\41\00\20\01\fc\0c\00\00\0b"
)
(module instance)
(assert_trap
  (invoke "run" (i32.const 0x30) (i32.const 0xffff_fff0))
  "out of bounds table access"
)
(assert_trap (invoke "test" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x4)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x5)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xb)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xc)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xd)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xe)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xf)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1f)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x20)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x21)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x22)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x23)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x24)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x25)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x26)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x27)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x28)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x29)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2f)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x30)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x31)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x32)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x33)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x34)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x35)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x36)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x37)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x38)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x39)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3a)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3b)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3c)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3f)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\03\60"
  "\00\01\7f\60\01\7f\01\7f\60\02\7f\7f\00\03\93\80"
  "\80\80\00\12\00\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\01\02\04\85\80\80\80\00\01\70\01\10"
  "\10\07\e4\80\80\80\00\12\02\66\30\00\00\02\66\31"
  "\00\01\02\66\32\00\02\02\66\33\00\03\02\66\34\00"
  "\04\02\66\35\00\05\02\66\36\00\06\02\66\37\00\07"
  "\02\66\38\00\08\02\66\39\00\09\03\66\31\30\00\0a"
  "\03\66\31\31\00\0b\03\66\31\32\00\0c\03\66\31\33"
  "\00\0d\03\66\31\34\00\0e\03\66\31\35\00\0f\04\74"
  "\65\73\74\00\10\03\72\75\6e\00\11\09\b4\80\80\80"
  "\00\01\05\70\10\d2\00\0b\d2\01\0b\d2\02\0b\d2\03"
  "\0b\d2\04\0b\d2\05\0b\d2\06\0b\d2\07\0b\d2\08\0b"
  "\d2\09\0b\d2\0a\0b\d2\0b\0b\d2\0c\0b\d2\0d\0b\d2"
  "\0e\0b\d2\0f\0b\0a\ae\81\80\80\00\12\84\80\80\80"
  "\00\00\41\00\0b\84\80\80\80\00\00\41\01\0b\84\80"
  "\80\80\00\00\41\02\0b\84\80\80\80\00\00\41\03\0b"
  "\84\80\80\80\00\00\41\04\0b\84\80\80\80\00\00\41"
  "\05\0b\84\80\80\80\00\00\41\06\0b\84\80\80\80\00"
  "\00\41\07\0b\84\80\80\80\00\00\41\08\0b\84\80\80"
  "\80\00\00\41\09\0b\84\80\80\80\00\00\41\0a\0b\84"
  "\80\80\80\00\00\41\0b\0b\84\80\80\80\00\00\41\0c"
  "\0b\84\80\80\80\00\00\41\0d\0b\84\80\80\80\00\00"
  "\41\0e\0b\84\80\80\80\00\00\41\0f\0b\87\80\80\80"
  "\00\00\20\00\11\00\00\0b\8c\80\80\80\00\00\20\00"
  "\41\08\20\01\fc\0c\00\00\0b"
)
(module instance)
(assert_trap
  (invoke "run" (i32.const 0x0) (i32.const 0xffff_fffc))
  "out of bounds table access"
)
(assert_trap (invoke "test" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x2)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x3)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x4)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x5)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xb)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xc)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xd)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xe)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0xf)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\03\82\80\80\80\00\01\00\04\84\80\80\80\00"
  "\01\70\00\01\09\c4\81\80\80\00\41\05\70\00\05\70"
  "\00\05\70\00\05\70\00\05\70\00\05\70\00\05\70\00"
  "\05\70\00\05\70\00\05\70\00\05\70\00\05\70\00\05"
  "\70\00\05\70\00\05\70\00\05\70\00\05\70\00\05\70"
  "\00\05\70\00\05\70\00\05\70\00\05\70\00\05\70\00"
  "\05\70\00\05\70\00\05\70\00\05\70\00\05\70\00\05"
  "\70\00\05\70\00\05\70\00\05\70\00\05\70\00\05\70"
  "\00\05\70\00\05\70\00\05\70\00\05\70\00\05\70\00"
  "\05\70\00\05\70\00\05\70\00\05\70\00\05\70\00\05"
  "\70\00\05\70\00\05\70\00\05\70\00\05\70\00\05\70"
  "\00\05\70\00\05\70\00\05\70\00\05\70\00\05\70\00"
  "\05\70\00\05\70\00\05\70\00\05\70\00\05\70\00\05"
  "\70\00\05\70\00\05\70\00\05\70\00\05\70\00\0a\92"
  "\80\80\80\00\01\8c\80\80\80\00\00\41\00\41\00\41"
  "\00\fc\0c\40\00\0b"
)
(module instance)
