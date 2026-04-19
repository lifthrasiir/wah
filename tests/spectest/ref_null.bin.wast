(module definition binary
  "\00\61\73\6d\01\00\00\00\01\99\80\80\80\00\06\60"
  "\00\00\60\00\01\6e\60\00\01\70\60\00\01\69\60\00"
  "\01\6f\60\00\01\63\00\03\86\80\80\80\00\05\01\02"
  "\03\04\05\06\9b\80\80\80\00\05\6e\00\d0\6e\0b\70"
  "\00\d0\70\0b\69\00\d0\69\0b\6f\00\d0\6f\0b\63\00"
  "\00\d0\00\0b\07\af\80\80\80\00\05\06\61\6e\79\72"
  "\65\66\00\00\07\66\75\6e\63\72\65\66\00\01\06\65"
  "\78\6e\72\65\66\00\02\09\65\78\74\65\72\6e\72\65"
  "\66\00\03\03\72\65\66\00\04\0a\ae\80\80\80\00\05"
  "\84\80\80\80\00\00\d0\6e\0b\84\80\80\80\00\00\d0"
  "\70\0b\84\80\80\80\00\00\d0\69\0b\84\80\80\80\00"
  "\00\d0\6f\0b\84\80\80\80\00\00\d0\00\0b"
)
(module instance)
(assert_return (invoke "anyref") (ref.null any))
(assert_return (invoke "funcref") (ref.null func))
(assert_return (invoke "exnref") (ref.null exn))
(assert_return (invoke "externref") (ref.null extern))
(assert_return (invoke "ref") (ref.null))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\a9\80\80\80\00\0a\60"
  "\00\00\60\00\01\6e\60\00\01\71\60\00\01\70\60\00"
  "\01\73\60\00\01\69\60\00\01\74\60\00\01\6f\60\00"
  "\01\72\60\00\01\63\00\03\8a\80\80\80\00\09\01\02"
  "\03\04\05\06\07\08\09\06\dd\80\80\80\00\12\71\00"
  "\d0\71\0b\73\00\d0\73\0b\74\00\d0\74\0b\72\00\d0"
  "\72\0b\6e\00\d0\6e\0b\6e\00\d0\71\0b\70\00\d0\70"
  "\0b\70\00\d0\73\0b\69\00\d0\69\0b\69\00\d0\74\0b"
  "\6f\00\d0\6f\0b\6f\00\d0\72\0b\71\00\d0\71\0b\73"
  "\00\d0\73\0b\74\00\d0\74\0b\72\00\d0\72\0b\63\00"
  "\00\d0\00\0b\63\00\00\d0\73\0b\07\e4\80\80\80\00"
  "\09\06\61\6e\79\72\65\66\00\00\07\6e\75\6c\6c\72"
  "\65\66\00\01\07\66\75\6e\63\72\65\66\00\02\0b\6e"
  "\75\6c\6c\66\75\6e\63\72\65\66\00\03\06\65\78\6e"
  "\72\65\66\00\04\0a\6e\75\6c\6c\65\78\6e\72\65\66"
  "\00\05\09\65\78\74\65\72\6e\72\65\66\00\06\0d\6e"
  "\75\6c\6c\65\78\74\65\72\6e\72\65\66\00\07\03\72"
  "\65\66\00\08\0a\d2\80\80\80\00\09\84\80\80\80\00"
  "\00\23\00\0b\84\80\80\80\00\00\23\00\0b\84\80\80"
  "\80\00\00\23\01\0b\84\80\80\80\00\00\23\01\0b\84"
  "\80\80\80\00\00\23\02\0b\84\80\80\80\00\00\23\02"
  "\0b\84\80\80\80\00\00\23\03\0b\84\80\80\80\00\00"
  "\23\03\0b\84\80\80\80\00\00\23\01\0b"
)
(module instance)
(assert_return (invoke "anyref") (ref.null any))
(assert_return (invoke "anyref") (ref.null none))
(assert_return (invoke "anyref") (ref.null))
(assert_return (invoke "nullref") (ref.null any))
(assert_return (invoke "nullref") (ref.null none))
(assert_return (invoke "nullref") (ref.null))
(assert_return (invoke "funcref") (ref.null func))
(assert_return (invoke "funcref") (ref.null nofunc))
(assert_return (invoke "funcref") (ref.null))
(assert_return (invoke "nullfuncref") (ref.null func))
(assert_return (invoke "nullfuncref") (ref.null nofunc))
(assert_return (invoke "nullfuncref") (ref.null))
(assert_return (invoke "exnref") (ref.null exn))
(assert_return (invoke "exnref") (ref.null noexn))
(assert_return (invoke "exnref") (ref.null))
(assert_return (invoke "nullexnref") (ref.null exn))
(assert_return (invoke "nullexnref") (ref.null noexn))
(assert_return (invoke "nullexnref") (ref.null))
(assert_return (invoke "externref") (ref.null extern))
(assert_return (invoke "externref") (ref.null noextern))
(assert_return (invoke "externref") (ref.null))
(assert_return (invoke "nullexternref") (ref.null extern))
(assert_return (invoke "nullexternref") (ref.null noextern))
(assert_return (invoke "nullexternref") (ref.null))
(assert_return (invoke "ref") (ref.null func))
(assert_return (invoke "ref") (ref.null nofunc))
(assert_return (invoke "ref") (ref.null))
