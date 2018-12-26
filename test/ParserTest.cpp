/** \file LexerTest.cpp
 * Lexer test module
 *
 * \author Filip Smola
 */
#define BOOST_TEST_MODULE "Parser"

#include <basilisk/Parser.h>
#include <basilisk/Tokens.h>
#include <basilisk/AST.h>
#include <basilisk/AST_util.h>

#include <boost/test/unit_test.hpp>

#include <vector>
#include <basilisk/Lexer.h>

namespace tokens = basilisk::tokens;
namespace tags = basilisk::tokens::tags;
namespace parser = basilisk::parser;
namespace ast = basilisk::ast;

//! Fixture that sets up two queues for use as lexer input and output
struct QueuesFixture {
    //! Token queue type
    // Note: queue front is the vector back
    typedef std::vector<tokens::Token> token_queue_t;

    //! Parser input queue in reverse order (back of this is front of queue)
    token_queue_t input;
    //! Parser compatible get function reference
    const parser::get_function_t get_f = std::bind(&QueuesFixture::get, this);
    //! Parser compatible peek function reference
    const parser::peek_function_t peek_f = std::bind(&QueuesFixture::peek, this, std::placeholders::_1);

    /**
     * \brief Pop a token from the front of the input queue and return it
     *
     * \return Popped token, or an error token when empty
     */
    tokens::Token get() {
        // Return error token if empty
        if (input.empty()) {
            return tokens::Token{tags::error, "No more input tokens."};
        }

        tokens::Token t = input.back();
        input.pop_back();
        return t;
    }

    /**
     * \brief Peek at the token \c offset elements from the front of the input queue
     *
     * \param offset Offset of the sought element from the front of the queue
     * \return Token \c offset elements from the front of the input queue, or the error token when not present
     */
    tokens::Token peek(unsigned offset) {
        // Compute index
        auto index = static_cast<unsigned int>(input.size() - 1 - offset);

        // Return error token if not valid
        if (index < 0) {
            std::ostringstream message;
            message << "No token " << offset << " elements from the front of the input queue.";
            return tokens::Token{tags::error, message.str()};
        }

        return input[index];
    }
};

