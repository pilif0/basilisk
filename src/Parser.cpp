/** \file Parser.cpp
* Parser implementation
*
* \author Filip Smola
*/

#include <basilisk/Parser.h>
#include <basilisk/Tokens.h>
#include <basilisk/AST.h>

#include <vector>

namespace basilisk::parser {

    /**
     * \brief Parse from an input token buffer into an AST
     *
     * User `get` function to obtain tokens from an input buffer, parse those into an AST and return that tree.
     *
     * \param get Function to get the next input token
     * \param peek Function to peek at the next input token
     * \return Resulting AST
     */
     //TODO is peek required?
    ast::Program parse(const get_function_t &get, const peek_function_t &peek) {
        // Root -> in Program node -> expecting set of variable and function definitions
        std::vector<ast::Definition> definitions{};

        //TODO gather definitions until error or END

        // There has to be at least one definition
        if (definitions.empty()) {
            throw ParserException("Program has to have at least one definition.");
        }

        // Return the program with the gathered definitions
        return ast::Program(definitions);
    }
}
