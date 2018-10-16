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
    constexpr tokens::tags::token_tag special_tags[] = {tokens::tags::LPAR, tokens::tags::RPAR, tokens::tags::LBRAC,
                                                        tokens::tags::RBRAC, tokens::tags::COMMA, tokens::tags::SEMICOLON,
                                                        tokens::tags::ASSIGN, tokens::tags::PLUS, tokens::tags::MINUS,
                                                        tokens::tags::STAR, tokens::tags::SLASH, tokens::tags::PERCENT};
    //! End of file character
    constexpr char end_of_file = std::char_traits<char>::eof();

    /**
     * \brief Lex from an input character buffer into an output Token buffer
     *
     * Use `get` function to obtain characters from an input buffer, lex the resulting string, and use `append` to write
     *  them into an output Token buffer.
     *
     * \param get Function to get next input character
     * \param append Function to write next output Token
     */
    void lex(get_function_t get, append_function_t append) {
        // Prepare the state variables
        bool stop = false;
        std::ostringstream content;
        constexpr char ret_pattern[] = {'r', 'e', 't', 'u', 'r', 'n'};
        unsigned ret_idx = 0;
        bool ret = false;
        bool identifier = false;
        bool double_literal = false;
        bool after_period = false;

        // Keep grabbing input until told to stop
        do {
            // Grab the next character
            char c = get();

            // Find the character in the special symbols array
            const char *p = std::find(std::begin(special_symbols), std::end(special_symbols), c);
            bool is_special = p != std::end(special_symbols);

            // Check if the character is end of input
            bool is_end =  c == '\0' || c == end_of_file;

            // Handle special symbols, whitespace and end of input
            if (is_special || std::isspace(c) != 0 || is_end) {
                // Check if return was read
                if (ret_idx == 6) {
                    // Append the return token
                    append(tokens::Token{tokens::tags::RETURN, ""});

                    // Set identifier to false as this was a valid return
                    identifier = false;
                }

                // Check if identifier was read
                if (identifier) {
                    // Append the identifier token
                    append(tokens::Token{tokens::tags::IDENTIFIER, content.str()});
                }

                // Check if double literal was read
                if (double_literal) {
                    // Append the double literal token
                    append(tokens::Token{tokens::tags::DOUBLE_LITERAL, content.str()});
                }

                // Reset complex token state variables
                //TODO: try clearing instead of creating a new stream
                content = std::ostringstream();
                ret_idx = 0;
                ret = false;
                identifier = false;
                double_literal = false;
                after_period = false;

                // Append the relevant token if special or stop if end of input
                if (is_special) {
                    // Compute the index of the special symbol
                    auto i = p - special_symbols;

                    // Append the relevant token
                    append(tokens::Token{special_tags[i], ""});
                } else if (is_end) {
                    // Append the end token
                    append(tokens::Token{tokens::tags::END, ""});

                    // Stop
                    stop = true;
                }
            } else {
                // The character is a part of a complex token or is an error --> check expectations
                if (identifier) {
                    // Expecting identifier (or return) --> valid: alphanumeric and underscore
                    if (std::isalnum(c) != 0 || c == '_') {
                        // Add to the content
                        content << c;

                        // Check if next return character
                        if (ret && ret_idx < 6 && c == ret_pattern[ret_idx]) {
                            // Advance
                            ret_idx++;
                        } else {
                            // Not return
                            ret_idx = 0;
                            ret = false;
                        }
                    } else {
                        // Invalid --> append error token and throw exception
                        std::ostringstream message;
                        message << "Invalid identifier or return character: \'" << c << "\'.";
                        append(tokens::Token{tokens::tags::ERROR, message.str()});
                        //TODO: throw exception
                    }
                } else if (double_literal) {
                    // Expecting double literal --> valid: digits and period (iff not yet encountered)
                    if (c == '.') {
                        if (after_period) {
                            // Period already encountered --> append error token and throw exception
                            std::ostringstream message;
                            message << "Invalid period in double literal.";
                            append(tokens::Token{tokens::tags::ERROR, message.str()});
                            //TODO: throw exception
                        } else {
                            // Add to the content and update flag
                            content << c;
                            after_period = true;
                        }
                    } else if (std::isdigit(c) != 0) {
                        // Add to the content
                        content << c;
                    } else {
                        // Invalid --> append error token and throw exception
                        std::ostringstream message;
                        message << "Invalid double literal character: \'" << c << "\'.";
                        append(tokens::Token{tokens::tags::ERROR, message.str()});
                        //TODO: throw exception
                    }
                } else {
                    // Not expecting anything --> valid: alpha for identifier or return, digit for double literal
                    if (std::isalpha(c) != 0) {
                        // Add to content
                        content << c;

                        // Expect identifier
                        identifier = true;

                        // Decide whether to expect return
                        if (c == 'r') {
                            ret = true;
                            ret_idx = 1;
                        }
                    } else if (std::isdigit(c) != 0) {
                        // Add to content
                        content << c;

                        // Expect double literal
                        double_literal = true;
                    } else {
                        // Invalid --> append error token and throw exception
                        std::ostringstream message;
                        message << "Unknown character: \'" << c << "\'.";
                        append(tokens::Token{tokens::tags::ERROR, message.str()});
                        //TODO: throw exception
                    }
                }
            }
        } while (!stop);
    }
}
