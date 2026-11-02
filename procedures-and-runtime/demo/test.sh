#!/usr/bin/env bash
# Build and run the project's complete WLP4 pipeline from any directory.
set -euo pipefail

# This script lives in wlp4-compiler/procedures-and-runtime/demo; the compiler modules are two levels up.
ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
for source in lexical-analysis/p3/wlp4scan.cc syntax-analysis/p5/wlp4parse.cc semantic-analysis/testing/wlp4type.cc procedures-and-runtime/p5/wlp4gen.cc; do
    if [[ ! -f "$ROOT/$source" ]]; then
        echo "Missing compiler source: $ROOT/$source" >&2
        echo 'Keep this script in wlp4-compiler/procedures-and-runtime/demo and include all compiler module directories.' >&2
        exit 1
    fi
done
COMPILE_ONLY=0
RUNNER=mips.array
RUN_INPUT='5 0 2 4 6 8'
FILES=()
usage() {
    echo "Usage: bash test.sh [--compile-only] [--runner array|twoints] [--input 'values'] file.wlp4 [...]"
    echo "Default: array input with length 5 and values 0, 2, 4, 6, 8."
}
while [[ $# -gt 0 ]]; do
    case "$1" in
        --compile-only) COMPILE_ONLY=1; shift ;;
        --runner)
            [[ $# -ge 2 ]] || { usage; exit 1; }
            case "$2" in
                array|twoints) RUNNER="mips.$2" ;;
                *) usage; exit 1 ;;
            esac
            shift 2 ;;
        --input)
            [[ $# -ge 2 ]] || { usage; exit 1; }
            RUN_INPUT=$2; shift 2 ;;
        --help|-h) usage; exit 0 ;;
        --*) usage; exit 1 ;;
        *) FILES+=("$1"); shift ;;
    esac
