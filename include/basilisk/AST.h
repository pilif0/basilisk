/** \file AST.h
 * Abstract Syntax Tree definitions
 *
 * \author Filip Smola
 */
#ifndef BASILISK_AST_H
#define BASILISK_AST_H

#include <vector>

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

    /** \class Definition
     * \brief Base class for both function and variable definitions
     */
    class Definition {
        //TODO implement
    };

    /** \class Program
     * \brief Program node
     *
     * Program node contains a set of one or more definitions.
     * Root node of the AST.
     */
    class Program {
        public:
            //! Definitions in this program
            //TODO in order of definition?
            const std::vector<Definition> definitions;

            //TODO make variadic
            explicit Program(std::vector<Definition> defs) : definitions(std::move(defs)) {}
    };

    /**
     * @}
     */
}

#endif //BASILISK_AST_H
