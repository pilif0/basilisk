#include <utility>

/** \file LexerTest.cpp
 * Lexer test module
 *
 * \author Filip Smola
 */
#define BOOST_TEST_MODULE "Lexer"

#include <basilisk/Lexer.h>
#include <basilisk/Tokens.h>

#include <boost/test/unit_test.hpp>

#include <queue>
#include <string>
#include <functional>
#include <map>

namespace tokens = basilisk::tokens;
namespace tags = tokens::tags;
namespace lexer = basilisk::lexer;

//! Fixture that sets up two queues for use as lexer input and output
struct QueuesFixture {
    //! Lexer input queue type
    typedef std::queue<char> in_queue_t;
    //! Lexer output queue type
    typedef std::queue<tokens::Token> out_queue_t;

    //! Lexer input queue
    in_queue_t input;
    //! Lexer output queue
    out_queue_t output;

    /**
     * \brief Pop a character from the front of the input queue and return it
     *
     * \return Popped character, or the null character when empty
     */
    char get() {
        // Return null char if empty
        if (input.empty()) {
            return '\0';
        }

        char c = input.front();
        input.pop();
        return c;
    }

    /**
     * \brief Peek at the character at the front of the input queue
     *
     * \return Character at the front of the input queue, or the null character when empty
     */
    char peek() {
        // Return null char if empty
        if (input.empty()) {
            return '\0';
        }

        return input.front();
    }

    /**
     * \brief Push a token to the end of the output queue
     *
     * \param t Token to push
     */
    void append(const tokens::Token &t) { output.push(t); }

    /**
     * \brief Load the contents of the string into the input queue
     *
     * \param in Input to load
     */
    void load(std::string in) {
        for (const char& c : in) {
            input.push(c);
        }
    }

    /**
     * \brief Lex the contents of the input queue into the output queue
     */
    void lex() {
        auto get_f = std::bind(&QueuesFixture::get, this);
        auto peek_f = std::bind(&QueuesFixture::peek, this);
        auto append_f = std::bind(&QueuesFixture::append, this, std::placeholders::_1);
        lexer::lex(get_f, peek_f, append_f);
    }
};

// Make the queues not printable
BOOST_TEST_DONT_PRINT_LOG_VALUE(QueuesFixture::in_queue_t)
BOOST_TEST_DONT_PRINT_LOG_VALUE(QueuesFixture::out_queue_t)


//! Function to print the output queues
std::string print_queue(QueuesFixture::out_queue_t queue) {
    if (!queue.empty()) {
        std::ostringstream stream;
        stream << '[';

        // Append first
        stream << queue.front();
        queue.pop();

        // Append rest
        while (!queue.empty()) {
            stream << ", " << queue.front();
            queue.pop();
        }

        stream << ']';
        return stream.str();
    } else {
        return "[]";
    }
}

BOOST_AUTO_TEST_SUITE(Lexer)

//! Test fixture is working correctly
BOOST_AUTO_TEST_CASE( fixture_test ) {
    // Constants in this test
    constexpr char subject_character = 'a';
    tokens::Token subject_token{tags::ASSIGN, {}};

    // Construct fixture
    QueuesFixture q;

    // Push input
    q.input.push(subject_character);

    // Check input size
    BOOST_CHECK(q.input.size() == 1);

    // Check peek
    BOOST_CHECK(q.peek() == subject_character);

    // Get input
    BOOST_CHECK(q.get() == subject_character);

    // Check pop success
    BOOST_CHECK(q.input.empty());

    // Append output
    q.append(subject_token);

    // Check output size
    BOOST_CHECK(q.output.size() == 1);

    // Pop output
    BOOST_CHECK(q.output.front() == subject_token);
}

/**
 * \brief Test whether the provided input lexes into the correct tokens
 *
 * \param input Input to lex
 * \param correct Correct output queue
 */
void test_input(const std::string &input, QueuesFixture::out_queue_t correct) {
    // Prepare fixture
    QueuesFixture q;
    q.load(input);

    // Lex the input
    q.lex();

    // Check tokens were added (tokens for input and END)
    BOOST_TEST_CHECK(q.output.size() == correct.size());

    // Check the tokens are correct if present
    BOOST_TEST_CHECK(q.output == correct, "\"" << input << "\" should lex to " << print_queue(correct)
                                               << ", lexes to "<< print_queue(q.output));
}

//! Test single naive tokens lex (excluding special tokens END and ERROR)
BOOST_AUTO_TEST_SUITE(unit_naive_lexes)

