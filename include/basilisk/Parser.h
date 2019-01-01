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
#include <memory>

// Forward declarations
namespace basilisk::tokens {
    struct Token;
}
namespace basilisk::ast {
    class Expression;
    namespace expressions {
        class Expression1;
        class Expression2;
        class Expression3;
        class Expression4;
        class ParExpression;
    }
    class Statement;
    class Definition;
    class FunctionDefinition;
    class VariableDefinition;
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
    //! Input peek function type - one input (offset to peek at with next being `0`) and return a single character
    typedef std::function<tokens::Token (unsigned)> peek_function_t;

    /** \class ExpressionParser
     * \brief Parser dedicated to expressions
     *
     * Dedicated parser for expressions.
     * This class is required due to the cyclic dependencies arising from nested expressions.
     * Additionally it groups expression-specific parsing together.
     */
    class ExpressionParser {
        private:
            //! Function to get the next input token
            const get_function_t &get;
            //! Function to peek at the next input token
            const peek_function_t &peek;
        public:
            /**
             * \brief Construct an ExpressionParser on an input token buffer
             *
             * \param get Function to get the next input token
             * \param peek Function to peek at the next input token
             */
            ExpressionParser(const get_function_t &get, const peek_function_t &peek)
                    : get(get), peek(peek) {}

            std::unique_ptr<ast::expressions::ParExpression> parse_exp_par();
            std::vector<std::unique_ptr<ast::Expression>> parse_exp_list();
            std::unique_ptr<ast::expressions::Expression4> parse_exp4();
            std::unique_ptr<ast::expressions::Expression3> parse_exp3();
            std::unique_ptr<ast::expressions::Expression2> parse_exp2();
            std::unique_ptr<ast::expressions::Expression1> parse_exp1();
            std::unique_ptr<ast::Expression> parse_expression();
    };

    std::unique_ptr<ast::VariableDefinition> parse_definition_var(const get_function_t &get, const peek_function_t &peek);
    std::unique_ptr<ast::Statement> parse_statement(const get_function_t &get, const peek_function_t &peek);
    std::unique_ptr<ast::FunctionDefinition> parse_definition_func(const get_function_t &get, const peek_function_t &peek);
    std::unique_ptr<ast::Definition> parse_definition(const get_function_t &get, const peek_function_t &peek);
    ast::Program parse_program(const get_function_t &get, const peek_function_t &peek);

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
