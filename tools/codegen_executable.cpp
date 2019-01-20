/** \file codegen_executable.cpp
 * Standalone basilisk Code generator
 *
 * \author Filip Smola
 */

#include <basilisk/Lexer.h>
#include <basilisk/Parser.h>
#include <basilisk/AST_util.h>
#include <basilisk/Tokens.h>
#include <basilisk/Codegen.h>
#include <basilisk/config.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <functional>
#include <basilisk/Parser.h>

/**
 * \brief Print usage into standard error
 *
 * \param name Executable name
 */
void show_usage(const std::string &name) {
    std::cerr << "Usage:\n"
              << '\t' << name << '\n'                       // Stdin input, stdout output
              << '\t' << name << " source\n"                // File input, stdout output
              << '\t' << name << " source destination\n"    // File input, file output
              << '\t' << name << " -h | --help\n"           // Show usage
              << '\t' << name << " -v | --version\n"        // Show basilisk version
              << '\n'
              << "Options:\n"
              << "\t-h --help\tShow this screen.\n"
              << "\t-v --version\tShow Basilisk version.\n";
}

char stdin_get() { return static_cast<char>(std::cin.get()); }
char stdin_peek() { return static_cast<char>(std::cin.peek()); }

/**
 * \brief Lex standard input stream
 *
 * \param append_f Token append function
 */
void lex_stdin(basilisk::lexer::append_function_t append_f) {
    auto get_f = std::bind(&stdin_get);
    auto peek_f = std::bind(&stdin_peek);

    basilisk::lexer::lex(get_f, peek_f, append_f);
}

char file_get(std::ifstream *stream) { return static_cast<char>(stream->get()); }
char file_peek(std::ifstream *stream) { return static_cast<char>(stream->peek()); }

/**
 * \brief Lex file input stream
 *
 * \param source_filename Name of the source file
 * \param append_f Token append function
 */
void lex_file(const std::string &source_filename, basilisk::lexer::append_function_t append_f) {
    // Open stream
    std::ifstream stream(source_filename, std::ios::in);

    if (!stream.is_open()) {
        // Print error if not open
        std::cerr << "Failed to open file " << source_filename << '\n';
    } else {
        auto get_f = std::bind(&file_get, &stream);
        auto peek_f = std::bind(&file_peek, &stream);

        basilisk::lexer::lex(get_f, peek_f, append_f);
    }
}

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

int main(int argc, char *argv[]) {
    // Compute actual argument number (without executable name)
    std::string name = argv[0];
    int n = argc - 1;

    // Lex the input
    std::vector<basilisk::tokens::Token> buffer;
    basilisk::lexer::append_function_t append_f = [&buffer](basilisk::tokens::Token t){ buffer.push_back(t); };
    if (n == 0) {
        // Stdin input
        lex_stdin(append_f);
    } else if (n == 1) {
        // Check the argument for help or version
        std::string arg = argv[1];

        if (arg == "-h" || arg == "--help") {
            // Show usage
            show_usage(name);
        } else if (arg == "-v" || arg == "--version") {
            // Show basilisk version
            std::cout << "Basilisk " << basilisk::version_full << "\n";
        } else {
            // File input
            lex_file(arg, append_f);
        }
    } else if (n == 2) {
        // File input
        std::string source = argv[1];
        lex_file(source, append_f);
    } else {
        // Invalid --> show usage
        show_usage(name);
    }

    // Check there are tokens
    if (!buffer.empty()) {
        // Reverse vector to prepare buffer
        std::reverse(buffer.begin(), buffer.end());

        // Bind functions
        auto get_f = std::bind(&parser_get, &buffer);
        auto peek_f = std::bind(&parser_peek, &buffer, std::placeholders::_1);

        // Parse program
        auto program = basilisk::parser::ProgramParser(get_f, peek_f).program();

        // Generate code
        llvm::LLVMContext context;
        llvm::IRBuilder<> builder(context);
        llvm::Module module("standalone codegen", context);
        basilisk::codegen::NamedValues named_values;
        basilisk::codegen::ProgramCodegen program_cg(context, builder, &module, named_values);
        program.accept(program_cg);

        // Write the output
        if (n < 2) {
            // Stdout output
            module.print(llvm::outs(), nullptr);
        } else if (n == 2) {
            // File output
            //TODO print llvm, not parsed tree
            std::string destination = argv[2];
            std::ofstream output(destination, std::ios::out);
            output << basilisk::ast::util::PrintVisitor::print(program);
        }
    } else {
        std::cerr << "No tokens could be lexed from the input.";
    }
}
