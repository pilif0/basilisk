/** \file lexer_executable.cpp
 * Standalone basilisk Lexer
 *
 * \author Filip Smola
 */

#include <basilisk/Lexer.h>
#include <basilisk/Tokens.h>
#include <basilisk/config.h>

#include <iostream>
#include <fstream>
#include <functional>

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
void stdout_append(const basilisk::tokens::Token &t) { std::cout << t << '|'; }

/**
 * \brief Lex standard input stream into standard output stream
 */
void lex_stdin_to_stdout() {
    auto get_f = std::bind(&stdin_get);
    auto peek_f = std::bind(&stdin_peek);
    auto append_f = std::bind(&stdout_append, std::placeholders::_1);

    basilisk::lexer::lex(get_f, peek_f, append_f);
}

char file_get(std::ifstream *stream) { return static_cast<char>(stream->get()); }
char file_peek(std::ifstream *stream) { return static_cast<char>(stream->peek()); }

/**
 * \brief Lex file input stream into standard output stream
 *
 * \param source_filename Name of the source file
 */
void lex_file_to_stdout(const std::string &source_filename) {
    // Open stream
    std::ifstream stream(source_filename, std::ios::in);

    if (!stream.is_open()) {
        // Print error if not open
        std::cerr << "Failed to open file " << source_filename << '\n';
    } else {
        auto get_f = std::bind(&file_get, &stream);
        auto peek_f = std::bind(&file_peek, &stream);
        auto append_f = std::bind(&stdout_append, std::placeholders::_1);

        basilisk::lexer::lex(get_f, peek_f, append_f);
    }
}

void file_append(std::ofstream *stream, const basilisk::tokens::Token &t) { (*stream) << t << '|'; }

/**
 * \brief Lex file input stream into file output stream
 *
 * \param source_filename Name of the source file
 * \param destination_filename Name of the destination file
 */
void lex_file_to_file(const std::string &source_filename, const std::string &destination_filename) {
    // Open stream
    std::ifstream input(source_filename, std::ios::in);
    std::ofstream output(destination_filename, std::ios::out);

    if (!input.is_open()) {
        // Print error if input not open
        std::cerr << "Failed to open file " << source_filename << '\n';
    } else if (!output.is_open()) {
        // Print error if output not open
        std::cerr << "Failed to open file " << destination_filename << '\n';
    } else {
        auto get_f = std::bind(&file_get, &input);
        auto peek_f = std::bind(&file_peek, &input);
        auto append_f = std::bind(&file_append, &output, std::placeholders::_1);

        basilisk::lexer::lex(get_f, peek_f, append_f);
    }
}

int main(int argc, char *argv[]) {
    // Compute actual argument number (without executable name)
    std::string name = argv[0];
    int n = argc - 1;

    // Decide what to do
    if (n == 0) {
        // Stdin input, stdout output
        lex_stdin_to_stdout();
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
            // File input, stdout output
            lex_file_to_stdout(arg);
        }
    } else if (n == 2) {
        // File input, file output
        std::string source = argv[1];
        std::string destination = argv[2];
        lex_file_to_file(source, destination);
    } else {
        // Invalid --> show usage
        show_usage(name);
    }
}
