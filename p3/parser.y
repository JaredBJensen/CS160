%{
    #include <cstdlib>
    #include <cstdio>
    #include <iostream>

    #define YYDEBUG 1

    int yylex(void);
    void yyerror(const char *);
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

%%

Classes 		: Classes Class
				| %empty
				;

Class			: T_ID ClassP T_OPENBRACE Members Methods T_CLOSEBRACE
				;
	
ClassP			: T_EXTENDS T_ID
				| %empty
				;
		
Members 		: Members Member
				| %empty
				;
				
Member			: Type T_ID T_SEMICOLON
				;
				
Methods			: Method Methods
				| %empty
				;
				
Method			: T_ID T_OPENPAREN Parameters T_CLOSEPAREN T_ARROW ReturnType T_OPENBRACE Body T_CLOSEBRACE
				;
				
Parameters		: Parameter ParametersP
				| %empty
				;
				
ParametersP		: T_COMMA Parameter ParametersP
				| %empty
				;
				
Parameter		: Type T_ID
				;
				
Body 			: Declarations Statements ReturnStatement
				;
				
Declarations	: Declarations Declaration
				| %empty
				;
				
Declaration		: Type DeclarationP
				;
				
DeclarationP 	: T_ID T_COMMA DeclarationP
				| T_ID T_SEMICOLON
				;
				
ReturnStatement : T_RETURN Expression T_SEMICOLON
				| %empty
				;
				
Statements		: Statement Statements 
				| %empty
				;
				
Statement 		: Assignment
				| MethodCall T_SEMICOLON
				| If
				| While
				| DoWhile
				| Print
				;
				
Assignment		: T_ID AssignmentP T_EQSIGN Expression T_SEMICOLON
				;
				
AssignmentP		: T_DOT T_ID
				| %empty
				;
				
If				: T_IF Expression T_OPENBRACE Block T_CLOSEBRACE
				| T_IF Expression T_OPENBRACE Block T_CLOSEBRACE T_ELSE T_OPENBRACE Block T_CLOSEBRACE
				;
				
While			: T_WHILE Expression T_OPENBRACE Block T_CLOSEBRACE
				;
				
DoWhile			: T_DO T_OPENBRACE Block T_CLOSEBRACE T_WHILE T_OPENPAREN Expression T_CLOSEPAREN T_SEMICOLON
				;
				
Print			: T_PRINT Expression T_SEMICOLON
				;
				
Block			: Statement Statements
				;
				
Expression		: Expression T_PLUS Expression
				| Expression T_MINUS Expression
				| Expression T_MULTIPLY Expression
				| Expression T_DIVIDE Expression
				| Expression T_GT Expression
				| Expression T_GTE Expression
				| Expression T_EQUALS Expression
				| Expression T_AND Expression
				| Expression T_OR Expression
				| T_NOT Expression
				| T_MINUS Expression %prec T_UNARYMINUS
				| T_ID
				| T_ID T_DOT T_ID
				| MethodCall
				| T_OPENPAREN Expression T_CLOSEPAREN
				| T_NUMBER
				| T_TRUE
				| T_FALSE
				| T_NEW T_ID
				| T_NEW T_ID T_OPENPAREN Arguments T_CLOSEPAREN
				;
				
MethodCall		: T_ID T_OPENPAREN Arguments T_CLOSEPAREN
				| T_ID T_DOT T_ID T_OPENPAREN Arguments T_CLOSEPAREN
				;
				
Arguments		: Expression ArgumentsP
				;
				
ArgumentsP		: T_COMMA Expression ArgumentsP
				| %empty
				;

Type			: T_INTEGER
				| T_BOOLEAN
				| T_ID
				;
				
ReturnType		: Type
				| T_NONE
				;

%%


extern int yylineno;

void yyerror(const char *s) {
  fprintf(stderr, "%s at line %d\n", s, yylineno);
  exit(1);
}
