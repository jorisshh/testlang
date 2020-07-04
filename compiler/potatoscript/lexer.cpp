#include "lexer.h"

#include <iostream>

using namespace lang::lexer;

static bool isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\v' || c == '\r' || c == '\n';
}

int toLower(int c) {
    if (c >= 'A' && c <= 'Z') return c + 0x20;

    return c;
}

static bool isLetter(int c) {
    c = toLower(c);
    return c >= 'a' && c <= 'z';
}

static bool isDigit(int c, int baseCount = 10) {

    if (baseCount != 10) {
        std::cerr << "only base 10 is allowed atm";
    }

    //if (baseCount == 2) {
    //    return c >= '0' && c <= '1';
    //}

    //if (baseCount == 16) {
    //    int l = toLower(c);
    //    if (l >= 'a' && l <= 'f') return true;
    //}

    return c >= '0' && c <= '9';
}

static bool continuesDigit(int c, int baseCount = 10) {

    return isDigit(c, baseCount) || c == '.' || c == 'f' || c == 'u' || c == 'i';
}

static bool startsIdentifier(char c) {
    return c == '_' || isLetter(c);
}

static bool continuesIdentifier(char c) {
    return isLetter(c) || c == '_' || isDigit(c);
}

static size_t countUntilCharacter(const std::string& s, size_t index, char find) {
    size_t count = 0;
    for (size_t i = index; i < s.size(); i++) {
        if (s[i] == find) {
            return count;
        }

        count++;
    }

    return 0;
}

static bool isCommentStart(const std::string& s, size_t index) {
    if (index + 1 >= s.size()) {
        return false;
    }

    return s[index] == '/' && s[index + 1] == '/';
}

static bool isFunctionStart(const std::string& s, size_t index) {
    if (index + 1 >= s.size()) {
        return false;
    }

    return s[index] == 'f' && s[index + 1] == 'n';
}

static bool isKeyword(const std::string& s, size_t index, const std::string& contains) {
    if (index + contains.size() >= s.size()) {
        return false;
    }

    for (size_t i = 0; i < contains.size(); i++) {
        if (s[i + index] != contains[i]) {
            return false;
        }
    }

    return true;
}

static Token createSingleToken(TokenType::Type t, const std::string& s, size_t index, size_t length) {
    Token res{
        .type = t,
        .span = TextSpan {
            .string = s.substr(index, length),
            .from = index,
            .to = index + length,
        },
        .fileName = "undefined", // TODO: Assign...
    };

    return res;
}

static size_t max(size_t a, size_t b) {
    if (a >= b) return a;
    return b;
}

static size_t min(size_t a, size_t b) {
    if (a < b) return a;
    return b;
}

static size_t minButNot0(size_t a, size_t b) {
    if (a < b && a > 0) return a;
    return b;
}

static bool tryGetReservedBasicType(const std::string& s, TokenType::Type* outResult) {
   
    if (s == "u8") { *outResult = TokenType::KEYWORD_UINT8; return true; }
    if (s == "u16") { *outResult = TokenType::KEYWORD_UINT16; return true; }
    if (s == "u32") { *outResult = TokenType::KEYWORD_UINT32; return true; }
    if (s == "u64") { *outResult = TokenType::KEYWORD_UINT64; return true; }

    if (s == "i8") { *outResult = TokenType::KEYWORD_INT8; return true; }
    if (s == "i16") { *outResult = TokenType::KEYWORD_INT16; return true; }
    if (s == "i32") { *outResult = TokenType::KEYWORD_INT32; return true; }
    if (s == "i64") { *outResult = TokenType::KEYWORD_INT64; return true; }

    if (s == "f32") { *outResult = TokenType::KEYWORD_FLOAT32; return true; }
    if (s == "f64") { *outResult = TokenType::KEYWORD_FLOAT64; return true; }

    if (s == "bool") { *outResult = TokenType::KEYWORD_BOOL; return true; }
    if (s == "string") { *outResult = TokenType::KEYWORD_STRING; return true; }
    if (s == "void") { *outResult = TokenType::KEYWORD_VOID; return true; }

    return false;
}

const char eol = '\n';
const char whiteSpace = ' ';
using namespace lang::lexer;

