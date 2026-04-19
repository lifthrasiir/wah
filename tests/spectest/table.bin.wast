(module definition binary
  "\00\61\73\6d\01\00\00\00\04\84\80\80\80\00\01\70"
  "\00\00"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\84\80\80\80\00\01\70"
  "\00\01"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\85\80\80\80\00\01\70"
  "\01\00\00"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\85\80\80\80\00\01\70"
  "\01\00\01"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\86\80\80\80\00\01\70"
  "\01\01\80\02"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\87\80\80\80\00\01\70"
  "\01\00\80\80\04"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\88\80\80\80\00\01\70"
  "\00\ff\ff\ff\ff\0f"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\89\80\80\80\00\01\70"
  "\01\00\ff\ff\ff\ff\0f"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\84\80\80\80\00\01\70"
  "\00\01"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\84\80\80\80\00\01\6f"
  "\00\01"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\04\85\80\80\80\00\01\63\00\00\01"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\87\80\80\80\00\02\70"
  "\00\00\70\00\00"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\94\80\80\80\00\01\08"
  "\73\70\65\63\74\65\73\74\05\74\61\62\6c\65\01\70"
  "\00\00\04\84\80\80\80\00\01\70\00\00"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\84\80\80\80\00\01\70"
  "\00\00"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\84\80\80\80\00\01\70"
  "\00\01"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\84\80\80\80\00\01\70"
  "\00\01"
)
(module instance)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\09\86\80\80\80\00\01\00"
    "\41\00\0b\00"
  )
  "unknown table"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\09\87\80\80\80\00"
    "\01\00\41\00\0b\01\00\0a\88\80\80\80\00\01\82\80"
    "\80\80\00\00\0b"
  )
  "unknown table"
)
(assert_invalid
  (module binary "\00\61\73\6d\01\00\00\00\04\85\80\80\80\00\01\70" "\01\01\00")
  "size minimum must not be greater than maximum"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\04\89\80\80\80\00\01\70"
    "\01\ff\ff\ff\ff\0f\00"
  )
  "size minimum must not be greater than maximum"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\04\88\80\80\80\00\01\70"
    "\00\80\80\80\80\10"
  )
  "table size"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\04\8d\80\80\80\00\01\70"
    "\01\80\80\80\80\10\80\80\80\80\10"
  )
  "table size"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\04\89\80\80\80\00\01\70"
    "\01\00\80\80\80\80\10"
  )
  "table size"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\09\86\80\80\80\00\01\00"
    "\41\00\0b\00"
  )
  "unknown table"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\09\87\80\80\80\00"
    "\01\00\41\00\0b\01\00\0a\88\80\80\80\00\01\82\80"
    "\80\80\00\00\0b"
  )
  "unknown table"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\04\89\80\80\80\00\01\40"
    "\00\70\00\01\41\00\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\04\8a\80\80\80\00\01\40"
    "\00\64\70\00\01\d0\6f\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\04\8a\80\80\80\00\01\40\00\64\00\00\01\d0"
    "\70\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary "\00\61\73\6d\01\00\00\00\04\85\80\80\80\00\01\64" "\70\00\01")
  "type mismatch"
)
(assert_invalid
  (module binary "\00\61\73\6d\01\00\00\00\04\85\80\80\80\00\01\64" "\70\00\00")
  "type mismatch"
)
(assert_invalid
  (module binary "\00\61\73\6d\01\00\00\00\04\85\80\80\80\00\01\64" "\6f\00\00")
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\04\85\80\80\80\00\01\64\00\00\00"
  )
  "type mismatch"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\03\82\80\80\80\00\01\00\06\87\80\80\80\00"
  "\01\64\00\00\d2\00\0b\07\85\80\80\80\00\01\01\67"
  "\03\00\0a\88\80\80\80\00\01\82\80\80\80\00\00\0b"
)
(module instance)
(register "M")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\00\60\00\01\70\02\89\80\80\80\00\01\01\4d\01"
  "\67\03\64\00\00\03\87\80\80\80\00\06\00\01\01\01"
  "\01\01\04\a6\80\80\80\00\05\70\00\0a\40\00\70\00"
  "\0a\d2\00\0b\40\00\64\00\00\0a\d2\00\0b\40\00\70"
  "\00\0a\23\00\0b\40\00\64\00\00\0a\23\00\0b\07\a4"
  "\80\80\80\00\05\04\67\65\74\31\00\01\04\67\65\74"
  "\32\00\02\04\67\65\74\33\00\03\04\67\65\74\34\00"
  "\04\04\67\65\74\35\00\05\0a\bf\80\80\80\00\06\82"
  "\80\80\80\00\00\0b\86\80\80\80\00\00\41\01\25\00"
  "\0b\86\80\80\80\00\00\41\04\25\01\0b\86\80\80\80"
  "\00\00\41\07\25\02\0b\86\80\80\80\00\00\41\08\25"
  "\03\0b\86\80\80\80\00\00\41\09\25\04\0b"
)
(module instance)
(assert_return (invoke "get1") (ref.null))
(assert_return (invoke "get2") (ref.func))
(assert_return (invoke "get3") (ref.func))
(assert_return (invoke "get4") (ref.func))
(assert_return (invoke "get5") (ref.func))
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\04\85\80\80\80\00\01\64\00\00\0a"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\04\85\80\80\80\00\01\64\00\00\00"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\04\86\80\80\80\00\01\64\00\01\00\00"
  )
  "type mismatch"
)
