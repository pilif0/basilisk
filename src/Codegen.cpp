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

    //--- Start NamedValues implementation
    /**
     * \brief Set the value of the identifier in the current scope
     *
     * \param identifier Identifier to set
     * \param value Value to set
     */
    void NamedValues::put(ast::Identifier identifier, llvm::Value *value) {
        // Put the pair into the top map
        scopes.back()[identifier] = value;
    }

    /**
     * \brief Get the value of the identifier
     *
     * Get the value of the identifier.
     * If the identifier is not set in current scope, parent scopes are checked up to the global scope.
     *
     * \param identifier Identifier to seek
     * \return Pointer to the named value, or `nullptr` if not found
     */
    llvm::Value* NamedValues::get(ast::Identifier identifier) {
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

    /**
     * \brief Push a new scope onto the stack
     */
    void NamedValues::push() {
        scopes.emplace_back();
    }

    /**
     * \brief Pop a scope from the stack
     *
     * Pop a scope from the stack, removing any named value definitions in that scope.
     */
    void NamedValues::pop() {
        scopes.pop_back();
    }
    //--- End NamedValues implementation

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

        //TODO this should be enough to generate the instructions
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
        // Prepare the function pointer
        std::vector<llvm::Type *> arg_types(node.arguments.size(), llvm::Type::getDoubleTy(context));
        llvm::FunctionType *func_type = llvm::FunctionType::get(llvm::Type::getDoubleTy(context), arg_types, false);
        llvm::Function *f = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, node.identifier, module.get());

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
        f->getBasicBlockList().push_back(body);

        // Start inserting into the body
        builder.SetInsertPoint(body);

        // Emit body
        //TODO will using the same builder here as for the expressions emit extra value instructions? shouldn't, but check
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
}
