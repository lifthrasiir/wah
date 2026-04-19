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
  "\00\00\01\61\03\65\66\34\00\00\03\89\80\80\80\00"
  "\08\00\00\00\00\00\01\02\02\04\89\80\80\80\00\02"
  "\70\01\1e\1e\70\01\1e\1e\07\9e\80\80\80\00\03\04"
  "\74\65\73\74\00\0a\08\63\68\65\63\6b\5f\74\30\00"
  "\0b\08\63\68\65\63\6b\5f\74\31\00\0c\09\cc\80\80"
  "\80\00\06\00\41\02\0b\04\03\01\04\01\05\70\04\d2"
  "\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\00\41\0c\0b\05"
  "\07\05\02\03\06\05\70\05\d2\05\0b\d2\09\0b\d2\02"
  "\0b\d2\07\0b\d2\06\0b\02\01\41\03\0b\00\04\01\03"
  "\01\04\02\01\41\0b\0b\00\05\06\03\02\05\07\0a\ce"
  "\80\80\80\00\08\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\83\80\80\80\00\00\01\0b\87\80\80\80\00\00"
  "\20\00\11\00\00\0b\87\80\80\80\00\00\20\00\11\00"
  "\01\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check_t0" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0xc)) (i32.const 0x7))
(assert_return (invoke "check_t0" (i32.const 0xd)) (i32.const 0x5))
(assert_return (invoke "check_t0" (i32.const 0xe)) (i32.const 0x2))
(assert_return (invoke "check_t0" (i32.const 0xf)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x10)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x2)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x4)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x6)) (i32.const 0x4))
(assert_trap (invoke "check_t1" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xa)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0xb)) (i32.const 0x6))
(assert_return (invoke "check_t1" (i32.const 0xc)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0xd)) (i32.const 0x2))
(assert_return (invoke "check_t1" (i32.const 0xe)) (i32.const 0x5))
(assert_return (invoke "check_t1" (i32.const 0xf)) (i32.const 0x7))
(assert_trap (invoke "check_t1" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\89\80\80\80\00"
  "\08\00\00\00\00\00\01\02\02\04\89\80\80\80\00\02"
  "\70\01\1e\1e\70\01\1e\1e\07\9e\80\80\80\00\03\04"
  "\74\65\73\74\00\0a\08\63\68\65\63\6b\5f\74\30\00"
  "\0b\08\63\68\65\63\6b\5f\74\31\00\0c\09\cc\80\80"
  "\80\00\06\00\41\02\0b\04\03\01\04\01\05\70\04\d2"
  "\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\00\41\0c\0b\05"
  "\07\05\02\03\06\05\70\05\d2\05\0b\d2\09\0b\d2\02"
  "\0b\d2\07\0b\d2\06\0b\02\01\41\03\0b\00\04\01\03"
  "\01\04\02\01\41\0b\0b\00\05\06\03\02\05\07\0a\d7"
  "\80\80\80\00\08\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\8c\80\80\80\00\00\41\0d\41\02\41\03\fc\0e"
  "\00\00\0b\87\80\80\80\00\00\20\00\11\00\00\0b\87"
  "\80\80\80\00\00\20\00\11\00\01\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check_t0" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0xc)) (i32.const 0x7))
(assert_return (invoke "check_t0" (i32.const 0xd)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0xe)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0xf)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x10)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x2)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x4)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x6)) (i32.const 0x4))
(assert_trap (invoke "check_t1" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xa)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0xb)) (i32.const 0x6))
(assert_return (invoke "check_t1" (i32.const 0xc)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0xd)) (i32.const 0x2))
(assert_return (invoke "check_t1" (i32.const 0xe)) (i32.const 0x5))
(assert_return (invoke "check_t1" (i32.const 0xf)) (i32.const 0x7))
(assert_trap (invoke "check_t1" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\89\80\80\80\00"
  "\08\00\00\00\00\00\01\02\02\04\89\80\80\80\00\02"
  "\70\01\1e\1e\70\01\1e\1e\07\9e\80\80\80\00\03\04"
  "\74\65\73\74\00\0a\08\63\68\65\63\6b\5f\74\30\00"
  "\0b\08\63\68\65\63\6b\5f\74\31\00\0c\09\cc\80\80"
  "\80\00\06\00\41\02\0b\04\03\01\04\01\05\70\04\d2"
  "\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\00\41\0c\0b\05"
  "\07\05\02\03\06\05\70\05\d2\05\0b\d2\09\0b\d2\02"
  "\0b\d2\07\0b\d2\06\0b\02\01\41\03\0b\00\04\01\03"
  "\01\04\02\01\41\0b\0b\00\05\06\03\02\05\07\0a\d7"
  "\80\80\80\00\08\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\8c\80\80\80\00\00\41\19\41\0f\41\02\fc\0e"
  "\00\00\0b\87\80\80\80\00\00\20\00\11\00\00\0b\87"
  "\80\80\80\00\00\20\00\11\00\01\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check_t0" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0xc)) (i32.const 0x7))
(assert_return (invoke "check_t0" (i32.const 0xd)) (i32.const 0x5))
(assert_return (invoke "check_t0" (i32.const 0xe)) (i32.const 0x2))
(assert_return (invoke "check_t0" (i32.const 0xf)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x10)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x18)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x19)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x1a)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x2)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x4)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x6)) (i32.const 0x4))
(assert_trap (invoke "check_t1" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xa)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0xb)) (i32.const 0x6))
(assert_return (invoke "check_t1" (i32.const 0xc)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0xd)) (i32.const 0x2))
(assert_return (invoke "check_t1" (i32.const 0xe)) (i32.const 0x5))
(assert_return (invoke "check_t1" (i32.const 0xf)) (i32.const 0x7))
(assert_trap (invoke "check_t1" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\89\80\80\80\00"
  "\08\00\00\00\00\00\01\02\02\04\89\80\80\80\00\02"
  "\70\01\1e\1e\70\01\1e\1e\07\9e\80\80\80\00\03\04"
  "\74\65\73\74\00\0a\08\63\68\65\63\6b\5f\74\30\00"
  "\0b\08\63\68\65\63\6b\5f\74\31\00\0c\09\cc\80\80"
  "\80\00\06\00\41\02\0b\04\03\01\04\01\05\70\04\d2"
  "\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\00\41\0c\0b\05"
  "\07\05\02\03\06\05\70\05\d2\05\0b\d2\09\0b\d2\02"
  "\0b\d2\07\0b\d2\06\0b\02\01\41\03\0b\00\04\01\03"
  "\01\04\02\01\41\0b\0b\00\05\06\03\02\05\07\0a\d7"
  "\80\80\80\00\08\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\8c\80\80\80\00\00\41\0d\41\19\41\03\fc\0e"
  "\00\00\0b\87\80\80\80\00\00\20\00\11\00\00\0b\87"
  "\80\80\80\00\00\20\00\11\00\01\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check_t0" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0xc)) (i32.const 0x7))
(assert_trap (invoke "check_t0" (i32.const 0xd)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xe)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xf)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x10)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x2)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x4)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x6)) (i32.const 0x4))
(assert_trap (invoke "check_t1" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xa)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0xb)) (i32.const 0x6))
(assert_return (invoke "check_t1" (i32.const 0xc)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0xd)) (i32.const 0x2))
(assert_return (invoke "check_t1" (i32.const 0xe)) (i32.const 0x5))
(assert_return (invoke "check_t1" (i32.const 0xf)) (i32.const 0x7))
(assert_trap (invoke "check_t1" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\89\80\80\80\00"
  "\08\00\00\00\00\00\01\02\02\04\89\80\80\80\00\02"
  "\70\01\1e\1e\70\01\1e\1e\07\9e\80\80\80\00\03\04"
  "\74\65\73\74\00\0a\08\63\68\65\63\6b\5f\74\30\00"
  "\0b\08\63\68\65\63\6b\5f\74\31\00\0c\09\cc\80\80"
  "\80\00\06\00\41\02\0b\04\03\01\04\01\05\70\04\d2"
  "\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\00\41\0c\0b\05"
  "\07\05\02\03\06\05\70\05\d2\05\0b\d2\09\0b\d2\02"
  "\0b\d2\07\0b\d2\06\0b\02\01\41\03\0b\00\04\01\03"
  "\01\04\02\01\41\0b\0b\00\05\06\03\02\05\07\0a\d7"
  "\80\80\80\00\08\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\8c\80\80\80\00\00\41\14\41\16\41\04\fc\0e"
  "\00\00\0b\87\80\80\80\00\00\20\00\11\00\00\0b\87"
  "\80\80\80\00\00\20\00\11\00\01\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check_t0" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0xc)) (i32.const 0x7))
