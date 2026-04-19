(module definition binary
  "\00\61\73\6d\01\00\00\00\01\91\80\80\80\00\05\60"
  "\00\00\5f\00\5e\78\00\60\01\6f\00\60\01\7f\00\03"
  "\88\80\80\80\00\07\00\03\04\04\04\04\04\04\84\80"
  "\80\80\00\01\6e\00\0a\07\de\80\80\80\00\06\04\69"
  "\6e\69\74\00\01\11\72\65\66\5f\63\61\73\74\5f\6e"
  "\6f\6e\5f\6e\75\6c\6c\00\02\0d\72\65\66\5f\63\61"
  "\73\74\5f\6e\75\6c\6c\00\03\0c\72\65\66\5f\63\61"
  "\73\74\5f\69\33\31\00\04\0f\72\65\66\5f\63\61\73"
  "\74\5f\73\74\72\75\63\74\00\05\0e\72\65\66\5f\63"
  "\61\73\74\5f\61\72\72\61\79\00\06\09\85\80\80\80"
  "\00\01\03\00\01\00\0a\d0\81\80\80\00\07\82\80\80"
  "\80\00\00\0b\ba\80\80\80\00\00\41\00\d0\6e\26\00"
  "\41\01\41\07\fb\1c\26\00\41\02\fb\01\01\26\00\41"
  "\03\41\00\fb\07\02\26\00\41\04\20\00\fb\1a\26\00"
  "\41\05\d0\6c\26\00\41\06\d0\6b\26\00\41\07\d0\71"
  "\26\00\0b\90\80\80\80\00\00\20\00\25\00\d4\1a\20"
  "\00\25\00\fb\17\6e\1a\0b\aa\80\80\80\00\00\20\00"
  "\25\00\fb\17\6e\1a\20\00\25\00\fb\17\6b\1a\20\00"
  "\25\00\fb\17\6a\1a\20\00\25\00\fb\17\6c\1a\20\00"
  "\25\00\fb\17\71\1a\0b\92\80\80\80\00\00\20\00\25"
  "\00\fb\16\6c\1a\20\00\25\00\fb\17\6c\1a\0b\92\80"
  "\80\80\00\00\20\00\25\00\fb\16\6b\1a\20\00\25\00"
  "\fb\17\6b\1a\0b\92\80\80\80\00\00\20\00\25\00\fb"
  "\16\6a\1a\20\00\25\00\fb\17\6a\1a\0b"
)
(module instance)
(invoke "init" (ref.extern 0))
(assert_trap (invoke "ref_cast_non_null" (i32.const 0x0)) "null reference")
(assert_return (invoke "ref_cast_non_null" (i32.const 0x1)))
(assert_return (invoke "ref_cast_non_null" (i32.const 0x2)))
(assert_return (invoke "ref_cast_non_null" (i32.const 0x3)))
(assert_return (invoke "ref_cast_non_null" (i32.const 0x4)))
(assert_trap (invoke "ref_cast_non_null" (i32.const 0x5)) "null reference")
(assert_trap (invoke "ref_cast_non_null" (i32.const 0x6)) "null reference")
(assert_trap (invoke "ref_cast_non_null" (i32.const 0x7)) "null reference")
(assert_return (invoke "ref_cast_null" (i32.const 0x0)))
(assert_trap (invoke "ref_cast_null" (i32.const 0x1)) "cast failure")
(assert_trap (invoke "ref_cast_null" (i32.const 0x2)) "cast failure")
(assert_trap (invoke "ref_cast_null" (i32.const 0x3)) "cast failure")
(assert_trap (invoke "ref_cast_null" (i32.const 0x4)) "cast failure")
(assert_return (invoke "ref_cast_null" (i32.const 0x5)))
(assert_return (invoke "ref_cast_null" (i32.const 0x6)))
(assert_return (invoke "ref_cast_null" (i32.const 0x7)))
(assert_trap (invoke "ref_cast_i31" (i32.const 0x0)) "cast failure")
(assert_return (invoke "ref_cast_i31" (i32.const 0x1)))
(assert_trap (invoke "ref_cast_i31" (i32.const 0x2)) "cast failure")
(assert_trap (invoke "ref_cast_i31" (i32.const 0x3)) "cast failure")
(assert_trap (invoke "ref_cast_i31" (i32.const 0x4)) "cast failure")
(assert_trap (invoke "ref_cast_i31" (i32.const 0x5)) "cast failure")
(assert_trap (invoke "ref_cast_i31" (i32.const 0x6)) "cast failure")
(assert_trap (invoke "ref_cast_i31" (i32.const 0x7)) "cast failure")
(assert_trap (invoke "ref_cast_struct" (i32.const 0x0)) "cast failure")
(assert_trap (invoke "ref_cast_struct" (i32.const 0x1)) "cast failure")
(assert_return (invoke "ref_cast_struct" (i32.const 0x2)))
(assert_trap (invoke "ref_cast_struct" (i32.const 0x3)) "cast failure")
(assert_trap (invoke "ref_cast_struct" (i32.const 0x4)) "cast failure")
(assert_trap (invoke "ref_cast_struct" (i32.const 0x5)) "cast failure")
(assert_trap (invoke "ref_cast_struct" (i32.const 0x6)) "cast failure")
(assert_trap (invoke "ref_cast_struct" (i32.const 0x7)) "cast failure")
(assert_trap (invoke "ref_cast_array" (i32.const 0x0)) "cast failure")
(assert_trap (invoke "ref_cast_array" (i32.const 0x1)) "cast failure")
(assert_trap (invoke "ref_cast_array" (i32.const 0x2)) "cast failure")
(assert_return (invoke "ref_cast_array" (i32.const 0x3)))
(assert_trap (invoke "ref_cast_array" (i32.const 0x4)) "cast failure")
(assert_trap (invoke "ref_cast_array" (i32.const 0x5)) "cast failure")
(assert_trap (invoke "ref_cast_array" (i32.const 0x6)) "cast failure")
(assert_trap (invoke "ref_cast_array" (i32.const 0x7)) "cast failure")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\bf\80\80\80\00\09\50"
  "\00\5f\00\50\01\00\5f\01\7f\00\50\01\00\5f\01\7f"
  "\00\50\01\01\5f\02\7f\00\7f\00\50\01\02\5f\02\7f"
  "\00\7f\00\50\01\00\5f\02\7f\00\7f\00\50\01\00\5f"
  "\00\50\01\06\5f\02\7f\00\7f\00\60\00\00\03\84\80"
  "\80\80\00\03\08\08\08\04\84\80\80\80\00\01\6b\00"
  "\14\07\99\80\80\80\00\02\08\74\65\73\74\2d\73\75"
  "\62\00\01\0a\74\65\73\74\2d\63\61\6e\6f\6e\00\02"
  "\0a\fa\82\80\80\00\03\ba\80\80\80\00\00\41\00\fb"
  "\01\00\26\00\41\0a\fb\01\00\26\00\41\01\fb\01\01"
  "\26\00\41\0b\fb\01\02\26\00\41\02\fb\01\03\26\00"
  "\41\0c\fb\01\04\26\00\41\03\fb\01\05\26\00\41\04"
  "\fb\01\07\26\00\0b\bc\81\80\80\00\00\10\00\d0\6b"
  "\fb\17\00\1a\41\00\25\00\fb\17\00\1a\41\01\25\00"
  "\fb\17\00\1a\41\02\25\00\fb\17\00\1a\41\03\25\00"
  "\fb\17\00\1a\41\04\25\00\fb\17\00\1a\d0\6b\fb\17"
  "\00\1a\41\01\25\00\fb\17\01\1a\41\02\25\00\fb\17"
  "\01\1a\d0\6b\fb\17\00\1a\41\02\25\00\fb\17\03\1a"
  "\d0\6b\fb\17\00\1a\41\03\25\00\fb\17\05\1a\41\04"
  "\25\00\fb\17\07\1a\41\00\25\00\fb\16\00\1a\41\01"
  "\25\00\fb\16\00\1a\41\02\25\00\fb\16\00\1a\41\03"
  "\25\00\fb\16\00\1a\41\04\25\00\fb\16\00\1a\41\01"
  "\25\00\fb\16\01\1a\41\02\25\00\fb\16\01\1a\41\02"
  "\25\00\fb\16\03\1a\41\03\25\00\fb\16\05\1a\41\04"
  "\25\00\fb\16\07\1a\0b\f4\80\80\80\00\00\10\00\41"
  "\00\25\00\fb\16\00\1a\41\01\25\00\fb\16\00\1a\41"
  "\02\25\00\fb\16\00\1a\41\03\25\00\fb\16\00\1a\41"
  "\04\25\00\fb\16\00\1a\41\0a\25\00\fb\16\00\1a\41"
  "\0b\25\00\fb\16\00\1a\41\0c\25\00\fb\16\00\1a\41"
  "\01\25\00\fb\16\02\1a\41\02\25\00\fb\16\02\1a\41"
  "\0b\25\00\fb\16\01\1a\41\0c\25\00\fb\16\01\1a\41"
  "\02\25\00\fb\16\04\1a\41\0c\25\00\fb\16\03\1a\0b"
)
(module instance)
(invoke "test-sub")
(invoke "test-canon")
