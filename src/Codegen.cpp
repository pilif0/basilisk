/** \file Codegen.cpp
* LLVM IR code generation implementation
*
* \author Filip Smola
*/

#include <basilisk/Codegen.h>

#include <llvm/IR/Verifier.h>

#include <sstream>
#include <vector>

namespace basilisk::codegen {

    //--- Start NamedValuesStacks implementation
    void NamedValuesStacks::put(ast::Identifier identifier, llvm::Value *value) {
        // Try to find in current scope
        auto n = counts.back();
        auto iter = data.rbegin();
        for (std::size_t i = 0; i < n ; i++) {
            // Check if equal identifier
            if (iter->first == identifier) {
                // Overwrite
                iter->second = value;
                return;
            }

            // Increment iterator
            iter++;
        }

        // Otherwise push on top and increment count
        data.emplace_back(identifier, value);
        counts.back()++;
    }

    llvm::Value *NamedValuesStacks::get(ast::Identifier identifier) {
        // Try to find from back to front
        for (auto iter = data.rbegin(); iter != data.rend(); iter++) {
            // Check if equal identifier
            if (iter->first == identifier) {
                return iter->second;
            }
        }

        // Return null if not found
        return nullptr;
    }

    void NamedValuesStacks::push() {
        // Push a 0 onto the counts
        counts.emplace_back(0);
    }

    void NamedValuesStacks::pop() {
        // Don't pop global scope
        if (counts.size() <= 1) {
            return;
        }

        // Pop top of counts and that many pairs
        auto n = counts.back();
        counts.pop_back();
        for (std::size_t i = 0; i < n; i++) {
            data.pop_back();
        }
    }
    //--- End NamedValuesStacks implementation

    //--- Start NamedValuesMap implementation
    void NamedValuesMap::put(ast::Identifier identifier, llvm::Value *value) {
        // Put the pair into the top map
        scopes.back()[identifier] = value;
    }

    llvm::Value* NamedValuesMap::get(ast::Identifier identifier) {
        // Check scopes in reverse order
        for (auto iter = scopes.rbegin(); iter != scopes.rend(); iter++) {
            // Check identifier present
            auto scope = *iter;
            try {
                return scope.at(identifier);
            } catch (std::out_of_range &) {
                // Ignore exception and continue to parent scope
                continue;
            }
        }

        // Not found in any scope --> nullptr
        return nullptr;
    }

    void NamedValuesMap::push() {
        scopes.emplace_back();
    }

    void NamedValuesMap::pop() {
        // Don't pop global scope
        if (scopes.size() <= 1) {
            return;
        }

        scopes.pop_back();
    }
    //--- End NamedValuesMap implementation

    //--- Start ExpressionCodegen implementation
    /**
     * \brief Throw an exception on visiting an unsupported Expression node
     *
     * \param node Expression node
     */
    void ExpressionCodegen::visit(ast::Expression &/*node*/) {
        throw CodegenException("Expression code generator encountered an unsupported Expression node.");
    }

    /**
     * \brief Generate remainder after division of the contained expressions
     *
     * \param node Modulo node
     */
     //TODO remainder has the same sign as dividend, should probably mention that somewhere in the docs
    void ExpressionCodegen::visit(ast::expressions::Modulo &node) {
         // Generate the values of the contained expressions
         node.x->accept(*this);
         llvm::Value *x = value;
         node.m->accept(*this);
         llvm::Value *m = value;

         // Set value as division of those
         value = builder.CreateFRem(x, m, "subtraction_tmp");
    }

    /**
     * \brief Throw an exception on visiting an unsupported Expression1 node
     *
     * \param node Expression1 node
     */
    void ExpressionCodegen::visit(ast::expressions::Expression1 &/*node*/) {
        throw CodegenException("Expression code generator encountered an unsupported Expression1 node.");
    }

    /**
     * \brief Generate sum of the contained expressions
     *
     * \param node Summation node
     */
    void ExpressionCodegen::visit(ast::expressions::Summation &node) {
        // Generate the values of the contained expressions
        node.lhs->accept(*this);
        llvm::Value *l = value;
        node.rhs->accept(*this);
        llvm::Value *r = value;

        // Set value as division of those
        value = builder.CreateFAdd(l, r, "sum_tmp");
    }

