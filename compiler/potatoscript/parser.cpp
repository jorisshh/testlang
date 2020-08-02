#include "parser.h"

#include <iostream>

using namespace lang::lexer;
using namespace lang::parser;

//void assert(bool b, const char* msg = nullptr) {
//	if (b == false) {
//		if (msg) {
//			std::cerr << msg;
//		}
//
//		std::abort();
//		return;
//	}
//}

ParserHelper p;


static bool isOperator(TokenType::Type t) {
	return (t >= 500 && t < 600) || t == TokenType::LEFT_ANGLE || t == TokenType::RIGHT_ANGLE;
}

static bool isIdentifier(TokenType::Type t) {
	return t == TokenType::IDENTIFIER;
}

static bool isConstant(TokenType::Type t) {
	return t == TokenType::INTEGER ||
		t == TokenType::FLOAT32 ||
		t == TokenType::FLOAT64 ||
		t == TokenType::STRING;
}

void assert(bool b, const Token& token, const char* msg) {
	if (b == false) {
		std::printf(msg, token.span.line, token.span.from, token.span.to);
		//std::cerr << buff << '\n';
		std::abort();
	}
}

void error(bool b, const Token& token, const char* msg) {
	if (b) {
		std::printf(msg, token.span.line, token.span.from, token.span.to);
		//std::cerr << buff << '\n';
		std::abort();
	}
}

ExprAST* identifier() {

	const Token& current = p.current();
	p.eat();

	const Token& next = p.current();
	if(isConstant(current.type))
	{
		return new NumberExprAST(0.0); // TODO: Make some constant AST node
	}
	else {
		// Regular indentifier like a variable
		return new VariableExprAST(current.span.string, "");
	}
}


//std::unique_ptr<IfAST> ifAst() {
//	std::abort(); // TODO;
//}

ExprAST* expression() {

	switch (p.current().type)
	{
	/*case TokenType::KEYWORD_IF: {
		return ifAst ();
	}*/
	case TokenType::IDENTIFIER: {
		return identifier();
	}

	default:
		break;
	}

	if (isConstant(p.current().type)) {
		return identifier();
	}


	//auto& next = p.next();
	//assert(next.type == TokenType::IDENTIFIER || isConstant(next.type), next, "Expected identifier at line: %d and token: %d - %d");
	//identifier();

	//auto& next2 = tokens[index + 1];
	//if (isOperator(next2.type)) {
	//	index++;
	//}

	//auto& next3 = tokens[index + 1];
	//assert(next3.type == TokenType::IDENTIFIER || isConstant(next3.type), next3, "Expected identifier at line: %d and token: %d - %d");
	//identifier(tokens, index + 1);

	return nullptr;
}

//void ifKeyword() {
//	
//	p.eat(); // eat if
//
//	size_t counter = 0;
//	for (size_t i = p.index; i < p.tokens.size(); i++) {
//		if (p.tokens[i].type == TokenType::LEFT_CURLY) {
//			break;
//		}
//
//		counter++;
//	}
//
//	std::cout << "if has " << counter << " tokens";
//
//	auto& current = p.current();
//	p.eat(); // Eat current
//
//	auto* node = expression();
//
//
//
//	auto& next = p.next();
//	
//
//	expression();
//
//	error(next.type != TokenType::LEFT_CURLY, next, "Expected identifier at line: %d and token: %d - %d");
//}

BinaryExprAST* binaryExpression(TokenType::Type terminator) {

	auto& current = p.current();
	assert(isIdentifier(current.type) || isConstant(current.type), current, "");
	assert(isOperator(p.next().type), p.next(), "");
	assert(isIdentifier(p.next(2).type) || isConstant(p.next(2).type), p.next(2), "");

	ExprAST* left = nullptr;
	ExprAST* right = nullptr;

	left = expression();
	auto& op = p.current();
	p.eat(); // eat Op
	right = expression();

	return new BinaryExprAST(op.type, left, right);
}

std::vector<ExprAST*> argumentsList(TokenType::Type terminator) {
	std::vector<ExprAST*> args;
	while (p.current().type != terminator) {
		args.push_back(new VariableExprAST(p.current(true).span.string, p.current(true).span.string));
	}
	
	return args;
}

std::vector<ExprAST*> lang::parser::parse(const std::vector<Token>& tokens)
{
	std::vector<ExprAST*> astNodes;
	p.tokens = std::move(tokens);
	p.index = 0;

	size_t i = 0;
	while (p.index < p.tokens.size()) {
		auto& current = p.current();
		switch (current.type)
		{
		case TokenType::LEFT_PAREN: {
			p.eat(); // eat (
			auto args = argumentsList(TokenType::RIGHT_PAREN);
			p.eat(); // eat )

			for (auto& arg : args) {
				astNodes.push_back(arg);
			}

			break;
		}
		case TokenType::KEYWORD_IF: {
			p.eat(); // eat if
			
			auto* t2 = binaryExpression(TokenType::LEFT_CURLY);

			std::vector<IfAST::ConditionAndBody> v;
			v.push_back(IfAST::ConditionAndBody(t2, nullptr)); // TODO: Assign if body

			auto* f = new IfAST(v, false, nullptr);
			astNodes.push_back(f);

			break;
		}
		case TokenType::KEYWORD_FUNC: {
			p.eat(); // eat func
			
			// Arguments list
			auto* t2 = identifier();
			astNodes.push_back(t2);

			// Return list
			auto* t3 = identifier();
			astNodes.push_back(t3);

			break;
		}
		case TokenType::IDENTIFIER: {
			auto* t = expression();
			astNodes.push_back(t);
			break;
		}

		default:
			std::cerr << "couldn't find logic for token type:: " << TokenType::toString(current.type) << "\n";
			p.eat();
			break;
		}
	}

	return std::move(astNodes);
}
