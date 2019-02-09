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

    //--- Start Helper functions
    /**
     * \brief Add a local variable alloca to the function entry block
     *
     * \param f Pointer to function to whose entry block to add alloca
     * \param identifier Identifier of the pointer temporary
     * \return Pointer to the alloca instruction
     */
    llvm::AllocaInst *create_entry_block_alloca(llvm::LLVMContext &context, llvm::Function *f, ast::Identifier identifier) {
        llvm::IRBuilder<> temp_builder(&f->getEntryBlock(), f->getEntryBlock().begin());
        return temp_builder.CreateAlloca(llvm::Type::getDoubleTy(context), 0, identifier + "_ptr");
    }
    //--- End Helper functions

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
     * \brief Load value of variable represented by the identifier
     *
     * \param node Identifier expression node
     */
    void ExpressionCodegen::visit(ast::expressions::IdentifierExpression &node) {
        // Look up pointer
        llvm::Value *ptr = variables.get(node.identifier);

        // Check pointer was found
        if (!ptr) {
            std::ostringstream message;
            message << "Could not find pointer for identifier \"" << node.identifier << "\".";
            throw CodegenException(message.str());
        }

        // Load the value
        llvm::Value *v = builder.CreateLoad(ptr, node.identifier + "_value");

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
        // Try to get variable pointer
        llvm::Value *ptr = variables.get(node.identifier);

        // Check whether a function is being built
        if (current) {
            // Function being built -> local variable

            // Allocate if not found
            if (!ptr) {
                // Add alloca to function entry block
                ptr = create_entry_block_alloca(context, current, node.identifier);

                // Add to named values
                variables.put(node.identifier, ptr);
            }

            // Generate value
            ExpressionCodegen expr_cg(context, builder, module, variables);
            node.value->accept(expr_cg);
            llvm::Value *val = expr_cg.get();

            // Store value
            builder.CreateStore(val, ptr);
        } else {
            // TODO this permits multiple initializers of a single global variable (in proper order) which is weird,
            //          but not necessarily wrong (the variable retains the latest initializer for full execution)

            // Otherwise -> global variable

            // Allocate if not found
            if (!ptr) {
                // Create global variable initialized as 0.0
                module->getOrInsertGlobal(node.identifier, llvm::Type::getDoubleTy(context));
                auto var = module->getGlobalVariable(node.identifier);
                var->setInitializer(llvm::ConstantFP::get(llvm::Type::getDoubleTy(context), 0.0));
                ptr = var;

                // Add to named values
                variables.put(node.identifier, ptr);
            }

            // Add intializer to global variable intializer function
            {
                // Get initializer function pointer
                llvm::Function *init_f = module->getFunction("global_var_init");

                // Check initializer function found
                if (!init_f) {
                    std::ostringstream message;
                    message << "Could not find global variable initializer function.";
                    throw CodegenException(message.str());
                }

                // Move builder to insert into entry
                builder.SetInsertPoint(&init_f->getEntryBlock());

                // Generate value
                ExpressionCodegen expr_cg(context, builder, module, variables);
                node.value->accept(expr_cg);
                llvm::Value *val = expr_cg.get();

                // Store value
                builder.CreateStore(val, ptr);

                // Clear builder insert
                builder.ClearInsertionPoint();
            }
        }
    }

    /**
     * \brief Generate instructions for an expression and discard the value
     *
     * \param node Discard statement node
     */
    void FunctionCodegen::visit(ast::statements::Discard &node) {
        // Generate value
        ExpressionCodegen expr_cg(context, builder, module, variables);
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
        ExpressionCodegen expr_cg(context, builder, module, variables);
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
        current = f;

        // Add body and start inserting to it
        llvm::BasicBlock *body = llvm::BasicBlock::Create(context, "entry", f);
        builder.SetInsertPoint(body);

        // Push a new scope and add argument pointers
        variables.push();
        unsigned i = 0;
        for (auto &arg : f->args()) {
            // Get argument identifier
            auto identifier = node.arguments[i];

            // Name the argument
            arg.setName(identifier);

            // Add alloca to function entry block
            llvm::Value *ptr = create_entry_block_alloca(context, current, identifier);

            // Store the value
            builder.CreateStore(&arg, ptr);

            // Add to named values
            variables.put(identifier, ptr);

            // Increment name index
            i++;
        }

        // Emit body
        for (auto &stmt : node.body) {
            stmt->accept(*this);
        }

        // Return 0 if last statement wasn't return
        if (node.body.empty() || !dynamic_cast<ast::statements::Return *>(node.body.back().get())) {
            // Last statement is not a return statement
            llvm::Value *ret_val = llvm::ConstantFP::get(context, llvm::APFloat(0.0));
            builder.CreateRet(ret_val);
        }

        // Stop inserting into the body and pop scope
        builder.ClearInsertionPoint();
        variables.pop();

        // Validate generated code
        llvm::verifyFunction(*f);

        // Set the function to the generated one
        function = f;
        current = nullptr;
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
        FunctionCodegen func_cg(context, builder, module, variables);
        node.accept(func_cg);
    }

    /**
     * \brief Generate value and add it to named values
     *
     * \param node Variable definition node
     */
    void ProgramCodegen::visit(ast::definitions::Variable &node) {
        // Create function codegen and have it visit the assignment statement
        FunctionCodegen func_cg(context, builder, module, variables);
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

        // Add global variable initializer function
        llvm::Function *init_f;
        {
            // void global_var_init()
            std::vector<llvm::Type *> arg_types;
            llvm::FunctionType *func_type = llvm::FunctionType::get(llvm::Type::getVoidTy(context), arg_types, false);
            init_f = llvm::Function::Create(func_type, llvm::Function::InternalLinkage, "global_var_init", module);

            // Add entry and return block
            llvm::BasicBlock::Create(context, "entry", init_f);

            // Validate generated code
            llvm::verifyFunction(*init_f);

            // Set as global constructor
            {
                // Prepare type
                auto struct_type = llvm::StructType::create(
                        "global_ctors_element",
                        llvm::Type::getInt32Ty(context),
                        llvm::PointerType::get(llvm::FunctionType::get(llvm::Type::getVoidTy(context), false), 0),
                        llvm::Type::getInt8PtrTy(context));
                auto type = llvm::ArrayType::get(struct_type, 1);

                // Create global
                module->getOrInsertGlobal("llvm.global_ctors", type);
                auto ctors_ptr = module->getGlobalVariable("llvm.global_ctors");
                ctors_ptr->setLinkage(llvm::GlobalVariable::AppendingLinkage);

                // Create and set initializer
                std::vector<llvm::Constant *> elements{
                        llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 65535),
                        init_f,
                        llvm::ConstantPointerNull::get(llvm::Type::getInt8PtrTy(context))};
                auto strct = llvm::ConstantStruct::get(struct_type, elements);
                auto ctors_val = llvm::ConstantArray::get(type, strct);
                ctors_ptr->setInitializer(ctors_val);
            }
        }

        // Visit definitions in the program in order
        for (auto &def : node.definitions) {
            def->accept(*this);
        }

        // Add return to global variable initializer function
        {
            builder.SetInsertPoint(&init_f->getEntryBlock());
            builder.CreateRetVoid();
            builder.ClearInsertionPoint();
        }

        // Insert main wrapper
        //TODO remove when no longer necessary
        if (auto main = module->getFunction("main_")) {
            // Define new main that calls the renamed one

            // i32 main()
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
