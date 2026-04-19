(module definition binary
  "\00\61\73\6d\01\00\00\00\01\91\80\80\80\00\04\60"
  "\00\01\7f\60\00\00\60\01\7e\01\7e\60\00\01\7e\03"
  "\87\80\80\80\00\06\00\01\00\01\02\03\05\83\80\80"
  "\80\00\01\04\00\07\d7\80\80\80\00\06\0c\6c\6f\61"
  "\64\5f\61\74\5f\7a\65\72\6f\00\00\0d\73\74\6f\72"
  "\65\5f\61\74\5f\7a\65\72\6f\00\01\11\6c\6f\61\64"
  "\5f\61\74\5f\70\61\67\65\5f\73\69\7a\65\00\02\12"
  "\73\74\6f\72\65\5f\61\74\5f\70\61\67\65\5f\73\69"
  "\7a\65\00\03\04\67\72\6f\77\00\04\04\73\69\7a\65"
  "\00\05\0a\cd\80\80\80\00\06\87\80\80\80\00\00\42"
  "\00\28\02\00\0b\89\80\80\80\00\00\42\00\41\02\36"
  "\02\00\0b\89\80\80\80\00\00\42\80\80\04\28\02\00"
  "\0b\8b\80\80\80\00\00\42\80\80\04\41\03\36\02\00"
  "\0b\86\80\80\80\00\00\20\00\40\00\0b\84\80\80\80"
  "\00\00\3f\00\0b"
)
(module instance)
(assert_return (invoke "size") (i64.const 0x0))
(assert_trap (invoke "store_at_zero") "out of bounds memory access")
(assert_trap (invoke "load_at_zero") "out of bounds memory access")
(assert_trap (invoke "store_at_page_size") "out of bounds memory access")
(assert_trap (invoke "load_at_page_size") "out of bounds memory access")
(assert_return (invoke "grow" (i64.const 0x1)) (i64.const 0x0))
(assert_return (invoke "size") (i64.const 0x1))
(assert_return (invoke "load_at_zero") (i32.const 0x0))
(assert_return (invoke "store_at_zero"))
(assert_return (invoke "load_at_zero") (i32.const 0x2))
(assert_trap (invoke "store_at_page_size") "out of bounds memory access")
(assert_trap (invoke "load_at_page_size") "out of bounds memory access")
(assert_return (invoke "grow" (i64.const 0x4)) (i64.const 0x1))
(assert_return (invoke "size") (i64.const 0x5))
(assert_return (invoke "load_at_zero") (i32.const 0x2))
(assert_return (invoke "store_at_zero"))
(assert_return (invoke "load_at_zero") (i32.const 0x2))
(assert_return (invoke "load_at_page_size") (i32.const 0x0))
(assert_return (invoke "store_at_page_size"))
(assert_return (invoke "load_at_page_size") (i32.const 0x3))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7e\01\7e\03\82\80\80\80\00\01\00\05\83\80\80"
  "\80\00\01\04\00\07\88\80\80\80\00\01\04\67\72\6f"
  "\77\00\00\0a\8c\80\80\80\00\01\86\80\80\80\00\00"
  "\20\00\40\00\0b"
)
(module instance)
(assert_return (invoke "grow" (i64.const 0x0)) (i64.const 0x0))
(assert_return (invoke "grow" (i64.const 0x1)) (i64.const 0x0))
(assert_return (invoke "grow" (i64.const 0x0)) (i64.const 0x1))
(assert_return (invoke "grow" (i64.const 0x2)) (i64.const 0x1))
(assert_return (invoke "grow" (i64.const 0x320)) (i64.const 0x3))
(assert_return (invoke "grow" (i64.const 0x1)) (i64.const 0x323))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7e\01\7e\03\82\80\80\80\00\01\00\05\84\80\80"
  "\80\00\01\05\00\0a\07\88\80\80\80\00\01\04\67\72"
  "\6f\77\00\00\0a\8c\80\80\80\00\01\86\80\80\80\00"
  "\00\20\00\40\00\0b"
)
(module instance)
(assert_return (invoke "grow" (i64.const 0x0)) (i64.const 0x0))
(assert_return (invoke "grow" (i64.const 0x1)) (i64.const 0x0))
(assert_return (invoke "grow" (i64.const 0x1)) (i64.const 0x1))
(assert_return (invoke "grow" (i64.const 0x2)) (i64.const 0x2))
(assert_return (invoke "grow" (i64.const 0x6)) (i64.const 0x4))
(assert_return (invoke "grow" (i64.const 0x0)) (i64.const 0xa))
(assert_return
  (invoke "grow" (i64.const 0x1))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "grow" (i64.const 0x1_0000))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8c\80\80\80\00\02\60"
  "\01\7e\01\7e\60\02\7e\7e\01\7f\03\83\80\80\80\00"
  "\02\00\01\05\83\80\80\80\00\01\04\01\07\9c\80\80"
  "\80\00\02\04\67\72\6f\77\00\00\11\63\68\65\63\6b"
  "\2d\6d\65\6d\6f\72\79\2d\7a\65\72\6f\00\01\0a\c4"
  "\80\80\80\00\02\86\80\80\80\00\00\20\00\40\00\0b"
  "\b3\80\80\80\00\01\01\7f\41\01\21\02\02\40\03\40"
  "\20\00\2d\00\00\21\02\20\02\41\00\47\0d\01\20\00"
  "\20\01\5a\0d\01\20\00\42\01\7c\21\00\20\00\20\01"
  "\58\0d\00\0b\0b\20\02\0b"
)
(module instance)
(assert_return
  (invoke "check-memory-zero" (i64.const 0x0) (i64.const 0xffff))
  (i32.const 0x0)
)
(assert_return (invoke "grow" (i64.const 0x1)) (i64.const 0x1))
(assert_return
  (invoke "check-memory-zero" (i64.const 0x1_0000) (i64.const 0x1_ffff))
  (i32.const 0x0)
)
(assert_return (invoke "grow" (i64.const 0x1)) (i64.const 0x2))
(assert_return
  (invoke "check-memory-zero" (i64.const 0x2_0000) (i64.const 0x2_ffff))
  (i32.const 0x0)
)
(assert_return (invoke "grow" (i64.const 0x1)) (i64.const 0x3))
(assert_return
  (invoke "check-memory-zero" (i64.const 0x3_0000) (i64.const 0x3_ffff))
  (i32.const 0x0)
)
(assert_return (invoke "grow" (i64.const 0x1)) (i64.const 0x4))
(assert_return
  (invoke "check-memory-zero" (i64.const 0x4_0000) (i64.const 0x4_ffff))
  (i32.const 0x0)
)
(assert_return (invoke "grow" (i64.const 0x1)) (i64.const 0x5))
(assert_return
  (invoke "check-memory-zero" (i64.const 0x5_0000) (i64.const 0x5_ffff))
  (i32.const 0x0)
)
