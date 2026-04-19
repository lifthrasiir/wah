(module definition binary
  "\00\61\73\6d\01\00\00\00\01\99\80\80\80\00\05\60"
  "\01\7f\01\64\6c\60\01\7f\01\7f\60\00\01\7f\60\00"
  "\02\7f\7f\60\01\7f\00\03\88\80\80\80\00\07\00\01"
  "\01\02\02\03\04\06\91\80\80\80\00\02\64\6c\00\41"
  "\02\fb\1c\0b\64\6c\01\41\03\fb\1c\0b\07\cc\80\80"
  "\80\00\07\03\6e\65\77\00\00\05\67\65\74\5f\75\00"
  "\01\05\67\65\74\5f\73\00\02\0a\67\65\74\5f\75\2d"
  "\6e\75\6c\6c\00\03\0a\67\65\74\5f\73\2d\6e\75\6c"
  "\6c\00\04\0b\67\65\74\5f\67\6c\6f\62\61\6c\73\00"
  "\05\0a\73\65\74\5f\67\6c\6f\62\61\6c\00\06\0a\d8"
  "\80\80\80\00\07\86\80\80\80\00\00\20\00\fb\1c\0b"
  "\88\80\80\80\00\00\20\00\fb\1c\fb\1e\0b\88\80\80"
  "\80\00\00\20\00\fb\1c\fb\1d\0b\86\80\80\80\00\00"
  "\d0\6c\fb\1e\0b\86\80\80\80\00\00\d0\6c\fb\1d\0b"
  "\8a\80\80\80\00\00\23\00\fb\1e\23\01\fb\1e\0b\88"
  "\80\80\80\00\00\20\00\fb\1c\24\01\0b"
)
(module instance)
(assert_return (invoke "new" (i32.const 0x1)) (ref.i31))
(assert_return (invoke "get_u" (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "get_u" (i32.const 0x64)) (i32.const 0x64))
(assert_return (invoke "get_u" (i32.const 0xffff_ffff)) (i32.const 0x7fff_ffff))
(assert_return (invoke "get_u" (i32.const 0x3fff_ffff)) (i32.const 0x3fff_ffff))
(assert_return (invoke "get_u" (i32.const 0x4000_0000)) (i32.const 0x4000_0000))
(assert_return (invoke "get_u" (i32.const 0x7fff_ffff)) (i32.const 0x7fff_ffff))
(assert_return (invoke "get_u" (i32.const 0xaaaa_aaaa)) (i32.const 0x2aaa_aaaa))
(assert_return (invoke "get_u" (i32.const 0xcaaa_aaaa)) (i32.const 0x4aaa_aaaa))
(assert_return (invoke "get_s" (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "get_s" (i32.const 0x64)) (i32.const 0x64))
(assert_return (invoke "get_s" (i32.const 0xffff_ffff)) (i32.const 0xffff_ffff))
(assert_return (invoke "get_s" (i32.const 0x3fff_ffff)) (i32.const 0x3fff_ffff))
(assert_return (invoke "get_s" (i32.const 0x4000_0000)) (i32.const 0xc000_0000))
(assert_return (invoke "get_s" (i32.const 0x7fff_ffff)) (i32.const 0xffff_ffff))
(assert_return (invoke "get_s" (i32.const 0xaaaa_aaaa)) (i32.const 0x2aaa_aaaa))
(assert_return (invoke "get_s" (i32.const 0xcaaa_aaaa)) (i32.const 0xcaaa_aaaa))
(assert_trap (invoke "get_u-null") "null i31 reference")
(assert_trap (invoke "get_s-null") "null i31 reference")
(assert_return (invoke "get_globals") (i32.const 0x2) (i32.const 0x3))
(invoke "set_global" (i32.const 0x4d2))
(assert_return (invoke "get_globals") (i32.const 0x2) (i32.const 0x4d2))
(module definition $tables_of_i31ref binary
  "\00\61\73\6d\01\00\00\00\01\96\80\80\80\00\04\60"
  "\00\01\7f\60\01\7f\01\7f\60\02\7f\7f\01\7f\60\03"
  "\7f\7f\7f\00\03\87\80\80\80\00\06\00\01\02\03\03"
  "\03\04\85\80\80\80\00\01\6c\01\03\0a\07\aa\80\80"
  "\80\00\06\04\73\69\7a\65\00\00\03\67\65\74\00\01"
  "\04\67\72\6f\77\00\02\04\66\69\6c\6c\00\03\04\63"
  "\6f\70\79\00\04\04\69\6e\69\74\00\05\09\af\80\80"
  "\80\00\02\06\00\41\00\0b\6c\03\41\e7\07\fb\1c\0b"
  "\41\f8\06\fb\1c\0b\41\89\06\fb\1c\0b\05\6c\03\41"
  "\fb\00\fb\1c\0b\41\c8\03\fb\1c\0b\41\95\06\fb\1c"
  "\0b\0a\dc\80\80\80\00\06\85\80\80\80\00\00\fc\10"
  "\00\0b\88\80\80\80\00\00\20\00\25\00\fb\1e\0b\8b"
  "\80\80\80\00\00\20\01\fb\1c\20\00\fc\0f\00\0b\8d"
  "\80\80\80\00\00\20\00\20\01\fb\1c\20\02\fc\11\00"
  "\0b\8c\80\80\80\00\00\20\00\20\01\20\02\fc\0e\00"
  "\00\0b\8c\80\80\80\00\00\20\00\20\01\20\02\fc\0c"
  "\01\00\0b"
)
(module instance $tables_of_i31ref $tables_of_i31ref)
(assert_return (invoke "size") (i32.const 0x3))
(assert_return (invoke "get" (i32.const 0x0)) (i32.const 0x3e7))
(assert_return (invoke "get" (i32.const 0x1)) (i32.const 0x378))
(assert_return (invoke "get" (i32.const 0x2)) (i32.const 0x309))
(assert_return
  (invoke "grow" (i32.const 0x2) (i32.const 0x14d))
  (i32.const 0x3)
)
(assert_return (invoke "size") (i32.const 0x5))
(assert_return (invoke "get" (i32.const 0x3)) (i32.const 0x14d))
(assert_return (invoke "get" (i32.const 0x4)) (i32.const 0x14d))
(invoke "fill" (i32.const 0x2) (i32.const 0x6f) (i32.const 0x2))
(assert_return (invoke "get" (i32.const 0x2)) (i32.const 0x6f))
(assert_return (invoke "get" (i32.const 0x3)) (i32.const 0x6f))
(invoke "copy" (i32.const 0x3) (i32.const 0x0) (i32.const 0x2))
(assert_return (invoke "get" (i32.const 0x3)) (i32.const 0x3e7))
(assert_return (invoke "get" (i32.const 0x4)) (i32.const 0x378))
(invoke "init" (i32.const 0x1) (i32.const 0x0) (i32.const 0x3))
(assert_return (invoke "get" (i32.const 0x1)) (i32.const 0x7b))
(assert_return (invoke "get" (i32.const 0x2)) (i32.const 0x1c8))
(assert_return (invoke "get" (i32.const 0x3)) (i32.const 0x315))
(module definition $env binary
  "\00\61\73\6d\01\00\00\00\06\86\80\80\80\00\01\7f"
  "\00\41\2a\0b\07\85\80\80\80\00\01\01\67\03\00"
)
(module instance $env $env)
(register "env")
(module definition $i31ref_of_global_table_initializer binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7f\01\7f\02\8a\80\80\80\00\01\03\65\6e\76\01"
  "\67\03\7f\00\03\82\80\80\80\00\01\00\04\8d\80\80"
  "\80\00\01\40\00\64\6c\01\03\03\23\00\fb\1c\0b\07"
  "\87\80\80\80\00\01\03\67\65\74\00\00\0a\8e\80\80"
  "\80\00\01\88\80\80\80\00\00\20\00\25\00\fb\1e\0b"
)
(module instance $i31ref_of_global_table_initializer $i31ref_of_global_table_initializer
)
(assert_return (invoke "get" (i32.const 0x0)) (i32.const 0x2a))
(assert_return (invoke "get" (i32.const 0x1)) (i32.const 0x2a))
(assert_return (invoke "get" (i32.const 0x2)) (i32.const 0x2a))
(module definition $i31ref_of_global_global_initializer binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\00\01\7f\02\8a\80\80\80\00\01\03\65\6e\76\01\67"
  "\03\7f\00\03\82\80\80\80\00\01\00\06\88\80\80\80"
  "\00\01\6c\00\23\00\fb\1c\0b\07\87\80\80\80\00\01"
  "\03\67\65\74\00\00\0a\8c\80\80\80\00\01\86\80\80"
  "\80\00\00\23\01\fb\1e\0b"
)
(module instance $i31ref_of_global_global_initializer $i31ref_of_global_global_initializer
)
(assert_return (invoke "get") (i32.const 0x2a))
(module definition $anyref_global_of_i31ref binary
  "\00\61\73\6d\01\00\00\00\01\8a\80\80\80\00\02\60"
  "\00\02\7f\7f\60\01\7f\00\03\83\80\80\80\00\02\00"
  "\01\06\91\80\80\80\00\02\6e\00\41\d2\09\fb\1c\0b"
  "\6e\01\41\ae\2c\fb\1c\0b\07\9c\80\80\80\00\02\0b"
  "\67\65\74\5f\67\6c\6f\62\61\6c\73\00\00\0a\73\65"
  "\74\5f\67\6c\6f\62\61\6c\00\01\0a\a3\80\80\80\00"
  "\02\90\80\80\80\00\00\23\00\fb\17\6c\fb\1e\23\01"
  "\fb\17\6c\fb\1e\0b\88\80\80\80\00\00\20\00\fb\1c"
  "\24\01\0b"
)
(module instance $anyref_global_of_i31ref $anyref_global_of_i31ref)
(assert_return (invoke "get_globals") (i32.const 0x4d2) (i32.const 0x162e))
(invoke "set_global" (i32.const 0x0))
(assert_return (invoke "get_globals") (i32.const 0x4d2) (i32.const 0x0))
(module definition $anyref_table_of_i31ref binary
  "\00\61\73\6d\01\00\00\00\01\96\80\80\80\00\04\60"
  "\00\01\7f\60\01\7f\01\7f\60\02\7f\7f\01\7f\60\03"
  "\7f\7f\7f\00\03\87\80\80\80\00\06\00\01\02\03\03"
  "\03\04\85\80\80\80\00\01\6e\01\03\0a\07\aa\80\80"
  "\80\00\06\04\73\69\7a\65\00\00\03\67\65\74\00\01"
  "\04\67\72\6f\77\00\02\04\66\69\6c\6c\00\03\04\63"
  "\6f\70\79\00\04\04\69\6e\69\74\00\05\09\af\80\80"
  "\80\00\02\06\00\41\00\0b\6c\03\41\e7\07\fb\1c\0b"
  "\41\f8\06\fb\1c\0b\41\89\06\fb\1c\0b\05\6c\03\41"
  "\fb\00\fb\1c\0b\41\c8\03\fb\1c\0b\41\95\06\fb\1c"
  "\0b\0a\df\80\80\80\00\06\85\80\80\80\00\00\fc\10"
  "\00\0b\8b\80\80\80\00\00\20\00\25\00\fb\17\6c\fb"
  "\1e\0b\8b\80\80\80\00\00\20\01\fb\1c\20\00\fc\0f"
  "\00\0b\8d\80\80\80\00\00\20\00\20\01\fb\1c\20\02"
  "\fc\11\00\0b\8c\80\80\80\00\00\20\00\20\01\20\02"
  "\fc\0e\00\00\0b\8c\80\80\80\00\00\20\00\20\01\20"
  "\02\fc\0c\01\00\0b"
)
(module instance $anyref_table_of_i31ref $anyref_table_of_i31ref)
(assert_return (invoke "size") (i32.const 0x3))
(assert_return (invoke "get" (i32.const 0x0)) (i32.const 0x3e7))
(assert_return (invoke "get" (i32.const 0x1)) (i32.const 0x378))
(assert_return (invoke "get" (i32.const 0x2)) (i32.const 0x309))
(assert_return
  (invoke "grow" (i32.const 0x2) (i32.const 0x14d))
  (i32.const 0x3)
)
(assert_return (invoke "size") (i32.const 0x5))
(assert_return (invoke "get" (i32.const 0x3)) (i32.const 0x14d))
(assert_return (invoke "get" (i32.const 0x4)) (i32.const 0x14d))
(invoke "fill" (i32.const 0x2) (i32.const 0x6f) (i32.const 0x2))
(assert_return (invoke "get" (i32.const 0x2)) (i32.const 0x6f))
(assert_return (invoke "get" (i32.const 0x3)) (i32.const 0x6f))
(invoke "copy" (i32.const 0x3) (i32.const 0x0) (i32.const 0x2))
(assert_return (invoke "get" (i32.const 0x3)) (i32.const 0x3e7))
(assert_return (invoke "get" (i32.const 0x4)) (i32.const 0x378))
(invoke "init" (i32.const 0x1) (i32.const 0x0) (i32.const 0x3))
(assert_return (invoke "get" (i32.const 0x1)) (i32.const 0x7b))
(assert_return (invoke "get" (i32.const 0x2)) (i32.const 0x1c8))
(assert_return (invoke "get" (i32.const 0x3)) (i32.const 0x315))
