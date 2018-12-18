/** \file Parser.cpp
* Parser implementation
*
* \author Filip Smola
*/

#include <basilisk/Parser.h>
#include <basilisk/Tokens.h>
#include <basilisk/AST.h>

#include <vector>
#include <sstream>
#include <memory>
#include <string>

//TODO improve function docs
//TODO improve reporting of unexpected tokens (might require a substantial redesign to have sufficient information)
//TODO expose more functions in interface to enable partial parsing and testing?
namespace basilisk::parser {

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
        //TODO
        std::vector<ast::Identifier> args;

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
        //TODO
        std::vector<ast::Statement> body;

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

        return std::make_unique<ast::FunctionDefinition>(id, args, body);
    }

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
        //TODO
        ast::Expression val;

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

        return std::make_unique<ast::VariableDefinition>(id, val);
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
        tokens::Token t = peek();
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
        for (tokens::Token t = peek(); t.tag != tokens::tags::end_of_input; t = peek()) {
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
        return ast::Program(definitions);
    }
}
