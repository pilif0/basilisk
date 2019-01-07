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
        // Return error token if not valid
        if (offset >= input.size()) {
            std::ostringstream message;
            message << "No token " << offset << " elements from the front of the input queue.";
            return tokens::Token{tags::error, message.str()};
        }

        // Compute index
        auto index = static_cast<unsigned int>(input.size() - 1 - offset);

        return input[index];
    }
};

/**
 * \brief Compare a parsed AST to a hard-coded correct AST and print the trees if different
 * 
 * \param result Pointer to parsed AST 
 * \param correct Pointer to correct AST
 */
void compare_ast(ast::Node *result, ast::Node *correct) {
    // Compare
    if (!result->equals(correct)) {
        // When wrong, display correct tree
        boost::unit_test::unit_test_log << "Correct tree:\n" << ast::util::print_ast(correct);
        boost::unit_test::unit_test_log << "Resulting tree:\n" << ast::util::print_ast(result);
    }
    BOOST_TEST_CHECK(result->equals(correct), "Parsed tree must match hard-coded correct tree.");
}

BOOST_AUTO_TEST_SUITE(Parser)
    BOOST_AUTO_TEST_SUITE(individual)
        BOOST_AUTO_TEST_SUITE(expression)
            BOOST_AUTO_TEST_SUITE(mod)
                // Check correct
                BOOST_AUTO_TEST_CASE( correct ) {
                    // Construct fixture
                    QueuesFixture qf("a % b");

                    // Correct result
                    auto a = std::make_unique<ast::expressions::IdentifierExpression>("a");
                    auto b = std::make_unique<ast::expressions::IdentifierExpression>("b");
                    auto correct = std::make_unique<ast::expressions::ModExpression>(std::move(a), std::move(b));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression();

                    compare_ast(result.get(), correct.get());
                }

                // Check right associativity
                BOOST_AUTO_TEST_CASE( right_associative ) {
                    // Construct fixture
                    QueuesFixture qf("a % b % c");

                    // Correct result
                    auto a = std::make_unique<ast::expressions::IdentifierExpression>("a");
                    auto b = std::make_unique<ast::expressions::IdentifierExpression>("b");
                    auto c = std::make_unique<ast::expressions::IdentifierExpression>("c");
                    auto rhs = std::make_unique<ast::expressions::ModExpression>(std::move(b), std::move(c));
                    auto correct = std::make_unique<ast::expressions::ModExpression>(std::move(a), std::move(rhs));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression();

                    compare_ast(result.get(), correct.get());
                }
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(expression_1)
                // Note: correct sum and sub recognition is taken care of by respecitve suites

                // Check mixed expression sum of sub
                BOOST_AUTO_TEST_CASE( mixed_sum_sub ) {
                    // Construct fixture
                    QueuesFixture qf("a + b - c");

                    // Correct result
                    auto a = std::make_unique<ast::expressions::IdentifierExpression>("a");
                    auto b = std::make_unique<ast::expressions::IdentifierExpression>("b");
                    auto c = std::make_unique<ast::expressions::IdentifierExpression>("c");
                    auto rhs = std::make_unique<ast::expressions::SubExpression>(std::move(b), std::move(c));
                    auto correct = std::make_unique<ast::expressions::SumExpression>(std::move(a), std::move(rhs));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_1();

                    compare_ast(result.get(), correct.get());
                }

                // Check mixed expression sub of sum
                BOOST_AUTO_TEST_CASE( mixed_sub_sum ) {
                    // Construct fixture
                    QueuesFixture qf("a - b + c");

                    // Correct result
                    auto a = std::make_unique<ast::expressions::IdentifierExpression>("a");
                    auto b = std::make_unique<ast::expressions::IdentifierExpression>("b");
                    auto c = std::make_unique<ast::expressions::IdentifierExpression>("c");
                    auto rhs = std::make_unique<ast::expressions::SumExpression>(std::move(b), std::move(c));
                    auto correct = std::make_unique<ast::expressions::SubExpression>(std::move(a), std::move(rhs));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_1();

                    compare_ast(result.get(), correct.get());
                }
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(sum)
                // Check correct
                BOOST_AUTO_TEST_CASE( correct ) {
                    // Construct fixture
                    QueuesFixture qf("a + b");

                    // Correct result
                    auto a = std::make_unique<ast::expressions::IdentifierExpression>("a");
                    auto b = std::make_unique<ast::expressions::IdentifierExpression>("b");
                    auto correct = std::make_unique<ast::expressions::SumExpression>(std::move(a), std::move(b));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_1();

                    compare_ast(result.get(), correct.get());
                }

                // Check right associativity
                BOOST_AUTO_TEST_CASE( right_associative ) {
                    // Construct fixture
                    QueuesFixture qf("a + b + c");

                    // Correct result
                    auto a = std::make_unique<ast::expressions::IdentifierExpression>("a");
                    auto b = std::make_unique<ast::expressions::IdentifierExpression>("b");
                    auto c = std::make_unique<ast::expressions::IdentifierExpression>("c");
                    auto rhs = std::make_unique<ast::expressions::SumExpression>(std::move(b), std::move(c));
                    auto correct = std::make_unique<ast::expressions::SumExpression>(std::move(a), std::move(rhs));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_1();

                    compare_ast(result.get(), correct.get());
                }
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(sub)
                // Check correct
                BOOST_AUTO_TEST_CASE( correct ) {
                    // Construct fixture
                    QueuesFixture qf("a - b");

                    // Correct result
                    auto a = std::make_unique<ast::expressions::IdentifierExpression>("a");
                    auto b = std::make_unique<ast::expressions::IdentifierExpression>("b");
                    auto correct = std::make_unique<ast::expressions::SubExpression>(std::move(a), std::move(b));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_1();

                    compare_ast(result.get(), correct.get());
                }

                // Check right associativity
                BOOST_AUTO_TEST_CASE( right_associative ) {
                    // Construct fixture
                    QueuesFixture qf("a - b - c");

                    // Correct result
                    auto a = std::make_unique<ast::expressions::IdentifierExpression>("a");
                    auto b = std::make_unique<ast::expressions::IdentifierExpression>("b");
                    auto c = std::make_unique<ast::expressions::IdentifierExpression>("c");
                    auto rhs = std::make_unique<ast::expressions::SubExpression>(std::move(b), std::move(c));
                    auto correct = std::make_unique<ast::expressions::SubExpression>(std::move(a), std::move(rhs));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_1();

                    compare_ast(result.get(), correct.get());
                }
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(expression_2)
                // Note: correct mul and div recognition is taken care of by respecitve suites

                // Check mixed expression mul of div
                BOOST_AUTO_TEST_CASE( mixed_mul_div ) {
                    // Construct fixture
                    QueuesFixture qf("a * b / c");

                    // Correct result
                    auto a = std::make_unique<ast::expressions::IdentifierExpression>("a");
                    auto b = std::make_unique<ast::expressions::IdentifierExpression>("b");
                    auto c = std::make_unique<ast::expressions::IdentifierExpression>("c");
                    auto rhs = std::make_unique<ast::expressions::DivExpression>(std::move(b), std::move(c));
                    auto correct = std::make_unique<ast::expressions::MulExpression>(std::move(a), std::move(rhs));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_2();

                    compare_ast(result.get(), correct.get());
                }

                // Check mixed expression div of mul
                BOOST_AUTO_TEST_CASE( mixed_div_mul ) {
                    // Construct fixture
                    QueuesFixture qf("a / b * c");

                    // Correct result
                    auto a = std::make_unique<ast::expressions::IdentifierExpression>("a");
                    auto b = std::make_unique<ast::expressions::IdentifierExpression>("b");
                    auto c = std::make_unique<ast::expressions::IdentifierExpression>("c");
                    auto rhs = std::make_unique<ast::expressions::MulExpression>(std::move(b), std::move(c));
                    auto correct = std::make_unique<ast::expressions::DivExpression>(std::move(a), std::move(rhs));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_2();

                    compare_ast(result.get(), correct.get());
                }
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(mul)
                // Check correct
                BOOST_AUTO_TEST_CASE( correct ) {
                    // Construct fixture
                    QueuesFixture qf("a * b");

                    // Correct result
                    auto a = std::make_unique<ast::expressions::IdentifierExpression>("a");
                    auto b = std::make_unique<ast::expressions::IdentifierExpression>("b");
                    auto correct = std::make_unique<ast::expressions::MulExpression>(std::move(a), std::move(b));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_2();

                    compare_ast(result.get(), correct.get());
                }

                // Check right associativity
                BOOST_AUTO_TEST_CASE( right_associative ) {
                    // Construct fixture
                    QueuesFixture qf("a * b * c");

                    // Correct result
                    auto a = std::make_unique<ast::expressions::IdentifierExpression>("a");
                    auto b = std::make_unique<ast::expressions::IdentifierExpression>("b");
                    auto c = std::make_unique<ast::expressions::IdentifierExpression>("c");
                    auto rhs = std::make_unique<ast::expressions::MulExpression>(std::move(b), std::move(c));
                    auto correct = std::make_unique<ast::expressions::MulExpression>(std::move(a), std::move(rhs));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_2();

                    compare_ast(result.get(), correct.get());
                }
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(div)
                // Check correct
                BOOST_AUTO_TEST_CASE( correct ) {
                    // Construct fixture
                    QueuesFixture qf("a / b");

                    // Correct result
                    auto a = std::make_unique<ast::expressions::IdentifierExpression>("a");
                    auto b = std::make_unique<ast::expressions::IdentifierExpression>("b");
                    auto correct = std::make_unique<ast::expressions::DivExpression>(std::move(a), std::move(b));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_2();

                    compare_ast(result.get(), correct.get());
                }

                // Check right associativity
                BOOST_AUTO_TEST_CASE( right_associative ) {
                    // Construct fixture
                    QueuesFixture qf("a / b / c");

                    // Correct result
                    auto a = std::make_unique<ast::expressions::IdentifierExpression>("a");
                    auto b = std::make_unique<ast::expressions::IdentifierExpression>("b");
                    auto c = std::make_unique<ast::expressions::IdentifierExpression>("c");
                    auto rhs = std::make_unique<ast::expressions::DivExpression>(std::move(b), std::move(c));
                    auto correct = std::make_unique<ast::expressions::DivExpression>(std::move(a), std::move(rhs));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_2();

                    compare_ast(result.get(), correct.get());
                }
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(neg)
                // Check correct
                BOOST_AUTO_TEST_CASE( correct ) {
                    // Construct fixture
                    QueuesFixture qf("-x");

                    // Correct result
                    auto expr = std::make_unique<ast::expressions::IdentifierExpression>("x");
                    auto correct = std::make_unique<ast::expressions::NegExpression>(std::move(expr));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_3();

                    compare_ast(result.get(), correct.get());
                }

                // Check double negation
                BOOST_AUTO_TEST_CASE( double_negation ) {
                    // Construct fixture
                    QueuesFixture qf("--x");

                    // Correct result
                    auto expr = std::make_unique<ast::expressions::IdentifierExpression>("x");
                    auto neg = std::make_unique<ast::expressions::NegExpression>(std::move(expr));
                    auto correct = std::make_unique<ast::expressions::NegExpression>(std::move(neg));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_3();

                    compare_ast(result.get(), correct.get());
                }
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(expression_4)
                // Check recognition of double literal
                BOOST_AUTO_TEST_CASE( pick_double_literal ) {
                    // Construct fixture
                    QueuesFixture qf("1.0");

                    // Correct result
                    auto correct = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_4();

                    compare_ast(result.get(), correct.get());
                }

                // Check recognition of parenthesised expression
                BOOST_AUTO_TEST_CASE( pick_parenthesised ) {
                    // Construct fixture
                    QueuesFixture qf("(x)");

                    // Correct result
                    auto expr = std::make_unique<ast::expressions::IdentifierExpression>("x");
                    auto correct = std::make_unique<ast::expressions::ParExpression>(std::move(expr));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_4();

                    compare_ast(result.get(), correct.get());
                }

                // Check recognition of function call
                BOOST_AUTO_TEST_CASE( pick_function_call ) {
                    // Construct fixture
                    QueuesFixture qf("f()");

                    // Correct result
                    std::vector<std::unique_ptr<ast::Expression>> in_args;
                    auto correct = std::make_unique<ast::expressions::FuncExpression>("f", std::move(in_args));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_4();

                    compare_ast(result.get(), correct.get());
                }

                // Check recognition of identifier expression
                BOOST_AUTO_TEST_CASE( pick_identifier ) {
                    // Construct fixture
                    QueuesFixture qf("x");

                    // Correct result
                    auto correct = std::make_unique<ast::expressions::IdentifierExpression>("x");

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_4();

                    compare_ast(result.get(), correct.get());
                }

                // Check exception on unexpected token
                BOOST_AUTO_TEST_CASE( unexpected_token ) {
                    // Construct fixture
                    QueuesFixture qf("{");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::ExpressionParser(qf.get_f, qf.peek_f).expression_4(), parser::ParserException);
                }
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(literal_double)
                // Check correct
                BOOST_AUTO_TEST_CASE( correct ) {
                    // Construct fixture
                    QueuesFixture qf("1.0");

                    // Correct result
                    auto correct = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).literal_double();

                    compare_ast(result.get(), correct.get());
                }

                // Check exception on not parsable
                BOOST_AUTO_TEST_CASE( not_parsable ) {
                    // Construct fixture
                    QueuesFixture qf;
                    qf.input.push_back({tags::end_of_input, ""});
                    qf.input.push_back({tags::double_literal, "x"});

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::ExpressionParser(qf.get_f, qf.peek_f).literal_double(), parser::ParserException);
                }

                // Check exception on out of range
                BOOST_AUTO_TEST_CASE( out_of_range ) {
                    // Construct fixture
                    std::ostringstream source;
                    source << 1 << std::to_string(std::numeric_limits<double>::max());
                    QueuesFixture qf(source.str());

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::ExpressionParser(qf.get_f, qf.peek_f).literal_double(), parser::ParserException);
                }

                // Check exception on unexpected token
                BOOST_AUTO_TEST_CASE( unexpected_token ) {
                    // Construct fixture
                    QueuesFixture qf("x");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::ExpressionParser(qf.get_f, qf.peek_f).literal_double(), parser::ParserException);
                }
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(parenthesised)
                // Check correct
                BOOST_AUTO_TEST_CASE( correct ) {
                    // Construct fixture
                    QueuesFixture qf("(x)");

                    // Correct result
                    auto expr = std::make_unique<ast::expressions::IdentifierExpression>("x");
                    auto correct = std::make_unique<ast::expressions::ParExpression>(std::move(expr));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).parenthesised();

                    compare_ast(result.get(), correct.get());
                }

                // Check exception on empty contents
                BOOST_AUTO_TEST_CASE( empty ) {
                    // Construct fixture
                    QueuesFixture qf("()");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::ExpressionParser(qf.get_f, qf.peek_f).parenthesised(), parser::ParserException);
                }

                // Check exception on unexpected token instead of LPAR
                BOOST_AUTO_TEST_CASE( unexpected_token_lpar ) {
                    // Construct fixture
                    QueuesFixture qf("1.0");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::ExpressionParser(qf.get_f, qf.peek_f).parenthesised(), parser::ParserException);
                }

                // Check exception on unexpected token instead of RPAR
                BOOST_AUTO_TEST_CASE( unexpected_token_rpar ) {
                    // Construct fixture
                    QueuesFixture qf("( x 1.0");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::ExpressionParser(qf.get_f, qf.peek_f).parenthesised(), parser::ParserException);
                }

                // Check closing parenthesis gets consumed
                BOOST_AUTO_TEST_CASE( closing_par_consumed ){
                    // Construct fixture
                    QueuesFixture qf("( x )");

                    // Parse out parenthesised expression
                    parser::ExpressionParser(qf.get_f, qf.peek_f).parenthesised();

                    // Check the top of the input is not a RPAR
                    BOOST_TEST_CHECK(qf.peek(0).tag != tags::rpar, "Closing parenthesis must be consumed.");
                }
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(identifier)
                // Check correct
                BOOST_AUTO_TEST_CASE( correct ) {
                    // Construct fixture
                    QueuesFixture qf("x");

                    // Correct result
                    auto correct = std::make_unique<ast::expressions::IdentifierExpression>("x");

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).identifier();

                    compare_ast(result.get(), correct.get());
                }

                // Check exception on unexpected token instead of identifier
                BOOST_AUTO_TEST_CASE( unexpected_token_identifier ) {
                    // Construct fixture
                    QueuesFixture qf("1.0");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::ExpressionParser(qf.get_f, qf.peek_f).identifier(), parser::ParserException);
                }
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(function_call)
                // Check correct with no arguments
                BOOST_AUTO_TEST_CASE( correct_no_args ) {
                    // Construct fixture
                    QueuesFixture qf("f()");

                    // Correct result
                    std::vector<std::unique_ptr<ast::Expression>> in_args;
                    auto correct = std::make_unique<ast::expressions::FuncExpression>("f", std::move(in_args));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).function_call();

                    compare_ast(result.get(), correct.get());
                }

                // Check correct with one argument
                BOOST_AUTO_TEST_CASE( correct_one_arg ) {
                    // Construct fixture
                    QueuesFixture qf("f(x)");

                    // Correct result
                    auto arg = std::make_unique<ast::expressions::IdentifierExpression>("x");
                    std::vector<std::unique_ptr<ast::Expression>> in_args;
                    in_args.push_back(std::move(arg));
                    auto correct = std::make_unique<ast::expressions::FuncExpression>("f", std::move(in_args));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).function_call();

                    compare_ast(result.get(), correct.get());
                }

                // Check correct with multiple arguments
                BOOST_AUTO_TEST_CASE( correct_multiple_args ) {
                    // Construct fixture
                    QueuesFixture qf("f(x, y)");

                    // Correct result
                    auto arg1 = std::make_unique<ast::expressions::IdentifierExpression>("x");
                    auto arg2 = std::make_unique<ast::expressions::IdentifierExpression>("y");
                    std::vector<std::unique_ptr<ast::Expression>> in_args;
                    in_args.push_back(std::move(arg1));
                    in_args.push_back(std::move(arg2));
                    auto correct = std::make_unique<ast::expressions::FuncExpression>("f", std::move(in_args));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).function_call();

                    compare_ast(result.get(), correct.get());
                }

                // Check exception on unexpected token instead of identifier
                BOOST_AUTO_TEST_CASE( unexpected_token_identifier ) {
                    // Construct fixture
                    QueuesFixture qf("1.0");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::ExpressionParser(qf.get_f, qf.peek_f).function_call(), parser::ParserException);
                }

                // Check exception on unexpected token instead of LPAR
                BOOST_AUTO_TEST_CASE( unexpected_token_lpar ) {
                    // Construct fixture
                    QueuesFixture qf("f 1.0");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::ExpressionParser(qf.get_f, qf.peek_f).function_call(), parser::ParserException);
                }

                // Check exception on unexpected token instead of RPAR
                BOOST_AUTO_TEST_CASE( unexpected_token_rpar ) {
                    // Construct fixture
                    QueuesFixture qf("f ( x 1.0");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::ExpressionParser(qf.get_f, qf.peek_f).function_call(), parser::ParserException);
                }

                // Check closing parenthesis gets consumed
                BOOST_AUTO_TEST_CASE( closing_par_consumed ){
                    // Construct fixture
                    QueuesFixture qf("f ( x )");

                    // Parse out parenthesised expression
                    parser::ExpressionParser(qf.get_f, qf.peek_f).function_call();

                    // Check the top of the input is not a RPAR
                    BOOST_TEST_CHECK(qf.peek(0).tag != tags::rpar, "Closing parenthesis must be consumed.");
                }
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(list)
                // Check singleton
                BOOST_AUTO_TEST_CASE( correct_singleton ) {
                    // Construct fixture
                    QueuesFixture qf("x");

                    // Correct result
                    auto x = std::make_unique<ast::expressions::IdentifierExpression>("x");
                    std::vector<std::unique_ptr<ast::Expression>> correct;
                    correct.push_back(std::move(x));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).list();

                    // Compare
                    if (!ast::util::vec_equals(result, correct)) {
                        // When wrong, display correct list
                        boost::unit_test::unit_test_log << "Correct list:\n";
                        for (auto &e : correct) {
                            boost::unit_test::unit_test_log << ast::util::print_ast(e.get());
                        }
                        boost::unit_test::unit_test_log << "Resulting list:\n";
                        for (auto &e : result) {
                            boost::unit_test::unit_test_log << ast::util::print_ast(e.get());
                        }
                    }
                    BOOST_TEST_CHECK(ast::util::vec_equals(result, correct), "Parsed list must match hard-coded correct list.");
                }

                // Check multiple
                BOOST_AUTO_TEST_CASE( correct_multiple_args ) {
                    // Construct fixture
                    QueuesFixture qf("x, y");

                    // Correct result
                    auto arg1 = std::make_unique<ast::expressions::IdentifierExpression>("x");
                    auto arg2 = std::make_unique<ast::expressions::IdentifierExpression>("y");
                    std::vector<std::unique_ptr<ast::Expression>> correct;
                    correct.push_back(std::move(arg1));
                    correct.push_back(std::move(arg2));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).list();

                    // Compare
                    if (!ast::util::vec_equals(result, correct)) {
                        // When wrong, display correct list
                        boost::unit_test::unit_test_log << "Correct list:\n";
                        for (auto &e : correct) {
                            boost::unit_test::unit_test_log << ast::util::print_ast(e.get());
                        }
                        boost::unit_test::unit_test_log << "Resulting list:\n";
                        for (auto &e : result) {
                            boost::unit_test::unit_test_log << ast::util::print_ast(e.get());
                        }
                    }
                    BOOST_TEST_CHECK(ast::util::vec_equals(result, correct), "Parsed list must match hard-coded correct list.");
                }
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(fallthrough)
                // Check expression to expression1
                BOOST_AUTO_TEST_CASE( expression_to_1 ) {
                    // Construct fixture
                    QueuesFixture qf("a + b");

                    // Correct result
                    auto a = std::make_unique<ast::expressions::IdentifierExpression>("a");
                    auto b = std::make_unique<ast::expressions::IdentifierExpression>("b");
                    auto correct = std::make_unique<ast::expressions::SumExpression>(std::move(a), std::move(b));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression();

                    compare_ast(result.get(), correct.get());
                }

                // Check expression1 to expression2
                BOOST_AUTO_TEST_CASE( expression1_to_2 ) {
                    // Construct fixture
                    QueuesFixture qf("a * b");

                    // Correct result
                    auto a = std::make_unique<ast::expressions::IdentifierExpression>("a");
                    auto b = std::make_unique<ast::expressions::IdentifierExpression>("b");
                    auto correct = std::make_unique<ast::expressions::MulExpression>(std::move(a), std::move(b));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_1();

                    compare_ast(result.get(), correct.get());
                }

                // Check expression2 to expression3
                BOOST_AUTO_TEST_CASE( expression2_to_3 ) {
                    // Construct fixture
                    QueuesFixture qf("-x");

                    // Correct result
                    auto x = std::make_unique<ast::expressions::IdentifierExpression>("x");
                    auto correct = std::make_unique<ast::expressions::NegExpression>(std::move(x));

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_2();

                    compare_ast(result.get(), correct.get());
                }

                // Check expression3 to expression4
                BOOST_AUTO_TEST_CASE( expression3_to_4 ) {
                    // Construct fixture
                    QueuesFixture qf("x");

                    // Correct result
                    auto correct = std::make_unique<ast::expressions::IdentifierExpression>("x");

                    // Parse
                    auto result = parser::ExpressionParser(qf.get_f, qf.peek_f).expression_3();

                    compare_ast(result.get(), correct.get());
                }
            BOOST_AUTO_TEST_SUITE_END()
        BOOST_AUTO_TEST_SUITE_END()

        BOOST_AUTO_TEST_SUITE(statement)
            // Check correctly recognising return statement
            BOOST_AUTO_TEST_CASE( pick_return ) {
                // Construct fixture
                QueuesFixture qf("return x;");

                // Correct result
                auto value = std::make_unique<ast::expressions::IdentifierExpression>("x");
                auto correct = std::make_unique<ast::ReturnStatement>(std::move(value));

                // Parse
                auto result = parser::StatementParser(qf.get_f, qf.peek_f).statement();

                compare_ast(result.get(), correct.get());
            }

            // Check correctly recognising variable statement
            BOOST_AUTO_TEST_CASE( pick_variable ) {
                // Construct fixture
                QueuesFixture qf("x = 1.0;");

                // Correct result
                auto value = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto correct = std::make_unique<ast::VariableStatement>("x", std::move(value));

                // Parse
                auto result = parser::StatementParser(qf.get_f, qf.peek_f).statement();

                compare_ast(result.get(), correct.get());
            }

            // Check correctly recognising standalone statement starting with identifier
            BOOST_AUTO_TEST_CASE( pick_standalone_identifier ) {
                // Construct fixture
                QueuesFixture qf("f();");

                // Correct result
                std::vector<std::unique_ptr<ast::Expression>> in_args;
                auto expr = std::make_unique<ast::expressions::FuncExpression>("f", std::move(in_args));
                auto correct = std::make_unique<ast::StandaloneStatement>(std::move(expr));

                // Parse
                auto result = parser::StatementParser(qf.get_f, qf.peek_f).statement();

                compare_ast(result.get(), correct.get());
            }

            // Check correctly recognising standalone statement not starting with identifier
            BOOST_AUTO_TEST_CASE( pick_standalone ) {
                // Construct fixture
                QueuesFixture qf("1.0;");

                // Correct result
                auto expr = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                auto correct = std::make_unique<ast::StandaloneStatement>(std::move(expr));

                // Parse
                auto result = parser::StatementParser(qf.get_f, qf.peek_f).statement();

                compare_ast(result.get(), correct.get());
            }

            BOOST_AUTO_TEST_SUITE(return_kw)
                // Check correct
                BOOST_AUTO_TEST_CASE( correct ) {
                    // Construct fixture
                    QueuesFixture qf("return 1.0;");

                    // Correct result
                    auto value = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                    auto correct = std::make_unique<ast::ReturnStatement>(std::move(value));

                    // Parse
                    auto result = parser::StatementParser(qf.get_f, qf.peek_f).return_kw();

                    compare_ast(result.get(), correct.get());
                }

                // Check exception on unexpected token instead of RETURN
                BOOST_AUTO_TEST_CASE( unexpected_token_return ) {
                    // Construct fixture
                    QueuesFixture qf("1.0");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::StatementParser(qf.get_f, qf.peek_f).return_kw(), parser::ParserException);
                }

                // Check exception on unexpected token instead of SEMICOLON
                BOOST_AUTO_TEST_CASE( unexpected_token_semicolon ) {
                    // Construct fixture
                    QueuesFixture qf("return 1.0");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::StatementParser(qf.get_f, qf.peek_f).return_kw(), parser::ParserException);
                }

                // Check semicolon gets consumed
                BOOST_AUTO_TEST_CASE( semicolon_consumed ){
                    // Construct fixture
                    QueuesFixture qf("return 1.0;");

                    // Parse out parenthesised expression
                    parser::StatementParser(qf.get_f, qf.peek_f).return_kw();

                    // Check the top of the input is not a SEMICOLON
                    BOOST_TEST_CHECK(qf.peek(0).tag != tags::semicolon, "Semicolon must be consumed.");
                }
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(standalone)
                // Check correct
                BOOST_AUTO_TEST_CASE( correct ) {
                    // Construct fixture
                    QueuesFixture qf("1.0;");

                    // Correct result
                    auto value = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                    auto correct = std::make_unique<ast::StandaloneStatement>(std::move(value));

                    // Parse
                    auto result = parser::StatementParser(qf.get_f, qf.peek_f).standalone();

                    compare_ast(result.get(), correct.get());
                }

                // Check exception on unexpected token instead of SEMICOLON
                BOOST_AUTO_TEST_CASE( unexpected_token_semicolon ) {
                    // Construct fixture
                    QueuesFixture qf("1.0");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::StatementParser(qf.get_f, qf.peek_f).standalone(), parser::ParserException);
                }

                // Check semicolon gets consumed
                BOOST_AUTO_TEST_CASE( semicolon_consumed ){
                    // Construct fixture
                    QueuesFixture qf("1.0;");

                    // Parse out parenthesised expression
                    parser::StatementParser(qf.get_f, qf.peek_f).standalone();

                    // Check the top of the input is not a SEMICOLON
                    BOOST_TEST_CHECK(qf.peek(0).tag != tags::semicolon, "Semicolon must be consumed.");
                }
            BOOST_AUTO_TEST_SUITE_END()

            BOOST_AUTO_TEST_SUITE(variable)
                // Check correct
                BOOST_AUTO_TEST_CASE( correct ) {
                    // Construct fixture
                    QueuesFixture qf("x = 1.0;");

                    // Correct result
                    auto value = std::make_unique<ast::expressions::DoubleLitExpression>(1.0);
                    auto correct = std::make_unique<ast::VariableStatement>("x", std::move(value));

                    // Parse
                    auto result = parser::StatementParser(qf.get_f, qf.peek_f).variable();

                    compare_ast(result.get(), correct.get());
                }

                // Check exception on unexpected token instead of identifier
                BOOST_AUTO_TEST_CASE( unexpected_token_identifier ) {
                    // Construct fixture
                    QueuesFixture qf("1.0");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::StatementParser(qf.get_f, qf.peek_f).variable(), parser::ParserException);
                }

                // Check exception on unexpected token instead of ASSIGN
                BOOST_AUTO_TEST_CASE( unexpected_token_assign ) {
                    // Construct fixture
                    QueuesFixture qf("x 1.0");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::StatementParser(qf.get_f, qf.peek_f).variable(), parser::ParserException);
                }

                // Check exception on unexpected token instead of SEMICOLON
                BOOST_AUTO_TEST_CASE( unexpected_token_semicolon ) {
                    // Construct fixture
                    QueuesFixture qf("x = 1.0");

                    // Check exception on parse
                    BOOST_CHECK_THROW(parser::StatementParser(qf.get_f, qf.peek_f).variable(), parser::ParserException);
                }

                // Check semicolon gets consumed
                BOOST_AUTO_TEST_CASE( semicolon_consumed ){
                    // Construct fixture
                    QueuesFixture qf("x = 1.0;");

                    // Parse out parenthesised expression
                    parser::StatementParser(qf.get_f, qf.peek_f).variable();

                    // Check the top of the input is not a SEMICOLON
                    BOOST_TEST_CHECK(qf.peek(0).tag != tags::semicolon, "Semicolon must be consumed.");
                }
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

                compare_ast(result.get(), correct.get());
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

                compare_ast(result.get(), correct.get());
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

                    compare_ast(result.get(), correct.get());
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

                    compare_ast(result.get(), correct.get());
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

                    compare_ast(result.get(), correct.get());
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

                // Check closing bracket gets consumed
                BOOST_AUTO_TEST_CASE( closing_brac_consumed ){
                    // Construct fixture
                    QueuesFixture qf("f () {}");

                    // Parse out parenthesised expression
                    parser::DefinitionParser(qf.get_f, qf.peek_f).definition();

                    // Check the top of the input is not a RPAR
                    BOOST_TEST_CHECK(qf.peek(0).tag != tags::rbrac, "Closing bracket must be consumed.");
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

                    compare_ast(result.get(), correct.get());
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

                compare_ast(&result, &correct);
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

            // Check end of input token gets consumed
            BOOST_AUTO_TEST_CASE( eoi_consumed ){
                // Construct fixture
                QueuesFixture qf("f () {}");

                // Parse out parenthesised expression
                parser::ProgramParser(qf.get_f, qf.peek_f).program();

                // Check the top of the input is not a RPAR
                BOOST_TEST_CHECK(qf.peek(0).tag != tags::end_of_input, "End of input token must be consumed.");
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

        compare_ast(&result, &correct);
    }
BOOST_AUTO_TEST_SUITE_END()
