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

namespace basilisk::parser {

    //--- Start ExpressionParser implementation
    /**
     * \brief Parse Parenthesised Expression node from the input
     *
     * \return Pointer to resulting Parenthesised Expression node
     */
    std::unique_ptr<exp::Parenthesised> ExpressionParser::parenthesised() {
        // Parenthesised expression -> expecting LPAR Expression RPAR

        // Check starting LPAR
        if (peek(0).tag == tokens::tags::lpar) {
            // Present -> consume
            get();
        } else {
            // Absent -> unexpected token
            std::ostringstream message;
            message << "Unexpected token " << peek(0) << " when parsing Parenthesised and expecting LPAR.";
            throw ParserException(message.str());
        }

        // Parse contained Expression
        auto expr = expression();

        // Check for closing RPAR
        if (peek(0).tag == tokens::tags::rpar) {
            // Present -> consume
            get();
        } else {
            // Unexpected token
            std::ostringstream message;
            message << "Unexpected token " << peek(0) << " when parsing Parenthesised and expecting RPAR.";
            throw ParserException(message.str());
        }

        return std::make_unique<exp::Parenthesised>(std::move(expr));
    }

    /**
     * \brief Parse list of Expression nodes from the input
     *
     * \return Vector of pointers to resulting Expression nodes
     */
    std::vector<std::unique_ptr<ast::Expression>> ExpressionParser::list() {
        // Expression list -> expecting one or more Expressions separated by COMMA

        // Prepare result
        std::vector<std::unique_ptr<ast::Expression>> result;

        // Parse first Expression
        result.push_back(expression());

        // On each following comma, parse another one
        for (tokens::Token t = peek(0); t.tag == tokens::tags::comma; t = peek(0)) {
            // Consume the comma
            get();

            // Parse the Expression
            result.push_back(expression());
        }

        // Not a comma -> end
        return result;
    }

    /**
     * \brief Parse Double Literal Expression node from the input
     *
     * \return Pointers to the resulting Double Literal Expression node
     */
    std::unique_ptr<ast::expressions::LiteralDouble> ExpressionParser::literal_double() {
        // Double Literal Expression -> expecting DOUBLE_LITERAL

        // DOUBLE_LITERAL
        tokens::Token t = get();
        if (t.tag == tokens::tags::double_literal) {
            // DOUBLE_LITERAL -> parse contents and return

            // Parse value
            double value;
            try {
                value = std::stod(t.content);
            } catch (const std::invalid_argument &/*e*/) {
                // Value cannot be parsed
                std::ostringstream message;
                message << "Double literal \'" << t.content << "\' cannot be parsed into a double value.";
                throw ParserException(message.str());
            } catch (const std::out_of_range &/*e*/) {
                // Value out of range
                std::ostringstream message;
                message << "Double literal \'" << t.content << "\' is out of the range of double.";
                throw ParserException(message.str());
            }

            // Return LiteralDouble
            return std::make_unique<exp::LiteralDouble>(value);
        } else {
            // Unexpected token
            std::ostringstream message;
            message << "Unexpected token " << peek(0) << " when parsing LiteralDouble and expecting DOUBLE_LITERAL.";
            throw ParserException(message.str());
        }
    }

    /**
     * \brief Parse Function Call Expression node from the input
     *
     * \return Pointers to the resulting Function Call Expression node
     */
    std::unique_ptr<ast::expressions::FunctionCall> ExpressionParser::function_call() {
        // Function Call Expression -> expecting IDENTIFIER LPAR (optional expression list) RPAR

        // Identifier
        std::string id;
        {
            // Get the token
            tokens::Token t = get();

            // Check the tag
            if (t.tag != tokens::tags::identifier) {
                // Unexpected token
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Function Call Expression and expecting IDENTIFIER.";
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
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Function Call Expression and expecting LPAR.";
                throw ParserException(message.str());
            }
        }

        // Arguments
        std::vector<std::unique_ptr<ast::Expression>> arguments;
        if (peek(0).tag != tokens::tags::rpar) {
            // Not RPAR -> parse expression list
            arguments = list();
        }

        // Right parenthesis
        {
            // Get the token
            tokens::Token t = get();

            // Check the tag
            if (t.tag != tokens::tags::rpar) {
                // Unexpected token
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Function Call Expression and expecting RPAR.";
                throw ParserException(message.str());
            }
        }

        return std::make_unique<exp::FunctionCall>(id, std::move(arguments));
    }

