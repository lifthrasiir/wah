(module definition binary
  "\00\61\73\6d\01\00\00\00\01\97\80\80\80\00\04\60"
  "\03\7f\6f\7f\00\60\01\7f\01\6f\60\03\7e\6f\7e\00"
  "\60\01\7e\01\6f\03\86\80\80\80\00\05\00\00\01\02"
  "\03\04\87\80\80\80\00\02\6f\00\0a\6f\04\0a\07\b1"
  "\80\80\80\00\05\04\66\69\6c\6c\00\00\0b\66\69\6c"
  "\6c\2d\61\62\62\72\65\76\00\01\03\67\65\74\00\02"
  "\08\66\69\6c\6c\2d\74\36\34\00\03\07\67\65\74\2d"
  "\74\36\34\00\04\0a\c7\80\80\80\00\05\8b\80\80\80"
  "\00\00\20\00\20\01\20\02\fc\11\00\0b\8b\80\80\80"
  "\00\00\20\00\20\01\20\02\fc\11\00\0b\86\80\80\80"
  "\00\00\20\00\25\00\0b\8b\80\80\80\00\00\20\00\20"
  "\01\20\02\fc\11\01\0b\86\80\80\80\00\00\20\00\25"
  "\01\0b"
)
(module instance)
(assert_return (invoke "get" (i32.const 0x1)) (ref.null extern))
(assert_return (invoke "get" (i32.const 0x2)) (ref.null extern))
(assert_return (invoke "get" (i32.const 0x3)) (ref.null extern))
(assert_return (invoke "get" (i32.const 0x4)) (ref.null extern))
(assert_return (invoke "get" (i32.const 0x5)) (ref.null extern))
(assert_return (invoke "fill" (i32.const 0x2) (ref.extern 1) (i32.const 0x3)))
(assert_return (invoke "get" (i32.const 0x1)) (ref.null extern))
(assert_return (invoke "get" (i32.const 0x2)) (ref.extern 1))
(assert_return (invoke "get" (i32.const 0x3)) (ref.extern 1))
(assert_return (invoke "get" (i32.const 0x4)) (ref.extern 1))
(assert_return (invoke "get" (i32.const 0x5)) (ref.null extern))
(assert_return (invoke "fill" (i32.const 0x4) (ref.extern 2) (i32.const 0x2)))
(assert_return (invoke "get" (i32.const 0x3)) (ref.extern 1))
(assert_return (invoke "get" (i32.const 0x4)) (ref.extern 2))
(assert_return (invoke "get" (i32.const 0x5)) (ref.extern 2))
(assert_return (invoke "get" (i32.const 0x6)) (ref.null extern))
(assert_return (invoke "fill" (i32.const 0x4) (ref.extern 3) (i32.const 0x0)))
(assert_return (invoke "get" (i32.const 0x3)) (ref.extern 1))
(assert_return (invoke "get" (i32.const 0x4)) (ref.extern 2))
(assert_return (invoke "get" (i32.const 0x5)) (ref.extern 2))
(assert_return (invoke "fill" (i32.const 0x8) (ref.extern 4) (i32.const 0x2)))
(assert_return (invoke "get" (i32.const 0x7)) (ref.null extern))
(assert_return (invoke "get" (i32.const 0x8)) (ref.extern 4))
(assert_return (invoke "get" (i32.const 0x9)) (ref.extern 4))
(assert_return
  (invoke "fill-abbrev" (i32.const 0x9) (ref.null extern) (i32.const 0x1))
)
(assert_return (invoke "get" (i32.const 0x8)) (ref.extern 4))
(assert_return (invoke "get" (i32.const 0x9)) (ref.null extern))
(assert_return (invoke "fill" (i32.const 0xa) (ref.extern 5) (i32.const 0x0)))
(assert_return (invoke "get" (i32.const 0x9)) (ref.null extern))
(assert_trap
  (invoke "fill" (i32.const 0x8) (ref.extern 6) (i32.const 0x3))
  "out of bounds table access"
)
(assert_return (invoke "get" (i32.const 0x7)) (ref.null extern))
(assert_return (invoke "get" (i32.const 0x8)) (ref.extern 4))
(assert_return (invoke "get" (i32.const 0x9)) (ref.null extern))
(assert_trap
  (invoke "fill" (i32.const 0xb) (ref.null extern) (i32.const 0x0))
  "out of bounds table access"
)
(assert_trap
  (invoke "fill" (i32.const 0xb) (ref.null extern) (i32.const 0xa))
  "out of bounds table access"
)
(assert_return (invoke "get-t64" (i64.const 0x1)) (ref.null extern))
(assert_return (invoke "get-t64" (i64.const 0x2)) (ref.null extern))
(assert_return (invoke "get-t64" (i64.const 0x3)) (ref.null extern))
(assert_return (invoke "get-t64" (i64.const 0x4)) (ref.null extern))
(assert_return (invoke "get-t64" (i64.const 0x5)) (ref.null extern))
(assert_return
  (invoke "fill-t64" (i64.const 0x2) (ref.extern 1) (i64.const 0x3))
)
(assert_return (invoke "get-t64" (i64.const 0x1)) (ref.null extern))
(assert_return (invoke "get-t64" (i64.const 0x2)) (ref.extern 1))
(assert_return (invoke "get-t64" (i64.const 0x3)) (ref.extern 1))
(assert_return (invoke "get-t64" (i64.const 0x4)) (ref.extern 1))
(assert_return (invoke "get-t64" (i64.const 0x5)) (ref.null extern))
(assert_return
  (invoke "fill-t64" (i64.const 0x4) (ref.extern 2) (i64.const 0x2))
)
(assert_return (invoke "get-t64" (i64.const 0x3)) (ref.extern 1))
(assert_return (invoke "get-t64" (i64.const 0x4)) (ref.extern 2))
(assert_return (invoke "get-t64" (i64.const 0x5)) (ref.extern 2))
(assert_return (invoke "get-t64" (i64.const 0x6)) (ref.null extern))
(assert_return
  (invoke "fill-t64" (i64.const 0x4) (ref.extern 3) (i64.const 0x0))
)
(assert_return (invoke "get-t64" (i64.const 0x3)) (ref.extern 1))
(assert_return (invoke "get-t64" (i64.const 0x4)) (ref.extern 2))
(assert_return (invoke "get-t64" (i64.const 0x5)) (ref.extern 2))
(assert_return
  (invoke "fill-t64" (i64.const 0x8) (ref.extern 4) (i64.const 0x2))
)
(assert_return (invoke "get-t64" (i64.const 0x7)) (ref.null extern))
(assert_return (invoke "get-t64" (i64.const 0x8)) (ref.extern 4))
(assert_return (invoke "get-t64" (i64.const 0x9)) (ref.extern 4))
(assert_return
  (invoke "fill-t64" (i64.const 0x9) (ref.null extern) (i64.const 0x1))
)
(assert_return (invoke "get-t64" (i64.const 0x8)) (ref.extern 4))
(assert_return (invoke "get-t64" (i64.const 0x9)) (ref.null extern))
(assert_return
  (invoke "fill-t64" (i64.const 0xa) (ref.extern 5) (i64.const 0x0))
)
(assert_return (invoke "get-t64" (i64.const 0x9)) (ref.null extern))
(assert_trap
  (invoke "fill-t64" (i64.const 0x8) (ref.extern 6) (i64.const 0x3))
  "out of bounds table access"
)
(assert_return (invoke "get-t64" (i64.const 0x7)) (ref.null extern))
(assert_return (invoke "get-t64" (i64.const 0x8)) (ref.extern 4))
(assert_return (invoke "get-t64" (i64.const 0x9)) (ref.null extern))
(assert_trap
  (invoke "fill-t64" (i64.const 0xb) (ref.null extern) (i64.const 0x0))
  "out of bounds table access"
)
(assert_trap
  (invoke "fill-t64" (i64.const 0xb) (ref.null extern) (i64.const 0xa))
  "out of bounds table access"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\04\84\80\80\80\00"
    "\01\6f\00\0a\0a\8b\80\80\80\00\01\85\80\80\80\00"
    "\00\fc\11\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\04\84\80\80\80\00"
    "\01\6f\00\0a\0a\8f\80\80\80\00\01\89\80\80\80\00"
    "\00\d0\6f\41\01\fc\11\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\04\84\80\80\80\00"
    "\01\6f\00\0a\0a\8f\80\80\80\00\01\89\80\80\80\00"
    "\00\41\01\41\01\fc\11\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\04\84\80\80\80\00"
    "\01\6f\00\0a\0a\8f\80\80\80\00\01\89\80\80\80\00"
    "\00\41\01\d0\6f\fc\11\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\04\84\80\80\80\00"
    "\01\6f\00\00\0a\94\80\80\80\00\01\8e\80\80\80\00"
    "\00\43\00\00\80\3f\d0\6f\41\01\fc\11\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\01\6f\00\03\82\80\80\80\00\01\00\04\84\80\80\80"
    "\00\01\70\00\00\0a\91\80\80\80\00\01\8b\80\80\80"
    "\00\00\41\01\20\00\41\01\fc\11\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\04\84\80\80\80\00"
    "\01\6f\00\00\0a\94\80\80\80\00\01\8e\80\80\80\00"
    "\00\41\01\d0\6f\43\00\00\80\3f\fc\11\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\01\6f\00\03\82\80\80\80\00\01\00\04\87\80\80\80"
    "\00\02\6f\00\01\70\00\01\0a\91\80\80\80\00\01\8b"
    "\80\80\80\00\00\41\00\20\00\41\01\fc\11\01\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\04\84\80\80\80"
    "\00\01\6f\00\01\0a\91\80\80\80\00\01\8b\80\80\80"
    "\00\00\41\00\d0\6f\41\01\fc\11\00\0b"
  )
  "type mismatch"
)
