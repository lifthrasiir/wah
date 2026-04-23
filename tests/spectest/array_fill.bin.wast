(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\8a\80\80\80\00\02\5e"
    "\78\00\60\02\64\00\7f\00\03\82\80\80\80\00\01\01"
    "\07\98\80\80\80\00\01\14\61\72\72\61\79\2e\66\69"
    "\6c\6c\2d\69\6d\6d\75\74\61\62\6c\65\00\00\0a\93"
    "\80\80\80\00\01\8d\80\80\80\00\00\20\00\41\00\20"
    "\01\41\00\fb\10\00\0b"
  )
  "array is immutable"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\8a\80\80\80\00\02\5e"
    "\78\01\60\02\64\00\70\00\03\82\80\80\80\00\01\01"
    "\07\98\80\80\80\00\01\14\61\72\72\61\79\2e\66\69"
    "\6c\6c\2d\69\6e\76\61\6c\69\64\2d\31\00\00\0a\93"
    "\80\80\80\00\01\8d\80\80\80\00\00\20\00\41\00\20"
    "\01\41\00\fb\10\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\8a\80\80\80\00\02\5e"
    "\70\01\60\02\64\00\7f\00\03\82\80\80\80\00\01\01"
    "\07\98\80\80\80\00\01\14\61\72\72\61\79\2e\66\69"
    "\6c\6c\2d\69\6e\76\61\6c\69\64\2d\31\00\00\0a\93"
    "\80\80\80\00\01\8d\80\80\80\00\00\20\00\41\00\20"
    "\01\41\00\fb\10\00\0b"
  )
  "type mismatch"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\95\80\80\80\00\05\5e"
  "\78\00\5e\78\01\60\01\7f\01\7f\60\00\00\60\03\7f"
  "\7f\7f\00\03\84\80\80\80\00\03\02\03\04\06\95\80"
  "\80\80\00\02\64\00\00\41\0a\41\0c\fb\06\00\0b\64"
  "\01\01\41\0c\fb\07\01\0b\07\b0\80\80\80\00\03\0d"
  "\61\72\72\61\79\5f\67\65\74\5f\6e\74\68\00\00\0f"
  "\61\72\72\61\79\5f\66\69\6c\6c\2d\6e\75\6c\6c\00"
  "\01\0a\61\72\72\61\79\5f\66\69\6c\6c\00\02\0a\b3"
  "\80\80\80\00\03\89\80\80\80\00\00\23\01\20\00\fb"
  "\0d\01\0b\8d\80\80\80\00\00\d0\01\41\00\41\00\41"
  "\00\fb\10\01\0b\8d\80\80\80\00\00\23\01\20\00\20"
  "\01\20\02\fb\10\01\0b"
)
(module instance)
(assert_trap (invoke "array_fill-null") "null array reference")
(assert_trap
  (invoke "array_fill" (i32.const 0xd) (i32.const 0x0) (i32.const 0x0))
  "out of bounds array access"
)
(assert_trap
  (invoke "array_fill" (i32.const 0x0) (i32.const 0x0) (i32.const 0xd))
  "out of bounds array access"
)
(assert_return
  (invoke "array_fill" (i32.const 0xc) (i32.const 0x0) (i32.const 0x0))
)
(assert_return (invoke "array_get_nth" (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "array_get_nth" (i32.const 0x5)) (i32.const 0x0))
(assert_return (invoke "array_get_nth" (i32.const 0xb)) (i32.const 0x0))
(assert_trap
  (invoke "array_get_nth" (i32.const 0xc))
  "out of bounds array access"
)
(assert_return
  (invoke "array_fill" (i32.const 0x2) (i32.const 0xb) (i32.const 0x2))
)
(assert_return (invoke "array_get_nth" (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "array_get_nth" (i32.const 0x2)) (i32.const 0xb))
(assert_return (invoke "array_get_nth" (i32.const 0x3)) (i32.const 0xb))
(assert_return (invoke "array_get_nth" (i32.const 0x4)) (i32.const 0x0))
