/** \file Parser.cpp
* Parser implementation
*
* \author Filip Smola
*/

#include <basilisk/Parser.h>
#include <basilisk/Tokens.h>
#include <basilisk/AST.h>

namespace exp = basilisk::ast::expressions;

#include <vector>
#include <sstream>
#include <memory>
#include <string>

//TODO improve function docs
//TODO improve reporting of unexpected tokens (might require a substantial redesign to have sufficient information)
//TODO expose more functions in interface to enable partial parsing and testing?
namespace basilisk::parser {

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

            std::unique_ptr<exp::ParExpression> parse_exp_par();
            std::vector<std::unique_ptr<ast::Expression>> parse_exp_list();
            std::unique_ptr<exp::Expression4> parse_exp4();
            std::unique_ptr<exp::Expression3> parse_exp3();
            std::unique_ptr<exp::Expression2> parse_exp2();
            std::unique_ptr<exp::Expression1> parse_exp1();
        public:
            /**
             * \brief Construct an ExpressionParser on an input token buffer
             *
             * \param get Function to get the next input token
             * \param peek Function to peek at the next input token
             */
            ExpressionParser(const get_function_t &get, const peek_function_t &peek)
                : get(get), peek(peek) {}

            std::unique_ptr<ast::Expression> parse_expression();
    };

    //--- Start ExpressionParser implementation
    /**
     * \brief Parse ParExpression from an input token buffer
     *
     * \return Pointer to resulting ParExpression node
     */
    std::unique_ptr<exp::ParExpression> ExpressionParser::parse_exp_par() {
        // Parenthesised expression -> expecting LPAR Expression RPAR

        // Check starting LPAR
        if (peek(0).tag != tokens::tags::lpar) {
            // Unexpected token
            //TODO test
            std::ostringstream message;
            message << "Unexpected token " << peek(0) << " when parsing ParExpression and expecting LPAR.";
            throw ParserException(message.str());
        }

        // Parse contained Expression
        auto expression = parse_expression();

        // Check for closing RPAR
        if (peek(0).tag == tokens::tags::rpar) {
            // Present -> consume
            get();
        } else {
            // Unexpected token
            //TODO test
            std::ostringstream message;
            message << "Unexpected token " << peek(0) << " when parsing ParExpression and expecting RPAR.";
            throw ParserException(message.str());
        }

        return std::make_unique<exp::ParExpression>(std::move(expression));
    }

    /**
     * \brief Parse list of Expressions from an input token buffer
     *
     * \return Vector of pointers to resulting Expression nodes
     */
    std::vector<std::unique_ptr<ast::Expression>> ExpressionParser::parse_exp_list() {
        // Expression list -> expecting one or more Expressions separated by COMMA

        // Prepare result
        std::vector<std::unique_ptr<ast::Expression>> result;

        // Parse first Expression
        result.push_back(parse_expression());

        // On each following comma, parse another one
        for (tokens::Token t = peek(0); t.tag == tokens::tags::comma; t = peek(0)) {
            // Consume the comma
            get();

            // Parse the Expression
            result.push_back(parse_expression());
        }

        // Not a comma -> end
        return result;
    }

    /**
     * \brief Parse Expression4 from an input token buffer
     *
     * \return Pointer to resulting Expression4 node
     */
    std::unique_ptr<exp::Expression4> ExpressionParser::parse_exp4() {
        // Expression4 -> DOUBLE_LITERAL, LPAR Expression RPAR, IDENTIFIER, IDENTIFIER LPAR (optional expression list) RPAR

        // Check next token
        tokens::Token t = peek(0);
        if (t.tag == tokens::tags::double_literal) {
            // Double literal -> return contents as literal expression

            // Consume token
            get();

            // Parse value
            double value;
            try {
                value = std::stod(t.content);
            } catch (const std::invalid_argument &/*e*/) {
                // Value cannot be parsed
                //TODO test
                std::ostringstream message;
                message << "Double literal \'" << t.content << "\' cannot be parsed into a double value.";
                throw ParserException(message.str());
            } catch (const std::out_of_range &/*e*/) {
                // Value out of range
                //TODO test
                std::ostringstream message;
                message << "Double literal \'" << t.content << "\' is out of the range of double.";
                throw ParserException(message.str());
            }

            // Return DoubleLitExpression
            return std::make_unique<exp::DoubleLitExpression>(value);
        } else if (t.tag == tokens::tags::lpar) {
            // LPAR -> parse and return the ParExpression
            return parse_exp_par();
        } else if (t.tag == tokens::tags::identifier) {
            // IDENTIFIER -> either IDENTIFIER or function call

            // Unpack and consume identifier
            ast::Identifier identifier = get().content;

            // Peek at the next token
            if (peek(0).tag == tokens::tags::lpar) {
                // LPAR -> either RPAR or expression list RPAR

                // Consume token
                get();

                // Peek at the next token
                std::vector<std::unique_ptr<ast::Expression>> expression_list;
                if (peek(0).tag != tokens::tags::rpar) {
                    // Not RPAR -> parse expression list
                    expression_list = parse_exp_list();
                }   // RPAR -> expression list may remain empty

                // Return FuncExpression
                return std::make_unique<exp::FuncExpression>(identifier, std::move(expression_list));
            } else {
                // Was just identifier
                return std::make_unique<exp::IdentifierExpression>(identifier);
            }
        } else {
            // Otherwise -> unexpected token
            //TODO test
            std::ostringstream message;
            message << "Unexpected token " << t << " when parsing Expression(4) and expecting DOUBLE_LITERAL, LPAR or IDENTIFIER.";
            throw ParserException(message.str());
        }
    }

    /**
     * \brief Parse Expression3 from an input token buffer
     *
     * \return Pointer to resulting Expression3 node
     */
    std::unique_ptr<exp::Expression3> ExpressionParser::parse_exp3() {
        // Expression3 -> Expression4 or MINUS Expression3

        // Check for operator
        if (peek(0).tag == tokens::tags::minus) {
            // Minus -> negate rhs

            // Consume operator
            get();

            // Parse rhs
            auto exp3 = parse_exp3();

            // Return MulExpression
            return std::make_unique<exp::NegExpression>(std::move(exp3));
        } else {
            // Absent -> parse ex Expression4
            return parse_exp4();
        }
    }

    /**
     * \brief Parse Expression2 from an input token buffer
     *
     * \return Pointer to resulting Expression2 node
     */
    std::unique_ptr<exp::Expression2> ExpressionParser::parse_exp2() {
        // Expression2 -> Expression3 with optional (STAR or SLASH) Expression2

        // Expression3
        auto exp3 = parse_exp3();

        // Check for operator
        tokens::Token t = peek(0);
        if (t.tag == tokens::tags::star) {
            // Star -> combine with a rhs

            // Consume operator
            get();

            // Parse rhs
            auto exp2 = parse_exp2();

            // Return MulExpression
            return std::make_unique<exp::MulExpression>(std::move(exp3), std::move(exp2));
        } else if (t.tag == tokens::tags::slash) {
            // Minus -> combine with a rhs

            // Consume operator
            get();

            // Parse rhs
            auto exp2 = parse_exp2();

            // Return DivExpression
            return std::make_unique<exp::DivExpression>(std::move(exp3), std::move(exp2));
        } else {
            // Absent -> return just the lhs
            return exp3;
        }
    }

    /**
     * \brief Parse Expression1 from an input token buffer
     *
     * \return Pointer to resulting Expression1 node
     */
    std::unique_ptr<exp::Expression1> ExpressionParser::parse_exp1() {
        // Expression1 -> Expression2 with optional (PLUS or MINUS) Expression1

        // Expression2
        auto exp2 = parse_exp2();

        // Check for operator
        tokens::Token t = peek(0);
        if (t.tag == tokens::tags::plus) {
            // Plus -> combine with a rhs

            // Consume operator
            get();

            // Parse rhs
            auto exp1 = parse_exp1();

            // Return SumExpression
            return std::make_unique<exp::SumExpression>(std::move(exp2), std::move(exp1));
        } else if (t.tag == tokens::tags::minus) {
            // Minus -> combine with a rhs

            // Consume operator
            get();

            // Parse rhs
            auto exp1 = parse_exp1();

            // Return SubExpression
            return std::make_unique<exp::SubExpression>(std::move(exp2), std::move(exp1));
        } else {
            // Absent -> return just the lhs
            return exp2;
        }
    }

    /**
     * \brief Parse Expression from an input token buffer
     *
     * \return Pointer to resulting Expression node
     */
    std::unique_ptr<ast::Expression> ExpressionParser::parse_expression() {
        // Expression -> Expression1 with optional PERCENT Expression

        // Expression1
        auto exp1 = parse_exp1();

        // Check for modulo operator
        if (peek(0).tag == tokens::tags::percent) {
            // Present -> combine with a rhs

            // Consume operator
            get();

            // Parse rhs
            auto exp = parse_expression();

            // Return ModExpression
            return std::make_unique<exp::ModExpression>(std::move(exp1), std::move(exp));
        } else {
            // Absent -> return just the lhs
            return exp1;
        }
    }
    //--- End ExpressionParser implementation

    /**
     * \brief Parse Variable Definition from an input token buffer
     *
     * \param get Function to get the next input token
     * \param peek Function to peek at the next input token
     * \return Pointer to resulting Variable Definition node
     */
    std::unique_ptr<ast::VariableDefinition> parse_definition_var(const get_function_t &get, const peek_function_t &peek) {
        // Variable definition -> expecting IDENTIFIER, ASSIGN, value STATEMENT and SEMICOLON

        // Identifier
        std::string id;
        {
            // Get the token
            tokens::Token t = get();

            // Check the tag
            if (t.tag != tokens::tags::identifier) {
                // Unexpected token
                //TODO test
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Variable Definition and expecting IDENTIFIER.";
                throw ParserException(message.str());
            }

            // Extract content
            id = t.content;
        }

        // Assign
        {
            // Get the token
            tokens::Token t = get();

            // Check the tag
            if (t.tag != tokens::tags::assign) {
                // Unexpected token
                //TODO test
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Variable Definition and expecting ASSIGN.";
                throw ParserException(message.str());
            }
        }

        // Value expression
        auto val = ExpressionParser(get, peek).parse_expression();

        // Semicolon
        {
            // Get the token
            tokens::Token t = get();

            // Check the tag
            if (t.tag != tokens::tags::semicolon) {
                // Unexpected token
                //TODO test
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Variable Definition and expecting SEMICOLON.";
                throw ParserException(message.str());
            }
        }

        return std::make_unique<ast::VariableDefinition>(id, std::move(val));
    }

    /**
     * \brief Parse Statement from an input token buffer
     *
     * \param get Function to get the next input token
     * \param peek Function to peek at the next input token
     * \return Pointer to resulting Statement node
     */
    std::unique_ptr<ast::Statement> parse_statement(const get_function_t &get, const peek_function_t &peek) {
        // Statement -> expecting RETURN Expression SEMICOLON, VariableDefinition, or Expression SEMICOLON

        // Check first token
        tokens::Token t = peek(0);
        if (t.tag == tokens::tags::kw_return) {
            // RETURN -> ReturnStatement

            // Consume tag
            get();

            // Parse Expression
            auto expression = ExpressionParser(get, peek).parse_expression();

            // Check SEMICOLON
            if (t.tag == tokens::tags::semicolon) {
                // Present -> consume
                get();
            } else {
                // Unexpected token
                //TODO test
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Return Statement and expecting SEMICOLON.";
                throw ParserException(message.str());
            }

            // Return result
            return std::make_unique<ast::ReturnStatement>(std::move(expression));
        } else if (t.tag == tokens::tags::identifier) {
            // IDENTIFIER -> VariableDefinition or StandaloneStatement

            // Check second token
            if (peek(1).tag == tokens::tags::assign) {
                // ASSIGN -> VariableDefinition (Expression cannot contain ASSIGN)

                // Parse the definition
                auto def = parse_definition_var(get, peek);

                // Return variable statement
                return std::make_unique<ast::VariableStatement>(std::move(def));
            } else {
                // Otherwise -> StandaloneStatement (VariableDefinition requires ASSIGN)

                // Parse expression
                auto expression = ExpressionParser(get, peek).parse_expression();

                // Return StandaloneStatement
                return std::make_unique<ast::StandaloneStatement>(std::move(expression));
            }
        } else {
            // Otherwise -> StandaloneStatement

            // Parse expression
            auto expression = ExpressionParser(get, peek).parse_expression();

            // Return StandaloneStatement
            return std::make_unique<ast::StandaloneStatement>(std::move(expression));
        }
    }

    /**
     * \brief Parse Function Definition from an input token buffer
     *
     * \param get Function to get the next input token
     * \param peek Function to peek at the next input token
     * \return Pointer to resulting Function Definition node
     */
    std::unique_ptr<ast::FunctionDefinition> parse_definition_func(const get_function_t &get, const peek_function_t &peek) {
        // Function definition -> expecting IDENTIFIER, LPAR, optional identifier list, RPAR, LBRAC, statement block and RBRAC

        // Identifier
        std::string id;
        {
            // Get the token
            tokens::Token t = get();

            // Check the tag
            if (t.tag != tokens::tags::identifier) {
                // Unexpected token
                //TODO test
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Function Definition and expecting IDENTIFIER.";
                throw ParserException(message.str());
            }

            // Extract content
            id = t.content;
        }

        // Left parenthesis
        {
            // Get the token
            tokens::Token t = get();

            // Check the tag
            if (t.tag != tokens::tags::lpar) {
                // Unexpected token
                //TODO test
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Function Definition and expecting LPAR.";
                throw ParserException(message.str());
            }
        }

        // Arguments
        std::vector<ast::Identifier> args;
        {
            // Gather identifiers until right parenthesis
            tokens::Token t = peek(0);
            while (t.tag != tokens::tags::rpar) {
                // Check the token is identifier
                if (t.tag != tokens::tags::identifier) {
                    // Unexpected token
                    //TODO test
                    std::ostringstream message;
                    message << "Unexpected token " << t << " when parsing Function Definition and expecting IDENTIFIER.";
                    throw ParserException(message.str());
                }

                // Add the identifier content, consuming the token
                args.push_back(get().content);

                // Check next is COMMA or RPAR
                t = peek(0);
                if (t.tag == tokens::tags::comma) {
                    // COMMA -> consume
                    get();
                } else if (t.tag != tokens::tags::rpar) {
                    // Not RPAR -> unexpected token
                    //TODO test
                    std::ostringstream message;
                    message << "Unexpected token " << t << " when parsing Function Definition and expecting COMMA or RPAR.";
                    throw ParserException(message.str());
                }   // RPAR -> leave to terminate
            }
        }

        // Right parenthesis
        {
            // Get the token
            tokens::Token t = get();

            // Check the tag
            if (t.tag != tokens::tags::rpar) {
                // Unexpected token
                //TODO test
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Function Definition and expecting RPAR.";
                throw ParserException(message.str());
            }
        }

        // Left bracket
        {
            // Get the token
            tokens::Token t = get();

            // Check the tag
            if (t.tag != tokens::tags::lbrac) {
                // Unexpected token
                //TODO test
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Function Definition and expecting LBRAC.";
                throw ParserException(message.str());
            }
        }

        // Body
        std::vector<std::unique_ptr<ast::Statement>> body;
        {
            // Gather statements until right bracket
            for (tokens::Token t = peek(0); t.tag != tokens::tags::rbrac; t = peek(0)) {
                // Parse statement
                auto statement = parse_statement(get, peek);

                // Add to body
                body.push_back(std::move(statement));
            }
        }

        // Right bracket
        {
            // Get the token
            tokens::Token t = get();

            // Check the tag
            if (t.tag != tokens::tags::rbrac) {
                // Unexpected token
                //TODO test
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Function Definition and expecting RBRAC.";
                throw ParserException(message.str());
            }
        }

        return std::make_unique<ast::FunctionDefinition>(id, args, std::move(body));
    }

    /**
     * \brief Parse Definition from an input token buffer
     *
     * \param get Function to get the next input token
     * \param peek Function to peek at the next input token
     * \return Pointer to resulting Definition node
     */
    std::unique_ptr<ast::Definition> parse_definition(const get_function_t &get, const peek_function_t &peek) {
        // Definition -> expecting IDENTIFIER followed by LPAR for function definition, ASSIGN for variable definition

        // Decide by second token
        tokens::Token t = peek(1);
        if (t.tag == tokens::tags::lpar) {
            // Function definition
            return parse_definition_func(get, peek);
        } else if (t.tag == tokens::tags::assign) {
            // Variable definition
            return parse_definition_var(get, peek);
        } else {
            // Unexpected token
            //TODO test
            std::ostringstream message;
            message << "Unexpected token " << t << " when parsing Definition and expecting LPAR or ASSIGN.";
            throw ParserException(message.str());
        }
    }

    /**
     * \brief Parse Program from an input token buffer
     *
     * Use `get` function to obtain tokens from an input buffer, parse those into an AST and return that tree.
     *
     * \param get Function to get the next input token
     * \param peek Function to peek at the next input token
     * \return Resulting Program node
     */
    ast::Program parse(const get_function_t &get, const peek_function_t &peek) {
        // Root -> in Program node -> expecting set of variable and function definitions
        std::vector<std::unique_ptr<ast::Definition>> definitions{};

        // Try to gather definitions until END
        for (tokens::Token t = peek(0); t.tag != tokens::tags::end_of_input; t = peek(0)) {
            // All definitions start with an identifier
            if (t.tag == tokens::tags::identifier) {
                // Consume definition
                definitions.push_back(parse_definition(get, peek));
            } else if (t.tag == tokens::tags::error) {
                // Lexer error
                //TODO test
                std::ostringstream message;
                message << "Lexer error: " << t.content;
                throw ParserException(message.str());
            } else {
                // Unexpected token
                //TODO test
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Program and expecting IDENTIFIER as start of Definition.";
                throw ParserException(message.str());
            }
        }


        // There has to be at least one definition
        if (definitions.empty()) {
            //TODO test
            throw ParserException("Program has to have at least one definition.");
        }

        // Return the program with the gathered definitions
        return ast::Program(std::move(definitions));
    }
}
