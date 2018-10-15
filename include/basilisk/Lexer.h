/** \file Lexer.h
 * Lexer
 *
 * \author Filip Smola
 */
#ifndef BASILISK_LEXER_H
#define BASILISK_LEXER_H

#include <vector>
#include <functional>

// Forward declarations
namespace basilisk::tokens {
    struct Token;
}

//! Namespace for all lexer-related code
namespace basilisk::lexer {
    /** \addtogroup Lexer
     * \brief Lexing module
     *
     * Module that contains any code related exclusively to lexing.
     * The main lexing function is \c lex.
     * It needs a function to get the next input character, and a function to append a Token to the output buffer.
     * Whitespace is ignored when lexing, apart from separating tokens.
     *
     * @{
     */

    //! Input get function type - no arguments and return a single character
    typedef std::function<char ()> get_function_t;
    //! Output append function type - single Token argument and no return
    typedef std::function<void (tokens::Token)> append_function_t;

    void lex(get_function_t get, append_function_t append);

    /**
     * @}
     */

}

#endif //BASILISK_LEXER_H
