# Basilisk Language Description
Basilisk is a C-based procedural programming language.
A basilisk program is a set of variable and function definitions.
The order of those definitions matters (a variable/function needs to be defined before it can be used).
Whitespace is ignored.
The entry point of the program is the function `main()`.

## Definition
Definitions are the basic building blocks of basilisk programs.
Functions can be defined only in the root context of the program (i.e. not inside another function).
Variables can be defined in the root context of the program - becoming global - or inside a function - becoming local.

### Variable Definition
> _identifier_ `=` _expression_ `;`

The data type for all variables is a `double`.
When defined inside a function, the variable is __local__, otherwise it is __global__.
A function argument is considered a local variable.
There is currently no difference between a variable definition and assignment, therefore:

- When a local variable is defined with the same identifier as a global one, it overshadows the global one for the duration of the function scope.
- When a local variable is defined with the same identifier as an already existing one, the value is overwritten.
- When a global variable is defined with the same identifier as an already existing one, the value is overwritten.
    Due to how global variables are initialized, the last value assigned to the global variable is used for the __whole program__.

### Function Definition
> _identifier_ `(` _identifier-list_ `) {` _statement-block_ `}`

The data type returned by each function, as well as of each argument, is a double.
The statements in the function's body get executed in sequence and a value is returned by the return statement.
A function with no return statement returns the value `0.0`.

## Statement
A statement can be understood as a command of the program (e.g. return statement), which has no value.
These statements form the bodies of functions.
Every statement is ended by a semicolon.

### Variable Assignment
> _identifier_ `=` _expression_ `;`

The variable assignment statement is currently the same as a variable definition, with the same behaviour.
The only difference is that a variable assignment statement can't be outside of a function.

### Discard Statement
> _expression_ `;`

A discard statement evaluates an expression and immediately discards the resulting value.
This is used to execute functions whose return value you don't care about.

### Return Statement
> `return` _expression_ `;`

A return statement immediately ends the execution of a function, returning the provided value.

## Expression
An expression is a tree of operations that has a value.
Basilisk supports expressions made up of the following operations, in order of increasing precedence:

- Precedence 0:
    - Function call - `f(` _expression-list_ `)` where _expression-list_ is a comma separated list of top-level expressions
    - Identifier for a variable
    - Parenthesised expression - `(` _expression_ `)` where _expression_ is a top-level expression
    - Double literal
- Precedence 1:
    - Numeric negation - `-` _expression-0_
- Precedence 2:
    - Multiplication - _expression-1_ `*` _expression-2_
    - Division - _expression-1_ `/` _expression-2_
- Precedence 3:
    - Addition - _expression-2_ `+` _expression-3_
    - Subtraction - _expression-2_ `-` _expression-3_
- Precedence 4:
    - Modulo - _expression-3_ `%` _expression-4_

where _expression-n_ is an expression with precedence level _n_ or lower and all binary operators are right associative.

# Standard Library
There is a standard library of functions that are accessible from basilisk.

## Print Line - `println(x)`
This function prints the argument into standard input followed by a new line.

# Valid Example Program
This program is a valid basilisk program for illustration.
For a full set of example programs, see contents of [examples](examples/).

Source:
```
pi = 3.14;

get_pi() {
    return pi;
}

write(x) {
    println(x);
}

main() {
    write(get_pi());
    pi = 3.0;
    write(pi);
    write(1.0 + (3.0 * 4.0) % 5.0);
    return 0.0;
}
```

Output:
```
3.14
3
3
```
