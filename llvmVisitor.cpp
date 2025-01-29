#include "output.hpp"
#include "llvmVisitor.hpp"
#include <iostream>
#include <fstream>


void LlvmVisitor::visit(ast::Num &node){
}

void LlvmVisitor::visit(ast::NumB &node){
}

void LlvmVisitor::visit(ast::String &node){
}

void LlvmVisitor::visit(ast::Bool &node){
}

void LlvmVisitor::visit(ast::ID &node){
}

void LlvmVisitor::visit(ast::BinOp &node){
}

void LlvmVisitor::visit(ast::RelOp &node){
}

void LlvmVisitor::visit(ast::Not &node){
}

void LlvmVisitor::visit(ast::And &node){
}

void LlvmVisitor::visit(ast::Or &node){
}

void LlvmVisitor::visit(ast::Type &node){
}

void LlvmVisitor::visit(ast::Cast &node){
}

void LlvmVisitor::visit(ast::Formals &node){
    int i = 49;
    for(const auto &formal : node.formals){
                formal->id->offset= i;
                formal->accept(*this);
                i--;
            }
}

void LlvmVisitor::visit(ast::Formal &node){
    std::string return_type_formal = node.type->toString();
    if(decl_formal == true){
        code_buffer.emit(return_type_formal + ", ");
    }
    else{
        int offset_array = 49 - node.id->offset;
        code_buffer.emit("store " + return_type_formal + " %" + std::to_string(offset_array) + ", " + return_type_formal +"* getelementptr ([50 x i32], [50 x i32]* %Array,i32 0, i32" + std::to_string(node.id->offset) +")" );
    }
}

void LlvmVisitor::visit(ast::Statements &node){
    for(const auto &statement : node.statements){
            statement->accept(*this);
            }    
}

void LlvmVisitor::visit(ast::VarDecl &node){
    code_buffer.emit(code_buffer.freshVar());
    
}

void LlvmVisitor::visit(ast::Assign &node){
}

void LlvmVisitor::visit(ast::Call &node){
}

void LlvmVisitor::visit(ast::ExpList &node){
}

void LlvmVisitor::visit(ast::Return &node){
}

void LlvmVisitor::visit(ast::Break &node){
}

void LlvmVisitor::visit(ast::Continue &node){
}

void LlvmVisitor::visit(ast::If &node){
}

void LlvmVisitor::visit(ast::While &node){
}

void LlvmVisitor::visit(ast::FuncDecl &node){
    std::string return_type_func = node.return_type->toString();
    code_buffer.emit("\ndefine "+ return_type_func +" @" + node.id->value + "(");
    node.formals->accept(*this);
    code_buffer.emit(") {\n");
    code_buffer.emit("%Array = alloca [50 x i32]\n");
    node.formals->accept(*this);
    node.body->accept(*this);
    code_buffer.emit("}\n");
   
}

void LlvmVisitor::visit(ast::Funcs &node){
    std::ifstream file("print_functions.llvm");
    std::string line;
    while (std::getline(file, line)) {
        code_buffer.emit(line);
    }
    file.close();

    for (auto &func : node.funcs) {
        func->accept(*this);
    }
}

