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
    template<typename T>
    void visit(T */*node*/, std::ostringstream &stream, std::vector<node_ind> &/*queue*/, unsigned /*indent*/) {
        stream << "- Unknown Node";
    }

    void visit(expressions::ModExpression* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent) {
        // Check for null
        if (!node) {
            stream << "- Mod Expression: null";
            return;
        }

        // Print description to the stream
        stream << "- Mod Expression: (1) % (2)\n";

        // Add children to the queue
        queue.emplace_back(node->x.get(), indent + 1);
        queue.emplace_back(node->m.get(), indent + 1);
    }

    void visit(expressions::SumExpression* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent) {
        // Check for null
        if (!node) {
            stream << "- Sum Expression: null";
            return;
        }

        // Print description to the stream
        stream << "- Sum Expression: (1) + (2)\n";

        // Add children to the queue
        queue.emplace_back(node->lhs.get(), indent + 1);
        queue.emplace_back(node->rhs.get(), indent + 1);
    }

    void visit(expressions::SubExpression* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent) {
        // Check for null
        if (!node) {
            stream << "- Subtraction Expression: null";
            return;
        }

        // Print description to the stream
        stream << "- Subtraction Expression: (1) - (2)\n";

        // Add children to the queue
        queue.emplace_back(node->lhs.get(), indent + 1);
        queue.emplace_back(node->rhs.get(), indent + 1);
    }

    void visit(expressions::MulExpression* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent) {
        // Check for null
        if (!node) {
            stream << "- Multiplication Expression: null";
            return;
        }

        // Print description to the stream
        stream << "- Multiplication Expression: (1) * (2)\n";

        // Add children to the queue
        queue.emplace_back(node->lhs.get(), indent + 1);
        queue.emplace_back(node->rhs.get(), indent + 1);
    }

    void visit(expressions::DivExpression* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent) {
        // Check for null
        if (!node) {
            stream << "- Division Expression: null";
            return;
        }

        // Print description to the stream
        stream << "- Division Expression: (1) / (2)\n";

        // Add children to the queue
        queue.emplace_back(node->lhs.get(), indent + 1);
        queue.emplace_back(node->rhs.get(), indent + 1);
    }

    void visit(expressions::NegExpression* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent) {
        // Check for null
        if (!node) {
            stream << "- Negation Expression: null";
            return;
        }

        // Print description to the stream
        stream << "- Negation Expression: - (1)\n";

        // Add children to the queue
        queue.emplace_back(node->x.get(), indent + 1);
    }

    void visit(expressions::DoubleLitExpression* node, std::ostringstream &stream, std::vector<node_ind> &/*queue*/, unsigned /*indent*/) {
        // Check for null
        if (!node) {
            stream << "- Double Literal Expression: null";
            return;
        }

        // Print description to the stream
        stream << "- Double Literal Expression: " << node->value << "\n";
    }

    void visit(expressions::ParExpression* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent) {
        // Check for null
        if (!node) {
            stream << "- Parenthesised Expression: null";
            return;
        }

        // Print description to the stream
        stream << "- Parenthesised Expression: ( (1) )\n";

        // Add children to the queue
        queue.emplace_back(node->expression.get(), indent + 1);
    }

    void visit(expressions::IdentifierExpression* node, std::ostringstream &stream, std::vector<node_ind> &/*queue*/, unsigned /*indent*/) {
        // Check for null
        if (!node) {
            stream << "- Identifier Expression: null";
            return;
        }

        // Print description to the stream
        stream << "- Identifier Expression: " << node->identifier << "\n";
    }

    void visit(expressions::FuncExpression* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent) {
        // Check for null
        if (!node) {
            stream << "- Function Call Expression: null";
            return;
        }

        // Print description to the stream
        stream << "- Function Call Expression: " << node->identifier << "(...)\n";

        // Add children to the queue
        for (auto &c : node->arguments) {
            queue.emplace_back(c.get(), indent + 1);
        }
    }

    void visit(ReturnStatement* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent) {
        // Check for null
        if (!node) {
            stream << "- Return Statement: null";
            return;
        }

        // Print description to the stream
        stream << "- Return Statement: return (1);\n";

        // Add children to the queue
        queue.emplace_back(node->expression.get(), indent + 1);
    }

    void visit(StandaloneStatement* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent) {
        // Check for null
        if (!node) {
            stream << "- Standalone Statement: null";
            return;
        }

        // Print description to the stream
        stream << "- Standalone Statement: (1);\n";

        // Add children to the queue
        queue.emplace_back(node->expression.get(), indent + 1);
    }

    void visit(FunctionDefinition* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent) {
        // Check for null
        if (!node) {
            stream << "- Function Definition: null";
            return;
        }

        // Print description to the stream
        stream << "- Function Definition: " << node->identifier << "(";

        // Print argument list
        bool not_first = false;
        for (auto &c : node->arguments) {
            // Print divider
            if (not_first) {
                stream << ", ";
            } else {
                not_first = true;
            }

            // Print argument
            stream << c;
        }

        // Print end of description
        stream << ") {...}\n";

        // Add children to the queue
        for (auto &c : node->body) {
            queue.emplace_back(c.get(), indent + 1);
        }
    }

    void visit(VariableDefinition* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent) {
        // Check for null
        if (!node) {
            stream << "- Variable Definition: null";
            return;
        }

        // Print description to the stream
        stream << "- Variable Definition: " << node->identifier <<" = (1);\n";

        // Add children to the queue
        queue.emplace_back(node->value.get(), indent + 1);
    }

    void visit(Program* node, std::ostringstream &stream, std::vector<node_ind> &queue, unsigned indent) {
        // Check for null
        if (!node) {
            stream << "- Program: null";
            return;
        }

        // Print description to the stream
        stream << "- Program:\n";

        // Add children to the queue
        for (auto &c : node->definitions) {
            queue.emplace_back(c.get(), indent + 1);
        }
    }

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

            // Visit the node
            visit(node, result, queue, ind);
        }

        // Return as string
        return result.str();
    }
    //--- End node printing
}