(assert_return (invoke "check_t0" (i32.const 0xd)) (i32.const 0x5))
(assert_return (invoke "check_t0" (i32.const 0xe)) (i32.const 0x2))
(assert_return (invoke "check_t0" (i32.const 0xf)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x10)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x2)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x4)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x6)) (i32.const 0x4))
(assert_trap (invoke "check_t1" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xa)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0xb)) (i32.const 0x6))
(assert_return (invoke "check_t1" (i32.const 0xc)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0xd)) (i32.const 0x2))
(assert_return (invoke "check_t1" (i32.const 0xe)) (i32.const 0x5))
(assert_return (invoke "check_t1" (i32.const 0xf)) (i32.const 0x7))
(assert_trap (invoke "check_t1" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\89\80\80\80\00"
  "\08\00\00\00\00\00\01\02\02\04\89\80\80\80\00\02"
  "\70\01\1e\1e\70\01\1e\1e\07\9e\80\80\80\00\03\04"
  "\74\65\73\74\00\0a\08\63\68\65\63\6b\5f\74\30\00"
  "\0b\08\63\68\65\63\6b\5f\74\31\00\0c\09\cc\80\80"
  "\80\00\06\00\41\02\0b\04\03\01\04\01\05\70\04\d2"
  "\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\00\41\0c\0b\05"
  "\07\05\02\03\06\05\70\05\d2\05\0b\d2\09\0b\d2\02"
  "\0b\d2\07\0b\d2\06\0b\02\01\41\03\0b\00\04\01\03"
  "\01\04\02\01\41\0b\0b\00\05\06\03\02\05\07\0a\d7"
  "\80\80\80\00\08\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\8c\80\80\80\00\00\41\19\41\01\41\03\fc\0e"
  "\00\00\0b\87\80\80\80\00\00\20\00\11\00\00\0b\87"
  "\80\80\80\00\00\20\00\11\00\01\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check_t0" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0xc)) (i32.const 0x7))
(assert_return (invoke "check_t0" (i32.const 0xd)) (i32.const 0x5))
(assert_return (invoke "check_t0" (i32.const 0xe)) (i32.const 0x2))
(assert_return (invoke "check_t0" (i32.const 0xf)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x10)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x19)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x1a)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x1b)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x2)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x4)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x6)) (i32.const 0x4))
(assert_trap (invoke "check_t1" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xa)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0xb)) (i32.const 0x6))
(assert_return (invoke "check_t1" (i32.const 0xc)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0xd)) (i32.const 0x2))
(assert_return (invoke "check_t1" (i32.const 0xe)) (i32.const 0x5))
(assert_return (invoke "check_t1" (i32.const 0xf)) (i32.const 0x7))
(assert_trap (invoke "check_t1" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\89\80\80\80\00"
  "\08\00\00\00\00\00\01\02\02\04\89\80\80\80\00\02"
  "\70\01\1e\1e\70\01\1e\1e\07\9e\80\80\80\00\03\04"
  "\74\65\73\74\00\0a\08\63\68\65\63\6b\5f\74\30\00"
  "\0b\08\63\68\65\63\6b\5f\74\31\00\0c\09\cc\80\80"
  "\80\00\06\00\41\02\0b\04\03\01\04\01\05\70\04\d2"
  "\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\00\41\0c\0b\05"
  "\07\05\02\03\06\05\70\05\d2\05\0b\d2\09\0b\d2\02"
  "\0b\d2\07\0b\d2\06\0b\02\01\41\03\0b\00\04\01\03"
  "\01\04\02\01\41\0b\0b\00\05\06\03\02\05\07\0a\d7"
  "\80\80\80\00\08\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\8c\80\80\80\00\00\41\0a\41\0c\41\07\fc\0e"
  "\00\00\0b\87\80\80\80\00\00\20\00\11\00\00\0b\87"
  "\80\80\80\00\00\20\00\11\00\01\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check_t0" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x9)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0xa)) (i32.const 0x7))
(assert_return (invoke "check_t0" (i32.const 0xb)) (i32.const 0x5))
(assert_return (invoke "check_t0" (i32.const 0xc)) (i32.const 0x2))
(assert_return (invoke "check_t0" (i32.const 0xd)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0xe)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0xf)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x2)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x4)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x6)) (i32.const 0x4))
(assert_trap (invoke "check_t1" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xa)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0xb)) (i32.const 0x6))
(assert_return (invoke "check_t1" (i32.const 0xc)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0xd)) (i32.const 0x2))
(assert_return (invoke "check_t1" (i32.const 0xe)) (i32.const 0x5))
(assert_return (invoke "check_t1" (i32.const 0xf)) (i32.const 0x7))
(assert_trap (invoke "check_t1" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\89\80\80\80\00"
  "\08\00\00\00\00\00\01\02\02\04\89\80\80\80\00\02"
  "\70\01\1e\1e\70\01\1e\1e\07\9e\80\80\80\00\03\04"
  "\74\65\73\74\00\0a\08\63\68\65\63\6b\5f\74\30\00"
  "\0b\08\63\68\65\63\6b\5f\74\31\00\0c\09\cc\80\80"
  "\80\00\06\00\41\02\0b\04\03\01\04\01\05\70\04\d2"
  "\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\00\41\0c\0b\05"
  "\07\05\02\03\06\05\70\05\d2\05\0b\d2\09\0b\d2\02"
  "\0b\d2\07\0b\d2\06\0b\02\01\41\03\0b\00\04\01\03"
  "\01\04\02\01\41\0b\0b\00\05\06\03\02\05\07\0a\d7"
  "\80\80\80\00\08\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\8c\80\80\80\00\00\41\0c\41\0a\41\07\fc\0e"
  "\00\00\0b\87\80\80\80\00\00\20\00\11\00\00\0b\87"
  "\80\80\80\00\00\20\00\11\00\01\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check_t0" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xb)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xc)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xd)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0xe)) (i32.const 0x7))
(assert_return (invoke "check_t0" (i32.const 0xf)) (i32.const 0x5))
(assert_return (invoke "check_t0" (i32.const 0x10)) (i32.const 0x2))
(assert_return (invoke "check_t0" (i32.const 0x11)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x12)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x2)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x4)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x6)) (i32.const 0x4))
(assert_trap (invoke "check_t1" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xa)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0xb)) (i32.const 0x6))
(assert_return (invoke "check_t1" (i32.const 0xc)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0xd)) (i32.const 0x2))
(assert_return (invoke "check_t1" (i32.const 0xe)) (i32.const 0x5))
(assert_return (invoke "check_t1" (i32.const 0xf)) (i32.const 0x7))
(assert_trap (invoke "check_t1" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\89\80\80\80\00"
  "\08\00\00\00\00\00\01\02\02\04\89\80\80\80\00\02"
  "\70\01\1e\1e\70\01\1e\1e\07\9e\80\80\80\00\03\04"
  "\74\65\73\74\00\0a\08\63\68\65\63\6b\5f\74\30\00"
  "\0b\08\63\68\65\63\6b\5f\74\31\00\0c\09\cc\80\80"
  "\80\00\06\00\41\02\0b\04\03\01\04\01\05\70\04\d2"
  "\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\00\41\0c\0b\05"
  "\07\05\02\03\06\05\70\05\d2\05\0b\d2\09\0b\d2\02"
  "\0b\d2\07\0b\d2\06\0b\02\01\41\03\0b\00\04\01\03"
  "\01\04\02\01\41\0b\0b\00\05\06\03\02\05\07\0a\d7"
  "\80\80\80\00\08\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\8c\80\80\80\00\00\41\0a\41\00\41\14\fc\0e"
  "\01\00\0b\87\80\80\80\00\00\20\00\11\00\00\0b\87"
  "\80\80\80\00\00\20\00\11\00\01\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check_t0" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0xc)) (i32.const 0x7))