    /**
     * \brief Parse Identifier Expression node from the input
     *
     * \return Pointers to the resulting Identifier Expression node
     */
    std::unique_ptr<ast::expressions::IdentifierExpression> ExpressionParser::identifier() {
        // Identifier Expression -> expecting IDENTIFIER

        // Identifier
        std::string id;
        {
            // Get the token
            tokens::Token t = get();

            // Check the tag
            if (t.tag != tokens::tags::identifier) {
                // Unexpected token
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Identifier Expression and expecting IDENTIFIER.";
                throw ParserException(message.str());
            }

            // Extract content
            id = t.content;
        }

        return std::make_unique<exp::IdentifierExpression>(id);
    }

    /**
     * \brief Parse Expression4 node from the input
     *
     * \return Pointer to resulting Expression4 node
     */
    std::unique_ptr<exp::Expression4> ExpressionParser::expression_4() {
        // Expression4 -> DOUBLE_LITERAL, LPAR Expression RPAR, IDENTIFIER, IDENTIFIER LPAR (optional expression list) RPAR

        // Check next token
        tokens::Token t = peek(0);
        if (t.tag == tokens::tags::double_literal) {
            // Double literal
            return literal_double();
        } else if (t.tag == tokens::tags::lpar) {
            // LPAR -> parse and return the Parenthesised
            return parenthesised();
        } else if (t.tag == tokens::tags::identifier) {
            // IDENTIFIER -> either IDENTIFIER or function call

            // Peek at the next token
            if (peek(1).tag == tokens::tags::lpar) {
                // LPAR -> Function Call Expression
                return function_call();
            } else {
                // Otherwise -> Identifier Expression
                return identifier();
            }
        } else {
            // Otherwise -> unexpected token
            std::ostringstream message;
            message << "Unexpected token " << t << " when parsing Expression(4) and expecting DOUBLE_LITERAL, LPAR or IDENTIFIER.";
            throw ParserException(message.str());
        }
    }

    /**
     * \brief Parse Expression3 node from the input
     *
     * \return Pointer to resulting Expression3 node
     */
    std::unique_ptr<exp::Expression3> ExpressionParser::expression_3() {
        // Expression3 -> Expression4 or MINUS Expression3

        // Check for operator
        if (peek(0).tag == tokens::tags::minus) {
            // Minus -> negate rhs

            // Consume operator
            get();

            // Parse rhs
            auto exp3 = expression_3();

            // Return Multiplication
            return std::make_unique<exp::NumericNegation>(std::move(exp3));
        } else {
            // Absent -> parse ex Expression4
            return expression_4();
        }
    }

    /**
     * \brief Parse Expression2 node from the input
     *
     * \return Pointer to resulting Expression2 node
     */
    std::unique_ptr<exp::Expression2> ExpressionParser::expression_2() {
        // Expression2 -> Expression3 with optional (STAR or SLASH) Expression2

        // Expression3
        auto exp3 = expression_3();

        // Check for operator
        tokens::Token t = peek(0);
        if (t.tag == tokens::tags::star) {
            // Star -> combine with a rhs

            // Consume operator
            get();

            // Parse rhs
            auto exp2 = expression_2();

            // Return Multiplication
            return std::make_unique<exp::Multiplication>(std::move(exp3), std::move(exp2));
        } else if (t.tag == tokens::tags::slash) {
            // Minus -> combine with a rhs

            // Consume operator
            get();

            // Parse rhs
            auto exp2 = expression_2();

            // Return Division
            return std::make_unique<exp::Division>(std::move(exp3), std::move(exp2));
        } else {
            // Absent -> return just the lhs
            return exp3;
        }
    }

    /**
     * \brief Parse Expression1 node from the input
     *
     * \return Pointer to resulting Expression1 node
     */
    std::unique_ptr<exp::Expression1> ExpressionParser::expression_1() {
        // Expression1 -> Expression2 with optional (PLUS or MINUS) Expression1

        // Expression2
        auto exp2 = expression_2();

        // Check for operator
        tokens::Token t = peek(0);
        if (t.tag == tokens::tags::plus) {
            // Plus -> combine with a rhs

            // Consume operator
            get();

            // Parse rhs
            auto exp1 = expression_1();

            // Return Summation
            return std::make_unique<exp::Summation>(std::move(exp2), std::move(exp1));
        } else if (t.tag == tokens::tags::minus) {
            // Minus -> combine with a rhs

            // Consume operator
            get();

            // Parse rhs
            auto exp1 = expression_1();

            // Return Subtraction
            return std::make_unique<exp::Subtraction>(std::move(exp2), std::move(exp1));
        } else {
            // Absent -> return just the lhs
            return exp2;
        }
    }

