/** \file AST.cpp
* Abstract Syntax Tree implementation
*
* \author Filip Smola
*/

#include <basilisk/AST.h>
#include <basilisk/AST_util.h>

#include <algorithm>

namespace basilisk::ast {

    //--- Start visitor accepting
    void Node::accept(Visitor &visitor) { visitor.visit(*this); }

    void Expression::accept(Visitor &visitor) { visitor.visit(*this); }
    namespace expressions {
        void Expression1::accept(Visitor &visitor) { visitor.visit(*this); }
        void Expression2::accept(Visitor &visitor) { visitor.visit(*this); }
        void Expression3::accept(Visitor &visitor) { visitor.visit(*this); }
        void Expression4::accept(Visitor &visitor) { visitor.visit(*this); }
        void Modulo::accept(Visitor &visitor) { visitor.visit(*this); }
        void Summation::accept(Visitor &visitor) { visitor.visit(*this); }
        void Subtraction::accept(Visitor &visitor) { visitor.visit(*this); }
        void Multiplication::accept(Visitor &visitor) { visitor.visit(*this); }
        void Division::accept(Visitor &visitor) { visitor.visit(*this); }
        void NumericNegation::accept(Visitor &visitor) { visitor.visit(*this); }
        void LiteralDouble::accept(Visitor &visitor) { visitor.visit(*this); }
        void Parenthesised::accept(Visitor &visitor) { visitor.visit(*this); }
        void IdentifierExpression::accept(Visitor &visitor) { visitor.visit(*this); }
        void FunctionCall::accept(Visitor &visitor) { visitor.visit(*this); }
    }

    void Statement::accept(Visitor &visitor) { visitor.visit(*this); }
    namespace statements {
        void Return::accept(Visitor &visitor) { visitor.visit(*this); }
        void Discard::accept(Visitor &visitor) { visitor.visit(*this); }
        void Assignment::accept(Visitor &visitor) { visitor.visit(*this); }
    }

    void Definition::accept(Visitor &visitor) { visitor.visit(*this); }
    namespace definitions {
        void Function::accept(Visitor &visitor) { visitor.visit(*this); }
        void Variable::accept(Visitor &visitor) { visitor.visit(*this); }
    }

    void Program::accept(Visitor &visitor) { visitor.visit(*this); }
    //--- End visitor accepting

    //--- Start node equality
    namespace expressions {
        bool Modulo::equals(Node *other) {
            // Two mod expressions are equal if their sub-expressions are equal

            // Check type
            auto rhs = dynamic_cast<Modulo*>(other);
            if (!rhs) {
                return false;
            }

            // Check sub-expressions
            return util::ptr_equals(x.get(), rhs->x.get()) && util::ptr_equals(m.get(), rhs->m.get());
        }

        bool Summation::equals(Node *other) {
            // Two sum expressions are equal if their sub-expressions are equal

            // Check type
            auto r = dynamic_cast<Summation*>(other);
            if (!r) {
                return false;
            }

            // Check sub-expressions
            return util::ptr_equals(lhs.get(), r->lhs.get()) && util::ptr_equals(rhs.get(), r->rhs.get());
        }

        bool Subtraction::equals(Node *other) {
            // Two subtraction expressions are equal if their sub-expressions are equal

            // Check type
            auto r = dynamic_cast<Subtraction*>(other);
            if (!r) {
                return false;
            }

            // Check sub-expressions
            return util::ptr_equals(lhs.get(), r->lhs.get()) && util::ptr_equals(rhs.get(), r->rhs.get());
        }

        bool Multiplication::equals(Node *other) {
            // Two multiplication expressions are equal if their sub-expressions are equal

            // Check type
            auto r = dynamic_cast<Multiplication*>(other);
            if (!r) {
                return false;
            }

            // Check sub-expressions
            return util::ptr_equals(lhs.get(), r->lhs.get()) && util::ptr_equals(rhs.get(), r->rhs.get());
        }

        bool Division::equals(Node *other) {
            // Two division expressions are equal if their sub-expressions are equal

            // Check type
            auto r = dynamic_cast<Division*>(other);
            if (!r) {
                return false;
            }

            // Check sub-expressions
            return util::ptr_equals(lhs.get(), r->lhs.get()) && util::ptr_equals(rhs.get(), r->rhs.get());
        }

        bool NumericNegation::equals(Node *other) {
            // Two negation expressions are equal if their sub-expression is equal

            // Check type
            auto r = dynamic_cast<NumericNegation*>(other);
            if (!r) {
                return false;
            }

            // Check sub-expressions
            return util::ptr_equals(x.get(), r->x.get());
        }

        bool LiteralDouble::equals(Node *other) {
            // Two double literals are equal if their value is equal

            // Check type
            auto r = dynamic_cast<LiteralDouble*>(other);
            if (!r) {
                return false;
            }

            // Check values
            return value == r->value;
        }