    /**
     * \brief Generate difference of the contained expressions
     *
     * \param node Subtraction node
     */
    void ExpressionCodegen::visit(ast::expressions::Subtraction &node) {
        // Generate the values of the contained expressions
        node.lhs->accept(*this);
        llvm::Value *l = value;
        node.rhs->accept(*this);
        llvm::Value *r = value;

        // Set value as division of those
        value = builder.CreateFSub(l, r, "subtraction_tmp");
    }

    /**
     * \brief Throw an exception on visiting an unsupported Expression2 node
     *
     * \param node Expression2 node
     */
    void ExpressionCodegen::visit(ast::expressions::Expression2 &/*node*/) {
        throw CodegenException("Expression code generator encountered an unsupported Expression2 node.");
    }

    /**
     * \brief Generate multiplication of the contained expressions
     *
     * \param node Multiplication node
     */
    void ExpressionCodegen::visit(ast::expressions::Multiplication &node) {
        // Generate the values of the contained expressions
        node.lhs->accept(*this);
        llvm::Value *l = value;
        node.rhs->accept(*this);
        llvm::Value *r = value;

        // Set value as multiplication of those
        value = builder.CreateFMul(l, r, "multiplication_tmp");
    }

    /**
     * \brief Generate division of the contained expressions
     *
     * \param node Division node
     */
    void ExpressionCodegen::visit(ast::expressions::Division &node) {
        // Generate the values of the contained expressions
        node.lhs->accept(*this);
        llvm::Value *l = value;
        node.rhs->accept(*this);
        llvm::Value *r = value;

        // Set value as division of those
        value = builder.CreateFDiv(l, r, "division_tmp");
    }

    /**
     * \brief Throw an exception on visiting an unsupported Expression3 node
     *
     * \param node Expression3 node
     */
    void ExpressionCodegen::visit(ast::expressions::Expression3 &/*node*/) {
        throw CodegenException("Expression code generator encountered an unsupported Expression3 node.");
    }

    /**
     * \brief Generate negation of the contained value
     *
     * \param node Numeric negation node
     */
    void ExpressionCodegen::visit(ast::expressions::NumericNegation &node) {
        // Generate the value of the contained expression
        node.x->accept(*this);

        // Set value as negation of the generated one
        value = builder.CreateFNeg(value, "numeric_negation_tmp");
    }

    /**
     * \brief Throw an exception on visiting an unsupported Expression4 node
     *
     * \param node Expression4 node
     */
    void ExpressionCodegen::visit(ast::expressions::Expression4 &/*node*/) {
        throw CodegenException("Expression code generator encountered an unsupported Expression4 node.");
    }

    /**
     * \brief Look up value represented by the identifier
     *
     * \param node Identifier expression node
     */
    void ExpressionCodegen::visit(ast::expressions::IdentifierExpression &node) {
        // Look up value
        llvm::Value *v = named_values.get(node.identifier);

        // Check value was found
        if (!v) {
            std::ostringstream message;
            message << "Could not find value for identifier \"" << node.identifier << "\".";
            throw CodegenException(message.str());
        }

        // Set value as the found one
        value = v;
    }

    /**
     * \brief Generate value of the contained parenthesised expression
     *
     * \param node Parenthesised expression node
     */
    void ExpressionCodegen::visit(ast::expressions::Parenthesised &node) {
        // Generate the value of the contained expression
        node.expression->accept(*this);

        // Note: value gets automatically set to the contained expression's value
    }

    /**
     * \brief Generate value that is the result of a function call
     *
     * \param node Function call node
     */
    void ExpressionCodegen::visit(ast::expressions::FunctionCall &node) {
        // Look up the function name
        llvm::Function *f = module->getFunction(node.identifier);

        // Check function was found
        if (!f) {
            std::ostringstream message;
            message << "Could not find function for identifier \"" << node.identifier << "\".";
            throw CodegenException(message.str());
        }

        // Check argument sizes match
        if (f->arg_size() != node.arguments.size()) {
            std::ostringstream message;
            message << "Function \"" << node.identifier << "\" given " << node.arguments.size() << " arguments, takes "
                << f->arg_size() << ".";
            throw CodegenException(message.str());
        }

        // Generate values of arguments
        std::vector<llvm::Value *> args;
        for (auto &expr : node.arguments) {
            expr->accept(*this);
            args.push_back(value);
        }

        // Set value as the function call
        value = builder.CreateCall(f, args, "call_tmp");
    }

    /**
     * \brief Generate numeric constant with the value of the double literal
     *
     * \param node Double literal node
     */
    void ExpressionCodegen::visit(ast::expressions::LiteralDouble &node) {
        value = llvm::ConstantFP::get(context, llvm::APFloat(node.value));
    }

