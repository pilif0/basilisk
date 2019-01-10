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

    //--- Start PrintVisitor implementation
    /**
     * \brief Print the AST under a node using this visitor
     *
     * \param node Node to print from
     * \return Indented string
     */
    std::string PrintVisitor::print(Node &node) {
        PrintVisitor visitor;
        node.accept(visitor);
        return visitor.str();
    }

    void PrintVisitor::indent() {
        for (unsigned i = 0; i < indent_level; i++) {
            stream << indent_string;
        }
    }

    void PrintVisitor::visit(Expression &) {
        indent();
        stream << "- Unknown Expression\n";
    }

    void PrintVisitor::visit(expressions::Modulo &node) {
        // Write division expression description
        indent();
        stream << "- Modulo Expression: (1) % (2)\n";

        // Write descriptions of left- and right-hand sides at increased indentation
        indent_level++;
        node.x->accept(*this);
        node.m->accept(*this);
        indent_level--;
    }

    void PrintVisitor::visit(expressions::Expression1 &) {
        indent();
        stream << "- Unknown Expression1\n";
    }

    void PrintVisitor::visit(expressions::Summation &node) {
        // Write summation expression description
        indent();
        stream << "- Summation Expression: (1) + (2)\n";

        // Write descriptions of left- and right-hand sides at increased indentation
        indent_level++;
        node.lhs->accept(*this);
        node.rhs->accept(*this);
        indent_level--;
    }

    void PrintVisitor::visit(expressions::Subtraction &node) {
        // Write Subtraction expression description
        indent();
        stream << "- Subtraction Expression: (1) - (2)\n";

        // Write descriptions of left- and right-hand sides at increased indentation
        indent_level++;
        node.lhs->accept(*this);
        node.rhs->accept(*this);
        indent_level--;
    }

    void PrintVisitor::visit(expressions::Expression2 &) {
        indent();
        stream << "- Unknown Expression2\n";
    }

    void PrintVisitor::visit(expressions::Multiplication &node) {
        // Write multiplication expression description
        indent();
        stream << "- Multiplication Expression: (1) * (2)\n";

        // Write descriptions of left- and right-hand sides at increased indentation
        indent_level++;
        node.lhs->accept(*this);
        node.rhs->accept(*this);
        indent_level--;
    }

    void PrintVisitor::visit(expressions::Division &node) {
        // Write division expression description
        indent();
        stream << "- Division Expression: (1) / (2)\n";

        // Write descriptions of left- and right-hand sides at increased indentation
        indent_level++;
        node.lhs->accept(*this);
        node.rhs->accept(*this);
        indent_level--;
    }

    void PrintVisitor::visit(expressions::Expression3 &) {
        indent();
        stream << "- Unknown Expression3\n";
    }

    void PrintVisitor::visit(expressions::NumericNegation &node) {
        // Write numeric negation expression description
        indent();
        stream << "- Numeric Negation Expression: - (1)\n";

        // Write expression description at increased indentation
        indent_level++;
        node.x->accept(*this);
        indent_level--;
    }

    void PrintVisitor::visit(expressions::Expression4 &) {
        stream << "Unknown Expression4\n";
    }

    void PrintVisitor::visit(expressions::IdentifierExpression &node) {
        // Write identifier expression description
        indent();
        stream << "- Identifier Expression: " << node.identifier << "\n";
    }

    void PrintVisitor::visit(expressions::Parenthesised &node) {
        // Write parenthesised expression description
        indent();
        stream << "- Parenthesised Expression: ( (1) )\n";

        // Write expression description at increased indentation
        indent_level++;
        node.expression->accept(*this);
        indent_level--;
    }

    void PrintVisitor::visit(expressions::FunctionCall &node) {
        // Write function call expression description
        indent();
        stream << "- Function Call Expression: " << node.identifier << "(...)\n";

        // Write descriptions for each argument expression at increased indentation
        indent_level++;
        for (auto &expression : node.arguments) {
            expression->accept(*this);
        }
        indent_level--;
    }

    void PrintVisitor::visit(expressions::LiteralDouble &node) {
        // Write double literal expression description
        indent();
        stream << "- Double Literal Expression: " << node.value << "\n";
    }

    void PrintVisitor::visit(Statement &) {
        indent();
        stream << "- Unknown Statement\n";
    }

    void PrintVisitor::visit(statements::Assignment &node) {
        // Write assignment statement description
        indent();
        stream << "- Assignment Statement: " << node.identifier << " = (1) ;\n";

        // Write value description at increased indentation
        indent_level++;
        node.value->accept(*this);
        indent_level--;
    }

    void PrintVisitor::visit(statements::Discard &node) {
        // Write discard statement description
        indent();
        stream << "- Discard Statement: (1) ;\n";

        // Write expression description at increased indentation
        indent_level++;
        node.expression->accept(*this);
        indent_level--;
    }

    void PrintVisitor::visit(statements::Return &node) {
        // Write return statement description
        indent();
        stream << "- Return Statement: (1) ;\n";

        // Write expression description at increased indentation
        indent_level++;
        node.expression->accept(*this);
        indent_level--;
    }

    void PrintVisitor::visit(Definition &) {
        indent();
        stream << "- Unknown Definition\n";
    }

    void PrintVisitor::visit(definitions::Function &node) {
        // Write function definition description
        indent();
        stream << "- Function Definition: " << node.identifier << "(";
        bool not_first = false;
        for (auto &arg : node.arguments) {
            // Print divider
            if (not_first) {
                stream << ", ";
            } else {
                not_first = true;
            }

            // Print argument
            stream << arg;
        }
        stream << ")\n";

        // Write description for each statement at increased indentation
        indent_level++;
        for (auto &statement : node.body) {
            statement->accept(*this);
        }
        indent_level--;
    }

    void PrintVisitor::visit(definitions::Variable &node) {
        // Write variable definition description
        indent();
        stream << "- Variable Definition: (1) ;\n";

        // Write assignment statement description at increased indentation
        indent_level++;
        node.statement->accept(*this);
        indent_level--;
    }

    void PrintVisitor::visit(Program &node) {
        // Write program description
        indent();
        stream << "- Program:\n";

        // Write description for each definition at increased indentation
        indent_level++;
        for (auto &definition : node.definitions) {
            definition->accept(*this);
        }
        indent_level--;
    }

    void PrintVisitor::visit(Node &) {
        indent();
        stream << "-Unknown Node\n";
    }
    //--- End PrintVisitor implementation
}
