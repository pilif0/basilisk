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

            // Check that two empty programs are equal
            BOOST_AUTO_TEST_CASE( empty ) {
                // Prepare two empty programs
                std::vector<std::unique_ptr<ast::Definition>> defs_a;
                std::vector<std::unique_ptr<ast::Definition>> defs_b;
                ast::Program a(std::move(defs_a));
                ast::Program b(std::move(defs_b));

                // Check equals
                BOOST_TEST_CHECK(a.equals(&b), "Empty programs not equal");
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