        bool Parenthesised::equals(Node *other) {
            // Two parenthesised expressions are equal if their sub-expression is equal

            // Check type
            auto r = dynamic_cast<Parenthesised*>(other);
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

        bool FunctionCall::equals(Node *other) {
            // Two function call expressions are equal if their identifier and argument list are equal

            // Check type
            auto rhs = dynamic_cast<FunctionCall*>(other);
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

    namespace statements {
        bool Return::equals(Node *other) {
            // Two return statements are equal if their value expression is equal

            // Check type
            auto rhs = dynamic_cast<Return*>(other);
            if (!rhs) {
                return false;
            }

            // Check value expressions
            return util::ptr_equals(expression.get(), rhs->expression.get());
        }

        bool Discard::equals(Node *other) {
            // Two discard statements are equal if their expression is equal

            // Check type
            auto rhs = dynamic_cast<Discard*>(other);
            if (!rhs) {
                return false;
            }

            // Check value expressions
            return util::ptr_equals(expression.get(), rhs->expression.get());
        }

        bool Assignment::equals(Node *other) {
            // Two assignment statements are equal if their identifier and value expression are equal

            // Check type
            auto rhs = dynamic_cast<Assignment*>(other);
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
    }

    namespace definitions {
        bool Function::equals(Node *other) {
            // Two function definitions are equal if all their identifier, argument lists and body are equal

            // Check type
            auto rhs = dynamic_cast<Function*>(other);
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

        bool Variable::equals(Node *other) {
            // Two variable definitions are equal if their statements is equal

            // Check type
            auto rhs = dynamic_cast<Variable*>(other);
            if (!rhs) {
                return false;
            }

            // Check value expressions
            return util::ptr_equals(statement.get(), rhs->statement.get());
        }
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
    std::string expressions::Modulo::describe() { return "Modulo Expression: (1) % (2)"; }

    std::string expressions::Summation::describe() { return "Summation Expression: (1) + (2)"; }

    std::string expressions::Subtraction::describe() { return "Subtraction Expression: (1) - (2)"; }

    std::string expressions::Multiplication::describe() { return "Multiplication Expression: (1) * (2)"; }

    std::string expressions::Division::describe() { return "Division Expression: (1) / (2)"; }

    std::string expressions::NumericNegation::describe() { return "Numeric Negation Expression: - (1)"; }

    std::string expressions::LiteralDouble::describe() {
        std::ostringstream result;
        result << "Double Literal Expression: " << value;
        return result.str();

    }
    std::string expressions::Parenthesised::describe() { return "Parenthesised Expression: ( (1) )"; }

    std::string expressions::IdentifierExpression::describe() {
        std::ostringstream result;
        result << "Identifier Expression: " << identifier;
        return result.str();
    }

    std::string expressions::FunctionCall::describe() {
        std::ostringstream result;
        result << "Function Call Expression: " << identifier << "(...)";
        return result.str();
    }

    std::string statements::Return::describe() { return "Return Statement: return (1) ;"; }

    std::string statements::Discard::describe() { return "Discard Statement: (1) ;"; }

    std::string definitions::Function::describe() {
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

    std::string statements::Assignment::describe() {
        std::ostringstream result;
        result << "Assignment Statement: " << identifier << " = (1) ;";
        return result.str();
    }

    std::string definitions::Variable::describe() { return "Variable Definition: (1) ;"; }

    std::string Program::describe() { return "Program:"; }
    //--- End node descriptions

    //--- Start node children
    std::vector<Node *> expressions::Modulo::children() { return {x.get(), m.get()}; }

    std::vector<Node *> expressions::Summation::children() { return {lhs.get(), rhs.get()}; }

    std::vector<Node *> expressions::Subtraction::children() { return {lhs.get(), rhs.get()}; }

    std::vector<Node *> expressions::Multiplication::children() { return {lhs.get(), rhs.get()}; }

    std::vector<Node *> expressions::Division::children() { return {lhs.get(), rhs.get()}; }

    std::vector<Node *> expressions::NumericNegation::children() { return {x.get()}; }

    std::vector<Node *> expressions::LiteralDouble::children() { return {}; }

    std::vector<Node *> expressions::Parenthesised::children() { return {expression.get()}; }

    std::vector<Node *> expressions::IdentifierExpression::children() { return {}; }

    std::vector<Node *> expressions::FunctionCall::children() {
        std::vector<Node *> result;
        for (auto &e : arguments) {
            result.push_back(e.get());
        }
        return result;
    }

    std::vector<Node *> statements::Return::children() { return {expression.get()}; }

    std::vector<Node *> statements::Discard::children() { return {expression.get()}; }

    std::vector<Node *> definitions::Function::children() {
        std::vector<Node *> result;
        for (auto &e : body) {
            result.push_back(e.get());
        }
        return result;
    }

    std::vector<Node *> statements::Assignment::children() { return {value.get()}; }

    std::vector<Node *> definitions::Variable::children() { return {statement.get()}; }

    std::vector<Node *> Program::children() {
        std::vector<Node *> result;
        for (auto &e : definitions) {
            result.push_back(e.get());
        }
        return result;
    }
    //--- End node children
}
