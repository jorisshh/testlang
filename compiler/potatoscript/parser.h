#pragma once

#include <vector>
#include <memory>

#include "types.h"
#include "lexer.h"

using namespace lang::lexer;

namespace lang::parser {

	class AstPrinter {
	public:
		size_t indentation = 0;
		std::string buffer;

		void print(const char* c) {
			for (size_t i = 0; i < indentation; i++) {
				buffer.append("\t");
			}

			buffer.append(c);
			buffer.append(" ");
		}

		void println(const char* c) {
			for (size_t i = 0; i < indentation; i++) {
				buffer.append("\t");
			}

			buffer.append(c);
			buffer.append("\n");
		}
	};


	class ExprAST {
	public:
		std::string type;

		ExprAST(const std::string& type) 
			: type{ type } 
		{

		}

		virtual ~ExprAST() {}

		virtual void print(AstPrinter& printer) = 0;

		virtual std::string prettyName() {
			return type;
		}
	};

	/// Expression class for numeric literals like "1.0".
	class NumberExprAST : public ExprAST {
		double float64Value;
		float float32Value;

		int32_t int32Value;
		int64_t int64Value;

		TokenType::Type type;

	public:
		NumberExprAST(float val) : ExprAST("Number"), float32Value(val), type(TokenType::FLOAT32) {}
		NumberExprAST(double val) : ExprAST("Number"), float64Value(val), type(TokenType::FLOAT64) {}
		NumberExprAST(int32_t val) : ExprAST("Number"), int32Value(val), type(TokenType::INTEGER32) {}
		NumberExprAST(int64_t val) : ExprAST("Number"), int64Value(val), type(TokenType::INTEGER64) {}

		virtual void print(AstPrinter& printer) override {
			switch (type)
			{
			case TokenType::FLOAT32:
				printer.print(std::to_string(float32Value).c_str());
				break;
			case TokenType::FLOAT64:
				printer.print(std::to_string(float64Value).c_str());
				break;
			case TokenType::INTEGER32:
				printer.print(std::to_string(int32Value).c_str());
				break;
			case TokenType::INTEGER64:
				printer.print(std::to_string(int64Value).c_str());
				break;
			default:
				break;
			}
		}
	};

	class ConstantStringExpr : public ExprAST {
		std::string stringValue;

	public:
		ConstantStringExpr(std::string val) : ExprAST("Number"), stringValue(val) {}

		virtual void print(AstPrinter& printer) override {
			printer.buffer += "\"";
			printer.buffer += stringValue.c_str();
			printer.buffer += "\" ";
		}
	};

	class ReturnAST : public ExprAST {
		ExprAST* value;

	public:
		ReturnAST(ExprAST* val) : ExprAST("Return"), value(val) {}

		virtual void print(AstPrinter& printer) override {
			if (value) {
				printer.print("return");
				value->print(printer);
			}
		}
	};

	class VariableExprAST : public ExprAST {
	public:
		std::string type;
		std::string name;
		
		ExprAST* assignment;

		VariableExprAST(const std::string& type, const std::string& name, ExprAST* assignment)
			: ExprAST("Variable"),
			type(type),
			name(name),
			assignment(assignment) {}

		virtual void print(AstPrinter& printer) override {
			//printer.print(type.c_str());
			printer.print(name.c_str());
			
			if (assignment) {
				assignment->print(printer);
			}
			// printer.print("(");
			// printer.print(type.c_str());
			// printer.print(")");
		}
	};

	class ArgumentListAST : public ExprAST {
	public:
		std::vector<ExprAST*> arguments;

		ArgumentListAST(std::vector<ExprAST*> arguments)
			: ExprAST("ArgumentList"),
			arguments(std::move(arguments)) {}


		virtual void print(AstPrinter& printer) override {
			printer.print("(");
			for (auto* a : arguments) {
				a->print(printer);
			}
			printer.print(")");
		}
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

		virtual void print(AstPrinter& printer) override {
			left->print(printer);
			printer.print(TokenType::toString(type));
			right->print(printer);
		}
	};

	/// CallExprAST - Expression class for function calls.
	class CallExprAST : public ExprAST {
		std::string callee;
		ArgumentListAST* args;

