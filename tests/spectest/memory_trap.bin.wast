(module definition binary
  "\00\61\73\6d\01\00\00\00\01\8f\80\80\80\00\03\60"
  "\00\01\7f\60\02\7f\7f\00\60\01\7f\01\7f\03\85\80"
  "\80\80\00\04\00\01\02\02\05\83\80\80\80\00\01\00"
  "\01\07\9e\80\80\80\00\03\05\73\74\6f\72\65\00\01"
  "\04\6c\6f\61\64\00\02\0b\6d\65\6d\6f\72\79\2e\67"
  "\72\6f\77\00\03\0a\ba\80\80\80\00\04\89\80\80\80"
  "\00\00\3f\00\41\80\80\04\6c\0b\8c\80\80\80\00\00"
  "\10\00\20\00\6a\20\01\36\02\00\0b\8a\80\80\80\00"
  "\00\10\00\20\00\6a\28\02\00\0b\86\80\80\80\00\00"
  "\20\00\40\00\0b"
)
(module instance)
(assert_return (invoke "store" (i32.const 0xffff_fffc) (i32.const 0x2a)))
(assert_return (invoke "load" (i32.const 0xffff_fffc)) (i32.const 0x2a))
(assert_trap
  (invoke "store" (i32.const 0xffff_fffd) (i32.const 0x1234_5678))
  "out of bounds memory access"
)
(assert_trap
  (invoke "load" (i32.const 0xffff_fffd))
  "out of bounds memory access"
)
(assert_trap
  (invoke "store" (i32.const 0xffff_fffe) (i32.const 0xd))
  "out of bounds memory access"
)
(assert_trap
  (invoke "load" (i32.const 0xffff_fffe))
  "out of bounds memory access"
)
(assert_trap
  (invoke "store" (i32.const 0xffff_ffff) (i32.const 0xd))
  "out of bounds memory access"
)
(assert_trap
  (invoke "load" (i32.const 0xffff_ffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "store" (i32.const 0x0) (i32.const 0xd))
  "out of bounds memory access"
)
(assert_trap (invoke "load" (i32.const 0x0)) "out of bounds memory access")
(assert_trap
  (invoke "store" (i32.const 0x8000_0000) (i32.const 0xd))
  "out of bounds memory access"
)
(assert_trap
  (invoke "load" (i32.const 0x8000_0000))
  "out of bounds memory access"
)
(assert_return
  (invoke "memory.grow" (i32.const 0x1_0001))
  (i32.const 0xffff_ffff)
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\01\a9\80\80\80\00\08\60"
  "\01\7f\01\7f\60\01\7f\01\7e\60\01\7f\01\7d\60\01"
  "\7f\01\7c\60\02\7f\7f\00\60\02\7f\7e\00\60\02\7f"
  "\7d\00\60\02\7f\7c\00\03\98\80\80\80\00\17\00\01"
  "\02\03\00\00\00\00\01\01\01\01\01\01\04\05\06\07"
  "\04\04\05\05\05\05\83\80\80\80\00\01\00\01\07\b3"
  "\82\80\80\00\17\08\69\33\32\2e\6c\6f\61\64\00\00"
  "\08\69\36\34\2e\6c\6f\61\64\00\01\08\66\33\32\2e"
  "\6c\6f\61\64\00\02\08\66\36\34\2e\6c\6f\61\64\00"
  "\03\0b\69\33\32\2e\6c\6f\61\64\38\5f\73\00\04\0b"
  "\69\33\32\2e\6c\6f\61\64\38\5f\75\00\05\0c\69\33"
  "\32\2e\6c\6f\61\64\31\36\5f\73\00\06\0c\69\33\32"
  "\2e\6c\6f\61\64\31\36\5f\75\00\07\0b\69\36\34\2e"
  "\6c\6f\61\64\38\5f\73\00\08\0b\69\36\34\2e\6c\6f"
  "\61\64\38\5f\75\00\09\0c\69\36\34\2e\6c\6f\61\64"
  "\31\36\5f\73\00\0a\0c\69\36\34\2e\6c\6f\61\64\31"
  "\36\5f\75\00\0b\0c\69\36\34\2e\6c\6f\61\64\33\32"
  "\5f\73\00\0c\0c\69\36\34\2e\6c\6f\61\64\33\32\5f"
  "\75\00\0d\09\69\33\32\2e\73\74\6f\72\65\00\0e\09"
  "\69\36\34\2e\73\74\6f\72\65\00\0f\09\66\33\32\2e"
  "\73\74\6f\72\65\00\10\09\66\36\34\2e\73\74\6f\72"
  "\65\00\11\0a\69\33\32\2e\73\74\6f\72\65\38\00\12"
  "\0b\69\33\32\2e\73\74\6f\72\65\31\36\00\13\0a\69"
  "\36\34\2e\73\74\6f\72\65\38\00\14\0b\69\36\34\2e"
  "\73\74\6f\72\65\31\36\00\15\0b\69\36\34\2e\73\74"
  "\6f\72\65\33\32\00\16\0a\a7\82\80\80\00\17\87\80"
  "\80\80\00\00\20\00\28\02\00\0b\87\80\80\80\00\00"
  "\20\00\29\03\00\0b\87\80\80\80\00\00\20\00\2a\02"
  "\00\0b\87\80\80\80\00\00\20\00\2b\03\00\0b\87\80"
  "\80\80\00\00\20\00\2c\00\00\0b\87\80\80\80\00\00"
  "\20\00\2d\00\00\0b\87\80\80\80\00\00\20\00\2e\01"
  "\00\0b\87\80\80\80\00\00\20\00\2f\01\00\0b\87\80"
  "\80\80\00\00\20\00\30\00\00\0b\87\80\80\80\00\00"
  "\20\00\31\00\00\0b\87\80\80\80\00\00\20\00\32\01"
  "\00\0b\87\80\80\80\00\00\20\00\33\01\00\0b\87\80"
  "\80\80\00\00\20\00\34\02\00\0b\87\80\80\80\00\00"
  "\20\00\35\02\00\0b\89\80\80\80\00\00\20\00\20\01"
  "\36\02\00\0b\89\80\80\80\00\00\20\00\20\01\37\03"
  "\00\0b\89\80\80\80\00\00\20\00\20\01\38\02\00\0b"
  "\89\80\80\80\00\00\20\00\20\01\39\03\00\0b\89\80"
  "\80\80\00\00\20\00\20\01\3a\00\00\0b\89\80\80\80"
  "\00\00\20\00\20\01\3b\01\00\0b\89\80\80\80\00\00"
  "\20\00\20\01\3c\00\00\0b\89\80\80\80\00\00\20\00"
  "\20\01\3d\01\00\0b\89\80\80\80\00\00\20\00\20\01"
  "\3e\02\00\0b\0b\9d\80\80\80\00\02\00\41\00\0b\08"
  "\61\62\63\64\65\66\67\68\00\41\f8\ff\03\0b\08\61"
  "\62\63\64\65\66\67\68"
)
(module instance)
(assert_trap
  (invoke "i32.store" (i32.const 0x1_0000) (i32.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.store" (i32.const 0xffff) (i32.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.store" (i32.const 0xfffe) (i32.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.store" (i32.const 0xfffd) (i32.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.store" (i32.const 0xffff_ffff) (i32.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.store" (i32.const 0xffff_fffe) (i32.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.store" (i32.const 0xffff_fffd) (i32.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.store" (i32.const 0xffff_fffc) (i32.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store" (i32.const 0x1_0000) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store" (i32.const 0xffff) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store" (i32.const 0xfffe) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store" (i32.const 0xfffd) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store" (i32.const 0xfffc) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store" (i32.const 0xfffb) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store" (i32.const 0xfffa) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store" (i32.const 0xfff9) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store" (i32.const 0xffff_ffff) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store" (i32.const 0xffff_fffe) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store" (i32.const 0xffff_fffd) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store" (i32.const 0xffff_fffc) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store" (i32.const 0xffff_fffb) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store" (i32.const 0xffff_fffa) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store" (i32.const 0xffff_fff9) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store" (i32.const 0xffff_fff8) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f32.store" (i32.const 0x1_0000) (f32.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f32.store" (i32.const 0xffff) (f32.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f32.store" (i32.const 0xfffe) (f32.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f32.store" (i32.const 0xfffd) (f32.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f32.store" (i32.const 0xffff_ffff) (f32.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f32.store" (i32.const 0xffff_fffe) (f32.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f32.store" (i32.const 0xffff_fffd) (f32.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f32.store" (i32.const 0xffff_fffc) (f32.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.store" (i32.const 0x1_0000) (f64.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.store" (i32.const 0xffff) (f64.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.store" (i32.const 0xfffe) (f64.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.store" (i32.const 0xfffd) (f64.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.store" (i32.const 0xfffc) (f64.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.store" (i32.const 0xfffb) (f64.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.store" (i32.const 0xfffa) (f64.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.store" (i32.const 0xfff9) (f64.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.store" (i32.const 0xffff_ffff) (f64.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.store" (i32.const 0xffff_fffe) (f64.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.store" (i32.const 0xffff_fffd) (f64.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.store" (i32.const 0xffff_fffc) (f64.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.store" (i32.const 0xffff_fffb) (f64.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.store" (i32.const 0xffff_fffa) (f64.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.store" (i32.const 0xffff_fff9) (f64.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.store" (i32.const 0xffff_fff8) (f64.const 0x0p+0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.store8" (i32.const 0x1_0000) (i32.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.store8" (i32.const 0xffff_ffff) (i32.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.store16" (i32.const 0x1_0000) (i32.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.store16" (i32.const 0xffff) (i32.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.store16" (i32.const 0xffff_ffff) (i32.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.store16" (i32.const 0xffff_fffe) (i32.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store8" (i32.const 0x1_0000) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store8" (i32.const 0xffff_ffff) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store16" (i32.const 0x1_0000) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store16" (i32.const 0xffff) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store16" (i32.const 0xffff_ffff) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store16" (i32.const 0xffff_fffe) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store32" (i32.const 0x1_0000) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store32" (i32.const 0xffff) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store32" (i32.const 0xfffe) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store32" (i32.const 0xfffd) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store32" (i32.const 0xffff_ffff) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store32" (i32.const 0xffff_fffe) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store32" (i32.const 0xffff_fffd) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.store32" (i32.const 0xffff_fffc) (i64.const 0x0))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load" (i32.const 0x1_0000))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load" (i32.const 0xffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load" (i32.const 0xfffe))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load" (i32.const 0xfffd))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load" (i32.const 0xffff_ffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load" (i32.const 0xffff_fffe))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load" (i32.const 0xffff_fffd))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load" (i32.const 0xffff_fffc))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load" (i32.const 0x1_0000))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load" (i32.const 0xffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load" (i32.const 0xfffe))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load" (i32.const 0xfffd))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load" (i32.const 0xfffc))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load" (i32.const 0xfffb))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load" (i32.const 0xfffa))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load" (i32.const 0xfff9))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load" (i32.const 0xffff_ffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load" (i32.const 0xffff_fffe))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load" (i32.const 0xffff_fffd))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load" (i32.const 0xffff_fffc))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load" (i32.const 0xffff_fffb))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load" (i32.const 0xffff_fffa))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load" (i32.const 0xffff_fff9))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load" (i32.const 0xffff_fff8))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f32.load" (i32.const 0x1_0000))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f32.load" (i32.const 0xffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f32.load" (i32.const 0xfffe))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f32.load" (i32.const 0xfffd))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f32.load" (i32.const 0xffff_ffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f32.load" (i32.const 0xffff_fffe))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f32.load" (i32.const 0xffff_fffd))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f32.load" (i32.const 0xffff_fffc))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.load" (i32.const 0x1_0000))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.load" (i32.const 0xffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.load" (i32.const 0xfffe))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.load" (i32.const 0xfffd))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.load" (i32.const 0xfffc))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.load" (i32.const 0xfffb))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.load" (i32.const 0xfffa))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.load" (i32.const 0xfff9))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.load" (i32.const 0xffff_ffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.load" (i32.const 0xffff_fffe))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.load" (i32.const 0xffff_fffd))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.load" (i32.const 0xffff_fffc))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.load" (i32.const 0xffff_fffb))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.load" (i32.const 0xffff_fffa))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.load" (i32.const 0xffff_fff9))
  "out of bounds memory access"
)
(assert_trap
  (invoke "f64.load" (i32.const 0xffff_fff8))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load8_s" (i32.const 0x1_0000))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load8_s" (i32.const 0xffff_ffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load8_u" (i32.const 0x1_0000))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load8_u" (i32.const 0xffff_ffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load16_s" (i32.const 0x1_0000))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load16_s" (i32.const 0xffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load16_s" (i32.const 0xffff_ffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load16_s" (i32.const 0xffff_fffe))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load16_u" (i32.const 0x1_0000))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load16_u" (i32.const 0xffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load16_u" (i32.const 0xffff_ffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i32.load16_u" (i32.const 0xffff_fffe))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load8_s" (i32.const 0x1_0000))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load8_s" (i32.const 0xffff_ffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load8_u" (i32.const 0x1_0000))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load8_u" (i32.const 0xffff_ffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load16_s" (i32.const 0x1_0000))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load16_s" (i32.const 0xffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load16_s" (i32.const 0xffff_ffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load16_s" (i32.const 0xffff_fffe))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load16_u" (i32.const 0x1_0000))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load16_u" (i32.const 0xffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load16_u" (i32.const 0xffff_ffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load16_u" (i32.const 0xffff_fffe))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load32_s" (i32.const 0x1_0000))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load32_s" (i32.const 0xffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load32_s" (i32.const 0xfffe))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load32_s" (i32.const 0xfffd))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load32_s" (i32.const 0xffff_ffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load32_s" (i32.const 0xffff_fffe))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load32_s" (i32.const 0xffff_fffd))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load32_s" (i32.const 0xffff_fffc))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load32_u" (i32.const 0x1_0000))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load32_u" (i32.const 0xffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load32_u" (i32.const 0xfffe))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load32_u" (i32.const 0xfffd))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load32_u" (i32.const 0xffff_ffff))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load32_u" (i32.const 0xffff_fffe))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load32_u" (i32.const 0xffff_fffd))
  "out of bounds memory access"
)
(assert_trap
  (invoke "i64.load32_u" (i32.const 0xffff_fffc))
  "out of bounds memory access"
)
(assert_return
  (invoke "i64.load" (i32.const 0xfff8))
  (i64.const 0x6867_6665_6463_6261)
)
(assert_return
  (invoke "i64.load" (i32.const 0x0))
  (i64.const 0x6867_6665_6463_6261)
)
(assert_return (invoke "i64.store" (i32.const 0xfff8) (i64.const 0x0)))
(assert_trap
  (invoke "i32.store" (i32.const 0xfffd) (i32.const 0x1234_5678))
  "out of bounds memory access"
)
(assert_return (invoke "i32.load" (i32.const 0xfffc)) (i32.const 0x0))
(assert_trap
  (invoke "i64.store" (i32.const 0xfff9) (i64.const 0x1234_5678_90ab_cdef))
  "out of bounds memory access"
)
(assert_return (invoke "i64.load" (i32.const 0xfff8)) (i64.const 0x0))
(assert_trap
  (invoke "f32.store" (i32.const 0xfffd) (f32.const 0x1.2345_68p+28))
  "out of bounds memory access"
)
(assert_return (invoke "f32.load" (i32.const 0xfffc)) (f32.const 0x0p+0))
(assert_trap
  (invoke "f64.store" (i32.const 0xfff9) (f64.const 0x1.2345_6789_0abc_ep+60))
  "out of bounds memory access"
)
(assert_return (invoke "f64.load" (i32.const 0xfff8)) (f64.const 0x0p+0))
