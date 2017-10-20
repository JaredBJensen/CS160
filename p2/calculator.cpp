
#include "calculator.hpp"
#include <string>
#include <cmath>
#include <iostream>

// Scanner implementation

// You may have to modify this constructor, although it might not be neccessary.
Scanner::Scanner() : line(1), 
                     value(0),
					 hasLookahead(false) {}

// You need to fill this method with the appropriate code for it to work as described in the project description.
Token Scanner::nextToken() {
	if (hasLookahead) return lookahead;
	
    char c = std::cin.get();
	
	switch (c) {
		case '+': lookahead = T_PLUS; break;
		case '-': lookahead = T_MINUS; break;
		case '*': lookahead = T_MULTIPLY; break;
		case '/': lookahead = T_DIVIDE; break;
		case 'm': 
			c = std::cin.get();
			if (c == 'o') {
				c = std::cin.get();
				if (c == 'd') lookahead = T_MODULO;
				else scanError(line, c);
			} else scanError(line, c);
			break;
		case '(': lookahead = T_OPENPAREN; break;
		case ')': lookahead = T_CLOSEPAREN; break;
		case ';': lookahead = T_SEMICOLON; break;
		case '\n': lookahead = T_NEWLN; break;
		case ' ': case '\t': case '\f': return nextToken();
		case EOF: lookahead = T_EOF; break;
		default:
			if (c >= '0' && c <= '9') {
				lookahead = T_NUMBER;

				long num64 = c - '0';
				
				int digit = std::cin.get();
				while (digit >= '0' && digit <= '9') {
					num64 *= 10;
					num64 += digit - '0';
					
					digit = std::cin.get();
				}
				std::cin.unget();
				
				if (evaluate && num64 > INT_MAX) outOfBoundsError(line, num64);
				else value = num64;
			}
			else scanError(line, c);
			break;
	}
	
	hasLookahead = true;
    return lookahead;
}

// You need to fill this method with the appropriate code for it to work as described in the project description.
void Scanner::eatToken(Token toConsume) {
	if (toConsume != nextToken()) {
		mismatchError(line, toConsume, nextToken());
	}
	
	if (toConsume == T_NEWLN) line++;
	
    hasLookahead = false;
}

int Scanner::lineNumber() {
    return this->line;
}

int Scanner::getNumberValue() {
    return this->value;
}

void Scanner::setEvaluate(bool eval) {
	evaluate = eval;
}

// Parser implementation

// You may need to modify this constructor and make it do stuff, although it might not be neccessary.
Parser::Parser(bool eval) : evaluate(eval) {
	scanner.setEvaluate(eval);
}

void Parser::parse() {
    start();
}

void Parser::start() {
	ExprList();
	
	if (evaluate) {
		for (size_t i=0; i<results.size(); i++) {
			std::cout << results.at(i) << std::endl;
		}
	}
}

void Parser::ExprList() {
	Token t = scanner.nextToken();
	switch (t) {
		case T_OPENPAREN:
		case T_NUMBER:
			results.push_back(Expression());
			ExprListP();
			break;
		case T_EOF:
			break;
		case T_NEWLN:
			scanner.eatToken(t);
			ExprList();
			break;
		default:
			parseError(scanner.lineNumber(), t);
			break;
	}
}

void Parser::ExprListP() {
	Token t = scanner.nextToken();
	switch (t) {
		case T_SEMICOLON:
			scanner.eatToken(t);
			ExprList();
			break;
		case T_EOF:
			break;
		case T_NEWLN:
			scanner.eatToken(t);
			ExprListP();
			break;
		default:
			parseError(scanner.lineNumber(), t);
			break;
	}
}

int Parser::Expression() {
	Token t = scanner.nextToken();
	switch (t) {
		case T_OPENPAREN:
		case T_NUMBER:
			return ExpressionP(Term());
		case T_NEWLN:
			scanner.eatToken(t);
			return Expression();
		default:
			parseError(scanner.lineNumber(), t);
			return 0;
	}
}

int Parser::ExpressionP(int lhs) {
	Token t = scanner.nextToken();
	int parent;
	switch (t) {
		case T_PLUS:
			scanner.eatToken(t);
			parent = lhs + Expression();
			break;
		case T_MINUS:
			scanner.eatToken(t);
			parent = lhs - Expression();
			break;
		case T_CLOSEPAREN:
		case T_DIVIDE:
		case T_MULTIPLY:
		case T_MODULO:
		case T_EOF:
		case T_SEMICOLON:
			return lhs;
			break;
		case T_NEWLN:
			scanner.eatToken(t);
			return ExpressionP(lhs);
			break;
		default:
			parseError(scanner.lineNumber(), t);
			break;
	}
	return parent;
}

int Parser::Term() {
	Token t = scanner.nextToken();
	int parent;
	int lhs;
	switch (t) {
		case T_OPENPAREN:
		case T_NUMBER:
			lhs = Factor();
			parent = TermP(lhs);
			break;
		case T_NEWLN:
			scanner.eatToken(t);
			return Term();
			break;
		default:
			parseError(scanner.lineNumber(), t);
			break;
	}
	return parent;
}

int Parser::TermP(int lhs) {
	Token t = scanner.nextToken();
	int parent;
	int rhs;
	long check;
	switch (t) {
		case T_MULTIPLY:
			scanner.eatToken(t);
			rhs = Term();
			check = ((long)lhs) * ((long)rhs);
			if (check > INT_MAX || check < INT_MIN) outOfBoundsError(scanner.lineNumber(), check);
			else parent = lhs * rhs;
			break;
		case T_DIVIDE:
			scanner.eatToken(t);
			rhs = Term();
			if (rhs != 0) parent = lhs / rhs;
			else divideByZeroError(scanner.lineNumber(), lhs);
			break;
		case T_MODULO:
			scanner.eatToken(t);
			parent = lhs % Term();
			break;
		case T_PLUS:
		case T_MINUS:
		case T_CLOSEPAREN:
		case T_EOF:
		case T_SEMICOLON:
			return lhs;
			break;
		case T_NEWLN:
			scanner.eatToken(t);
			return TermP(lhs);
			break;
		default:
			parseError(scanner.lineNumber(), t);
			break;
	}
	return parent;
}

int Parser::Factor() {
	Token t = scanner.nextToken();
	int parent;
	switch (t) {
		case T_OPENPAREN:
			scanner.eatToken(T_OPENPAREN);
			parent = Expression();
			scanner.eatToken(T_CLOSEPAREN);
			break;
		case T_NUMBER:
			parent = scanner.getNumberValue();
			scanner.eatToken(t);
			break;
		case T_NEWLN:
			scanner.eatToken(t);
			return Factor();
			break;
		default:
			parseError(scanner.lineNumber(), t);
			break;
	}
	return parent;
}

