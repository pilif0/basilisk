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
}
