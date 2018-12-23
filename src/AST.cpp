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
            return !std::equal(arguments.begin(), arguments.end(), rhs->arguments.begin(), rhs->arguments.end());
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

    //TODO adjust when determined whether definition order matters
    bool Program::equals(Node *other) {
        // Two programs are equal if they have equivalent definitions

        // Check type
        auto rhs = dynamic_cast<Program*>(other);
        if (!rhs) {
            return false;
        }

        // Check sizes match
        if (definitions.size() != rhs->definitions.size()) {
            return false;
        }

        return util::vec_equals(definitions, rhs->definitions);
    }
    //--- End node equality
}
