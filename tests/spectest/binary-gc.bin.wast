(assert_malformed
  (module binary "\00\61\73\6d\01\00\00\00\01\04\01\5e\78\02")
  "malformed mutability"
)
