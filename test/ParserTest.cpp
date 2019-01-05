/** \file ParserTest.cpp
 * Parser test module
 *
 * \author Filip Smola
 */
#define BOOST_TEST_MODULE "Parser"

#include <basilisk/Parser.h>
#include <basilisk/Tokens.h>
#include <basilisk/AST.h>
#include <basilisk/AST_util.h>
#include <basilisk/Lexer.h>

#include <boost/test/unit_test.hpp>

#include <vector>

namespace tokens = basilisk::tokens;
namespace tags = basilisk::tokens::tags;
namespace parser = basilisk::parser;
namespace ast = basilisk::ast;

//TODO make all check messages properly descriptive

//! Fixture that sets up two queues for use as lexer input and output
struct QueuesFixture {
    //! Token queue type
    // Note: queue front is the vector back
    typedef std::vector<tokens::Token> token_queue_t;

    //! Parser input queue in reverse order (back of this is front of queue)
    token_queue_t input;
    //! Parser compatible get function reference
    const parser::get_f_t get_f = std::bind(&QueuesFixture::get, this);
    //! Parser compatible peek function reference
    const parser::peek_f_t peek_f = std::bind(&QueuesFixture::peek, this, std::placeholders::_1);

    /**
     * \brief Construct fixture with empty input buffer
     */
    QueuesFixture() {}

    /**
     * \brief Construct fixture by lexing a string (ignoring any exceptions raised)
     *
     * /param src Source string to lex
     */
    explicit QueuesFixture(const std::string &src) {
        // Prepare iterator
        auto iterator = src.begin();
        auto end = src.end();

        // Prepare functions
        basilisk::lexer::get_function_t lexer_get = [&iterator, &end](){
            // Return null character when no more input
            if (iterator >= end) {
                return '\0';
            }

            // Pop and return character
            auto buffer = *iterator;
            iterator++;
            return buffer;
        };
        basilisk::lexer::peek_function_t lexer_peek = [&iterator, &end](){
            // Return null character when no more input
            if (iterator >= end) {
                return '\0';
            }

            // Return character
            return *iterator;
        };
        basilisk::lexer::append_function_t lexer_append = [this](tokens::Token t){ this->input.push_back(t); };

        // Lex
        try {
            basilisk::lexer::lex(lexer_get, lexer_peek, lexer_append);
        } catch (std::exception &/*e*/) {}

        // Reverse order to move top of the queue to the back of the vecter
        std::reverse(input.begin(), input.end());
    }

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
    BOOST_AUTO_TEST_SUITE(individual)
        BOOST_AUTO_TEST_SUITE(expression)
            BOOST_AUTO_TEST_SUITE(mod)
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(sum)
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(sub)
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(mul)
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(div)
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(neg)
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(literal_double)
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(parenthesised)
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(identifier)
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(function_call)
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(definition)
            BOOST_AUTO_TEST_SUITE_END()
        BOOST_AUTO_TEST_SUITE_END()

        BOOST_AUTO_TEST_SUITE(statement)
            BOOST_AUTO_TEST_SUITE(return_kw)
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(standalone)
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(variable)
            BOOST_AUTO_TEST_SUITE_END()
        BOOST_AUTO_TEST_SUITE_END()

        BOOST_AUTO_TEST_SUITE(definition)
            // Check correctly recognising function definition
            BOOST_AUTO_TEST_CASE( pick_function ) {
                // Construct fixture
                QueuesFixture qf("f() {}");

                // Correct result
                std::vector<std::unique_ptr<ast::Statement>> body;
                std::vector<ast::Identifier> args;
                auto correct = std::make_unique<ast::FunctionDefinition>("f", std::move(args), std::move(body));

                // Parse
                auto result = parser::DefinitionParser(qf.get_f, qf.peek_f).definition();

                // Compare
                if (!result->equals(correct.get())) {
                    // When wrong, display correct tree
                    boost::unit_test::unit_test_log << "Correct tree:\n" << ast::util::print_ast(correct.get());
                    boost::unit_test::unit_test_log << "Resulting tree:\n" << ast::util::print_ast(result.get());
                }
                BOOST_TEST_CHECK(result->equals(correct.get()), "Parsed tree must match hard-coded correct tree.");
            }

