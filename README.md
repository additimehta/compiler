# WLP4 Compiler

A compiler project targeting MIPS, with implementations of lexical analysis, parsing, semantic analysis, and code generation. The repository also includes supporting automata, assembler, and runtime work.

## Project layout

| Directory | Contents |
| --- | --- |
| `mips-fundamentals/` | Machine instructions and introductory assembly programs. |
| `automata-and-lexing/` | Deterministic finite automata and maximal-munch scanning. |
| `mips-assembler/` | MIPS assembly routines and assembler implementation. |
| `lexical-analysis/` | WLP4 tokenization and sample programs. |
| `syntax-analysis/` | Context-free grammars, bottom-up parsing, and the WLP4 parser. |
| `semantic-analysis/` | Type checking and typed syntax trees. |
| `code-generation/` | Initial MIPS code generation. |
| `procedures-and-runtime/` | Procedure calls, memory allocation, runtime integration, and the complete compiler demo. |

## Run the demo

Requires Bash and a C++17 compiler. Compile the sample to MIPS assembly:

```bash
cd procedures-and-runtime/demo
bash test.sh --compile-only demo.wlp4
```

See [the demo guide](procedures-and-runtime/demo/README.md) for execution options and expected output. The numbered `p` subdirectories preserve the incremental implementations and their examples.
