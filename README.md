# VECTx

A small statically typed scripting language with dynamic support, written in C++17.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

VECTx is a work in progress. It has variables, arithmetic, comparison, logical
operators, reassignment, `if` statements and a `conout()` builtin — enough to
write small branching programs. See [Not implemented yet](#not-implemented-yet)
for what is missing.

## Reason
Why did I make VECTx? I don't know either. "Why not?", I probably thought, before spending 3 hours 
understanding what even the error means. It literally has no point (apart from being faster than python). 
If you seriously plan to use this unfinished *thing*, I recommend seeing a Therapist. I will implement cool features in 
it though. So stay tuned.

## Build

Requires CMake 3.16+ and a C++17 compiler.

```sh
cmake -B build -S .
cmake --build build
./build/bin/vectx
```

Reconfigure to change the build type:

```sh
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Or compile directly, without CMake:

```sh
g++ -std=c++17 -o vectx src/*.cpp
./vectx
```

## Running

`main()` currently executes a hardcoded demo program, so there is nothing to
pass on the command line yet. The output of the demo is:

```
5
15
10
3.14
Pineapples
true
five
true
false
greeting is a string
true
7.5
true
greater than seven
5
between zero and ten
true
false
int, or counter is five
```

## Tests

```sh
cmake -B build -S .
cmake --build build
ctest --test-dir build --output-on-failure
```

Or run the test binary directly for per-check output:

```sh
./build/vectx_tests
```

63 checks covering if statements, type checks, logical operators, reassignment,
conditions, scoping, syntax errors and arithmetic.

## Language

### Values

`int`, `float`, `string`, `bool`, and a null value produced by builtins that
return nothing.

### Variables

```
var <type>: <name> = <initializer>;
```

```vectx
var int: count = 5;
var float: pi = 3.14;
var string: greeting = "Pineapples";
var bool: truthy = true;
```

The declared type is checked at the point of declaration, and again on every
later [reassignment](#reassignment). A mismatch raises a `Type mismatch` error.

### `any`

`any` is the dynamic type. It applies no static constraint, so the initializer
may be of any type and the variable keeps whatever runtime type the initializer
produced:

```vectx
var any: value = 5;              // runtime type: int
var any: computed = value * 3;   // runtime type: int
var any: ratio = computed / 2;   // runtime type: float
```

Because the type is not known statically, it can be inspected at runtime with a
type check. An `any` variable is also the only kind that can change what it
holds after declaration, since it has no declared type to hold it to.

### Type checks

```
<expr> is(type: <type>)
```

Evaluates to a boolean, and works with `any` variables as well as concrete
ones. `is` is a postfix operator, so it binds tighter than every binary
operator: `a + b is(type: int)` parses as `a + (b is(type: int))`.

```vectx
var any: value = 5;
conout(value is(type: int));    // true
conout(value is(type: float));  // false
conout(value is(type: any));    // true
```

`is(type: any)` is true for any value that carries data. It is false for a
value with no data — such as the result of a builtin that returns nothing —
which has no type of its own to report.

Because `is` binds tighter than every other operator, be careful mixing it with
a comparison. `a == b is(type: int)` means `a == (b is(type: int))`, which
compares a value against a boolean and so is false. Wrap the comparison in
parentheses, or check the type in a separate statement.

### Reassignment

```
<name> = <value>;
```

Assignment writes a new value into an existing variable. It is a statement, not
an expression, so it cannot be chained like `a = b = 1`. The name must already
exist; assigning to an undefined one raises `Undefined variable`.

A concretely typed variable keeps its type, so the new value has to match it or
the assignment raises `Type mismatch`. An `any` variable imposes no such
constraint, which is how one changes what it holds:

```vectx
var any: value = 5;
value = "five";               // allowed: any takes a value of any type
conout(value is(type: string));   // true

var int: count = 1;
count = count + 4;            // still an int
count = "many";               // Type mismatch in assignment to 'count'
```

`==` remains a comparison, so `count == 1` is a test and not an assignment.

### Logical operators

```
<expr> && <expr>
<expr> || <expr>
```

Both operands **must be a bool**, exactly like an `if` condition. There is no
truthiness: `1 && true` is an error that reports what it got. `&&` binds tighter
than `||`, and both bind looser than every comparison, so `a || b && c` means
`a || (b && c)`. Use parentheses when in doubt.

This is what lets a condition combine a comparison with a type check:

```vectx
var any: value = 5;

if (value is(type: int) && value > 3) {
    conout("a big int");
}
```

Both short-circuit, so the right operand is not evaluated when the left one
already decides the result. That is how a check can guard an expression that
would otherwise fail:

```vectx
var any: divisor = 0;

if (divisor is(type: int) && divisor != 0) {
    conout(100 / divisor);    // never runs
}
```

### If statements

```
if ( <condition> ) <then> [ else <otherwise> ]
```

The condition **must be a bool**. There is no truthiness: `if (1)` is an error
that reports what it got, not something that quietly means `true`.

```vectx
var int: number = 5;

if (number == 5) {
    conout("five");
} else {
    conout("not five");
}
```

`else if` chains work as expected:

```vectx
var any: label = "Pineapples";

if (label is(type: int)) {
    conout("an int");
} else if (label is(type: string)) {
    conout("a string");
} else {
    conout("something else");
}
```

Type checks return a bool, so they drop straight into a condition — this is the
main way to inspect an `any` value. Where a condition needs more than one test,
combine them with [`&&` and `||`](#logical-operators).

Braces are optional around a single statement, as in C++:

```vectx
if (true) conout("one statement");
```

Blocks may nest to any depth.

### Scoping

Each braced block is a scope. A variable declared inside one is gone once the
block ends, and an inner declaration of the same name shadows the outer one
without changing it.

```vectx
var int: x = 1;

if (true) {
    var int: x = 99;   // shadows
    conout(x);         // 99
}

conout(x);             // 1, the outer x is untouched
```

A variable declared outside is visible inside, so blocks can read, shadow and
reassign what came before them — see
[Reassignment in blocks](#reassignment-in-blocks).

If a statement throws part way through a block, that block's scope is still
discarded rather than left behind.

### Arithmetic

`int op int` stays an `int`, so arithmetic can be used directly in an `int`
declaration:

```vectx
var int: a = 5 + 3;   // 8, an int
var int: b = 5 * 3;   // 15, an int
```

Division is the exception: it always widens to `float`, because `5 / 2` has no
integer answer.

```vectx
var float: c = 7 / 2;   // 3.5
```

Any operation involving a `float` produces a `float`. Results that fall outside
the range of an `int` raise an `Integer overflow` error rather than wrapping.

### Operators

| Operators   | Notes                                       |
| ----------- | ------------------------------------------- |
| `+ - * /`   | arithmetic; numeric operands only           |
| `== !=`     | equality; differing types are never equal   |
| `< <= > >=` | comparison; numeric operands only           |
| `&& \|\|`   | logical; bool operands only, short-circuits |
| `is(type:)` | type check; postfix, binds tightest         |
| `=`         | assignment; a statement, not an expression  |
| `( )`       | grouping                                    |

Loosest to tightest: `||`, `&&`, `== !=`, `< <= > >=`, `+ -`, `* /`, `is`.

### Reassignment in blocks

Assignment resolves the name the same way a read does, so a block that did not
shadow the name writes to the variable outside it. That is what makes a variable
a counter or an accumulator:

```vectx
var int: total = 0;

if (true) {
    total = total + 5;
}

conout(total);   // 5
```

### Builtins

`conout(value)` prints a single value followed by a newline. It is currently
the only builtin, and it requires exactly one argument.

### Reserved words

`var`, `if`, `else`, `int`, `float`, `string`, `bool`, `any`, `is`, `type`,
`true`, `false`

These cannot be used as identifiers.

## Not implemented yet

Known gaps, so you do not have to go looking for them:

- **No comments.** Neither `//` nor `/* */`.
- **No unary minus.** Write `0 - 5`; `-5` is a parse error.
- **No logical negation.** `!` and `not` do not exist; write
  `x == false`.
- **No compound assignment.** `+=` and friends do not exist; write `x = x + 1`.
- **No loops.** `while` and `for` do not exist.
- **No functions.**
- **No string escapes.** `"\n"` prints a literal backslash-n.
- **No string concatenation.** `+` only works on numbers.
- **No `null` literal.** `null` parses as an undefined variable name.
- **No file input.** The demo program is hardcoded in `src/main.cpp`.

## Layout

```
CMakeLists.txt
LICENSE
src/
  Token.hpp         token types
  Lexer.{hpp,cpp}   source text -> tokens
  AST.hpp           expression and statement nodes
  Parser.{hpp,cpp}  tokens -> AST
  Value.{hpp,cpp}   runtime value (a std::variant)
  Environment.{hpp,cpp}  variable storage, as a stack of scopes
  Interpreter.{hpp,cpp}  AST -> execution
  main.cpp          entry point and demo program
tests/
  test_language.cpp language, scoping, logical and assignment tests
```

## License

MIT — see [LICENSE](LICENSE).

## Roadmap

- Functions, with `def func name(args) -> ret_glob { return v }` returning through a
  global return variable
- Loops
- Comments
- Unary minus
- Logical negation, `!`
- Compound assignment, `+=` and friends
- File input, so scripts can be run from disk
