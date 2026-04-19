(module definition binary
  "\00\61\73\6d\01\00\00\00\01\c2\80\80\80\00\14\5e"
  "\78\00\5e\77\00\5e\7f\00\5e\7e\00\5e\7d\00\5e\7c"
  "\00\5e\6e\00\5e\64\6b\00\5e\64\00\00\5e\63\01\00"
  "\5e\78\01\5e\77\01\5e\7f\01\5e\7e\01\5e\7f\01\5e"
  "\7e\01\5e\6e\01\5e\64\6b\01\5e\64\00\01\5e\6c\01"
)
(module instance)
(assert_invalid
  (module binary "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\5e" "\63\0a\01")
  "unknown type"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\93\80\80\80\00\03\4e"
  "\02\5e\64\01\00\5e\64\00\00\5e\7f\00\60\01\64\02"
  "\00\03\82\80\80\80\00\01\03\0a\88\80\80\80\00\01"
  "\82\80\80\80\00\00\0b"
)
(module instance)
(assert_invalid
  (module binary "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\5e" "\64\01\00")
  "unknown type"
)
(assert_invalid
  (module binary "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\5e" "\64\01\01")
  "unknown type"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\b0\80\80\80\00\09\5e"
  "\7d\00\5e\7d\01\60\00\01\64\00\60\02\7f\64\00\01"
  "\7d\60\01\7f\01\7d\60\03\7f\64\01\7d\01\7d\60\02"
  "\7f\7d\01\7d\60\01\64\6a\01\7f\60\00\01\7f\03\88"
  "\80\80\80\00\07\02\03\04\05\06\07\08\06\98\80\80"
  "\80\00\02\64\00\00\43\00\00\80\3f\41\03\fb\06\00"
  "\0b\64\00\00\41\03\fb\07\00\0b\07\9d\80\80\80\00"
  "\04\03\6e\65\77\00\00\03\67\65\74\00\02\07\73\65"
  "\74\5f\67\65\74\00\04\03\6c\65\6e\00\06\0a\e7\80"
  "\80\80\00\07\87\80\80\80\00\00\41\03\fb\07\00\0b"
  "\89\80\80\80\00\00\20\01\20\00\fb\0b\00\0b\88\80"
  "\80\80\00\00\20\00\10\00\10\01\0b\92\80\80\80\00"
  "\00\20\01\20\00\20\02\fb\0e\01\20\01\20\00\fb\0b"
  "\01\0b\8d\80\80\80\00\00\20\00\41\03\fb\07\01\20"
  "\01\10\03\0b\86\80\80\80\00\00\20\00\fb\0f\0b\86"
  "\80\80\80\00\00\10\00\10\05\0b"
)
(module instance)
(assert_return (invoke "new") (ref.array))
(assert_return (invoke "new") (ref.eq))
(assert_return (invoke "get" (i32.const 0x0)) (f32.const 0x0p+0))
(assert_return
  (invoke "set_get" (i32.const 0x1) (f32.const 0x1.cp+2))
  (f32.const 0x1.cp+2)
)
(assert_return (invoke "len") (i32.const 0x3))
(assert_trap (invoke "get" (i32.const 0xa)) "out of bounds array access")
(assert_trap
  (invoke "set_get" (i32.const 0xa) (f32.const 0x1.cp+2))
  "out of bounds array access"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\b0\80\80\80\00\09\5e"
  "\7d\00\5e\7d\01\60\00\01\64\00\60\02\7f\64\00\01"
  "\7d\60\01\7f\01\7d\60\03\7f\64\01\7d\01\7d\60\02"
  "\7f\7d\01\7d\60\01\64\6a\01\7f\60\00\01\7f\03\88"
  "\80\80\80\00\07\02\03\04\05\06\07\08\06\93\80\80"
  "\80\00\01\64\00\00\43\00\00\80\3f\43\00\00\00\40"
  "\fb\08\00\02\0b\07\9d\80\80\80\00\04\03\6e\65\77"
  "\00\00\03\67\65\74\00\02\07\73\65\74\5f\67\65\74"
  "\00\04\03\6c\65\6e\00\06\0a\fe\80\80\80\00\07\90"
  "\80\80\80\00\00\43\00\00\80\3f\43\00\00\00\40\fb"
  "\08\00\02\0b\89\80\80\80\00\00\20\01\20\00\fb\0b"
  "\00\0b\88\80\80\80\00\00\20\00\10\00\10\01\0b\92"
  "\80\80\80\00\00\20\01\20\00\20\02\fb\0e\01\20\01"
  "\20\00\fb\0b\01\0b\9b\80\80\80\00\00\20\00\43\00"
  "\00\80\3f\43\00\00\00\40\43\00\00\40\40\fb\08\01"
  "\03\20\01\10\03\0b\86\80\80\80\00\00\20\00\fb\0f"
  "\0b\86\80\80\80\00\00\10\00\10\05\0b"
)
(module instance)
(assert_return (invoke "new") (ref.array))
(assert_return (invoke "new") (ref.eq))
(assert_return (invoke "get" (i32.const 0x0)) (f32.const 0x1p+0))
(assert_return
  (invoke "set_get" (i32.const 0x1) (f32.const 0x1.cp+2))
  (f32.const 0x1.cp+2)
)
(assert_return (invoke "len") (i32.const 0x2))
(assert_trap (invoke "get" (i32.const 0xa)) "out of bounds array access")
(assert_trap
  (invoke "set_get" (i32.const 0xa) (f32.const 0x1.cp+2))
  "out of bounds array access"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\b3\80\80\80\00\0a\5e"
  "\78\00\5e\78\01\60\00\01\64\00\60\02\7f\64\00\01"
  "\7f\60\01\7f\01\7f\60\03\7f\64\01\7f\01\7f\60\02"
  "\7f\7f\01\7f\60\01\64\6a\01\7f\60\00\01\7f\60\00"
  "\00\03\8c\80\80\80\00\0b\02\02\03\04\03\04\05\06"
  "\07\08\09\07\c2\80\80\80\00\07\03\6e\65\77\00\00"
  "\0c\6e\65\77\2d\6f\76\65\72\66\6c\6f\77\00\01\05"
  "\67\65\74\5f\75\00\03\05\67\65\74\5f\73\00\05\07"
  "\73\65\74\5f\67\65\74\00\07\03\6c\65\6e\00\09\09"
  "\64\72\6f\70\5f\73\65\67\73\00\0a\0c\81\80\80\80"
  "\00\01\0a\a9\81\80\80\00\0b\8a\80\80\80\00\00\41"
  "\01\41\03\fb\09\00\00\0b\92\80\80\80\00\00\41\80"
  "\80\80\80\78\41\80\80\80\80\78\fb\09\00\00\0b\89"
  "\80\80\80\00\00\20\01\20\00\fb\0d\00\0b\88\80\80"
  "\80\00\00\20\00\10\00\10\02\0b\89\80\80\80\00\00"
  "\20\01\20\00\fb\0c\00\0b\88\80\80\80\00\00\20\00"
  "\10\00\10\04\0b\92\80\80\80\00\00\20\01\20\00\20"
  "\02\fb\0e\01\20\01\20\00\fb\0d\01\0b\90\80\80\80"
  "\00\00\20\00\41\01\41\03\fb\09\01\00\20\01\10\06"
  "\0b\86\80\80\80\00\00\20\00\fb\0f\0b\86\80\80\80"
  "\00\00\10\00\10\08\0b\85\80\80\80\00\00\fc\09\00"
  "\0b\0b\88\80\80\80\00\01\01\05\00\01\02\ff\04"
)
(module instance)
(assert_return (invoke "new") (ref.array))
(assert_return (invoke "new") (ref.eq))
(assert_return (invoke "get_u" (i32.const 0x2)) (i32.const 0xff))
(assert_return (invoke "get_s" (i32.const 0x2)) (i32.const 0xffff_ffff))
(assert_return
  (invoke "set_get" (i32.const 0x1) (i32.const 0x7))
  (i32.const 0x7)
)
(assert_return (invoke "len") (i32.const 0x3))
(assert_trap (invoke "new-overflow") "out of bounds memory access")
(assert_trap (invoke "get_u" (i32.const 0xa)) "out of bounds array access")
(assert_trap (invoke "get_s" (i32.const 0xa)) "out of bounds array access")
(assert_trap
  (invoke "set_get" (i32.const 0xa) (i32.const 0x7))
  "out of bounds array access"
)
(assert_return (invoke "drop_segs"))
(assert_trap (invoke "new") "out of bounds memory access")
(assert_trap (invoke "new-overflow") "out of bounds memory access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\cd\80\80\80\00\0f\5e"
  "\78\00\5e\64\00\00\5e\64\00\01\5e\63\00\00\5e\6e"
  "\01\60\00\01\64\01\60\00\01\64\03\60\00\01\64\04"
  "\60\03\7f\7f\64\01\01\7f\60\02\7f\7f\01\7f\60\04"
  "\7f\7f\64\02\7f\01\7f\60\03\7f\7f\7f\01\7f\60\01"
  "\64\6a\01\7f\60\00\01\7f\60\00\00\03\8c\80\80\80"
  "\00\0b\05\05\06\07\08\09\0a\0b\0c\0d\0e\07\b8\80"
  "\80\80\00\06\03\6e\65\77\00\00\0c\6e\65\77\2d\6f"
  "\76\65\72\66\6c\6f\77\00\01\03\67\65\74\00\05\07"
  "\73\65\74\5f\67\65\74\00\07\03\6c\65\6e\00\09\09"
  "\64\72\6f\70\5f\73\65\67\73\00\0a\09\96\80\80\80"
  "\00\01\05\64\00\02\41\07\41\03\fb\06\00\0b\41\01"
  "\41\02\fb\08\00\02\0b\0a\bf\81\80\80\00\0b\8a\80"
  "\80\80\00\00\41\00\41\02\fb\0a\01\00\0b\92\80\80"
  "\80\00\00\41\80\80\80\80\78\41\80\80\80\80\78\fb"
  "\0a\01\00\0b\8a\80\80\80\00\00\41\00\41\02\fb\0a"
  "\03\00\0b\8a\80\80\80\00\00\41\00\41\02\fb\0a\04"
  "\00\0b\8e\80\80\80\00\00\20\02\20\00\fb\0b\01\20"
  "\01\fb\0d\00\0b\8a\80\80\80\00\00\20\00\20\01\10"
  "\00\10\04\0b\9c\80\80\80\00\00\20\02\20\00\20\02"
  "\20\03\fb\0b\02\fb\0e\02\20\02\20\00\fb\0b\02\20"
  "\01\fb\0d\00\0b\92\80\80\80\00\00\20\00\20\01\41"
  "\00\41\02\fb\0a\02\00\20\02\10\06\0b\86\80\80\80"
  "\00\00\20\00\fb\0f\0b\86\80\80\80\00\00\10\00\10"
  "\08\0b\85\80\80\80\00\00\fc\0d\00\0b"
)
(module instance)
(assert_return (invoke "new") (ref.array))
(assert_return (invoke "new") (ref.eq))
(assert_return (invoke "get" (i32.const 0x0) (i32.const 0x0)) (i32.const 0x7))
(assert_return (invoke "get" (i32.const 0x1) (i32.const 0x0)) (i32.const 0x1))
(assert_return
  (invoke "set_get" (i32.const 0x0) (i32.const 0x1) (i32.const 0x1))
  (i32.const 0x2)
)
(assert_return (invoke "len") (i32.const 0x2))
(assert_trap (invoke "new-overflow") "out of bounds table access")
(assert_trap
  (invoke "get" (i32.const 0xa) (i32.const 0x0))
  "out of bounds array access"
)
(assert_trap
  (invoke "set_get" (i32.const 0xa) (i32.const 0x0) (i32.const 0x0))
  "out of bounds array access"
)
(assert_return (invoke "drop_segs"))
(assert_trap (invoke "new") "out of bounds table access")
(assert_trap (invoke "new-overflow") "out of bounds table access")
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\5e"
    "\7e\00\60\01\64\00\00\03\82\80\80\80\00\01\01\07"
    "\97\80\80\80\00\01\13\61\72\72\61\79\2e\73\65\74"
    "\2d\69\6d\6d\75\74\61\62\6c\65\00\00\0a\91\80\80"
    "\80\00\01\8b\80\80\80\00\00\20\00\41\00\42\01\fb"
    "\0e\00\0b"
  )
  "array is immutable"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\5e"
    "\78\00\06\8d\80\80\80\00\01\64\00\00\41\01\41\03"
    "\fb\09\00\00\0b\0c\81\80\80\80\00\01\0b\88\80\80"
    "\80\00\01\01\05\00\01\02\03\04"
  )
  "constant expression required"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\5e"
    "\78\00\5e\64\00\00\06\8d\80\80\80\00\01\64\01\00"
    "\41\00\41\01\fb\0a\01\00\0b\09\88\80\80\80\00\01"
    "\05\64\00\01\d0\00\0b"
  )
  "constant expression required"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\87\80\80\80\00\02\5e"
  "\7f\01\60\00\00\03\83\80\80\80\00\02\01\01\07\a3"
  "\80\80\80\00\02\0e\61\72\72\61\79\2e\67\65\74\2d"
  "\6e\75\6c\6c\00\00\0e\61\72\72\61\79\2e\73\65\74"
  "\2d\6e\75\6c\6c\00\01\0a\a6\80\80\80\00\02\8d\80"
  "\80\80\00\01\01\63\00\20\00\41\00\fb\0b\00\1a\0b"
  "\8e\80\80\80\00\01\01\63\00\20\00\41\00\41\00\fb"
  "\0e\00\0b"
)
(module instance)
(assert_trap (invoke "array.get-null") "null array reference")
(assert_trap (invoke "array.set-null") "null array reference")