    /**
     * \brief Throw an exception on visiting an unsupported node
     *
     * \param node Visited node
     */
    void ExpressionCodegen::visit(ast::Node &/*node*/) {
        throw CodegenException("Expression code generator encountered an unsupported node.");
    }
    //--- End ExpressionCodegen implementation

    //--- Start FunctionCodegen implementation
    /**
     * \brief Throw an exception on visiting an unsupported statement node
     *
     * \param node Visited node
     */
    void FunctionCodegen::visit(ast::Statement &/*node*/) {
        throw CodegenException("Function code generator encountered an unsupported statement node.");
    }

    /**
     * \brief Generate value and set a named value to it
     *
     * \param node Assignment statement node
     */
    void FunctionCodegen::visit(ast::statements::Assignment &node) {
        // Generate value
        ExpressionCodegen expr_cg(context, builder, module, named_values);
        node.value->accept(expr_cg);
        llvm::Value *val = expr_cg.get();

        // Set named value
        named_values.put(node.identifier, val);
    }

    /**
     * \brief Generate instructions for an expression and discard the value
     *
     * \param node Discard statement node
     */
    void FunctionCodegen::visit(ast::statements::Discard &node) {
        // Generate value
        ExpressionCodegen expr_cg(context, builder, module, named_values);
        node.expression->accept(expr_cg);
        expr_cg.get();
    }

    /**
     * \brief Generate return instruction
     *
     * \param node Return statement node
     */
    void FunctionCodegen::visit(ast::statements::Return &node) {
        // Generate value
        ExpressionCodegen expr_cg(context, builder, module, named_values);
        node.expression->accept(expr_cg);
        llvm::Value *val = expr_cg.get();

        // Generate instruction
        builder.CreateRet(val);
    }

    /**
     * \brief Generate function definition
     *
     * \param node Function definition node
     */
    void FunctionCodegen::visit(ast::definitions::Function &node) {
        // Change main() function name to main_ to support wrapper
        //TODO remove this when sufficient data types are added to the language to support returning i32
        if (node.identifier == "main" && node.arguments.empty()) {
            node.identifier = "main_";
        }

        // Check if already present
        if (auto f = module->getFunction(node.identifier)) {
            // Check for same number of arguments
            if (f->arg_size() == node.arguments.size()) {
                std::ostringstream message;
                message << "Function \"" << node.identifier << "\" with " << node.arguments.size()
                        << " arguments is already defined.";
                throw CodegenException(message.str());
            }
        }

        // Prepare the function pointer
        std::vector<llvm::Type *> arg_types(node.arguments.size(), llvm::Type::getDoubleTy(context));
        llvm::FunctionType *func_type = llvm::FunctionType::get(llvm::Type::getDoubleTy(context), arg_types, false);
        llvm::Function *f = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, node.identifier, module);

        // Set argument names
        unsigned i = 0;
        for (auto &arg : f->args()) {
            arg.setName(node.arguments[i]);
            i++;
        }

        // Push a new scope and add arguments
        named_values.push();
        for (auto &arg : f->args()) {
            named_values.put(arg.getName(), &arg);
        }

        // Add body
        llvm::BasicBlock *body = llvm::BasicBlock::Create(context, "entry", f);

        // Start inserting into the body
        builder.SetInsertPoint(body);

        // Emit body
        for (auto &stmt : node.body) {
            stmt->accept(*this);
        }

        // Return 0 if last statement wasn't return
        auto last_stmt = dynamic_cast<ast::statements::Return *>(node.body.back().get());
        if (!last_stmt) {
            // Last statement is not a return statement
            llvm::Value *ret_val = llvm::ConstantFP::get(context, llvm::APFloat(0.0));
            builder.CreateRet(ret_val);
        }

        // Stop inserting into the body and pop scope
        builder.ClearInsertionPoint();
        named_values.pop();

        // Validate generated code
        llvm::verifyFunction(*f);

