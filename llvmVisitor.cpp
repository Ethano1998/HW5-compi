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
}

void LlvmVisitor::visit(ast::Formal &node){
}

void LlvmVisitor::visit(ast::Statements &node){
}

void LlvmVisitor::visit(ast::VarDecl &node){
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

