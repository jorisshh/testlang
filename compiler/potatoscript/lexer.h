#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "types.h"

namespace lang::lexer {

	struct TextSpan {
		std::string string;
        size_t line;
		size_t from;
        size_t to;
	};

	namespace TokenType {
        enum Type {
            DOT = '.',
            COMMA = ',',
            EQUALS = '=',
            //SEMICOLON = ';',
            COLON = ':',
            LEFT_PAREN = '(',
            RIGHT_PAREN = ')',
            STAR = '*',
            SLASH = '/',
            PERCENT = '%',
            PLUS = '+',
            MINUS = '-',
            VERTICAL_BAR = '|', // bitwise Or
            CARET = '^', // bitwise Xor
            AMPERSAND = '&', // bitwise And

            LEFT_BRACKET = '[',
            RIGHT_BRACKET = ']',

            LEFT_CURLY = '{',
            RIGHT_CURLY = '}',

            LEFT_ANGLE = '<',
            RIGHT_ANGLE = '>',

            EXCLAMATION = '!',
            TILDE = '~',

            //TAG = '@',

            END = 256,
            INTEGER32,
            INTEGER64,
            FLOAT32,
            FLOAT64,
            IDENTIFIER,
            STRING,


            KEYWORD_FUNC,
            KEYWORD_VAR,
            KEYWORD_STRUCT,
            KEYWORD_ENUM,
            KEYWORD_OPERATOR,
            KEYWORD_EXTERN,

            KEYWORD_IF,
            //KEYWORD_WHEN,
            KEYWORD_ELSE,

            KEYWORD_WHILE,
            KEYWORD_BREAK,
            KEYWORD_CONTINUE,
            KEYWORD_FOR,
            KEYWORD_SWITCH,
            KEYWORD_CASE,
            KEYWORD_DEFAULT,

            KEYWORD_RETURN,

            KEYWORD_VOID,
            KEYWORD_STRING,
            //KEYWORD_INT,
            //KEYWORD_UINT,
            KEYWORD_UINT8,
            KEYWORD_UINT16,
            KEYWORD_UINT32,
            KEYWORD_UINT64,
            KEYWORD_INT8,
            KEYWORD_INT16,
            KEYWORD_INT32,
            KEYWORD_INT64,
            KEYWORD_FLOAT32,
            KEYWORD_FLOAT64,

            KEYWORD_BOOL,
            KEYWORD_TRUE,
            KEYWORD_FALSE,
            KEYWORD_NULL,

            // TODO: Re-enable these and allow casting, typeof(), strideof(), alignof()
            //KEYWORD_CAST,
            KEYWORD_SIZEOF,
            //KEYWORD_TYPEOF,
            //KEYWORD_STRIDEOF,
            //KEYWORD_ALIGNOF,

            TEMPORARY_KEYWORD_C_VARARGS = 400,

            GE_OP = 500,                // >=
            LE_OP,                // <=
            NE_OP,                // !=
            EQ_OP,                // ==
            AND_OP,               // &&
            XOR_OP,               // ^^
            OR_OP,                // ||
            //ARROW,                // ->
            DEREFERENCE_OR_SHIFT, // <<
            RIGHT_SHIFT,          // >>

            PLUS_EQ,              // +=
            MINUS_EQ,             // -=
            STAR_EQ,              // *=
            SLASH_EQ,             // /=
            PERCENT_EQ,           // %=
            AMPERSAND_EQ,         // &=
            VERTICAL_BAR_EQ,      // |=
            CARET_EQ,             // ^=

            //DOTDOT,               // ..
            //DOTDOTLT,             // ..<

            COMMENT = 600,
        };

