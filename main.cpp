#include "SemanticVisitor.hpp"
#include "llvmVisitor.hpp"
#include "nodes.hpp"
#include <iostream>

extern int yyparse();

extern std::shared_ptr<ast::Node> program;

int main(){
    
    yyparse();

    SemanticVisitor Visitor;

    program->accept(Visitor);

    LlvmVisitor llvmVisitor;

    program->accept(llvmVisitor);

    std::cout << llvmVisitor.buffer;

}