std::vector<Token> lang::lexer::parse(const std::string& s) noexcept
{
    std::vector<Token> token;
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        
        if (isCommentStart(s, i)) {
            size_t commentLength = countUntilCharacter(s, i, eol);
            
            std::string s2 = s.substr(i, commentLength);
            Token t{
                .type = TokenType::COMMENT,
                .span = TextSpan {
                    .string = s2,
                    .from = i,
                    .to = i + commentLength,
                },
                .fileName = "undefined", // TODO: Assign...
            };

            token.push_back(t);

            i += commentLength;
            continue;
        }

        if (isKeyword(s, i, "fn")) {
            size_t identifierLength = 2;
            token.push_back(createSingleToken(TokenType::KEYWORD_FUNC, s, i, identifierLength));
            i += identifierLength;
            continue;
        }

        if (isKeyword(s, i, "return")) {
            size_t identifierLength = 6;
            token.push_back(createSingleToken(TokenType::KEYWORD_RETURN, s, i, identifierLength));
            i += identifierLength;
            continue;
        }

        if (isKeyword(s, i, "if")) {
            size_t identifierLength = 2;
            token.push_back(createSingleToken(TokenType::KEYWORD_IF, s, i, identifierLength));
            i += identifierLength;
            continue;
        }





        

        if (startsIdentifier(c)) {
            //size_t l1 = countUntilCharacter(s, i, whiteSpace);
            //size_t l2 = countUntilCharacter(s, i, eol);
            //size_t identifierLength = minButNot0(l1, l2);

            size_t start = i;
            size_t end = i;
            while (continuesIdentifier(c)) {
                c = s[end];
                end++;
            }
            end--;

            std::string str = s.substr(start, end - start);
            TokenType::Type type;
            if (tryGetReservedBasicType(str, &type)) {
                Token t{
                    .type = type,
                    .span = TextSpan {
                        .string = str,
                        .from = start,
                        .to = end,
                    },
                    .fileName = "undefined", // TODO: Assign...
                };

                i += end - start - 1;
                token.push_back(t);
                continue;
            }

            Token t{
                .type = TokenType::IDENTIFIER,
                .span = TextSpan {
                    .string = str,
                    .from = start,
                    .to = end,
                },
                .fileName = "undefined", // TODO: Assign...
            };

            i += end - start - 1;
            token.push_back(t);
            continue;
        }


        if (isKeyword(s, i, "==")) {
            size_t identifierLength = 2;
            token.push_back(createSingleToken(TokenType::EQ_OP, s, i, identifierLength));
            i += identifierLength;
            continue;
        }

        if (isKeyword(s, i, "!=")) {
            size_t identifierLength = 2;
            token.push_back(createSingleToken(TokenType::NE_OP, s, i, identifierLength));
            i += identifierLength;
            continue;
        }


        if (c == '(') {
            token.push_back(createSingleToken(TokenType::LEFT_PAREN, s, i, 1));
            continue;
        }
        else if (c == ')') {
            token.push_back(createSingleToken(TokenType::RIGHT_PAREN, s, i, 1));
            continue;
        }
        else if (c == '{') {
            token.push_back(createSingleToken(TokenType::LEFT_CURLY, s, i, 1));
            continue;
        }
        else if (c == '}') {
            token.push_back(createSingleToken(TokenType::RIGHT_CURLY, s, i, 1));
            continue;
        }
        else if (c == '[') {
            token.push_back(createSingleToken(TokenType::LEFT_BRACKET, s, i, 1));
            continue;
        }
        else if (c == ']') {
            token.push_back(createSingleToken(TokenType::RIGHT_BRACKET, s, i, 1));
            continue;
        }
        else if (c == '+') {
            token.push_back(createSingleToken(TokenType::PLUS, s, i, 1));
            continue;
        }
        else if (c == '-') {
            token.push_back(createSingleToken(TokenType::MINUS, s, i, 1));
            continue;
        }
        else if (c == '=') {
            token.push_back(createSingleToken(TokenType::EQUALS, s, i, 1));
            continue;
        }
        else if (c == '<') {
            token.push_back(createSingleToken(TokenType::LEFT_ANGLE, s, i, 1));
            continue;
        }
        else if (c == '>') {
            token.push_back(createSingleToken(TokenType::RIGHT_ANGLE, s, i, 1));
            continue;
        }




        if (isDigit(c)) {
            size_t start = i;
            size_t end = i;
            while (continuesDigit(c)) {
                c = s[end];
                end++;
            }
            end--;

            std::string s2 = s.substr(i, end - start);

            TokenType::Type type = TokenType::INTEGER;
            if (s2.find(".") != -1) type = TokenType::FLOAT32;
            if (s2.ends_with("f32")) type = TokenType::FLOAT32;
            if (s2.ends_with("f64")) type = TokenType::FLOAT64;

            if (s2.ends_with("u8")) type = TokenType::INTEGER;
            if (s2.ends_with("u16")) type = TokenType::INTEGER;
            if (s2.ends_with("u32")) type = TokenType::INTEGER;
            if (s2.ends_with("u64")) type = TokenType::INTEGER;

            if (s2.ends_with("i8")) type = TokenType::INTEGER;
            if (s2.ends_with("i16")) type = TokenType::INTEGER;
            if (s2.ends_with("i32")) type = TokenType::INTEGER;
            if (s2.ends_with("i64")) type = TokenType::INTEGER;

            Token t{
                .type = type,
                .span = TextSpan {
                    .string = s2,
                    .from = start,
                    .to = end,
                },
                .fileName = "undefined", // TODO: Assign...
            };

            token.push_back(t);

            i += end - start - 1;
            continue;
        }

        // TODO: only ignore if we're not inside a string atm...
        if (isWhitespace(c)) {
            continue;
        }

        std::cout << "couldn't identify: " << c << "\n";
    }

    return token;
}
