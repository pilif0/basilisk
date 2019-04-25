/** \file Codegen.h
 * LLVM IR code generation
 *
 * \author Filip Smola
 */
#ifndef BASILISK_CODEGEN_H
#define BASILISK_CODEGEN_H

#include <basilisk/AST.h>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include <memory>
#include <map>
#include <vector>

//! LLVM IR code generation
namespace basilisk::codegen {
    /** \addtogroup Codegen
     * \brief LLVM IR code generation
     *
     * Generation of LLVM IR from the AST.
     *
     * @{
     */

    /** \class NamedValues
     * \brief Named values data structure capable of handling nested scopes
     *
     * Resolves identifiers to the respective named values from the top-most scope where they are found.
     */
    class NamedValues {
        protected:
            NamedValues() = default;
        public:
            /**
             * \brief Set the value of the identifier in the current scope
             *
             * \param identifier Identifier to set
             * \param value Value to set
             */
            virtual void put(ast::Identifier identifier, llvm::Value *value) = 0;
            /**
             * \brief Get the value of the identifier
             *
             * Get the value of the identifier from the top-most scope where it is found.
             *
             * \param identifier Identifier to seek
             * \return Pointer to the named value, or `nullptr` if not found
             */
            virtual llvm::Value* get(ast::Identifier identifier) = 0;

            //! Push a new scope on the stack
            virtual void push() = 0;
            //! Pop a scope off of the stack
            virtual void pop() = 0;

            virtual ~NamedValues() = default;
    };

    /** \class NamedValuesStacks
     * \brief Named values data structure backed by two stacks
     *
     * Named Values data structure backed by a stack of identifier - value pairs and a stack of scope value counts.
     */
     // Note: This implementation uses the fact that one always inserts named values into the top-most scope
    class NamedValuesStacks : public NamedValues {
        private:
            //! Type of the identifier - value pair
            typedef std::pair<ast::Identifier, llvm::Value *> pair_t;
            //! Stack of identifier - value pairs
            std::vector<pair_t> data;
            //! Stack of value counts in the active scopes
            std::vector<std::size_t> counts{0};
        public:
            void put(ast::Identifier identifier, llvm::Value *value) override;
            llvm::Value *get(ast::Identifier identifier) override;
            void push() override;
            void pop() override;
    };

    /** \class NamedValuesMap
     * \brief Named value map capable of handling nested scopes
     *
     * Wrapper around an `std::map`.
     * Resolves identifiers to the respective named values.
     * Handles nested scopes (including variable shadowing).
     */
     // Note: shadowing is by selecting the matching value from the top-most scope where it is present
     // Note: This implementation uses the trivial interpretation of named values as a stack of identifier - value maps
    class NamedValuesMap : public NamedValues {
        public:
            //! Type of the named value map
            typedef std::map<ast::Identifier, llvm::Value *> map_t;
            //! Vector of active scopes
            std::vector<map_t> scopes{{}};

            void put(ast::Identifier identifier, llvm::Value *value) override;
            llvm::Value* get(ast::Identifier identifier) override;

            void push() override;
            void pop() override;
    };

    /** \class ExpressionCodegen
     * \brief Expression-specific code generation AST visitor
     *
     * AST visitor that generates LLVM IR Value from expression nodes.
     */
    class ExpressionCodegen : public ast::Visitor {
        private:
            //! LLVM context
            llvm::LLVMContext &context;
            //! LLVM instruction generation helper
            llvm::IRBuilder<> &builder;
            //! LLVM module
            llvm::Module *module;
            //! Pointers to variables
            NamedValues &variables;

            //! Pointer to the last value built
            llvm::Value *value = nullptr;
        public:
            /**
             * \brief Construct an AST visitor to generate LLVM IR from expression nodes into the provided module
             *
             * \param context LLVM context
             * \param builder LLVM IR builder
             * \param module LLVM module
             * \param variables Variable scope
             */
            ExpressionCodegen(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
                    llvm::Module *module, NamedValues &variables)
                : context(context), builder(builder), module(module), variables(variables) {}

