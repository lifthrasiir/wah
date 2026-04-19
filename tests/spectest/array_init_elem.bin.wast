(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\5e"
    "\70\00\60\01\64\00\00\03\82\80\80\80\00\01\01\07"
    "\9d\80\80\80\00\01\19\61\72\72\61\79\2e\69\6e\69"
    "\74\5f\65\6c\65\6d\2d\69\6d\6d\75\74\61\62\6c\65"
    "\00\00\09\84\80\80\80\00\01\05\70\00\0a\94\80\80"
    "\80\00\01\8e\80\80\80\00\00\20\00\41\00\41\00\41"
    "\00\fb\13\00\00\0b"
  )
  "array is immutable"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\5e"
    "\78\01\60\01\64\00\00\03\82\80\80\80\00\01\01\07"
    "\9d\80\80\80\00\01\19\61\72\72\61\79\2e\69\6e\69"
    "\74\5f\65\6c\65\6d\2d\69\6e\76\61\6c\69\64\2d\31"
    "\00\00\09\84\80\80\80\00\01\05\70\00\0a\94\80\80"
    "\80\00\01\8e\80\80\80\00\00\20\00\41\00\41\00\41"
    "\00\fb\13\00\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\5e"
    "\70\01\60\01\64\00\00\03\82\80\80\80\00\01\01\07"
    "\9d\80\80\80\00\01\19\61\72\72\61\79\2e\69\6e\69"
    "\74\5f\65\6c\65\6d\2d\69\6e\76\61\6c\69\64\2d\32"
    "\00\00\09\84\80\80\80\00\01\05\6f\00\0a\94\80\80"
    "\80\00\01\8e\80\80\80\00\00\20\00\41\00\41\00\41"
    "\00\fb\13\00\00\0b"
  )
  "type mismatch"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\95\80\80\80\00\05\60"
  "\00\00\5e\64\00\00\5e\70\01\60\01\7f\00\60\03\7f"
  "\7f\7f\00\03\86\80\80\80\00\05\00\03\00\04\00\04"
  "\84\80\80\80\00\01\70\00\01\06\95\80\80\80\00\02"
  "\64\01\00\d2\00\41\0c\fb\06\01\0b\64\02\00\41\0c"
  "\fb\07\02\0b\07\c7\80\80\80\00\04\0e\61\72\72\61"
  "\79\5f\63\61\6c\6c\5f\6e\74\68\00\01\14\61\72\72"
  "\61\79\5f\69\6e\69\74\5f\65\6c\65\6d\2d\6e\75\6c"
  "\6c\00\02\0f\61\72\72\61\79\5f\69\6e\69\74\5f\65"
  "\6c\65\6d\00\03\09\64\72\6f\70\5f\73\65\67\73\00"
  "\04\09\90\80\80\80\00\01\01\00\0c\00\00\00\00\00"
  "\00\00\00\00\00\00\00\0a\cf\80\80\80\00\05\82\80"
  "\80\80\00\00\0b\92\80\80\80\00\00\41\00\23\01\20"
  "\00\fb\0b\02\26\00\41\00\11\00\00\0b\8e\80\80\80"
  "\00\00\d0\02\41\00\41\00\41\00\fb\13\02\00\0b\8e"
  "\80\80\80\00\00\23\01\20\00\20\01\20\02\fb\13\02"
  "\00\0b\85\80\80\80\00\00\fc\0d\00\0b"
)
(module instance)
(assert_trap (invoke "array_init_elem-null") "null array reference")
(assert_trap
  (invoke "array_init_elem" (i32.const 0xd) (i32.const 0x0) (i32.const 0x0))
  "out of bounds array access"
)
(assert_trap
  (invoke "array_init_elem" (i32.const 0x0) (i32.const 0xd) (i32.const 0x0))
  "out of bounds table access"
)
(assert_trap
  (invoke "array_init_elem" (i32.const 0x0) (i32.const 0x0) (i32.const 0xd))
  "out of bounds array access"
)
(assert_trap
  (invoke "array_init_elem" (i32.const 0x0) (i32.const 0x0) (i32.const 0xd))
  "out of bounds array access"
)
(assert_return
  (invoke "array_init_elem" (i32.const 0xc) (i32.const 0x0) (i32.const 0x0))
)
(assert_return
  (invoke "array_init_elem" (i32.const 0x0) (i32.const 0xc) (i32.const 0x0))
)
(assert_trap (invoke "array_call_nth" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke "array_call_nth" (i32.const 0x5)) "uninitialized element")
(assert_trap (invoke "array_call_nth" (i32.const 0xb)) "uninitialized element")
(assert_trap
  (invoke "array_call_nth" (i32.const 0xc))
  "out of bounds array access"
)
(assert_return
  (invoke "array_init_elem" (i32.const 0x2) (i32.const 0x3) (i32.const 0x2))
)
(assert_trap (invoke "array_call_nth" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke "array_call_nth" (i32.const 0x2)))
(assert_return (invoke "array_call_nth" (i32.const 0x3)))
(assert_trap (invoke "array_call_nth" (i32.const 0x4)) "uninitialized element")
(assert_return (invoke "drop_segs"))
(assert_return
  (invoke "array_init_elem" (i32.const 0x0) (i32.const 0x0) (i32.const 0x0))
)
(assert_trap
  (invoke "array_init_elem" (i32.const 0x0) (i32.const 0x0) (i32.const 0x1))
  "out of bounds table access"
)
