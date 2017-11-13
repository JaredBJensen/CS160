%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <iostream>

    #include "ast.hpp"
    
    #define YYDEBUG 1
	#define YYINITDEPTH 10000
    int yylex(void);
    void yyerror(const char *);
    
    extern ASTNode* astRoot;
%}

%error-verbose

%token T_OPENPAREN T_CLOSEPAREN T_OPENBRACE T_CLOSEBRACE T_COMMA T_SEMICOLON
%token T_PLUS T_MINUS T_UNARYMINUS T_MULTIPLY T_DIVIDE T_GT T_GTE T_DOT T_EQSIGN T_ARROW
%token T_PRINT T_RETURN T_IF T_ELSE T_WHILE T_NEW T_INTEGER T_BOOLEAN T_NONE T_EQUALS T_AND T_OR T_NOT T_TRUE T_FALSE T_EXTENDS T_DO
%token T_ID T_NUMBER

%left T_OR
%left T_AND
%left T_GT T_GTE T_EQUALS
%left T_PLUS T_MINUS
%left T_MULTIPLY T_DIVIDE
%precedence T_NOT T_UNARYMINUS

%type <program_ptr> Start
%type <class_list_ptr> Classes
%type <class_ptr> Class
%type <identifier_ptr> ClassP
%type <declaration_list_ptr> Members Declarations
%type <declaration_ptr> Member Declaration
%type <identifier_list_ptr> DeclarationP Ids
%type <method_list_ptr> Methods
%type <method_ptr> Method
%type <parameter_list_ptr> Parameters ParametersP
%type <parameter_ptr> Parameter
%type <methodbody_ptr> Body
%type <statement_list_ptr> Statements Block
%type <statement_ptr> Statement
%type <returnstatement_ptr> ReturnStatement
%type <assignment_ptr> Assignment
%type <ifelse_ptr> If
%type <while_ptr> While
%type <dowhile_ptr> DoWhile
%type <print_ptr> Print
%type <methodcall_ptr> MethodCall
%type <expression_ptr> Expression
%type <expression_list_ptr> Arguments
%type <type_ptr> Type ReturnType

%type <base_char_ptr> T_ID
%type <base_int> T_NUMBER

%%

Start	: Classes	{ astRoot = new ProgramNode($1); }
		;

Classes	: Classes Class		{ $$ = $1; $$->push_back($2); }
		| %empty			{ $$ = new std::list<ClassNode*>(); }
		;

Class	: T_ID ClassP T_OPENBRACE Members Methods T_CLOSEBRACE	{ $$ = new ClassNode(new IdentifierNode($1), $2, $4, $5); }
		;
	
ClassP	: T_EXTENDS T_ID	{ $$ = new IdentifierNode($2); }
		| %empty			{ $$ = NULL; }
		;
		
Members 	: Members Member	{ $$ = $1; $$->push_back($2); }
			| %empty			{ $$ = new std::list<DeclarationNode*>(); }
			;
				
Member		: Type Ids T_SEMICOLON		{ $$ = new DeclarationNode($1, $2); }
			;
					
Ids 		: T_ID		{ $$ = new std::list<IdentifierNode*>(); $$->push_back(new IdentifierNode($1)); }
			;
				
Methods		: Method Methods	{ $$ = $2; $$->push_front($1); }
			| %empty			{ $$ = new std::list<MethodNode*>(); }
			;
				
Method		: T_ID T_OPENPAREN Parameters T_CLOSEPAREN T_ARROW ReturnType T_OPENBRACE Body T_CLOSEBRACE		{ $$ = new MethodNode(new IdentifierNode($1), $3, $6, $8); }
			;
				
Parameters	: ParametersP T_COMMA Parameter		{ $$ = $1; $$->push_back($3); }
			| Parameter							{ $$ = new std::list<ParameterNode*>(); $$->push_back($1); }
			| %empty							{ $$ = new std::list<ParameterNode*>(); }
			;

ParametersP	: ParametersP T_COMMA Parameter		{ $$ = $1; $$->push_back($3); }
			| Parameter							{ $$ = new std::list<ParameterNode*>(); $$->push_back($1); }

Parameter	: Type T_ID		{ $$ = new ParameterNode($1, new IdentifierNode($2)); }
			;
				
Body 		: Declarations Statements ReturnStatement	{ $$ = new MethodBodyNode($1, $2, $3); }
			;
				
Declarations	: Declarations Declaration		{ $$ = $1; $$->push_back($2); }
				| %empty						{ $$ = new std::list<DeclarationNode*>(); }
				;
				
Declaration		: Type DeclarationP		{ $$ = new DeclarationNode($1, $2); }
				;
				
DeclarationP 	: T_ID T_COMMA DeclarationP		{ $$ = $3; $$->push_front(new IdentifierNode($1)); }
				| T_ID T_SEMICOLON				{ $$ = new std::list<IdentifierNode*>(); $$->push_back(new IdentifierNode($1)); }
				;
				