BOOST_AUTO_TEST_SUITE(Parser)

    BOOST_AUTO_TEST_SUITE(equals)

        BOOST_AUTO_TEST_SUITE(program)

            // Check different type
            BOOST_AUTO_TEST_CASE( different_type ) {
                // Prepare program and variable definition
                std::vector<std::unique_ptr<ast::Definition>> defs_a;
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                defs_a.push_back(std::make_unique<ast::VariableDefinition>("x", std::move(exp_a)));
                ast::Program a(std::move(defs_a));
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto b = std::make_unique<ast::VariableDefinition>("x", std::move(exp_b));

                // Check not equal
                BOOST_TEST_CHECK(!a.equals(b.get()), "Program equal to different type.");
            }

            // Check reflexivity
            BOOST_AUTO_TEST_CASE( reflexive ) {
                // Prepare program
                std::vector<std::unique_ptr<ast::Definition>> defs_a;
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                defs_a.push_back(std::make_unique<ast::VariableDefinition>("x", std::move(exp_a)));
                ast::Program a(std::move(defs_a));

                // Check reflexive property
                BOOST_TEST_CHECK(a.equals(&a), "Program equality isn't reflexive.");
            }

            // Check that two empty programs are equal
            BOOST_AUTO_TEST_CASE( empty ) {
                // Prepare two empty programs
                std::vector<std::unique_ptr<ast::Definition>> defs_a;
                std::vector<std::unique_ptr<ast::Definition>> defs_b;
                ast::Program a(std::move(defs_a));
                ast::Program b(std::move(defs_b));

                // Check equals
                BOOST_TEST_CHECK(a.equals(&b), "Empty programs not equal.");
            }

            // Check programs with matching definitions
            BOOST_AUTO_TEST_CASE( matching ) {
                // Prepare programs
                std::vector<std::unique_ptr<ast::Definition>> defs_a;
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                defs_a.push_back(std::make_unique<ast::VariableDefinition>("x", std::move(exp_a)));
                ast::Program a(std::move(defs_a));
                std::vector<std::unique_ptr<ast::Definition>> defs_b;
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                defs_b.push_back(std::make_unique<ast::VariableDefinition>("x", std::move(exp_b)));
                ast::Program b(std::move(defs_b));

                // Check equals
                BOOST_TEST_CHECK(a.equals(&b), "Identical programs not equal.");
            }

            // Check programs with different definitions
            BOOST_AUTO_TEST_CASE( different_definitions ) {
                // Prepare programs
                std::vector<std::unique_ptr<ast::Definition>> defs_a;
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                defs_a.push_back(std::make_unique<ast::VariableDefinition>("a", std::move(exp_a)));
                ast::Program a(std::move(defs_a));
                std::vector<std::unique_ptr<ast::Definition>> defs_b;
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                defs_b.push_back(std::make_unique<ast::VariableDefinition>("b", std::move(exp_b)));
                ast::Program b(std::move(defs_b));

                // Check equals
                BOOST_TEST_CHECK(!a.equals(&b), "Programs with different definitions are equal.");
            }

            // Check programs with matching definitions in order
            BOOST_AUTO_TEST_CASE( matching_order ) {
                // Prepare programs
                std::vector<std::unique_ptr<ast::Definition>> defs_a;
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                defs_a.push_back(std::make_unique<ast::VariableDefinition>("x", std::move(exp_a1)));
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                defs_a.push_back(std::make_unique<ast::VariableDefinition>("y", std::move(exp_a2)));
                ast::Program a(std::move(defs_a));
                std::vector<std::unique_ptr<ast::Definition>> defs_b;
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                defs_b.push_back(std::make_unique<ast::VariableDefinition>("x", std::move(exp_b1)));
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                defs_b.push_back(std::make_unique<ast::VariableDefinition>("y", std::move(exp_b2)));
                ast::Program b(std::move(defs_b));

                // Check equals
                BOOST_TEST_CHECK(a.equals(&b), "Programs with matching definitions in order are equal.");
            }

            // Check programs with matching definitions but out of order
            BOOST_AUTO_TEST_CASE( different_order ) {
                // Prepare programs
                std::vector<std::unique_ptr<ast::Definition>> defs_a;
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                defs_a.push_back(std::make_unique<ast::VariableDefinition>("x", std::move(exp_a1)));
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                defs_a.push_back(std::make_unique<ast::VariableDefinition>("y", std::move(exp_a2)));
                ast::Program a(std::move(defs_a));
                std::vector<std::unique_ptr<ast::Definition>> defs_b;
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                defs_b.push_back(std::make_unique<ast::VariableDefinition>("y", std::move(exp_b1)));
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                defs_b.push_back(std::make_unique<ast::VariableDefinition>("x", std::move(exp_b2)));
                ast::Program b(std::move(defs_b));

                // Check equals
                BOOST_TEST_CHECK(!a.equals(&b), "Programs with matching definitions out of order are equal.");
            }

        BOOST_AUTO_TEST_SUITE_END()

        BOOST_AUTO_TEST_SUITE(variable_definition)

            // Check different type
            BOOST_AUTO_TEST_CASE( different_type ) {
                // Prepare variable definition and a variable statement
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::VariableDefinition>("x", std::move(exp_a));
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto def_b = std::make_unique<ast::VariableDefinition>("x", std::move(exp_b));
                auto b = std::make_unique<ast::VariableStatement>(std::move(def_b));

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Variable definition equal to different type.");
            }

            // Check reflexivity
            BOOST_AUTO_TEST_CASE( reflexive ) {
                // Prepare definition
                auto exp = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::VariableDefinition>("x", std::move(exp));

                // Check reflexive property
                BOOST_TEST_CHECK(a->equals(a.get()), "Variable equality isn't reflexive.");
            }

            // Check matching definitions
            BOOST_AUTO_TEST_CASE( matching ) {
                // Prepare definitions
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::VariableDefinition>("x", std::move(exp_a));
                auto b = std::make_unique<ast::VariableDefinition>("x", std::move(exp_b));

                // Check equals
                BOOST_TEST_CHECK(a->equals(b.get()), "Identical variable definitions don't match.");
            }

            // Check different identifiers
            BOOST_AUTO_TEST_CASE( different_identifier ) {
                // Prepare definitions
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::VariableDefinition>("a", std::move(exp_a));
                auto b = std::make_unique<ast::VariableDefinition>("b", std::move(exp_b));

                // Check equals
                BOOST_TEST_CHECK(!a->equals(b.get()), "Variable definitions with different identifiers are equal.");
            }

            // Check different identifiers
            BOOST_AUTO_TEST_CASE( different_expression ) {
                // Prepare definitions
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_b = std::make_unique<ast::expressions::IdentifierExpression>("identifier");
                auto a = std::make_unique<ast::VariableDefinition>("x", std::move(exp_a));
                auto b = std::make_unique<ast::VariableDefinition>("x", std::move(exp_b));

                // Check equals
                BOOST_TEST_CHECK(!a->equals(b.get()), "Variable definitions with different expressions are equal.");
            }

        BOOST_AUTO_TEST_SUITE_END()

        BOOST_AUTO_TEST_SUITE(double_lit_expression)

            // Check different type
            BOOST_AUTO_TEST_CASE( different_type ) {
                // Prepare double lit expression and identifier expression
                auto a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto b = std::make_unique<ast::expressions::IdentifierExpression>("x");

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Double literal expression equal to different type.");
            }

            // Check reflexivity
            BOOST_AUTO_TEST_CASE( reflexivity ) {
                // Prepare double lit expression
                auto a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);

                // Check reflexive property
                BOOST_TEST_CHECK(a->equals(a.get()), "Double literal expression equality isn't reflexive.");
            }

            // Check matching values
            BOOST_AUTO_TEST_CASE( matching ) {
                // Prepare double lit expressions
                auto a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);

                // Check equal
                BOOST_TEST_CHECK(a->equals(b.get()), "Matching double literal expressions not equal.");
            }

            // Check different values
            BOOST_AUTO_TEST_CASE( different_value ) {
                // Prepare double lit expressions
                auto a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto b = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);

                // Check equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Double literal expressions with different values are equal.");
            }

        BOOST_AUTO_TEST_SUITE_END()

        BOOST_AUTO_TEST_SUITE(identifier_expression)

            // Check different type
            BOOST_AUTO_TEST_CASE( different_type ) {
                // Prepare identifier expression and double lit expression
                auto a = std::make_unique<ast::expressions::IdentifierExpression>("x");
                auto b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Identifier expression equal to different type.");
            }

            // Check reflexivity
            BOOST_AUTO_TEST_CASE( reflexivity ) {
                // Prepare identifier expression
                auto a = std::make_unique<ast::expressions::IdentifierExpression>("x");

                // Check reflexive property
                BOOST_TEST_CHECK(a->equals(a.get()), "Identifier expression equality isn't reflexive.");
            }

            // Check matching content
            BOOST_AUTO_TEST_CASE( matching ) {
                // Prepare identifier expressions
                auto a = std::make_unique<ast::expressions::IdentifierExpression>("x");
                auto b = std::make_unique<ast::expressions::IdentifierExpression>("x");

                // Check equal
                BOOST_TEST_CHECK(a->equals(b.get()), "Matching identifier expressions not equal.");
            }

            // Check different content
            BOOST_AUTO_TEST_CASE( different_value ) {
                // Prepare identifier expressions
                auto a = std::make_unique<ast::expressions::IdentifierExpression>("a");
                auto b = std::make_unique<ast::expressions::IdentifierExpression>("b");

                // Check equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Identifier expressions with different contents are equal.");
            }

        BOOST_AUTO_TEST_SUITE_END()

        BOOST_AUTO_TEST_SUITE(func_expression)

            // Check different type
            BOOST_AUTO_TEST_CASE( different_type ) {
                // Prepare function call expression and double lit expression
                std::vector<std::unique_ptr<ast::Expression>> arg_a;
                auto a = std::make_unique<ast::expressions::FuncExpression>("f", std::move(arg_a));
                auto b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Function call expression equal to different type.");
            }

            // Check reflexivity
            BOOST_AUTO_TEST_CASE( reflexivity ) {
                // Prepare func expression
                std::vector<std::unique_ptr<ast::Expression>> arg_a;
                auto a = std::make_unique<ast::expressions::FuncExpression>("f", std::move(arg_a));

                // Check reflexive property
                BOOST_TEST_CHECK(a->equals(a.get()), "Function call expression equality isn't reflexive.");
            }

            // Check matching function calls
            BOOST_AUTO_TEST_CASE( matching ) {
                // Prepare func expressions
                std::vector<std::unique_ptr<ast::Expression>> arg_a;
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                arg_a.push_back(std::move(exp_a));
                auto a = std::make_unique<ast::expressions::FuncExpression>("f", std::move(arg_a));
                std::vector<std::unique_ptr<ast::Expression>> arg_b;
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                arg_b.push_back(std::move(exp_b));
                auto b = std::make_unique<ast::expressions::FuncExpression>("f", std::move(arg_b));

                // Check equal
                BOOST_TEST_CHECK(a->equals(b.get()), "Matching function call expressions not equal.");
            }

            // Check different identifiers
            BOOST_AUTO_TEST_CASE( different_identifier ) {
                // Prepare func expressions
                std::vector<std::unique_ptr<ast::Expression>> arg_a;
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                arg_a.push_back(std::move(exp_a));
                auto a = std::make_unique<ast::expressions::FuncExpression>("f", std::move(arg_a));
                std::vector<std::unique_ptr<ast::Expression>> arg_b;
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                arg_b.push_back(std::move(exp_a));
                auto b = std::make_unique<ast::expressions::FuncExpression>("g", std::move(arg_b));

                // Check equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Function call expressions with different identifiers are equal.");
            }

            // Check different arguments
            BOOST_AUTO_TEST_CASE( different_argument ) {
                // Prepare func expressions
                std::vector<std::unique_ptr<ast::Expression>> arg_a;
                auto exp_a = std::make_unique<ast::expressions::IdentifierExpression>("x");
                arg_a.push_back(std::move(exp_a));
                auto a = std::make_unique<ast::expressions::FuncExpression>("f", std::move(arg_a));
                std::vector<std::unique_ptr<ast::Expression>> arg_b;
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                arg_b.push_back(std::move(exp_a));
                auto b = std::make_unique<ast::expressions::FuncExpression>("f", std::move(arg_b));

                // Check equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Function call expressions with different arguments are equal.");
            }

            // Check match arguments in order
            BOOST_AUTO_TEST_CASE( matching_order ) {
                // Prepare func expressions
                std::vector<std::unique_ptr<ast::Expression>> arg_a;
                auto exp_a1 = std::make_unique<ast::expressions::IdentifierExpression>("x");
                arg_a.push_back(std::move(exp_a1));
                auto exp_a2 = std::make_unique<ast::expressions::IdentifierExpression>("y");
                arg_a.push_back(std::move(exp_a2));
                auto a = std::make_unique<ast::expressions::FuncExpression>("f", std::move(arg_a));
                std::vector<std::unique_ptr<ast::Expression>> arg_b;
                auto exp_b1 = std::make_unique<ast::expressions::IdentifierExpression>("x");
                arg_b.push_back(std::move(exp_b1));
                auto exp_b2 = std::make_unique<ast::expressions::IdentifierExpression>("y");
                arg_b.push_back(std::move(exp_b2));
                auto b = std::make_unique<ast::expressions::FuncExpression>("f", std::move(arg_b));

                // Check equal
                BOOST_TEST_CHECK(a->equals(b.get()), "Function call expressions with matching arguments in order are not equal.");
            }

            // Check match arguments out of order
            BOOST_AUTO_TEST_CASE( different_order ) {
                // Prepare func expressions
                std::vector<std::unique_ptr<ast::Expression>> arg_a;
                auto exp_a1 = std::make_unique<ast::expressions::IdentifierExpression>("x");
                arg_a.push_back(std::move(exp_a1));
                auto exp_a2 = std::make_unique<ast::expressions::IdentifierExpression>("y");
                arg_a.push_back(std::move(exp_a2));
                auto a = std::make_unique<ast::expressions::FuncExpression>("f", std::move(arg_a));
                std::vector<std::unique_ptr<ast::Expression>> arg_b;
                auto exp_b1 = std::make_unique<ast::expressions::IdentifierExpression>("y");
                arg_b.push_back(std::move(exp_b1));
                auto exp_b2 = std::make_unique<ast::expressions::IdentifierExpression>("x");
                arg_b.push_back(std::move(exp_b2));
                auto b = std::make_unique<ast::expressions::FuncExpression>("f", std::move(arg_b));

                // Check equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Function call expressions with matching arguments out of order are equal.");
            }

        BOOST_AUTO_TEST_SUITE_END()

        BOOST_AUTO_TEST_SUITE(par_expression)

            // Check different type
            BOOST_AUTO_TEST_CASE( different_type ) {
                // Prepare par expression and double lit expression
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::expressions::ParExpression>(std::move(exp_a));
                auto b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Parenthesised expression equal to different type.");
            }

            // Check reflexivity
            BOOST_AUTO_TEST_CASE( reflexivity ) {
                // Prepare par expression
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::expressions::ParExpression>(std::move(exp_a));

                // Check reflexive property
                BOOST_TEST_CHECK(a->equals(a.get()), "Parenthesised expression equality isn't reflexive.");
            }

            // Check matching
            BOOST_AUTO_TEST_CASE( matching ) {
                // Prepare par expressions
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::expressions::ParExpression>(std::move(exp_a));
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto b = std::make_unique<ast::expressions::ParExpression>(std::move(exp_b));

                // Check equal
                BOOST_TEST_CHECK(a->equals(b.get()), "Identical parenthesised expressions not equal.");
            }

            // Check different expression
            BOOST_AUTO_TEST_CASE( different_expessions ) {
                // Prepare par expressions
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::expressions::ParExpression>(std::move(exp_a));
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto b = std::make_unique<ast::expressions::ParExpression>(std::move(exp_b));

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Parenthesised expressions with different contents are equal.");
            }

        BOOST_AUTO_TEST_SUITE_END()

        BOOST_AUTO_TEST_SUITE(neg_expression)

            // Check different type
            BOOST_AUTO_TEST_CASE( different_type ) {
                // Prepare neg expression and double lit expression
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::expressions::NegExpression>(std::move(exp_a));
                auto b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Negation expression equal to different type.");
            }

            // Check reflexivity
            BOOST_AUTO_TEST_CASE( reflexivity ) {
                // Prepare neg expression
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::expressions::NegExpression>(std::move(exp_a));

                // Check reflexive property
                BOOST_TEST_CHECK(a->equals(a.get()), "Negation expression equality isn't reflexive.");
            }

            // Check matching
            BOOST_AUTO_TEST_CASE( matching ) {
                // Prepare par expressions
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::expressions::NegExpression>(std::move(exp_a));
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto b = std::make_unique<ast::expressions::NegExpression>(std::move(exp_b));

                // Check equal
                BOOST_TEST_CHECK(a->equals(b.get()), "Identical negation expressions not equal.");
            }

            // Check different expression
            BOOST_AUTO_TEST_CASE( different_expessions ) {
                // Prepare neg expressions
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::expressions::NegExpression>(std::move(exp_a));
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto b = std::make_unique<ast::expressions::NegExpression>(std::move(exp_b));

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Negation expressions with different contents are equal.");
            }

        BOOST_AUTO_TEST_SUITE_END()

        BOOST_AUTO_TEST_SUITE(div_expression)

            // Check different type
            BOOST_AUTO_TEST_CASE( different_type ) {
                // Prepare div expression and double lit expression
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::DivExpression>(std::move(exp_a1), std::move(exp_a2));
                auto b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Division expression equal to different type.");
            }

            // Check reflexivity
            BOOST_AUTO_TEST_CASE( reflexivity ) {
                // Prepare div expression
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::DivExpression>(std::move(exp_a1), std::move(exp_a2));

                // Check reflexive property
                BOOST_TEST_CHECK(a->equals(a.get()), "Division expression equality isn't reflexive.");
            }

            // Check matching
            BOOST_AUTO_TEST_CASE( matching ) {
                // Prepare div expressions
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::DivExpression>(std::move(exp_a1), std::move(exp_a2));
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto b = std::make_unique<ast::expressions::DivExpression>(std::move(exp_b1), std::move(exp_b2));

                // Check equal
                BOOST_TEST_CHECK(a->equals(b.get()), "Identical division expressions not equal.");
            }

            // Check different left hand side
            BOOST_AUTO_TEST_CASE( different_lhs ) {
                // Prepare div expressions
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::DivExpression>(std::move(exp_a1), std::move(exp_a2));
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(3.0);
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto b = std::make_unique<ast::expressions::DivExpression>(std::move(exp_b1), std::move(exp_b2));

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Division expressions with different left hand sides are equal.");
            }

            // Check different right hand side
            BOOST_AUTO_TEST_CASE( different_rhs ) {
                // Prepare div expressions
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::DivExpression>(std::move(exp_a1), std::move(exp_a2));
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(3.0);
                auto b = std::make_unique<ast::expressions::DivExpression>(std::move(exp_b1), std::move(exp_b2));

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Division expressions with different right hand sides are equal.");
            }

        BOOST_AUTO_TEST_SUITE_END()
        
        BOOST_AUTO_TEST_SUITE(mul_expression)

            // Check different type
            BOOST_AUTO_TEST_CASE( different_type ) {
                // Prepare mul expression and double lit expression
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::MulExpression>(std::move(exp_a1), std::move(exp_a2));
                auto b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Multiplication expression equal to different type.");
            }

            // Check reflexivity
            BOOST_AUTO_TEST_CASE( reflexivity ) {
                // Prepare mul expression
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::MulExpression>(std::move(exp_a1), std::move(exp_a2));

                // Check reflexive property
                BOOST_TEST_CHECK(a->equals(a.get()), "Multiplication expression equality isn't reflexive.");
            }

            // Check matching
            BOOST_AUTO_TEST_CASE( matching ) {
                // Prepare mul expressions
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::MulExpression>(std::move(exp_a1), std::move(exp_a2));
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto b = std::make_unique<ast::expressions::MulExpression>(std::move(exp_b1), std::move(exp_b2));

                // Check equal
                BOOST_TEST_CHECK(a->equals(b.get()), "Identical multiplication expressions not equal.");
            }

            // Check different left hand side
            BOOST_AUTO_TEST_CASE( different_lhs ) {
                // Prepare mul expressions
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::MulExpression>(std::move(exp_a1), std::move(exp_a2));
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(3.0);
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto b = std::make_unique<ast::expressions::MulExpression>(std::move(exp_b1), std::move(exp_b2));

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Multiplication expressions with different left hand sides are equal.");
            }

            // Check different right hand side
            BOOST_AUTO_TEST_CASE( different_rhs ) {
                // Prepare mul expressions
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::MulExpression>(std::move(exp_a1), std::move(exp_a2));
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(3.0);
                auto b = std::make_unique<ast::expressions::MulExpression>(std::move(exp_b1), std::move(exp_b2));

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Multiplication expressions with different right hand sides are equal.");
            }
        
        BOOST_AUTO_TEST_SUITE_END()
        
        BOOST_AUTO_TEST_SUITE(sub_expression)

            // Check different type
            BOOST_AUTO_TEST_CASE( different_type ) {
                // Prepare sub expression and double lit expression
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::SubExpression>(std::move(exp_a1), std::move(exp_a2));
                auto b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Subtraction expression equal to different type.");
            }

            // Check reflexivity
            BOOST_AUTO_TEST_CASE( reflexivity ) {
                // Prepare sub expression
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::SubExpression>(std::move(exp_a1), std::move(exp_a2));

                // Check reflexive property
                BOOST_TEST_CHECK(a->equals(a.get()), "Subtraction expression equality isn't reflexive.");
            }

            // Check matching
            BOOST_AUTO_TEST_CASE( matching ) {
                // Prepare sub expressions
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::SubExpression>(std::move(exp_a1), std::move(exp_a2));
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto b = std::make_unique<ast::expressions::SubExpression>(std::move(exp_b1), std::move(exp_b2));

                // Check equal
                BOOST_TEST_CHECK(a->equals(b.get()), "Identical subtraction expressions not equal.");
            }

            // Check different left hand side
            BOOST_AUTO_TEST_CASE( different_lhs ) {
                // Prepare sub expressions
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::SubExpression>(std::move(exp_a1), std::move(exp_a2));
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(3.0);
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto b = std::make_unique<ast::expressions::SubExpression>(std::move(exp_b1), std::move(exp_b2));

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Subtraction expressions with different left hand sides are equal.");
            }

            // Check different right hand side
            BOOST_AUTO_TEST_CASE( different_rhs ) {
                // Prepare sub expressions
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::SubExpression>(std::move(exp_a1), std::move(exp_a2));
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(3.0);
                auto b = std::make_unique<ast::expressions::SubExpression>(std::move(exp_b1), std::move(exp_b2));

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Subtraction expressions with different right hand sides are equal.");
            }
        BOOST_AUTO_TEST_SUITE_END()

        BOOST_AUTO_TEST_SUITE(sum_expression)

            // Check different type
            BOOST_AUTO_TEST_CASE( different_type ) {
                // Prepare sum expression and double lit expression
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::SumExpression>(std::move(exp_a1), std::move(exp_a2));
                auto b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Sum expression equal to different type.");
            }

            // Check reflexivity
            BOOST_AUTO_TEST_CASE( reflexivity ) {
                // Prepare sum expression
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::SumExpression>(std::move(exp_a1), std::move(exp_a2));

                // Check reflexive property
                BOOST_TEST_CHECK(a->equals(a.get()), "Sum expression equality isn't reflexive.");
            }

            // Check matching
            BOOST_AUTO_TEST_CASE( matching ) {
                // Prepare sum expressions
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::SumExpression>(std::move(exp_a1), std::move(exp_a2));
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto b = std::make_unique<ast::expressions::SumExpression>(std::move(exp_b1), std::move(exp_b2));

                // Check equal
                BOOST_TEST_CHECK(a->equals(b.get()), "Identical sum expressions not equal.");
            }

            // Check different left hand side
            BOOST_AUTO_TEST_CASE( different_lhs ) {
                // Prepare sum expressions
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::SumExpression>(std::move(exp_a1), std::move(exp_a2));
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(3.0);
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto b = std::make_unique<ast::expressions::SumExpression>(std::move(exp_b1), std::move(exp_b2));

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Sum expressions with different left hand sides are equal.");
            }

            // Check different right hand side
            BOOST_AUTO_TEST_CASE( different_rhs ) {
                // Prepare sum expressions
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::SumExpression>(std::move(exp_a1), std::move(exp_a2));
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(3.0);
                auto b = std::make_unique<ast::expressions::SumExpression>(std::move(exp_b1), std::move(exp_b2));

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Sum expressions with different right hand sides are equal.");
            }
        
        BOOST_AUTO_TEST_SUITE_END()

        BOOST_AUTO_TEST_SUITE(mod_expression)

            // Check different type
            BOOST_AUTO_TEST_CASE( different_type ) {
                // Prepare mod expression and double lit expression
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::ModExpression>(std::move(exp_a1), std::move(exp_a2));
                auto b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Modulo expression equal to different type.");
            }

            // Check reflexivity
            BOOST_AUTO_TEST_CASE( reflexivity ) {
                // Prepare mod expression
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::ModExpression>(std::move(exp_a1), std::move(exp_a2));

                // Check reflexive property
                BOOST_TEST_CHECK(a->equals(a.get()), "Modulo expression equality isn't reflexive.");
            }

            // Check matching
            BOOST_AUTO_TEST_CASE( matching ) {
                // Prepare mod expressions
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::ModExpression>(std::move(exp_a1), std::move(exp_a2));
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto b = std::make_unique<ast::expressions::ModExpression>(std::move(exp_b1), std::move(exp_b2));

                // Check equal
                BOOST_TEST_CHECK(a->equals(b.get()), "Identical modulo expressions not equal.");
            }

            // Check different argument
            BOOST_AUTO_TEST_CASE( different_x ) {
                // Prepare mod expressions
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::ModExpression>(std::move(exp_a1), std::move(exp_a2));
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(3.0);
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto b = std::make_unique<ast::expressions::ModExpression>(std::move(exp_b1), std::move(exp_b2));

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Modulo expressions with different arguments are equal.");
            }

            // Check different parameter
            BOOST_AUTO_TEST_CASE( different_rhs ) {
                // Prepare mod expressions
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto a = std::make_unique<ast::expressions::ModExpression>(std::move(exp_a1), std::move(exp_a2));
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(3.0);
                auto b = std::make_unique<ast::expressions::ModExpression>(std::move(exp_b1), std::move(exp_b2));

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Modulo expressions with different parameters are equal.");
            }
        
        BOOST_AUTO_TEST_SUITE_END()

        BOOST_AUTO_TEST_SUITE(return_statement)

            // Check different type
            BOOST_AUTO_TEST_CASE( different_type ) {
                // Prepare return statement and double lit expression
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::ReturnStatement>(std::move(exp_a));
                auto b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Return statement equal to different type.");
            }

            // Check reflexivity
            BOOST_AUTO_TEST_CASE( reflexivity ) {
                // Prepare return statement
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::ReturnStatement>(std::move(exp_a));

                // Check reflexive property
                BOOST_TEST_CHECK(a->equals(a.get()), "Return statement equality isn't reflexive.");
            }

            // Check matching
            BOOST_AUTO_TEST_CASE( matching ) {
                // Prepare return statements
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::ReturnStatement>(std::move(exp_a));
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto b = std::make_unique<ast::ReturnStatement>(std::move(exp_b));

                // Check equal
                BOOST_TEST_CHECK(a->equals(b.get()), "Identical return statements not equal.");
            }

            // Check different expression
            BOOST_AUTO_TEST_CASE( different_expessions ) {
                // Prepare return statements
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::ReturnStatement>(std::move(exp_a));
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto b = std::make_unique<ast::ReturnStatement>(std::move(exp_b));

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Return statements with different contents are equal.");
            }

        BOOST_AUTO_TEST_SUITE_END()

        BOOST_AUTO_TEST_SUITE(standalone_statement)

            // Check different type
            BOOST_AUTO_TEST_CASE( different_type ) {
                // Prepare standalone statement and double lit expression
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::StandaloneStatement>(std::move(exp_a));
                auto b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Standalone statement equal to different type.");
            }

            // Check reflexivity
            BOOST_AUTO_TEST_CASE( reflexivity ) {
                // Prepare standalone statement
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::StandaloneStatement>(std::move(exp_a));

                // Check reflexive property
                BOOST_TEST_CHECK(a->equals(a.get()), "Standalone statement equality isn't reflexive.");
            }

            // Check matching
            BOOST_AUTO_TEST_CASE( matching ) {
                // Prepare standalone statements
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::StandaloneStatement>(std::move(exp_a));
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto b = std::make_unique<ast::StandaloneStatement>(std::move(exp_b));

                // Check equal
                BOOST_TEST_CHECK(a->equals(b.get()), "Identical standalone statements not equal.");
            }

            // Check different expression
            BOOST_AUTO_TEST_CASE( different_expessions ) {
                // Prepare standalone statements
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto a = std::make_unique<ast::StandaloneStatement>(std::move(exp_a));
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto b = std::make_unique<ast::StandaloneStatement>(std::move(exp_b));

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Standalone statements with different contents are equal.");
            }

        BOOST_AUTO_TEST_SUITE_END()

        BOOST_AUTO_TEST_SUITE(function_definition)

            // Check different type
            BOOST_AUTO_TEST_CASE( different_type ) {
                // Prepare function definition and double literal expression
                std::vector<ast::Identifier> arg_a{"x"};
                std::vector<std::unique_ptr<ast::Statement>> body_a;
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto sta_a = std::make_unique<ast::ReturnStatement>(std::move(exp_a));
                body_a.push_back(std::move(sta_a));
                auto a = std::make_unique<ast::FunctionDefinition>("f", std::move(arg_a), std::move(body_a));
                auto b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Function definition equal to different type.");
            }

            // Check reflexivity
            BOOST_AUTO_TEST_CASE( reflexivity ) {
                // Prepare function definition
                std::vector<ast::Identifier> arg_a{"x"};
                std::vector<std::unique_ptr<ast::Statement>> body_a;
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto sta_a = std::make_unique<ast::ReturnStatement>(std::move(exp_a));
                body_a.push_back(std::move(sta_a));
                auto a = std::make_unique<ast::FunctionDefinition>("f", std::move(arg_a), std::move(body_a));

                // Check reflexive property
                BOOST_TEST_CHECK(a->equals(a.get()), "Function definition isn't reflexive.");
            }

            // Check matching
            BOOST_AUTO_TEST_CASE( matching ) {
                // Prepare function definition
                std::vector<ast::Identifier> arg_a{"x"};
                std::vector<std::unique_ptr<ast::Statement>> body_a;
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto sta_a = std::make_unique<ast::ReturnStatement>(std::move(exp_a));
                body_a.push_back(std::move(sta_a));
                auto a = std::make_unique<ast::FunctionDefinition>("f", std::move(arg_a), std::move(body_a));
                std::vector<ast::Identifier> arg_b{"x"};
                std::vector<std::unique_ptr<ast::Statement>> body_b;
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto sta_b = std::make_unique<ast::ReturnStatement>(std::move(exp_b));
                body_b.push_back(std::move(sta_b));
                auto b = std::make_unique<ast::FunctionDefinition>("f", std::move(arg_b), std::move(body_b));

                // Check equal
                BOOST_TEST_CHECK(a->equals(a.get()), "Identical function definitions not equal.");
            }

            // Check different identifier
            BOOST_AUTO_TEST_CASE( different_identifier ) {
                // Prepare function definition
                std::vector<ast::Identifier> arg_a{"x"};
                std::vector<std::unique_ptr<ast::Statement>> body_a;
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto sta_a = std::make_unique<ast::ReturnStatement>(std::move(exp_a));
                body_a.push_back(std::move(sta_a));
                auto a = std::make_unique<ast::FunctionDefinition>("f", std::move(arg_a), std::move(body_a));
                std::vector<ast::Identifier> arg_b{"x"};
                std::vector<std::unique_ptr<ast::Statement>> body_b;
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto sta_b = std::make_unique<ast::ReturnStatement>(std::move(exp_b));
                body_b.push_back(std::move(sta_b));
                auto b = std::make_unique<ast::FunctionDefinition>("g", std::move(arg_b), std::move(body_b));

                // Check equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Function definitions with different identifiers are equal.");
            }

            // Check different arguments
            BOOST_AUTO_TEST_CASE( different_argument ) {
                // Prepare function definition
                std::vector<ast::Identifier> arg_a{"x"};
                std::vector<std::unique_ptr<ast::Statement>> body_a;
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto sta_a = std::make_unique<ast::ReturnStatement>(std::move(exp_a));
                body_a.push_back(std::move(sta_a));
                auto a = std::make_unique<ast::FunctionDefinition>("f", std::move(arg_a), std::move(body_a));
                std::vector<ast::Identifier> arg_b{"y"};
                std::vector<std::unique_ptr<ast::Statement>> body_b;
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto sta_b = std::make_unique<ast::ReturnStatement>(std::move(exp_b));
                body_b.push_back(std::move(sta_b));
                auto b = std::make_unique<ast::FunctionDefinition>("f", std::move(arg_b), std::move(body_b));

                // Check equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Function definitions with different arguments are equal.");
            }

            // Check matching arguments in order
            BOOST_AUTO_TEST_CASE( matching_argument_order ) {
                // Prepare function definition
                std::vector<ast::Identifier> arg_a{"x", "y"};
                std::vector<std::unique_ptr<ast::Statement>> body_a;
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto sta_a = std::make_unique<ast::ReturnStatement>(std::move(exp_a));
                body_a.push_back(std::move(sta_a));
                auto a = std::make_unique<ast::FunctionDefinition>("f", std::move(arg_a), std::move(body_a));
                std::vector<ast::Identifier> arg_b{"x", "y"};
                std::vector<std::unique_ptr<ast::Statement>> body_b;
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto sta_b = std::make_unique<ast::ReturnStatement>(std::move(exp_b));
                body_b.push_back(std::move(sta_b));
                auto b = std::make_unique<ast::FunctionDefinition>("f", std::move(arg_b), std::move(body_b));

                // Check equal
                BOOST_TEST_CHECK(a->equals(b.get()), "Function definitions with identical arguments not equal.");
            }

            // Check matching arguments out of order
            BOOST_AUTO_TEST_CASE( different_argument_order ) {
                // Prepare function definition
                std::vector<ast::Identifier> arg_a{"x", "y"};
                std::vector<std::unique_ptr<ast::Statement>> body_a;
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto sta_a = std::make_unique<ast::ReturnStatement>(std::move(exp_a));
                body_a.push_back(std::move(sta_a));
                auto a = std::make_unique<ast::FunctionDefinition>("f", std::move(arg_a), std::move(body_a));
                std::vector<ast::Identifier> arg_b{"y", "x"};
                std::vector<std::unique_ptr<ast::Statement>> body_b;
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto sta_b = std::make_unique<ast::ReturnStatement>(std::move(exp_b));
                body_b.push_back(std::move(sta_b));
                auto b = std::make_unique<ast::FunctionDefinition>("f", std::move(arg_b), std::move(body_b));

                // Check equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Function definitions with arguments out of order are equal.");
            }

            // Check different body
            BOOST_AUTO_TEST_CASE( different_body ) {
                // Prepare function definition
                std::vector<ast::Identifier> arg_a{"x"};
                std::vector<std::unique_ptr<ast::Statement>> body_a;
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto sta_a = std::make_unique<ast::ReturnStatement>(std::move(exp_a));
                body_a.push_back(std::move(sta_a));
                auto a = std::make_unique<ast::FunctionDefinition>("f", std::move(arg_a), std::move(body_a));
                std::vector<ast::Identifier> arg_b{"x"};
                std::vector<std::unique_ptr<ast::Statement>> body_b;
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto sta_b = std::make_unique<ast::ReturnStatement>(std::move(exp_b));
                body_b.push_back(std::move(sta_b));
                auto b = std::make_unique<ast::FunctionDefinition>("f", std::move(arg_b), std::move(body_b));

                // Check equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Function definitions with different body are equal.");
            }

            // Check matching body in order
            BOOST_AUTO_TEST_CASE( matching_body_order ) {
                // Prepare function definition
                std::vector<ast::Identifier> arg_a{"x"};
                std::vector<std::unique_ptr<ast::Statement>> body_a;
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto sta_a1 = std::make_unique<ast::ReturnStatement>(std::move(exp_a1));
                auto sta_a2 = std::make_unique<ast::ReturnStatement>(std::move(exp_a2));
                body_a.push_back(std::move(sta_a1));
                body_a.push_back(std::move(sta_a2));
                auto a = std::make_unique<ast::FunctionDefinition>("f", std::move(arg_a), std::move(body_a));
                std::vector<ast::Identifier> arg_b{"x"};
                std::vector<std::unique_ptr<ast::Statement>> body_b;
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto sta_b1 = std::make_unique<ast::ReturnStatement>(std::move(exp_b1));
                auto sta_b2 = std::make_unique<ast::ReturnStatement>(std::move(exp_b2));
                body_b.push_back(std::move(sta_b1));
                body_b.push_back(std::move(sta_b2));
                auto b = std::make_unique<ast::FunctionDefinition>("f", std::move(arg_b), std::move(body_b));

                // Check equal
                BOOST_TEST_CHECK(a->equals(b.get()), "Function definitions with identical body not equal.");
            }

            // Check matching body out of order
            BOOST_AUTO_TEST_CASE( different_body_order ) {
                // Prepare function definition
                std::vector<ast::Identifier> arg_a{"x"};
                std::vector<std::unique_ptr<ast::Statement>> body_a;
                auto exp_a1 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto exp_a2 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto sta_a1 = std::make_unique<ast::ReturnStatement>(std::move(exp_a1));
                auto sta_a2 = std::make_unique<ast::ReturnStatement>(std::move(exp_a2));
                body_a.push_back(std::move(sta_a1));
                body_a.push_back(std::move(sta_a2));
                auto a = std::make_unique<ast::FunctionDefinition>("f", std::move(arg_a), std::move(body_a));
                std::vector<ast::Identifier> arg_b{"x"};
                std::vector<std::unique_ptr<ast::Statement>> body_b;
                auto exp_b1 = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto exp_b2 = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto sta_b1 = std::make_unique<ast::ReturnStatement>(std::move(exp_b1));
                auto sta_b2 = std::make_unique<ast::ReturnStatement>(std::move(exp_b2));
                body_b.push_back(std::move(sta_b1));
                body_b.push_back(std::move(sta_b2));
                auto b = std::make_unique<ast::FunctionDefinition>("f", std::move(arg_b), std::move(body_b));

                // Check equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Function definitions with body out of order are equal.");
            }

        BOOST_AUTO_TEST_SUITE_END()

        BOOST_AUTO_TEST_SUITE(variable_statement)

            // Check different type
            BOOST_AUTO_TEST_CASE( different_type ) {
                // Prepare variable statement and double lit expression
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto def_a = std::make_unique<ast::VariableDefinition>("x", std::move(exp_a));
                auto a = std::make_unique<ast::VariableStatement>(std::move(def_a));
                auto b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Variable statement equal to different type.");
            }

            // Check reflexivity
            BOOST_AUTO_TEST_CASE( reflexivity ) {
                // Prepare variable statement
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto def_a = std::make_unique<ast::VariableDefinition>("x", std::move(exp_a));
                auto a = std::make_unique<ast::VariableStatement>(std::move(def_a));

                // Check reflexive property
                BOOST_TEST_CHECK(a->equals(a.get()), "Variable statement equality isn't reflexive.");
            }

            // Check matching
            BOOST_AUTO_TEST_CASE( matching ) {
                // Prepare variable statements
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto def_a = std::make_unique<ast::VariableDefinition>("x", std::move(exp_a));
                auto a = std::make_unique<ast::VariableStatement>(std::move(def_a));
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto def_b = std::make_unique<ast::VariableDefinition>("x", std::move(exp_b));
                auto b = std::make_unique<ast::VariableStatement>(std::move(def_b));

                // Check equal
                BOOST_TEST_CHECK(a->equals(b.get()), "Identical variable statements not equal.");
            }

            // Check different expression
            BOOST_AUTO_TEST_CASE( different_expessions ) {
                // Prepare variable statements
                auto exp_a = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto def_a = std::make_unique<ast::VariableDefinition>("x", std::move(exp_a));
                auto a = std::make_unique<ast::VariableStatement>(std::move(def_a));
                auto exp_b = std::make_unique<ast::expressions::DoubleLitExpression>(2.0);
                auto def_b = std::make_unique<ast::VariableDefinition>("x", std::move(exp_b));
                auto b = std::make_unique<ast::VariableStatement>(std::move(def_b));

                // Check not equal
                BOOST_TEST_CHECK(!a->equals(b.get()), "Variable statements with different contents are equal.");
            }

        BOOST_AUTO_TEST_SUITE_END()

    BOOST_AUTO_TEST_SUITE_END()

    BOOST_AUTO_TEST_SUITE(parsing)

        BOOST_AUTO_TEST_CASE( variable_definition ) {
            // Construct fixture
            QueuesFixture qf;
            qf.input = {
                    {tags::identifier, "x"},
                    {tags::assign, {}},
                    {tags::double_literal, "1.0"},
                    {tags::semicolon, {}},
                    {tags::end_of_input, {}}
            };
            std::reverse(qf.input.begin(), qf.input.end());

            // Correct result
            auto value = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
            auto var_def = std::make_unique<ast::VariableDefinition>("x", std::move(value));
            std::vector<std::unique_ptr<ast::Definition>> corr_defs{};
            corr_defs.push_back(std::move(var_def));
            ast::Program correct(std::move(corr_defs));

            // Parse
            ast::Program result = parser::parse(qf.get_f, qf.peek_f);

            // Compare
            if (!result.equals(&correct)) {
                // When wrong, display correct tree
                boost::unit_test::unit_test_log << "Correct tree:\n" << ast::util::print_ast(&correct);
            }
            BOOST_TEST_CHECK(result.equals(&correct), "Resulting tree:\n" << ast::util::print_ast(&result));
        }

    BOOST_AUTO_TEST_SUITE_END()

    //TODO case for program with no definitions

BOOST_AUTO_TEST_SUITE_END()
