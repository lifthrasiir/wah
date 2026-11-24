#!/bin/sh
# Checks one module against the reference interpreter for `make spec-diff`, printing the outcome and the module:
#   pass      the reference interpreter agrees with all that WAH did
#   category  both reject the module, but one as malformed and the other as invalid (not distinguished by WAH)
#   skip      WAH stopped at its limits or the module has imports
#   timeout   the reference interpreter took too long (e.g. growing a table by billions of elements)
#   nan       they disagree, but not with WAH keeping NaN payloads in conversions (nan_variant.sh)
#   fail      they disagree; the script and the output of the reference interpreter are kept in <outdir>
# Usage: check_one.sh <wah_to_wast> <wah_to_wast of nan_variant.sh> <reference interpreter> <outdir> <module>
gen=$1 nangen=$2 ref=$3 dir=$4 f=$5
w=$dir/$(basename "$f").wast
"$gen" "$f" "$w" || { echo "skip $f"; exit 0; }
perl -e 'alarm 20; exec @ARGV' "$ref" -b 100000 "$w" > "$w.out" 2>&1
rc=$?
if [ $rc -eq 0 ]; then
    echo "pass $f"; rm -f "$w" "$w.out"
elif [ $rc -eq 142 ]; then
    echo "timeout $f"; rm -f "$w" "$w.out"
elif head -c 16 "$w" | grep -q '^(assert_\(invalid\|malformed\)' && grep -q 'decoding error\|validation error' "$w.out"; then
    echo "category $f"; rm -f "$w" "$w.out"
elif "$nangen" "$f" "${w%.wast}.nan.wast" && perl -e 'alarm 20; exec @ARGV' "$ref" -b 100000 "${w%.wast}.nan.wast" > /dev/null 2>&1; then
    echo "nan $f"; rm -f "$w" "$w.out" "${w%.wast}.nan.wast"
else
    echo "fail $f"; rm -f "${w%.wast}.nan.wast"
fi
