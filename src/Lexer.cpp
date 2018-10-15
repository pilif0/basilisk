/** \file Lexer.cpp
 * Lexer implementation
 *
 * \author Filip Smola
 */

#include <basilisk/Lexer.h>

namespace basilisk::lexer {

    /**
     * \brief Lex from an input character buffer into an output Token buffer
     *
     * Use `get` function to obtain characters from an input buffer, lex the resulting string, and use `append` to write
     *  them into an output Token buffer.
     *
     * \param get Function to get next input character
     * \param append Function to write next output Token
     */
    void lex(get_function_t get, append_function_t append) {
        //TODO: implement
    }
}
