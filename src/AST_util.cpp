/** \file AST_util.cpp
* AST utility function implementations
*
* \author Filip Smola
*/
#include <basilisk/AST_util.h>

#include <sstream>
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
        std::vector<node_ind> queue{{root, 0}};
        while (!queue.empty()) {
            // Pop off of the queue
            auto [node, ind] = queue.back();
            queue.pop_back();

            // Indent the line
            result << std::string(ind, '\t');

            // Print description
            result << "- " << node->describe() << "\n";

            // Push all children
            //TODO watch out for order
            for (auto e : node->children()) {
                queue.emplace_back(e, ind + 1);
            }
        }

        // Return as string
        return result.str();
    }
    //--- End node printing
}
