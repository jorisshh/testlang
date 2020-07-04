#include <Windows.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <chrono>

#include "types.h"
#include "util.h"
#include "lexer.h"

int main(int argc, char** argv) {

	using hc = std::chrono::high_resolution_clock;
	hc::time_point startTime = hc::now();

	if (argc < 2) {
		std::cerr << "you have to pass in an entry point for compilation\n";
		return -1;
	}

	const char* arg = argv[1];
	std::cout << "Starting compilation of " << arg << "\n";

	std::string contents = lang::fsutil::readTextFile(arg);
	std::vector<lang::lexer::Token> tokens = lang::lexer::parse(contents);


	hc::time_point endLexingTime = hc::now();

	for (lang::lexer::Token& t : tokens) {
		std::cout << "\"" << t.span.string << "\":" << lang::lexer::TokenType::toString(t.type) << " " << t.span.from << ":" << t.span.to << "\n";
	}


	f64 microSeconds = (f64)std::chrono::duration_cast<std::chrono::microseconds>((endLexingTime - startTime)).count();
	std::cout << "Lexing time:: " << (microSeconds * 1000) << "ms\n";

	return 0;
}