%{
/* Declaration Section */
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include "nodes.hpp"
#include "parser.tab.h"
#include "output.hpp"

char* my_string;
int i;
void allocString();
    
void freeString();

%}

%option yylineno
%option noyywrap
%x STR
%x BACKSLASH

%%
void                        {return VOID; }
int                         {return INT; }
byte                        {return BYTE; }
bool                        {return BOOL; }
and                         {return AND; }
or                          {return OR; }
not                         {return NOT; }
true                        {return TRUE; }
false                       {return FALSE; }
return                      {return RETURN; }
if                          {return IF; }
else                        {return ELSE; }
while                       {return WHILE;}
break                       {return BREAK;}
continue                    {return CONTINUE;}
;                           {return SC;}
,                           {return COMMA;}
\(                          {return LPAREN;}
\)                          {return RPAREN;}
\{                          {return LBRACE;}
\}                          {return RBRACE;}
=                           {return ASSIGN;}
[1-9][0-9]*|0               {yylval = std::make_shared<ast::Num>(yytext); return NUM;}
[1-9][0-9]*b|0b             {yylval = std::make_shared<ast::NumB>(yytext); return NUM_B;}
[a-zA-Z][a-zA-Z0-9]*        {yylval = std::make_shared<ast::ID>(yytext); return ID;}
==                          {yylval = std::make_shared<ast::RelOp>(std::make_shared<ast::Num>("0"), std::make_shared<ast::Num>("0"), ast::EQ); return EQUA;}
!=                          {yylval = std::make_shared<ast::RelOp>(std::make_shared<ast::Num>("0"), std::make_shared<ast::Num>("0"), ast::NE); return EQUA;}
\<                          {yylval = std::make_shared<ast::RelOp>(std::make_shared<ast::Num>("0"), std::make_shared<ast::Num>("0"), ast::LT); return REL;}
>                           {yylval = std::make_shared<ast::RelOp>(std::make_shared<ast::Num>("0"), std::make_shared<ast::Num>("0"), ast::GT); return REL;}
\<=                         {yylval = std::make_shared<ast::RelOp>(std::make_shared<ast::Num>("0"), std::make_shared<ast::Num>("0"), ast::LE); return REL;}
>=                          {yylval = std::make_shared<ast::RelOp>(std::make_shared<ast::Num>("0"), std::make_shared<ast::Num>("0"), ast::GE); return REL;}
\+                          {yylval = std::make_shared<ast::BinOp>(std::make_shared<ast::Num>("0"), std::make_shared<ast::Num>("0"), ast::ADD); return ADD;}
\-                          {yylval = std::make_shared<ast::BinOp>(std::make_shared<ast::Num>("0"), std::make_shared<ast::Num>("0"), ast::SUB); return ADD;}
\*                          {yylval = std::make_shared<ast::BinOp>(std::make_shared<ast::Num>("0"), std::make_shared<ast::Num>("0"), ast::MUL); return MUL;}
\/                          {yylval = std::make_shared<ast::BinOp>(std::make_shared<ast::Num>("0"), std::make_shared<ast::Num>("0"), ast::DIV); return MUL;}


\/\/[^\r\n]*[ \r|\n|\r\n]?              {} 
[ \n\t\r]                               {}
   

\"                    {allocString(); i = 0; my_string[i] = '\"'; i = i + 1;
                        BEGIN(STR);  }
<STR>[^\\\n\r\"]     {my_string[i]=yytext[0];
                                i=i+1;}
<STR>\\          {BEGIN(BACKSLASH); }
<STR><<EOF>>       { freeString();
                       output::errorLex(yylineno); }
<STR>\"             {BEGIN(INITIAL); my_string[i] = '\"'; my_string[i+1] = '\0'; yylval = std::make_shared<ast::String>(my_string); freeString();
                        return STRING;}
<STR>\r             {output::errorLex(yylineno);}
<STR>\n             {output::errorLex(yylineno- 1);}
<BACKSLASH>n                                  {BEGIN(STR); 
                                                my_string[i] = '\n';  i = i + 1;}  
<BACKSLASH>r                                  {BEGIN(STR); 
                                                 my_string[i] = '\r'; i = i + 1;}
<BACKSLASH>t                                 {BEGIN(STR); 
                                                 my_string[i] = '\t'; i = i + 1;}
<BACKSLASH>\"                                  {BEGIN(STR); 
                                                 my_string[i] = '\"'; i = i + 1;}
<BACKSLASH>\\                                  {BEGIN(STR); 
                                                 my_string[i] = '\\'; i = i + 1;}

.               {output::errorLex(yylineno);}      


%%
void allocString()
{   
    my_string = (char*)malloc(1025 * sizeof(char));
}

void freeString()
{
    my_string[0] = '\0';
    free(my_string);
}