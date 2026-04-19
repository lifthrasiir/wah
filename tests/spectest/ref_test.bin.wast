(module definition binary
  "\00\61\73\6d\01\00\00\00\01\92\80\80\80\00\05\60"
  "\00\00\5f\00\5e\78\00\60\01\6f\00\60\01\7f\01\7f"
  "\03\8d\80\80\80\00\0c\00\03\04\04\04\04\04\04\04"
  "\04\04\04\04\8a\80\80\80\00\03\6e\00\0a\70\00\0a"
  "\6f\00\0a\07\ba\81\80\80\00\0b\04\69\6e\69\74\00"
  "\01\12\72\65\66\5f\74\65\73\74\5f\6e\75\6c\6c\5f"
  "\64\61\74\61\00\02\0c\72\65\66\5f\74\65\73\74\5f"
  "\61\6e\79\00\03\0b\72\65\66\5f\74\65\73\74\5f\65"
  "\71\00\04\0c\72\65\66\5f\74\65\73\74\5f\69\33\31"
  "\00\05\0f\72\65\66\5f\74\65\73\74\5f\73\74\72\75"
  "\63\74\00\06\0e\72\65\66\5f\74\65\73\74\5f\61\72"
  "\72\61\79\00\07\12\72\65\66\5f\74\65\73\74\5f\6e"
  "\75\6c\6c\5f\66\75\6e\63\00\08\0d\72\65\66\5f\74"
  "\65\73\74\5f\66\75\6e\63\00\09\14\72\65\66\5f\74"
  "\65\73\74\5f\6e\75\6c\6c\5f\65\78\74\65\72\6e\00"
  "\0a\0f\72\65\66\5f\74\65\73\74\5f\65\78\74\65\72"
  "\6e\00\0b\09\85\80\80\80\00\01\03\00\01\00\0a\de"
  "\82\80\80\00\0c\82\80\80\80\00\00\0b\fb\80\80\80"
  "\00\00\41\00\d0\6e\26\00\41\01\d0\6b\26\00\41\02"
  "\d0\71\26\00\41\03\41\07\fb\1c\26\00\41\04\fb\01"
  "\01\26\00\41\05\41\00\fb\07\02\26\00\41\06\20\00"
  "\fb\1a\26\00\41\07\d0\6f\fb\1a\26\00\41\00\d0\73"
  "\26\01\41\01\d0\70\26\01\41\02\d2\00\26\01\41\00"
  "\d0\72\26\02\41\01\d0\6f\26\02\41\02\20\00\26\02"
  "\41\03\41\08\fb\1c\fb\1b\26\02\41\04\fb\01\01\fb"
  "\1b\26\02\41\05\d0\6e\fb\1b\26\02\0b\8f\80\80\80"
  "\00\00\20\00\25\00\d1\20\00\25\00\fb\15\71\6a\0b"
  "\91\80\80\80\00\00\20\00\25\00\fb\14\6e\20\00\25"
  "\00\fb\15\6e\6a\0b\91\80\80\80\00\00\20\00\25\00"
  "\fb\14\6d\20\00\25\00\fb\15\6d\6a\0b\91\80\80\80"
  "\00\00\20\00\25\00\fb\14\6c\20\00\25\00\fb\15\6c"
  "\6a\0b\91\80\80\80\00\00\20\00\25\00\fb\14\6b\20"
  "\00\25\00\fb\15\6b\6a\0b\91\80\80\80\00\00\20\00"
  "\25\00\fb\14\6a\20\00\25\00\fb\15\6a\6a\0b\8f\80"
  "\80\80\00\00\20\00\25\01\d1\20\00\25\01\fb\15\73"
  "\6a\0b\91\80\80\80\00\00\20\00\25\01\fb\14\70\20"
  "\00\25\01\fb\15\70\6a\0b\8f\80\80\80\00\00\20\00"
  "\25\02\d1\20\00\25\02\fb\15\72\6a\0b\91\80\80\80"
  "\00\00\20\00\25\02\fb\14\6f\20\00\25\02\fb\15\6f"
  "\6a\0b"
)
(module instance)
(invoke "init" (ref.extern 0))
(assert_return (invoke "ref_test_null_data" (i32.const 0x0)) (i32.const 0x2))
(assert_return (invoke "ref_test_null_data" (i32.const 0x1)) (i32.const 0x2))
(assert_return (invoke "ref_test_null_data" (i32.const 0x2)) (i32.const 0x2))
(assert_return (invoke "ref_test_null_data" (i32.const 0x3)) (i32.const 0x0))
(assert_return (invoke "ref_test_null_data" (i32.const 0x4)) (i32.const 0x0))
(assert_return (invoke "ref_test_null_data" (i32.const 0x5)) (i32.const 0x0))
(assert_return (invoke "ref_test_null_data" (i32.const 0x6)) (i32.const 0x0))
(assert_return (invoke "ref_test_null_data" (i32.const 0x7)) (i32.const 0x2))
(assert_return (invoke "ref_test_any" (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ref_test_any" (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "ref_test_any" (i32.const 0x2)) (i32.const 0x1))
(assert_return (invoke "ref_test_any" (i32.const 0x3)) (i32.const 0x2))
(assert_return (invoke "ref_test_any" (i32.const 0x4)) (i32.const 0x2))
(assert_return (invoke "ref_test_any" (i32.const 0x5)) (i32.const 0x2))
(assert_return (invoke "ref_test_any" (i32.const 0x6)) (i32.const 0x2))
(assert_return (invoke "ref_test_any" (i32.const 0x7)) (i32.const 0x1))
(assert_return (invoke "ref_test_eq" (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ref_test_eq" (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "ref_test_eq" (i32.const 0x2)) (i32.const 0x1))
(assert_return (invoke "ref_test_eq" (i32.const 0x3)) (i32.const 0x2))
(assert_return (invoke "ref_test_eq" (i32.const 0x4)) (i32.const 0x2))
(assert_return (invoke "ref_test_eq" (i32.const 0x5)) (i32.const 0x2))
(assert_return (invoke "ref_test_eq" (i32.const 0x6)) (i32.const 0x0))
(assert_return (invoke "ref_test_eq" (i32.const 0x7)) (i32.const 0x1))
(assert_return (invoke "ref_test_i31" (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ref_test_i31" (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "ref_test_i31" (i32.const 0x2)) (i32.const 0x1))
(assert_return (invoke "ref_test_i31" (i32.const 0x3)) (i32.const 0x2))
(assert_return (invoke "ref_test_i31" (i32.const 0x4)) (i32.const 0x0))
(assert_return (invoke "ref_test_i31" (i32.const 0x5)) (i32.const 0x0))
(assert_return (invoke "ref_test_i31" (i32.const 0x6)) (i32.const 0x0))
(assert_return (invoke "ref_test_i31" (i32.const 0x7)) (i32.const 0x1))
(assert_return (invoke "ref_test_struct" (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ref_test_struct" (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "ref_test_struct" (i32.const 0x2)) (i32.const 0x1))
(assert_return (invoke "ref_test_struct" (i32.const 0x3)) (i32.const 0x0))
(assert_return (invoke "ref_test_struct" (i32.const 0x4)) (i32.const 0x2))
(assert_return (invoke "ref_test_struct" (i32.const 0x5)) (i32.const 0x0))
(assert_return (invoke "ref_test_struct" (i32.const 0x6)) (i32.const 0x0))
(assert_return (invoke "ref_test_struct" (i32.const 0x7)) (i32.const 0x1))
(assert_return (invoke "ref_test_array" (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ref_test_array" (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "ref_test_array" (i32.const 0x2)) (i32.const 0x1))
(assert_return (invoke "ref_test_array" (i32.const 0x3)) (i32.const 0x0))
(assert_return (invoke "ref_test_array" (i32.const 0x4)) (i32.const 0x0))
(assert_return (invoke "ref_test_array" (i32.const 0x5)) (i32.const 0x2))
(assert_return (invoke "ref_test_array" (i32.const 0x6)) (i32.const 0x0))
(assert_return (invoke "ref_test_array" (i32.const 0x7)) (i32.const 0x1))
(assert_return (invoke "ref_test_null_func" (i32.const 0x0)) (i32.const 0x2))
(assert_return (invoke "ref_test_null_func" (i32.const 0x1)) (i32.const 0x2))
(assert_return (invoke "ref_test_null_func" (i32.const 0x2)) (i32.const 0x0))
(assert_return (invoke "ref_test_func" (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ref_test_func" (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "ref_test_func" (i32.const 0x2)) (i32.const 0x2))
(assert_return (invoke "ref_test_null_extern" (i32.const 0x0)) (i32.const 0x2))
(assert_return (invoke "ref_test_null_extern" (i32.const 0x1)) (i32.const 0x2))
(assert_return (invoke "ref_test_null_extern" (i32.const 0x2)) (i32.const 0x0))
(assert_return (invoke "ref_test_null_extern" (i32.const 0x3)) (i32.const 0x0))
(assert_return (invoke "ref_test_null_extern" (i32.const 0x4)) (i32.const 0x0))
(assert_return (invoke "ref_test_null_extern" (i32.const 0x5)) (i32.const 0x2))
(assert_return (invoke "ref_test_extern" (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke "ref_test_extern" (i32.const 0x1)) (i32.const 0x1))
(assert_return (invoke "ref_test_extern" (i32.const 0x2)) (i32.const 0x2))
(assert_return (invoke "ref_test_extern" (i32.const 0x3)) (i32.const 0x2))
(assert_return (invoke "ref_test_extern" (i32.const 0x4)) (i32.const 0x2))
(assert_return (invoke "ref_test_extern" (i32.const 0x5)) (i32.const 0x1))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\bf\80\80\80\00\09\50"
  "\00\5f\00\50\01\00\5f\01\7f\00\50\01\00\5f\01\7f"
  "\00\50\01\01\5f\02\7f\00\7f\00\50\01\02\5f\02\7f"
  "\00\7f\00\50\01\00\5f\02\7f\00\7f\00\50\01\00\5f"
  "\00\50\01\06\5f\02\7f\00\7f\00\60\00\00\03\84\80"
  "\80\80\00\03\08\08\08\04\84\80\80\80\00\01\6b\00"
  "\14\07\99\80\80\80\00\02\08\74\65\73\74\2d\73\75"
  "\62\00\01\0a\74\65\73\74\2d\63\61\6e\6f\6e\00\02"
  "\0a\ca\86\80\80\00\03\ba\80\80\80\00\00\41\00\fb"
  "\01\00\26\00\41\0a\fb\01\00\26\00\41\01\fb\01\01"
  "\26\00\41\0b\fb\01\02\26\00\41\02\fb\01\03\26\00"
  "\41\0c\fb\01\04\26\00\41\03\fb\01\05\26\00\41\04"
  "\fb\01\07\26\00\0b\eb\84\80\80\00\00\10\00\02\40"
  "\d0\6b\fb\15\00\45\0d\00\d0\00\fb\15\00\45\0d\00"
  "\d0\01\fb\15\00\45\0d\00\d0\03\fb\15\00\45\0d\00"
  "\d0\05\fb\15\00\45\0d\00\d0\07\fb\15\00\45\0d\00"
  "\41\00\25\00\fb\15\00\45\0d\00\41\01\25\00\fb\15"
  "\00\45\0d\00\41\02\25\00\fb\15\00\45\0d\00\41\03"
  "\25\00\fb\15\00\45\0d\00\41\04\25\00\fb\15\00\45"
  "\0d\00\d0\6b\fb\15\01\45\0d\00\d0\00\fb\15\01\45"
  "\0d\00\d0\01\fb\15\01\45\0d\00\d0\03\fb\15\01\45"
  "\0d\00\d0\05\fb\15\01\45\0d\00\d0\07\fb\15\01\45"
  "\0d\00\41\01\25\00\fb\15\01\45\0d\00\41\02\25\00"
  "\fb\15\01\45\0d\00\d0\6b\fb\15\03\45\0d\00\d0\00"
  "\fb\15\03\45\0d\00\d0\01\fb\15\03\45\0d\00\d0\03"
  "\fb\15\03\45\0d\00\d0\05\fb\15\03\45\0d\00\d0\07"
  "\fb\15\03\45\0d\00\41\02\25\00\fb\15\03\45\0d\00"
  "\d0\6b\fb\15\05\45\0d\00\d0\00\fb\15\05\45\0d\00"
  "\d0\01\fb\15\05\45\0d\00\d0\03\fb\15\05\45\0d\00"
  "\d0\05\fb\15\05\45\0d\00\d0\07\fb\15\05\45\0d\00"
  "\41\03\25\00\fb\15\05\45\0d\00\d0\6b\fb\15\07\45"
  "\0d\00\d0\00\fb\15\07\45\0d\00\d0\01\fb\15\07\45"
  "\0d\00\d0\03\fb\15\07\45\0d\00\d0\05\fb\15\07\45"
  "\0d\00\d0\07\fb\15\07\45\0d\00\41\04\25\00\fb\15"
  "\07\45\0d\00\41\00\25\00\fb\14\00\45\0d\00\41\01"
  "\25\00\fb\14\00\45\0d\00\41\02\25\00\fb\14\00\45"
  "\0d\00\41\03\25\00\fb\14\00\45\0d\00\41\04\25\00"
  "\fb\14\00\45\0d\00\41\01\25\00\fb\14\01\45\0d\00"
  "\41\02\25\00\fb\14\01\45\0d\00\41\02\25\00\fb\14"
  "\03\45\0d\00\41\03\25\00\fb\14\05\45\0d\00\41\04"
  "\25\00\fb\14\07\45\0d\00\d0\6b\fb\14\00\0d\00\d0"
  "\6b\fb\14\01\0d\00\d0\6b\fb\14\03\0d\00\d0\6b\fb"
  "\14\05\0d\00\d0\6b\fb\14\07\0d\00\41\00\25\00\fb"
  "\14\01\0d\00\41\03\25\00\fb\14\01\0d\00\41\04\25"
  "\00\fb\14\01\0d\00\41\00\25\00\fb\14\03\0d\00\41"
  "\01\25\00\fb\14\03\0d\00\41\03\25\00\fb\14\03\0d"
  "\00\41\04\25\00\fb\14\03\0d\00\41\00\25\00\fb\14"
  "\05\0d\00\41\01\25\00\fb\14\05\0d\00\41\02\25\00"
  "\fb\14\05\0d\00\41\04\25\00\fb\14\05\0d\00\41\00"
  "\25\00\fb\14\07\0d\00\41\01\25\00\fb\14\07\0d\00"
  "\41\02\25\00\fb\14\07\0d\00\41\03\25\00\fb\14\07"
  "\0d\00\0f\0b\00\0b\95\81\80\80\00\00\10\00\02\40"
  "\41\00\25\00\fb\14\00\45\0d\00\41\01\25\00\fb\14"
  "\00\45\0d\00\41\02\25\00\fb\14\00\45\0d\00\41\03"
  "\25\00\fb\14\00\45\0d\00\41\04\25\00\fb\14\00\45"
  "\0d\00\41\0a\25\00\fb\14\00\45\0d\00\41\0b\25\00"
  "\fb\14\00\45\0d\00\41\0c\25\00\fb\14\00\45\0d\00"
  "\41\01\25\00\fb\14\02\45\0d\00\41\02\25\00\fb\14"
  "\02\45\0d\00\41\0b\25\00\fb\14\01\45\0d\00\41\0c"
  "\25\00\fb\14\01\45\0d\00\41\02\25\00\fb\14\04\45"
  "\0d\00\41\0c\25\00\fb\14\03\45\0d\00\0f\0b\00\0b"
)
(module instance)
(assert_return (invoke "test-sub"))
(assert_return (invoke "test-canon"))
