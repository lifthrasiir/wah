(module definition $Mf binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\00\01\7f\03\83\80\80\80\00\02\00\00\07\88\80\80"
  "\80\00\01\04\63\61\6c\6c\00\00\0a\93\80\80\80\00"
  "\02\84\80\80\80\00\00\10\01\0b\84\80\80\80\00\00"
  "\41\02\0b"
)
(module instance $Mf $Mf)
(register "Mf" $Mf)
(module definition $Nf binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\00\01\7f\02\8b\80\80\80\00\01\02\4d\66\04\63\61"
  "\6c\6c\00\00\03\84\80\80\80\00\03\00\00\00\07\a1"
  "\80\80\80\00\03\07\4d\66\2e\63\61\6c\6c\00\00\0c"
  "\63\61\6c\6c\20\4d\66\2e\63\61\6c\6c\00\01\04\63"
  "\61\6c\6c\00\02\0a\9c\80\80\80\00\03\84\80\80\80"
  "\00\00\10\00\0b\84\80\80\80\00\00\10\03\0b\84\80"
  "\80\80\00\00\41\03\0b"
)
(module instance $Nf $Nf)
(assert_return (invoke $Mf "call") (i32.const 0x2))
(assert_return (invoke $Nf "Mf.call") (i32.const 0x2))
(assert_return (invoke $Nf "call") (i32.const 0x3))
(assert_return (invoke $Nf "call Mf.call") (i32.const 0x2))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\01\7f\00\02\96\80\80\80\00\01\08\73\70\65\63\74"
  "\65\73\74\09\70\72\69\6e\74\5f\69\33\32\00\00\07"
  "\89\80\80\80\00\01\05\70\72\69\6e\74\00\00"
)
(module instance)
(register "reexport_f")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\01\7e\00\02\94\80\80\80\00\01\0a\72\65\65\78\70"
  "\6f\72\74\5f\66\05\70\72\69\6e\74\00\00"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7f\01\7f\02\94\80\80\80\00\01\0a\72\65\65\78"
  "\70\6f\72\74\5f\66\05\70\72\69\6e\74\00\00"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition $Mg binary
  "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\60"
  "\00\01\7f\60\01\7f\00\03\84\80\80\80\00\03\00\00"
  "\01\06\8c\80\80\80\00\02\7f\00\41\2a\0b\7f\01\41"
  "\8e\01\0b\07\ad\80\80\80\00\05\04\67\6c\6f\62\03"
  "\00\03\67\65\74\00\00\08\6d\75\74\5f\67\6c\6f\62"
  "\03\01\07\67\65\74\5f\6d\75\74\00\01\07\73\65\74"
  "\5f\6d\75\74\00\02\0a\9e\80\80\80\00\03\84\80\80"
  "\80\00\00\23\00\0b\84\80\80\80\00\00\23\01\0b\86"
  "\80\80\80\00\00\20\00\24\01\0b"
)
(module instance $Mg $Mg)
(register "Mg" $Mg)
(module definition $Ng binary
  "\00\61\73\6d\01\00\00\00\01\89\80\80\80\00\02\60"
  "\00\01\7f\60\01\7f\00\02\be\80\80\80\00\05\02\4d"
  "\67\04\67\6c\6f\62\03\7f\00\02\4d\67\08\6d\75\74"
  "\5f\67\6c\6f\62\03\7f\01\02\4d\67\03\67\65\74\00"
  "\00\02\4d\67\07\67\65\74\5f\6d\75\74\00\00\02\4d"
  "\67\07\73\65\74\5f\6d\75\74\00\01\03\82\80\80\80"
  "\00\01\00\06\86\80\80\80\00\01\7f\00\41\2b\0b\07"
  "\c9\80\80\80\00\07\07\4d\67\2e\67\6c\6f\62\03\00"
  "\06\4d\67\2e\67\65\74\00\00\04\67\6c\6f\62\03\02"
  "\03\67\65\74\00\03\0b\4d\67\2e\6d\75\74\5f\67\6c"
  "\6f\62\03\01\0a\4d\67\2e\67\65\74\5f\6d\75\74\00"
  "\01\0a\4d\67\2e\73\65\74\5f\6d\75\74\00\02\0a\8a"
  "\80\80\80\00\01\84\80\80\80\00\00\23\02\0b"
)
(module instance $Ng $Ng)
(assert_return (get $Mg "glob") (i32.const 0x2a))
(assert_return (get $Ng "Mg.glob") (i32.const 0x2a))
(assert_return (get $Ng "glob") (i32.const 0x2b))
(assert_return (invoke $Mg "get") (i32.const 0x2a))
(assert_return (invoke $Ng "Mg.get") (i32.const 0x2a))
(assert_return (invoke $Ng "get") (i32.const 0x2b))
(assert_return (get $Mg "mut_glob") (i32.const 0x8e))
(assert_return (get $Ng "Mg.mut_glob") (i32.const 0x8e))
(assert_return (invoke $Mg "get_mut") (i32.const 0x8e))
(assert_return (invoke $Ng "Mg.get_mut") (i32.const 0x8e))
(assert_return (invoke $Mg "set_mut" (i32.const 0xf1)))
(assert_return (get $Mg "mut_glob") (i32.const 0xf1))
(assert_return (get $Ng "Mg.mut_glob") (i32.const 0xf1))
(assert_return (invoke $Mg "get_mut") (i32.const 0xf1))
(assert_return (invoke $Ng "Mg.get_mut") (i32.const 0xf1))
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\90\80\80\80\00\01\02"
  "\4d\67\08\6d\75\74\5f\67\6c\6f\62\03\7f\00"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\8c\80\80\80\00\01\02"
  "\4d\67\04\67\6c\6f\62\03\7f\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition $Mref_ex binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\03\82\80\80\80\00\01\00\06\b9\80\80\80\00"
  "\0a\70\00\d0\70\0b\64\70\00\d2\00\0b\63\00\00\d0"
  "\00\0b\64\00\00\d2\00\0b\6f\00\d0\6f\0b\70\01\d0"
  "\70\0b\64\70\01\d2\00\0b\63\00\01\d0\00\0b\64\00"
  "\01\d2\00\0b\6f\01\d0\6f\0b\07\9d\81\80\80\00\0a"
  "\10\67\2d\63\6f\6e\73\74\2d\66\75\6e\63\6e\75\6c"
  "\6c\03\00\0c\67\2d\63\6f\6e\73\74\2d\66\75\6e\63"
  "\03\01\0f\67\2d\63\6f\6e\73\74\2d\72\65\66\6e\75"
  "\6c\6c\03\02\0b\67\2d\63\6f\6e\73\74\2d\72\65\66"
  "\03\03\0e\67\2d\63\6f\6e\73\74\2d\65\78\74\65\72"
  "\6e\03\04\0e\67\2d\76\61\72\2d\66\75\6e\63\6e\75"
  "\6c\6c\03\05\0a\67\2d\76\61\72\2d\66\75\6e\63\03"
  "\06\0d\67\2d\76\61\72\2d\72\65\66\6e\75\6c\6c\03"
  "\07\09\67\2d\76\61\72\2d\72\65\66\03\08\0c\67\2d"
  "\76\61\72\2d\65\78\74\65\72\6e\03\09\09\85\80\80"
  "\80\00\01\03\00\01\00\0a\88\80\80\80\00\01\82\80"
  "\80\80\00\00\0b"
)
(module instance $Mref_ex $Mref_ex)
(register "Mref_ex" $Mref_ex)
(module definition $Mref_im binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\f7\82\80\80\00\0f\07\4d\72\65\66\5f\65"
  "\78\10\67\2d\63\6f\6e\73\74\2d\66\75\6e\63\6e\75"
  "\6c\6c\03\70\00\07\4d\72\65\66\5f\65\78\0c\67\2d"
  "\63\6f\6e\73\74\2d\66\75\6e\63\03\70\00\07\4d\72"
  "\65\66\5f\65\78\0f\67\2d\63\6f\6e\73\74\2d\72\65"
  "\66\6e\75\6c\6c\03\70\00\07\4d\72\65\66\5f\65\78"
  "\0b\67\2d\63\6f\6e\73\74\2d\72\65\66\03\70\00\07"
  "\4d\72\65\66\5f\65\78\0c\67\2d\63\6f\6e\73\74\2d"
  "\66\75\6e\63\03\64\70\00\07\4d\72\65\66\5f\65\78"
  "\0b\67\2d\63\6f\6e\73\74\2d\72\65\66\03\64\70\00"
  "\07\4d\72\65\66\5f\65\78\0f\67\2d\63\6f\6e\73\74"
  "\2d\72\65\66\6e\75\6c\6c\03\63\00\00\07\4d\72\65"
  "\66\5f\65\78\0b\67\2d\63\6f\6e\73\74\2d\72\65\66"
  "\03\63\00\00\07\4d\72\65\66\5f\65\78\0b\67\2d\63"
  "\6f\6e\73\74\2d\72\65\66\03\64\00\00\07\4d\72\65"
  "\66\5f\65\78\0e\67\2d\63\6f\6e\73\74\2d\65\78\74"
  "\65\72\6e\03\6f\00\07\4d\72\65\66\5f\65\78\0e\67"
  "\2d\76\61\72\2d\66\75\6e\63\6e\75\6c\6c\03\70\01"
  "\07\4d\72\65\66\5f\65\78\0a\67\2d\76\61\72\2d\66"
  "\75\6e\63\03\64\70\01\07\4d\72\65\66\5f\65\78\0d"
  "\67\2d\76\61\72\2d\72\65\66\6e\75\6c\6c\03\63\00"
  "\01\07\4d\72\65\66\5f\65\78\09\67\2d\76\61\72\2d"
  "\72\65\66\03\64\00\01\07\4d\72\65\66\5f\65\78\0c"
  "\67\2d\76\61\72\2d\65\78\74\65\72\6e\03\6f\01"
)
(module instance $Mref_im $Mref_im)
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\9b\80\80\80\00\01\07"
  "\4d\72\65\66\5f\65\78\0e\67\2d\63\6f\6e\73\74\2d"
  "\65\78\74\65\72\6e\03\70\00"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\9e\80\80\80\00\01\07\4d\72\65\66\5f\65"
  "\78\10\67\2d\63\6f\6e\73\74\2d\66\75\6e\63\6e\75"
  "\6c\6c\03\64\70\00"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\9d\80\80\80\00\01\07\4d\72\65\66\5f\65"
  "\78\0f\67\2d\63\6f\6e\73\74\2d\72\65\66\6e\75\6c"
  "\6c\03\64\70\00"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\9c\80\80\80\00\01\07\4d\72\65\66\5f\65"
  "\78\0e\67\2d\63\6f\6e\73\74\2d\65\78\74\65\72\6e"
  "\03\64\70\00"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\9e\80\80\80\00\01\07\4d\72\65\66\5f\65"
  "\78\10\67\2d\63\6f\6e\73\74\2d\66\75\6e\63\6e\75"
  "\6c\6c\03\63\00\00"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\9a\80\80\80\00\01\07\4d\72\65\66\5f\65"
  "\78\0c\67\2d\63\6f\6e\73\74\2d\66\75\6e\63\03\63"
  "\00\00"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\9c\80\80\80\00\01\07\4d\72\65\66\5f\65"
  "\78\0e\67\2d\63\6f\6e\73\74\2d\65\78\74\65\72\6e"
  "\03\63\00\00"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\9e\80\80\80\00\01\07\4d\72\65\66\5f\65"
  "\78\10\67\2d\63\6f\6e\73\74\2d\66\75\6e\63\6e\75"
  "\6c\6c\03\64\00\00"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\9a\80\80\80\00\01\07\4d\72\65\66\5f\65"
  "\78\0c\67\2d\63\6f\6e\73\74\2d\66\75\6e\63\03\64"
  "\00\00"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\9d\80\80\80\00\01\07\4d\72\65\66\5f\65"
  "\78\0f\67\2d\63\6f\6e\73\74\2d\72\65\66\6e\75\6c"
  "\6c\03\64\00\00"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\9c\80\80\80\00\01\07\4d\72\65\66\5f\65"
  "\78\0e\67\2d\63\6f\6e\73\74\2d\65\78\74\65\72\6e"
  "\03\64\00\00"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\97\80\80\80\00\01\07"
  "\4d\72\65\66\5f\65\78\0a\67\2d\76\61\72\2d\66\75"
  "\6e\63\03\70\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\9a\80\80\80\00\01\07"
  "\4d\72\65\66\5f\65\78\0d\67\2d\76\61\72\2d\72\65"
  "\66\6e\75\6c\6c\03\70\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\96\80\80\80\00\01\07"
  "\4d\72\65\66\5f\65\78\09\67\2d\76\61\72\2d\72\65"
  "\66\03\70\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\99\80\80\80\00\01\07"
  "\4d\72\65\66\5f\65\78\0c\67\2d\76\61\72\2d\65\78"
  "\74\65\72\6e\03\70\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\9c\80\80\80\00\01\07"
  "\4d\72\65\66\5f\65\78\0e\67\2d\76\61\72\2d\66\75"
  "\6e\63\6e\75\6c\6c\03\64\70\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\9b\80\80\80\00\01\07"
  "\4d\72\65\66\5f\65\78\0d\67\2d\76\61\72\2d\72\65"
  "\66\6e\75\6c\6c\03\64\70\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\97\80\80\80\00\01\07"
  "\4d\72\65\66\5f\65\78\09\67\2d\76\61\72\2d\72\65"
  "\66\03\64\70\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\9a\80\80\80\00\01\07"
  "\4d\72\65\66\5f\65\78\0c\67\2d\76\61\72\2d\65\78"
  "\74\65\72\6e\03\64\70\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\9c\80\80\80\00\01\07\4d\72\65\66\5f\65"
  "\78\0e\67\2d\76\61\72\2d\66\75\6e\63\6e\75\6c\6c"
  "\03\63\00\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\98\80\80\80\00\01\07\4d\72\65\66\5f\65"
  "\78\0a\67\2d\76\61\72\2d\66\75\6e\63\03\63\00\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\97\80\80\80\00\01\07\4d\72\65\66\5f\65"
  "\78\09\67\2d\76\61\72\2d\72\65\66\03\63\00\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\9a\80\80\80\00\01\07\4d\72\65\66\5f\65"
  "\78\0c\67\2d\76\61\72\2d\65\78\74\65\72\6e\03\63"
  "\00\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\9c\80\80\80\00\01\07\4d\72\65\66\5f\65"
  "\78\0e\67\2d\76\61\72\2d\66\75\6e\63\6e\75\6c\6c"
  "\03\64\00\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\98\80\80\80\00\01\07\4d\72\65\66\5f\65"
  "\78\0a\67\2d\76\61\72\2d\66\75\6e\63\03\64\00\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\9b\80\80\80\00\01\07\4d\72\65\66\5f\65"
  "\78\0d\67\2d\76\61\72\2d\72\65\66\6e\75\6c\6c\03"
  "\64\00\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\9a\80\80\80\00\01\07\4d\72\65\66\5f\65"
  "\78\0c\67\2d\76\61\72\2d\65\78\74\65\72\6e\03\64"
  "\00\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\9b\80\80\80\00\01\07"
  "\4d\72\65\66\5f\65\78\0e\67\2d\76\61\72\2d\66\75"
  "\6e\63\6e\75\6c\6c\03\6f\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\97\80\80\80\00\01\07"
  "\4d\72\65\66\5f\65\78\0a\67\2d\76\61\72\2d\66\75"
  "\6e\63\03\6f\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\9a\80\80\80\00\01\07"
  "\4d\72\65\66\5f\65\78\0d\67\2d\76\61\72\2d\72\65"
  "\66\6e\75\6c\6c\03\6f\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\96\80\80\80\00\01\07"
  "\4d\72\65\66\5f\65\78\09\67\2d\76\61\72\2d\72\65"
  "\66\03\6f\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition $Mt binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\01\7f\60\00\00\60\01\7f\01\7f\03\84\80\80\80"
  "\00\03\00\00\02\04\84\80\80\80\00\01\70\00\0a\07"
  "\92\80\80\80\00\03\03\74\61\62\01\00\01\68\00\01"
  "\04\63\61\6c\6c\00\02\09\8a\80\80\80\00\01\00\41"
  "\02\0b\04\00\00\00\00\0a\9f\80\80\80\00\03\84\80"
  "\80\80\00\00\41\04\0b\84\80\80\80\00\00\41\7c\0b"
  "\87\80\80\80\00\00\20\00\11\00\00\0b"
)
(module instance $Mt $Mt)
(register "Mt" $Mt)
(module definition $Nt binary
  "\00\61\73\6d\01\00\00\00\01\8d\80\80\80\00\03\60"
  "\00\00\60\00\01\7f\60\01\7f\01\7f\02\92\80\80\80"
  "\00\02\02\4d\74\04\63\61\6c\6c\00\02\02\4d\74\01"
  "\68\00\01\03\84\80\80\80\00\03\01\02\02\04\85\80"
  "\80\80\00\01\70\01\05\05\07\a1\80\80\80\00\03\07"
  "\4d\74\2e\63\61\6c\6c\00\00\0c\63\61\6c\6c\20\4d"
  "\74\2e\63\61\6c\6c\00\03\04\63\61\6c\6c\00\04\09"
  "\95\80\80\80\00\01\04\41\00\0b\05\d2\02\0b\d2\02"
  "\0b\d2\02\0b\d2\01\0b\d2\00\0b\0a\a1\80\80\80\00"
  "\03\84\80\80\80\00\00\41\05\0b\86\80\80\80\00\00"
  "\20\00\10\00\0b\87\80\80\80\00\00\20\00\11\01\00"
  "\0b"
)
(module instance $Nt $Nt)
(assert_return (invoke $Mt "call" (i32.const 0x2)) (i32.const 0x4))
(assert_return (invoke $Nt "Mt.call" (i32.const 0x2)) (i32.const 0x4))
(assert_return (invoke $Nt "call" (i32.const 0x2)) (i32.const 0x5))
(assert_return (invoke $Nt "call Mt.call" (i32.const 0x2)) (i32.const 0x4))
(assert_trap (invoke $Mt "call" (i32.const 0x1)) "uninitialized element")
(assert_trap (invoke $Nt "Mt.call" (i32.const 0x1)) "uninitialized element")
(assert_return (invoke $Nt "call" (i32.const 0x1)) (i32.const 0x5))
(assert_trap
  (invoke $Nt "call Mt.call" (i32.const 0x1))
  "uninitialized element"
)
(assert_trap (invoke $Mt "call" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke $Nt "Mt.call" (i32.const 0x0)) "uninitialized element")
(assert_return (invoke $Nt "call" (i32.const 0x0)) (i32.const 0x5))
(assert_trap
  (invoke $Nt "call Mt.call" (i32.const 0x0))
  "uninitialized element"
)
(assert_trap (invoke $Mt "call" (i32.const 0x14)) "undefined element")
(assert_trap (invoke $Nt "Mt.call" (i32.const 0x14)) "undefined element")
(assert_trap (invoke $Nt "call" (i32.const 0x7)) "undefined element")
(assert_trap (invoke $Nt "call Mt.call" (i32.const 0x14)) "undefined element")
(assert_return (invoke $Nt "call" (i32.const 0x3)) (i32.const 0xffff_fffc))
(assert_trap (invoke $Nt "call" (i32.const 0x4)) "indirect call type mismatch")
(module definition $Ot binary
  "\00\61\73\6d\01\00\00\00\01\8a\80\80\80\00\02\60"
  "\00\01\7f\60\01\7f\01\7f\02\93\80\80\80\00\02\02"
  "\4d\74\01\68\00\00\02\4d\74\03\74\61\62\01\70\00"
  "\05\03\83\80\80\80\00\02\00\01\07\88\80\80\80\00"
  "\01\04\63\61\6c\6c\00\02\09\88\80\80\80\00\01\00"
  "\41\01\0b\02\01\00\0a\96\80\80\80\00\02\84\80\80"
  "\80\00\00\41\06\0b\87\80\80\80\00\00\20\00\11\00"
  "\00\0b"
)
(module instance $Ot $Ot)
(assert_return (invoke $Mt "call" (i32.const 0x3)) (i32.const 0x4))
(assert_return (invoke $Nt "Mt.call" (i32.const 0x3)) (i32.const 0x4))
(assert_return (invoke $Nt "call Mt.call" (i32.const 0x3)) (i32.const 0x4))
(assert_return (invoke $Ot "call" (i32.const 0x3)) (i32.const 0x4))
(assert_return (invoke $Mt "call" (i32.const 0x2)) (i32.const 0xffff_fffc))
(assert_return (invoke $Nt "Mt.call" (i32.const 0x2)) (i32.const 0xffff_fffc))
(assert_return (invoke $Nt "call" (i32.const 0x2)) (i32.const 0x5))
(assert_return
  (invoke $Nt "call Mt.call" (i32.const 0x2))
  (i32.const 0xffff_fffc)
)
(assert_return (invoke $Ot "call" (i32.const 0x2)) (i32.const 0xffff_fffc))
(assert_return (invoke $Mt "call" (i32.const 0x1)) (i32.const 0x6))
(assert_return (invoke $Nt "Mt.call" (i32.const 0x1)) (i32.const 0x6))
(assert_return (invoke $Nt "call" (i32.const 0x1)) (i32.const 0x5))
(assert_return (invoke $Nt "call Mt.call" (i32.const 0x1)) (i32.const 0x6))
(assert_return (invoke $Ot "call" (i32.const 0x1)) (i32.const 0x6))
(assert_trap (invoke $Mt "call" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke $Nt "Mt.call" (i32.const 0x0)) "uninitialized element")
(assert_return (invoke $Nt "call" (i32.const 0x0)) (i32.const 0x5))
(assert_trap
  (invoke $Nt "call Mt.call" (i32.const 0x0))
  "uninitialized element"
)
(assert_trap (invoke $Ot "call" (i32.const 0x0)) "uninitialized element")
(assert_trap (invoke $Ot "call" (i32.const 0x14)) "undefined element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\8c\80\80\80\00\01\02\4d\74\03\74\61\62"
  "\01\70\00\00\03\82\80\80\80\00\01\00\09\87\80\80"
  "\80\00\01\00\41\09\0b\01\00\0a\88\80\80\80\00\01"
  "\82\80\80\80\00\00\0b"
)
(module instance)
(module definition $G1 binary
  "\00\61\73\6d\01\00\00\00\06\86\80\80\80\00\01\7f"
  "\00\41\05\0b\07\85\80\80\80\00\01\01\67\03\00"
)
(module instance $G1 $G1)
(register "G1" $G1)
(module definition $G2 binary
  "\00\61\73\6d\01\00\00\00\02\89\80\80\80\00\01\02"
  "\47\31\01\67\03\7f\00\06\86\80\80\80\00\01\7f\00"
  "\23\00\0b\07\85\80\80\80\00\01\01\67\03\01"
)
(module instance $G2 $G2)
(assert_return (get $G2 "g") (i32.const 0x5))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\8c\80\80\80\00\01\02\4d\74\03\74\61\62"
  "\01\70\00\00\03\82\80\80\80\00\01\00\09\87\80\80"
  "\80\00\01\00\41\0a\0b\01\00\0a\88\80\80\80\00\01"
  "\82\80\80\80\00\00\0b"
)
(assert_trap (module instance) "out of bounds table access")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\00\01\7f\02\96\80\80\80\00\02\02\4d\74\03\74\61"
  "\62\01\70\00\0a\02\4d\74\03\6d\65\6d\02\00\01\03"
  "\82\80\80\80\00\01\00\09\8d\80\80\80\00\02\00\41"
  "\07\0b\01\00\00\41\09\0b\01\00\0a\8a\80\80\80\00"
  "\01\84\80\80\80\00\00\41\00\0b"
)
(assert_unlinkable (module instance) "unknown import")
(assert_trap (invoke $Mt "call" (i32.const 0x7)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\00\01\7f\02\8c\80\80\80\00\01\02\4d\74\03\74\61"
  "\62\01\70\00\0a\03\82\80\80\80\00\01\00\09\91\80"
  "\80\80\00\02\00\41\07\0b\01\00\00\41\08\0b\05\00"
  "\00\00\00\00\0a\8a\80\80\80\00\01\84\80\80\80\00"
  "\00\41\00\0b"
)
(assert_trap (module instance) "out of bounds table access")
(assert_return (invoke $Mt "call" (i32.const 0x7)) (i32.const 0x0))
(assert_trap (invoke $Mt "call" (i32.const 0x8)) "uninitialized element")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\00\01\7f\02\8c\80\80\80\00\01\02\4d\74\03\74\61"
  "\62\01\70\00\0a\03\82\80\80\80\00\01\00\05\83\80"
  "\80\80\00\01\00\01\09\87\80\80\80\00\01\00\41\07"
  "\0b\01\00\0a\8a\80\80\80\00\01\84\80\80\80\00\00"
  "\41\00\0b\0b\89\80\80\80\00\01\00\41\80\80\04\0b"
  "\01\64"
)
(assert_trap (module instance) "out of bounds memory access")
(assert_return (invoke $Mt "call" (i32.const 0x7)) (i32.const 0x0))
(module definition $Mtable_ex binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\04\8b\80\80\80\00\03\70\00\01\63\00\00\01"
  "\6f\00\01\07\a5\80\80\80\00\03\0a\74\2d\66\75\6e"
  "\63\6e\75\6c\6c\01\00\09\74\2d\72\65\66\6e\75\6c"
  "\6c\01\01\08\74\2d\65\78\74\65\72\6e\01\02"
)
(module instance $Mtable_ex $Mtable_ex)
(register "Mtable_ex" $Mtable_ex)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\ca\80\80\80\00\03\09\4d\74\61\62\6c\65"
  "\5f\65\78\0a\74\2d\66\75\6e\63\6e\75\6c\6c\01\70"
  "\00\01\09\4d\74\61\62\6c\65\5f\65\78\09\74\2d\72"
  "\65\66\6e\75\6c\6c\01\63\00\00\01\09\4d\74\61\62"
  "\6c\65\5f\65\78\08\74\2d\65\78\74\65\72\6e\01\6f"
  "\00\01"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\99\80\80\80\00\01\09"
  "\4d\74\61\62\6c\65\5f\65\78\09\74\2d\72\65\66\6e"
  "\75\6c\6c\01\70\00\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\98\80\80\80\00\01\09"
  "\4d\74\61\62\6c\65\5f\65\78\08\74\2d\65\78\74\65"
  "\72\6e\01\70\00\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\9b\80\80\80\00\01\09\4d\74\61\62\6c\65"
  "\5f\65\78\0a\74\2d\66\75\6e\63\6e\75\6c\6c\01\63"
  "\00\00\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\99\80\80\80\00\01\09\4d\74\61\62\6c\65"
  "\5f\65\78\08\74\2d\65\78\74\65\72\6e\01\63\00\00"
  "\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\9a\80\80\80\00\01\09"
  "\4d\74\61\62\6c\65\5f\65\78\0a\74\2d\66\75\6e\63"
  "\6e\75\6c\6c\01\6f\00\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\99\80\80\80\00\01\09"
  "\4d\74\61\62\6c\65\5f\65\78\09\74\2d\72\65\66\6e"
  "\75\6c\6c\01\6f\00\01"
)
(assert_unlinkable (module instance) "incompatible import type")
(module definition $Mm binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7f\01\7f\03\82\80\80\80\00\01\00\05\84\80\80"
  "\80\00\01\01\01\05\07\8e\80\80\80\00\02\03\6d\65"
  "\6d\02\00\04\6c\6f\61\64\00\00\0a\8d\80\80\80\00"
  "\01\87\80\80\80\00\00\20\00\2d\00\00\0b\0b\90\80"
  "\80\80\00\01\00\41\0a\0b\0a\00\01\02\03\04\05\06"
  "\07\08\09"
)
(module instance $Mm $Mm)
(register "Mm" $Mm)
(module definition $Nm binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7f\01\7f\02\8b\80\80\80\00\01\02\4d\6d\04\6c"
  "\6f\61\64\00\00\03\82\80\80\80\00\01\00\05\83\80"
  "\80\80\00\01\00\01\07\92\80\80\80\00\02\07\4d\6d"
  "\2e\6c\6f\61\64\00\00\04\6c\6f\61\64\00\01\0a\8d"
  "\80\80\80\00\01\87\80\80\80\00\00\20\00\2d\00\00"
  "\0b\0b\8c\80\80\80\00\01\00\41\0a\0b\06\f0\f1\f2"
  "\f3\f4\f5"
)
(module instance $Nm $Nm)
(assert_return (invoke $Mm "load" (i32.const 0xc)) (i32.const 0x2))
(assert_return (invoke $Nm "Mm.load" (i32.const 0xc)) (i32.const 0x2))
(assert_return (invoke $Nm "load" (i32.const 0xc)) (i32.const 0xf2))
(module definition $Om binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7f\01\7f\02\8b\80\80\80\00\01\02\4d\6d\03\6d"
  "\65\6d\02\00\01\03\82\80\80\80\00\01\00\07\88\80"
  "\80\80\00\01\04\6c\6f\61\64\00\00\0a\8d\80\80\80"
  "\00\01\87\80\80\80\00\00\20\00\2d\00\00\0b\0b\8e"
  "\80\80\80\00\01\00\41\05\0b\08\a0\a1\a2\a3\a4\a5"
  "\a6\a7"
)
(module instance $Om $Om)
(assert_return (invoke $Mm "load" (i32.const 0xc)) (i32.const 0xa7))
(assert_return (invoke $Nm "Mm.load" (i32.const 0xc)) (i32.const 0xa7))
(assert_return (invoke $Nm "load" (i32.const 0xc)) (i32.const 0xf2))
(assert_return (invoke $Om "load" (i32.const 0xc)) (i32.const 0xa7))
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\8b\80\80\80\00\01\02"
  "\4d\6d\03\6d\65\6d\02\00\00\0b\89\80\80\80\00\01"
  "\00\41\ff\ff\03\0b\01\61"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\8b\80\80\80\00\01\02"
  "\4d\6d\03\6d\65\6d\02\00\00\0b\89\80\80\80\00\01"
  "\00\41\80\80\04\0b\01\61"
)
(assert_trap (module instance) "out of bounds memory access")
(module definition $Pm binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7f\01\7f\02\8c\80\80\80\00\01\02\4d\6d\03\6d"
  "\65\6d\02\01\01\08\03\82\80\80\80\00\01\00\07\88"
  "\80\80\80\00\01\04\67\72\6f\77\00\00\0a\8c\80\80"
  "\80\00\01\86\80\80\80\00\00\20\00\40\00\0b"
)
(module instance $Pm $Pm)
(assert_return (invoke $Pm "grow" (i32.const 0x0)) (i32.const 0x1))
(assert_return (invoke $Pm "grow" (i32.const 0x2)) (i32.const 0x1))
(assert_return (invoke $Pm "grow" (i32.const 0x0)) (i32.const 0x3))
(assert_return (invoke $Pm "grow" (i32.const 0x1)) (i32.const 0x3))
(assert_return (invoke $Pm "grow" (i32.const 0x1)) (i32.const 0x4))
(assert_return (invoke $Pm "grow" (i32.const 0x0)) (i32.const 0x5))
(assert_return (invoke $Pm "grow" (i32.const 0x1)) (i32.const 0xffff_ffff))
(assert_return (invoke $Pm "grow" (i32.const 0x0)) (i32.const 0x5))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\a7\80\80\80\00\03\08\73\70\65\63\74\65"
  "\73\74\05\70\72\69\6e\74\00\00\02\4d\6d\03\6d\65"
  "\6d\02\00\01\02\4d\6d\03\74\61\62\01\70\00\00\0b"
  "\89\80\80\80\00\01\00\41\00\0b\03\61\62\63"
)
(assert_unlinkable (module instance) "unknown import")
(assert_return (invoke $Mm "load" (i32.const 0x0)) (i32.const 0x0))
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\8b\80\80\80\00\01\02"
  "\4d\6d\03\6d\65\6d\02\00\01\0b\a2\80\80\80\00\02"
  "\00\41\00\0b\03\61\62\63\00\41\f6\ff\13\0b\12\7a"
  "\7a\7a\7a\7a\7a\7a\7a\7a\7a\7a\7a\7a\7a\7a\7a\7a"
  "\7a"
)
(assert_trap (module instance) "out of bounds memory access")
(assert_return (invoke $Mm "load" (i32.const 0x0)) (i32.const 0x61))
(assert_return (invoke $Mm "load" (i32.const 0x4_fff6)) (i32.const 0x0))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\02\8b\80\80\80\00\01\02\4d\6d\03\6d\65\6d"
  "\02\00\01\03\82\80\80\80\00\01\00\04\84\80\80\80"
  "\00\01\70\00\00\09\87\80\80\80\00\01\00\41\00\0b"
  "\01\00\0a\88\80\80\80\00\01\82\80\80\80\00\00\0b"
  "\0b\89\80\80\80\00\01\00\41\00\0b\03\61\62\63"
)
(assert_trap (module instance) "out of bounds table access")
(assert_return (invoke $Mm "load" (i32.const 0x0)) (i32.const 0x61))
(module definition $Ms binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\00\01\7f\03\83\80\80\80\00\02\00\00\04\84\80\80"
  "\80\00\01\70\00\01\05\83\80\80\80\00\01\00\01\07"
  "\b1\80\80\80\00\04\06\6d\65\6d\6f\72\79\02\00\05"
  "\74\61\62\6c\65\01\00\0d\67\65\74\20\6d\65\6d\6f"
  "\72\79\5b\30\5d\00\00\0c\67\65\74\20\74\61\62\6c"
  "\65\5b\30\5d\00\01\0a\99\80\80\80\00\02\87\80\80"
  "\80\00\00\41\00\2d\00\00\0b\87\80\80\80\00\00\41"
  "\00\11\00\00\0b"
)
(module instance $Ms $Ms)
(register "Ms" $Ms)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\88\80\80\80\00\02\60"
  "\00\01\7f\60\00\00\02\9b\80\80\80\00\02\02\4d\73"
  "\06\6d\65\6d\6f\72\79\02\00\01\02\4d\73\05\74\61"
  "\62\6c\65\01\70\00\01\03\83\80\80\80\00\02\00\01"
  "\08\81\80\80\80\00\01\09\87\80\80\80\00\01\00\41"
  "\00\0b\01\00\0a\94\80\80\80\00\02\86\80\80\80\00"
  "\00\41\ad\bd\03\0b\83\80\80\80\00\00\00\0b\0b\8b"
  "\80\80\80\00\01\00\41\00\0b\05\68\65\6c\6c\6f"
)
(assert_trap (module instance) "unreachable")
(assert_return (invoke $Ms "get memory[0]") (i32.const 0x68))
(assert_return (invoke $Ms "get table[0]") (i32.const 0xdead))
