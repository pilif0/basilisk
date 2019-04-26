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
#include <basilisk/Codegen.h>

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
              << "\t-h --help\tShow this screen.\n"
              << "\t-v --version\tShow Basilisk version.\n"
              << "\t-o --output\tPath to output file. If not set, uses standard output stream."
              << "\t-l --lex\tPerform only lexing, and output the tokens."
              << "\t-p --parse\tPerform only lexing and parsing, and output the AST."
              << "\t-g --codegen\tPerform only lexing, parsing and code generation, and output the LLVM IR.";
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
        error() << "Lexer exception - " << e.what();
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
    } else {
        auto get_f = std::bind(&file_get, &stream);
        auto peek_f = std::bind(&file_peek, &stream);

        // Try to lex
        try {
            basilisk::lexer::lex(get_f, peek_f, append_f);
        } catch (basilisk::lexer::LexerException e) {
            // Print exception and return failure
            error() << "Lexer exception - " << e.what();
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
        unsigned ops = 0; // 0 -> full, 1+ -> lex, 2+ -> parse, 3+ -> codegen
        bool exit = false;

        // Go through arguments
        for (int i = 1; i < argc; i++) {
            // Get the argument
            std::string arg = argv[i];

            // Check for options
            if (arg == "-h" || arg == "--help") {
                // Help -> show usage, flag for exit and stop processing
                show_usage();
                exit = true;
                break;
            } else if (arg == "-v" || arg == "--version") {
                // Version -> show version, flag for exit and stop processing
                show_version();
                exit = true;
                break;
            } else if (arg == "-o" || arg == "--output") {
                // Output -> update state, incrementing i to consume the following argument (filename)
                file_out = true;
                filename_out = argv[++i];
            } else if (arg == "-l" || arg == "--output") {
                // Lex -> set ops to at least lex
                ops = std::max(ops, 1u);
            } else if (arg == "-p" || arg == "--output") {
                // Parse -> set ops to at least parse
                ops = std::max(ops, 2u);
            } else if (arg == "-g" || arg == "--output") {
                // Codegen -> set ops to at least codegen
                ops = std::max(ops, 3u);
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

        // Only continue if exit was not requested
        if (!exit) {
            // Lex the input
            std::vector<basilisk::tokens::Token> buffer;
            auto append_f = [&buffer](basilisk::tokens::Token t){ buffer.push_back(t); };
            bool lex_success = false;
            if (file_in) {
                // File input
                lex_success = lex_file(filename_in, append_f);
            } else {
                // Standard input
                lex_success = lex_stdin(append_f);
            }

            // Only continue iff lexing succeeded, token buffer is not empty, and further steps are requested
            if (lex_success && !buffer.empty() && ops != 1) {
                // Reverse tokens to prepare buffer
                std::reverse(buffer.begin(), buffer.end());

                // Bind functions
                auto get_f = std::bind(&parser_get, &buffer);
                auto peek_f = std::bind(&parser_peek, &buffer, std::placeholders::_1);

                // Parse program
                bool parse_success = true;
                basilisk::ast::Program program({});
                try {
                    program = basilisk::parser::ProgramParser(get_f, peek_f).program();
                } catch (basilisk::parser::ParserException e) {
                    // Print exception and note failure
                    error() << "Parser exception - " << e.what();
                    parse_success = false;
                }

                // Only continue iff parsing succeeded and further steps are requested
                if (parse_success && (ops == 0 || ops == 3)) {
                    // Prepare state
                    llvm::LLVMContext context;
                    llvm::IRBuilder<> builder(context);
                    llvm::Module module("standalone codegen", context); //TODO different module name? e.g. src file name
                    basilisk::codegen::NamedValuesStacks named_values;
                    basilisk::codegen::ProgramCodegen program_cg(context, builder, &module, named_values);

                    // Generate LLVM IR
                    bool codegen_success = true;
                    try {
                        program.accept(program_cg);
                    } catch (std::exception e) {
                        // Print exception and note failure
                        error() << "LLVM IR generation exception - " << e.what();
                        codegen_success = false;
                    }

                    // Only continue iff codegen succeeded and further steps are requested
                    if (codegen_success && ops == 0) {
                        //TODO
                    } else if (!codegen_success) {
                        error() << "LLVM IR generation failed.";
                    } else {
                        // Otherwise only codegen requested -> output LLVM IR
                        //TODO
                    }
                } else if (!parse_success) {
                    error() << "Parsing failed.";
                } else {
                    // Otherwise only parsing requested -> output AST
                    //TODO
                }
            } else if (!lex_success) {
                error() << "Lexing failed.";
            } else if (buffer.empty()) {
                error() << "Lexing resulted in no tokens.";
            } else {
                // Otherwise only lexing requested -> output tokens
                //TODO
            }
        }
    }
}