        // Set the function to the generated one
        function = f;
    }

    /**
     * \brief Throw an exception on visiting an unsupported node
     *
     * \param node Visited node
     */
    void FunctionCodegen::visit(ast::Node &/*node*/) {
        throw CodegenException("Function code generator encountered an unsupported node.");
    }
    //--- End FunctionCodegen implementation

    //--- Start ProgramCodegen implementation
    /**
     * \brief Throw an exception on visiting an unsupported definition node
     *
     * \param node Visited definition node
     */
    void ProgramCodegen::visit(ast::Definition &/*node*/) {
        throw CodegenException("Expression code generator encountered an unsupported node.");
    }

    /**
     * \brief Generate code for function definition
     *
     * \param node Function definition node
     */
    void ProgramCodegen::visit(ast::definitions::Function &node) {
        // Create function codegen and have it visit the function definition
        FunctionCodegen func_cg(context, builder, module, named_values);
        node.accept(func_cg);
    }

    /**
     * \brief Generate value and add it to named values
     *
     * \param node Variable definition node
     */
    void ProgramCodegen::visit(ast::definitions::Variable &node) {
        // Create function codegen and have it visit the assignment statement
        FunctionCodegen func_cg(context, builder, module, named_values);
        node.statement->accept(func_cg);
    }

    /**
     * \brief Generate LLVM IR definitions for the STL function `println` and the supporting external function `printf`
     *
     * \param context LLVM context
     * \param module LLVM module
     * \param builder IR builder
     */
    void generate_stl(llvm::LLVMContext &context, llvm::Module *module, llvm::IRBuilder<> &builder) {
        // Add external printf
        llvm::Function *printf;
        {
            // i32 printf(i8*, ...)
            std::vector<llvm::Type *> arg_types{llvm::Type::getInt8PtrTy(context)};
            auto *func_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), arg_types, true);
            printf = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "printf", module);
        }

        // Add internal println
        llvm::Function *println;
        {
            // Note: doesn't add named values, so not pushing or popping scope

            // double println(double)
            std::vector<llvm::Type *> arg_types{llvm::Type::getDoubleTy(context)};
            llvm::FunctionType *func_type = llvm::FunctionType::get(llvm::Type::getDoubleTy(context), arg_types, false);
            println = llvm::Function::Create(func_type, llvm::Function::PrivateLinkage, "println", module);

            // Add body
            llvm::BasicBlock *body = llvm::BasicBlock::Create(context, "entry", println);

            // Start inserting into the body
            builder.SetInsertPoint(body);

            // Check printf was found
            if (!printf) {
                std::ostringstream message;
                message << "Could not find STL function \'printf\'.";
                throw CodegenException(message.str());
            }

            // Generate pointer to format string
            auto format_ptr = builder.CreateGlobalStringPtr("%f\n", "println_format");

            // Put arguments into vector
            std::vector<llvm::Value *> args;
            args.push_back(format_ptr);
            args.push_back(println->arg_begin());

            // Emit function call
            builder.CreateCall(printf, args, "printf_tmp");

            // Return 0
            llvm::Value *ret_val = llvm::ConstantFP::get(context, llvm::APFloat(0.0));
            builder.CreateRet(ret_val);

            // Stop inserting into the body and pop scope
            builder.ClearInsertionPoint();

            // Validate generated code
            llvm::verifyFunction(*println);
        }
    }

    /**
     * \brief Generate code for definitions in a program
     *
     * \param node Program node
     */
    void ProgramCodegen::visit(ast::Program &node) {
        // Add standard library definitions
        generate_stl(context, module, builder);

        // Visit definitions in the program in order
        for (auto &def : node.definitions) {
            def->accept(*this);
        }

        // Insert main wrapper
        //TODO remove when no longer necessary
        if (auto main = module->getFunction("main_")) {
            // Define new main that calls the renamed one

            // double println(double)
            std::vector<llvm::Type *> arg_types;
            llvm::FunctionType *func_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), arg_types, false);
            auto main_wrap = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "main", module);

            // Add body
            llvm::BasicBlock *body = llvm::BasicBlock::Create(context, "entry", main_wrap);

            // Start inserting into the body
            builder.SetInsertPoint(body);

            // Emit main function call
            std::vector<llvm::Value *> args;
            auto main_ret = builder.CreateCall(main, args, "main_tmp");

            // Return i32 cast of main return
            auto ret_val = builder.CreateCast(llvm::Instruction::CastOps::FPToSI, main_ret, llvm::Type::getInt32Ty(context), "ret_val");
            builder.CreateRet(ret_val);

            // Stop inserting into the body and pop scope
            builder.ClearInsertionPoint();

            // Validate generated code
            llvm::verifyFunction(*main_wrap);
        }
    }

    /**
     * \brief Throw an exception on visiting an unsupported node
     *
     * \param node Visited node
     */
    void ProgramCodegen::visit(ast::Node &/*node*/) {
        throw CodegenException("Program code generator encountered an unsupported node.");
    }
    //--- End ProgramCodegen implementation
}
