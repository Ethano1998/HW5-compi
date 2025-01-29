%{

#include "nodes.hpp"
#include "output.hpp"
#include <memory>

// bison declarations
extern int yylineno;
extern int yylex();

void yyerror(const char*);

// root of the AST, set by the parser and used by other parts of the compiler
std::shared_ptr<ast::Node> program;

using namespace std;

// TODO: Place any additional declarations here
%}


// TODO: Define tokens here 
%token VOID
%token INT
%token BYTE
%token BOOL
%token TRUE
%token FALSE
%token RETURN
%token WHILE
%token BREAK
%token CONTINUE
%token SC
%token COMMA
%token LBRACE
%token RBRACE
%token NUM
%token NUM_B
%token ID
%token STRING

%precedence IF
%precedence ELSE

%right ASSIGN
%left OR
%left AND
%left EQUA
%left REL
%left ADD
%left MUL
%right CAST
%right NOT
%left LPAREN
%left RPAREN


// TODO: Define precedence and associativity here

%%

// While reducing the start variable, set the root of the AST
Program:  Funcs         { program = $1; }
Funcs:                  {$$ = std::make_shared<ast::Funcs>();}
    |           FuncDecl Funcs  {std::dynamic_pointer_cast<ast::Funcs>($2)->push_front(std::dynamic_pointer_cast<ast::FuncDecl>($1)); $$ = $2;}
FuncDecl:       RetType ID LPAREN Formals RPAREN LBRACE Statements RBRACE {$$ = std::make_shared<ast::FuncDecl>(std::dynamic_pointer_cast<ast::ID>($2),std::dynamic_pointer_cast<ast::Type>($1),std::dynamic_pointer_cast<ast::Formals>($4),std::dynamic_pointer_cast<ast::Statements>($7));}
RetType:        Type   {$$ = $1;}
    |           VOID      {$$ = std::make_shared<ast::Type>(ast::VOID);}
Formals:              {$$ = std::make_shared<ast::Formals>();}
    |           FormalsList   {$$ = $1;}
FormalsList:    FormalDecl {$$ = std::make_shared<ast::Formals>(std::dynamic_pointer_cast<ast::Formal>($1));}
    |           FormalDecl COMMA FormalsList    {std::dynamic_pointer_cast<ast::Formals>($3)->push_front(std::dynamic_pointer_cast<ast::Formal>($1));$$ = $3;}
FormalDecl:     Type ID {$$ = std::make_shared<ast::Formal>(std::dynamic_pointer_cast<ast::ID>($2),std::dynamic_pointer_cast<ast::Type>($1));}
Statements:     Statement {$$ = std::make_shared<ast::Statements>(std::dynamic_pointer_cast<ast::Statement>($1));}
    |           Statements Statement {std::dynamic_pointer_cast<ast::Statements>($1)->push_back(std::dynamic_pointer_cast<ast::Statement>($2)); $$ = $1;}
Statement:      LBRACE Statements RBRACE    {$$ = $2;}
    |           Type ID SC     {$$ = std::make_shared<ast::VarDecl>(std::dynamic_pointer_cast<ast::ID>($2),std::dynamic_pointer_cast<ast::Type>($1));}
    |           Type ID ASSIGN Exp SC   {$$ = std::make_shared<ast::VarDecl>(std::dynamic_pointer_cast<ast::ID>($2),std::dynamic_pointer_cast<ast::Type>($1),std::dynamic_pointer_cast<ast::Exp>($4));}
    |           ID ASSIGN Exp SC    {$$ = std::make_shared<ast::Assign>(std::dynamic_pointer_cast<ast::ID>($1),std::dynamic_pointer_cast<ast::Exp>($3));}
    |           Call SC     {$$ = $1;}
    |           RETURN SC   {$$ = std::make_shared<ast::Return>();}
    |           RETURN Exp SC   {$$ = std::make_shared<ast::Return>(std::dynamic_pointer_cast<ast::Exp>($2));}
    |           IF LPAREN Exp RPAREN Statement %prec IF {$$ = std::make_shared<ast::If>(std::dynamic_pointer_cast<ast::Exp>($3),std::dynamic_pointer_cast<ast::Statement>($5));}
    |           IF LPAREN Exp RPAREN Statement ELSE Statement {$$ = std::make_shared<ast::If>(std::dynamic_pointer_cast<ast::Exp>($3),std::dynamic_pointer_cast<ast::Statement>($5),std::dynamic_pointer_cast<ast::Statement>($7));}
    |           WHILE LPAREN Exp RPAREN Statement   {$$ = std::make_shared<ast::While>(std::dynamic_pointer_cast<ast::Exp>($3),std::dynamic_pointer_cast<ast::Statement>($5));}
    |           BREAK SC    {$$ = std::make_shared<ast::Break>();}
    |           CONTINUE SC {$$ = std::make_shared<ast::Continue>();}
