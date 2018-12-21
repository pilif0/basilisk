#include <utility>

#include <utility>

#include <utility>

/** \file AST.h
 * Abstract Syntax Tree definitions
 *
 * \author Filip Smola
 */
#ifndef BASILISK_AST_H
#define BASILISK_AST_H

#include <vector>
#include <string>
#include <memory>

//TODO review usage of std::move once stable
//TODO add docs to constructors
//TODO should invariants (e.g. at least one definition in program) be checked here or in parser?
//! Abstract Syntax Tree definitions
namespace basilisk::ast {
    /** \addtogroup AST
     * \brief Abstract Syntax Tree definitions
     *
     * Definitions of nodes of the Abstract Syntax Tree.
     *
     * @{
     */

    //TODO: extract a base class

    //! Identifiers are strings
    typedef std::string Identifier;


    /** \class Expression
     * \brief Base class for expression nodes
     */
    class Expression {
        protected:
            Expression() = default;
    };

    //! Various types of expressions
    // Note: all operators are right associative
    namespace expressions {
        // Expression levels
        /** \class Expression1
         * \brief Base class for Expression1 and SumExpression and SubExpression
         */
        class Expression1 : public Expression {
            protected:
                Expression1() = default;
        };

        /** \class Expression2
         * \brief Base class for Expression2, MulExpression and DivExpression
         */
        class Expression2 : public Expression1 {
            protected:
                Expression2() = default;
        };

        /** \class Expression3
         * \brief Base class for Expression3 and NegExpression
         */
        class Expression3 : public Expression2 {
            protected:
                Expression3() = default;
        };

        /** \class Expression4
         * \brief Base class for DoubleLitExpression, ParExpression, IdentifierExpression and FuncExpression
         */
        class Expression4 : public Expression3 {
            protected:
                Expression4() = default;
        };

        // Base level
        /** \class ModExpression
         * \brief Expression of modulo operation `x mod m`
         */
        class ModExpression : public Expression {
            public:
                const std::unique_ptr<Expression1> x;
                const std::unique_ptr<Expression> m;

                ModExpression(std::unique_ptr<Expression1> x, std::unique_ptr<Expression> m)
                    : x(std::move(x)), m(std::move(m)) {}
        };

        // 1st level
        /** \class SumExpression
         * \brief Expression of sum operation `lhs + rhs`
         */
        class SumExpression : public Expression1 {
            public:
                const std::unique_ptr<Expression2> lhs;
                const std::unique_ptr<Expression1> rhs;

                SumExpression(std::unique_ptr<Expression2> lhs, std::unique_ptr<Expression1> rhs)
                    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
        };

        /** \class SubExpression
         * \brief Expression of subtract operation `lhs - rhs`
         */
        class SubExpression : public Expression1 {
            public:
                const std::unique_ptr<Expression2> lhs;
                const std::unique_ptr<Expression1> rhs;

                SubExpression(std::unique_ptr<Expression2> lhs, std::unique_ptr<Expression1> rhs)
                    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
        };

        // 2nd level
        /** \class MulExpression
         * \brief Expression of multiply operation `lhs * rhs`
         */
        class MulExpression : public Expression2 {
            public:
                const std::unique_ptr<Expression3> lhs;
                const std::unique_ptr<Expression2> rhs;

                MulExpression(std::unique_ptr<Expression3> lhs, std::unique_ptr<Expression2> rhs)
                    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
        };

        /** \class DivExpression
         * \brief Expression of divide operation `lhs / rhs`
         */
        class DivExpression : public Expression2 {
            public:
                const std::unique_ptr<Expression3> lhs;
                const std::unique_ptr<Expression2> rhs;

                DivExpression(std::unique_ptr<Expression3> lhs, std::unique_ptr<Expression2> rhs)
                    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
        };

        // 3rd level
        /** \class NegExpression
         * \brief Expression of negate operation `- x`
         */
        class NegExpression : public Expression3 {
            public:
                const std::unique_ptr<Expression3> x;

                explicit NegExpression(std::unique_ptr<Expression3> x)
                    : x(std::move(x)) {}
        };