        static const char* toString(Type t) {
            switch (t)
            {
            case TokenType::DOT: return "DOT";
            case TokenType::COMMA: return "COMMA";
            case TokenType::EQUALS: return "EQUALS";
            case TokenType::COLON: return "COLON";
            case TokenType::LEFT_PAREN: return "LEFT_PAREN";
            case TokenType::RIGHT_PAREN: return "RIGHT_PAREN";
            case TokenType::STAR: return "STAR";
            case TokenType::SLASH: return "SLASH";
            case TokenType::PERCENT: return "PERCENT";
            case TokenType::PLUS: return "PLUS";
            case TokenType::MINUS: return "MINUS";
            case TokenType::VERTICAL_BAR: return "VERTICAL_BAR";
            case TokenType::CARET: return "CARET";
            case TokenType::AMPERSAND: return "AMPERSAND";
            case TokenType::LEFT_BRACKET: return "LEFT_BRACKET";
            case TokenType::RIGHT_BRACKET: return "RIGHT_BRACKET";
            case TokenType::LEFT_CURLY: return "LEFT_CURLY";
            case TokenType::RIGHT_CURLY: return "RIGHT_CURLY";
            case TokenType::LEFT_ANGLE: return "LEFT_ANGLE";
            case TokenType::RIGHT_ANGLE: return "RIGHT_ANGLE";
            case TokenType::EXCLAMATION: return "EXCLAMATION";
            case TokenType::TILDE: return "TILDE";
            case TokenType::END: return "END";
            case TokenType::INTEGER32: return "INTEGER32";
            case TokenType::INTEGER64: return "INTEGER64";
            case TokenType::FLOAT32: return "FLOAT32";
            case TokenType::FLOAT64: return "FLOAT64";
            case TokenType::IDENTIFIER: return "IDENTIFIER";
            case TokenType::STRING: return "STRING";
            case TokenType::KEYWORD_FUNC: return "KEYWORD_FUNC";
            case TokenType::KEYWORD_VAR: return "KEYWORD_VAR";
            case TokenType::KEYWORD_STRUCT: return "KEYWORD_STRUCT";
            case TokenType::KEYWORD_ENUM: return "KEYWORD_ENUM";
            case TokenType::KEYWORD_OPERATOR: return "KEYWORD_OPERATOR";
            case TokenType::KEYWORD_IF: return "KEYWORD_IF";
            case TokenType::KEYWORD_ELSE: return "KEYWORD_ELSE";
            case TokenType::KEYWORD_WHILE: return "KEYWORD_WHILE";
            case TokenType::KEYWORD_BREAK: return "KEYWORD_BREAK";
            case TokenType::KEYWORD_CONTINUE: return "KEYWORD_CONTINUE";
            case TokenType::KEYWORD_FOR: return "KEYWORD_FOR";
            case TokenType::KEYWORD_SWITCH: return "KEYWORD_SWITCH";
            case TokenType::KEYWORD_CASE: return "KEYWORD_CASE";
            case TokenType::KEYWORD_DEFAULT: return "KEYWORD_DEFAULT";
            case TokenType::KEYWORD_RETURN: return "KEYWORD_RETURN";
            case TokenType::KEYWORD_VOID: return "KEYWORD_VOID";
            case TokenType::KEYWORD_STRING: return "KEYWORD_STRING";
            case TokenType::KEYWORD_UINT8: return "KEYWORD_UINT8";
            case TokenType::KEYWORD_UINT16: return "KEYWORD_UINT16";
            case TokenType::KEYWORD_UINT32: return "KEYWORD_UINT32";
            case TokenType::KEYWORD_UINT64: return "KEYWORD_UINT64";
            case TokenType::KEYWORD_INT8: return "KEYWORD_INT8";
            case TokenType::KEYWORD_INT16: return "KEYWORD_INT16";
            case TokenType::KEYWORD_INT32: return "KEYWORD_INT32";
            case TokenType::KEYWORD_INT64: return "KEYWORD_INT64";
            case TokenType::KEYWORD_FLOAT32: return "KEYWORD_FLOAT32";
            case TokenType::KEYWORD_FLOAT64: return "KEYWORD_FLOAT64";
            case TokenType::KEYWORD_BOOL: return "KEYWORD_BOOL";
            case TokenType::KEYWORD_TRUE: return "KEYWORD_TRUE";
            case TokenType::KEYWORD_FALSE: return "KEYWORD_FALSE";
            case TokenType::KEYWORD_NULL: return "KEYWORD_NULL";
            case TokenType::KEYWORD_SIZEOF: return "KEYWORD_SIZEOF";
            case TokenType::TEMPORARY_KEYWORD_C_VARARGS: return "TEMPORARY_KEYWORD_C_VARARGS";
            case TokenType::GE_OP: return "GE_OP";
            case TokenType::LE_OP: return "LE_OP";
            case TokenType::NE_OP: return "NE_OP";
            case TokenType::EQ_OP: return "EQ_OP";
            case TokenType::AND_OP: return "AND_OP";
            case TokenType::XOR_OP: return "XOR_OP";
            case TokenType::OR_OP: return "OR_OP";
            case TokenType::DEREFERENCE_OR_SHIFT: return "DEREFERENCE_OR_SHIFT";
            case TokenType::RIGHT_SHIFT: return "RIGHT_SHIFT";
            case TokenType::PLUS_EQ: return "PLUS_EQ";
            case TokenType::MINUS_EQ: return "MINUS_EQ";
            case TokenType::STAR_EQ: return "STAR_EQ";
            case TokenType::SLASH_EQ: return "SLASH_EQ";
            case TokenType::PERCENT_EQ: return "PERCENT_EQ";
            case TokenType::AMPERSAND_EQ: return "AMPERSAND_EQ";
            case TokenType::VERTICAL_BAR_EQ: return "VERTICAL_BAR_EQ";
            case TokenType::CARET_EQ: return "CARET_EQ";
            case TokenType::COMMENT: return "COMMENT";
            default:
                return "Undefined";
                break;
            }
        }

	}

	struct Token {
        TokenType::Type type;
		TextSpan span;
        std::string fileName;

	};

	std::vector<Token> parse(const std::string& contents) noexcept;

}