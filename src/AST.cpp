/** \file AST.cpp
* Abstract Syntax Tree implementation
*
* \author Filip Smola
*/

#include <basilisk/AST.h>
#include <basilisk/AST_util.h>

#include <algorithm>

namespace basilisk::ast {

    //--- Start node equality
    namespace expressions {
        bool ModExpression::equals(Node *other) {
            // Two mod expressions are equal if their sub-expressions are equal

            // Check type
            auto rhs = dynamic_cast<ModExpression*>(other);
            if (!rhs) {
                return false;
            }

            // Check sub-expressions
            return util::ptr_equals(x.get(), rhs->x.get()) && util::ptr_equals(m.get(), rhs->m.get());
        }

        bool SumExpression::equals(Node *other) {
            // Two sum expressions are equal if their sub-expressions are equal

            // Check type
            auto r = dynamic_cast<SumExpression*>(other);
            if (!r) {
                return false;
            }

            // Check sub-expressions
            return util::ptr_equals(lhs.get(), r->lhs.get()) && util::ptr_equals(rhs.get(), r->rhs.get());
        }

        bool SubExpression::equals(Node *other) {
            // Two subtraction expressions are equal if their sub-expressions are equal

            // Check type
            auto r = dynamic_cast<SubExpression*>(other);
            if (!r) {
                return false;
            }

            // Check sub-expressions
            return util::ptr_equals(lhs.get(), r->lhs.get()) && util::ptr_equals(rhs.get(), r->rhs.get());
        }

        bool MulExpression::equals(Node *other) {
            // Two multiplication expressions are equal if their sub-expressions are equal

            // Check type
            auto r = dynamic_cast<MulExpression*>(other);
            if (!r) {
                return false;
            }

            // Check sub-expressions
            return util::ptr_equals(lhs.get(), r->lhs.get()) && util::ptr_equals(rhs.get(), r->rhs.get());
        }

        bool DivExpression::equals(Node *other) {
            // Two division expressions are equal if their sub-expressions are equal

            // Check type
            auto r = dynamic_cast<DivExpression*>(other);
            if (!r) {
                return false;
            }

            // Check sub-expressions
            return util::ptr_equals(lhs.get(), r->lhs.get()) && util::ptr_equals(rhs.get(), r->rhs.get());
        }

        bool NegExpression::equals(Node *other) {
            // Two negation expressions are equal if their sub-expression is equal

            // Check type
            auto r = dynamic_cast<NegExpression*>(other);
            if (!r) {
                return false;
            }

            // Check sub-expressions
            return util::ptr_equals(x.get(), r->x.get());
        }

        bool DoubleLitExpression::equals(Node *other) {
            // Two double literals are equal if their value is equal

            // Check type
            auto r = dynamic_cast<DoubleLitExpression*>(other);
            if (!r) {
                return false;
            }

            // Check values
            return value == r->value;
        }

        bool ParExpression::equals(Node *other) {
            // Two parenthesised expressions are equal if their sub-expression is equal

            // Check type
            auto r = dynamic_cast<ParExpression*>(other);
            if (!r) {
                return false;
            }

            // Check sub-expressions
            return util::ptr_equals(expression.get(), r->expression.get());
        }

        bool IdentifierExpression::equals(Node *other) {
            // Two identifier expressions are equal if their identifier is equal

            // Check type
            auto r = dynamic_cast<IdentifierExpression*>(other);
            if (!r) {
                return false;
            }

            // Check identifier
            return identifier == r->identifier;
        }

        bool FuncExpression::equals(Node *other) {
            // Two function call expressions are equal if their identifier and argument list are equal

            // Check type
            auto rhs = dynamic_cast<FuncExpression*>(other);
            if (!rhs) {
                return false;
            }

            // Check identifiers match
            if (identifier != rhs->identifier) {
                return false;
            }

            // Check arguments
            return util::vec_equals(arguments, rhs->arguments);
        }
    }

    bool ReturnStatement::equals(Node *other) {
        // Two return statements are equal if their value expression is equal

        // Check type
        auto rhs = dynamic_cast<ReturnStatement*>(other);
        if (!rhs) {
            return false;
        }

        // Check value expressions
        return util::ptr_equals(expression.get(), rhs->expression.get());
    }

    bool StandaloneStatement::equals(Node *other) {
        // Two standalone statements are equal if their expression is equal

        // Check type
        auto rhs = dynamic_cast<StandaloneStatement*>(other);
        if (!rhs) {
            return false;
        }

        // Check value expressions
        return util::ptr_equals(expression.get(), rhs->expression.get());
    }

    bool FunctionDefinition::equals(Node *other) {
        // Two function definitions are equal if all their identifier, argument lists and body are equal

        // Check type
        auto rhs = dynamic_cast<FunctionDefinition*>(other);
        if (!rhs) {
            return false;
        }

        // Check identifiers match
        if (identifier != rhs->identifier) {
            return false;
        }

        // Check arguments
        if (!std::equal(arguments.begin(), arguments.end(), rhs->arguments.begin(), rhs->arguments.end())) {
            return false;
        }

        // Check bodies match
        return util::vec_equals(body, rhs->body);
    }

