/** \file Tokens.h
 * Token definitions
 *
 * \author Filip Smola
 */
#ifndef BASILISK_TOKENS_H
#define BASILISK_TOKENS_H

#include <string>

//! Token definitions
namespace basilisk::tokens {
    /** \addtogroup Tokens
     * \brief Token definitions
     *
     * Definitions of tokens created by the lexer and used by the parser.
     *
     * Tokens:
     *  - `IDENTIFIER`      = letter followed by alphanumeric or `_`
     *  - `LPAR`            = `(`
     *  - `RPAR`            = `)`
     *  - `LBRAC`           = `{`
     *  - `RBRAC`           = '}`
     *  - `COMMA`           = `,`
     *  - `SEMICOLON`       = `;`
     *  - `ASSIGN`          = `=`
     *  - `RETURN`          = `return`
     *  - `DOUBLE_LITERAL`  = at least one digit followed by a decimal point and then at least one digit
     *  - `PLUS`            = `+`
     *  - `MINUS`           = `-`
     *  - `STAR`            = `*`
     *  - `SLASH`           = `/`
     *  - `PERCENT`         = `%`
     *  - `END`             = represents the end of input
     *  - `ERROR`           = represents a lexing error
     * @{
     */

    //! Token tags
    namespace tags {
        /** \enum token_tag
         *  \brief Token tag
         */
        enum token_tag {
            IDENTIFIER,
            LPAR,
            RPAR.
            LBRAC,
            RBRAC,
            COMMA,
            SEMICOLON,
            ASSIGN,
            RETURN,
            DOUBLE_LITERAL,
            PLUS,
            MINUS,
            STAR,
            SLASH,
            PERCENT,
            ERROR
        };
    }

    /** \struct Token
     *  \brief Token information
     *
     *  Token information containing at least the token tag.
     *  Also contains the token contents (string) if the tag is not enough (for example with identifier tokens).
     */
    struct Token {
        //! Token tag
        tags::token_tag tag;
        //! Tag content if tag is not enough, empty otherwise
        std::string content;
    };


    /**
     * @}
     */
}

#endif //BASILISK_TOKENS_H
