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

//TODO add docs to constructors
//TODO determine whether definition order in program should matter
//! Abstract Syntax Tree definitions
namespace basilisk::ast {
    /** \addtogroup AST
     * \brief Abstract Syntax Tree definitions
     *
     * Definitions of nodes of the Abstract Syntax Tree.
     *
     * @{
     */

    /** \class Node
     * \brief Base class for all AST nodes
     */
    class Node {
        protected:
            Node() = default;
        public:
            /**
             * \brief Whether this node is equal to another
             *
             * \param other Node to compare to
             * \return Whether the nodes are equal
             */
            virtual bool equals(Node *other) = 0;
            /**
             * \brief Describe this node in one line
             *
             * \return One line string description
             */
            virtual std::string describe() = 0;
            /**
             * \brief Vector of pointers to child nodes of this node
             *
             * \return Vector of pointers to Node
             */
            virtual std::vector<Node*> children() = 0;

            virtual ~Node() = default;
    };

    //! Identifiers are strings
    typedef std::string Identifier;


    /** \class Expression
     * \brief Base class for expression nodes
     */
    class Expression : public Node {
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
        /** \class Modulo
         * \brief Expression of modulo (`x mod m`)
         */
        class Modulo : public Expression {
            public:
                std::unique_ptr<Expression1> x;
                std::unique_ptr<Expression> m;

                Modulo(std::unique_ptr<Expression1> x, std::unique_ptr<Expression> m)
                    : x(std::move(x)), m(std::move(m)) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };

        // 1st level
        /** \class Summation
         * \brief Expression of summation (`lhs + rhs`)
         */
        class Summation : public Expression1 {
            public:
                std::unique_ptr<Expression2> lhs;
                std::unique_ptr<Expression1> rhs;

                Summation(std::unique_ptr<Expression2> lhs, std::unique_ptr<Expression1> rhs)
                    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };

        /** \class Subtraction
         * \brief Expression of subtraction (`lhs - rhs`)
         */
        class Subtraction : public Expression1 {
            public:
                std::unique_ptr<Expression2> lhs;
                std::unique_ptr<Expression1> rhs;

                Subtraction(std::unique_ptr<Expression2> lhs, std::unique_ptr<Expression1> rhs)
                    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };

        // 2nd level
        /** \class Multiplication
         * \brief Expression of multiplication (`lhs * rhs`)
         */
        class Multiplication : public Expression2 {
            public:
                std::unique_ptr<Expression3> lhs;
                std::unique_ptr<Expression2> rhs;

                Multiplication(std::unique_ptr<Expression3> lhs, std::unique_ptr<Expression2> rhs)
                    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };

        /** \class Division
         * \brief Expression of division (`lhs / rhs`)
         */
        class Division : public Expression2 {
            public:
                std::unique_ptr<Expression3> lhs;
                std::unique_ptr<Expression2> rhs;

                Division(std::unique_ptr<Expression3> lhs, std::unique_ptr<Expression2> rhs)
                    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };

        // 3rd level
        /** \class NumericNegation
         * \brief Expression of numeric negation (`- x`)
         */
        class NumericNegation : public Expression3 {
            public:
                std::unique_ptr<Expression3> x;

                explicit NumericNegation(std::unique_ptr<Expression3> x)
                    : x(std::move(x)) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };

        // 4th level
        /** \class LiteralDouble
         * \brief Expression of double literal (e.g. `3.14`)
         */
        class LiteralDouble : public Expression4 {
            public:
                //! Value of the literal
                double value;

                explicit LiteralDouble(double value)
                    : value(value) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };

        /** \class Parenthesised
         * \brief Expression in parentheses (e.g. `(a + b)`)
         */
        class Parenthesised : public Expression4 {
            public:
                //! Inner expression
                std::unique_ptr<Expression> expression;

                explicit Parenthesised(std::unique_ptr<Expression> expression)
                    : expression(std::move(expression)) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };

        /** \class IdentifierExpression
         * \brief Expression of an identifier (e.g. `x`)
         */
        class IdentifierExpression : public Expression4 {
            public:
                //! Contained identifier
                Identifier identifier;

