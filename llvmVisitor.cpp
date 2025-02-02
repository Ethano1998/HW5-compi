#include "output.hpp"
#include "llvmVisitor.hpp"
#include <iostream>
#include <fstream>


void LlvmVisitor::visit(ast::Num &node){
    if(get_var_exp == false){
        node.var = std::to_string(node.value);
    }else{
        code_buffer.emit(node.var);
    }
}

void LlvmVisitor::visit(ast::NumB &node){
    if(get_var_exp == false){
        node.var = std::to_string(node.value);
    }else{
        code_buffer.emit(node.var);
    }
}

void LlvmVisitor::visit(ast::String &node){
    if(get_var_exp == false){
        node.var = node.value;
    }else{
        code_buffer.emit(node.var);
    }
}

void LlvmVisitor::visit(ast::Bool &node){

}

void LlvmVisitor::visit(ast::ID &node){
    if(get_var_exp == false){
    std::string ptr_reg = "%ptr_" + std::to_string(node.offset);
    code_buffer.emit(ptr_reg + " = getelementptr [50 x i32], [50 x i32]* %Array, i32 0, i32 " + std::to_string(node.offset) );
    node.var = "%val_" + node.offset;
    code_buffer.emit( node.var + " = load i32, i32* " + ptr_reg);
    }else{
        code_buffer.emit(node.var);
    }
}

void LlvmVisitor::visit(ast::BinOp &node){
    if(get_var_exp == false){

    }else{
        code_buffer.emit(node.var);
    }
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
    for(const auto &formal : node.formals){
        formal->accept(*this);
    }
}

void LlvmVisitor::visit(ast::Formal &node){
    std::string return_type_formal = node.type->toString();
    if(decl_formal == true){
        code_buffer.emit(return_type_formal + ", ");
    }
    else{
        int offset_array = 50 + node.id->offset;
        code_buffer.emit("store " + return_type_formal + " %" + std::to_string(offset_array) + ", " + return_type_formal +"* getelementptr ([50 x i32], [50 x i32]* %Array,i32 0, i32" + std::to_string(node.id->offset) +")" );
    }
}

void LlvmVisitor::visit(ast::Statements &node){
    for(const auto &statement : node.statements){
        statement->accept(*this);
    }    
}

void LlvmVisitor::visit(ast::VarDecl &node){
    std::string return_type_var = node.type->toString();
    std::string reg_ptr = code_buffer.freshVar();
    code_buffer.emit(reg_ptr + " = getelementptr [50 x i32], [50 x i32]* %Array, i32 0, i32 " + std::to_string(node.id->offset));
    if(node.init_exp != nullptr){
        get_var_exp= false;
        node.init_exp->accept(*this);
        get_var_exp = true;
        code_buffer.emit("store i32 ");
        node.init_exp->accept(*this);
        code_buffer.emit(", i32* "+ reg_ptr);
    }
    

}

void LlvmVisitor::visit(ast::Assign &node){
    std::string reg_ptr = code_buffer.freshVar();
    code_buffer.emit(reg_ptr + " = getelementptr [50 x i32], [50 x i32]* %Array, i32 0, i32 " + std::to_string(node.id->offset) + "\n");
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
    decl_formal = true;
    node.formals->accept(*this);
    code_buffer.emit(") {\n");
    code_buffer.emit("%Array = alloca [50 x i32]\n");
    decl_formal = false;
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

