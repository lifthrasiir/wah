#!/bin/sh
# Checks what WAH did with each module (its first command in the scripts of check_one.sh, kept in <outdir>/first)
# against the interpreter of SpecTec for `make spec-diff SPECTEC=...`, which runs the spec itself. Only validation
# and instantiation are checked, as running functions takes the interpreter too long. It skips assert_malformed,
# which become assert_invalid here: the interpreter decodes modules there, so decoding errors are malformed modules.
# Prints for each module:
#   pass      the interpreter agrees
#   category  WAH rejects the module as invalid, the interpreter as malformed
#   skip      the interpreter ran out of memory or stack (it keeps memories and tables as lists)
#   timeout   its batch took too long
#   fail      they disagree; the module and the output are kept in <outdir>/spectec
# Usage: check_spectec.sh <spectec directory, with spectec built> <outdir>
spectec=$1 dir=$2 out=$2/spectec
rm -rf "$out" && mkdir -p "$out"
i=0
for f in "$dir"/first/*.wast; do
    b=$out/$((i / 500))
    [ -d "$b" ] || mkdir "$b"
    sed 's/^(assert_malformed/(assert_invalid/' "$f" > "$b/$(basename "$f")"
    i=$((i + 1))
done
# One interpreter per batch, as loading the spec takes seconds
ls -d "$out"/*/ | sed 's:/$::' | xargs -P "$(getconf _NPROCESSORS_ONLN)" -I {} sh -c \
    'cd "$0" && perl -e "alarm 900; exec @ARGV" ./spectec ../specification/wasm-3.0/* --interpreter "$1" \
        > "$1.out" 2>&1; echo "$?" > "$1.status"' "$spectec" {}
for b in "$out"/*/; do
    b=${b%/}
    if [ "$(cat "$b.status")" != 0 ]; then
        for f in "$b"/*.wast; do echo "timeout $(basename "$f" .wast)"; done
        continue
    fi
    for f in "$b"/*.wast; do
        n=$(basename "$f")
        msg=$(grep -F -- "- Test failed at $f:" "$b.out" | head -1 | sed 's/^[^(]*(//; s/)$//')
        if [ -z "$msg" ]; then res=pass
        else
            case "$msg" in
                *OutOfMemory* | *"Stack overflow"*) res=skip ;;
                "Error(_,"*)
                    case "$(head -c 16 "$dir/first/$n")" in
                        "(assert_malform"*) res=pass ;;
                        "(assert_invalid"*) res=category ;;
                        *) res=fail ;;
                    esac ;;
                *) res=fail ;;
            esac
        fi
        echo "$res ${n%.wast}"
        if [ "$res" = fail ]; then
            cp "$dir/first/$n" "$out/$n"
            grep -F -A 20 -- "- Test failed at $f:" "$b.out" > "$out/$n.out"
        fi
    done
done
