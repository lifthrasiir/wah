(module definition binary
  "\00\61\73\6d\01\00\00\00\04\84\80\80\80\00\01\70"
  "\04\00"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\84\80\80\80\00\01\70"
  "\04\01"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\85\80\80\80\00\01\70"
  "\05\00\00"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\85\80\80\80\00\01\70"
  "\05\00\01"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\86\80\80\80\00\01\70"
  "\05\01\80\02"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\87\80\80\80\00\01\70"
  "\05\00\80\80\04"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\89\80\80\80\00\01\70"
  "\05\00\ff\ff\ff\ff\0f"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\89\80\80\80\00\01\70"
  "\05\00\80\80\80\80\10"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\8d\80\80\80\00\01\70"
  "\04\ff\ff\ff\ff\ff\ff\ff\ff\ff\01"
)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\8e\80\80\80\00\01\70"
  "\05\00\ff\ff\ff\ff\ff\ff\ff\ff\ff\01"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\04\87\80\80\80\00\02\70"
  "\04\00\70\04\00"
)
(module instance)
(module definition binary
  "\00\61\73\6d\01\00\00\00\02\96\80\80\80\00\01\08"
  "\73\70\65\63\74\65\73\74\07\74\61\62\6c\65\36\34"
  "\01\70\04\00\04\84\80\80\80\00\01\70\04\00"
)
(module instance)
(assert_invalid
  (module binary "\00\61\73\6d\01\00\00\00\04\85\80\80\80\00\01\70" "\05\01\00")
  "size minimum must not be greater than maximum"
)
(assert_invalid
  (module binary
    "\00\61\73\6d\01\00\00\00\04\89\80\80\80\00\01\70"
    "\05\ff\ff\ff\ff\0f\00"
  )
  "size minimum must not be greater than maximum"
)