                explicit IdentifierExpression(Identifier identifier)
                    : identifier(std::move(identifier)) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };

        /** \class FunctionCall
         * \brief Expression of a function call (e.g. `f(...)`)
         */
        class FunctionCall : public Expression4 {
            public:
                //! Function identifier
                Identifier identifier;
                //! Supplied expressions for arguments
                std::vector<std::unique_ptr<Expression>> arguments;

                FunctionCall(Identifier identifier, std::vector<std::unique_ptr<Expression>> arguments)
                        : identifier(std::move(identifier)), arguments(std::move(arguments)) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };
    }

    /** \class Statement
     * \brief Base class for statements
     */
    class Statement : public Node {
        protected:
            Statement() = default;
    };

    //! Various types of statements
    namespace statements {
        /** \class Return
         * \brief Return Statement node
         *
         * Return Statement node contains an expression whose value is returned from a function.
         */
            class Return : public Statement {
                public:
                    //! Returned expression
                    std::unique_ptr<Expression> expression;

                    explicit Return(std::unique_ptr<Expression> expression)
                            : expression(std::move(expression)) {}

                    bool equals(Node *other) override;
                    std::string describe() override;
                    std::vector<Node *> children() override;
            };

            /** \class Standalone
             * \brief Standalone Statement node
             *
             * Standalone Statement node contains an expression that is evaluated and its value is then discarded.
             */
            //TODO better name? Void?
            class Standalone : public Statement {
                public:
                    std::unique_ptr<Expression> expression;

                    explicit Standalone(std::unique_ptr<Expression> expression)
                            : expression(std::move(expression)) {}

                    bool equals(Node *other) override;
                    std::string describe() override;
                    std::vector<Node *> children() override;
            };

            /** \class Variable
             * \brief Variable Statement node
             *
             * Variable Statement node contains the variable identifier and the value expression.
             */
             //TODO rename to assignment? might be more descriptive
            class Variable : public Statement {
                public:
                    //! Variable identifier
                    Identifier identifier;
                    //! Value expression
                    std::unique_ptr<Expression> value;

                    Variable(Identifier id, std::unique_ptr<Expression> val)
                            : identifier(std::move(id)), value(std::move(val)) {}

                    bool equals(Node *other) override;
                    std::string describe() override;
                    std::vector<Node *> children() override;
            };
    }

    /** \class Definition
     * \brief Base class for both function and variable definitions
     */
    class Definition : public Node {
        protected:
            Definition() = default;
    };

    //! Various types of definitions
    namespace definitions {
        /** \class Function
         * \brief Function Definition node
         *
         * Function Definition node contains the function identifier, a set of arguments and a statement-block body.
         */
        class Function : public Definition {
            public:
                //! Function identifier
                Identifier identifier;
                //! Arguments of the function in order of declaration
                std::vector<Identifier> arguments;
                //! Statements acting as the function body
                std::vector<std::unique_ptr<Statement>> body;

                Function(Identifier id, std::vector<Identifier> args, std::vector<std::unique_ptr<Statement>> body)
                        : identifier(std::move(id)), arguments(std::move(args)), body(std::move(body)) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };

        /** \class Variable
         * \brief Variable Definition node
         *
         * Variable Definition contains the variable statement.
         */
        class Variable : public Definition {
            public:
                std::unique_ptr<statements::Variable> statement;

                explicit Variable(std::unique_ptr<statements::Variable> statement)
                        : statement(std::move(statement)) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };
    }


    /** \class Program
     * \brief Program node
     *
     * Program node contains a set of definitions.
     * Root node of the AST.
     */
    class Program : public Node {
        public:
            //! Pointers to definitions in this program in order of definition
            std::vector<std::unique_ptr<Definition>> definitions;

            explicit Program(std::vector<std::unique_ptr<Definition>> defs)
                : definitions(std::move(defs)) {}

            bool equals(Node *other) override;
            std::string describe() override;
            std::vector<Node *> children() override;
    };

    /**
     * @}
     */
}

#endif //BASILISK_AST_H