    /**
     * \brief Parse Expression node from the input
     *
     * \return Pointer to resulting Expression node
     */
    std::unique_ptr<ast::Expression> ExpressionParser::expression() {
        // Expression -> Expression1 with optional PERCENT Expression

        // Expression1
        auto exp1 = expression_1();

        // Check for modulo operator
        if (peek(0).tag == tokens::tags::percent) {
            // Present -> combine with a rhs

            // Consume operator
            get();

            // Parse rhs
            auto exp = expression();

            // Return Modulo
            return std::make_unique<exp::Modulo>(std::move(exp1), std::move(exp));
        } else {
            // Absent -> return just the lhs
            return exp1;
        }
    }
    //--- End ExpressionParser implementation

    //--- Start StatementParser implementation
    /**
     * \brief Parse Return Statement node from the input
     *
     * \return Pointer to resulting Return Statement node
     */
    std::unique_ptr<ast::statements::Return> StatementParser::return_kw() {
        // Return Statement -> expecting RETURN Expression SEMICOLON

        // RETURN
        {
            // Get the token
            tokens::Token t = get();

            // Check the tag
            if (t.tag != tokens::tags::kw_return) {
                // Unexpected token
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Return Statement and expecting RETURN.";
                throw ParserException(message.str());
            }
        }

        // Expression
        auto expr = ExpressionParser(get, peek).expression();

        // SEMICOLON
        {
            // Get the token
            tokens::Token t = get();

            // Check the tag
            if (t.tag != tokens::tags::semicolon) {
                // Unexpected token
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Return Statement and expecting SEMICOLON.";
                throw ParserException(message.str());
            }
        }

        return std::make_unique<ast::statements::Return>(std::move(expr));
    }

    /**
     * \brief Parse Discard Statement node from the input
     *
     * \return Pointer to resulting Discard Statement node
     */
    std::unique_ptr<ast::statements::Discard> StatementParser::discard() {
        // Discard Statement -> expecting Expression SEMICOLON

        // Expression
        auto expr = ExpressionParser(get, peek).expression();

        // SEMICOLON
        {
            // Get the token
            tokens::Token t = get();

            // Check the tag
            if (t.tag != tokens::tags::semicolon) {
                // Unexpected token
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Discard Statement and expecting SEMICOLON.";
                throw ParserException(message.str());
            }
        }

        return std::make_unique<ast::statements::Discard>(std::move(expr));
    }

    /**
     * \brief Parse Variable Statement node from the input
     *
     * \return Pointer to resulting Variable Statement node
     */
    std::unique_ptr<ast::statements::Assignment> StatementParser::assignment() {
        // Assignment statement -> expecting IDENTIFIER, ASSIGN, value expression and SEMICOLON

        // Identifier
        std::string id;
        {
            // Get the token
            tokens::Token t = get();

            // Check the tag
            if (t.tag != tokens::tags::identifier) {
                // Unexpected token
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Assignment Statement and expecting IDENTIFIER.";
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
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Assignment Statement and expecting ASSIGN.";
                throw ParserException(message.str());
            }
        }

        // Value expression
        auto val = ExpressionParser(get, peek).expression();

        // Semicolon
        {
            // Get the token
            tokens::Token t = get();

            // Check the tag
            if (t.tag != tokens::tags::semicolon) {
                // Unexpected token
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Assignment Statement and expecting SEMICOLON.";
                throw ParserException(message.str());
            }
        }

        return std::make_unique<ast::statements::Assignment>(id, std::move(val));
    }

    /**
     * \brief Parse Statement node from the input
     *
     * \return Pointer to resulting Statement node
     */
    std::unique_ptr<ast::Statement> StatementParser::statement() {
        // Statement -> expecting RETURN Expression SEMICOLON, Assignment Statement, or Expression SEMICOLON

        // Check first token
        tokens::Token t = peek(0);
        if (t.tag == tokens::tags::kw_return) {
            // RETURN -> Return Statement
            return return_kw();
        } else if (t.tag == tokens::tags::identifier) {
            // IDENTIFIER -> Assignment Statement or Discard Statement

            // Check second token
            if (peek(1).tag == tokens::tags::assign) {
                // ASSIGN -> Assignment Statement (Expression cannot contain ASSIGN)
                return assignment();
            } else {
                // Otherwise -> Discard Statement (Assignment Statement requires ASSIGN)
                return discard();
            }
        } else {
            // Otherwise -> Discard Statement
            return discard();
        }
    }
    //--- End StatementParser implementation