BOOST_AUTO_TEST_CASE(identifier) {
    // Data
    std::string input = "identifier";
    QueuesFixture::out_queue_t correct({{tags::IDENTIFIER, "identifier"}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

BOOST_AUTO_TEST_CASE(lpar) {
    // Data
    std::string input = "(";
    QueuesFixture::out_queue_t correct({{tags::LPAR, ""}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

BOOST_AUTO_TEST_CASE(rpar) {
    // Data
    std::string input = ")";
    QueuesFixture::out_queue_t correct({{tags::RPAR, ""}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

BOOST_AUTO_TEST_CASE(lbrac) {
    // Data
    std::string input = "{";
    QueuesFixture::out_queue_t correct({{tags::LBRAC, ""}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

BOOST_AUTO_TEST_CASE(rbrac) {
    // Data
    std::string input = "}";
    QueuesFixture::out_queue_t correct({{tags::RBRAC, ""}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

BOOST_AUTO_TEST_CASE(comma) {
    // Data
    std::string input = ",";
    QueuesFixture::out_queue_t correct({{tags::COMMA, ""}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

BOOST_AUTO_TEST_CASE(semicolon) {
    // Data
    std::string input = ";";
    QueuesFixture::out_queue_t correct({{tags::SEMICOLON, ""}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

BOOST_AUTO_TEST_CASE(assign) {
    // Data
    std::string input = "=";
    QueuesFixture::out_queue_t correct({{tags::ASSIGN, ""}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

BOOST_AUTO_TEST_CASE(unit_return) {
    // Data
    std::string input = "return";
    QueuesFixture::out_queue_t correct({{tags::RETURN, ""}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

BOOST_AUTO_TEST_CASE(double_literal) {
    // Data
    std::string input = "3.14";
    QueuesFixture::out_queue_t correct({{tags::DOUBLE_LITERAL, "3.14"}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

BOOST_AUTO_TEST_CASE(plus) {
    // Data
    std::string input = "+";
    QueuesFixture::out_queue_t correct({{tags::PLUS, ""}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

BOOST_AUTO_TEST_CASE(minus) {
    // Data
    std::string input = "-";
    QueuesFixture::out_queue_t correct({{tags::MINUS, ""}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

BOOST_AUTO_TEST_CASE(star) {
    // Data
    std::string input = "*";
    QueuesFixture::out_queue_t correct({{tags::STAR, ""}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

BOOST_AUTO_TEST_CASE(slash) {
    // Data
    std::string input = "/";
    QueuesFixture::out_queue_t correct({{tags::SLASH, ""}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

BOOST_AUTO_TEST_CASE(percent) {
    // Data
    std::string input = "%";
    QueuesFixture::out_queue_t correct({{tags::PERCENT, ""}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

BOOST_AUTO_TEST_SUITE_END() // unit_naive_lexes

//! Test special symbols and whitespace ending complex tokens (identifier, return, double literal)
BOOST_AUTO_TEST_SUITE(special_end_complex)

//! Test special symbol ending identifier
BOOST_AUTO_TEST_CASE(identifier_special) {
    // Data
    std::string input = "identifier)";
    QueuesFixture::out_queue_t correct({{tags::IDENTIFIER, "identifier"}, {tags::RPAR, ""}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

//! Test whitespace symbol ending identifier
BOOST_AUTO_TEST_CASE(identifier_whitespace) {
    // Data
    std::string input = "identifier  ";
    QueuesFixture::out_queue_t correct({{tags::IDENTIFIER, "identifier"}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

//! Test special symbol ending identifier
BOOST_AUTO_TEST_CASE(return_special) {
    // Data
    std::string input = "return)";
    QueuesFixture::out_queue_t correct({{tags::RETURN, ""}, {tags::RPAR, ""}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

//! Test whitespace symbol ending identifier
BOOST_AUTO_TEST_CASE(return_whitespace) {
    // Data
    std::string input = "return  ";
    QueuesFixture::out_queue_t correct({{tags::RETURN, ""}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

//! Test special symbol ending identifier
BOOST_AUTO_TEST_CASE(double_special) {
    // Data
    std::string input = "3.14)";
    QueuesFixture::out_queue_t correct({{tags::DOUBLE_LITERAL, "3.14"}, {tags::RPAR, ""}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

//! Test whitespace symbol ending identifier
BOOST_AUTO_TEST_CASE(double_whitespace) {
    // Data
    std::string input = "3.14  ";
    QueuesFixture::out_queue_t correct({{tags::DOUBLE_LITERAL, "3.14"}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

BOOST_AUTO_TEST_SUITE_END() // special_end_complex

//TODO: test errors and exceptions happen where expected
//! Test exceptions and errors on invalid input
BOOST_AUTO_TEST_SUITE(exceptions_and_errors)

//! Test exception and error on invalid input when not expecting anything
BOOST_AUTO_TEST_CASE(invalid_input) {
    // Note: does not require the exact error message, just that an exception is thrown and error appended

    // Data
    std::string input = "$";

    // Prepare fixture
    QueuesFixture q;
    q.load(input);

    // Lex the input (should throw LexerException)
    BOOST_CHECK_THROW(q.lex(), lexer::LexerException);

    // Check only one token was added
    BOOST_TEST_CHECK(q.output.size() == 1);

    // Check the last token is an ERROR
    BOOST_TEST_CHECK(q.output.front().tag == tags::ERROR);
}

BOOST_AUTO_TEST_SUITE_END()

//! Test identifier starting with "return" is lexed properly into an identifier
BOOST_AUTO_TEST_CASE(return_like_identifier) {
    // Data
    std::string input = "return_like_identifier";
    QueuesFixture::out_queue_t correct({{tags::IDENTIFIER, "return_like_identifier"}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

//! Test more complex identifier lexing
BOOST_AUTO_TEST_CASE(complex_identifier) {
    // Data
    std::string input = "this_identifier_15_c0mpl3x";
    QueuesFixture::out_queue_t correct({{tags::IDENTIFIER, "this_identifier_15_c0mpl3x"}, {tags::END, ""}});

    // Test
    test_input(input, correct);
}

//! Test no input lexing
BOOST_AUTO_TEST_CASE(empty) {
    // Data
    std::string input;
    QueuesFixture::out_queue_t correct({{tags::END, ""}});

    // Test
    test_input(input, correct);
}

BOOST_AUTO_TEST_SUITE_END() // Lexer
