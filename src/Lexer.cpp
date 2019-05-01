/** \file Lexer.cpp
 * Lexer implementation
 *
 * \author Filip Smola
 */

#include <basilisk/Lexer.h>
#include <basilisk/Tokens.h>

#include <sstream>
#include <algorithm>
#include <cctype>
#include <string>

namespace basilisk::lexer {
    //! Symbols that translate directly to tokens
    constexpr char special_symbols[] = {'(', ')', '{', '}', ',', ';', '=', '+', '-', '*', '/', '%'};
    //! Tags for the special symbols in the same order
    constexpr tokens::tags::token_tag special_tags[] = {tokens::tags::lpar, tokens::tags::rpar, tokens::tags::lbrac,
                                                        tokens::tags::rbrac, tokens::tags::comma, tokens::tags::semicolon,
                                                        tokens::tags::assign, tokens::tags::plus, tokens::tags::minus,
                                                        tokens::tags::star, tokens::tags::slash, tokens::tags::percent};
    //! End of file character
    constexpr char end_of_file = std::char_traits<char>::eof();

    // Character classification helpers
    /**
     * \brief Whether a character is end of input
     *
     * \param c Character to check
     * \return `true` when end of input, `false` otherwise
     */
    bool is_end(char c) { return c == '\0' || c == end_of_file; }

    /**
     * \brief Compute index of the character in the special symbols array, or `-1` if not found
     *
     * \param c Character to find
     * \return index into the special symbols array, or `-1` if not found
     */
    long which_special(char c) {
        const char *p = std::find(std::begin(special_symbols), std::end(special_symbols), c);
        if (p == std::end(special_symbols)) {
            return -1;
        } else {
            return p - special_symbols;
        }
    }

    // Specific lexing cases
    /**
     * \brief Lex from an input character buffer into an output Token buffer, assuming an alpha character was peeked.
     *
     * Use `get` function to obtain characters from an input buffer, lex the resulting string, and use `append` to write
     *  them into an output Token buffer.
     * We are expecting either an identifier or a keyword.
     * By principle of maximum munch, consume the maximum valid identifier and before tokenization check it against the
     *  valid keywords.
     *
     * \param get Function to get next input character
     * \param peek Function to peek at next input character
     * \param append Function to write next output Token
     */
    void lex_alpha(const get_function_t &get, const peek_function_t &peek, const append_function_t &append) {
        // Alpha was detected and we are not expecting anything --> expect return or identifier
        constexpr auto return_pattern = "return";
        std::ostringstream stream;

        // Consume characters until the next one is not alphanumeric or underscore
        // Note: the requirement for the first character to be only alpha is satisfied by how this function is called
        for (char c = peek(); std::isalnum(c) > 0 || c == '_'; c = peek()) {
            stream << get();
        }

        // Decide what token to append
        std::string content = stream.str();
        if (content == return_pattern) {
            append(tokens::Token{tokens::tags::kw_return, ""});
        } else {
            append(tokens::Token{tokens::tags::identifier, content});
        }
    }


    /**
     * \brief Lex from an input character buffer into an output Token buffer, assuming an digit was peeked.
     *
     * Use `get` function to obtain characters from an input buffer, lex the resulting string, and use `append` to write
     *  them into an output Token buffer.
     * We are expecting a double literal.
     * Consume a sequence of digits, a decimal point, and a sequence of digits.
     *
     * \param get Function to get next input character
     * \param peek Function to peek at next input character
     * \param append Function to write next output Token
     */
    void lex_digit(const get_function_t &get, const peek_function_t &peek, const append_function_t &append) {
        // Digit was detected and we are not expecting anything --> expect double literal
        std::ostringstream stream;

        // Consume digits
        for (char c = peek(); std::isdigit(c) > 0; c = peek()) {
            stream << get();
        }

        // Check that the next character is a decimal point
        {
            char c = get();
            if (c != '.') {
                // Invalid input --> append error token and throw exception
                std::ostringstream message;
                message << "Unexpected character: \'" << c << "\', expecting a decimal point.";
                append(tokens::Token{tokens::tags::error, message.str()});
                throw LexerException(message.str());
            } else {
                stream << c;
            }
        }

        // Check at least one digit follows
        {
            char c = peek();
            if (std::isdigit(c) == 0) {
                // Invalid input --> eat it, append error token and throw exception
                get();
                std::ostringstream message;
                message << "Unexpected character: \'" << c << "\', expecting a digit.";
                append(tokens::Token{tokens::tags::error, message.str()});
                throw LexerException(message.str());
            }
        }

        // Consume digits
        for (char c = peek(); std::isdigit(c) > 0; c = peek()) {
            stream << get();
        }

        // Append the token
        append(tokens::Token{tokens::tags::double_literal, stream.str()});
    }

    // Lexing itself
    /**
     * \brief Lex from an input character buffer into an output Token buffer
     *
     * Use `get` function to obtain characters from an input buffer, lex the resulting string, and use `append` to write
     *  them into an output Token buffer.
     *
     * \param get Function to get next input character
     * \param peek Function to peek at next input character
     * \param append Function to write next output Token
     */
    void lex(const get_function_t &get, const peek_function_t &peek, const append_function_t &append) {
        bool stop = false;

        do {
            // Peek at the next character
            char next = peek();

            if (is_end(next)) {                         // Detect end of input
                // Eat it, append token, stop
                get();
                append(tokens::Token{tokens::tags::end_of_input, ""});
                stop = true;
            } else if (std::isspace(next) > 0) {        // Detect whitespace
                // Eat it
                get();
            } else if (std::isalpha(next) > 0) {        // Detect alpha
                lex_alpha(get, peek, append);
            } else if (std::isdigit(next) > 0) {        // Detect digit
                lex_digit(get, peek, append);
            } else if (which_special(next) >= 0) {      // Detect special characters
                // Eat it and append the token
                append(tokens::Token{special_tags[which_special(get())], ""});
            } else {
                // Invalid input --> eat it, append error token and throw exception
                get();
                std::ostringstream message;
                message << "Unknown character: \'" << next << "\'.";
                append(tokens::Token{tokens::tags::error, message.str()});
                throw LexerException(message.str());
            }
        } while (!stop);
    }
}
