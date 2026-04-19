(module definition binary
  "\00\61\73\6d\01\00\00\00\01\95\80\80\80\00\04\60"
  "\01\7f\01\6f\60\02\7f\6f\00\60\02\7f\6f\01\7f\60"
  "\00\01\7f\03\86\80\80\80\00\05\00\01\02\02\03\04"
  "\84\80\80\80\00\01\6f\00\00\07\a9\80\80\80\00\05"
  "\03\67\65\74\00\00\03\73\65\74\00\01\04\67\72\6f"
  "\77\00\02\0b\67\72\6f\77\2d\61\62\62\72\65\76\00"
  "\03\04\73\69\7a\65\00\04\0a\bf\80\80\80\00\05\86"
  "\80\80\80\00\00\20\00\25\00\0b\88\80\80\80\00\00"
  "\20\00\20\01\26\00\0b\89\80\80\80\00\00\20\01\20"
  "\00\fc\0f\00\0b\89\80\80\80\00\00\20\01\20\00\fc"
  "\0f\00\0b\85\80\80\80\00\00\fc\10\00\0b"
)
(module instance)
(assert_return (invoke "size") (i32.const 0x0))
(assert_trap
  (invoke "set" (i32.const 0x0) (ref.extern 2))
  "out of bounds table access"
)
(assert_trap (invoke "get" (i32.const 0x0)) "out of bounds table access")
(assert_return
  (invoke "grow" (i32.const 0x1) (ref.null extern))
  (i32.const 0x0)
)
(assert_return (invoke "size") (i32.const 0x1))
(assert_return (invoke "get" (i32.const 0x0)) (ref.null extern))
(assert_return (invoke "set" (i32.const 0x0) (ref.extern 2)))
(assert_return (invoke "get" (i32.const 0x0)) (ref.extern 2))
(assert_trap
  (invoke "set" (i32.const 0x1) (ref.extern 2))
  "out of bounds table access"
)
(assert_trap (invoke "get" (i32.const 0x1)) "out of bounds table access")
(assert_return
  (invoke "grow-abbrev" (i32.const 0x4) (ref.extern 3))
  (i32.const 0x1)
)
(assert_return (invoke "size") (i32.const 0x5))
(assert_return (invoke "get" (i32.const 0x0)) (ref.extern 2))
(assert_return (invoke "set" (i32.const 0x0) (ref.extern 2)))
(assert_return (invoke "get" (i32.const 0x0)) (ref.extern 2))
(assert_return (invoke "get" (i32.const 0x1)) (ref.extern 3))
(assert_return (invoke "get" (i32.const 0x4)) (ref.extern 3))
(assert_return (invoke "set" (i32.const 0x4) (ref.extern 4)))
(assert_return (invoke "get" (i32.const 0x4)) (ref.extern 4))
(assert_trap
  (invoke "set" (i32.const 0x5) (ref.extern 2))
  "out of bounds table access"
)
(assert_trap (invoke "get" (i32.const 0x5)) "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\00\01\7f\03\82\80\80\80\00\01\00\04\84\80\80\80"
  "\00\01\70\00\10\07\88\80\80\80\00\01\04\67\72\6f"
  "\77\00\00\09\85\80\80\80\00\01\03\00\01\00\0a\8f"
  "\80\80\80\00\01\89\80\80\80\00\00\d2\00\41\70\fc"
  "\0f\00\0b"
)
(module instance)
(assert_return (invoke "grow") (i32.const 0xffff_ffff))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7f\01\7f\03\82\80\80\80\00\01\00\04\84\80\80"
  "\80\00\01\6f\00\00\07\88\80\80\80\00\01\04\67\72"
  "\6f\77\00\00\0a\8f\80\80\80\00\01\89\80\80\80\00"
  "\00\d0\6f\20\00\fc\0f\00\0b"
)
(module instance)
(assert_return (invoke "grow" (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "grow" (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "grow" (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "grow" (i32.const 0x2)) (i32.const 0x1))
(assert_return (invoke "grow" (i32.const 0x320)) (i32.const 0x3))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7f\01\7f\03\82\80\80\80\00\01\00\04\85\80\80"
  "\80\00\01\6f\01\00\0a\07\88\80\80\80\00\01\04\67"
  "\72\6f\77\00\00\0a\8f\80\80\80\00\01\89\80\80\80"
  "\00\00\d0\6f\20\00\fc\0f\00\0b"
)
(module instance)
(assert_return (invoke "grow" (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "grow" (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "grow" (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "grow" (i32.const 0x2)) (i32.const 0x2))
(assert_return (invoke "grow" (i32.const 0x6)) (i32.const 0x4))
(assert_return (invoke "grow" (i32.const 0x0)) (i32.const 0xa))
(assert_return (invoke "grow" (i32.const 0x1)) (i32.const 0xffff_ffff))
(assert_return (invoke "grow" (i32.const 0x1_0000)) (i32.const 0xffff_ffff))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8c\80\80\80\00\02\60"
  "\01\7f\01\7f\60\02\7f\7f\01\70\03\83\80\80\80\00"
  "\02\00\01\04\84\80\80\80\00\01\70\00\0a\07\9b\80"
  "\80\80\00\02\04\67\72\6f\77\00\00\10\63\68\65\63"
  "\6b\2d\74\61\62\6c\65\2d\6e\75\6c\6c\00\01\09\85"
  "\80\80\80\00\01\03\00\01\01\0a\c5\80\80\80\00\02"
  "\89\80\80\80\00\00\d0\70\20\00\fc\0f\00\0b\b1\80"
  "\80\80\00\01\01\70\d2\01\21\02\02\40\03\40\20\00"
  "\25\00\21\02\20\02\d1\45\0d\01\20\00\20\01\4f\0d"
  "\01\20\00\41\01\6a\21\00\20\00\20\01\4d\0d\00\0b"
  "\0b\20\02\0b"
)
(module instance)
(assert_return
  (invoke "check-table-null" (i32.const 0x0) (i32.const 0x9))
  (ref.null func)
)
(assert_return (invoke "grow" (i32.const 0xa)) (i32.const 0xa))
(assert_return
  (invoke "check-table-null" (i32.const 0x0) (i32.const 0x13))
  (ref.null func)
)
(module definition $Tgt binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\00\01\7f\03\82\80\80\80\00\01\00\04\84\80\80\80"
  "\00\01\70\00\01\07\90\80\80\80\00\02\05\74\61\62"
  "\6c\65\01\00\04\67\72\6f\77\00\00\0a\8f\80\80\80"
  "\00\01\89\80\80\80\00\00\d0\70\41\01\fc\0f\00\0b"
)
(module instance $Tgt $Tgt)
(register "grown-table" $Tgt)
(assert_return (invoke $Tgt "grow") (i32.const 0x1))
(module definition $Tgit1 binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\00\01\7f\02\97\80\80\80\00\01\0b\67\72\6f\77\6e"
  "\2d\74\61\62\6c\65\05\74\61\62\6c\65\01\70\00\02"
  "\03\82\80\80\80\00\01\00\07\90\80\80\80\00\02\05"
  "\74\61\62\6c\65\01\00\04\67\72\6f\77\00\00\0a\8f"
  "\80\80\80\00\01\89\80\80\80\00\00\d0\70\41\01\fc"
  "\0f\00\0b"
)
(module instance $Tgit1 $Tgit1)
(register "grown-imported-table" $Tgit1)
(assert_return (invoke $Tgit1 "grow") (i32.const 0x2))
(module definition $Tgit2 binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\00\01\7f\02\a0\80\80\80\00\01\14\67\72\6f\77\6e"
  "\2d\69\6d\70\6f\72\74\65\64\2d\74\61\62\6c\65\05"
  "\74\61\62\6c\65\01\70\00\03\03\82\80\80\80\00\01"
  "\00\07\88\80\80\80\00\01\04\73\69\7a\65\00\00\0a"
  "\8b\80\80\80\00\01\85\80\80\80\00\00\fc\10\00\0b"
)
(module instance $Tgit2 $Tgit2)
(assert_return (invoke $Tgit2 "size") (i32.const 0x3))
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\04\84\80\80\80"
    "\00\01\6f\00\00\0a\8b\80\80\80\00\01\85\80\80\80"
    "\00\00\fc\0f\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\04\84\80\80\80"
    "\00\01\6f\00\00\0a\8d\80\80\80\00\01\87\80\80\80"
    "\00\00\d0\6f\fc\0f\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\04\84\80\80\80"
    "\00\01\6f\00\00\0a\8d\80\80\80\00\01\87\80\80\80"
    "\00\00\41\01\fc\0f\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7f\03\82\80\80\80\00\01\00\04\84\80\80\80"
    "\00\01\6f\00\00\0a\92\80\80\80\00\01\8c\80\80\80"
    "\00\00\d0\6f\43\00\00\80\3f\fc\0f\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
    "\01\6f\01\7f\03\82\80\80\80\00\01\00\04\84\80\80"
    "\80\00\01\70\00\00\0a\8f\80\80\80\00\01\89\80\80"
    "\80\00\00\20\00\41\01\fc\0f\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\04\84\80\80\80\00"
    "\01\6f\00\01\0a\8f\80\80\80\00\01\89\80\80\80\00"
    "\00\d0\6f\41\00\fc\0f\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\00\01\7d\03\82\80\80\80\00\01\00\04\84\80\80\80"
    "\00\01\6f\00\01\0a\8f\80\80\80\00\01\89\80\80\80"
    "\00\00\d0\6f\41\00\fc\0f\00\0b"
  )
  "type mismatch"
)
