# CVM++
## STACK-BASED VIRTUAL MACHINE & CUSTOM COMPILER

A custom scripting language built in C++ that compiles down to proprietary bytecode using a custom compiler, executed by a custom stack-based Virtual Machine all
built from scratch.

---

## OBJECTIVE

Understand how compilers work in the background, how the raw text is converted to instructions that the computer can actually execute.

---

## Project Structure

```
cvm++/
├── include/
│   ├── Token.h        # Token types and Token struct
│   ├── AST.h          # AST node definitions (expressions and statements)
│   ├── Lexer.h        # Lexer class declaration
│   ├── Parser.h       # Parser class declaration
│   ├── Compiler.h     # Compiler, Instruction, and Operand definitions
│   ├── Serializer.h   # Bytecode file serializer/deserializer
│   └── VM.h           # Virtual Machine class declaration
├── src/
│   ├── Lexer.cpp      # Tokenizes raw source into Token list
│   ├── Parser.cpp     # Builds AST using recursive descent parsing
│   ├── Compiler.cpp   # Walks AST and emits bytecode instructions
│   ├── VM.cpp         # Executes bytecode on a stack-based machine
│   └── main.cpp       # CLI entry point (compile / run / REPL)
└── scripts/
    ├── test1.cvm      # Arithmetic and variables
    ├── test2.cvm      # If/else and booleans
    ├── test3.cvm      # While loop
    └── test4.cvm      # User input
```

---

## Building

Requirements: **g++ with C++14 support** (GCC 5 or higher)

```bash
g++ -std=c++14 -Wall -Iinclude src/Lexer.cpp src/Parser.cpp src/Compiler.cpp src/VM.cpp src/main.cpp -o cvm
```

On Windows:
```bash
g++ -std=c++14 -Wall -Iinclude src/Lexer.cpp src/Parser.cpp src/Compiler.cpp src/VM.cpp src/main.cpp -o cvm.exe
```

---

## Usage

### Compile a `.cvm` file to bytecode
```bash
./cvm compile scripts/test1.cvm
# outputs: scripts/test1.bytecode
```

### Run a bytecode file
```bash
./cvm run scripts/test1.bytecode
```

### Compile and run a `.cvm` file directly
```bash
./cvm run scripts/test1.cvm
```

### View bytecode disassembly
```bash
./cvm compile scripts/test1.cvm --debug
```

### Interactive REPL
```bash
./cvm
```
Type `debug` inside the REPL to toggle bytecode disassembly. Type `exit` to quit.

---

## The CVM Language

### Variables
```
let x = 10;
let y = x + 5;
```

### Arithmetic
```
let result = (x + y) * 2 / 3 - 1;
```

### Booleans
```
let flag = true;
let check = (x == 10);
```

### If / Else
```
if (x > 5) {
    print(x);
} else {
    print(0);
}
```

### While Loop
```
let i = 1;
while (i <= 10) {
    print(i);
    i = i + 1;
}
```

### Print
```
print(x);
print(x + y);
```

### Input
```
input(x);   // reads an integer from the user into variable x
print(x);
```

### Comments
```
// this is a comment
```

---

## Supported Operators

| Operator | Description |
|----------|-------------|
| `+` `-` `*` `/` | Arithmetic |
| `==` `!=` | Equality |
| `<` `>` `<=` `>=` | Comparison |
| `=` | Assignment |

---

## How It Works

### Lexer
Scans the source string character by character and produces a flat list of `Token` objects. Handles keywords, identifiers, integers, booleans, operators, and comments.

### Parser
Consumes the token list using **recursive descent parsing** and builds an **Abstract Syntax Tree (AST)**. Operator precedence is handled naturally by the call hierarchy: `assignment → equality → comparison → term → factor → unary → primary`.

### Compiler
Walks the AST and emits a flat array of `Instruction` objects (bytecode). Control flow like `if/else` and `while` is handled using `JUMP` and `JUMP_IF_FALSE` opcodes with backpatching.

### Serializer
Writes compiled bytecode to a binary `.bytecode` file and reads it back. Format: 1-byte opcode + 1-byte operand type + operand bytes.

### Virtual Machine
Executes bytecode using a **stack-based execution loop**. Operands are pushed onto the stack, instructions pop and push values, and variables are stored in a hash map.

---

## Bytecode Instruction Set

| Opcode | Description |
|--------|-------------|
| `PUSH_INT` | Push integer constant onto stack |
| `PUSH_BOOL` | Push boolean constant onto stack |
| `LOAD` | Load variable value onto stack |
| `STORE` | Pop stack and store into variable |
| `ADD` `SUB` `MUL` `DIV` | Arithmetic operations |
| `EQ` `NEQ` `LT` `GT` `LTE` `GTE` | Comparison operations |
| `JUMP` | Unconditional jump to instruction index |
| `JUMP_IF_FALSE` | Pop stack, jump if value is false/zero |
| `PRINT` | Pop and print top of stack |
| `INPUT` | Read integer from user into variable |
| `POP` | Discard top of stack |
| `HALT` | Stop execution |

---

## Example — Bytecode Disassembly

Source (`test3.cvm`):
```
let i = 1;
while (i <= 5) {
    print(i);
    i = i + 1;
}
```

Disassembly (`--debug`):
```
── Bytecode Disassembly ─────────────────
   0  PUSH_INT        1
   1  STORE           i
   2  LOAD            i
   3  PUSH_INT        5
   4  LTE
   5  JUMP_IF_FALSE   15
   6  LOAD            i
   7  PRINT
   8  LOAD            i
   9  PUSH_INT        1
  10  ADD
  11  STORE           i
  12  LOAD            i
  13  POP
  14  JUMP            2
  15  HALT
─────────────────────────────────────────
```

---

## References

- [Crafting Interpreters by Robert Nystrom](https://craftinginterpreters.com) — the primary reference for this project's architecture
