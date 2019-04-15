/** \file CodegenTest.cpp
 * Codegen test module
 *
 * \author Filip Smola
 */
#define BOOST_TEST_MODULE "Codegen"

#include <basilisk/Parser.h>
#include <basilisk/Tokens.h>
#include <basilisk/AST.h>
#include <basilisk/AST_util.h>
#include <basilisk/Lexer.h>
#include <basilisk/Codegen.h>

#include <boost/test/unit_test.hpp>

#include <vector>
#include <memory>

using namespace basilisk;

/** \class Generator
 * \brief Generates LLVM IR from Basilisk source code into contained module
 */
class Generator {
    public:
        llvm::LLVMContext context;
        llvm::IRBuilder<> builder;
        llvm::Module module;
        basilisk::codegen::NamedValuesStacks variables;

        Generator() : builder(context), module("test_source", context) {}

        /**
         * \brief Generate LLVM IR from Basilisk source code
         *
         * \param src Basilisk source code
         */
        void from_source(const std::string &src) {
            // Lex
            std::vector<basilisk::tokens::Token> buffer;
            {
                // Prepare iterator
                auto iterator = src.begin();
                auto end = src.end();

                // Prepare functions
                basilisk::lexer::get_function_t lexer_get = [&iterator, &end]() {
                    // Return null character when no more input
                    if (iterator >= end) {
                        return '\0';
                    }

                    // Pop and return character
                    auto b = *iterator;
                    iterator++;
                    return b;
                };
                basilisk::lexer::peek_function_t lexer_peek = [&iterator, &end]() {
                    // Return null character when no more input
                    if (iterator >= end) {
                        return '\0';
                    }

                    // Return character
                    return *iterator;
                };
                basilisk::lexer::append_function_t lexer_append = [&buffer](tokens::Token t) { buffer.push_back(t); };

                // Lex
                try {
                    basilisk::lexer::lex(lexer_get, lexer_peek, lexer_append);
                } catch (std::exception &/*e*/) {}

                // Reverse order to move top of the queue to the back of the vector
                std::reverse(buffer.begin(), buffer.end());
            }

            // Parse
            basilisk::parser::get_f_t parser_get = [&buffer](){
                // Return error token if empty
                if (buffer.empty()) {
                    return tokens::Token{basilisk::tokens::tags::error, "No more input tokens."};
                }

                tokens::Token t = buffer.back();
                buffer.pop_back();
                return t;
            };
            basilisk::parser::peek_f_t parser_peek = [&buffer](int offset){
                // Return error token if not valid
                if (static_cast<unsigned int>(offset) >= buffer.size()) {
                    std::ostringstream message;
                    message << "No token " << offset << " elements from the front of the input queue.";
                    return tokens::Token{basilisk::tokens::tags::error, message.str()};
                }

                // Compute index
                auto index = static_cast<unsigned int>(buffer.size() - 1 - offset);

                return buffer[index];
            };
            auto program = basilisk::parser::ProgramParser(parser_get, parser_peek).program();

            // Codegen
            basilisk::codegen::ProgramCodegen program_cg(context, builder, &module, variables);
            program.accept(program_cg);
        }
};

