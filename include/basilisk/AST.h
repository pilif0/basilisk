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

    // Forward-define visitor base class
    class Visitor;

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
             * \brief Accept a visitor
             *
             * Accept the visitor, calling the appropriate visit method.
             *
             * \param visitor Visitor to accept
             */
            virtual void accept(Visitor &visitor);

            virtual ~Node() = default;
    };

    //! Identifiers are strings
    typedef std::string Identifier;


    /** \class Expression
     * \brief Base class for expression nodes
     *
     * Base class for all expression nodes.
     * Expressions (as opposed to statements) have a value.
     * This class represents the top level in the precedence hierarchy (ie. modulo).
     */
    class Expression : public Node {
        protected:
            Expression() = default;
        public:
            void accept(Visitor &visitor) override;
    };

    //! Various types of expressions
    // Note: all operators are right associative
    namespace expressions {
        // Expression levels
        /** \class Expression1
         * \brief Base class for first-level expressions
         *
         * Base class for first-level expressions, as well as all lower levels.
         * This class represents the first level in the precedence hierarchy (ie. summation, subtraction).
         */
        class Expression1 : public Expression {
            protected:
                Expression1() = default;
            public:
                void accept(Visitor &visitor) override;
        };

        /** \class Expression2
         * \brief Base class for second-level expressions
         *
         * Base class for second-level expressions, as well as all lower levels.
         * This class represents the second level in the precedence hierarchy (ie. multiplication, division).
         */
        class Expression2 : public Expression1 {
            protected:
                Expression2() = default;
            public:
                void accept(Visitor &visitor) override;
        };

        /** \class Expression3
         * \brief Base class for third-level expressions
         *
         * Base class for third-level expressions, as well as all lower levels.
         * This class represents the third level in the precedence hierarchy (ie. negation).
         */
        class Expression3 : public Expression2 {
            protected:
                Expression3() = default;
            public:
                void accept(Visitor &visitor) override;
        };

        /** \class Expression4
         * \brief Base class for fourth-level expressions
         *
         * Base class for fourth-level expressions, as well as all lower levels.
         * This class represents the fourth (bottom) level in the precedence hierarchy (ie. literals, identifiers,
         *  parenthesised expressions, function calls).
         */
        class Expression4 : public Expression3 {
            protected:
                Expression4() = default;
            public:
                void accept(Visitor &visitor) override;
        };

        // Base level
        /** \class Modulo
         * \brief Expression of modulo (`x mod m`)
         */
        class Modulo : public Expression {
            public:
                std::unique_ptr<Expression1> x;
                std::unique_ptr<Expression> m;

                /**
                 * \brief Construct a Modulo Expression node
                 *
                 * Construct a Modulo Expression node from the argument and divisor.
                 *
                 * \param x Pointer to argument expression
                 * \param m Pointer to divisor expression
                 */
                Modulo(std::unique_ptr<Expression1> x, std::unique_ptr<Expression> m)
                    : x(std::move(x)), m(std::move(m)) {}

                bool equals(Node *other) override;
                void accept(Visitor &visitor) override;
        };

        // 1st level
        /** \class Summation
         * \brief Expression of summation (`lhs + rhs`)
         */
        class Summation : public Expression1 {
            public:
                std::unique_ptr<Expression2> lhs;
                std::unique_ptr<Expression1> rhs;

                /**
                 * \brief Construct a Summation Expression node
                 *
                 * Construct a Summation Expression node from the left and right hand sides.
                 *
                 * \param lhs Pointer to left hand side expression
                 * \param rhs Pointer to right hand side expression
                 */
                Summation(std::unique_ptr<Expression2> lhs, std::unique_ptr<Expression1> rhs)
                    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

                bool equals(Node *other) override;
                void accept(Visitor &visitor) override;
        };

        /** \class Subtraction
         * \brief Expression of subtraction (`lhs - rhs`)
         */
        class Subtraction : public Expression1 {
            public:
                std::unique_ptr<Expression2> lhs;
                std::unique_ptr<Expression1> rhs;

                /**
                 * \brief Construct a Subtraction Expression node
                 *
                 * Construct a Subtraction Expression node from the left and right hand sides.
                 *
                 * \param lhs Pointer to left hand side expression
                 * \param rhs Pointer to right hand side expression
                 */
                Subtraction(std::unique_ptr<Expression2> lhs, std::unique_ptr<Expression1> rhs)
                    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

                bool equals(Node *other) override;
                void accept(Visitor &visitor) override;
        };

        // 2nd level
        /** \class Multiplication
         * \brief Expression of multiplication (`lhs * rhs`)
         */
        class Multiplication : public Expression2 {
            public:
                std::unique_ptr<Expression3> lhs;
                std::unique_ptr<Expression2> rhs;

                /**
                 * \brief Construct a Multiplication Expression node
                 *
                 * Construct a Multiplication Expression node from the left and right hand sides.
                 *
                 * \param lhs Pointer to left hand side expression
                 * \param rhs Pointer to right hand side expression
                 */
                Multiplication(std::unique_ptr<Expression3> lhs, std::unique_ptr<Expression2> rhs)
                    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

                bool equals(Node *other) override;
                void accept(Visitor &visitor) override;
        };

        /** \class Division
         * \brief Expression of division (`lhs / rhs`)
         */
        class Division : public Expression2 {
            public:
                std::unique_ptr<Expression3> lhs;
                std::unique_ptr<Expression2> rhs;

                /**
                 * \brief Construct a Division Expression node
                 *
                 * Construct a Division Expression node from the left and right hand sides.
                 *
                 * \param lhs Pointer to left hand side expression
                 * \param rhs Pointer to right hand side expression
                 */
                Division(std::unique_ptr<Expression3> lhs, std::unique_ptr<Expression2> rhs)
                    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

                bool equals(Node *other) override;
                void accept(Visitor &visitor) override;
        };

        // 3rd level
        /** \class NumericNegation
         * \brief Expression of numeric negation (`- x`)
         */
        class NumericNegation : public Expression3 {
            public:
                std::unique_ptr<Expression3> x;

                /**
                 * \brief Construct a Numeric Negation Expression node
                 *
                 * Construct a Numeric Negation Expression node from the negated expression.
                 *
                 * \param x Pointer to negated expression
                 */
                explicit NumericNegation(std::unique_ptr<Expression3> x)
                    : x(std::move(x)) {}

                bool equals(Node *other) override;
                void accept(Visitor &visitor) override;
        };

        // 4th level
        /** \class LiteralDouble
         * \brief Expression of double literal (e.g. `3.14`)
         */
        class LiteralDouble : public Expression4 {
            public:
                //! Value of the literal
                double value;

                /**
                 * \brief Construct a Double Literal Expression node
                 *
                 * Construct a Double Literal Expression node from a value.
                 *
                 * \param value Value
                 */
                explicit LiteralDouble(double value)
                    : value(value) {}

                bool equals(Node *other) override;
                void accept(Visitor &visitor) override;
        };

        /** \class Parenthesised
         * \brief Expression in parentheses (e.g. `(a + b)`)
         */
        class Parenthesised : public Expression4 {
            public:
                //! Inner expression
                std::unique_ptr<Expression> expression;

                /**
                 * \brief Construct a Parenthesised Expression node
                 *
                 * Construct a Parenthesised Expression node from the contained expression.
                 *
                 * \param expression Pointer to contained expression
                 */
                explicit Parenthesised(std::unique_ptr<Expression> expression)
                    : expression(std::move(expression)) {}

                bool equals(Node *other) override;
                void accept(Visitor &visitor) override;
        };

        /** \class IdentifierExpression
         * \brief Expression of an identifier (e.g. `x`)
         */
        class IdentifierExpression : public Expression4 {
            public:
                //! Contained identifier
                Identifier identifier;

                /**
                 * \brief Construct a Identifier Expression node
                 *
                 * Construct an Identifier Expression node from an identifier.
                 *
                 * \param identifier Identifier
                 */
                explicit IdentifierExpression(Identifier identifier)
                    : identifier(std::move(identifier)) {}

                bool equals(Node *other) override;
                void accept(Visitor &visitor) override;
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

                /**
                 * \brief Construct a Function Call Expression node
                 *
                 * Construct a Function Call Expression node from the function's identifier and the argument expressions.
                 *
                 * \param identifier Function identifier
                 * \param arguments Pointers to expressions for each argument
                 */
                FunctionCall(Identifier identifier, std::vector<std::unique_ptr<Expression>> arguments)
                        : identifier(std::move(identifier)), arguments(std::move(arguments)) {}

                bool equals(Node *other) override;
                void accept(Visitor &visitor) override;
        };
    }

    /** \class Statement
     * \brief Base class for statements
     */
    class Statement : public Node {
        protected:
            Statement() = default;
        public:
            void accept(Visitor &visitor) override;
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

                    /**
                     * \brief Construct a Return Statement node
                     *
                     * Construct a Return Statement node from the return value expression.
                     *
                     * \param expression Pointer to return value expression
                     */
                    explicit Return(std::unique_ptr<Expression> expression)
                            : expression(std::move(expression)) {}

                    bool equals(Node *other) override;
                    void accept(Visitor &visitor) override;
            };

            /** \class Standalone
             * \brief Standalone Statement node
             *
             * Standalone Statement node contains an expression that is evaluated and its value is then discarded.
             */
            class Discard : public Statement {
                public:
                    std::unique_ptr<Expression> expression;

                    /**
                     * \brief Construct a Standalone Statement node
                     *
                     * Construct a Standalone Statement node from an expression.
                     *
                     * \param expression Pointer to contained expression
                     */
                    explicit Discard(std::unique_ptr<Expression> expression)
                            : expression(std::move(expression)) {}

                    bool equals(Node *other) override;
                    void accept(Visitor &visitor) override;
            };

            /** \class Assignment
             * \brief Assignment Statement node
             *
             * Assignment Statement node contains the variable identifier and the value expression.
             */
            class Assignment : public Statement {
                public:
                    //! Variable identifier
                    Identifier identifier;
                    //! Value expression
                    std::unique_ptr<Expression> value;

                    /**
                     * \brief Construct an Assignment Statement node
                     *
                     * Construct an Assignment Statement node from the variable's identifier and value expression.
                     *
                     * \param id Identifier
                     * \param val Pointer to value expression
                     */
                    Assignment(Identifier id, std::unique_ptr<Expression> val)
                            : identifier(std::move(id)), value(std::move(val)) {}

                    bool equals(Node *other) override;
                    void accept(Visitor &visitor) override;
            };
    }

    /** \class Definition
     * \brief Base class for both function and variable definitions
     */
    class Definition : public Node {
        protected:
            Definition() = default;
        public:
            void accept(Visitor &visitor) override;
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

                /**
                 * \brief Construct a Function Definition node
                 *
                 * Construct a Function Definition node from the function's identifier, arguments and body.
                 *
                 * \param id Identifier
                 * \param args Argument identifiers
                 * \param body Pointers to statements comprising the function body
                 */
                Function(Identifier id, std::vector<Identifier> args, std::vector<std::unique_ptr<Statement>> body)
                        : identifier(std::move(id)), arguments(std::move(args)), body(std::move(body)) {}

                bool equals(Node *other) override;
                void accept(Visitor &visitor) override;
        };

        /** \class Variable
         * \brief Variable Definition node
         *
         * Variable Definition contains an assignment statement.
         */
        class Variable : public Definition {
            public:
                std::unique_ptr<statements::Assignment> statement;

                /**
                 * \brief Construct a Variable Definition node
                 *
                 * Construct a Variable Definition node from an assignment statement.
                 *
                 * \param statement Pointer to the assignment statement
                 */
                explicit Variable(std::unique_ptr<statements::Assignment> statement)
                        : statement(std::move(statement)) {}

                bool equals(Node *other) override;
                void accept(Visitor &visitor) override;
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

            /**
             * \brief Construct a Program node
             *
             * Construct a Program node from the definitions comprising the program.
             *
             * \param defs Pointers to definitions comprising the program
             */
            explicit Program(std::vector<std::unique_ptr<Definition>> defs)
                : definitions(std::move(defs)) {}

            bool equals(Node *other) override;
            void accept(Visitor &visitor) override;
    };

    /** \class Visitor
     * \brief Base class for AST node visitors
     */
    class Visitor  {
        protected:
            Visitor() = default;
        public:
            virtual void visit(Expression &node)
                { visit(static_cast<Node&>(node)); }
            virtual void visit(expressions::Modulo &node)
                { visit(static_cast<Expression&>(node)); }
            virtual void visit(expressions::Expression1 &node)
                { visit(static_cast<Expression&>(node)); }
            virtual void visit(expressions::Summation &node)
                { visit(static_cast<expressions::Expression1&>(node)); }
            virtual void visit(expressions::Subtraction &node)
                { visit(static_cast<expressions::Expression1&>(node)); }
            virtual void visit(expressions::Expression2 &node)
                { visit(static_cast<expressions::Expression1&>(node)); }
            virtual void visit(expressions::Multiplication &node)
                { visit(static_cast<expressions::Expression2&>(node)); }
            virtual void visit(expressions::Division &node)
                { visit(static_cast<expressions::Expression2&>(node)); }
            virtual void visit(expressions::Expression3 &node)
                { visit(static_cast<expressions::Expression2&>(node)); }
            virtual void visit(expressions::NumericNegation &node)
                { visit(static_cast<expressions::Expression3&>(node)); }
            virtual void visit(expressions::Expression4 &node)
                { visit(static_cast<expressions::Expression3&>(node)); }
            virtual void visit(expressions::IdentifierExpression &node)
                { visit(static_cast<expressions::Expression4&>(node)); }
            virtual void visit(expressions::Parenthesised &node)
                { visit(static_cast<expressions::Expression4&>(node)); }
            virtual void visit(expressions::FunctionCall &node)
                { visit(static_cast<expressions::Expression4&>(node)); }
            virtual void visit(expressions::LiteralDouble &node)
                { visit(static_cast<expressions::Expression4&>(node)); }

            virtual void visit(Statement &node)
                { visit(static_cast<Node&>(node)); }
            virtual void visit(statements::Assignment &node)
                { visit(static_cast<Statement&>(node)); }
            virtual void visit(statements::Discard &node)
                { visit(static_cast<Statement&>(node)); }
            virtual void visit(statements::Return &node)
                { visit(static_cast<Statement&>(node)); }

            virtual void visit(Definition &node)
                { visit(static_cast<Node&>(node)); }
            virtual void visit(definitions::Function &node)
                { visit(static_cast<Definition&>(node)); }
            virtual void visit(definitions::Variable &node)
                { visit(static_cast<Definition&>(node)); }

            virtual void visit(Program &node)
                { visit(static_cast<Node&>(node)); }

            virtual void visit(Node &) = 0;
    };

    /**
     * @}
     */
}

#endif //BASILISK_AST_H