        // 4th level
        /** \class DoubleLitExpression
         * \brief Expression of double literal (e.g. `3.14`)
         */
        class DoubleLitExpression : public Expression4 {
            public:
                //! Value of the literal
                const double value;

                explicit DoubleLitExpression(const double value)
                    : value(value) {}
        };

        /** \class ParExpression
         * \brief Expression in parentheses
         */
        class ParExpression : public Expression4 {
            public:
                //! Inner expression
                const std::unique_ptr<Expression> expression;

                explicit ParExpression(std::unique_ptr<Expression> expression)
                    : expression(std::move(expression)) {}
        };

        /** \class IdentifierExpression
         * \brief Expression of an identifier
         */
        class IdentifierExpression : public Expression4 {
            public:
                //! Contained identifier
                const Identifier identifier;

                explicit IdentifierExpression(Identifier identifier)
                    : identifier(std::move(identifier)) {}
        };

        /** \class FuncExpression
         * \brief Expression of a function call
         */
        class FuncExpression : public Expression4 {
            public:
                //! Function identifier
                const Identifier identifier;
                //! Supplied expressions for arguments
                const std::vector<std::unique_ptr<Expression>> arguments;

                FuncExpression(Identifier identifier, std::vector<std::unique_ptr<Expression>> arguments)
                        : identifier(std::move(identifier)), arguments(std::move(arguments)) {}
        };
    }

    /** \class Statement
     * \brief Base class for statements
     */
    class Statement {
        protected:
            Statement() = default;
    };

    /** \class ReturnStatement
     * \brief Return Statement node
     *
     * Return Statement node contains an expression whose value is returned from a function.
     */
    class ReturnStatement : public Statement {
        public:
            //! Returned expression
            const std::unique_ptr<Expression> expression;

            explicit ReturnStatement(std::unique_ptr<Expression> expression)
                : expression(std::move(expression)) {}
    };

    /** \class StandaloneStatement
     * \brief Standalone Statement node
     *
     * Standalone Statement node contains an expression that is evaluated and its value is then discarded.
     */
     //TODO better name? VoidStatement?
    class StandaloneStatement : public Statement {
        public:
            const std::unique_ptr<Expression> expression;

            explicit StandaloneStatement(std::unique_ptr<Expression> expression)
                : expression(std::move(expression)) {}
    };

    /** \class Definition
     * \brief Base class for both function and variable definitions
     */
    class Definition {
        protected:
            Definition() = default;
    };

    /** \class FunctionDefinition
     * \brief Function Definition node
     *
     * Function Definition node contains the function identifier, a set of arguments and a statement-block body.
     */
    class FunctionDefinition : public Definition {
        public:
            //! Function identifier
            const Identifier identifier;
            //! Arguments of the function in order of declaration
            const std::vector<Identifier> arguments;
            //! Non-empty block of statements acting as the function body
            const std::vector<Statement> body;

            FunctionDefinition(Identifier id, std::vector<Identifier> args, std::vector<Statement> body)
                : identifier(std::move(id)), arguments(std::move(args)), body(std::move(body)) {}
    };

    /** \class VariableDefinition
     * \brief Variable Definition node
     *
     * Variable Definition contains the variable identifier and the value expression.
     */
    class VariableDefinition : public Definition, public Statement {
        public:
            //! Variable identifier
            const Identifier identifier;
            //! Value expression
            const std::unique_ptr<Expression> value;

            VariableDefinition(Identifier id, std::unique_ptr<Expression> val)
                : identifier(std::move(id)), value(std::move(val)) {}
    };

    /** \class Program
     * \brief Program node
     *
     * Program node contains a set of one or more definitions.
     * Root node of the AST.
     */
    class Program {
        public:
            //! Pointers to definitions in this program in order of definition
            const std::vector<std::unique_ptr<Definition>> definitions;

            //TODO make variadic?
            explicit Program(std::vector<std::unique_ptr<Definition>> defs)
                : definitions(std::move(defs)) {}
    };

    /**
     * @}
     */
}

#endif //BASILISK_AST_H
