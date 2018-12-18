/** \file Parser.h
 * Parser
 *
 * \author Filip Smola
 */
#ifndef BASILISK_PARSER_H
#define BASILISK_PARSER_H

#include <vector>
#include <functional>
#include <exception>

// Forward declarations
namespace basilisk::tokens {
    struct Token;
}
namespace basilisk::ast {
    class Program;
}

//! Namespace for all parser-related code
namespace basilisk::parser {
    /** \addtogroup Parser
     * \brief Parsing module
     *
     * Module that contains any code related exclusively to parsing.
     * The main parsing function is \c parse.
     * It needs a function to get the next input token and returns the parse tree resulting from consuming all tokens
     *  until the first \c END token.
     *
     * @{
     */

    //! Input get function type - no arguments and return a single character
    typedef std::function<tokens::Token ()> get_function_t;
    //! Input peek function type - no arguments and return a single character
    typedef std::function<tokens::Token ()> peek_function_t;

    ast::Program parse(const get_function_t &get, const peek_function_t &peek);

    /** \class ParserException
     * \brief Exception during parsing (for example an unexpected token)
     */
    class ParserException : public std::runtime_error {
        public:
            explicit ParserException(const std::string &message) : std::runtime_error(message) {}
    };

    /**
     * @}
     */

}

#endif //BASILISK_PARSER_H
