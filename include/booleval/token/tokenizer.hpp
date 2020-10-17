/*
 * Copyright (c) 2019, Marin Peko
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following disclaimer
 *   in the documentation and/or other materials provided with the
 *   distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef BOOLEVAL_TOKENIZER_H
#define BOOLEVAL_TOKENIZER_H

#include <vector>
#include <string_view>
#include <booleval/token/token.hpp>
#include <booleval/utils/string_utils.hpp>
#include <booleval/utils/split_range.hpp>

namespace booleval {

namespace token {

/**
 * class tokenizer
 *
 * Represents the mechanism for tokenizing the expressions, i.e. transforming
 * the expressions from a form of a string to the collection of tokens.
 */
template <char quote_char = utils::double_quote_char>
class tokenizer {
public:
    tokenizer() = default;
    tokenizer(tokenizer&& rhs) = default;
    tokenizer(tokenizer const& rhs) = default;

    tokenizer(std::string_view expression) noexcept
        : expression_(expression) {
    }

    tokenizer& operator=(tokenizer&& rhs) = default;
    tokenizer& operator=(tokenizer const& rhs) = default;

    ~tokenizer() = default;

    /**
     * Sets the expression that needs to be tokenized.
     *
     * @param expression Expression to be tokenized
     */
    void expression(std::string_view expression) noexcept {
        expression_ = expression;
    }

    /**
     * Gets the expression that needs to be tokenized.
     *
     * @return Expression to be tokenized
     */
    [[nodiscard]] std::string_view expression() const noexcept {
        return expression_;
    }

    /**
     * Checks whether more tokens exist or not.
     *
     * @return True if there is more tokens, otherwise false
     */
    [[nodiscard]] bool has_tokens() const noexcept {
        return current_token_index_ < tokens_.size();
    }

    /**
     * Passes the token by incrementing the current token index.
     */
    void pass_token() noexcept {
        current_token_index_++;
    }

    /**
     * Gets the next token and increments the current token index.
     *
     * @return Next token
     */
    [[nodiscard]] token const& next_token() {
        return tokens_.at(current_token_index_++);
    }

    /**
     * Gets the next token without incrementing the current token index.
     *
     * @return Next token
     */
    [[nodiscard]] token const& weak_next_token() {
        return tokens_.at(current_token_index_);
    }

    /**
     * Tokenizes the expression and transforms it into the collection of tokens.
     */
    void tokenize();

    /**
     * Clears the collection of tokens and sets the current index to zero.
     */
    void reset() noexcept {
        current_token_index_ = 0;
    }

private:
    std::string_view expression_;
    std::size_t current_token_index_{ 0 };
    std::vector<token> tokens_;
};

template <char quote_char>
void tokenizer<quote_char>::tokenize() {
    tokens_.clear();
    reset();

    constexpr auto parenthesis_symbols = parenthesis_symbol_expressions();
    constexpr auto options =
        utils::split_options::include_delimiters |
        utils::split_options::split_by_whitespace |
        utils::split_options::allow_quoted_strings;

    auto delims = utils::join(std::begin(parenthesis_symbols), std::end(parenthesis_symbols));
    auto tokens_range = utils::split_range<options, quote_char>(expression_, delims);

    for (auto const& [quoted, index, value] : tokens_range) {
        auto type = quoted ? token_type::field : map_to_token_type(value);

        if (token_type::field == type) {
            if (!tokens_.empty() && tokens_.back().is(token_type::field)) {
                tokens_.emplace_back(token_type::eq, map_to_token_value(token_type::eq));
            }
        }

        tokens_.emplace_back(type, value);
    }
}

} // token

} // booleval

#endif // BOOLEVAL_TOKENIZER_H