/** \file Tokens.h
 * Token definitions
 *
 * \author Filip Smola
 */
#ifndef BASILISK_TOKENS_H
#define BASILISK_TOKENS_H

#include <string>
#include <ostream>

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
        // Note: These identifiers are used to represent the tags in code, text should still use the names as the
        //  grammar definition states them for consistency with documentation.
        // Note: Prefix "kw_" used to signify keyword (and prevent recognition by compiler)
        enum token_tag {
            identifier,
            lpar,
            rpar,
            lbrac,
            rbrac,
            comma,
            semicolon,
            assign,
            kw_return,
            double_literal,
            plus,
            minus,
            star,
            slash,
            percent,
            error,
            end_of_input
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

        friend bool operator==(const Token &lhs, const Token &rhs) {
            return lhs.tag == rhs.tag && lhs.content == rhs.content;
        }

        friend bool operator!=(const Token &lhs, const Token &rhs) { return !(rhs == lhs); }

        friend std::ostream &operator<<(std::ostream &os, const Token &token) {
            // Tag labels in the same order as the enumeration
            static const char* labels[] = {
                    "IDENTIFIER",
                    "LPAR",
                    "RPAR",
                    "LBRAC",
                    "RBRAC",
                    "COMMA",
                    "SEMICOLON",
                    "ASSIGN",
                    "RETURN",
                    "DOUBLE_LITERAL",
                    "PLUS",
                    "MINUS",
                    "STAR",
                    "SLASH",
                    "PERCENT",
                    "ERROR",
                    "END"
            };

            // Append the tag
            if (static_cast<std::size_t>(token.tag) < sizeof(labels) / sizeof(*labels)) {
                os << labels[token.tag];
            } else {
                os << static_cast<int>(token.tag);
            }

            // Append the contents if non-empty
            if (!token.content.empty()) {
                os << "{" << token.content << "}";
            }

            return os;
        }
    };


    /**
     * @}
     */
}

#endif //BASILISK_TOKENS_H
