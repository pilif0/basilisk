/** \file basilisk_c.cpp
 * Main basilisk compiler executable, combining the whole process from a source file to an object file.
 *
 * \author Filip Smola
 */

#include <basilisk/config.h>
#include <basilisk/Lexer.h>
#include <basilisk/Tokens.h>
#include <basilisk/Parser.h>
#include <basilisk/AST.h>
#include <basilisk/AST_util.h>
#include <basilisk/Codegen.h>

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <math.h>
#include <functional>
#include <exception>

//! Print usage into standard output
// Note: inspired by output of `clang --help`
void show_usage() {
    std::cout << "OVERVIEW: basilisk LLVM compiler\n\n"
              << "USAGE: basilisk [options] file\n\n"
              << "OPTIONS:\n"
              << "\t-h, --help\n\t\tShow this screen.\n"
              << "\t-v, --version\n\t\tShow Basilisk version.\n"
              << "\t-o, --output\n\t\tPath to output file. If not set, uses standard output stream.\n"
              << "\t-l, --lex\n\t\tPerform only lexing, and output the tokens.\n"
              << "\t-p, --parse\n\t\tPerform only lexing and parsing, and output the AST.\n"
              << "\t-g, --codegen\n\t\tPerform only lexing, parsing and code generation, and output the LLVM IR.\n"
              << "\t-G, --codegen-opt\n\t\tPerform only lexing, parsing, code generation and optimization, and output the optimized LLVM IR.\n";
}

//! Print version into standard output
void show_version() {
    std::cout << "Basilisk " << basilisk::version_full << "\n";
}

//! Return reference to standard error stream with the executable tag already printed
std::ostream& error() {
    return std::cerr << "basilisk: ";
}

//----- Start Lexing Section
//! Function to get a character from standard input
char stdin_get() { return static_cast<char>(std::cin.get()); }
//! Function to peek at a character from standard input
char stdin_peek() { return static_cast<char>(std::cin.peek()); }

/**
 * \brief Lex standard input stream
 *
 * \param append_f Token append function
 * \return `false` when there was a LexerException during lexing, `true` otherwise
 */
bool lex_stdin(const basilisk::lexer::append_function_t &append_f) {
    auto get_f = std::bind(&stdin_get);
    auto peek_f = std::bind(&stdin_peek);

    // Try to lex
    try {
        basilisk::lexer::lex(get_f, peek_f, append_f);
    } catch (basilisk::lexer::LexerException e) {
        // Print exception and return failure
        error() << "Lexer exception - " << e.what() << '\n';
        return false;
    }

    // Return success
    return true;
}

//! Function to get a character from the provided stream
char file_get(std::ifstream *stream) { return static_cast<char>(stream->get()); }
//! Function to peek at a character from the provided stream
char file_peek(std::ifstream *stream) { return static_cast<char>(stream->peek()); }

/**
 * \brief Lex file input stream
 *
 * \param source_filename Name of the source file
 * \param append_f Token append function
 * \return `false` when there was a LexerException during lexing, `true` otherwise
 */
bool lex_file(const std::string &source_filename, const basilisk::lexer::append_function_t &append_f) {
    // Open stream
    std::ifstream stream(source_filename, std::ios::in);

    if (!stream.is_open()) {
        // Print error if not open
        error() << "Failed to open file " << source_filename << '\n';
        return false;
    } else {
        auto get_f = std::bind(&file_get, &stream);
        auto peek_f = std::bind(&file_peek, &stream);

        // Try to lex
        try {
            basilisk::lexer::lex(get_f, peek_f, append_f);
        } catch (basilisk::lexer::LexerException e) {
            // Print exception and return failure
            error() << "Lexer exception - " << e.what() << '\n';
            return false;
        }
    }

    // Return success
    return true;
}
//----- End Lexing Section

//----- Start Parsing Section
/**
 * \brief Pop a token from the front of the queue and return it
 *
 * \param queue Pointer to the input token queue
 * \return Popped token, or an error token when empty
 */
basilisk::tokens::Token parser_get(std::vector<basilisk::tokens::Token> *queue) {
    // Return error if no queue
    if (!queue) {
        std::ostringstream message;
        message << "No input queue.";
        return basilisk::tokens::Token{basilisk::tokens::tags::error, message.str()};
    }

    // Return error token if empty
    if (queue->empty()) {
        return basilisk::tokens::Token{basilisk::tokens::tags::error, "No more input tokens."};
    }

    basilisk::tokens::Token t = queue->back();
    queue->pop_back();
    return t;
}

/**
 * \brief Peek at the token \c offset elements from the front of the input queue
 *
 * \param queue Pointer to the input token queue
 * \param offset Offset of the sought element from the front of the queue
 * \return Token \c offset elements from the front of the input queue, or the error token when not present
 */
