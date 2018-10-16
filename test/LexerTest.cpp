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
     * \return Popped character
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
     * \brief Push a token to the end of the output queue
     *
     * \param t Token to push
     */
    void append(const tokens::Token &t) { output.push(t); }
};

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

//! Test single naive tokens lex (excluding special tokens END and ERROR)
BOOST_AUTO_TEST_CASE(unit_naive_lexes) {
    // Dataset - (input, token) pairs
    std::map<std::string, tokens::Token> data{
        {"identifier", {tags::IDENTIFIER, "identifier"}},
        {"(", {tags::LPAR, ""}},
        {")", {tags::RPAR, ""}},
        {"{", {tags::LBRAC, ""}},
        {"}", {tags::RBRAC, ""}},
        {",", {tags::COMMA, ""}},
        {";", {tags::SEMICOLON, ""}},
        {"=", {tags::ASSIGN, ""}},
        {"return", {tags::RETURN, ""}},
        {"3.14", {tags::DOUBLE_LITERAL, "3.14"}},
        {"+", {tags::PLUS, ""}},
        {"-", {tags::MINUS, ""}},
        {"*", {tags::STAR, ""}},
        {"/", {tags::SLASH, ""}},
        {"%", {tags::PERCENT, ""}}};

    // Test each input results in the assigned token
    for(auto pair : data) {
        // Prepare fixture
        QueuesFixture q;

        // Push input
        for(const char& c : pair.first) {
            q.input.push(c);
        }

        // Lex the input
        auto get_f = std::bind(&QueuesFixture::get, &q);
        auto append_f = std::bind(&QueuesFixture::append, &q, std::placeholders::_1);
        lexer::lex(get_f, append_f);

        // Check tokens were added (token for input and END)
        BOOST_TEST_CHECK(q.output.size() == 2);

        // Check the token is correct if present
        if (q.output.size() == 2) {
            BOOST_TEST_CHECK(q.output.front() == pair.second,
                             "\"" << pair.first << "\" should lex to " << pair.second << ", instead lexes to "
                                  << q.output.front());
        }
    }
}
