# Basilisk Grammar
Basilisk grammar is built for a hand-written recursive descent parser.
The grammar as written here is supposed to be a description, not necessary an exact mirror of the implementation.

## Tokens
- `IDENTIFIER`
    - Letter character followed by alphanumeric characters or '_'
- `LPAR`, `RPAR`
    - Left and right parenthesis respectively
- `LBRAC`, `RBRAC`
    - Left and right bracket respectively
- `COMMA`
- `SEMICOLON`
- `ASSIGN`
    - Single equals sign
- `RETURN`
    - Keyword `return`
- `DOUBLE\_LITERAL`
    - At least one digit, followed by a decimal point and then at least one digit
- `PLUS`
- `MINUS`
- `STAR`
- `SLASH`
- `PERCENT`
- `END`
    - End of input
- `ERROR`
    - Lexing error

## Grammar
Non-terminals are in lower-case, terminals are in upper-case.
The terminal `EPSILON` refers to no token.

```
program := END
         | definition program

definition := definition-func
            | definition-var

definition-func := IDENTIFIER LPAR RPAR LBRAC statement-block RBRAC
                 | IDENTIFIER LPAR identifier-list RPAR LBRAC statement-block RBRAC

identifier-list := IDENTIFIER
                 | IDENTIFIER COMMA identifier-list

statement-block := EPSILON
                 | statement
                 | statement statement-block

statement := definition-var
           | expression SEMICOLON
           | RETURN expression SEMICOLON

definition-var := IDENTIFIER ASSIGN expression SEMICOLON

expression := expr1 PERCENT expression
            | expr1

expr1 := expr2 PLUS expr1
       | expr2 MINUS expr1 | expr2

expr2 := expr3 STAR expr2
       | expr3 SLASH expr2 | expr3

expr3 := MINUS expr3
       | expr4

expr4 := literal
       | LPAR expression RPAR
       | IDENTIFIER
       | function-call

literal := DOUBLE_LITERAL

function-call := IDENTIFIER LPAR RPAR
               | IDENTIFIER LPAR expression-list RPAR

expression-list := expression
                 | expression COMMA expression-list
```