    bool VariableDefinition::equals(Node *other) {
        // Two variable definitions are equal if their identifier and value expression are equal

        // Check type
        auto rhs = dynamic_cast<VariableDefinition*>(other);
        if (!rhs) {
            return false;
        }

        // Check identifiers match
        if (identifier != rhs->identifier) {
            return false;
        }

        // Check value expressions
        return util::ptr_equals(value.get(), rhs->value.get());
    }

    bool VariableStatement::equals(Node *other) {
        // Two variable statements are equal if their definition is equal

        // Check type
        auto rhs = dynamic_cast<VariableStatement*>(other);
        if (!rhs) {
            return false;
        }

        // Check value expressions
        return util::ptr_equals(definition.get(), rhs->definition.get());
    }

    //TODO adjust when determined whether definition order matters
    bool Program::equals(Node *other) {
        // Two programs are equal if they have equivalent definitions

        // Check type
        auto rhs = dynamic_cast<Program*>(other);
        if (!rhs) {
            return false;
        }

        return util::vec_equals(definitions, rhs->definitions);
    }
    //--- End node equality

    //--- Start node descriptions
    std::string expressions::ModExpression::describe() { return "Modulo Expression: (1) % (2)"; }

    std::string expressions::SumExpression::describe() { return "Sum Expression: (1) + (2)"; }

    std::string expressions::SubExpression::describe() { return "Subtraction Expression: (1) - (2)"; }

    std::string expressions::MulExpression::describe() { return "Multiplication Expression: (1) * (2)"; }

    std::string expressions::DivExpression::describe() { return "Division Expression: (1) / (2)"; }

    std::string expressions::NegExpression::describe() { return "Negation Expression: - (1)"; }

    std::string expressions::DoubleLitExpression::describe() {
        std::ostringstream result;
        result << "Double Literal Expression: " << value;
        return result.str();

    }
    std::string expressions::ParExpression::describe() { return "Parenthesised Expression: ( (1) )"; }

    std::string expressions::IdentifierExpression::describe() {
        std::ostringstream result;
        result << "Identifier Expression: " << identifier;
        return result.str();
    }

    std::string expressions::FuncExpression::describe() {
        std::ostringstream result;
        result << "Function Call Expression: " << identifier << "(...)";
        return result.str();
    }

    std::string ReturnStatement::describe() { return "Return Statement: return (1) ;"; }

    std::string StandaloneStatement::describe() { return "Standalone Statement: (1) ;"; }

    std::string FunctionDefinition::describe() {
        std::ostringstream result;
        result << "Function Definition: " << identifier << "(";
        bool not_first = false;
        for (auto &c : arguments) {
            // Print divider
            if (not_first) {
                result << ", ";
            } else {
                not_first = true;
            }

            // Print argument
            result << c;
        }
        result << ")";
        return result.str();
    }

    std::string VariableDefinition::describe() {
        std::ostringstream result;
        result << "Variable Definition: " << identifier << " = (1) ;";
        return result.str();
    }

    std::string VariableStatement::describe() { return "Variable Definition Statement: (1) ;"; }

    std::string Program::describe() { return "Program:"; }
    //--- End node descriptions

    //--- Start node children
    std::vector<Node *> expressions::ModExpression::children() { return {x.get(), m.get()}; }

    std::vector<Node *> expressions::SumExpression::children() { return {lhs.get(), rhs.get()}; }

    std::vector<Node *> expressions::SubExpression::children() { return {lhs.get(), rhs.get()}; }

    std::vector<Node *> expressions::MulExpression::children() { return {lhs.get(), rhs.get()}; }

    std::vector<Node *> expressions::DivExpression::children() { return {lhs.get(), rhs.get()}; }

    std::vector<Node *> expressions::NegExpression::children() { return {x.get()}; }

    std::vector<Node *> expressions::DoubleLitExpression::children() { return {}; }

    std::vector<Node *> expressions::ParExpression::children() { return {expression.get()}; }

    std::vector<Node *> expressions::IdentifierExpression::children() { return {}; }

    std::vector<Node *> expressions::FuncExpression::children() {
        std::vector<Node *> result;
        for (auto &e : arguments) {
            result.push_back(e.get());
        }
        return result;
    }

    std::vector<Node *> ReturnStatement::children() { return {expression.get()}; }

    std::vector<Node *> StandaloneStatement::children() { return {expression.get()}; }

    std::vector<Node *> FunctionDefinition::children() {
        std::vector<Node *> result;
        for (auto &e : body) {
            result.push_back(e.get());
        }
        return result;
    }

    std::vector<Node *> VariableDefinition::children() { return {value.get()}; }

    std::vector<Node *> VariableStatement::children() { return {definition.get()}; }

    std::vector<Node *> Program::children() {
        std::vector<Node *> result;
        for (auto &e : definitions) {
            result.push_back(e.get());
        }
        return result;
    }
    //--- End node children
}