            void visit(ast::Expression &node) override;
            void visit(ast::expressions::Modulo &node) override;

            void visit(ast::expressions::Expression1 &node) override;
            void visit(ast::expressions::Summation &node) override;
            void visit(ast::expressions::Subtraction &node) override;

            void visit(ast::expressions::Expression2 &node) override;
            void visit(ast::expressions::Multiplication &node) override;
            void visit(ast::expressions::Division &node) override;

            void visit(ast::expressions::Expression3 &node) override;
            void visit(ast::expressions::NumericNegation &node) override;

            void visit(ast::expressions::Expression4 &node) override;
            void visit(ast::expressions::IdentifierExpression &node) override;
            void visit(ast::expressions::Parenthesised &node) override;
            void visit(ast::expressions::FunctionCall &node) override;
            void visit(ast::expressions::LiteralDouble &node) override;

            void visit(ast::Node &node) override;

            /**
             * \brief Get pointer to the last value built
             *
             * \return Pointer to the last value built
             */
            llvm::Value* get() { return value; }
    };

    /** \class FunctionCodegen
     * \brief Function-specific code generation AST visitor
     *
     * AST visitor that generates LLVM IR from function definition nodes.
     */
    class FunctionCodegen : public ast::Visitor {
        private:
            //! LLVM context
            llvm::LLVMContext &context;
            //! LLVM instruction generation helper
            llvm::IRBuilder<> &builder;
            //! LLVM module
            llvm::Module *module;
            //! Pointers to variables
            NamedValues &variables;

            //! Pointer to the function being currently built, or `nullptr` if none is being built
            llvm::Function *current = nullptr;

            //! Pointer to the last function built
            llvm::Function *function = nullptr;
        public:
            /**
             * \brief Construct an AST visitor to generate LLVM IR from function definition and statement nodes into the
             *  provided module
             *
             * \param context LLVM context
             * \param builder LLVM IR builder
             * \param module LLVM module
             * \param variables Variable scope
             */
            FunctionCodegen(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
                    llvm::Module *module, NamedValues &variables)
            : context(context), builder(builder), module(module), variables(variables) {}

            void visit(ast::Statement &node) override;
            void visit(ast::statements::Assignment &node) override;
            void visit(ast::statements::Discard &node) override;
            void visit(ast::statements::Return &node) override;

            void visit(ast::definitions::Function &node) override;

            void visit(ast::Node &node) override;

            /**
             * \brief Get pointer to the last function built
             *
             * \return Pointer to the last function built
             */
            llvm::Function* get() { return function; }
    };

    /** \class ProgramCodegen
     * \brief Program-specific code generation AST visitor
     *
     * AST visitor that generates LLVM IR from program nodes.
     */
    class ProgramCodegen : public ast::Visitor {
        private:
            //! LLVM context
            llvm::LLVMContext &context;
            //! LLVM instruction generation helper
            llvm::IRBuilder<> &builder;
            //! LLVM module
            llvm::Module *module;
            //! Pointers to variables
            NamedValues &variables;
        public:
            /**
             * \brief Construct an AST visitor to generate LLVM IR from the program node into the provided module
             *
             * \param context LLVM context
             * \param builder LLVM IR builder
             * \param module LLVM module
             * \param variables Variable scope
             */
            ProgramCodegen(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
                    llvm::Module *module, NamedValues &variables)
            : context(context), builder(builder), module(module), variables(variables) {}

            void visit(ast::Definition &node) override;
            void visit(ast::definitions::Function &node) override;
            void visit(ast::definitions::Variable &node) override;

            void visit(ast::Program &node) override;

            void visit(ast::Node &node) override;
    };

    /** \class CodegenException
     * \brief Exception during code generation
     */
    class CodegenException : public std::runtime_error {
        public:
            //! Construct a codegen exception from its message
            explicit CodegenException(const std::string &message) : std::runtime_error(message) {}
    };

    /**
     * @}
     */
}

#endif //BASILISK_CODEGEN_H
