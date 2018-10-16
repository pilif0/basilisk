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

//! Fixture that sets up two queues for use as lexer input and output
struct QueuesFixture {
    //! Lexer input queue type
    typedef std::queue<char> in_queue_t;
    //! Lexer output queue type
    typedef std::queue<basilisk::tokens::Token> out_queue_t;

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
    void append(const basilisk::tokens::Token &t) { output.push(t); }
};

//! Test fixture is working correctly
BOOST_AUTO_TEST_CASE( fixture_test ) {
    // Constants in this test
    constexpr char subject_character = 'a';
    basilisk::tokens::Token subject_token{basilisk::tokens::tags::ASSIGN, {}};

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