Call:           ID LPAREN ExpList RPAREN {$$ = std::make_shared<ast::Call>(std::dynamic_pointer_cast<ast::ID>($1),std::dynamic_pointer_cast<ast::ExpList>($3));}
    |           ID LPAREN RPAREN         {$$ = std::make_shared<ast::Call>(std::dynamic_pointer_cast<ast::ID>($1));}
ExpList:        Exp     {$$ = std::make_shared<ast::ExpList>(std::dynamic_pointer_cast<ast::Exp>($1));}
    |           Exp COMMA ExpList   {std::dynamic_pointer_cast<ast::ExpList>($3)->push_front(std::dynamic_pointer_cast<ast::Exp>($1));$$ = $3;}
Type:           INT     {$$ = std::make_shared<ast::Type>(ast::INT);}
    |           BYTE    {$$ = std::make_shared<ast::Type>(ast::BYTE);}
    |           BOOL    {$$ = std::make_shared<ast::Type>(ast::BOOL);}
Exp:            LPAREN Exp RPAREN  {$$ = $2;}
    |           Exp ADD Exp      {$2 = std::make_shared<ast::BinOp>(std::dynamic_pointer_cast<ast::Exp>($1),std::dynamic_pointer_cast<ast::Exp>($3),std::dynamic_pointer_cast<ast::BinOp>($2)->op); $$ = $2;}
    |           Exp MUL Exp      {$2 = std::make_shared<ast::BinOp>(std::dynamic_pointer_cast<ast::Exp>($1),std::dynamic_pointer_cast<ast::Exp>($3),std::dynamic_pointer_cast<ast::BinOp>($2)->op); $$ = $2;}
    |           ID                 {$$ = $1;}
    |           Call               {$$ = $1;}
    |           NUM                {$$ = $1;}
    |           NUM_B                {$$ = $1;}
    |           STRING                {$$ = $1;}
    |           TRUE                {$$ = std::make_shared<ast::Bool>(true);}
    |           FALSE                {$$ = std::make_shared<ast::Bool>(false);}
    |           NOT Exp                {$$ = std::make_shared<ast::Not>(std::dynamic_pointer_cast<ast::Exp>($2));}
    |           Exp AND Exp          {$$ = std::make_shared<ast::And>(std::dynamic_pointer_cast<ast::Exp>($1),std::dynamic_pointer_cast<ast::Exp>($3)); }  
    |           Exp OR Exp            {$$ = std::make_shared<ast::Or>(std::dynamic_pointer_cast<ast::Exp>($1),std::dynamic_pointer_cast<ast::Exp>($3)); }
    |           Exp EQUA Exp        {$2 = std::make_shared<ast::RelOp>(std::dynamic_pointer_cast<ast::Exp>($1),std::dynamic_pointer_cast<ast::Exp>($3),std::dynamic_pointer_cast<ast::RelOp>($2)->op); $$ = $2;}
    |           Exp REL Exp        {$2 = std::make_shared<ast::RelOp>(std::dynamic_pointer_cast<ast::Exp>($1),std::dynamic_pointer_cast<ast::Exp>($3),std::dynamic_pointer_cast<ast::RelOp>($2)->op); $$ = $2;}
    |           LPAREN Type RPAREN Exp %prec CAST {$$ = std::make_shared<ast::Cast>(std::dynamic_pointer_cast<ast::Exp>($4),std::dynamic_pointer_cast<ast::Type>($2));}






// TODO: Define grammar here

%%

void yyerror(const char*) {
    output::errorSyn(yylineno);
}
// TODO: Place any additional code here