            // Check correctly recognising variable definition
            BOOST_AUTO_TEST_CASE( pick_variable ) {
                // Construct fixture
                QueuesFixture qf("x = 1.0;");

                // Correct result
                auto value = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto var_stmt = std::make_unique<ast::VariableStatement>("x", std::move(value));
                auto correct = std::make_unique<ast::VariableDefinition>(std::move(var_stmt));

                // Parse
                auto result = parser::DefinitionParser(qf.get_f, qf.peek_f).definition();

                // Compare
                if (!result->equals(correct.get())) {
                    // When wrong, display correct tree
                    boost::unit_test::unit_test_log << "Correct tree:\n" << ast::util::print_ast(correct.get());
                    boost::unit_test::unit_test_log << "Resulting tree:\n" << ast::util::print_ast(result.get());
                }
                BOOST_TEST_CHECK(result->equals(correct.get()), "Parsed tree must match hard-coded correct tree.");
            }

            // Check exception on unexpected token
            BOOST_AUTO_TEST_CASE( unexpected_token ) {
                // Construct fixture
                QueuesFixture qf("1.0");

                // Check exception on parse
                BOOST_CHECK_THROW(parser::DefinitionParser(qf.get_f, qf.peek_f).definition(), parser::ParserException);
            }

            BOOST_AUTO_TEST_SUITE(function_definition)
                // Check correct
                BOOST_AUTO_TEST_CASE( correct ) {
                    // Construct fixture
                    QueuesFixture qf("f(x) { return 1.0; }");

                    // Correct result
                    auto value = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                    auto ret = std::make_unique<ast::ReturnStatement>(std::move(value));
                    std::vector<std::unique_ptr<ast::Statement>> body;
                    body.push_back(std::move(ret));
                    std::vector<ast::Identifier> args{"x"};
                    auto correct = std::make_unique<ast::FunctionDefinition>("f", std::move(args), std::move(body));

                    // Parse
                    auto result = parser::DefinitionParser(qf.get_f, qf.peek_f).function();

                    // Compare
                    if (!result->equals(correct.get())) {
                        // When wrong, display correct tree
                        boost::unit_test::unit_test_log << "Correct tree:\n" << ast::util::print_ast(correct.get());
                        boost::unit_test::unit_test_log << "Resulting tree:\n" << ast::util::print_ast(result.get());
                    }
                    BOOST_TEST_CHECK(result->equals(correct.get()), "Parsed tree must match hard-coded correct tree.");
                }

                // Check correct with no arguments and no body
                BOOST_AUTO_TEST_CASE( correct_empty ) {
                    // Construct fixture
                    QueuesFixture qf("f() {}");

                    // Correct result
                    std::vector<std::unique_ptr<ast::Statement>> body;
                    std::vector<ast::Identifier> args;
                    auto correct = std::make_unique<ast::FunctionDefinition>("f", std::move(args), std::move(body));

                    // Parse
                    auto result = parser::DefinitionParser(qf.get_f, qf.peek_f).function();

                    // Compare
                    if (!result->equals(correct.get())) {
                        // When wrong, display correct tree
                        boost::unit_test::unit_test_log << "Correct tree:\n" << ast::util::print_ast(correct.get());
                        boost::unit_test::unit_test_log << "Resulting tree:\n" << ast::util::print_ast(result.get());
                    }
                    BOOST_TEST_CHECK(result->equals(correct.get()), "Parsed tree must match hard-coded correct tree.");
                }

                // Check multiple arguments
                BOOST_AUTO_TEST_CASE( correct_multiple_arguments ) {
                    // Construct fixture
                    QueuesFixture qf("f(x, y) { return 1.0; }");

                    // Correct result
                    auto value = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                    auto ret = std::make_unique<ast::ReturnStatement>(std::move(value));
                    std::vector<std::unique_ptr<ast::Statement>> body;
                    body.push_back(std::move(ret));
                    std::vector<ast::Identifier> args{"x", "y"};
                    auto correct = std::make_unique<ast::FunctionDefinition>("f", std::move(args), std::move(body));

                    // Parse
                    auto result = parser::DefinitionParser(qf.get_f, qf.peek_f).function();

                    // Compare
                    if (!result->equals(correct.get())) {
                        // When wrong, display correct tree
                        boost::unit_test::unit_test_log << "Correct tree:\n" << ast::util::print_ast(correct.get());
                        boost::unit_test::unit_test_log << "Resulting tree:\n" << ast::util::print_ast(result.get());
                    }
                    BOOST_TEST_CHECK(result->equals(correct.get()), "Parsed tree must match hard-coded correct tree.");
                }

