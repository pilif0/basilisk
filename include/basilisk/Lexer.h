/** \file Lexer.h
 * Lexer
 *
 * \author Filip Smola
 */
#ifndef BASILISK_LEXER_H
#define BASILISK_LEXER_H

#include <vector>
#include <functional>
#include <exception>

// Forward declarations
///\cond
namespace basilisk::tokens {
    struct Token;
}
///\endcond

/** \namespace basilisk::lexer
 * \brief Namespace for all lexer-related code
 *
 * The main lexing function is \ref lex.
 * It needs a function to get the next input character (\ref get_function_t), a function to peek at the next input
 *  character (\ref peek_function_t, and a function to append a Token to the output buffer (\ref append_function_t).
 * Whitespace is ignored when lexing, apart from separating tokens.
 */
namespace basilisk::lexer {
    //! Input get function type - no arguments and return a single character
    typedef std::function<char ()> get_function_t;
    //! Input peek function type - no arguments and return a single character
    typedef std::function<char ()> peek_function_t;
    //! Output append function type - single Token argument and no return
    typedef std::function<void (tokens::Token)> append_function_t;

    void lex(const get_function_t &get, const peek_function_t &peek, const append_function_t &append);

    /** \class LexerException
     * \brief Exception during lexing (for example an invalid character)
     */
    class LexerException : public std::runtime_error {
        public:
            //! Construct a lexer exception from its message
            explicit LexerException(const std::string &message) : std::runtime_error(message) {}
    };

}

#endif //BASILISK_LEXER_H
