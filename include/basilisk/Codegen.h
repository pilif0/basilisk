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

//! LLVM IR code generation
namespace basilisk::codegen {
    /** \addtogroup Codegen
     * \brief LLVM IR code generation
     *
     * Generation of LLVM IR from the AST.
     *
     * @{
     */

    /** \class ExpressionCodegen
     * \brief Expression-specific code generation AST visitor
     *
     * AST visitor that generates LLVM IR Value from expression nodes
     */
    class ExpressionCodegen : public ast::Visitor {
        private:
            //! LLVM context
            llvm::LLVMContext &context;
            //! LLVM instruction generation helper
            llvm::IRBuilder<> &builder;
            //! LLVM module
            std::unique_ptr<llvm::Module> &module;
            //! Map of current named values
            std::map<ast::Identifier, llvm::Value *> &named_values;

            //! Pointer to the last value built
            llvm::Value *value;
        public:
            //TODO doc
            ExpressionCodegen(llvm::LLVMContext &context, llvm::IRBuilder<> &builder,
                    std::unique_ptr<llvm::Module> &module, std::map<ast::Identifier, llvm::Value *> &named_values)
                : context(context), builder(builder), module(module), named_values(named_values) {}

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
    };

    /** \class CodegenException
     * \brief Exception during code generation
     */
    class CodegenException : public std::runtime_error {
        public:
            explicit CodegenException(const std::string &message) : std::runtime_error(message) {}
    };

    /**
     * @}
     */
}

#endif //BASILISK_CODEGEN_H
