#include "parser.h"

#include <iostream>
#include "parser.h"

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

template <typename T, class ...Args>
static T* createAst(Args&&... args) {
	auto* a = new T(std::forward<Args>(args)...);
	p.astNodesFlat.push_back((ExprAST*)a);
	return a;
}


static bool isOperator(TokenType::Type t) {
	return (t >= 500 && t < 600) || t == TokenType::LEFT_ANGLE || t == TokenType::RIGHT_ANGLE
		|| t == TokenType::STAR || t == TokenType::SLASH
		|| t == TokenType::PERCENT || t == TokenType::PLUS
		|| t == TokenType::MINUS || t == TokenType::VERTICAL_BAR
		|| t == TokenType::CARET || t == TokenType::AMPERSAND;
}

static bool isIdentifier(TokenType::Type t) {
	return t == TokenType::IDENTIFIER;
}

static bool isConstant(TokenType::Type t) {
	return t == TokenType::INTEGER32 ||
		t == TokenType::INTEGER64 ||
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

ExprAST* lang::parser::identifier() {

	const Token& current = p.current();
	p.eat();

	const Token& next = p.current();
	if(isConstant(current.type))
	{
		switch (current.type)
		{
		case TokenType::FLOAT32:
			//return createAst<NumberExprAST>(std::stof(current.span.string)); // TODO...
			return createAst<NumberExprAST>(1.0f);
		case TokenType::FLOAT64:
			//return createAst<NumberExprAST>(std::stof(current.span.string)); // TODO...
			return createAst<NumberExprAST>(1.0f);
		case TokenType::INTEGER32:
			return createAst<NumberExprAST>(static_cast<int32_t>(std::stoi(current.span.string)));
		case TokenType::INTEGER64:
			return createAst<NumberExprAST>(static_cast<int64_t>(std::stoi(current.span.string)));
		case TokenType::STRING:
			return createAst<ConstantStringExpr>(current.span.string);
		default:
			std::abort();
		}
	}
	else if (next.type == TokenType::LEFT_PAREN) {
		return createAst<CallExprAST>(current.span.string, argumentsList(TokenType::RIGHT_PAREN));
	}
	else {
		// Regular indentifier like a variable
		ExprAST* assignment = nullptr;
		if (next.type == TokenType::EQUALS) {
			p.eat(); // eat = 
			assignment = expression();
		}

		return createAst<VariableExprAST>(current.span.string, current.span.string, assignment);
	}
}


BinaryExprAST* lang::parser::binaryExpression(TokenType::Type terminator) {

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

	return createAst<BinaryExprAST>(op.type, left, right);
}

VariableExprAST* variableExpr() {
	auto& type = p.current(true);
	auto& name = p.current(true);

	ExprAST* assignment = nullptr;
	if (p.next().type == TokenType::EQUALS) {
		p.eat(); // eat = 
		assignment = expression();
	}

   	return createAst<VariableExprAST>(type.span.string, name.span.string, assignment);
}

ArgumentListAST* lang::parser::argumentsDefinitionList(TokenType::Type terminator) {
	std::vector<ExprAST*> args;

	assert(p.current().type == TokenType::LEFT_PAREN, p.current(), "Expected (");
	p.eat(); // Eat "("

	while (p.current().type != terminator) {

		args.push_back(variableExpr());

		if (p.current().type == TokenType::COMMA) {
			p.eat();
			continue;
		}
		else if (p.current().type == terminator) {
			continue;
		}

		std::abort();
	}

	assert(p.current().type == TokenType::RIGHT_PAREN, p.current(), "Expected )");
	p.eat(); // Eat ")"

	return createAst<ArgumentListAST>(args);
}

ArgumentListAST* lang::parser::argumentsList(TokenType::Type terminator) {
	std::vector<ExprAST*> args;

	assert(p.current().type == TokenType::LEFT_PAREN, p.current(), "Expected (");
	p.eat(); // Eat "("

	while (p.current().type != terminator) {
		args.push_back(expression());

		if (p.current().type == TokenType::COMMA) {
			p.eat();
			continue;
		}
		else if (p.current().type == terminator) {
			continue;
		}

		std::abort();
		//assert(p.current().type == terminator || p.current().type == TokenType::COMMA, p.current(), "Expected ) or ,");
	}

	assert(p.current().type == TokenType::RIGHT_PAREN, p.current(), "Expected )");
	p.eat(); // Eat ")"

	return createAst<ArgumentListAST>(args);
}

CodeBlockAST* lang::parser::codeBlock()
{
	size_t scopeDepthBefore = p.scopeDepth;
	assert(p.current().type == TokenType::LEFT_CURLY, p.current(), "Expected {");
	p.eat();

	p.scopeDepth++;

	std::vector<ExprAST*> codeBlock;
	while (p.scopeDepth != scopeDepthBefore) {
		auto* e = expression();
		codeBlock.push_back(e);

		if (p.current().type == TokenType::RIGHT_CURLY) {
			p.scopeDepth--;
			p.eat();
		}
	}

	//assert(p.current().type == TokenType::RIGHT_CURLY, p.current(), "Expected }");
	//p.eat();

	return createAst<CodeBlockAST>(codeBlock);
}


ExprAST* lang::parser::expression() {

	if (p.index >= p.tokens.size()) {
		throw new std::exception("escape recursion");
		//std::abort();
	}

	switch (p.current().type)
	{
	case TokenType::IDENTIFIER: {
		return identifier();
	}
	case TokenType::KEYWORD_FLOAT32:
	case TokenType::KEYWORD_FLOAT64:
	case TokenType::KEYWORD_UINT8:
	case TokenType::KEYWORD_UINT16:
	case TokenType::KEYWORD_UINT32:
	case TokenType::KEYWORD_UINT64:
	case TokenType::KEYWORD_INT8:
	case TokenType::KEYWORD_INT16:
	case TokenType::KEYWORD_INT32:
	case TokenType::KEYWORD_INT64:
	case TokenType::KEYWORD_BOOL:
	case TokenType::KEYWORD_STRING:
		return variableExpr();
	case TokenType::COMMENT:
		p.eat();
		return expression();
	case TokenType::KEYWORD_RETURN: {

		ExprAST* value = nullptr;
		if (p.current().span.line == p.next().span.line) {
			// There's a return value
			p.eat(); // Eat return keyword
			value = expression();
		}
		else {
			p.eat(); // Eat return keyword
		}

		return createAst<ReturnAST>(value);
	}
	case TokenType::EQUALS:
	case TokenType::PLUS:
	case TokenType::MINUS:
	case TokenType::STAR:
	case TokenType::SLASH:
	case TokenType::PERCENT:
	case TokenType::VERTICAL_BAR:
	case TokenType::CARET:
	case TokenType::AMPERSAND:
	{
		auto* left = p.astNodesFlat.back();
		
		if (p.astNodes.size() > 0) {
			p.astNodes.resize(p.astNodes.size() - 1);
		}
		p.astNodesFlat.resize(p.astNodesFlat.size() - 1);

		auto& op = p.current(true);
		auto* right = expression();

		return createAst<BinaryExprAST>(op.type, left, right);
	}
	case TokenType::KEYWORD_IF: {
		p.eat(); // eat if

		auto* condition = binaryExpression(TokenType::LEFT_CURLY);
		std::vector<IfAST::ConditionAndBody> v;

		auto* body = codeBlock();
		v.push_back(IfAST::ConditionAndBody(condition, body));
		return createAst<IfAST>(v, false, nullptr);
	}
	case TokenType::KEYWORD_FUNC: {
		p.eat(); // eat func
		auto& name = p.current();
		p.eat();

		assert(p.current().type == TokenType::LEFT_PAREN, p.current(), "Expected (");
		auto args = argumentsDefinitionList(TokenType::RIGHT_PAREN);
		
		ArgumentListAST* returnList = nullptr;
		if (p.current().type == TokenType::LEFT_PAREN) {
			returnList = argumentsDefinitionList(TokenType::RIGHT_PAREN);
		}
		
		if (p.current().type != TokenType::LEFT_CURLY) {
			auto* iden = identifier();
			std::vector<ExprAST*> arguments;
			arguments.push_back(iden);
			returnList = createAst<ArgumentListAST>(arguments);
		}
		
		auto def = createAst<FunctionSignatureAST>(name.span.string, args, returnList);
		auto* body = codeBlock();

		return createAst<FunctionAST>(def, body);
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

	std::cerr << "undefined token type:: " << TokenType::toString(p.current().type) << "\n";
	p.eat();

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

std::vector<ExprAST*> lang::parser::parse(const std::vector<Token>& tokens)
{
	p.tokens = std::move(tokens);
	p.index = 0;

	try {
		size_t i = 0;
		while (p.index < p.tokens.size()) {
			auto* t = expression();
			p.astNodes.push_back(t);
		}
	}
	catch (std::exception* e) {
		return std::move(p.astNodes);
	}
	
	return std::move(p.astNodes);
}


