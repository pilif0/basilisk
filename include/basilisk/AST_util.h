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
    std::string print_ast(Node *root);
}

#endif //BASILISK_AST_UTIL_H
