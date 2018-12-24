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

        return false;
    }

    //! Pairing of node pointer with its indentation level
    typedef std::pair<Node*, unsigned> node_ind;

    /**
     * \brief Helper for `print_ast` that visits a given node
     *
     * \param node Pointer to node to visit
     * \param stream Stream to which to print the node's description
     * \param queue Node queue to which to push child nodes
     * \param indent Current indentation level (to be propagated incremented to children)
     */
    template <typename T>
    void visit(T* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent);

    template <>
    void visit(expressions::ModExpression* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent);

    template <>
    void visit(expressions::SumExpression* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent);

    template <>
    void visit(expressions::SubExpression* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent);

    template <>
    void visit(expressions::MulExpression* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent);

    template <>
    void visit(expressions::DivExpression* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent);

    template <>
    void visit(expressions::NegExpression* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent);

    template <>
    void visit(expressions::DoubleLitExpression* node, std::ostringstream &stream, std::vector<node_ind> &/*queue*/, unsigned /*indent*/);

    template <>
    void visit(expressions::ParExpression* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent);

    template <>
    void visit(expressions::IdentifierExpression* node, std::ostringstream &stream, std::vector<node_ind> &/*queue*/, unsigned /*indent*/);

    template <>
    void visit(expressions::FuncExpression* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent);

    template <>
    void visit(ReturnStatement* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent);

    template <>
    void visit(StandaloneStatement* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent);

    template <>
    void visit(FunctionDefinition* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent);

    template <>
    void visit(VariableDefinition* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent);

    template <>
    void visit(Program* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent);

    /**
     * \brief Print the AST under the provided node to a string
     *
     * \param root AST root
     * \return Resulting string
     */
    std::string print_ast(Node *root);
}

#endif //BASILISK_AST_UTIL_H
