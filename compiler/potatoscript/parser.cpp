#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING 1

#include "parser.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/APSint.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include <iostream>
#include "parser.h"


using namespace lang::lexer;
using namespace lang::parser;

// Code generation
static llvm::LLVMContext llvmContext;
static llvm::IRBuilder<> llvmBuilder(llvmContext);
static std::unique_ptr<llvm::Module> llvmModule;
static std::map<std::string, llvm::Value*> llvmNamedValues;

static ParserHelper p;

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

void assert2(bool b, const Token& token, const char* msg) {
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
	assert2(isIdentifier(current.type) || isConstant(current.type), current, "");
	assert2(isOperator(p.next().type), p.next(), "");
	assert2(isIdentifier(p.next(2).type) || isConstant(p.next(2).type), p.next(2), "");

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

	assert2(p.current().type == TokenType::LEFT_PAREN, p.current(), "Expected (");
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

	assert2(p.current().type == TokenType::RIGHT_PAREN, p.current(), "Expected )");
	p.eat(); // Eat ")"

	return createAst<ArgumentListAST>(args);
}

// The argumengs you pass into a function, e.g. variable list
ArgumentListAST* lang::parser::argumentsList(TokenType::Type terminator) {
	std::vector<ExprAST*> args;

	assert2(p.current().type == TokenType::LEFT_PAREN, p.current(), "Expected (");
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
		//assert2(p.current().type == terminator || p.current().type == TokenType::COMMA, p.current(), "Expected ) or ,");
	}

	assert2(p.current().type == TokenType::RIGHT_PAREN, p.current(), "Expected )");
	p.eat(); // Eat ")"

	return createAst<ArgumentListAST>(args);
}

CodeBlockAST* lang::parser::codeBlock()
{
	size_t scopeDepthBefore = p.scopeDepth;
	assert2(p.current().type == TokenType::LEFT_CURLY, p.current(), "Expected {");
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

	//assert2(p.current().type == TokenType::RIGHT_CURLY, p.current(), "Expected }");
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

		assert2(p.current().type == TokenType::LEFT_PAREN, p.current(), "Expected (");
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
	//assert2(next.type == TokenType::IDENTIFIER || isConstant(next.type), next, "Expected identifier at line: %d and token: %d - %d");
	//identifier();

	//auto& next2 = tokens[index + 1];
	//if (isOperator(next2.type)) {
	//	index++;
	//}

	//auto& next3 = tokens[index + 1];
	//assert2(next3.type == TokenType::IDENTIFIER || isConstant(next3.type), next3, "Expected identifier at line: %d and token: %d - %d");
	//identifier(tokens, index + 1);

	std::cerr << "undefined token type:: " << TokenType::toString(p.current().type) << "\n";
	p.eat();

	return nullptr;
}

/// LogError* - These are little helper functions for error handling.
ExprAST* LogError(const char* str) {
	fprintf(stderr, "Error: %s\n", str);
	return nullptr;
}

llvm::Value* LogErrorV(const char* str) {
	LogError(str);
	return nullptr;
}

std::vector<ExprAST*> lang::parser::parse(const std::vector<Token>& tokens)
{
	llvmModule = std::make_unique<llvm::Module>("potatoscript", llvmContext);

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
		// Ignored (used to escape parsing, kind of ugly...)
	}
	
	for (auto* n : p.astNodes) {
		if (n == nullptr) continue;

		auto* r = n->codegen();
		r->print(llvm::errs());
	}

	llvmModule->print(llvm::errs(), nullptr);
	return std::move(p.astNodes);
}

llvm::Value* lang::parser::NumberExprAST::codegen()
{
	switch (type) {
	case TokenType::FLOAT32: return llvm::ConstantFP::get(llvmContext, llvm::APFloat(value.float32Value));
	case TokenType::FLOAT64: return llvm::ConstantFP::get(llvmContext, llvm::APFloat(value.float64Value));
	case TokenType::INTEGER32: return llvm::ConstantInt::get(llvmContext, llvm::APInt(32, value.int32Value, true));
	case TokenType::INTEGER64: return llvm::ConstantInt::get(llvmContext, llvm::APInt(64, value.int64Value, true));
	// TODO: Add unsigned integers
	}
	
	return LogErrorV("Value type not found");
}

llvm::Value* lang::parser::ConstantStringExpr::codegen()
{
	return LogErrorV("Not imlemented");
}

llvm::Value* lang::parser::ReturnAST::codegen()
{
	auto* v = value->codegen();
	return llvm::ReturnInst::Create(llvmContext, v);
}

llvm::Value* lang::parser::VariableExprAST::codegen()
{
	llvm::Value* v = llvmNamedValues.at(name);
	assert(v != nullptr);

	return v;
}

llvm::Value* lang::parser::ArgumentListAST::codegen()
{
	return LogErrorV("Not imlemented");

	//for (auto* t : arguments) {
	//	llvmNamedValues[t->name] = t->codegen();
	//}

}

