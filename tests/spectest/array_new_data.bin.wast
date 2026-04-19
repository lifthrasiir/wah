(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8b\80\80\80\00\02\5e"
  "\78\01\60\02\7f\7f\01\64\00\03\82\80\80\80\00\01"
  "\01\07\92\80\80\80\00\01\0e\61\72\72\61\79\2d\6e"
  "\65\77\2d\64\61\74\61\00\00\0c\81\80\80\80\00\01"
  "\0a\90\80\80\80\00\01\8a\80\80\80\00\00\20\00\20"
  "\01\fb\09\00\00\0b\0b\87\80\80\80\00\01\01\04\61"
  "\62\63\64"
)
(module instance)
(assert_return
  (invoke "array-new-data" (i32.const 0x0) (i32.const 0x0))
  (ref.array)
)
(assert_return
  (invoke "array-new-data" (i32.const 0x0) (i32.const 0x4))
  (ref.array)
)
(assert_return
  (invoke "array-new-data" (i32.const 0x1) (i32.const 0x2))
  (ref.array)
)
(assert_return
  (invoke "array-new-data" (i32.const 0x4) (i32.const 0x0))
  (ref.array)
)
(assert_trap
  (invoke "array-new-data" (i32.const 0x0) (i32.const 0x5))
  "out of bounds memory access"
)
(assert_trap
  (invoke "array-new-data" (i32.const 0x5) (i32.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "array-new-data" (i32.const 0x1) (i32.const 0x4))
  "out of bounds memory access"
)
(assert_trap
  (invoke "array-new-data" (i32.const 0x4) (i32.const 0x1))
  "out of bounds memory access"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8a\80\80\80\00\03\5e"
  "\7f\00\5e\7e\00\60\00\00\03\8d\80\80\80\00\0c\02"
  "\02\02\02\02\02\02\02\02\02\02\02\07\bd\80\80\80"
  "\00\0c\02\66\30\00\00\02\66\31\00\01\02\66\32\00"
  "\02\02\66\33\00\03\02\66\34\00\04\02\66\39\00\05"
  "\02\67\30\00\06\02\67\31\00\07\02\67\34\00\08\02"
  "\67\37\00\09\02\67\38\00\0a\02\67\39\00\0b\0c\81"
  "\80\80\80\00\07\0a\c1\81\80\80\00\0c\8b\80\80\80"
  "\00\00\41\00\41\01\fb\09\00\00\1a\0b\8b\80\80\80"
  "\00\00\41\00\41\01\fb\09\00\01\1a\0b\8b\80\80\80"
  "\00\00\41\00\41\01\fb\09\00\02\1a\0b\8b\80\80\80"
  "\00\00\41\00\41\01\fb\09\00\03\1a\0b\8b\80\80\80"
  "\00\00\41\00\41\01\fb\09\00\04\1a\0b\8b\80\80\80"
  "\00\00\41\06\41\01\fb\09\00\06\1a\0b\8b\80\80\80"
  "\00\00\41\00\41\01\fb\09\01\00\1a\0b\8b\80\80\80"
  "\00\00\41\00\41\01\fb\09\01\01\1a\0b\8b\80\80\80"
  "\00\00\41\00\41\01\fb\09\01\04\1a\0b\8b\80\80\80"
  "\00\00\41\00\41\01\fb\09\01\05\1a\0b\8b\80\80\80"
  "\00\00\41\00\41\01\fb\09\01\06\1a\0b\8b\80\80\80"
  "\00\00\41\02\41\01\fb\09\01\06\1a\0b\0b\a9\80\80"
  "\80\00\07\01\00\01\01\31\01\02\31\32\01\03\31\32"
  "\33\01\04\31\32\33\34\01\07\31\32\33\34\35\36\37"
  "\01\09\31\32\33\34\35\36\37\38\39"
)
(module instance)
(assert_trap (invoke "f0") "out of bounds memory access")
(assert_trap (invoke "f1") "out of bounds memory access")
(assert_trap (invoke "f2") "out of bounds memory access")
(assert_trap (invoke "f3") "out of bounds memory access")
(assert_return (invoke "f4"))
(assert_trap (invoke "f9") "out of bounds memory access")
(assert_trap (invoke "g0") "out of bounds memory access")
(assert_trap (invoke "g1") "out of bounds memory access")
(assert_trap (invoke "g4") "out of bounds memory access")
(assert_trap (invoke "g7") "out of bounds memory access")
(assert_return (invoke "g8"))
(assert_trap (invoke "g9") "out of bounds memory access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\5e"
  "\78\01\60\00\02\7f\7f\03\82\80\80\80\00\01\01\07"
  "\9b\80\80\80\00\01\17\61\72\72\61\79\2d\6e\65\77"
  "\2d\64\61\74\61\2d\63\6f\6e\74\65\6e\74\73\00\00"
  "\0c\81\80\80\80\00\01\0a\a3\80\80\80\00\01\9d\80"
  "\80\80\00\01\01\64\00\41\01\41\02\fb\09\00\00\21"
  "\00\20\00\41\00\fb\0d\00\20\00\41\01\fb\0d\00\0b"
  "\0b\87\80\80\80\00\01\01\04\aa\bb\cc\dd"
)
(module instance)
(assert_return
  (invoke "array-new-data-contents")
  (i32.const 0xbb)
  (i32.const 0xcc)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\5e"
  "\7f\01\60\00\01\7f\03\82\80\80\80\00\01\01\07\a0"
  "\80\80\80\00\01\1c\61\72\72\61\79\2d\6e\65\77\2d"
  "\64\61\74\61\2d\6c\69\74\74\6c\65\2d\65\6e\64\69"
  "\61\6e\00\00\0c\81\80\80\80\00\01\0a\95\80\80\80"
  "\00\01\8f\80\80\80\00\00\41\00\41\01\fb\09\00\00"
  "\41\00\fb\0b\00\0b\0b\87\80\80\80\00\01\01\04\aa"
  "\bb\cc\dd"
)
(module instance)
(assert_return (invoke "array-new-data-little-endian") (i32.const 0xddcc_bbaa))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\5e"
  "\77\01\60\00\01\7f\03\82\80\80\80\00\01\01\07\9c"
  "\80\80\80\00\01\18\61\72\72\61\79\2d\6e\65\77\2d"
  "\64\61\74\61\2d\75\6e\61\6c\69\67\6e\65\64\00\00"
  "\0c\81\80\80\80\00\01\0a\95\80\80\80\00\01\8f\80"
  "\80\80\00\00\41\01\41\01\fb\09\00\00\41\00\fb\0d"
  "\00\0b\0b\86\80\80\80\00\01\01\03\00\11\22"
)
(module instance)
(assert_return (invoke "array-new-data-unaligned") (i32.const 0x2211))
