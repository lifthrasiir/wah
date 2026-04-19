(module definition binary
  "\00\61\73\6d\01\00\00\00\05\83\80\80\80\00\01\00"
  "\00"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\05\83\80\80\80\00\01\00"
  "\01"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\05\84\80\80\80\00\01\01"
  "\00\00"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\05\84\80\80\80\00\01\01"
  "\00\01"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\05\85\80\80\80\00\01\01"
  "\01\80\02"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\05\85\80\80\80\00\01\00"
  "\80\80\04"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\05\86\80\80\80\00\01\01"
  "\00\80\80\04"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\00\01\7f\03\82\80\80\80\00\01\00\05\84\80\80\80"
  "\00\01\01\00\00\07\8b\80\80\80\00\01\07\6d\65\6d"
  "\73\69\7a\65\00\00\0a\8a\80\80\80\00\01\84\80\80"
  "\80\00\00\3f\00\0b\0b\86\80\80\80\00\01\00\41\00"
  "\0b\00"
)
(module instance)
(assert_return (invoke "memsize") (i32.const 0x0))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\00\01\7f\03\82\80\80\80\00\01\00\05\84\80\80\80"
  "\00\01\01\00\00\07\8b\80\80\80\00\01\07\6d\65\6d"
  "\73\69\7a\65\00\00\0a\8a\80\80\80\00\01\84\80\80"
  "\80\00\00\3f\00\0b\0b\86\80\80\80\00\01\00\41\00"
  "\0b\00"
)
(module instance)
(assert_return (invoke "memsize") (i32.const 0x0))
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\85\80\80\80\00\01\60"
  "\00\01\7f\03\82\80\80\80\00\01\00\05\84\80\80\80"
  "\00\01\01\01\01\07\8b\80\80\80\00\01\07\6d\65\6d"
  "\73\69\7a\65\00\00\0a\8a\80\80\80\00\01\84\80\80"
  "\80\00\00\3f\00\0b\0b\87\80\80\80\00\01\00\41\00"
  "\0b\01\78"
)
(module instance)
(assert_return (invoke "memsize") (i32.const 0x1))
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\0b\86\80\80\80\00\01\00"
    "\41\00\0b\00"
  )
  "unknown memory"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\0b\86\80\80\80\00\01\00"
    "\41\00\0b\00"
  )
  "unknown memory"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\0b\87\80\80\80\00\01\00"
    "\41\00\0b\01\78"
  )
  "unknown memory"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\8e\80\80\80\00"
    "\01\88\80\80\80\00\00\41\00\2a\02\00\1a\0b"
  )
  "unknown memory"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\92\80\80\80\00"
    "\01\8c\80\80\80\00\00\41\00\43\00\00\00\00\38\02"
    "\00\0b"
  )
  "unknown memory"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\8e\80\80\80\00"
    "\01\88\80\80\80\00\00\41\00\2c\00\00\1a\0b"
  )
  "unknown memory"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\8f\80\80\80\00"
    "\01\89\80\80\80\00\00\41\00\41\00\3a\00\00\0b"
  )
  "unknown memory"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\8b\80\80\80\00"
    "\01\85\80\80\80\00\00\3f\00\1a\0b"
  )
  "unknown memory"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\01\84\80\80\80\00\01\60"
    "\00\00\03\82\80\80\80\00\01\00\0a\8d\80\80\80\00"
    "\01\87\80\80\80\00\00\41\00\40\00\1a\0b"
  )
  "unknown memory"
)
(assert_invalid
  (module binary "\00\61\73\6d\01\00\00\00\05\84\80\80\80\00\01\01" "\01\00")
  "size minimum must not be greater than maximum"
)
(assert_invalid
  (module binary "\00\61\73\6d\01\00\00\00\05\85\80\80\80\00\01\00" "\81\80\04")
  "memory size"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\05\87\80\80\80\00\01\00"
    "\80\80\80\80\08"
  )
  "memory size"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\05\87\80\80\80\00\01\00"
    "\ff\ff\ff\ff\0f"
  )
  "memory size"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\05\86\80\80\80\00\01\01"
    "\00\81\80\04"
  )
  "memory size"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\05\88\80\80\80\00\01\01"
    "\00\80\80\80\80\08"
  )
  "memory size"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\05\88\80\80\80\00\01\01"
    "\00\ff\ff\ff\ff\0f"
  )
  "memory size"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\05\87\80\80\80\00\01\00"
    "\80\80\80\80\10"
  )
  "memory size"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\05\8c\80\80\80\00\01\01"
    "\80\80\80\80\10\80\80\80\80\10"
  )
  "memory size"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\05\88\80\80\80\00\01\01"
    "\00\80\80\80\80\10"
  )
  "memory size"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\02\8c\80\80\80\00\01\01"
    "\4d\01\6d\02\00\80\80\80\80\10"
  )
  "memory size"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\02\91\80\80\80\00\01\01"
    "\4d\01\6d\02\01\80\80\80\80\10\80\80\80\80\10"
  )
  "memory size"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\02\8d\80\80\80\00\01\01"
    "\4d\01\6d\02\01\00\80\80\80\80\10"
  )
  "memory size"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\93\80\80\80\00\04\60"
  "\00\01\7f\60\00\01\7c\60\01\7f\01\7f\60\01\7e\01"
  "\7e\03\8d\80\80\80\00\0c\00\01\02\02\02\02\03\03"
  "\03\03\03\03\05\83\80\80\80\00\01\00\01\07\a1\81"
  "\80\80\00\0c\04\64\61\74\61\00\00\04\63\61\73\74"
  "\00\01\0b\69\33\32\5f\6c\6f\61\64\38\5f\73\00\02"
  "\0b\69\33\32\5f\6c\6f\61\64\38\5f\75\00\03\0c\69"
  "\33\32\5f\6c\6f\61\64\31\36\5f\73\00\04\0c\69\33"
  "\32\5f\6c\6f\61\64\31\36\5f\75\00\05\0b\69\36\34"
  "\5f\6c\6f\61\64\38\5f\73\00\06\0b\69\36\34\5f\6c"
  "\6f\61\64\38\5f\75\00\07\0c\69\36\34\5f\6c\6f\61"
  "\64\31\36\5f\73\00\08\0c\69\36\34\5f\6c\6f\61\64"
  "\31\36\5f\75\00\09\0c\69\36\34\5f\6c\6f\61\64\33"
  "\32\5f\73\00\0a\0c\69\36\34\5f\6c\6f\61\64\33\32"
  "\5f\75\00\0b\0a\cf\82\80\80\00\0c\ce\80\80\80\00"
  "\00\41\00\2d\00\00\41\c1\00\46\41\03\2d\00\00\41"
  "\a7\01\46\71\41\06\2d\00\00\41\00\46\41\13\2d\00"
  "\00\41\00\46\71\71\41\14\2d\00\00\41\d7\00\46\41"
  "\17\2d\00\00\41\cd\00\46\71\41\18\2d\00\00\41\00"
  "\46\41\ff\07\2d\00\00\41\00\46\71\71\71\0b\b8\80"
  "\80\80\00\00\41\08\42\c7\9f\7f\37\03\00\41\08\2b"
  "\03\00\42\c7\9f\7f\bf\61\04\40\44\00\00\00\00\00"
  "\00\00\00\0f\0b\41\09\42\00\37\00\00\41\0f\41\c5"
  "\80\01\3b\00\00\41\09\2b\00\00\0b\8e\80\80\80\00"
  "\00\41\08\20\00\3a\00\00\41\08\2c\00\00\0b\8e\80"
  "\80\80\00\00\41\08\20\00\3a\00\00\41\08\2d\00\00"
  "\0b\8e\80\80\80\00\00\41\08\20\00\3b\01\00\41\08"
  "\2e\01\00\0b\8e\80\80\80\00\00\41\08\20\00\3b\01"
  "\00\41\08\2f\01\00\0b\8e\80\80\80\00\00\41\08\20"
  "\00\3c\00\00\41\08\30\00\00\0b\8e\80\80\80\00\00"
  "\41\08\20\00\3c\00\00\41\08\31\00\00\0b\8e\80\80"
  "\80\00\00\41\08\20\00\3d\01\00\41\08\32\01\00\0b"
  "\8e\80\80\80\00\00\41\08\20\00\3d\01\00\41\08\33"
  "\01\00\0b\8e\80\80\80\00\00\41\08\20\00\3e\02\00"
  "\41\08\34\02\00\0b\8e\80\80\80\00\00\41\08\20\00"
  "\3e\02\00\41\08\35\02\00\0b\0b\94\80\80\80\00\02"
  "\00\41\00\0b\05\41\42\43\a7\44\00\41\14\0b\04\57"
  "\41\53\4d"
)
(module instance)
(assert_return (invoke "data") (i32.const 0x1))
(assert_return (invoke "cast") (f64.const 0x1.5p+5))
(assert_return
  (invoke "i32_load8_s" (i32.const 0xffff_ffff))
  (i32.const 0xffff_ffff)
)
(assert_return (invoke "i32_load8_u" (i32.const 0xffff_ffff)) (i32.const 0xff))
(assert_return
  (invoke "i32_load16_s" (i32.const 0xffff_ffff))
  (i32.const 0xffff_ffff)
)
(assert_return
  (invoke "i32_load16_u" (i32.const 0xffff_ffff))
  (i32.const 0xffff)
)
(assert_return (invoke "i32_load8_s" (i32.const 0x64)) (i32.const 0x64))
(assert_return (invoke "i32_load8_u" (i32.const 0xc8)) (i32.const 0xc8))
(assert_return (invoke "i32_load16_s" (i32.const 0x4e20)) (i32.const 0x4e20))
(assert_return (invoke "i32_load16_u" (i32.const 0x9c40)) (i32.const 0x9c40))
(assert_return (invoke "i32_load8_s" (i32.const 0xfedc_6543)) (i32.const 0x43))
(assert_return
  (invoke "i32_load8_s" (i32.const 0x3456_cdef))
  (i32.const 0xffff_ffef)
)
(assert_return (invoke "i32_load8_u" (i32.const 0xfedc_6543)) (i32.const 0x43))
(assert_return (invoke "i32_load8_u" (i32.const 0x3456_cdef)) (i32.const 0xef))
(assert_return
  (invoke "i32_load16_s" (i32.const 0xfedc_6543))
  (i32.const 0x6543)
)
(assert_return
  (invoke "i32_load16_s" (i32.const 0x3456_cdef))
  (i32.const 0xffff_cdef)
)
(assert_return
  (invoke "i32_load16_u" (i32.const 0xfedc_6543))
  (i32.const 0x6543)
)
(assert_return
  (invoke "i32_load16_u" (i32.const 0x3456_cdef))
  (i32.const 0xcdef)
)
(assert_return
  (invoke "i64_load8_s" (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "i64_load8_u" (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0xff)
)
(assert_return
  (invoke "i64_load16_s" (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "i64_load16_u" (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0xffff)
)
(assert_return
  (invoke "i64_load32_s" (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0xffff_ffff_ffff_ffff)
)
(assert_return
  (invoke "i64_load32_u" (i64.const 0xffff_ffff_ffff_ffff))
  (i64.const 0xffff_ffff)
)
(assert_return (invoke "i64_load8_s" (i64.const 0x64)) (i64.const 0x64))
(assert_return (invoke "i64_load8_u" (i64.const 0xc8)) (i64.const 0xc8))
(assert_return (invoke "i64_load16_s" (i64.const 0x4e20)) (i64.const 0x4e20))
(assert_return (invoke "i64_load16_u" (i64.const 0x9c40)) (i64.const 0x9c40))
(assert_return (invoke "i64_load32_s" (i64.const 0x4e20)) (i64.const 0x4e20))
(assert_return (invoke "i64_load32_u" (i64.const 0x9c40)) (i64.const 0x9c40))
(assert_return
  (invoke "i64_load8_s" (i64.const 0xfedc_ba98_5634_6543))
  (i64.const 0x43)
)
(assert_return
  (invoke "i64_load8_s" (i64.const 0x3456_4365_98ba_cdef))
  (i64.const 0xffff_ffff_ffff_ffef)
)
(assert_return
  (invoke "i64_load8_u" (i64.const 0xfedc_ba98_5634_6543))
  (i64.const 0x43)
)
(assert_return
  (invoke "i64_load8_u" (i64.const 0x3456_4365_98ba_cdef))
  (i64.const 0xef)
)
(assert_return
  (invoke "i64_load16_s" (i64.const 0xfedc_ba98_5634_6543))
  (i64.const 0x6543)
)
(assert_return
  (invoke "i64_load16_s" (i64.const 0x3456_4365_98ba_cdef))
  (i64.const 0xffff_ffff_ffff_cdef)
)
(assert_return
  (invoke "i64_load16_u" (i64.const 0xfedc_ba98_5634_6543))
  (i64.const 0x6543)
)
(assert_return
  (invoke "i64_load16_u" (i64.const 0x3456_4365_98ba_cdef))
  (i64.const 0xcdef)
)
(assert_return
  (invoke "i64_load32_s" (i64.const 0xfedc_ba98_5634_6543))
  (i64.const 0x5634_6543)
)
(assert_return
  (invoke "i64_load32_s" (i64.const 0x3456_4365_98ba_cdef))
  (i64.const 0xffff_ffff_98ba_cdef)
)
(assert_return
  (invoke "i64_load32_u" (i64.const 0xfedc_ba98_5634_6543))
  (i64.const 0x5634_6543)
)
(assert_return
  (invoke "i64_load32_u" (i64.const 0x3456_4365_98ba_cdef))
  (i64.const 0x98ba_cdef)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\86\80\80\80\00\01\60"
  "\01\7f\01\7f\03\82\80\80\80\00\01\00\05\84\80\80"
  "\80\00\01\01\01\01\06\96\80\80\80\00\03\7f\00\41"
  "\90\ce\00\0b\7f\00\41\90\ce\00\0b\7f\00\41\90\ce"
  "\00\0b\07\ba\80\80\80\00\05\06\6d\65\6d\6f\72\79"
  "\02\00\0a\5f\5f\64\61\74\61\5f\65\6e\64\03\00\0b"
  "\5f\5f\73\74\61\63\6b\5f\74\6f\70\03\01\0b\5f\5f"
  "\68\65\61\70\5f\62\61\73\65\03\02\04\6c\6f\61\64"
  "\00\00\0a\8d\80\80\80\00\01\87\80\80\80\00\00\20"
  "\00\2d\00\00\0b"
)
(module instance)
(assert_return (invoke "load" (i32.const 0x0)) (i32.const 0x0))
(assert_return (invoke "load" (i32.const 0x2710)) (i32.const 0x0))
(assert_return (invoke "load" (i32.const 0x4e20)) (i32.const 0x0))
(assert_return (invoke "load" (i32.const 0x7530)) (i32.const 0x0))
(assert_return (invoke "load" (i32.const 0x9c40)) (i32.const 0x0))
(assert_return (invoke "load" (i32.const 0xc350)) (i32.const 0x0))
(assert_return (invoke "load" (i32.const 0xea60)) (i32.const 0x0))
(assert_return (invoke "load" (i32.const 0xffff)) (i32.const 0x0))