llvm::Value* lang::parser::BinaryExprAST::codegen()
{
	llvm::Value* l = left->codegen();
	llvm::Value* r = right->codegen();
	if (!l || !r)
	{
		return LogErrorV("Binary expression failed, couldn't find left and/or righgt");
	}

	switch (type) {
	case TokenType::PLUS: return llvmBuilder.CreateFAdd(l, r, "addtmp");
	case TokenType::MINUS: return llvmBuilder.CreateFSub(l, r, "subtmp");
	case TokenType::STAR: return llvmBuilder.CreateFMul(l, r, "multmp");
	case TokenType::SLASH: return llvmBuilder.CreateFDiv(l, r, "divtmp");
	case TokenType::LEFT_ANGLE: return llvmBuilder.CreateFCmpOGT(l, r, "gttmp");
	case TokenType::RIGHT_ANGLE: return llvmBuilder.CreateFCmpOLT(l, r, "lttmp");
	}

	return LogErrorV("Binary expression failed, did not recognize binary op");
}

llvm::Value* lang::parser::CallExprAST::codegen()
{
	llvm::Function* function = llvmModule->getFunction(callee);
	if (function == nullptr) {
		return LogErrorV("Couldn't find function in module");
	}

	if (function->arg_size() != args->arguments.size()) {
		return LogErrorV("Argument list mismatch. Expected: %d, Given: %d");
	}

	std::vector<llvm::Value*> llvmArgs;
	for (size_t i = 0; i < args->arguments.size(); i++) {
		llvm::Value* arg = args->arguments[i]->codegen();
		if (arg == nullptr) {
			LogError("Couldn't gen code for argument...");
		}

		llvmArgs.push_back(arg);
	}

	return llvmBuilder.CreateCall(function, llvmArgs, "calltmp");
}

llvm::Function* lang::parser::FunctionSignatureAST::codegen()
{
	std::vector<llvm::Type*> params;
	for (auto* t : args->arguments) {
		params.push_back(llvm::Type::getFloatTy(llvmContext)); // TODO: Make sure this is the right type
		//params.push_back(static_cast<VariableExprAST*>(t)->type);
		//params.push_back(t->codegen()->getType()); // NOTE: Not sure if this is valid...?
	}

	llvm::FunctionType* ft = llvm::FunctionType::get(llvm::Type::getFloatTy(llvmContext), params, false);
	llvm::Function* f = llvm::Function::Create(ft, llvm::GlobalValue::LinkageTypes::ExternalLinkage, name, llvmModule.get());
	
	size_t index = 0;
	for (auto& arg : f->args()) {
		arg.setName(static_cast<VariableExprAST*>(args->arguments[index])->name);
		index++;
	}

	return f;
}

llvm::Value* lang::parser::FunctionAST::codegen()
{
	llvm::Function* f = llvmModule->getFunction(signature->name);
	if (f == nullptr) {
		f = signature->codegen();
	}

	if (f == nullptr) {
		return LogErrorV("Couldn't generate function implementation");
	}

	llvm::BasicBlock* llvmBody = llvm::BasicBlock::Create(llvmContext, "functionEntry", f);
	llvmBuilder.SetInsertPoint(llvmBody);

	llvmNamedValues.clear(); // Entering new scope, clear local variables list
	
	// Add arguments
	for (auto& arg : f->args()) {
		llvmNamedValues.try_emplace(arg.getName().str(), &arg);
	}

	// Add local scope variables
	//for (auto* v : body->body) {
	//	auto* variable = dynamic_cast<VariableExprAST*>(v);
	//	if (variable) {
	//		llvmNamedValues.try_emplace(variable->name, variable->codegen()); // Not sure if this is valid...
	//	}
	//}


	llvm::Value* retValue = body->codegen();
	if (retValue) {
		llvmBuilder.CreateRet(retValue);
		llvm::verifyFunction(*f);
		return f;
	}

	f->eraseFromParent();
	return LogErrorV("Couldn't generate function body");
}

llvm::Value* lang::parser::IfAST::codegen()
{
	llvm::Function* parentFunction = llvmBuilder.GetInsertBlock()->getParent();

	assert(chain.size() <= 1); // TODO: Add more later...
	assert(hasElseAtEnd == false);

	auto& a = chain[0];
	llvm::Value* cond = a.condition->codegen();


	//llvm::BasicBlock* trueBlock = llvm::BasicBlock::Create(llvmContext, "trueBlock", parentFunction);
	//llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(llvmContext, "ifcont");
	////llvm::BasicBlock* falseBlock = llvm::BasicBlock::Create(llvmContext, "falseBlock", parentFunction);
	//llvm::BasicBlock* falseBlock = nullptr;

	//llvm::Value* val = llvmBuilder.CreateCondBr(cond, trueBlock, falseBlock, nullptr, nullptr);
	//
	//llvmBuilder.SetInsertPoint(trueBlock);
	//llvm::Value* trueBody = a.body->codegen();
	//if (trueBody == nullptr) {
	//	return LogErrorV("Couldn't generate code block for true branch of if statement");
	//}

	//llvmBuilder.CreateBr(mergeBB);
	//trueBlock = llvmBuilder.GetInsertBlock();

	//auto& functionBodyBlock = parentFunction->getBasicBlockList();
	//functionBodyBlock.push_back(trueBlock);


	return cond;
}

llvm::Value* lang::parser::CodeBlockAST::codegen()
{
	llvm::Function* parentFunction = llvmBuilder.GetInsertBlock()->getParent();
	
	std::string name = "block1";
	llvm::BasicBlock* block = llvm::BasicBlock::Create(llvmContext, name, parentFunction);
	llvmBuilder.SetInsertPoint(block);
	
	for (auto* n : body) {
		//auto& list = block->getInstList();
		//list.addNodeToList(n->codegen());

		auto* val = n->codegen();
	}

	return block;
}
