/** \file AST_util.cpp
* AST utility function implementations
*
* \author Filip Smola
*/
#include <basilisk/AST_util.h>

#include <sstream>
#include <algorithm>
#include <utility>

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

    //--- Start node printing
    //! Pairing of node pointer with its indentation level
    typedef std::pair<Node*, unsigned> node_ind;

    std::string print_ast(Node *root) {
        // Prepare result
        std::ostringstream result;

        // Print this node and children (Depth-first search)
        std::vector<node_ind> stack{{root, 0}};
        while (!stack.empty()) {
            // Pop off of the stack
            auto [node, ind] = stack.back();
            stack.pop_back();

            // Indent the line
            result << std::string(ind, '\t');

            // Print description
            result << "- " << node->describe() << "\n";

            // Push all children (in reverse order because pushing onto a stack)
            auto children = node->children();
            std::reverse(children.begin(), children.end());
            for (auto e : children) {
                stack.emplace_back(e, ind + 1);
            }
        }

        // Return as string
        return result.str();
    }
    //--- End node printing
}
