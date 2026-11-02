# WLP4 compiler demo

From this directory, run:

```bash
bash test.sh demo.wlp4
```

For local compilation to assembly without the university runtime tools:

```bash
bash test.sh --compile-only demo.wlp4
```

The script builds your lexical-analysis scanner, syntax-analysis parser (with the grammar header in
`semantic-analysis/p3`), the complete type checker in `semantic-analysis/testing`, and the procedures-and-runtime/p5 code generator.
Scanning is the tokenization step. It writes tokens, parse trees, typed trees,
assembly, and execution logs under `demo-output` beside each input file.
It finds the assignment sources relative to the script, so it can be launched
from any directory; input paths are relative to your current directory.

Local execution automatically uses `uwmips/target/release/uwmips` when built.
The included `mips_tools.py` assembles the CS241 MIPS instructions and links
the existing `print.merl` and `alloc.merl` libraries into plain machine code.
This path requires Python 3 and does not depend on the current ARM64 course tools.
The local emulator has been patched to place array input after program code
(where the allocation library expects it) and return failure on execution errors.
After changing its Rust source, rebuild with `cd uwmips && cargo build --release`.
Reference compiler comparison is skipped for local execution.

Without the local emulator, execution requires the legacy MIPS course commands `cs241.linkasm`, `cs241.linker`,
and `mips.array` on PATH, plus the existing libraries in
`procedures-and-runtime/p5`. The script links `alloc.merl` last and runs the linked MERL directly,
as in the existing procedures-and-runtime test scripts. It compares standard output and the return
register against `wlp4c` when that optional reference compiler is available;
otherwise it runs your program and reports that comparison was skipped.

Default input is an array of length 5 containing 0, 2, 4, 6, 8. The demo should
print 120 and 165 and return 45 if allocation succeeds.

Custom array input:

```bash
bash test.sh --input '3 1 2 3' demo.wlp4
```

For a program whose `wain` takes two integers:

```bash
bash test.sh --runner twoints --input '4 7' path/to/program.wlp4
```