ReturnStatement : T_RETURN Expression T_SEMICOLON		{ $$ = new ReturnStatementNode($2); }
				| %empty								{ $$ = NULL; }
				;
				
Statements		: Statement Statements 		{ $$ = $2; $$->push_front($1); }
				| %empty					{ $$ = new std::list<StatementNode*>(); }
				;
				
Statement 		: Assignment 					{ $$ = $1; }
				| MethodCall T_SEMICOLON		{ $$ = new CallNode($1); }
				| If							{ $$ = $1; }
				| While							{ $$ = $1; }
				| DoWhile						{ $$ = $1; }
				| Print							{ $$ = $1; }
				;
				
Assignment		: T_ID T_EQSIGN Expression T_SEMICOLON					{ $$ = new AssignmentNode(new IdentifierNode($1), NULL, $3); }
				| T_ID T_DOT T_ID T_EQSIGN Expression T_SEMICOLON		{ $$ = new AssignmentNode(new IdentifierNode($1), new IdentifierNode($3), $5); }
				;
				
If				: T_IF Expression T_OPENBRACE Block T_CLOSEBRACE											{ $$ = new IfElseNode($2, $4, NULL); }
				| T_IF Expression T_OPENBRACE Block T_CLOSEBRACE T_ELSE T_OPENBRACE Block T_CLOSEBRACE		{ $$ = new IfElseNode($2, $4, $8); }
				;
				
While			: T_WHILE Expression T_OPENBRACE Block T_CLOSEBRACE		{ $$ = new WhileNode($2, $4); }
				;
				
DoWhile			: T_DO T_OPENBRACE Block T_CLOSEBRACE T_WHILE T_OPENPAREN Expression T_CLOSEPAREN T_SEMICOLON	{ $$ = new DoWhileNode($3, $7); }
				;
				
Print			: T_PRINT Expression T_SEMICOLON	{ $$ = new PrintNode($2); }
				;
				
Block			: Statement Statements	{ $$ = $2; $$->push_front($1); }
				;
				
Expression		: Expression T_PLUS Expression						{ $$ = new PlusNode($1, $3); }
				| Expression T_MINUS Expression						{ $$ = new MinusNode($1, $3); }
				| Expression T_MULTIPLY Expression					{ $$ = new TimesNode($1, $3); }
				| Expression T_DIVIDE Expression					{ $$ = new DivideNode($1, $3); }
				| Expression T_GT Expression						{ $$ = new GreaterNode($1, $3); }
				| Expression T_GTE Expression						{ $$ = new GreaterEqualNode($1, $3); }
				| Expression T_EQUALS Expression					{ $$ = new EqualNode($1, $3); }
				| Expression T_AND Expression						{ $$ = new AndNode($1, $3); }
				| Expression T_OR Expression						{ $$ = new OrNode($1, $3); }
				| T_NOT Expression									{ $$ = new NotNode($2); }
				| T_MINUS Expression %prec T_UNARYMINUS				{ $$ = new NegationNode($2); }
				| T_ID												{ $$ = new VariableNode(new IdentifierNode($1)); }
				| T_ID T_DOT T_ID									{ $$ = new MemberAccessNode(new IdentifierNode($1), new IdentifierNode($3)); }
				| MethodCall										{ $$ = $1; }
				| T_OPENPAREN Expression T_CLOSEPAREN				{ $$ = $2; }
				| T_NUMBER											{ $$ = new IntegerLiteralNode(new IntegerNode($1)); }
				| T_TRUE											{ $$ = new BooleanLiteralNode(new IntegerNode(1)); }
				| T_FALSE											{ $$ = new BooleanLiteralNode(new IntegerNode(0)); }
				| T_NEW T_ID										{ $$ = new NewNode(new IdentifierNode($2), NULL); }
				| T_NEW T_ID T_OPENPAREN Arguments T_CLOSEPAREN		{ $$ = new NewNode(new IdentifierNode($2), $4); }
				;
				
MethodCall		: T_ID T_OPENPAREN Arguments T_CLOSEPAREN				{ $$ = new MethodCallNode(new IdentifierNode($1), NULL, $3); }
				| T_ID T_DOT T_ID T_OPENPAREN Arguments T_CLOSEPAREN	{ $$ = new MethodCallNode(new IdentifierNode($1), new IdentifierNode($3), $5); }
				;
				
Arguments		: Arguments T_COMMA Expression		{ $$ = $1; $$->push_back($3); }
				| Expression						{ $$ = new std::list<ExpressionNode*>(); $$->push_back($1); }
				;

Type			: T_INTEGER		{ $$ = new IntegerTypeNode(); }
				| T_BOOLEAN		{ $$ = new BooleanTypeNode(); }
				| T_ID			{ $$ = new ObjectTypeNode(new IdentifierNode($1)); }
				;
				
ReturnType		: Type			{ $$ = $1; }
				| T_NONE		{ $$ = new NoneNode(); }
				;

%%

extern int yylineno;

void yyerror(const char *s) {
  fprintf(stderr, "%s at line %d\n", s, yylineno);
  exit(0);
}
