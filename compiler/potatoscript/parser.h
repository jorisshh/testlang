#pragma once

#include <vector>
#include <memory>

#include "types.h"
#include "lexer.h"

using namespace lang::lexer;

namespace lang::parser {

	class ExprAST {
	public:
		std::string type;

		ExprAST(const std::string& type) 
			: type{ type } 
		{

		}

		virtual ~ExprAST() {}

		virtual std::string prettyName() {
			return type;
		}
	};

	/// Expression class for numeric literals like "1.0".
	class NumberExprAST : public ExprAST {
		double val;

	public:
		NumberExprAST(double val) : ExprAST("Number"), val(val) {}
	};

	class VariableExprAST : public ExprAST {
	public:
		std::string name;
		std::string type;

		VariableExprAST(const std::string& name, const std::string& type) 
			: ExprAST("Variable"),
			name(name),
			type(type) {}
	};

	/// BinaryExprAST - Expression class for a binary operator.
	class BinaryExprAST : public ExprAST {
	public:
		TokenType::Type type;
		ExprAST* left;
		ExprAST* right;

		BinaryExprAST(TokenType::Type type, ExprAST* left, ExprAST* right)
			: ExprAST("Binary expression"), 
			type(type),
			left(left),
			right(right) {}
	};

	/// CallExprAST - Expression class for function calls.
	class CallExprAST : public ExprAST {
		std::string callee;
		std::vector<ExprAST*> args;

	public:
		CallExprAST(const std::string& callee, std::vector<ExprAST*> args)
			: ExprAST("Function call"),
			callee(callee),
			args(std::move(args)) {}
	};

	/// PrototypeAST - This class represents the "prototype" for a function,
	/// which captures its name, and its argument names (thus implicitly the number
	/// of arguments the function takes).
	class FunctionDefAST {
		std::string name;
		std::vector<std::string> args;

	public:
		FunctionDefAST(const std::string& name, std::vector<std::string> args)
			: name(name),
			args(std::move(args)) {}

		const std::string& getName() const { return name; }
	};

	/// FunctionAST - This class represents a function definition itself.
	class FunctionAST {
		FunctionDefAST* proto;
		ExprAST* body;

	public:
		FunctionAST(FunctionDefAST* proto, ExprAST* body)
			: proto(proto),
			body(body) {}
	};

	class IfAST : public ExprAST {
	public:
		
		struct ConditionAndBody {
			BinaryExprAST* condition;
			ExprAST* body;

			ConditionAndBody(BinaryExprAST* condition, ExprAST* body)
				: condition(condition),
				body(body) {}
		};

		// [0] = first if
		// [1] = is else if
		// [2] = is next else if... etc
		std::vector<ConditionAndBody> condition;
		bool hasElseAtEnd;
		ExprAST* elseBody;

	public:
		IfAST(std::vector<ConditionAndBody> condition, bool hasElseAtEnd, ExprAST* elseBody)
			: ExprAST("If"),
			condition(condition),
			hasElseAtEnd(hasElseAtEnd),
			elseBody(elseBody)
		{
		
		}
	};

	class ParserHelper {
	public:
		std::vector<lang::lexer::Token> tokens;
		size_t index;

		void eat(size_t count = 1) {
			index += count;
		}

		const lang::lexer::Token& next(size_t forward = 1, bool eat = false) {
			auto& t = tokens[index + forward];
			if (eat) {
				this->eat(forward);
			}
			return t;
		}

		const lang::lexer::Token& prev(size_t back = 1) {
			return tokens[index - back];
		}

		const lang::lexer::Token& current(bool eat = false) {
			auto& t = tokens[index];
			if (eat) {
				this->eat(1);
			}
			return t;
		}
	};


	std::vector<ExprAST*> parse(const std::vector<lang::lexer::Token>& tokens);
}