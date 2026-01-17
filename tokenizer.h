#pragma once

#include <variant>
#include <optional>
#include <istream>
#include "error.h"

struct SymbolToken {
    std::string name;

    bool operator==(const SymbolToken& other) const {
        return name == other.name;
    };
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const {
        return true;
    };
};

struct DotToken {
    bool operator==(const DotToken&) const {
        return true;
    };
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int value;

    bool operator==(const ConstantToken& other) const {
        return value == other.value;
    };
};
struct BooleanToken {
    bool f;

    bool operator==(const BooleanToken& other) const {
        return f == other.f;
    }
};

using Token =
    std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken, BooleanToken>;

class Tokenizer {
private:
    int symb_ = 0;
    std::istream* stream_;
    std::optional<Token> current_;

public:
    Tokenizer(std::istream* in) : stream_(in) {
        Next();
    };

    bool IsEnd() {
        return !current_.has_value();
    };
    bool IsStartSymbol(int c) {
        if (c == EOF) {
            return false;
        }
        unsigned char ch = static_cast<unsigned char>(c);
        return std::isalpha(ch) || ch == '<' || ch == '>' || ch == '=' || ch == '*' || ch == '/' ||
               ch == '#';
    }

    bool IsMidSymbol(int c) {
        if (c == EOF) {
            return false;
        }
        unsigned char ch = static_cast<unsigned char>(c);
        return std::isalpha(ch) || std::isdigit(ch) || ch == '<' || ch == '>' || ch == '=' ||
               ch == '*' || ch == '/' || ch == '#' || ch == '?' || ch == '!' || ch == '-';
    }
    void SkipWhitespace() {
        while (stream_->peek() != EOF &&
               std::isspace(static_cast<unsigned char>(stream_->peek()))) {
            stream_->get();
        }
    }

    void Next() {
        SkipWhitespace();
        if (stream_->peek() == EOF) {
            current_.reset();
            return;
        }
        symb_ = stream_->get();
        std::string token;
        switch (symb_) {
            case '(':
                current_ = Token{BracketToken::OPEN};
                return;
            case ')':
                current_ = Token{BracketToken::CLOSE};
                return;
            case '\'':
                current_ = Token{QuoteToken{}};
                return;
            case '.':
                current_ = Token{DotToken{}};
                return;
            default:
                break;
        }
        if (std::isdigit(static_cast<unsigned char>(symb_))) {
            int value = symb_ - '0';
            while (stream_->peek() != EOF &&
                   std::isdigit(static_cast<unsigned char>(stream_->peek()))) {
                symb_ = stream_->get();

                value = value * 10 + (symb_ - '0');
            }
            current_ = Token{ConstantToken{value}};
            return;
        }
        if (symb_ == '+' || symb_ == '-') {
            if (stream_->peek() != EOF &&
                std::isdigit(static_cast<unsigned char>(stream_->peek()))) {
                char sign = symb_;
                int value = 0;
                symb_ = stream_->get();
                value = symb_ - '0';
                while (stream_->peek() != EOF &&
                       std::isdigit(static_cast<unsigned char>(stream_->peek()))) {
                    symb_ = stream_->get();

                    value = value * 10 + (symb_ - '0');
                }
                if (sign == '+') {
                    current_ = Token{ConstantToken{value}};
                    return;
                }
                current_ = Token{ConstantToken{-value}};
                return;
            }
            current_ = Token{SymbolToken{std::string(1, static_cast<char>(symb_))}};
            return;
        } else if (symb_ == '#') {
            int c = stream_->peek();
            if ((c == 't' || c == 'f') && c != EOF) {
                stream_->get();
                int after = stream_->peek();
                if (!IsMidSymbol(after)) {
                    current_ = Token{BooleanToken{c == 't'}};
                    return;
                }
                std::string token;
                token.push_back('#');
                token.push_back(static_cast<char>(c));
                while (IsMidSymbol(stream_->peek())) {
                    symb_ = stream_->get();
                    token.push_back(static_cast<char>(symb_));
                }
                current_ = Token{SymbolToken{token}};
                return;
            }
        } else if (IsStartSymbol(symb_)) {
            token.push_back(static_cast<char>(symb_));
            while (IsMidSymbol(stream_->peek())) {
                symb_ = stream_->get();

                token.push_back(static_cast<char>(symb_));
            }
            current_ = Token{SymbolToken{token}};
            return;
        }
        throw(SyntaxError(""));
    }
    Token GetToken() {
        if (!current_) {
            throw SyntaxError("");
        }
        return *current_;
    }
};