done
[[ ${#FILES[@]} -gt 0 ]] || { usage; exit 1; }

BUILD=$(mktemp -d "${TMPDIR:-/tmp}/wlp4-demo.XXXXXX")
trap 'rm -rf "$BUILD"' EXIT
echo 'Building your scanner, parser, type checker, and code generator...'
"${CXX:-g++}" -std=c++17 "$ROOT/lexical-analysis/p3/wlp4scan.cc" -o "$BUILD/wlp4scan"
"${CXX:-g++}" -std=c++17 -I "$ROOT/semantic-analysis/p3" "$ROOT/syntax-analysis/p5/wlp4parse.cc" -o "$BUILD/wlp4parse"
"${CXX:-g++}" -std=c++17 "$ROOT/semantic-analysis/testing/wlp4type.cc" -o "$BUILD/wlp4type"
"${CXX:-g++}" -std=c++17 "$ROOT/procedures-and-runtime/p5/wlp4gen.cc" -o "$BUILD/wlp4gen"

# Some student stages report errors but return zero. Check diagnostics too.
stage() {
    local label=$1 executable=$2 source=$3 output=$4
    echo "$label"
    if ! "$executable" < "$source" > "$output" 2> "$output.err"; then
        cat "$output.err" >&2
        echo "FAILED: $label" >&2
        return 1
    fi
    if rg_error "$output.err" || rg_error "$output" || [[ ! -s "$output" ]]; then
        cat "$output.err" >&2
        echo "FAILED: $label (error reported or empty output)" >&2
        return 1
    fi
}
rg_error() { grep -q 'ERROR' "$1"; }

LOCAL_EMULATOR="$ROOT/procedures-and-runtime/demo/uwmips/target/release/uwmips"
run_local_mips() {
    # uwmips reads one input value per line.
    tr ' \t' '\n\n' | sed '/^$/d' | "$LOCAL_EMULATOR" "${RUNNER#mips.}" "$1"
}

test_file() {
    local source=$1 dir name out tool actual_reg expected_reg
    [[ -f "$source" ]] || { echo "File not found: $source" >&2; return 1; }
    dir=$(cd "$(dirname "$source")" && pwd)
    name=$(basename "$source")
    # Preserve any existing assignment artifacts by using a dedicated directory.
    out="$dir/demo-output/${name%.wlp4}"
    mkdir -p "$(dirname "$out")"
    echo "=== Testing: $source ==="
    stage '1. Scanning / tokenizing WLP4' "$BUILD/wlp4scan" "$source" "$out.tokens" || return 1
    stage '2. Parsing tokens' "$BUILD/wlp4parse" "$out.tokens" "$out.wlp4i" || return 1
    stage '3. Type checking' "$BUILD/wlp4type" "$out.wlp4i" "$out.wlp4ti" || return 1
    stage '4. Generating MIPS assembly' "$BUILD/wlp4gen" "$out.wlp4ti" "$out.asm" || return 1
    echo "Assembly created: $out.asm"
    if [[ "$COMPILE_ONLY" -eq 1 ]]; then
        echo 'Compilation to assembly passed; execution was not requested.'
        return 0
    fi
    if [[ ! -x "$LOCAL_EMULATOR" ]]; then
        # Noninteractive shells may not have the course environment enabled.
        for tool in cs241.linkasm cs241.linker "$RUNNER"; do
            if ! command -v "$tool" > /dev/null 2>&1 && [[ -r /u/cs241/setup ]]; then
                # Course setup scripts may reference unset environment variables.
                set +u
                source /u/cs241/setup
                set -u
                break
            fi
        done
        for tool in cs241.linkasm cs241.linker "$RUNNER"; do
            if ! command -v "$tool" > /dev/null 2>&1; then
                echo "Missing course tool: $tool. Assembly generation succeeded." >&2
                echo 'Try: source /u/cs241/setup' >&2
                echo 'Use --compile-only if you only want assembly output.' >&2
                return 1
            fi
        done
    else
        command -v python3 > /dev/null 2>&1 || { echo 'Missing local tool: python3' >&2; return 1; }
    fi
    for tool in print.merl alloc.merl; do
        [[ -f "$ROOT/procedures-and-runtime/p5/$tool" ]] || { echo "Missing library: procedures-and-runtime/p5/$tool" >&2; return 1; }
    done
    if [[ -x "$LOCAL_EMULATOR" ]]; then
        echo '5–6. Assembling MIPS and linking runtime libraries locally'
        if ! python3 "$ROOT/procedures-and-runtime/demo/mips_tools.py" "$out.asm" "$ROOT/procedures-and-runtime/p5/print.merl" "$ROOT/procedures-and-runtime/p5/alloc.merl" > "$out.mips" 2> "$out.link.err"; then
            cat "$out.link.err" >&2
            return 1
        fi
    else
        stage '5. Assembling' cs241.linkasm "$out.asm" "$out.merl" || return 1
        echo '6. Linking runtime libraries'
        cs241.linker "$out.merl" "$ROOT/procedures-and-runtime/p5/print.merl" "$ROOT/procedures-and-runtime/p5/alloc.merl" > "$out.linked.merl" || return 1
    fi
    echo "7. Running your program with $RUNNER; input: $RUN_INPUT"
    if [[ -x "$LOCAL_EMULATOR" ]]; then
        printf '%s\n' "$RUN_INPUT" | run_local_mips "$out.mips" > "$out.actual.stdout" 2> "$out.actual.stderr" || { cat "$out.actual.stderr" >&2; return 1; }
    else
        printf '%s\n' "$RUN_INPUT" | "$RUNNER" "$out.linked.merl" > "$out.actual.stdout" 2> "$out.actual.stderr" || { cat "$out.actual.stderr" >&2; return 1; }
    fi
    cat "$out.actual.stdout" "$out.actual.stderr" > "$out.actual.log"
    cat "$out.actual.log"
    actual_reg=$(sed -n 's/.*\$03 = \([^ ]*\).*/\1/p' "$out.actual.log")
    if [[ -z "$actual_reg" ]]; then
        echo "FAILED: could not extract return register; inspect $out.actual.log" >&2
        return 1
    fi
    if [[ -x "$LOCAL_EMULATOR" ]] || ! command -v wlp4c > /dev/null 2>&1; then
        echo "Execution completed (return register: $actual_reg)."
        echo 'Reference comparison skipped for this execution backend.'
        return 0
    fi
    stage '8. Compiling with the course reference compiler' wlp4c "$source" "$out.expected.mips" || return 1
    printf '%s\n' "$RUN_INPUT" | "$RUNNER" "$out.expected.mips" > "$out.expected.stdout" 2> "$out.expected.stderr" || { cat "$out.expected.stderr" >&2; return 1; }
    cat "$out.expected.stdout" "$out.expected.stderr" > "$out.expected.log"
    expected_reg=$(sed -n 's/.*\$03 = \([^ ]*\).*/\1/p' "$out.expected.log")
    if [[ -z "$actual_reg" || -z "$expected_reg" ]]; then
        echo "FAILED: could not extract return registers; inspect $out.actual.log and $out.expected.log" >&2
        return 1
    fi
    if [[ "$actual_reg" != "$expected_reg" ]] || ! diff -u "$out.expected.stdout" "$out.actual.stdout"; then
        echo "FAILED: output or return value differs (yours: $actual_reg; reference: $expected_reg)" >&2
        return 1
    fi
    echo "PASS: standard output and return value match the reference ($actual_reg)."
}

STATUS=0
for source in "${FILES[@]}"; do
    if ! test_file "$source"; then STATUS=1; fi
done
exit "$STATUS"