(assert_return (invoke "check_t0" (i32.const 0xd)) (i32.const 0x5))
(assert_return (invoke "check_t0" (i32.const 0xe)) (i32.const 0x2))
(assert_return (invoke "check_t0" (i32.const 0xf)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x10)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x2)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x4)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x6)) (i32.const 0x4))
(assert_trap (invoke "check_t1" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0xc)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0xd)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0xe)) (i32.const 0x4))
(assert_return (invoke "check_t1" (i32.const 0xf)) (i32.const 0x1))
(assert_trap (invoke "check_t1" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x15)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x16)) (i32.const 0x7))
(assert_return (invoke "check_t1" (i32.const 0x17)) (i32.const 0x5))
(assert_return (invoke "check_t1" (i32.const 0x18)) (i32.const 0x2))
(assert_return (invoke "check_t1" (i32.const 0x19)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x1a)) (i32.const 0x6))
(assert_trap (invoke "check_t1" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\89\80\80\80\00"
  "\08\00\00\00\00\00\01\02\02\04\89\80\80\80\00\02"
  "\70\01\1e\1e\70\01\1e\1e\07\9e\80\80\80\00\03\04"
  "\74\65\73\74\00\0a\08\63\68\65\63\6b\5f\74\30\00"
  "\0b\08\63\68\65\63\6b\5f\74\31\00\0c\09\cc\80\80"
  "\80\00\06\02\01\41\02\0b\00\04\03\01\04\01\05\70"
  "\04\d2\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\02\01\41"
  "\0c\0b\00\05\07\05\02\03\06\05\70\05\d2\05\0b\d2"
  "\09\0b\d2\02\0b\d2\07\0b\d2\06\0b\00\41\03\0b\04"
  "\01\03\01\04\00\41\0b\0b\05\06\03\02\05\07\0a\ce"
  "\80\80\80\00\08\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\83\80\80\80\00\00\01\0b\87\80\80\80\00\00"
  "\20\00\11\00\01\0b\87\80\80\80\00\00\20\00\11\00"
  "\00\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check_t0" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0xc)) (i32.const 0x7))
(assert_return (invoke "check_t0" (i32.const 0xd)) (i32.const 0x5))
(assert_return (invoke "check_t0" (i32.const 0xe)) (i32.const 0x2))
(assert_return (invoke "check_t0" (i32.const 0xf)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x10)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x2)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x4)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x6)) (i32.const 0x4))
(assert_trap (invoke "check_t1" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xa)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0xb)) (i32.const 0x6))
(assert_return (invoke "check_t1" (i32.const 0xc)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0xd)) (i32.const 0x2))
(assert_return (invoke "check_t1" (i32.const 0xe)) (i32.const 0x5))
(assert_return (invoke "check_t1" (i32.const 0xf)) (i32.const 0x7))
(assert_trap (invoke "check_t1" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\89\80\80\80\00"
  "\08\00\00\00\00\00\01\02\02\04\89\80\80\80\00\02"
  "\70\01\1e\1e\70\01\1e\1e\07\9e\80\80\80\00\03\04"
  "\74\65\73\74\00\0a\08\63\68\65\63\6b\5f\74\30\00"
  "\0b\08\63\68\65\63\6b\5f\74\31\00\0c\09\cc\80\80"
  "\80\00\06\02\01\41\02\0b\00\04\03\01\04\01\05\70"
  "\04\d2\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\02\01\41"
  "\0c\0b\00\05\07\05\02\03\06\05\70\05\d2\05\0b\d2"
  "\09\0b\d2\02\0b\d2\07\0b\d2\06\0b\00\41\03\0b\04"
  "\01\03\01\04\00\41\0b\0b\05\06\03\02\05\07\0a\d7"
  "\80\80\80\00\08\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\8c\80\80\80\00\00\41\0d\41\02\41\03\fc\0e"
  "\01\01\0b\87\80\80\80\00\00\20\00\11\00\01\0b\87"
  "\80\80\80\00\00\20\00\11\00\00\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check_t0" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0xc)) (i32.const 0x7))
(assert_return (invoke "check_t0" (i32.const 0xd)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0xe)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0xf)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x10)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x2)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x4)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x6)) (i32.const 0x4))
(assert_trap (invoke "check_t1" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xa)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0xb)) (i32.const 0x6))
(assert_return (invoke "check_t1" (i32.const 0xc)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0xd)) (i32.const 0x2))
(assert_return (invoke "check_t1" (i32.const 0xe)) (i32.const 0x5))
(assert_return (invoke "check_t1" (i32.const 0xf)) (i32.const 0x7))
(assert_trap (invoke "check_t1" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\89\80\80\80\00"
  "\08\00\00\00\00\00\01\02\02\04\89\80\80\80\00\02"
  "\70\01\1e\1e\70\01\1e\1e\07\9e\80\80\80\00\03\04"
  "\74\65\73\74\00\0a\08\63\68\65\63\6b\5f\74\30\00"
  "\0b\08\63\68\65\63\6b\5f\74\31\00\0c\09\cc\80\80"
  "\80\00\06\02\01\41\02\0b\00\04\03\01\04\01\05\70"
  "\04\d2\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\02\01\41"
  "\0c\0b\00\05\07\05\02\03\06\05\70\05\d2\05\0b\d2"
  "\09\0b\d2\02\0b\d2\07\0b\d2\06\0b\00\41\03\0b\04"
  "\01\03\01\04\00\41\0b\0b\05\06\03\02\05\07\0a\d7"
  "\80\80\80\00\08\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\8c\80\80\80\00\00\41\19\41\0f\41\02\fc\0e"
  "\01\01\0b\87\80\80\80\00\00\20\00\11\00\01\0b\87"
  "\80\80\80\00\00\20\00\11\00\00\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check_t0" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0xc)) (i32.const 0x7))
(assert_return (invoke "check_t0" (i32.const 0xd)) (i32.const 0x5))
(assert_return (invoke "check_t0" (i32.const 0xe)) (i32.const 0x2))
(assert_return (invoke "check_t0" (i32.const 0xf)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x10)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x18)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x19)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x1a)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x2)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x4)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x6)) (i32.const 0x4))
(assert_trap (invoke "check_t1" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xa)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0xb)) (i32.const 0x6))
(assert_return (invoke "check_t1" (i32.const 0xc)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0xd)) (i32.const 0x2))
(assert_return (invoke "check_t1" (i32.const 0xe)) (i32.const 0x5))
(assert_return (invoke "check_t1" (i32.const 0xf)) (i32.const 0x7))
(assert_trap (invoke "check_t1" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\89\80\80\80\00"
  "\08\00\00\00\00\00\01\02\02\04\89\80\80\80\00\02"
  "\70\01\1e\1e\70\01\1e\1e\07\9e\80\80\80\00\03\04"
  "\74\65\73\74\00\0a\08\63\68\65\63\6b\5f\74\30\00"
  "\0b\08\63\68\65\63\6b\5f\74\31\00\0c\09\cc\80\80"
  "\80\00\06\02\01\41\02\0b\00\04\03\01\04\01\05\70"
  "\04\d2\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\02\01\41"
  "\0c\0b\00\05\07\05\02\03\06\05\70\05\d2\05\0b\d2"
  "\09\0b\d2\02\0b\d2\07\0b\d2\06\0b\00\41\03\0b\04"
  "\01\03\01\04\00\41\0b\0b\05\06\03\02\05\07\0a\d7"
  "\80\80\80\00\08\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\8c\80\80\80\00\00\41\0d\41\19\41\03\fc\0e"
  "\01\01\0b\87\80\80\80\00\00\20\00\11\00\01\0b\87"
  "\80\80\80\00\00\20\00\11\00\00\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check_t0" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0xc)) (i32.const 0x7))
(assert_trap (invoke "check_t0" (i32.const 0xd)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xe)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xf)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x10)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x2)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x4)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x6)) (i32.const 0x4))
(assert_trap (invoke "check_t1" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xa)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0xb)) (i32.const 0x6))
(assert_return (invoke "check_t1" (i32.const 0xc)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0xd)) (i32.const 0x2))
(assert_return (invoke "check_t1" (i32.const 0xe)) (i32.const 0x5))
(assert_return (invoke "check_t1" (i32.const 0xf)) (i32.const 0x7))
(assert_trap (invoke "check_t1" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\89\80\80\80\00"
  "\08\00\00\00\00\00\01\02\02\04\89\80\80\80\00\02"
  "\70\01\1e\1e\70\01\1e\1e\07\9e\80\80\80\00\03\04"
  "\74\65\73\74\00\0a\08\63\68\65\63\6b\5f\74\30\00"
  "\0b\08\63\68\65\63\6b\5f\74\31\00\0c\09\cc\80\80"
  "\80\00\06\02\01\41\02\0b\00\04\03\01\04\01\05\70"
  "\04\d2\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\02\01\41"
  "\0c\0b\00\05\07\05\02\03\06\05\70\05\d2\05\0b\d2"
  "\09\0b\d2\02\0b\d2\07\0b\d2\06\0b\00\41\03\0b\04"
  "\01\03\01\04\00\41\0b\0b\05\06\03\02\05\07\0a\d7"
  "\80\80\80\00\08\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\8c\80\80\80\00\00\41\14\41\16\41\04\fc\0e"
  "\01\01\0b\87\80\80\80\00\00\20\00\11\00\01\0b\87"
  "\80\80\80\00\00\20\00\11\00\00\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check_t0" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0xc)) (i32.const 0x7))
