/** \file AST_util.h
 * AST utility functions
 *
 * \author Filip Smola
 */
#ifndef BASILISK_AST_UTIL_H
#define BASILISK_AST_UTIL_H

#include <basilisk/AST.h>

#include <vector>
#include <memory>
#include <string>
#include <sstream>

//! AST utility functions
namespace basilisk::ast::util {
    /**
     * \brief Check whether two pointers contain equal nodes
     *
     * \param lhs First pointer
     * \param rhs Second pointer
     * \return `true` when the two pointers contain equal nodes or are both `null`, `false` otherwise
     */
    bool ptr_equals(Node *lhs, Node *rhs);

    /**
     * \brief Check whether two vectors of pointers to nodes contain equal nodes in the same order
     *
     * \param lhs First vector
     * \param rhs Second vector
     * \return `true` when the two vectors contain pointers to equal nodes in equal order, `false` otherwise
     */
    template <typename T>
    bool vec_equals(const std::vector<std::unique_ptr<T>> &lhs, const std::vector<std::unique_ptr<T>> &rhs) {
        // Check sizes match
        if (lhs.size() != rhs.size()) {
            return false;
        }

        // Check elements point to equal nodes
        for (unsigned long i = 0; i < lhs.size(); i++) {
            if (!ptr_equals(lhs[i].get(), rhs[i].get())) {
                return false;
            }
        }

        return true;
    }

    /**
     * \brief Print the AST under the provided node to a string
     *
     * \param root AST root
     * \return Resulting string
     */
     [[deprecated]]
    std::string print_ast(Node *root);

    /** \class PrintVisitor
     * \brief Prints the AST to a string
     *
     * Prints the AST under the visited node to an indented string.
     */
    class PrintVisitor : public Visitor {
        protected:
            //! String stream
            std::ostringstream stream;
            //! Current level of indentation
            unsigned indent_level = 0;
            //! String representing each level of indentation
            static constexpr char indent_string[] = "\t";

            //! Indent to current level
            void indent();
        public:
            static std::string print(Node &node);

            /**
             * \brief Copy the buffered string
             *
             * \return String
             */
            std::string str() { return stream.str(); }

            void visit(Expression &) override;
            void visit(expressions::Modulo &) override;
            void visit(expressions::Expression1 &) override;
            void visit(expressions::Summation &) override;
            void visit(expressions::Subtraction &) override;
            void visit(expressions::Expression2 &) override;
            void visit(expressions::Multiplication &) override;
            void visit(expressions::Division &) override;
            void visit(expressions::Expression3 &) override;
            void visit(expressions::NumericNegation &) override;
            void visit(expressions::Expression4 &) override;
            void visit(expressions::IdentifierExpression &) override;
            void visit(expressions::Parenthesised &) override;
            void visit(expressions::FunctionCall &) override;
            void visit(expressions::LiteralDouble &) override;

            void visit(Statement &) override;
            void visit(statements::Assignment &) override;
            void visit(statements::Discard &) override;
            void visit(statements::Return &) override;

            void visit(Definition &) override;
            void visit(definitions::Function &) override;
            void visit(definitions::Variable &) override;

            void visit(Program &) override;

            void visit(Node &) override;
    };
}

#endif //BASILISK_AST_UTIL_H