BOOST_AUTO_TEST_SUITE(Codegen)

    BOOST_AUTO_TEST_CASE( return_append ) {
        // Generate code
        Generator generator;
        generator.from_source("f () {\n"
                              "    x = 1.0;\n"
                              "}");

        // Check that the last instruction of the function is return 0.0
        auto f = generator.module.getFunction("f");
        BOOST_TEST_CHECK(f, "Function f has to be present.");
        if (f) {
            // Check last instruction is return
            auto last = &*f->getEntryBlock().rbegin();
            BOOST_TEST_CHECK(llvm::isa<llvm::ReturnInst>(last), "Last instruction must be return.");

            // Check return type is double
            auto ret = llvm::dyn_cast<llvm::ReturnInst>(last);
            auto val = ret->getReturnValue();
            BOOST_TEST_CHECK(val->getType() == llvm::Type::getDoubleTy(generator.context),
                             "Function f must return double.");

            // Check return value is 0.0
            BOOST_TEST_CHECK(llvm::isa<llvm::ConstantFP>(val), "Function f return value must be a constant.");
            auto const_val = llvm::dyn_cast<llvm::ConstantFP>(val);
            auto zero = llvm::ConstantFP::get(generator.context, llvm::APFloat(0.0));
            BOOST_TEST_CHECK(const_val == zero, "Function f must return zero.");
        }
    }

    BOOST_AUTO_TEST_CASE( arguments_on_stack ) {
        // Generate code
        Generator generator;
        generator.from_source("f (a, b, c) {\n"
                              "    return 0.0;"
                              "}");

        // Check if there is exactly three allocas and each argument gets stored in one
        auto f = generator.module.getFunction("f");
        BOOST_TEST_CHECK(f, "Function f has to be present.");
        if (f) {
            // Get entry block
            auto &entry = f->getEntryBlock();

            // Check correct number of allocas
            unsigned count = 0;
            for (auto iter = entry.begin(); iter != entry.end(); iter++) {
                auto instr = &*iter;

                if (llvm::isa<llvm::AllocaInst>(instr)) {
                    count++;
                }
            }
            BOOST_TEST_CHECK(count == 3, "Entry block of f must contain exactly three alloca instructions.");

            // Check each argument gets stored in alloca'd space
            for (auto &v : f->args()) {
                bool stored = false;

                for (auto iter = entry.begin(); iter != entry.end(); iter++) {
                    auto instr = &*iter;

                    if (auto store_inst = llvm::dyn_cast<llvm::StoreInst>(instr)) {
                        if (store_inst->getOperand(0) == &v) {
                            stored = true;
                        }
                    }
                }

                BOOST_TEST_CHECK(stored, "Entry block of f has to contain a store for argument " << v.getName().str() << ".");
            }
        }
    }

    BOOST_AUTO_TEST_CASE( empty_body ) {
        // Generate code
        Generator generator;
        generator.from_source("f () {}");

        // Note: this test succeeds by successfully generating code and not crashing, but check presence to be sure
        auto f = generator.module.getFunction("f");
        BOOST_TEST_CHECK(f, "Function f must be present.");
    }

    BOOST_AUTO_TEST_CASE( local_vars_stack ) {
        // Generate code
        Generator generator;
        generator.from_source("f () {x = 1.0; y = x; }");

        // Find f
        auto f = generator.module.getFunction("f");
        BOOST_TEST_CHECK(f, "Function f must be present.");

        // Entry block of f has to start with 2 allocas
        //TODO could make more general by counting allocas instead of assuming they are first and second, but that would
        //  make check of store destination harder to do
        auto entry = f->getEntryBlock().begin();
        auto y_alloca = llvm::dyn_cast<llvm::AllocaInst>(&*(entry++));
        BOOST_TEST_CHECK(y_alloca, "First instruction must be alloca.");
        auto x_alloca = llvm::dyn_cast<llvm::AllocaInst>(&*(entry++));
        BOOST_TEST_CHECK(x_alloca, "Second instruction must be alloca.");

        // Value of x should be stored to alloca'd space
        auto x_store = llvm::dyn_cast<llvm::StoreInst>(&*(entry++));
        BOOST_TEST_CHECK(x_store, "Third instruction must be store.");
        if (x_store) {
            BOOST_TEST_CHECK(x_store->getOperand(1) == x_alloca, "Third instruction must store to x.");
        }

        // Value of x should be loaded from alloca'd space
        auto x_load = llvm::dyn_cast<llvm::LoadInst>(&*(entry++));
        BOOST_TEST_CHECK(x_load ,"Fourth instruction must be load.");
        if (x_load) {
            BOOST_TEST_CHECK(x_load->getOperand(0) == x_alloca, "Fourth instruction must load from x.");
        }

        // Loaded value should be stored to y's alloca'd space
        auto y_store = llvm::dyn_cast<llvm::StoreInst>(&*(entry++));
        BOOST_TEST_CHECK(y_store, "Fifth instruction must be store.");
        if (y_store) {
            BOOST_TEST_CHECK(y_store->getOperand(0) == x_load, "Fifth instruction must store loaded value.");
            BOOST_TEST_CHECK(y_store->getOperand(1) == y_alloca, "Fifth instruction must store to y.");
        }
    }

    BOOST_AUTO_TEST_CASE( global_vars_last_init ) {
        // Generate code
        Generator generator;
        generator.from_source("a = 1.0;\n"
                              "a = 2.0;");

        // Find global variable initializer function
        auto global_init = generator.module.getFunction("global_var_init");
        BOOST_TEST_CHECK(global_init, "Global variable initializer function must be present.");

        // Check global variable pi
        auto global_a = generator.module.getGlobalVariable("a");
        BOOST_TEST_CHECK(global_a, "Global variable a must be present.");
        if (global_a) {
            // Check initializer is 0
            auto zero = llvm::ConstantFP::get(generator.context, llvm::APFloat(0.0));
            BOOST_TEST_CHECK(global_a->hasInitializer(), "Global variable a must have initializer.");
            BOOST_TEST_CHECK(global_a->getInitializer() == zero, "Global variable a must have initializer of 0.0 .");

            // Check global variable initializer function contains store of 2.0
            auto &init_f_entry = global_init->getEntryBlock();
            bool last_store_correct = false;
            for (auto iter = init_f_entry.begin(); iter != init_f_entry.end(); iter++) {
                auto ptr = &*iter;
                if (ptr && llvm::isa<llvm::StoreInst>(ptr)) {
                    auto store = llvm::dyn_cast<llvm::StoreInst>(ptr);

                    // Check destination is pi
                    if (store->getOperand(1) != global_a) {
                        continue;
                    }

                    // Check value is 2.0
                    auto val = llvm::ConstantFP::get(generator.context, llvm::APFloat(2.0));
                    last_store_correct = store->getOperand(0) == val;
                }
            }
            BOOST_TEST_CHECK(last_store_correct, "Global variable initializer must end by initializing a to 2.0");
        }
    }

    //TODO check global variables

    BOOST_AUTO_TEST_CASE( example_program ) {
        // Generate code
        Generator generator;
        generator.from_source("pi = 3.14;\n"
                         "\n"
                         "get_pi() {\n"
                         "    return pi;\n"
                         "}\n"
                         "\n"
                         "write(x) {\n"
                         "    println(x);\n"
                         "}\n"
                         "\n"
                         "main() {\n"
                         "    write(get_pi());\n"
                         "    pi = 3.0;\n"
                         "    write(pi);\n"
                         "    write(1.0 + (3.0 * 4.0) % 5.0);\n"
                         "    return 0.0;\n"
                         "}");

        // Check STL (printf, println)
        BOOST_TEST_CHECK(generator.module.getFunction("printf"), "STL function printf must be present.");
        BOOST_TEST_CHECK(generator.module.getFunction("println"), "STL function println must be present.");

        // Check global variable initializer
        auto global_init = generator.module.getFunction("global_var_init");
        BOOST_TEST_CHECK(global_init, "Global variable initializer function must be present.");
        if (global_init) {
            // Check last instruction is return void
            auto last = &*global_init->getEntryBlock().rbegin();
            BOOST_TEST_CHECK(llvm::isa<llvm::ReturnInst>(last), "Last global initializer instruction must be return.");
            BOOST_TEST_CHECK(global_init->getReturnType() == llvm::Type::getVoidTy(generator.context),
                    "Global initializer must return void.");
        }

        // Check llvm.global_ctors
        auto global_ctors = generator.module.getGlobalVariable("llvm.global_ctors");
        BOOST_TEST_CHECK(global_ctors, "LLVM global ctors variable must be present.");
        if (global_ctors) {
            // Check type
            {
                auto type = global_ctors->getValueType();
                BOOST_TEST_CHECK(llvm::isa<llvm::ArrayType>(type), "LLVM global ctors must be an array.");
                auto array = llvm::dyn_cast<llvm::ArrayType>(type);
                BOOST_TEST_CHECK(array->getNumElements() == 1,
                                 "LLVM global ctors array must contain exactly one element.");
                auto elem_t = array->getElementType();
                BOOST_TEST_CHECK(llvm::isa<llvm::StructType>(elem_t), "LLVM global ctors elements must be structs.");
                auto elem_struct = llvm::dyn_cast<llvm::StructType>(elem_t);
                bool elem_0 = llvm::Type::getInt32Ty(generator.context) == elem_struct->getElementType(0);
                bool elem_1 =llvm::PointerType::get(llvm::FunctionType::get(llvm::Type::getVoidTy(generator.context), false),0) == elem_struct->getElementType(1);
                bool elem_2 = llvm::Type::getInt8PtrTy(generator.context) == elem_struct->getElementType(2);
                BOOST_TEST_CHECK((elem_0 && elem_1 && elem_2),
                                 "LLVM global ctors element structs must consist of i32, void ()* and i8*.");
            }

            // Check appending linkage
            BOOST_TEST_CHECK(global_ctors->getLinkage() == llvm::GlobalVariable::LinkageTypes::AppendingLinkage,
                    "LLVM global ctors must have appending linkage.");

            // Check contents
            auto array = llvm::dyn_cast<llvm::ConstantArray>(global_ctors->getInitializer());
            auto elem_struct = llvm::dyn_cast<llvm::ConstantStruct>(array->getAggregateElement(unsigned(0)));
            auto priority = llvm::dyn_cast<llvm::ConstantInt>(elem_struct->getAggregateElement(unsigned(0)));
            bool priority_valid = priority->getValue() == 65535;
            BOOST_TEST_CHECK(priority_valid, "LLVM global ctors element priority must be 65535.");
            auto f_ptr = llvm::dyn_cast<llvm::Function>(elem_struct->getAggregateElement(unsigned(1)));
            BOOST_TEST_CHECK(f_ptr == global_init, "LLVM global ctors element must point to global_var_init().");
            auto data = llvm::dyn_cast<llvm::ConstantPointerNull>(elem_struct->getAggregateElement(unsigned(2)));
            BOOST_TEST_CHECK(data, "LLVM global ctors element must have data pointer equal to null pointer.");
        }

        // Check global variable pi
        auto global_pi = generator.module.getGlobalVariable("pi");
        BOOST_TEST_CHECK(global_pi, "Global variable pi must be present.");
        if (global_pi) {
            // Check initializer is 0
            auto zero = llvm::ConstantFP::get(generator.context, llvm::APFloat(0.0));
            BOOST_TEST_CHECK(global_pi->hasInitializer(), "Global variable pi must have initializer.");
            BOOST_TEST_CHECK(global_pi->getInitializer() == zero, "Global variable pi must have initializer of 0.0 .");

            // Check global variable initializer contains store of 3.14
            auto &init_f_entry = generator.module.getFunction("global_var_init")->getEntryBlock();
            bool has_pi_store = false;
            for (auto iter = init_f_entry.begin(); iter != init_f_entry.end(); iter++) {
                auto ptr = &*iter;
                if (ptr && llvm::isa<llvm::StoreInst>(ptr)) {
                    auto store = llvm::dyn_cast<llvm::StoreInst>(ptr);

                    // Check destination is pi
                    if (store->getOperand(1) != global_pi) {
                        continue;
                    }

                    // Check value is 3.14
                    auto pi_val = llvm::ConstantFP::get(generator.context, llvm::APFloat(3.14));
                    if (store->getOperand(0) != pi_val) {
                        continue;
                    }

                    has_pi_store = true;
                }
            }
            BOOST_TEST_CHECK(has_pi_store, "Global variable initializer must contain store of 3.14 to pi");
        }

        // Check main wrapper and renamed main_
        auto main = generator.module.getFunction("main_");
        BOOST_TEST_CHECK(main, "Main function must be present and renamed to 'main_'.");
        if (main) {
            // Check return type is double
            BOOST_TEST_CHECK(main->getReturnType() == llvm::Type::getDoubleTy(generator.context),
                    "Main function must return double.");
        }
        auto main_wrap = generator.module.getFunction("main");
        BOOST_TEST_CHECK(main_wrap, "Main function wrapper must be present and named 'main'.");
        if (main_wrap) {
            // Check return type is i32
            BOOST_TEST_CHECK(main_wrap->getReturnType() == llvm::Type::getInt32Ty(generator.context),
                             "Main function wrapper must return i32.");
        }

        // Check get_pi
        auto get_pi = generator.module.getFunction("get_pi");
        BOOST_TEST_CHECK(get_pi, "Function get_pi must be present.");
        if (get_pi) {
            // Check return type is double
            BOOST_TEST_CHECK(get_pi->getReturnType() == llvm::Type::getDoubleTy(generator.context),
                             "Function get_pi must return double.");

            // Check number of arguments
            BOOST_TEST_CHECK(get_pi->arg_size() == 0, "Function get_pi must take no arguments.");
        }

        // Check write
        auto write = generator.module.getFunction("write");
        BOOST_TEST_CHECK(write, "Function write must be present.");
        if (write) {
            // Check return type is double
            BOOST_TEST_CHECK(write->getReturnType() == llvm::Type::getDoubleTy(generator.context),
                             "Function write must return double.");

            // Check number of arguments
            BOOST_TEST_CHECK(write->arg_size() == 1, "Function write must take one argument.");
        }
    }
BOOST_AUTO_TEST_SUITE_END()
