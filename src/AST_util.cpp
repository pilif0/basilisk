/** \file AST_util.cpp
* AST utility function implementations
*
* \author Filip Smola
*/
#include <basilisk/AST_util.h>

namespace basilisk::ast::util {

    bool ptr_equals(Node *lhs, Node *rhs) {
        // Check for equal pointers
        if (lhs == rhs) {
            // Equal pointers -> both null or both point to the same node
            return true;
        }

        // Check for null
        if (!lhs || !rhs) {
            // One is null -> distinct
            return false;
        }

        // Check nodes
        return lhs->equals(rhs);
    }
}
