# Basilisk Language Description
Basilisk is a C-based procedural programming language.
A basilisk program is a set of variable and function definitions.
Whitespace is ignored.
The entry point of the program is the function `main()`.

## Variable Definition
The data type for all variables is a double for all variables.
A variable is either local (when defined inside a function) or global (when defined outside any function).
A variable definition follows the form _identifier_ `=` _expression_ `;`.
The identifier then assumes the value of the expression on the RHS.
When a local variable is defined with the same identifier as a global one, it overshadow the global one for the duration of the function scope.
When a local variable is defined with the same identifier as an already existing one, the value is overwritten.
A function argument is considered a local variable.

## Function Definition
The data type returned by each function, as well as of each argument, is a double.
A function definition follows the form _identifier_ `(` _identifier-list_ `) {` _statement-block_ `}`.
The statements in the function's body get executed in sequence and a value is returned by the `return` statement.

## Expression
An expression is built up from parenthised expressions, literals, identifier and function calls combined using various mathematical operations, that can be evaluated.
The supported mathematical operations in order of precedence are:
- negation (`- i`),
- multiplication (`a * b`),
- division (`a / b`),
- addition (`a + b`),
- subtraction (`a - b`),
- modulo (`p % m`).
Parentheses can also be used to group expressions.
Parenthised expressions, literals, identifiers and function calls are treated as at the base of precedence.
Any function called needs to be defined before the call is made.

## Statement
A statement is either an assignment to a variable, an expression whose value gets discarded (usually a function call), or a return statement.
Every statement is ended by a semicolon.

### Return Statement
A return statement immediately ends the execution of a function, returning the provided value.
For example `return 3.14;`.

# Standard Library
There is a standard library of functions that are accessible from basilisk.

## Print Line - `println(x)`
This function prints the argument into standard input followed by a new line.

# Valid Example Program
This program is a valid basilisk program for illustration.

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
    pi = 3;
    write(pi);
    write(1 + (3 * 4) % 5);
    return 0;
}
```

Output:
```
3.14
3
3
```