    //--- Start DefinitionParser implementation
    /**
     * \brief Parse Variable Definition node from the input
     *
     * \return Pointer to resulting Variable Definition node
     */
    std::unique_ptr<ast::definitions::Variable> DefinitionParser::variable() {
        // Variable Definition -> expecting Assignment Statement

        // Assignment statement
        auto stmt = StatementParser(get, peek).assignment();

        return std::make_unique<ast::definitions::Variable>(std::move(stmt));
    }

    /**
     * \brief Parse Function Definition node from the input
     *
     * \return Pointer to resulting Function Definition node
     */
    std::unique_ptr<ast::definitions::Function> DefinitionParser::function() {
        // Function definition -> expecting IDENTIFIER, LPAR, optional identifier list, RPAR, LBRAC, statement block and RBRAC

        // Identifier
        std::string id;
        {
            // Get the token
            tokens::Token t = get();

            // Check the tag
            if (t.tag != tokens::tags::identifier) {
                // Unexpected token
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
                    std::ostringstream message;
                    message << "Unexpected token " << t << " when parsing Function Definition and expecting IDENTIFIER.";
                    throw ParserException(message.str());
                }

                // Add the identifier content, consuming the token
                args.push_back(get().content);

                // Check next is COMMA or RPAR
                t = peek(0);
                if (t.tag == tokens::tags::comma) {
                    // COMMA -> consume and repeek
                    get();
                    t = peek(0);
                } else if (t.tag != tokens::tags::rpar) {
                    // Not RPAR -> unexpected token
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
                auto stmt = StatementParser(get, peek).statement();

                // Add to body
                body.push_back(std::move(stmt));
            }
        }

        // Right bracket
        {
            // Get the token
            tokens::Token t = get();

            // Check the tag
            if (t.tag != tokens::tags::rbrac) {
                // Unexpected token
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Function Definition and expecting RBRAC.";
                throw ParserException(message.str());
            }
        }

        return std::make_unique<ast::definitions::Function>(id, args, std::move(body));
    }

    /**
     * \brief Parse Definition node from the input
     *
     * \return Pointer to resulting Definition node
     */
    std::unique_ptr<ast::Definition> DefinitionParser::definition() {
        // Definition -> expecting IDENTIFIER followed by LPAR for function definition, ASSIGN for variable definition

        // Decide by second token
        tokens::Token t = peek(1);
        if (t.tag == tokens::tags::lpar) {
            // Function definition
            return function();
        } else if (t.tag == tokens::tags::assign) {
            // Variable definition
            return variable();
        } else {
            // Unexpected token
            std::ostringstream message;
            message << "Unexpected token " << t << " when parsing Definition and expecting LPAR or ASSIGN.";
            throw ParserException(message.str());
        }
    }
    //--- End DefinitionParser implementation

    //--- Start ProgramParser implementation
    /**
     * \brief Parse Program node from the input
     *
     * \return Resulting Program node
     */
    ast::Program ProgramParser::program() {
        // Program -> expecting set of variable and function definitions
        std::vector<std::unique_ptr<ast::Definition>> definitions;

        // Try to gather definitions until END
        for (tokens::Token t = peek(0); t.tag != tokens::tags::end_of_input; t = peek(0)) {
            // All definitions start with an identifier
            if (t.tag == tokens::tags::identifier) {
                // Consume definition
                definitions.push_back(DefinitionParser(get, peek).definition());
            } else if (t.tag == tokens::tags::error) {
                // Lexer error
                std::ostringstream message;
                message << "Lexer error: " << t.content;
                throw ParserException(message.str());
            } else {
                // Unexpected token
                std::ostringstream message;
                message << "Unexpected token " << t << " when parsing Program and expecting IDENTIFIER as start of Definition.";
                throw ParserException(message.str());
            }
        }

        // Consume END
        // Note: top token is now END by termination condition of the loop
        get();

        // Return the program with the gathered definitions
        return ast::Program(std::move(definitions));
    }
    //--- End ProgramParser implementation
}