(assert_return (invoke "check_t0" (i32.const 0xd)) (i32.const 0x5))
(assert_return (invoke "check_t0" (i32.const 0xe)) (i32.const 0x2))
(assert_return (invoke "check_t0" (i32.const 0xf)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x10)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x2)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x4)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x6)) (i32.const 0x4))
(assert_trap (invoke "check_t1" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xa)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0xb)) (i32.const 0x6))
(assert_return (invoke "check_t1" (i32.const 0xc)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0xd)) (i32.const 0x2))
(assert_return (invoke "check_t1" (i32.const 0xe)) (i32.const 0x5))
(assert_return (invoke "check_t1" (i32.const 0xf)) (i32.const 0x7))
(assert_trap (invoke "check_t1" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\89\80\80\80\00"
  "\08\00\00\00\00\00\01\02\02\04\89\80\80\80\00\02"
  "\70\01\1e\1e\70\01\1e\1e\07\9e\80\80\80\00\03\04"
  "\74\65\73\74\00\0a\08\63\68\65\63\6b\5f\74\30\00"
  "\0b\08\63\68\65\63\6b\5f\74\31\00\0c\09\cc\80\80"
  "\80\00\06\02\01\41\02\0b\00\04\03\01\04\01\05\70"
  "\04\d2\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\02\01\41"
  "\0c\0b\00\05\07\05\02\03\06\05\70\05\d2\05\0b\d2"
  "\09\0b\d2\02\0b\d2\07\0b\d2\06\0b\00\41\03\0b\04"
  "\01\03\01\04\00\41\0b\0b\05\06\03\02\05\07\0a\d7"
  "\80\80\80\00\08\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\8c\80\80\80\00\00\41\19\41\01\41\03\fc\0e"
  "\01\01\0b\87\80\80\80\00\00\20\00\11\00\01\0b\87"
  "\80\80\80\00\00\20\00\11\00\00\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check_t0" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0xc)) (i32.const 0x7))
(assert_return (invoke "check_t0" (i32.const 0xd)) (i32.const 0x5))
(assert_return (invoke "check_t0" (i32.const 0xe)) (i32.const 0x2))
(assert_return (invoke "check_t0" (i32.const 0xf)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x10)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x19)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x1a)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x1b)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x2)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x4)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x6)) (i32.const 0x4))
(assert_trap (invoke "check_t1" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xa)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0xb)) (i32.const 0x6))
(assert_return (invoke "check_t1" (i32.const 0xc)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0xd)) (i32.const 0x2))
(assert_return (invoke "check_t1" (i32.const 0xe)) (i32.const 0x5))
(assert_return (invoke "check_t1" (i32.const 0xf)) (i32.const 0x7))
(assert_trap (invoke "check_t1" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\89\80\80\80\00"
  "\08\00\00\00\00\00\01\02\02\04\89\80\80\80\00\02"
  "\70\01\1e\1e\70\01\1e\1e\07\9e\80\80\80\00\03\04"
  "\74\65\73\74\00\0a\08\63\68\65\63\6b\5f\74\30\00"
  "\0b\08\63\68\65\63\6b\5f\74\31\00\0c\09\cc\80\80"
  "\80\00\06\02\01\41\02\0b\00\04\03\01\04\01\05\70"
  "\04\d2\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\02\01\41"
  "\0c\0b\00\05\07\05\02\03\06\05\70\05\d2\05\0b\d2"
  "\09\0b\d2\02\0b\d2\07\0b\d2\06\0b\00\41\03\0b\04"
  "\01\03\01\04\00\41\0b\0b\05\06\03\02\05\07\0a\d7"
  "\80\80\80\00\08\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\8c\80\80\80\00\00\41\0a\41\0c\41\07\fc\0e"
  "\01\01\0b\87\80\80\80\00\00\20\00\11\00\01\0b\87"
  "\80\80\80\00\00\20\00\11\00\00\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check_t0" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x9)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0xa)) (i32.const 0x7))
(assert_return (invoke "check_t0" (i32.const 0xb)) (i32.const 0x5))
(assert_return (invoke "check_t0" (i32.const 0xc)) (i32.const 0x2))
(assert_return (invoke "check_t0" (i32.const 0xd)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0xe)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0xf)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x2)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x4)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x6)) (i32.const 0x4))
(assert_trap (invoke "check_t1" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xa)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0xb)) (i32.const 0x6))
(assert_return (invoke "check_t1" (i32.const 0xc)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0xd)) (i32.const 0x2))
(assert_return (invoke "check_t1" (i32.const 0xe)) (i32.const 0x5))
(assert_return (invoke "check_t1" (i32.const 0xf)) (i32.const 0x7))
(assert_trap (invoke "check_t1" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\89\80\80\80\00"
  "\08\00\00\00\00\00\01\02\02\04\89\80\80\80\00\02"
  "\70\01\1e\1e\70\01\1e\1e\07\9e\80\80\80\00\03\04"
  "\74\65\73\74\00\0a\08\63\68\65\63\6b\5f\74\30\00"
  "\0b\08\63\68\65\63\6b\5f\74\31\00\0c\09\cc\80\80"
  "\80\00\06\02\01\41\02\0b\00\04\03\01\04\01\05\70"
  "\04\d2\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\02\01\41"
  "\0c\0b\00\05\07\05\02\03\06\05\70\05\d2\05\0b\d2"
  "\09\0b\d2\02\0b\d2\07\0b\d2\06\0b\00\41\03\0b\04"
  "\01\03\01\04\00\41\0b\0b\05\06\03\02\05\07\0a\d7"
  "\80\80\80\00\08\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\8c\80\80\80\00\00\41\0c\41\0a\41\07\fc\0e"
  "\01\01\0b\87\80\80\80\00\00\20\00\11\00\01\0b\87"
  "\80\80\80\00\00\20\00\11\00\00\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check_t0" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xb)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xc)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xd)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0xe)) (i32.const 0x7))
