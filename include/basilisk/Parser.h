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
///\cond
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
        class Parenthesised;
        class LiteralDouble;
        class FunctionCall;
        class IdentifierExpression;
    }
    class Statement;
    namespace statements {
        class Return;
        class Discard;
        class Assignment;
    }
    class Definition;
    namespace definitions {
        class Function;
        class Variable;
    }
    class Program;
}
///\endcond

/** \namespace basilisk::parser
 * \brief Namespace for all parser-related code
 *
 * The main parsing tool is \ref ProgramParser, which returns the parse tree resulting from consuming all tokens until
 *  the first \c END token.
 * Each parser needs a function to get the next input token (\ref get_f_t) and a function to peek at the input tokens
 *  (\ref peek_f_t).
 */
namespace basilisk::parser {
    /**
     * \brief Input get function type
     *
     * Input get function type.
     * Takes no argument.
     * Pop the top element from the buffer and returns it.
     */
    typedef std::function<tokens::Token ()> get_f_t;
    /**
     * \brief Input peek function type
     *
     * Input peek function type.
     * Takes an unsigned argument that is the offset of the token (top of the buffer has offset `0`).
     * Returns the token `offset` elements from the top of the buffer.
     */
    typedef std::function<tokens::Token (unsigned)> peek_f_t;

    /** \class ExpressionParser
     * \brief Parser dedicated to expressions
     *
     * Dedicated parser for expressions.
     * This class groups expression-specific parsing together.
     */
    class ExpressionParser {
        private:
            //! Function to get the next input token
            const get_f_t &get;
            //! Function to peek at the next input token
            const peek_f_t &peek;
        public:
            /**
             * \brief Construct an Expression Parser on an input token buffer
             *
             * \param get Function to get the next input token
             * \param peek Function to peek at the next input token
             */
            ExpressionParser(const get_f_t &get, const peek_f_t &peek)
                    : get(get), peek(peek) {}

            std::unique_ptr<ast::expressions::Parenthesised> parenthesised();
            std::vector<std::unique_ptr<ast::Expression>> list();
            std::unique_ptr<ast::expressions::LiteralDouble> literal_double();
            std::unique_ptr<ast::expressions::FunctionCall> function_call();
            std::unique_ptr<ast::expressions::IdentifierExpression> identifier();
            std::unique_ptr<ast::expressions::Expression4> expression_4();
            std::unique_ptr<ast::expressions::Expression3> expression_3();
            std::unique_ptr<ast::expressions::Expression2> expression_2();
            std::unique_ptr<ast::expressions::Expression1> expression_1();
            std::unique_ptr<ast::Expression> expression();
    };

    /** \class StatementParser
     * \brief Parser dedicated to statements
     *
     * Dedicated parser for statements.
     * This class groups statement-specific parsing together.
     */
    class StatementParser {
        private:
            //! Function to get the next input token
            const get_f_t &get;
            //! Function to peek at the next input token
            const peek_f_t &peek;
        public:
            /**
             * \brief Construct a Statement Parser on an input token buffer
             *
             * \param get Function to get the next input token
             * \param peek Function to peek at the next input token
             */
            StatementParser(const get_f_t &get, const peek_f_t &peek)
                    : get(get), peek(peek) {}

            std::unique_ptr<ast::statements::Return> return_kw();
            std::unique_ptr<ast::statements::Discard> discard();
            std::unique_ptr<ast::statements::Assignment> assignment();
            std::unique_ptr<ast::Statement> statement();
    };

    /** \class DefinitionParser
     * \brief Parser dedicated to definitions
     *
     * Dedicated parser for definitions.
     * This class groups definition-specific parsing together.
     */
    class DefinitionParser {
        private:
            //! Function to get the next input token
            const get_f_t &get;
            //! Function to peek at the next input token
            const peek_f_t &peek;
        public:
            /**
             * \brief Construct a Definition Parser on an input token buffer
             *
             * \param get Function to get the next input token
             * \param peek Function to peek at the next input token
             */
            DefinitionParser(const get_f_t &get, const peek_f_t &peek)
                    : get(get), peek(peek) {}

            std::unique_ptr<ast::definitions::Variable> variable();
            std::unique_ptr<ast::definitions::Function> function();
            std::unique_ptr<ast::Definition> definition();
    };

    /** \class ProgramParser
     * \brief Parser dedicated to programs
     *
     * Dedicated parser for programs.
     * This class groups program-specific parsing together.
     */
    class ProgramParser {
        private:
            //! Function to get the next input token
            const get_f_t &get;
            //! Function to peek at the next input token
            const peek_f_t &peek;
        public:
            /**
             * \brief Construct a Program Parser on an input token buffer
             *
             * \param get Function to get the next input token
             * \param peek Function to peek at the next input token
             */
            ProgramParser(const get_f_t &get, const peek_f_t &peek)
                    : get(get), peek(peek) {}

            ast::Program program();
    };

    /** \class ParserException
     * \brief Exception during parsing (for example an unexpected token)
     */
    class ParserException : public std::runtime_error {
        public:
            //! Construct a parser exception from its message
            explicit ParserException(const std::string &message) : std::runtime_error(message) {}
    };
}

#endif //BASILISK_PARSER_H