basilisk::tokens::Token parser_peek(std::vector<basilisk::tokens::Token> *queue, unsigned offset) {
    // Return error if no queue
    if (!queue) {
        std::ostringstream message;
        message << "No input queue.";
        return basilisk::tokens::Token{basilisk::tokens::tags::error, message.str()};
    }

    // Return error token if not valid
    if (offset >= queue->size()) {
        std::ostringstream message;
        message << "No token " << offset << " elements from the front of the input queue.";
        return basilisk::tokens::Token{basilisk::tokens::tags::error, message.str()};
    }

    // Compute index
    auto index = static_cast<unsigned int>(queue->size() - 1 - offset);

    return (*queue)[index];
}
//----- End Parsing Section

int main(int argc, char *argv[]) {
    // Compute actual argument number (without executable name)
    int arg_n = argc - 1;

    // Decide on actions to perform based on arguments
    if (arg_n == 0) {
        // No arguments -> print usage
        show_usage();
    } else {
        // Some arguments -> process them (-h and -v stop execution)

        // Prepare state
        bool file_out = false;
        std::string filename_out;
        bool file_in = false;
        std::string filename_in;
        unsigned ops = 0; // 0 -> full, 1+ -> lex, 2+ -> parse, 3+ -> codegen, 4+ -> codegen-opt

        // Go through arguments
        for (int i = 1; i < argc; i++) {
            // Get the argument
            std::string arg = argv[i];

            // Check for options
            if (arg == "-h" || arg == "--help") {
                // Help -> show usage, flag for exit and stop processing
                show_usage();
                return 0;
            } else if (arg == "-v" || arg == "--version") {
                // Version -> show version, flag for exit and stop processing
                show_version();
                return 0;
            } else if (arg == "-o" || arg == "--output") {
                // Output -> update state, incrementing i to consume the following argument (filename)
                file_out = true;
                filename_out = argv[++i];
            } else if (arg == "-l" || arg == "--lex") {
                // Lex -> set ops to at least lex
                ops = std::max(ops, 1u);
            } else if (arg == "-p" || arg == "--parse") {
                // Parse -> set ops to at least parse
                ops = std::max(ops, 2u);
            } else if (arg == "-g" || arg == "--codegen") {
                // Codegen -> set ops to at least codegen
                ops = std::max(ops, 3u);
            } else if (arg == "-G" || arg == "--codegen-opt") {
                // Codegen -> set ops to at least codegen
                ops = std::max(ops, 4u);
            } else if (arg == "-") {
                // Standard input -> set file input to false and stop processing
                file_in = false;
                break;
            } else {
                // Input filename -> update state and stop processing
                file_in = true;
                filename_in = arg;
                break;
            }
        }

        // Lex the input
        std::vector<basilisk::tokens::Token> buffer;
        auto append_f = [&buffer](basilisk::tokens::Token t){ buffer.push_back(t); };
        bool lex_success;
        if (file_in) {
            // File input
            lex_success = lex_file(filename_in, append_f);
        } else {
            // Standard input
            lex_success = lex_stdin(append_f);
        }

        // Print error and terminate on lexing failure
        if (!lex_success) {
            error() << "Lexing failed.\n";
            return 1;
        }

        // Print error and terminate on no tokens
        if (buffer.empty()) {
            error() << "Lexing resulted in no tokens.\n";
            return 1;
        }

        // Output tokens if only lexing requested
        if (ops == 1) {
            // Only lexing requested -> output tokens
            if (file_out) {
                // Open a stream to the output file
                std::ofstream stream(filename_out, std::ios::out);

                if (!stream.is_open()) {
                    // Output not open -> print error and terminate
                    error() << "Failed to open file " << filename_out << '\n';
                    return 1;
                } else {
                    // Print tokens to the stream separated with vertical bars
                    for (basilisk::tokens::Token t : buffer) {
                        std::cout << t;
                        if (t != *(buffer.end())) {
                            std::cout << '|';
                        }
                    }
                }
            } else {
                // Print tokens to the stream separated with vertical bars
                for (basilisk::tokens::Token t : buffer) {
                    std::cout << t;
                    if (t != *(buffer.end())) {
                        std::cout << '|';
                    }
                }
            }
        } else {
            // Otherwise -> parse

            // Reverse tokens to prepare buffer
            std::reverse(buffer.begin(), buffer.end());

            // Bind functions
            auto get_f = std::bind(&parser_get, &buffer);
            auto peek_f = std::bind(&parser_peek, &buffer, std::placeholders::_1);

            // Parse program
            basilisk::ast::Program program({});
            try {
                program = basilisk::parser::ProgramParser(get_f, peek_f).program();
            } catch (basilisk::parser::ParserException e) {
                // Print exception and note failure
                error() << "Parser exception - " << e.what() << '\n'
                        << "Parsing failed.\n";
                return 1;
            }

            // Output AST if only parsing requested
            if (ops == 2) {
                // Only parsing requested -> output AST
                if (file_out) {
                    // Open a stream to the output file
                    std::ofstream stream(filename_out, std::ios::out);

                    if (!stream.is_open()) {
                        // Print error if output not open
                        error() << "Failed to open file " << filename_out << '\n';
                        return 1;
                    } else {
                        // Print AST to the stream
                        stream << basilisk::ast::util::PrintVisitor::print(program);
                    }
                } else {
                    // Print AST to the stream
                    std::cout << basilisk::ast::util::PrintVisitor::print(program);
                }
            } else {
                // Otherwise -> generate LLVM IR

                // Prepare state
                llvm::LLVMContext context;
                llvm::IRBuilder<> builder(context);
                llvm::Module module(file_in ? filename_in : "standard input", context);
                basilisk::codegen::NamedValuesStacks named_values;
                basilisk::codegen::ProgramCodegen program_cg(context, builder, &module, named_values);

                // Generate LLVM IR
                try {
                    program.accept(program_cg);
                } catch (std::exception e) {
                    // Print exception and note failure
                    error() << "LLVM IR generation exception - " << e.what() << '\n'
                            << "LLVM IR generation failed.\n";
                    return 1;
                }

                // Optimize unless unoptimized code generation was requested
                if (ops != 3) {
                    // Initialize optimization passes
                    auto pass_manager = llvm::legacy::PassManager();

                    // Add passes recommended in https://llvm.org/docs/tutorial/LangImpl04.html
                    pass_manager.add(llvm::createInstructionCombiningPass());   // Merge instructions
                    pass_manager.add(llvm::createReassociatePass());    // Use associativity to improve constant propagation
                    pass_manager.add(llvm::createGVNPass());    // Most importantly promote stack to registers
                    pass_manager.add(llvm::createCFGSimplificationPass());  // Simplify control flow graph

                    // Run the passes
                    try {
                        pass_manager.run(module);
                    } catch (std::exception e) {
                        // Print exception, note failure and terminate
                        error() << "LLVM optimization pass exception - " << e.what() << '\n'
                                << "LLVM optimization failed.\n";
                        return 1;
                    }
                }

                // Output LLVM IR if only (optimized or unoptimized) code generation was requested
                if (ops == 3 || ops == 4) {
                    // Only codegen requested -> output LLVM IR
                    if (file_out) {
                        // Open a stream to the output file and print the module into it
                        std::error_code ec;
                        llvm::raw_fd_ostream stream = llvm::raw_fd_ostream(filename_out, ec);
                        if (ec) {
                            // Error -> print message
                            error() << "Failed to open file " << filename_out << " - " << ec.message() <<'\n';
                            return 1;
                        } else {
                            // Fine -> print module
                            module.print(stream, nullptr);
                        }
                    } else {
                        // Print AST to the stream
                        module.print(llvm::outs(), nullptr);
                    }
                } else {
                    // Otherwise -> output object code

                    // Pick target
                    auto target_triple = llvm::sys::getDefaultTargetTriple();
                    llvm::InitializeAllTargetInfos();
                    llvm::InitializeAllTargets();
                    llvm::InitializeAllTargetMCs();
                    llvm::InitializeAllAsmParsers();
                    llvm::InitializeAllAsmPrinters();
                    std::string err;
                    auto target = llvm::TargetRegistry::lookupTarget(target_triple, err);

                    // Print error and terminate if target lookup failed
                    if (!target) {
                        error() << "Failed to look up target - " << err;
                        return 1;
                    }

                    // Get target machine
                    auto cpu = "generic";
                    auto features = "";
                    llvm::TargetOptions options;
                    auto relocation_model = llvm::Optional<llvm::Reloc::Model>(llvm::Reloc::Model::PIC_);
                    auto target_machine = target->createTargetMachine(target_triple, cpu, features, options, relocation_model);

                    // Configure module
                    module.setDataLayout(target_machine->createDataLayout());
                    module.setTargetTriple(target_triple);

                    // Prepare output stream and emit the object code
                    if (file_out) {
                        // Open a stream to the output file and emit the code into it
                        std::error_code ec;
                        auto stream = llvm::raw_fd_ostream(filename_out, ec);

                        // Print error and terminate on file open error
                        if (ec) {
                            error() << "Failed to open file " << filename_out << " - " << ec.message() <<'\n';
                            return 1;
                        }

                        // Create and run pass to emit object code
                        llvm::legacy::PassManager pass;
                        auto file_type = llvm::TargetMachine::CGFT_ObjectFile;
                        if (target_machine->addPassesToEmitFile(pass, stream, nullptr, file_type)) {
                            // Error -> print message
                            error() << "File emit pass error - TargetMachine can't emit a file of this type";
                            return 1;
                        } else {
                            // Otherwise -> run the pass
                            pass.run(module);
                        }
                    } else {
                        // Prepare a stream to a small string
                        llvm::SmallString<0> small_string;
                        auto stream = llvm::raw_svector_ostream(small_string);

                        // Create and run pass to emit object code
                        llvm::legacy::PassManager pass;
                        auto file_type = llvm::TargetMachine::CGFT_ObjectFile;
                        if (target_machine->addPassesToEmitFile(pass, stream, nullptr, file_type)) {
                            // Error -> print message
                            error() << "File emit pass error - TargetMachine can't emit a file of this type";
                            return 1;
                        } else {
                            // Otherwise -> run the pass
                            pass.run(module);
                        }

                        // Print the small string into standard output
                        std::cout << small_string.c_str();
                    }
                }
            }
        }
    }

    return 0;
}
