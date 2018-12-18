/** \file AST.h
 * Abstract Syntax Tree definitions
 *
 * \author Filip Smola
 */
#ifndef BASILISK_AST_H
#define BASILISK_AST_H

#include <vector>
#include <string>
#include <memory>

//TODO review usage of std::move once stable
//TODO add docs to constructors
//TODO should invariants (e.g. at least one definition in program) be checked here or in parser?
//! Abstract Syntax Tree definitions
namespace basilisk::ast {
    /** \addtogroup AST
     * \brief Abstract Syntax Tree definitions
     *
     * Definitions of nodes of the Abstract Syntax Tree.
     *
     * @{
     */

    //TODO: extract a base class

    //! Identifiers are strings
    typedef std::string Identifier;

    class Expression {
            //TODO implement
    };

    class Statement {
            //TODO implement
    };

    /** \class Definition
     * \brief Base class for both function and variable definitions
     */
    class Definition {};

    /** \class FunctionDefinition
     * \brief Function Definition node
     *
     * Function Definition node contains the function identifier, a set of arguments and a statement-block body.
     */
    class FunctionDefinition : public Definition {
        public:
            //! Function identifier
            const Identifier identifier;
            //! Arguments of the function in order of declaration
            const std::vector<Identifier> arguments;
            //! Non-empty block of statements acting as the function body
            const std::vector<Statement> body;

            FunctionDefinition(Identifier id, std::vector<Identifier> args, std::vector<Statement> body)
                : identifier(std::move(id)), arguments(std::move(args)), body(std::move(body)) {}
    };

    /** \class VariableDefinition
     * \brief Variable Definition node
     *
     * Variable Definition contains the variable identifier and the value expression.
     */
    class VariableDefinition : public Definition {
        public:
            //! Variable identifier
            const Identifier identifier;
            //! Value expression
            const Expression value;

            VariableDefinition(Identifier id, Expression val) : identifier(std::move(id)), value(val) {}
    };

    /** \class Program
     * \brief Program node
     *
     * Program node contains a set of one or more definitions.
     * Root node of the AST.
     */
    class Program {
        public:
            //! Pointers to definitions in this program in order of definition
            const std::vector<std::unique_ptr<Definition>> definitions;

            //TODO make variadic?
            explicit Program(std::vector<std::unique_ptr<Definition>> defs) : definitions(std::move(defs)) {}
    };

    /**
     * @}
     */
}

#endif //BASILISK_AST_H
