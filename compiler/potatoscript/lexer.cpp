#include "lexer.h"

#include <iostream>

using namespace lang::lexer;

static bool isWhitespace(char c) {
    return c == ' ' || c == '\t'; //  || c == '\v' ( https://en.wikipedia.org/wiki/Tab_key )
}

static bool isLineBreak(char c) {
    return c == '\n' || c == '\r';
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

static bool continuesDigit(char c, int baseCount = 10) {

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

static Token createSingleToken(TokenType::Type t, const std::string& s, size_t lineNumber, size_t index, size_t length) {
    Token res{
        .type = t,
        .span = TextSpan {
            .string = s.substr(index, length),
            .line = lineNumber,
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
using namespace lang::lexer;

std::vector<Token> lang::lexer::parse(const std::string& s) noexcept
{
    size_t lineNumber = 0;

    std::vector<Token> token;
    size_t i = 0;

    while(i < s.size()) {
    //for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];

        if (isLineBreak(c)) {
            lineNumber++;
            i++;
            continue;
        }

        // TODO: only ignore if we're not inside a string atm...
        if (isWhitespace(c)) {
            i++;
            continue;
        }

        if (isCommentStart(s, i)) {
            size_t commentLength = countUntilCharacter(s, i, eol);
            if (commentLength < 2) {
                commentLength = s.size() - i; // Go to end of the file
            }

            std::string s2 = s.substr(i, commentLength);
            Token t{
                .type = TokenType::COMMENT,
                .span = TextSpan {
                    .string = s2,
                    .line = lineNumber,
                    .from = i,
                    .to = i + commentLength,
                },
                .fileName = "undefined", // TODO: Assign...
            };

            token.push_back(t);

            i += commentLength;
            lineNumber++;
            continue;
        }

        if (c == '"') {
            size_t stringLength = countUntilCharacter(s, i + 1, '"');
            i += 1; // + 1 for "
            token.push_back(createSingleToken(TokenType::STRING, s, lineNumber, i, stringLength));
            i += 1; // + 1 for "
            i += stringLength;
            continue;
        }

        if (isKeyword(s, i, "fn")) {
            size_t identifierLength = 2;
            token.push_back(createSingleToken(TokenType::KEYWORD_FUNC, s, lineNumber, i, identifierLength));
            i += identifierLength;
            continue;
        }

        if (isKeyword(s, i, "return")) {
            size_t identifierLength = 6;
            token.push_back(createSingleToken(TokenType::KEYWORD_RETURN, s, lineNumber, i, identifierLength));
            i += identifierLength;
            continue;
        }

        if (isKeyword(s, i, "if")) {
            size_t identifierLength = 2;
            token.push_back(createSingleToken(TokenType::KEYWORD_IF, s, lineNumber, i, identifierLength));
            i += identifierLength;
            continue;
        }

        if (isKeyword(s, i, "extern")) {
            size_t identifierLength = 6;
            token.push_back(createSingleToken(TokenType::KEYWORD_EXTERN, s, lineNumber, i, identifierLength));
            i += identifierLength;
            continue;
        }

        if (isKeyword(s, i, "true")) {
            size_t identifierLength = 4;
            token.push_back(createSingleToken(TokenType::KEYWORD_TRUE, s, lineNumber, i, identifierLength));
            i += identifierLength;
            continue;
        }

        if (isKeyword(s, i, "false")) {
            size_t identifierLength = 5;
            token.push_back(createSingleToken(TokenType::KEYWORD_FALSE, s, lineNumber, i, identifierLength));
            i += identifierLength;
            continue;
        }



        

        if (startsIdentifier(c)) {
            //size_t l1 = countUntilCharacter(s, i, whiteSpace);
            //size_t l2 = countUntilCharacter(s, i, eol);
            //size_t identifierLength = minButNot0(l1, l2);

            size_t length = 0;
            while (continuesIdentifier(s[i + length])) {
                length++;
            }

            std::string str = s.substr(i, length);
            TokenType::Type type;
            if (tryGetReservedBasicType(str, &type)) {
                Token t{
                    .type = type,
                    .span = TextSpan {
                        .string = str,
                        .line = lineNumber,
                        .from = i,
                        .to = i + length,
                    },
                    .fileName = "undefined", // TODO: Assign...
                };

                i += length;
                token.push_back(t);
                continue;
            }

            Token t{
                .type = TokenType::IDENTIFIER,
                .span = TextSpan {
                    .string = str,
                    .line = lineNumber,
                    .from = i,
                    .to = i + length,
                },
                .fileName = "undefined", // TODO: Assign...
            };

            i += length;
            token.push_back(t);
            continue;
        }


        if (isKeyword(s, i, "==")) {
            size_t identifierLength = 2;
            token.push_back(createSingleToken(TokenType::EQ_OP, s, lineNumber, i, identifierLength));
            i += identifierLength;
            continue;
        }

        if (isKeyword(s, i, "!=")) {
            size_t identifierLength = 2;
            token.push_back(createSingleToken(TokenType::NE_OP, s, lineNumber, i, identifierLength));
            i += identifierLength;
            continue;
        }

        if (c == ',') {
            token.push_back(createSingleToken(TokenType::COMMA, s, lineNumber, i, 1));
            i++;
            continue;
        }

        if (c == '(') {
            token.push_back(createSingleToken(TokenType::LEFT_PAREN, s, lineNumber, i, 1));
            i++;
            continue;
        }
        else if (c == ')') {
            token.push_back(createSingleToken(TokenType::RIGHT_PAREN, s, lineNumber, i, 1));
            i++;
            continue;
        }
        else if (c == '{') {
            token.push_back(createSingleToken(TokenType::LEFT_CURLY, s, lineNumber, i, 1));
            i++;
            continue;
        }
        else if (c == '}') {
            token.push_back(createSingleToken(TokenType::RIGHT_CURLY, s, lineNumber, i, 1));
            i++;
            continue;
        }
        else if (c == '[') {
            token.push_back(createSingleToken(TokenType::LEFT_BRACKET, s, lineNumber, i, 1));
            i++;
            continue;
        }
        else if (c == ']') {
            token.push_back(createSingleToken(TokenType::RIGHT_BRACKET, s, lineNumber, i, 1));
            i++;
            continue;
        }
        else if (c == '+') {
            token.push_back(createSingleToken(TokenType::PLUS, s, lineNumber, i, 1));
            i++;
            continue;
        }
        else if (c == '-') {
            token.push_back(createSingleToken(TokenType::MINUS, s, lineNumber, i, 1));
            i++;
            continue;
        }
        else if (c == '=') {
            token.push_back(createSingleToken(TokenType::EQUALS, s, lineNumber, i, 1));
            i++;
            continue;
        }
        else if (c == '<') {
            token.push_back(createSingleToken(TokenType::LEFT_ANGLE, s, lineNumber, i, 1));
            i++;
            continue;
        }
        else if (c == '>') {
            token.push_back(createSingleToken(TokenType::RIGHT_ANGLE, s, lineNumber, i, 1));
            i++;
            continue;
        }




        if (isDigit(c)) {
            size_t length = 0;
            while (continuesDigit(s[i + length])) {
                length++;
            }

            std::string s2 = s.substr(i, length);

            TokenType::Type type = TokenType::INTEGER32;
            if (s2.find(".") != -1) type = TokenType::FLOAT32;
            if (s2.ends_with("f32")) type = TokenType::FLOAT32;
            if (s2.ends_with("f64")) type = TokenType::FLOAT64;

            if (s2.ends_with("u8")) type = TokenType::INTEGER32;
            if (s2.ends_with("u16")) type = TokenType::INTEGER32;
            if (s2.ends_with("u32")) type = TokenType::INTEGER32;
            if (s2.ends_with("u64")) type = TokenType::INTEGER64;

            if (s2.ends_with("i8")) type = TokenType::INTEGER32;
            if (s2.ends_with("i16")) type = TokenType::INTEGER32;
            if (s2.ends_with("i32")) type = TokenType::INTEGER32;
            if (s2.ends_with("i64")) type = TokenType::INTEGER64;

            Token t{
                .type = type,
                .span = TextSpan {
                    .string = s2,
                    .line = lineNumber,
                    .from = i,
                    .to = i + length,
                },
                .fileName = "undefined", // TODO: Assign...
            };

            token.push_back(t);

            i += length;
            continue;
        }

        i++;
        std::cout << "couldn't identify: " << c << " (ignoring)" << "\n";
    }

    return std::move(token);
}
