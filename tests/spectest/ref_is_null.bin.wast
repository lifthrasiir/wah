(module definition binary
  "\00\61\73\6d\01\00\00\00\01\a1\80\80\80\00\07\60"
  "\00\00\60\01\70\01\7f\60\01\6f\01\7f\60\01\63\00"
  "\01\7f\60\00\01\7f\60\01\6f\00\60\01\7f\01\7f\03"
  "\8b\80\80\80\00\0a\00\01\02\03\04\05\00\06\06\06"
  "\04\8b\80\80\80\00\03\70\00\02\6f\00\02\63\00\00"
  "\02\07\dd\80\80\80\00\08\07\66\75\6e\63\72\65\66"
  "\00\01\09\65\78\74\65\72\6e\72\65\66\00\02\08\72"
  "\65\66\2d\6e\75\6c\6c\00\04\04\69\6e\69\74\00\05"
  "\06\64\65\69\6e\69\74\00\06\0c\66\75\6e\63\72\65"
  "\66\2d\65\6c\65\6d\00\07\0e\65\78\74\65\72\6e\72"
  "\65\66\2d\65\6c\65\6d\00\08\08\72\65\66\2d\65\6c"
  "\65\6d\00\09\09\92\80\80\80\00\02\00\41\01\0b\01"
  "\00\06\02\41\01\0b\64\00\01\d2\00\0b\0a\fe\80\80"
  "\80\00\0a\82\80\80\80\00\00\0b\85\80\80\80\00\00"
  "\20\00\d1\0b\85\80\80\80\00\00\20\00\d1\0b\85\80"
  "\80\80\00\00\20\00\d1\0b\86\80\80\80\00\00\d0\00"
  "\10\03\0b\88\80\80\80\00\00\41\01\20\00\26\01\0b"
  "\94\80\80\80\00\00\41\01\d0\70\26\00\41\01\d0\6f"
  "\26\01\41\01\d0\00\26\02\0b\88\80\80\80\00\00\20"
  "\00\25\00\10\01\0b\88\80\80\80\00\00\20\00\25\01"
  "\10\02\0b\88\80\80\80\00\00\20\00\25\02\10\03\0b"
)
(module instance)
(assert_return (invoke "funcref" (ref.null func)) (i32.const 0x1))
(assert_return (invoke "externref" (ref.null extern)) (i32.const 0x1))
(assert_return (invoke "ref-null") (i32.const 0x1))
(assert_return (invoke "externref" (ref.extern 1)) (i32.const 0x0))
(invoke "init" (ref.extern 0))
(assert_return (invoke "funcref-elem" (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "externref-elem" (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ref-elem" (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "funcref-elem" (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "externref-elem" (i32.const 0x1)) (i32.const 0x0))
(assert_return (invoke "ref-elem" (i32.const 0x1)) (i32.const 0x0))
(invoke "deinit")
(assert_return (invoke "funcref-elem" (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "externref-elem" (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ref-elem" (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "funcref-elem" (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "externref-elem" (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "ref-elem" (i32.const 0x1)) (i32.const 0x1))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\93\80\80\80\00\04\60"
  "\00\00\60\01\64\00\00\60\01\64\70\00\60\01\64\6f"
  "\00\03\84\80\80\80\00\03\01\02\03\0a\a2\80\80\80"
  "\00\03\86\80\80\80\00\00\20\00\d1\1a\0b\86\80\80"
  "\80\00\00\20\00\d1\1a\0b\86\80\80\80\00\00\20\00"
  "\d1\1a\0b"
)
(module instance)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
    "\01\7f\00\03\82\80\80\80\00\01\00\0a\8b\80\80\80"
    "\00\01\85\80\80\80\00\00\20\00\d1\0b"
  )
  "type mismatch"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\89\80\80\80\00"
    "\01\83\80\80\80\00\00\d1\0b"
  )
  "type mismatch"
)
