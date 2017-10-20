#pragma once

#include "helpers.hpp"

#include <climits>
#include <string>
#include <vector>
#include <stack>

// SCANNER DEFINITION

// You can freely add member fields and functions to this class.
class Scanner {
    int line;
    int value;
	Token lookahead;
	bool hasLookahead;
	bool evaluate;

public:
    // You really need to implement these four methods for the scanner to work.
    Token nextToken();
    void eatToken(Token);
    int lineNumber();
    int getNumberValue();
	void setEvaluate(bool eval);
    
    Scanner();

};

// PARSER DEFINITION

// You can freely add member fields and functions to this class.
class Parser {
    Scanner scanner;
	std::vector<long> results;
    
    bool evaluate;
    void start();
	
public:
    void parse();
    
	void ExprList();
	void ExprListP();
	int Expression();
	int ExpressionP(int lhs);
	int Term();
	int TermP(int lhs);
	int Factor();
	
    Parser(bool);
};