	public:
		CallExprAST(const std::string& callee, ArgumentListAST* args)
			: ExprAST("Function call"),
			callee(callee),
			args(args) {}

		virtual void print(AstPrinter& printer) override {
			printer.print(callee.c_str());
			args->print(printer);
		}
	};

	/// CodeBlockAST - Anything inside of {} is considered a code block
	class CodeBlockAST : public ExprAST {
		std::vector<ExprAST*> body;

	public:
		CodeBlockAST(std::vector<ExprAST*> body)
			: ExprAST("CodeBlock"),
			body(body) {}

		virtual void print(AstPrinter& printer) override {
			printer.print("{");
			for (auto* a : body) {
				a->print(printer);
			}
			printer.print("}");
		}
	};

	/// PrototypeAST - This class represents the "prototype" for a function,
	/// which captures its name, and its argument names (thus implicitly the number
	/// of arguments the function takes).
	class FunctionSignatureAST {
	public:
		std::string name;
		ArgumentListAST* args;
		ArgumentListAST* returnList;
		// TODO: Add return list?

		FunctionSignatureAST(const std::string& name, ArgumentListAST* args, ArgumentListAST* returnList)
			: name(name),
			args(args),
			returnList(returnList) {}

		const std::string& getName() const { return name; }
	};

	/// FunctionAST - This class represents a function definition itself.
	class FunctionAST : public ExprAST {
		FunctionSignatureAST* proto;
		CodeBlockAST* body;

	public:
		FunctionAST(FunctionSignatureAST* proto, CodeBlockAST* body)
			: ExprAST("Function"),
			proto(proto),
			body(body) {}

		virtual void print(AstPrinter& printer) override {
			printer.print(proto->getName().c_str());
			printer.print("(");
			if (proto->args) {
				proto->args->print(printer);
			}
			printer.print(")");

			if (proto->returnList) {
				printer.print("(");
				proto->returnList->print(printer);
				printer.print(")");
			}

			printer.print("{");
			printer.indentation++;

			if (body) {
				printer.println("");
				body->print(printer);
				printer.println("");
			}

			printer.indentation--;
			printer.print("}");
		}
	};

	class IfAST : public ExprAST {
	public:
		
		struct ConditionAndBody {
			BinaryExprAST* condition;
			CodeBlockAST* body;

			ConditionAndBody(BinaryExprAST* condition, CodeBlockAST* body)
				: condition(condition),
				body(body) {}
		};

		// [0] = first if
		// [1] = is else if
		// [2] = is next else if... etc
		std::vector<ConditionAndBody> chain;
		bool hasElseAtEnd;
		CodeBlockAST* elseBody;

	public:
		IfAST(std::vector<ConditionAndBody> condition, bool hasElseAtEnd, CodeBlockAST* elseBody)
			: ExprAST("If"),
			chain(condition),
			hasElseAtEnd(hasElseAtEnd),
			elseBody(elseBody)
		{
		
		}

		virtual void print(AstPrinter& printer) override {
			for (auto& a : chain) {
				printer.print("if");
				a.condition->print(printer);

				if (a.body) {
					printer.println("{");
					printer.indentation++;
					a.body->print(printer);
					printer.indentation--;
					printer.println("");
					printer.println("}");
				}
			}

			if (hasElseAtEnd) {
				printer.print("else");

				if (elseBody) {
					printer.println("{");
					printer.indentation++;
					elseBody->print(printer);
					printer.indentation--;
					printer.println("}");
				}
			}
		}
	};

	class ParserHelper {
	public:
		std::vector<lang::lexer::Token> tokens;
		std::vector<ExprAST*> astNodes; // Root nodes (e.g. if with all child nodes)
		std::vector<ExprAST*> astNodesFlat; // Flattened representation where all nodes are just added one after another.

		size_t index;

		// +1 for each {
		// -1 for each }
		size_t scopeDepth;

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


	ExprAST* identifier();
	BinaryExprAST* binaryExpression(TokenType::Type terminator);
	ArgumentListAST* argumentsDefinitionList(TokenType::Type terminator);
	ArgumentListAST* argumentsList(TokenType::Type terminator);
	ExprAST* expression();
	CodeBlockAST* codeBlock();

	std::vector<ExprAST*> parse(const std::vector<lang::lexer::Token>& tokens);
}