(assert_return (invoke "check_t0" (i32.const 0xf)) (i32.const 0x5))
(assert_return (invoke "check_t0" (i32.const 0x10)) (i32.const 0x2))
(assert_return (invoke "check_t0" (i32.const 0x11)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x12)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x2)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x4)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x6)) (i32.const 0x4))
(assert_trap (invoke "check_t1" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xa)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0xb)) (i32.const 0x6))
(assert_return (invoke "check_t1" (i32.const 0xc)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0xd)) (i32.const 0x2))
(assert_return (invoke "check_t1" (i32.const 0xe)) (i32.const 0x5))
(assert_return (invoke "check_t1" (i32.const 0xf)) (i32.const 0x7))
(assert_trap (invoke "check_t1" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\02\a9\80\80\80"
  "\00\05\01\61\03\65\66\30\00\00\01\61\03\65\66\31"
  "\00\00\01\61\03\65\66\32\00\00\01\61\03\65\66\33"
  "\00\00\01\61\03\65\66\34\00\00\03\89\80\80\80\00"
  "\08\00\00\00\00\00\01\02\02\04\89\80\80\80\00\02"
  "\70\01\1e\1e\70\01\1e\1e\07\9e\80\80\80\00\03\04"
  "\74\65\73\74\00\0a\08\63\68\65\63\6b\5f\74\30\00"
  "\0b\08\63\68\65\63\6b\5f\74\31\00\0c\09\cc\80\80"
  "\80\00\06\02\01\41\02\0b\00\04\03\01\04\01\05\70"
  "\04\d2\02\0b\d2\07\0b\d2\01\0b\d2\08\0b\02\01\41"
  "\0c\0b\00\05\07\05\02\03\06\05\70\05\d2\05\0b\d2"
  "\09\0b\d2\02\0b\d2\07\0b\d2\06\0b\00\41\03\0b\04"
  "\01\03\01\04\00\41\0b\0b\05\06\03\02\05\07\0a\d7"
  "\80\80\80\00\08\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\8c\80\80\80\00\00\41\0a\41\00\41\14\fc\0e"
  "\00\01\0b\87\80\80\80\00\00\20\00\11\00\01\0b\87"
  "\80\80\80\00\00\20\00\11\00\00\0b"
)
(module instance)
(invoke "test")
(assert_trap (invoke "check_t0" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0x2)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t0" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "check_t0" (i32.const 0x5)) (i32.const 0x1))
(assert_trap (invoke "check_t0" (i32.const 0x6)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check_t0" (i32.const 0xc)) (i32.const 0x7))
(assert_return (invoke "check_t0" (i32.const 0xd)) (i32.const 0x5))
(assert_return (invoke "check_t0" (i32.const 0xe)) (i32.const 0x2))
(assert_return (invoke "check_t0" (i32.const 0xf)) (i32.const 0x3))
(assert_return (invoke "check_t0" (i32.const 0x10)) (i32.const 0x6))
(assert_trap (invoke "check_t0" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x15)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x16)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x17)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x18)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x19)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1a)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t0" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x2)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x3)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x4)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x5)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0x6)) (i32.const 0x4))
(assert_trap (invoke "check_t1" (i32.const 0x7)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x8)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x9)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xa)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0xb)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0xc)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0xd)) (i32.const 0x1))
(assert_return (invoke "check_t1" (i32.const 0xe)) (i32.const 0x4))
(assert_return (invoke "check_t1" (i32.const 0xf)) (i32.const 0x1))
(assert_trap (invoke "check_t1" (i32.const 0x10)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x11)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x12)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x13)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x14)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x15)) "uninitialized element")
(assert_return (invoke "check_t1" (i32.const 0x16)) (i32.const 0x7))
(assert_return (invoke "check_t1" (i32.const 0x17)) (i32.const 0x5))
(assert_return (invoke "check_t1" (i32.const 0x18)) (i32.const 0x2))
(assert_return (invoke "check_t1" (i32.const 0x19)) (i32.const 0x3))
(assert_return (invoke "check_t1" (i32.const 0x1a)) (i32.const 0x6))
(assert_trap (invoke "check_t1" (i32.const 0x1b)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1c)) "uninitialized element")
(assert_trap (invoke "check_t1" (i32.const 0x1d)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\42\1c\42\01\42\03\fc\0e\00"
  "\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\f0\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\90\80\80\80\00\00\42\fe\ff\ff\ff\0f\42\01\42"
  "\02\fc\0e\00\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\42\0f\42\19\42\06\fc\0e\00"
  "\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\f0\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\90\80\80\80\00\00\42\0f\42\fe\ff\ff\ff\0f\42"
  "\02\fc\0e\00\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\42\0f\42\19\42\00\fc\0e\00"
  "\00\0b"
)
(module instance)
(invoke "test")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\42\1e\42\0f\42\00\fc\0e\00"
  "\00\0b"
)
(module instance)
(invoke "test")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\42\1f\42\0f\42\00\fc\0e\00"
  "\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\42\0f\42\1e\42\00\fc\0e\00"
  "\00\0b"
)
(module instance)
(invoke "test")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\42\0f\42\1f\42\00\fc\0e\00"
  "\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\42\1e\42\1e\42\00\fc\0e\00"
  "\00\0b"
)
(module instance)
(invoke "test")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\42\1f\42\1f\42\00\fc\0e\00"
  "\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\42\1c\42\01\42\03\fc\0e\01"
  "\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\f0\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\90\80\80\80\00\00\42\fe\ff\ff\ff\0f\42\01\42"
  "\02\fc\0e\01\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\42\0f\42\19\42\06\fc\0e\01"
  "\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\f0\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\90\80\80\80\00\00\42\0f\42\fe\ff\ff\ff\0f\42"
  "\02\fc\0e\01\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\42\0f\42\19\42\00\fc\0e\01"
  "\00\0b"
)
(module instance)
(invoke "test")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\42\1e\42\0f\42\00\fc\0e\01"
  "\00\0b"
)
(module instance)
(invoke "test")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\42\1f\42\0f\42\00\fc\0e\01"
  "\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\42\0f\42\1e\42\00\fc\0e\01"
  "\00\0b"
)
(module instance)
(invoke "test")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\42\0f\42\1f\42\00\fc\0e\01"
  "\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\42\1e\42\1e\42\00\fc\0e\01"
  "\00\0b"
)
(module instance)
(invoke "test")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\03\8c\80\80\80\00\0b\00\00\00"
  "\00\00\00\00\00\00\00\01\04\89\80\80\80\00\02\70"
  "\05\1e\1e\70\05\1e\1e\07\88\80\80\80\00\01\04\74"
  "\65\73\74\00\0a\09\b5\80\80\80\00\04\00\42\02\0b"
  "\04\03\01\04\01\05\70\04\d2\02\0b\d2\07\0b\d2\01"
  "\0b\d2\08\0b\00\42\0c\0b\05\07\05\02\03\06\05\70"
  "\05\d2\05\0b\d2\09\0b\d2\02\0b\d2\07\0b\d2\06\0b"
  "\0a\ec\80\80\80\00\0b\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\8c\80\80\80\00\00\42\1f\42\1f\42\00\fc\0e\01"
  "\00\0b"
)
(module instance)
(assert_trap (invoke "test") "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\90\80\80\80\00\03\60"
  "\00\01\7f\60\01\7f\01\7f\60\03\7f\7f\7f\00\03\93"
  "\80\80\80\00\12\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\00\01\02\04\85\80\80\80\00\01\70\01"
  "\20\40\07\e4\80\80\80\00\12\02\66\30\00\00\02\66"
  "\31\00\01\02\66\32\00\02\02\66\33\00\03\02\66\34"
  "\00\04\02\66\35\00\05\02\66\36\00\06\02\66\37\00"
  "\07\02\66\38\00\08\02\66\39\00\09\03\66\31\30\00"
  "\0a\03\66\31\31\00\0b\03\66\31\32\00\0c\03\66\31"
  "\33\00\0d\03\66\31\34\00\0e\03\66\31\35\00\0f\04"
  "\74\65\73\74\00\10\03\72\75\6e\00\11\09\8e\80\80"
  "\80\00\01\00\41\00\0b\08\00\01\02\03\04\05\06\07"
  "\0a\ae\81\80\80\00\12\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\84\80\80\80\00\00\41\0a\0b\84\80\80\80\00\00"
  "\41\0b\0b\84\80\80\80\00\00\41\0c\0b\84\80\80\80"
  "\00\00\41\0d\0b\84\80\80\80\00\00\41\0e\0b\84\80"
  "\80\80\00\00\41\0f\0b\87\80\80\80\00\00\20\00\11"
  "\00\00\0b\8c\80\80\80\00\00\20\00\20\01\20\02\fc"
  "\0e\00\00\0b"
)
(module instance)
(assert_trap
  (invoke "run" (i32.const 0x18) (i32.const 0x0) (i32.const 0x10))
  "out of bounds table access"
)
(assert_return (invoke "test" (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "test" (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "test" (i32.const 0x2)) (i32.const 0x2))
(assert_return (invoke "test" (i32.const 0x3)) (i32.const 0x3))
(assert_return (invoke "test" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "test" (i32.const 0x5)) (i32.const 0x5))
(assert_return (invoke "test" (i32.const 0x6)) (i32.const 0x6))
(assert_return (invoke "test" (i32.const 0x7)) (i32.const 0x7))
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
  "\00\61\73\6d\01\00\00\00\01\90\80\80\80\00\03\60"
  "\00\01\7f\60\01\7f\01\7f\60\03\7f\7f\7f\00\03\93"
  "\80\80\80\00\12\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\00\01\02\04\85\80\80\80\00\01\70\01"
  "\20\40\07\e4\80\80\80\00\12\02\66\30\00\00\02\66"
  "\31\00\01\02\66\32\00\02\02\66\33\00\03\02\66\34"
  "\00\04\02\66\35\00\05\02\66\36\00\06\02\66\37\00"
  "\07\02\66\38\00\08\02\66\39\00\09\03\66\31\30\00"
  "\0a\03\66\31\31\00\0b\03\66\31\32\00\0c\03\66\31"
  "\33\00\0d\03\66\31\34\00\0e\03\66\31\35\00\0f\04"
  "\74\65\73\74\00\10\03\72\75\6e\00\11\09\8f\80\80"
  "\80\00\01\00\41\00\0b\09\00\01\02\03\04\05\06\07"
  "\08\0a\ae\81\80\80\00\12\84\80\80\80\00\00\41\00"
  "\0b\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00"
  "\41\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80"
  "\00\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\84\80\80\80\00\00\41\0a\0b\84\80\80\80\00"
  "\00\41\0b\0b\84\80\80\80\00\00\41\0c\0b\84\80\80"
  "\80\00\00\41\0d\0b\84\80\80\80\00\00\41\0e\0b\84"
  "\80\80\80\00\00\41\0f\0b\87\80\80\80\00\00\20\00"
  "\11\00\00\0b\8c\80\80\80\00\00\20\00\20\01\20\02"
  "\fc\0e\00\00\0b"
)
(module instance)
(assert_trap
  (invoke "run" (i32.const 0x17) (i32.const 0x0) (i32.const 0xf))
  "out of bounds table access"
)
(assert_return (invoke "test" (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "test" (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "test" (i32.const 0x2)) (i32.const 0x2))
(assert_return (invoke "test" (i32.const 0x3)) (i32.const 0x3))
(assert_return (invoke "test" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "test" (i32.const 0x5)) (i32.const 0x5))
(assert_return (invoke "test" (i32.const 0x6)) (i32.const 0x6))
(assert_return (invoke "test" (i32.const 0x7)) (i32.const 0x7))
(assert_return (invoke "test" (i32.const 0x8)) (i32.const 0x8))
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
  "\00\61\73\6d\01\00\00\00\01\90\80\80\80\00\03\60"
  "\00\01\7f\60\01\7f\01\7f\60\03\7f\7f\7f\00\03\93"
  "\80\80\80\00\12\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\00\01\02\04\85\80\80\80\00\01\70\01"
  "\20\40\07\e4\80\80\80\00\12\02\66\30\00\00\02\66"
  "\31\00\01\02\66\32\00\02\02\66\33\00\03\02\66\34"
  "\00\04\02\66\35\00\05\02\66\36\00\06\02\66\37\00"
  "\07\02\66\38\00\08\02\66\39\00\09\03\66\31\30\00"
  "\0a\03\66\31\31\00\0b\03\66\31\32\00\0c\03\66\31"
  "\33\00\0d\03\66\31\34\00\0e\03\66\31\35\00\0f\04"
  "\74\65\73\74\00\10\03\72\75\6e\00\11\09\8e\80\80"
  "\80\00\01\00\41\18\0b\08\00\01\02\03\04\05\06\07"
  "\0a\ae\81\80\80\00\12\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\84\80\80\80\00\00\41\0a\0b\84\80\80\80\00\00"
  "\41\0b\0b\84\80\80\80\00\00\41\0c\0b\84\80\80\80"
  "\00\00\41\0d\0b\84\80\80\80\00\00\41\0e\0b\84\80"
  "\80\80\00\00\41\0f\0b\87\80\80\80\00\00\20\00\11"
  "\00\00\0b\8c\80\80\80\00\00\20\00\20\01\20\02\fc"
  "\0e\00\00\0b"
)
(module instance)
(assert_trap
  (invoke "run" (i32.const 0x0) (i32.const 0x18) (i32.const 0x10))
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
(assert_return (invoke "test" (i32.const 0x18)) (i32.const 0x0))
(assert_return (invoke "test" (i32.const 0x19)) (i32.const 0x1))
(assert_return (invoke "test" (i32.const 0x1a)) (i32.const 0x2))
(assert_return (invoke "test" (i32.const 0x1b)) (i32.const 0x3))
(assert_return (invoke "test" (i32.const 0x1c)) (i32.const 0x4))
(assert_return (invoke "test" (i32.const 0x1d)) (i32.const 0x5))
(assert_return (invoke "test" (i32.const 0x1e)) (i32.const 0x6))
(assert_return (invoke "test" (i32.const 0x1f)) (i32.const 0x7))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\90\80\80\80\00\03\60"
  "\00\01\7f\60\01\7f\01\7f\60\03\7f\7f\7f\00\03\93"
  "\80\80\80\00\12\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\00\01\02\04\85\80\80\80\00\01\70\01"
  "\20\40\07\e4\80\80\80\00\12\02\66\30\00\00\02\66"
  "\31\00\01\02\66\32\00\02\02\66\33\00\03\02\66\34"
  "\00\04\02\66\35\00\05\02\66\36\00\06\02\66\37\00"
  "\07\02\66\38\00\08\02\66\39\00\09\03\66\31\30\00"
  "\0a\03\66\31\31\00\0b\03\66\31\32\00\0c\03\66\31"
  "\33\00\0d\03\66\31\34\00\0e\03\66\31\35\00\0f\04"
  "\74\65\73\74\00\10\03\72\75\6e\00\11\09\8f\80\80"
  "\80\00\01\00\41\17\0b\09\00\01\02\03\04\05\06\07"
  "\08\0a\ae\81\80\80\00\12\84\80\80\80\00\00\41\00"
  "\0b\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00"
  "\41\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80"
  "\00\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80"
  "\80\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b"
  "\84\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41"
  "\09\0b\84\80\80\80\00\00\41\0a\0b\84\80\80\80\00"
  "\00\41\0b\0b\84\80\80\80\00\00\41\0c\0b\84\80\80"
  "\80\00\00\41\0d\0b\84\80\80\80\00\00\41\0e\0b\84"
  "\80\80\80\00\00\41\0f\0b\87\80\80\80\00\00\20\00"
  "\11\00\00\0b\8c\80\80\80\00\00\20\00\20\01\20\02"
  "\fc\0e\00\00\0b"
)
(module instance)
(assert_trap
  (invoke "run" (i32.const 0x0) (i32.const 0x17) (i32.const 0xf))
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
(assert_return (invoke "test" (i32.const 0x17)) (i32.const 0x0))
(assert_return (invoke "test" (i32.const 0x18)) (i32.const 0x1))
(assert_return (invoke "test" (i32.const 0x19)) (i32.const 0x2))
(assert_return (invoke "test" (i32.const 0x1a)) (i32.const 0x3))
(assert_return (invoke "test" (i32.const 0x1b)) (i32.const 0x4))
(assert_return (invoke "test" (i32.const 0x1c)) (i32.const 0x5))
(assert_return (invoke "test" (i32.const 0x1d)) (i32.const 0x6))
(assert_return (invoke "test" (i32.const 0x1e)) (i32.const 0x7))
(assert_return (invoke "test" (i32.const 0x1f)) (i32.const 0x8))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\90\80\80\80\00\03\60"
  "\00\01\7f\60\01\7f\01\7f\60\03\7f\7f\7f\00\03\93"
  "\80\80\80\00\12\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\00\01\02\04\85\80\80\80\00\01\70\01"
  "\20\40\07\e4\80\80\80\00\12\02\66\30\00\00\02\66"
  "\31\00\01\02\66\32\00\02\02\66\33\00\03\02\66\34"
  "\00\04\02\66\35\00\05\02\66\36\00\06\02\66\37\00"
  "\07\02\66\38\00\08\02\66\39\00\09\03\66\31\30\00"
  "\0a\03\66\31\31\00\0b\03\66\31\32\00\0c\03\66\31"
  "\33\00\0d\03\66\31\34\00\0e\03\66\31\35\00\0f\04"
  "\74\65\73\74\00\10\03\72\75\6e\00\11\09\8e\80\80"
  "\80\00\01\00\41\0b\0b\08\00\01\02\03\04\05\06\07"
  "\0a\ae\81\80\80\00\12\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\84\80\80\80\00\00\41\0a\0b\84\80\80\80\00\00"
  "\41\0b\0b\84\80\80\80\00\00\41\0c\0b\84\80\80\80"
  "\00\00\41\0d\0b\84\80\80\80\00\00\41\0e\0b\84\80"
  "\80\80\00\00\41\0f\0b\87\80\80\80\00\00\20\00\11"
  "\00\00\0b\8c\80\80\80\00\00\20\00\20\01\20\02\fc"
  "\0e\00\00\0b"
)
(module instance)
(assert_trap
  (invoke "run" (i32.const 0x18) (i32.const 0xb) (i32.const 0x10))
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
(assert_return (invoke "test" (i32.const 0xb)) (i32.const 0x0))
(assert_return (invoke "test" (i32.const 0xc)) (i32.const 0x1))
(assert_return (invoke "test" (i32.const 0xd)) (i32.const 0x2))
(assert_return (invoke "test" (i32.const 0xe)) (i32.const 0x3))
(assert_return (invoke "test" (i32.const 0xf)) (i32.const 0x4))
(assert_return (invoke "test" (i32.const 0x10)) (i32.const 0x5))
(assert_return (invoke "test" (i32.const 0x11)) (i32.const 0x6))
(assert_return (invoke "test" (i32.const 0x12)) (i32.const 0x7))
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
  "\00\61\73\6d\01\00\00\00\01\90\80\80\80\00\03\60"
  "\00\01\7f\60\01\7f\01\7f\60\03\7f\7f\7f\00\03\93"
  "\80\80\80\00\12\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\00\01\02\04\85\80\80\80\00\01\70\01"
  "\20\40\07\e4\80\80\80\00\12\02\66\30\00\00\02\66"
  "\31\00\01\02\66\32\00\02\02\66\33\00\03\02\66\34"
  "\00\04\02\66\35\00\05\02\66\36\00\06\02\66\37\00"
  "\07\02\66\38\00\08\02\66\39\00\09\03\66\31\30\00"
  "\0a\03\66\31\31\00\0b\03\66\31\32\00\0c\03\66\31"
  "\33\00\0d\03\66\31\34\00\0e\03\66\31\35\00\0f\04"
  "\74\65\73\74\00\10\03\72\75\6e\00\11\09\8e\80\80"
  "\80\00\01\00\41\18\0b\08\00\01\02\03\04\05\06\07"
  "\0a\ae\81\80\80\00\12\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\84\80\80\80\00\00\41\0a\0b\84\80\80\80\00\00"
  "\41\0b\0b\84\80\80\80\00\00\41\0c\0b\84\80\80\80"
  "\00\00\41\0d\0b\84\80\80\80\00\00\41\0e\0b\84\80"
  "\80\80\00\00\41\0f\0b\87\80\80\80\00\00\20\00\11"
  "\00\00\0b\8c\80\80\80\00\00\20\00\20\01\20\02\fc"
  "\0e\00\00\0b"
)
(module instance)
(assert_trap
  (invoke "run" (i32.const 0xb) (i32.const 0x18) (i32.const 0x10))
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
(assert_return (invoke "test" (i32.const 0x18)) (i32.const 0x0))
(assert_return (invoke "test" (i32.const 0x19)) (i32.const 0x1))
(assert_return (invoke "test" (i32.const 0x1a)) (i32.const 0x2))
(assert_return (invoke "test" (i32.const 0x1b)) (i32.const 0x3))
(assert_return (invoke "test" (i32.const 0x1c)) (i32.const 0x4))
(assert_return (invoke "test" (i32.const 0x1d)) (i32.const 0x5))
(assert_return (invoke "test" (i32.const 0x1e)) (i32.const 0x6))
(assert_return (invoke "test" (i32.const 0x1f)) (i32.const 0x7))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\90\80\80\80\00\03\60"
  "\00\01\7f\60\01\7f\01\7f\60\03\7f\7f\7f\00\03\93"
  "\80\80\80\00\12\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\00\01\02\04\85\80\80\80\00\01\70\01"
  "\20\40\07\e4\80\80\80\00\12\02\66\30\00\00\02\66"
  "\31\00\01\02\66\32\00\02\02\66\33\00\03\02\66\34"
  "\00\04\02\66\35\00\05\02\66\36\00\06\02\66\37\00"
  "\07\02\66\38\00\08\02\66\39\00\09\03\66\31\30\00"
  "\0a\03\66\31\31\00\0b\03\66\31\32\00\0c\03\66\31"
  "\33\00\0d\03\66\31\34\00\0e\03\66\31\35\00\0f\04"
  "\74\65\73\74\00\10\03\72\75\6e\00\11\09\8e\80\80"
  "\80\00\01\00\41\15\0b\08\00\01\02\03\04\05\06\07"
  "\0a\ae\81\80\80\00\12\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\84\80\80\80\00\00\41\0a\0b\84\80\80\80\00\00"
  "\41\0b\0b\84\80\80\80\00\00\41\0c\0b\84\80\80\80"
  "\00\00\41\0d\0b\84\80\80\80\00\00\41\0e\0b\84\80"
  "\80\80\00\00\41\0f\0b\87\80\80\80\00\00\20\00\11"
  "\00\00\0b\8c\80\80\80\00\00\20\00\20\01\20\02\fc"
  "\0e\00\00\0b"
)
(module instance)
(assert_trap
  (invoke "run" (i32.const 0x18) (i32.const 0x15) (i32.const 0x10))
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
(assert_return (invoke "test" (i32.const 0x15)) (i32.const 0x0))
(assert_return (invoke "test" (i32.const 0x16)) (i32.const 0x1))
(assert_return (invoke "test" (i32.const 0x17)) (i32.const 0x2))
(assert_return (invoke "test" (i32.const 0x18)) (i32.const 0x3))
(assert_return (invoke "test" (i32.const 0x19)) (i32.const 0x4))
(assert_return (invoke "test" (i32.const 0x1a)) (i32.const 0x5))
(assert_return (invoke "test" (i32.const 0x1b)) (i32.const 0x6))
(assert_return (invoke "test" (i32.const 0x1c)) (i32.const 0x7))
(assert_trap (invoke "test" (i32.const 0x1d)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1e)) "uninitialized element")
(assert_trap (invoke "test" (i32.const 0x1f)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\90\80\80\80\00\03\60"
  "\00\01\7f\60\01\7f\01\7f\60\03\7f\7f\7f\00\03\93"
  "\80\80\80\00\12\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\00\01\02\04\85\80\80\80\00\01\70\01"
  "\20\40\07\e4\80\80\80\00\12\02\66\30\00\00\02\66"
  "\31\00\01\02\66\32\00\02\02\66\33\00\03\02\66\34"
  "\00\04\02\66\35\00\05\02\66\36\00\06\02\66\37\00"
  "\07\02\66\38\00\08\02\66\39\00\09\03\66\31\30\00"
  "\0a\03\66\31\31\00\0b\03\66\31\32\00\0c\03\66\31"
  "\33\00\0d\03\66\31\34\00\0e\03\66\31\35\00\0f\04"
  "\74\65\73\74\00\10\03\72\75\6e\00\11\09\8e\80\80"
  "\80\00\01\00\41\18\0b\08\00\01\02\03\04\05\06\07"
  "\0a\ae\81\80\80\00\12\84\80\80\80\00\00\41\00\0b"
  "\84\80\80\80\00\00\41\01\0b\84\80\80\80\00\00\41"
  "\02\0b\84\80\80\80\00\00\41\03\0b\84\80\80\80\00"
  "\00\41\04\0b\84\80\80\80\00\00\41\05\0b\84\80\80"
  "\80\00\00\41\06\0b\84\80\80\80\00\00\41\07\0b\84"
  "\80\80\80\00\00\41\08\0b\84\80\80\80\00\00\41\09"
  "\0b\84\80\80\80\00\00\41\0a\0b\84\80\80\80\00\00"
  "\41\0b\0b\84\80\80\80\00\00\41\0c\0b\84\80\80\80"
  "\00\00\41\0d\0b\84\80\80\80\00\00\41\0e\0b\84\80"
  "\80\80\00\00\41\0f\0b\87\80\80\80\00\00\20\00\11"
  "\00\00\0b\8c\80\80\80\00\00\20\00\20\01\20\02\fc"
  "\0e\00\00\0b"
)
(module instance)
(assert_trap
  (invoke "run" (i32.const 0x15) (i32.const 0x18) (i32.const 0x10))
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
(assert_return (invoke "test" (i32.const 0x18)) (i32.const 0x0))
(assert_return (invoke "test" (i32.const 0x19)) (i32.const 0x1))
(assert_return (invoke "test" (i32.const 0x1a)) (i32.const 0x2))
(assert_return (invoke "test" (i32.const 0x1b)) (i32.const 0x3))
(assert_return (invoke "test" (i32.const 0x1c)) (i32.const 0x4))
(assert_return (invoke "test" (i32.const 0x1d)) (i32.const 0x5))
(assert_return (invoke "test" (i32.const 0x1e)) (i32.const 0x6))
(assert_return (invoke "test" (i32.const 0x1f)) (i32.const 0x7))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\90\80\80\80\00\03\60"
  "\00\01\7f\60\01\7f\01\7f\60\03\7f\7f\7f\00\03\93"
  "\80\80\80\00\12\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\00\01\02\04\85\80\80\80\00\01\70\01"
  "\20\40\07\e4\80\80\80\00\12\02\66\30\00\00\02\66"
  "\31\00\01\02\66\32\00\02\02\66\33\00\03\02\66\34"
  "\00\04\02\66\35\00\05\02\66\36\00\06\02\66\37\00"
  "\07\02\66\38\00\08\02\66\39\00\09\03\66\31\30\00"
  "\0a\03\66\31\31\00\0b\03\66\31\32\00\0c\03\66\31"
  "\33\00\0d\03\66\31\34\00\0e\03\66\31\35\00\0f\04"
  "\74\65\73\74\00\10\03\72\75\6e\00\11\09\91\80\80"
  "\80\00\01\00\41\15\0b\0b\00\01\02\03\04\05\06\07"
  "\08\09\0a\0a\ae\81\80\80\00\12\84\80\80\80\00\00"
  "\41\00\0b\84\80\80\80\00\00\41\01\0b\84\80\80\80"
  "\00\00\41\02\0b\84\80\80\80\00\00\41\03\0b\84\80"
  "\80\80\00\00\41\04\0b\84\80\80\80\00\00\41\05\0b"
  "\84\80\80\80\00\00\41\06\0b\84\80\80\80\00\00\41"
  "\07\0b\84\80\80\80\00\00\41\08\0b\84\80\80\80\00"
  "\00\41\09\0b\84\80\80\80\00\00\41\0a\0b\84\80\80"
  "\80\00\00\41\0b\0b\84\80\80\80\00\00\41\0c\0b\84"
  "\80\80\80\00\00\41\0d\0b\84\80\80\80\00\00\41\0e"
  "\0b\84\80\80\80\00\00\41\0f\0b\87\80\80\80\00\00"
  "\20\00\11\00\00\0b\8c\80\80\80\00\00\20\00\20\01"
  "\20\02\fc\0e\00\00\0b"
)
(module instance)
(assert_trap
  (invoke "run" (i32.const 0x15) (i32.const 0x15) (i32.const 0x10))
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
(assert_return (invoke "test" (i32.const 0x15)) (i32.const 0x0))
(assert_return (invoke "test" (i32.const 0x16)) (i32.const 0x1))
(assert_return (invoke "test" (i32.const 0x17)) (i32.const 0x2))
(assert_return (invoke "test" (i32.const 0x18)) (i32.const 0x3))
(assert_return (invoke "test" (i32.const 0x19)) (i32.const 0x4))
(assert_return (invoke "test" (i32.const 0x1a)) (i32.const 0x5))
(assert_return (invoke "test" (i32.const 0x1b)) (i32.const 0x6))
(assert_return (invoke "test" (i32.const 0x1c)) (i32.const 0x7))
(assert_return (invoke "test" (i32.const 0x1d)) (i32.const 0x8))
(assert_return (invoke "test" (i32.const 0x1e)) (i32.const 0x9))
(assert_return (invoke "test" (i32.const 0x1f)) (i32.const 0xa))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\90\80\80\80\00\03\60"
  "\00\01\7f\60\01\7f\01\7f\60\03\7f\7f\7f\00\03\93"
  "\80\80\80\00\12\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\00\01\02\04\87\80\80\80\00\01\70\01"
  "\80\01\80\01\07\e4\80\80\80\00\12\02\66\30\00\00"
  "\02\66\31\00\01\02\66\32\00\02\02\66\33\00\03\02"
  "\66\34\00\04\02\66\35\00\05\02\66\36\00\06\02\66"
  "\37\00\07\02\66\38\00\08\02\66\39\00\09\03\66\31"
  "\30\00\0a\03\66\31\31\00\0b\03\66\31\32\00\0c\03"
  "\66\31\33\00\0d\03\66\31\34\00\0e\03\66\31\35\00"
  "\0f\04\74\65\73\74\00\10\03\72\75\6e\00\11\09\97"
  "\80\80\80\00\01\00\41\f0\00\0b\10\00\01\02\03\04"
  "\05\06\07\08\09\0a\0b\0c\0d\0e\0f\0a\ae\81\80\80"
  "\00\12\84\80\80\80\00\00\41\00\0b\84\80\80\80\00"
  "\00\41\01\0b\84\80\80\80\00\00\41\02\0b\84\80\80"
  "\80\00\00\41\03\0b\84\80\80\80\00\00\41\04\0b\84"
  "\80\80\80\00\00\41\05\0b\84\80\80\80\00\00\41\06"
  "\0b\84\80\80\80\00\00\41\07\0b\84\80\80\80\00\00"
  "\41\08\0b\84\80\80\80\00\00\41\09\0b\84\80\80\80"
  "\00\00\41\0a\0b\84\80\80\80\00\00\41\0b\0b\84\80"
  "\80\80\00\00\41\0c\0b\84\80\80\80\00\00\41\0d\0b"
  "\84\80\80\80\00\00\41\0e\0b\84\80\80\80\00\00\41"
  "\0f\0b\87\80\80\80\00\00\20\00\11\00\00\0b\8c\80"
  "\80\80\00\00\20\00\20\01\20\02\fc\0e\00\00\0b"
)
(module instance)
(assert_trap
  (invoke "run" (i32.const 0x0) (i32.const 0x70) (i32.const 0xffff_ffe0))
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
(assert_return (invoke "test" (i32.const 0x70)) (i32.const 0x0))
(assert_return (invoke "test" (i32.const 0x71)) (i32.const 0x1))
(assert_return (invoke "test" (i32.const 0x72)) (i32.const 0x2))
(assert_return (invoke "test" (i32.const 0x73)) (i32.const 0x3))
(assert_return (invoke "test" (i32.const 0x74)) (i32.const 0x4))
(assert_return (invoke "test" (i32.const 0x75)) (i32.const 0x5))
(assert_return (invoke "test" (i32.const 0x76)) (i32.const 0x6))
(assert_return (invoke "test" (i32.const 0x77)) (i32.const 0x7))
(assert_return (invoke "test" (i32.const 0x78)) (i32.const 0x8))
(assert_return (invoke "test" (i32.const 0x79)) (i32.const 0x9))
(assert_return (invoke "test" (i32.const 0x7a)) (i32.const 0xa))
(assert_return (invoke "test" (i32.const 0x7b)) (i32.const 0xb))
(assert_return (invoke "test" (i32.const 0x7c)) (i32.const 0xc))
(assert_return (invoke "test" (i32.const 0x7d)) (i32.const 0xd))
(assert_return (invoke "test" (i32.const 0x7e)) (i32.const 0xe))
(assert_return (invoke "test" (i32.const 0x7f)) (i32.const 0xf))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\90\80\80\80\00\03\60"
  "\00\01\7f\60\01\7f\01\7f\60\03\7f\7f\7f\00\03\93"
  "\80\80\80\00\12\00\00\00\00\00\00\00\00\00\00\00"
  "\00\00\00\00\00\01\02\04\87\80\80\80\00\01\70\01"
  "\80\01\80\01\07\e4\80\80\80\00\12\02\66\30\00\00"
  "\02\66\31\00\01\02\66\32\00\02\02\66\33\00\03\02"
  "\66\34\00\04\02\66\35\00\05\02\66\36\00\06\02\66"
  "\37\00\07\02\66\38\00\08\02\66\39\00\09\03\66\31"
  "\30\00\0a\03\66\31\31\00\0b\03\66\31\32\00\0c\03"
  "\66\31\33\00\0d\03\66\31\34\00\0e\03\66\31\35\00"
  "\0f\04\74\65\73\74\00\10\03\72\75\6e\00\11\09\96"
  "\80\80\80\00\01\00\41\00\0b\10\00\01\02\03\04\05"
  "\06\07\08\09\0a\0b\0c\0d\0e\0f\0a\ae\81\80\80\00"
  "\12\84\80\80\80\00\00\41\00\0b\84\80\80\80\00\00"
  "\41\01\0b\84\80\80\80\00\00\41\02\0b\84\80\80\80"
  "\00\00\41\03\0b\84\80\80\80\00\00\41\04\0b\84\80"
  "\80\80\00\00\41\05\0b\84\80\80\80\00\00\41\06\0b"
  "\84\80\80\80\00\00\41\07\0b\84\80\80\80\00\00\41"
  "\08\0b\84\80\80\80\00\00\41\09\0b\84\80\80\80\00"
  "\00\41\0a\0b\84\80\80\80\00\00\41\0b\0b\84\80\80"
  "\80\00\00\41\0c\0b\84\80\80\80\00\00\41\0d\0b\84"
  "\80\80\80\00\00\41\0e\0b\84\80\80\80\00\00\41\0f"
  "\0b\87\80\80\80\00\00\20\00\11\00\00\0b\8c\80\80"
  "\80\00\00\20\00\20\01\20\02\fc\0e\00\00\0b"
)
(module instance)
(assert_trap
  (invoke "run" (i32.const 0x70) (i32.const 0x0) (i32.const 0xffff_ffe0))
  "out of bounds table access"
)
(assert_return (invoke "test" (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "test" (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "test" (i32.const 0x2)) (i32.const 0x2))
(assert_return (invoke "test" (i32.const 0x3)) (i32.const 0x3))
(assert_return (invoke "test" (i32.const 0x4)) (i32.const 0x4))
(assert_return (invoke "test" (i32.const 0x5)) (i32.const 0x5))
(assert_return (invoke "test" (i32.const 0x6)) (i32.const 0x6))
(assert_return (invoke "test" (i32.const 0x7)) (i32.const 0x7))
(assert_return (invoke "test" (i32.const 0x8)) (i32.const 0x8))
(assert_return (invoke "test" (i32.const 0x9)) (i32.const 0x9))
(assert_return (invoke "test" (i32.const 0xa)) (i32.const 0xa))
(assert_return (invoke "test" (i32.const 0xb)) (i32.const 0xb))
(assert_return (invoke "test" (i32.const 0xc)) (i32.const 0xc))
(assert_return (invoke "test" (i32.const 0xd)) (i32.const 0xd))
(assert_return (invoke "test" (i32.const 0xe)) (i32.const 0xe))
(assert_return (invoke "test" (i32.const 0xf)) (i32.const 0xf))
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