                // Check exception on unexpected token instead of identifier
                BOOST_AUTO_TEST_CASE( unexpected_token_identifier ) {
                    // Construct fixture
                    QueuesFixture qf("1.0");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::DefinitionParser(qf.get_f, qf.peek_f).definition(), parser::ParserException);
                }

                // Check exception on unexpected token instead of LPAR
                BOOST_AUTO_TEST_CASE( unexpected_token_lpar ) {
                    // Construct fixture
                    QueuesFixture qf("f 1.0");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::DefinitionParser(qf.get_f, qf.peek_f).definition(), parser::ParserException);
                }

                // Check exception on unexpected token instead of identifier in args
                BOOST_AUTO_TEST_CASE( unexpected_token_args_identifier ) {
                    // Construct fixture
                    QueuesFixture qf("f ( 1.0");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::DefinitionParser(qf.get_f, qf.peek_f).definition(), parser::ParserException);
                }

                // Check exception on unexpected token instead of COMMA or RPAR in args
                BOOST_AUTO_TEST_CASE( unexpected_token_args_comma ) {
                    // Construct fixture
                    QueuesFixture qf("f ( x 1.0");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::DefinitionParser(qf.get_f, qf.peek_f).definition(), parser::ParserException);
                }

                // Check exception on multiple arguments without separating comma
                BOOST_AUTO_TEST_CASE( multiple_arguments_no_comma ) {
                    // Construct fixture
                    QueuesFixture qf("f (x y) {}");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::DefinitionParser(qf.get_f, qf.peek_f).definition(), parser::ParserException);
                }

                // Check exception on unexpected token instead of LBRAC
                BOOST_AUTO_TEST_CASE( unexpected_token_lbrac ) {
                    // Construct fixture
                    QueuesFixture qf("f () 1.0");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::DefinitionParser(qf.get_f, qf.peek_f).definition(), parser::ParserException);
                }

                // Check exception on unexpected token instead of RBRAC
                BOOST_AUTO_TEST_CASE( unexpected_token_rbrac ) {
                    // Construct fixture
                    QueuesFixture qf("f () { 1.0");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::DefinitionParser(qf.get_f, qf.peek_f).definition(), parser::ParserException);
                }
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(variable_definition)
                // Check correct
                BOOST_AUTO_TEST_CASE( correct ) {
                    // Construct fixture
                    QueuesFixture qf("x = 1.0;");

                    // Correct result
                    auto value = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                    auto var_stmt = std::make_unique<ast::VariableStatement>("x", std::move(value));
                    auto correct = std::make_unique<ast::VariableDefinition>(std::move(var_stmt));

                    // Parse
                    auto result = parser::DefinitionParser(qf.get_f, qf.peek_f).variable();

                    // Compare
                    if (!result->equals(correct.get())) {
                        // When wrong, display correct tree
                        boost::unit_test::unit_test_log << "Correct tree:\n" << ast::util::print_ast(correct.get());
                        boost::unit_test::unit_test_log << "Resulting tree:\n" << ast::util::print_ast(result.get());
                    }
                    BOOST_TEST_CHECK(result->equals(correct.get()), "Parsed tree must match hard-coded correct tree.");
                }

            BOOST_AUTO_TEST_SUITE_END()
        BOOST_AUTO_TEST_SUITE_END()

        BOOST_AUTO_TEST_SUITE(program)
            // Check correct
            BOOST_AUTO_TEST_CASE( correct ) {
                // Construct fixture
                QueuesFixture qf("x = 1.0;\n"
                                 "\n"
                                 "f() {\n"
                                 "    return x;\n"
                                 "}");

                // Correct result
                std::vector<std::unique_ptr<ast::Definition>> corr_defs;
                {
                    // x = 1.0;
                    auto value = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                    auto stmt = std::make_unique<ast::VariableStatement>("x", std::move(value));
                    auto def = std::make_unique<ast::VariableDefinition>(std::move(stmt));
                    corr_defs.push_back(std::move(def));
                }
                {
                    // f() { return x; }
                    auto value = std::make_unique<ast::expressions::IdentifierExpression>("x");
                    auto ret = std::make_unique<ast::ReturnStatement>(std::move(value));
                    std::vector<std::unique_ptr<ast::Statement>> body;
                    body.push_back(std::move(ret));
                    std::vector<ast::Identifier> args;
                    auto def = std::make_unique<ast::FunctionDefinition>("f", std::move(args), std::move(body));
                    corr_defs.push_back(std::move(def));
                }
                ast::Program correct(std::move(corr_defs));

                // Parse
                auto result = parser::ProgramParser(qf.get_f, qf.peek_f).program();

                // Compare
                if (!result.equals(&correct)) {
                    // When wrong, display correct tree
                    boost::unit_test::unit_test_log << "Correct tree:\n" << ast::util::print_ast(&correct);
                    boost::unit_test::unit_test_log << "Resulting tree:\n" << ast::util::print_ast(&result);
                }
                BOOST_TEST_CHECK(result.equals(&correct), "Parsed tree must match hard-coded correct tree.");
            }

            // Check exception on lexer error token
            BOOST_AUTO_TEST_CASE( lexer_error ) {
                // Construct fixture
                QueuesFixture qf("1");

                // Check exception on parse
                BOOST_CHECK_THROW(parser::ProgramParser(qf.get_f, qf.peek_f).program(), parser::ParserException);
            }

            // Check exception on unexpected token
            BOOST_AUTO_TEST_CASE( unexpected_token ) {
                // Construct fixture
                QueuesFixture qf("1.0");

                // Check exception on parse
                BOOST_CHECK_THROW(parser::ProgramParser(qf.get_f, qf.peek_f).program(), parser::ParserException);
            }

            // Check exception on no definitions
            BOOST_AUTO_TEST_CASE( no_definitions ) {
                // Construct fixture
                QueuesFixture qf("");

                // Check exception on parse
                BOOST_CHECK_THROW(parser::ProgramParser(qf.get_f, qf.peek_f).program(), parser::ParserException);
            }
        BOOST_AUTO_TEST_SUITE_END()
    BOOST_AUTO_TEST_SUITE_END()

    BOOST_AUTO_TEST_CASE( example_program ) {
        // Construct fixture
        QueuesFixture qf("pi = 3.14;\n"
                         "\n"
                         "get_pi() {\n"
                         "    return pi;\n"
                         "}\n"
                         "\n"
                         "write(x) {\n"
                         "    println(x);\n"
                         "}\n"
                         "\n"
                         "main() {\n"
                         "    write(get_pi());\n"
                         "    pi = 3.0;\n"
                         "    write(pi);\n"
                         "    write(1.0 + (3.0 * 4.0) % 5.0);\n"
                         "    return 0.0;\n"
                         "}");

        // Correct result
        std::vector<std::unique_ptr<ast::Definition>> corr_defs;
        {
            // pi = 3.14;
            auto value = std::make_unique<ast::expressions::DoubleLitExpression>(3.14);
            auto stmt = std::make_unique<ast::VariableStatement>("pi", std::move(value));
            auto def = std::make_unique<ast::VariableDefinition>(std::move(stmt));
            corr_defs.push_back(std::move(def));
        }
        {
            // get_pi() { return pi; }
            auto value = std::make_unique<ast::expressions::IdentifierExpression>("pi");
            auto ret = std::make_unique<ast::ReturnStatement>(std::move(value));
            std::vector<std::unique_ptr<ast::Statement>> body;
            body.push_back(std::move(ret));
            std::vector<ast::Identifier> args;
            auto def = std::make_unique<ast::FunctionDefinition>("get_pi", std::move(args), std::move(body));
            corr_defs.push_back(std::move(def));
        }
        {
            // write(x) { println(x); }
            auto param = std::make_unique<ast::expressions::IdentifierExpression>("x");
            std::vector<std::unique_ptr<ast::Expression>> in_args;
            in_args.push_back(std::move(param));
            auto value = std::make_unique<ast::expressions::FuncExpression>("println", std::move(in_args));
            auto stmt = std::make_unique<ast::StandaloneStatement>(std::move(value));
            std::vector<std::unique_ptr<ast::Statement>> body;
            body.push_back(std::move(stmt));
            std::vector<ast::Identifier> args{"x"};
            auto def = std::make_unique<ast::FunctionDefinition>("write", std::move(args), std::move(body));
            corr_defs.push_back(std::move(def));
        }
        {
            // main() { write(get_pi()); pi = 3.0; write(pi); write(1.0 + (3.0 * 4.0) % 5.0); return 0.0; }
            std::vector<std::unique_ptr<ast::Statement>> body;
            {
                // write(get_pi());
                std::vector<std::unique_ptr<ast::Expression>> in_in_args;
                auto param = std::make_unique<ast::expressions::FuncExpression>("get_pi", std::move(in_in_args));
                std::vector<std::unique_ptr<ast::Expression>> in_args;
                in_args.push_back(std::move(param));
                auto expr = std::make_unique<ast::expressions::FuncExpression>("write", std::move(in_args));
                auto stmt = std::make_unique<ast::StandaloneStatement>(std::move(expr));
                body.push_back(std::move(stmt));
            }
            {
                // pi = 3.0;
                auto value = std::make_unique<ast::expressions::DoubleLitExpression>(3);
                auto stmt = std::make_unique<ast::VariableStatement>("pi", std::move(value));
                body.push_back(std::move(stmt));
            }
            {
                // write(pi);
                auto param = std::make_unique<ast::expressions::IdentifierExpression>("pi");
                std::vector<std::unique_ptr<ast::Expression>> in_args;
                in_args.push_back(std::move(param));
                auto value = std::make_unique<ast::expressions::FuncExpression>("write", std::move(in_args));
                auto stmt = std::make_unique<ast::StandaloneStatement>(std::move(value));
                body.push_back(std::move(stmt));
            }
            {
                // write(1.0 + (3.0 * 4.0) % 5.0);
                auto exp_1 = std::make_unique<ast::expressions::DoubleLitExpression>(1);
                auto exp_3 = std::make_unique<ast::expressions::DoubleLitExpression>(3);
                auto exp_4 = std::make_unique<ast::expressions::DoubleLitExpression>(4);
                auto exp_5 = std::make_unique<ast::expressions::DoubleLitExpression>(5);
                auto mul = std::make_unique<ast::expressions::MulExpression>(std::move(exp_3), std::move(exp_4));
                auto par = std::make_unique<ast::expressions::ParExpression>(std::move(mul));
                auto sum = std::make_unique<ast::expressions::SumExpression>(std::move(exp_1), std::move(par));
                auto mod = std::make_unique<ast::expressions::ModExpression>(std::move(sum), std::move(exp_5));
                std::vector<std::unique_ptr<ast::Expression>> in_args;
                in_args.push_back(std::move(mod));
                auto value = std::make_unique<ast::expressions::FuncExpression>("write", std::move(in_args));
                auto stmt = std::make_unique<ast::StandaloneStatement>(std::move(value));
                body.push_back(std::move(stmt));
            }
            {
                // return 0.0;
                auto value = std::make_unique<ast::expressions::DoubleLitExpression>(0);
                auto ret = std::make_unique<ast::ReturnStatement>(std::move(value));
                body.push_back(std::move(ret));
            }
            std::vector<ast::Identifier> args;
            auto def = std::make_unique<ast::FunctionDefinition>("main", std::move(args), std::move(body));
            corr_defs.push_back(std::move(def));
        }
        ast::Program correct(std::move(corr_defs));

        // Parse
        ast::Program result = parser::ProgramParser(qf.get_f, qf.peek_f).program();

        // Compare
        if (!result.equals(&correct)) {
            // When wrong, display correct tree
            boost::unit_test::unit_test_log << "Correct tree:\n" << ast::util::print_ast(&correct);
            boost::unit_test::unit_test_log << "Resulting tree:\n" << ast::util::print_ast(&result);
        }
        BOOST_TEST_CHECK(result.equals(&correct), "Parsed tree must match hard-coded correct tree.");
    }

    //TODO cases for consuming all closing tokens (RPAR, RBRAC, SEMICOLON, ...)

BOOST_AUTO_TEST_SUITE_END()
