(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\5e"
    "\78\00\60\01\64\00\00\03\82\80\80\80\00\01\01\07"
    "\9d\80\80\80\00\01\19\61\72\72\61\79\2e\69\6e\69"
    "\74\5f\64\61\74\61\2d\69\6d\6d\75\74\61\62\6c\65"
    "\00\00\0c\81\80\80\80\00\01\0a\94\80\80\80\00\01"
    "\8e\80\80\80\00\00\20\00\41\00\41\00\41\00\fb\12"
    "\00\00\0b\0b\84\80\80\80\00\01\01\01\61"
  )
  "array is immutable"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\5e"
    "\70\01\60\01\64\00\00\03\82\80\80\80\00\01\01\07"
    "\9d\80\80\80\00\01\19\61\72\72\61\79\2e\69\6e\69"
    "\74\5f\64\61\74\61\2d\69\6e\76\61\6c\69\64\2d\31"
    "\00\00\0c\81\80\80\80\00\01\0a\94\80\80\80\00\01"
    "\8e\80\80\80\00\00\20\00\41\00\41\00\41\00\fb\12"
    "\00\00\0b\0b\84\80\80\80\00\01\01\01\61"
  )
  "array type is not numeric or vector"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\98\80\80\80\00\06\5e"
  "\78\00\5e\78\01\5e\77\01\60\01\7f\01\7f\60\00\00"
  "\60\03\7f\7f\7f\00\03\87\80\80\80\00\06\03\03\04"
  "\05\05\04\06\9e\80\80\80\00\03\64\00\00\41\0a\41"
  "\0c\fb\06\00\0b\64\01\01\41\0c\fb\07\01\0b\64\02"
  "\00\41\06\fb\07\02\0b\07\f0\80\80\80\00\06\0d\61"
  "\72\72\61\79\5f\67\65\74\5f\6e\74\68\00\00\11\61"
  "\72\72\61\79\5f\67\65\74\5f\6e\74\68\5f\69\31\36"
  "\00\01\14\61\72\72\61\79\5f\69\6e\69\74\5f\64\61"
  "\74\61\2d\6e\75\6c\6c\00\02\0f\61\72\72\61\79\5f"
  "\69\6e\69\74\5f\64\61\74\61\00\03\13\61\72\72\61"
  "\79\5f\69\6e\69\74\5f\64\61\74\61\5f\69\31\36\00"
  "\04\09\64\72\6f\70\5f\73\65\67\73\00\05\0c\81\80"
  "\80\80\00\01\0a\e0\80\80\80\00\06\89\80\80\80\00"
  "\00\23\01\20\00\fb\0d\01\0b\89\80\80\80\00\00\23"
  "\02\20\00\fb\0d\02\0b\8e\80\80\80\00\00\d0\01\41"
  "\00\41\00\41\00\fb\12\01\00\0b\8e\80\80\80\00\00"
  "\23\01\20\00\20\01\20\02\fb\12\01\00\0b\8e\80\80"
  "\80\00\00\23\02\20\00\20\01\20\02\fb\12\02\00\0b"
  "\85\80\80\80\00\00\fc\09\00\0b\0b\8f\80\80\80\00"
  "\01\01\0c\61\62\63\64\65\66\67\68\69\6a\6b\6c"
)
(module instance)
(assert_trap (invoke "array_init_data-null") "null array reference")
(assert_trap
  (invoke "array_init_data" (i32.const 0xd) (i32.const 0x0) (i32.const 0x0))
  "out of bounds array access"
)
(assert_trap
  (invoke "array_init_data" (i32.const 0x0) (i32.const 0xd) (i32.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "array_init_data" (i32.const 0x0) (i32.const 0x0) (i32.const 0xd))
  "out of bounds array access"
)
(assert_trap
  (invoke "array_init_data" (i32.const 0x0) (i32.const 0x0) (i32.const 0xd))
  "out of bounds array access"
)
(assert_trap
  (invoke "array_init_data_i16" (i32.const 0x0) (i32.const 0x0) (i32.const 0x7))
  "out of bounds array access"
)
(assert_return
  (invoke "array_init_data" (i32.const 0xc) (i32.const 0x0) (i32.const 0x0))
)
(assert_return
  (invoke "array_init_data" (i32.const 0x0) (i32.const 0xc) (i32.const 0x0))
)
(assert_return
  (invoke "array_init_data_i16" (i32.const 0x0) (i32.const 0x6) (i32.const 0x0))
)
(assert_return (invoke "array_get_nth" (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "array_get_nth" (i32.const 0x5)) (i32.const 0x0))
(assert_return (invoke "array_get_nth" (i32.const 0xb)) (i32.const 0x0))
(assert_trap
  (invoke "array_get_nth" (i32.const 0xc))
  "out of bounds array access"
)
(assert_return (invoke "array_get_nth_i16" (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "array_get_nth_i16" (i32.const 0x2)) (i32.const 0x0))
(assert_return (invoke "array_get_nth_i16" (i32.const 0x5)) (i32.const 0x0))
(assert_trap
  (invoke "array_get_nth_i16" (i32.const 0x6))
  "out of bounds array access"
)
(assert_return
  (invoke "array_init_data" (i32.const 0x4) (i32.const 0x2) (i32.const 0x2))
)
(assert_return (invoke "array_get_nth" (i32.const 0x3)) (i32.const 0x0))
(assert_return (invoke "array_get_nth" (i32.const 0x4)) (i32.const 0x63))
(assert_return (invoke "array_get_nth" (i32.const 0x5)) (i32.const 0x64))
(assert_return (invoke "array_get_nth" (i32.const 0x6)) (i32.const 0x0))
(assert_return
  (invoke "array_init_data_i16" (i32.const 0x2) (i32.const 0x5) (i32.const 0x2))
)
(assert_return (invoke "array_get_nth_i16" (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "array_get_nth_i16" (i32.const 0x2)) (i32.const 0x6766))
(assert_return (invoke "array_get_nth_i16" (i32.const 0x3)) (i32.const 0x6968))
(assert_return (invoke "array_get_nth_i16" (i32.const 0x4)) (i32.const 0x0))
(assert_return (invoke "drop_segs"))
(assert_return
  (invoke "array_init_data" (i32.const 0x0) (i32.const 0x0) (i32.const 0x0))
)
(assert_trap
  (invoke "array_init_data" (i32.const 0x0) (i32.const 0x0) (i32.const 0x1))
  "out of bounds memory access"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8a\80\80\80\00\03\5e"
  "\7f\01\5e\7e\01\60\00\00\03\8d\80\80\80\00\0c\02"
  "\02\02\02\02\02\02\02\02\02\02\02\07\bd\80\80\80"
  "\00\0c\02\66\30\00\00\02\66\31\00\01\02\66\32\00"
  "\02\02\66\33\00\03\02\66\34\00\04\02\66\39\00\05"
  "\02\67\30\00\06\02\67\31\00\07\02\67\34\00\08\02"
  "\67\37\00\09\02\67\38\00\0a\02\67\39\00\0b\0c\81"
  "\80\80\80\00\07\0a\89\82\80\80\00\0c\91\80\80\80"
  "\00\00\41\01\fb\07\00\41\00\41\00\41\01\fb\12\00"
  "\00\0b\91\80\80\80\00\00\41\01\fb\07\00\41\00\41"
  "\00\41\01\fb\12\00\01\0b\91\80\80\80\00\00\41\01"
  "\fb\07\00\41\00\41\00\41\01\fb\12\00\02\0b\91\80"
  "\80\80\00\00\41\01\fb\07\00\41\00\41\00\41\01\fb"
  "\12\00\03\0b\91\80\80\80\00\00\41\01\fb\07\00\41"
  "\00\41\00\41\01\fb\12\00\04\0b\91\80\80\80\00\00"
  "\41\01\fb\07\00\41\00\41\06\41\01\fb\12\00\06\0b"
  "\91\80\80\80\00\00\41\01\fb\07\01\41\00\41\00\41"
  "\01\fb\12\01\00\0b\91\80\80\80\00\00\41\01\fb\07"
  "\01\41\00\41\00\41\01\fb\12\01\01\0b\91\80\80\80"
  "\00\00\41\01\fb\07\01\41\00\41\00\41\01\fb\12\01"
  "\04\0b\91\80\80\80\00\00\41\01\fb\07\01\41\00\41"
  "\00\41\01\fb\12\01\05\0b\91\80\80\80\00\00\41\01"
  "\fb\07\01\41\00\41\00\41\01\fb\12\01\06\0b\91\80"
  "\80\80\00\00\41\01\fb\07\01\41\00\41\02\41\01\fb"
  "\12\01\06\0b\0b\a9\80\80\80\00\07\01\00\01\01\31"
  "\01\02\31\32\01\03\31\32\33\01\04\31\32\33\34\01"
  "\07\31\32\33\34\35\36\37\01\09\31\32\33\34\35\36"
  "\37\38\39"
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
