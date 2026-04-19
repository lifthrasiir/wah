(module definition binary "\00\61\73\6d\01\00\00\00")
(module instance)
(module definition binary "\00\61\73\6d\01\00\00\00")
(module instance)
(module definition $M1 binary "\00\61\73\6d\01\00\00\00")
(module instance $M1 $M1)
(module definition $M2 binary "\00\61\73\6d\01\00\00\00")
(module instance $M2 $M2)
(assert_malformed (module binary) "unexpected end")
(assert_malformed (module binary "\01") "unexpected end")
(assert_malformed (module binary "\00\61\73") "unexpected end")
(assert_malformed (module binary "\61\73\6d\00") "magic header not detected")
(assert_malformed (module binary "\6d\73\61\00") "magic header not detected")
(assert_malformed
  (module binary "\6d\73\61\00\01\00\00\00")
  "magic header not detected"
)
(assert_malformed
  (module binary "\6d\73\61\00\00\00\00\01")
  "magic header not detected"
)
(assert_malformed
  (module binary "\61\73\6d\01\00\00\00\00")
  "magic header not detected"
)
(assert_malformed
  (module binary "\77\61\73\6d\01\00\00\00")
  "magic header not detected"
)
(assert_malformed
  (module binary "\7f\61\73\6d\01\00\00\00")
  "magic header not detected"
)
(assert_malformed
  (module binary "\80\61\73\6d\01\00\00\00")
  "magic header not detected"
)
(assert_malformed
  (module binary "\82\61\73\6d\01\00\00\00")
  "magic header not detected"
)
(assert_malformed
  (module binary "\ff\61\73\6d\01\00\00\00")
  "magic header not detected"
)
(assert_malformed
  (module binary "\00\00\00\01\6d\73\61\00")
  "magic header not detected"
)
(assert_malformed
  (module binary "\61\00\6d\73\00\01\00\00")
  "magic header not detected"
)
(assert_malformed
  (module binary "\73\6d\00\61\00\00\01\00")
  "magic header not detected"
)
(assert_malformed
  (module binary "\00\41\53\4d\01\00\00\00")
  "magic header not detected"
)
(assert_malformed
  (module binary "\00\81\a2\94\01\00\00\00")
  "magic header not detected"
)
(assert_malformed
  (module binary "\ef\bb\bf\00\61\73\6d\01\00\00\00")
  "magic header not detected"
)
(assert_malformed (module binary "\00\61\73\6d") "unexpected end")
(assert_malformed (module binary "\00\61\73\6d\01") "unexpected end")
(assert_malformed (module binary "\00\61\73\6d\01\00\00") "unexpected end")
(assert_malformed
  (module binary "\00\61\73\6d\00\00\00\00")
  "unknown binary version"
)
(assert_malformed
  (module binary "\00\61\73\6d\0d\00\00\00")
  "unknown binary version"
)
(assert_malformed
  (module binary "\00\61\73\6d\0e\00\00\00")
  "unknown binary version"
)
(assert_malformed
  (module binary "\00\61\73\6d\00\01\00\00")
  "unknown binary version"
)
(assert_malformed
  (module binary "\00\61\73\6d\00\00\01\00")
  "unknown binary version"
)
(assert_malformed
  (module binary "\00\61\73\6d\00\00\00\01")
  "unknown binary version"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\0e\01\00")
  "malformed section id"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\7f\01\00")
  "malformed section id"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\80\01\00\01\01\00")
  "malformed section id"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\81\01\00\01\01\00")
  "malformed section id"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\ff\01\00\01\01\00")
  "malformed section id"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\03"
    "\02\00\00\0a\0c\02\04\00\41\01\1a\05\00\41\01\1a"
    "\0b"
  )
  "END opcode expected"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\0a\06\01\04\00\41\01\1a"
  )
  "unexpected end of section or function"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\0a\06\01\04\00\41\01\1a\0b\03\01\01\00"
  )
  "section size mismatch"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\06\05\01\7f\00\41\00\0a\04\01\02\00\0b"
  )
  "unexpected end of section or function"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\0a\0c\01\0a\02\80\80\80\80\10\7f\02\7e\0b"
  )
  "integer too large"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\0a\0c\01\0a\02\80\80\80\80\10\7f\02\7e\0b"
  )
  "integer too large"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\0a\0c\01\0a\02\ff\ff\ff\ff\0f\7f\02\7e\0b"
  )
  "too many locals"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\06\01\60\02\7f\7f\00"
    "\03\02\01\00\0a\1c\01\1a\04\80\80\80\80\04\7f\80"
    "\80\80\80\04\7e\80\80\80\80\04\7d\80\80\80\80\04"
    "\7c\0b"
  )
  "too many locals"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\03\82\80\80\80\00\01\00\0a\8a\80\80\80\00"
  "\01\84\80\80\80\00\01\02\7d\0b"
)
(module instance)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\03" "\02\00\00")
  "function and code section have inconsistent lengths"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\0a\04\01\02\00\0b")
  "function and code section have inconsistent lengths"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\03"
    "\02\00\00\0a\04\01\02\00\0b"
  )
  "function and code section have inconsistent lengths"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\0a\07\02\02\00\0b\02\00\0b"
  )
  "function and code section have inconsistent lengths"
)
(module definition binary "\00\61\73\6d\01\00\00\00")
(module instance)
(module definition binary "\00\61\73\6d\01\00\00\00")
(module instance)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\0c\01\03\0b\05\02\01\00" "\01\00")
  "data count and data section have inconsistent lengths"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\0c\01\01\0b\05\02\01\00" "\01\00")
  "data count and data section have inconsistent lengths"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\05\03\01\00\01\0c\01\01")
  "data count and data section have inconsistent lengths"
)
(module definition binary "\00\61\73\6d\01\00\00\00")
(module instance)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\05\03\01\00\00\0a\0e\01\0c\00\41\00\41\00"
    "\41\00\fc\08\00\00\0b\0b\03\01\01\00"
  )
  "data count section required"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\05\03\01\00\00\0a\07\01\05\00\fc\09\00\0b"
    "\0b\03\01\01\00"
  )
  "data count section required"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\04\04\01\70\00\00\05\03\01\00\00\09\07\01"
    "\05\70\01\f3\00\0b\0a\04\01\02\00\0b"
  )
  "illegal opcode"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\04\04\01\70\00\00\05\03\01\00\00\09\07\01"
    "\05\7f\01\d2\00\0b\0a\04\01\02\00\0b"
  )
  "malformed reference type"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\03\82\80\80\80\00\01\00\04\84\80\80\80\00"
  "\01\70\00\00\05\83\80\80\80\00\01\00\00\09\87\80"
  "\80\80\00\01\05\70\01\d2\00\0b\0a\88\80\80\80\00"
  "\01\82\80\80\80\00\00\0b"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\03\82\80\80\80\00\01\00\04\84\80\80\80\00"
  "\01\70\00\00\05\83\80\80\80\00\01\00\00\09\87\80"
  "\80\80\00\01\05\70\01\d0\70\0b\0a\88\80\80\80\00"
  "\01\82\80\80\80\00\00\0b"
)
(module instance)
(module definition binary "\00\61\73\6d\01\00\00\00")
(module instance)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\01\07\02\60\00\00")
  "length out of bounds"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\01\07\01\60\00\00\60\00" "\00")
  "section size mismatch"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\01\7f\00"
)
(module instance)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\02\04\01\00\00\05")
  "malformed import kind"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\02\05\01\00\00\05\00")
  "malformed import kind"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\02\04\01\00\00\05")
  "malformed import kind"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\02\05\01\00\00\05\00")
  "malformed import kind"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\02\04\01\00\00\80")
  "malformed import kind"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\02\05\01\00\00\80\00")
  "malformed import kind"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\05\01\60\01\7f\00\02"
    "\16\02\08\73\70\65\63\74\65\73\74\09\70\72\69\6e"
    "\74\5f\69\33\32\00\00"
  )
  "unexpected end of section or function"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\09\02\60\01\7f\00\60"
    "\01\7d\00\02\2b\01\08\73\70\65\63\74\65\73\74\09"
    "\70\72\69\6e\74\5f\69\33\32\00\00\08\73\70\65\63"
    "\74\65\73\74\09\70\72\69\6e\74\5f\66\33\32\00\01"
  )
  "section size mismatch"
)
(module definition binary "\00\61\73\6d\01\00\00\00")
(module instance)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\04\01\01")
  "unexpected end of section or function"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\04\03\01\70\08")
  "malformed limits flags"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\04\04\01\70\08\00")
  "malformed limits flags"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\04\06\01\70\81\00\00\00")
  "malformed limits flags"
)
(module definition binary "\00\61\73\6d\01\00\00\00")
(module instance)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\05\01\01")
  "unexpected end of section or function"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\05\02\01\08")
  "malformed limits flags"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\05\03\01\10\00")
  "malformed limits flags"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\05\05\01\81\00\00\00")
  "malformed limits flags"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\05\05\01\81\01\00\00")
  "malformed limits flags"
)
(module definition binary "\00\61\73\6d\01\00\00\00")
(module instance)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\06\06\02\7f\00\41\00\0b")
  "unexpected end of section or function"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\06\0b\01\7f\00\41\00\0b"
    "\7f\00\41\00\0b"
  )
  "section size mismatch"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\03\83\80\80\80\00\02\00\00\0a\8f\80\80\80"
  "\00\02\82\80\80\80\00\00\0b\82\80\80\80\00\00\0b"
)
(module instance)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\03"
    "\02\00\00\07\06\02\02\66\31\00\00\0a\07\02\02\00"
    "\0b\02\00\0b"
  )
  "length out of bounds"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\03"
    "\02\00\00\07\0b\01\02\66\31\00\00\02\66\32\00\01"
    "\0a\07\02\02\00\0b\02\00\0b"
  )
  "section size mismatch"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\03\82\80\80\80\00\01\00\04\84\80\80\80\00"
  "\01\70\00\01\0a\88\80\80\80\00\01\82\80\80\80\00"
  "\00\0b"
)
(module instance)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\04\04\01\70\00\01\09\07\02\00\41\00\0b\01"
    "\00"
  )
  "unexpected end"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\04\04\01\70\00\01\09\0a\02\00\41\00\0b\01"
    "\00\00\41\00"
  )
  "unexpected end"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\04\04\01\70\00\01\09\0d\01\00\41\00\0b\01"
    "\00\00\41\00\0b\01\00\0a\04\01\02\00\0b"
  )
  "section size mismatch"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\05\83\80\80\80\00\01\00"
  "\01"
)
(module instance)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\05\03\01\00\01\0b\07\02"
    "\00\41\00\0b\01\61"
  )
  "unexpected end of section or function"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\05\03\01\00\01\0b\0d\01"
    "\00\41\00\0b\01\61\00\41\01\0b\01\62"
  )
  "section size mismatch"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\05\03\01\00\01\0b\0c\01"
    "\00\41\03\0b\07\61\62\63\64\65\66"
  )
  "unexpected end of section or function"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\05\03\01\00\01\0b\0c\01"
    "\00\41\00\0b\05\61\62\63\64\65\66"
  )
  "section size mismatch"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\03\82\80\80\80\00\01\00\0a\95\80\80\80\00"
  "\01\8f\80\80\80\00\00\02\40\41\01\04\40\41\01\0e"
  "\00\02\0b\0b\0b"
)
(module instance)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\25\0c\60\00\00\60\00"
    "\00\60\00\00\60\00\00\60\00\00\60\00\00\60\00\00"
    "\60\00\00\60\00\00\60\00\00\60\00\00\60\00\00\03"
    "\02\01\00\0a\13\01\11\00\02\40\41\01\04\40\41\01"
    "\0e\01\00\01\02\0b\0b\0b"
  )
  "unexpected end of section or function"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
  "\00\00\03\82\80\80\80\00\01\00\08\81\80\80\80\00"
  "\00\0a\88\80\80\80\00\01\82\80\80\80\00\00\0b"
)
(module instance)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\08\01\00\08\01\00\0a\04\01\02\00\0b"
  )
  "unexpected content after last section"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\03\02\01\00\0a\07\02\02\00\0b\02\00\0b"
  )
  "unexpected content after last section"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\03"
    "\02\00\00\0a\04\01\02\00\0b\0a\04\01\02\00\0b"
  )
  "unexpected content after last section"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\0c\01\01\0c\01\01")
  "unexpected content after last section"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\0b\01\00\0b\01\00")
  "unexpected content after last section"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\06\01\00\06\01\00")
  "unexpected content after last section"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\07\01\00\07\01\00")
  "unexpected content after last section"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\04\01\00\04\01\00")
  "unexpected content after last section"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\09\01\00\09\01\00")
  "unexpected content after last section"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\02\01\00\02\01\00")
  "unexpected content after last section"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\01\01\00\01\01\00")
  "unexpected content after last section"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\05\01\00\05\01\00")
  "unexpected content after last section"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\02\01\00\01\01\00")
  "unexpected content after last section"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\03\01\00\02\01\00")
  "unexpected content after last section"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\04\01\00\03\01\00")
  "unexpected content after last section"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\05\01\00\04\01\00")
  "unexpected content after last section"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\06\01\00\05\01\00")
  "unexpected content after last section"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\07\01\00\06\01\00")
  "unexpected content after last section"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\08\01\00\07\01\00"
  )
  "unexpected content after last section"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\09\01\00\08\01\00"
  )
  "unexpected content after last section"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\0c\01\01\09\01\00")
  "unexpected content after last section"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\0a\01\00\0c\01\01")
  "unexpected content after last section"
)
(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\0b\01\00\0a\01\00")
  "unexpected content after last section"
)
(assert_malformed
  (module binary
    "\00\61\73\6d\01\00\00\00\01\04\01\60\00\00\03\02"
    "\01\00\0a\08\01\06\00\00\ff\00\00\0b"
  )
  "illegal opcode ff"
)
