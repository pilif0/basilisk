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
        /** \class ModExpression
         * \brief Expression of modulo operation `x mod m`
         */
        class ModExpression : public Expression {
            public:
                std::unique_ptr<Expression1> x;
                std::unique_ptr<Expression> m;

                ModExpression(std::unique_ptr<Expression1> x, std::unique_ptr<Expression> m)
                    : x(std::move(x)), m(std::move(m)) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };

        // 1st level
        /** \class SumExpression
         * \brief Expression of sum operation `lhs + rhs`
         */
        class SumExpression : public Expression1 {
            public:
                std::unique_ptr<Expression2> lhs;
                std::unique_ptr<Expression1> rhs;

                SumExpression(std::unique_ptr<Expression2> lhs, std::unique_ptr<Expression1> rhs)
                    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };

        /** \class SubExpression
         * \brief Expression of subtract operation `lhs - rhs`
         */
        class SubExpression : public Expression1 {
            public:
                std::unique_ptr<Expression2> lhs;
                std::unique_ptr<Expression1> rhs;

                SubExpression(std::unique_ptr<Expression2> lhs, std::unique_ptr<Expression1> rhs)
                    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };

        // 2nd level
        /** \class MulExpression
         * \brief Expression of multiply operation `lhs * rhs`
         */
        class MulExpression : public Expression2 {
            public:
                std::unique_ptr<Expression3> lhs;
                std::unique_ptr<Expression2> rhs;

                MulExpression(std::unique_ptr<Expression3> lhs, std::unique_ptr<Expression2> rhs)
                    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };

        /** \class DivExpression
         * \brief Expression of divide operation `lhs / rhs`
         */
        class DivExpression : public Expression2 {
            public:
                std::unique_ptr<Expression3> lhs;
                std::unique_ptr<Expression2> rhs;

                DivExpression(std::unique_ptr<Expression3> lhs, std::unique_ptr<Expression2> rhs)
                    : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };

        // 3rd level
        /** \class NegExpression
         * \brief Expression of negate operation `- x`
         */
        class NegExpression : public Expression3 {
            public:
                std::unique_ptr<Expression3> x;

                explicit NegExpression(std::unique_ptr<Expression3> x)
                    : x(std::move(x)) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };

        // 4th level
        /** \class DoubleLitExpression
         * \brief Expression of double literal (e.g. `3.14`)
         */
        class DoubleLitExpression : public Expression4 {
            public:
                //! Value of the literal
                double value;

                explicit DoubleLitExpression(double value)
                    : value(value) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };

        /** \class ParExpression
         * \brief Expression in parentheses
         */
        class ParExpression : public Expression4 {
            public:
                //! Inner expression
                std::unique_ptr<Expression> expression;

                explicit ParExpression(std::unique_ptr<Expression> expression)
                    : expression(std::move(expression)) {}

                bool equals(Node *other) override;
                std::string describe() override;
                std::vector<Node *> children() override;
        };

        /** \class IdentifierExpression
         * \brief Expression of an identifier
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

        /** \class FuncExpression
         * \brief Expression of a function call
         */
        class FuncExpression : public Expression4 {
            public:
                //! Function identifier
                Identifier identifier;
                //! Supplied expressions for arguments
                std::vector<std::unique_ptr<Expression>> arguments;

                FuncExpression(Identifier identifier, std::vector<std::unique_ptr<Expression>> arguments)
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

    /** \class ReturnStatement
     * \brief Return Statement node
     *
     * Return Statement node contains an expression whose value is returned from a function.
     */
    class ReturnStatement : public Statement {
        public:
            //! Returned expression
            std::unique_ptr<Expression> expression;

            explicit ReturnStatement(std::unique_ptr<Expression> expression)
                : expression(std::move(expression)) {}

            bool equals(Node *other) override;
            std::string describe() override;
            std::vector<Node *> children() override;
    };

    /** \class StandaloneStatement
     * \brief Standalone Statement node
     *
     * Standalone Statement node contains an expression that is evaluated and its value is then discarded.
     */
     //TODO better name? VoidStatement?
    class StandaloneStatement : public Statement {
        public:
            std::unique_ptr<Expression> expression;

            explicit StandaloneStatement(std::unique_ptr<Expression> expression)
                : expression(std::move(expression)) {}

            bool equals(Node *other) override;
            std::string describe() override;
            std::vector<Node *> children() override;
    };

    /** \class Definition
     * \brief Base class for both function and variable definitions
     */
    class Definition : public Node {
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
            Identifier identifier;
            //! Arguments of the function in order of declaration
            std::vector<Identifier> arguments;
            //! Non-empty block of statements acting as the function body
            std::vector<std::unique_ptr<Statement>> body;

            FunctionDefinition(Identifier id, std::vector<Identifier> args, std::vector<std::unique_ptr<Statement>> body)
                : identifier(std::move(id)), arguments(std::move(args)), body(std::move(body)) {}

            bool equals(Node *other) override;
            std::string describe() override;
            std::vector<Node *> children() override;
    };

    /** \class VariableDefinition
     * \brief Variable Definition node
     *
     * Variable Definition contains the variable identifier and the value expression.
     */
    class VariableDefinition : public Definition, public Statement {
        public:
            //! Variable identifier
            Identifier identifier;
            //! Value expression
            std::unique_ptr<Expression> value;

            VariableDefinition(Identifier id, std::unique_ptr<Expression> val)
                : identifier(std::move(id)), value(std::move(val)) {}

            bool equals(Node *other) override;
            std::string describe() override;
            std::vector<Node *> children() override;
    };

    /** \class Program
     * \brief Program node
     *
     * Program node contains a set of one or more definitions.
     * Root node of the AST.
     */
    class Program : public Node{
        public:
            //! Pointers to definitions in this program in order of definition
            std::vector<std::unique_ptr<Definition>> definitions;

            //TODO make variadic?
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
