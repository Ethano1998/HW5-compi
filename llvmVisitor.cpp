#include "output.hpp"
#include "llvmVisitor.hpp"
#include <iostream>
#include <fstream>


void LlvmVisitor::visit(ast::Num &node){
    node.var = std::to_string(node.value);
}

void LlvmVisitor::visit(ast::NumB &node){
    node.var = std::to_string(node.value);
}

void LlvmVisitor::visit(ast::String &node){
        node.var = node.value;
}

void LlvmVisitor::visit(ast::Bool &node){
        if(node.value){
            node.var = code_buffer.freshVar();
            code_buffer.emit(node.var + " = add i1 1, 0\n");
        }    
        else{
            node.var = code_buffer.freshVar();
            code_buffer.emit(node.var + " = add i1 0, 0\n");
        }    
}

void LlvmVisitor::visit(ast::ID &node){
    std::string ptr_reg = code_buffer.freshVar();
    code_buffer.emit(ptr_reg + " = getelementptr [50 x i32], [50 x i32]* %Array, i32 0, i32 " + std::to_string(node.offset) + "\n");
    node.var = code_buffer.freshVar();
    code_buffer.emit( node.var + " = load i32, i32* " + ptr_reg + "\n");
    if(node.type == ast::BuiltInType::BOOL ){
        std::string reg_bool = code_buffer.freshVar();
        code_buffer.emit(reg_bool + "trunc i32 " + node.var + "to i1\n");
        node.var = reg_bool;
    }
}

void LlvmVisitor::visit(ast::BinOp &node){
    node.left->accept(*this);
    node.right->accept(*this);
    node.var = code_buffer.freshVar();
    switch (node.op)
    {
    case ast::BinOpType::ADD :
        code_buffer.emit(node.var + " = add i32 " + node.left->var + ", " + node.right->var + "\n");
        break;
    case ast::BinOpType::SUB :
        code_buffer.emit(node.var + " = sub i32 " + node.left->var + ", " + node.right->var + "\n");
        break;
    case ast::BinOpType::MUL :
        code_buffer.emit(node.var + " = mul i32 " + node.left->var + ", " + node.right->var + "\n");
        break;
    case ast::BinOpType::DIV : 
        code_buffer.emit(node.var + " = sdiv i32 " + node.left->var + ", " + node.right->var + "\n");
        break;       
    default:
        break;
    }
}

void LlvmVisitor::visit(ast::RelOp &node){
    node.left->accept(*this);
    node.right->accept(*this);
    node.var = code_buffer.freshVar();
    switch (node.op)
    {
    case ast::RelOpType::EQ :
        code_buffer.emit(node.var + " = icomp eq i32 " + node.left->var + ", " + node.right->var + "\n");
        break;
    case ast::RelOpType::GE :
        code_buffer.emit(node.var + " = icomp sge i32 " + node.left->var + ", " + node.right->var + "\n");
        break;    
    case ast::RelOpType::GT :
        code_buffer.emit(node.var + " = icomp sgt i32 " + node.left->var + ", " + node.right->var + "\n");
        break;
    case ast::RelOpType::LE :
        code_buffer.emit(node.var + " = icomp sle i32 " + node.left->var + ", " + node.right->var + "\n");
        break;
    case ast::RelOpType::LT :
        code_buffer.emit(node.var + " = icomp slt i32 " + node.left->var + ", " + node.right->var + "\n");
        break;
    case ast::RelOpType::NE :
        code_buffer.emit(node.var + " = icomp ne i32 " + node.left->var + ", " + node.right->var + "\n");
        break;        
    default:
        break;
    }
}

void LlvmVisitor::visit(ast::Not &node){
    node.exp->accept(*this);
    node.var = code_buffer.freshVar();
    code_buffer.emit(node.var + " = xor i1 " + node.exp->var + ", 1\n" );
}

void LlvmVisitor::visit(ast::And &node){
    node.left->accept(*this);
    node.right->accept(*this);
    node.var = code_buffer.freshVar();
    code_buffer.emit(node.var + " = and i1 " + node.left->var + ", " + node.right->var + "\n" );
}

void LlvmVisitor::visit(ast::Or &node){
    node.left->accept(*this);
    node.right->accept(*this);
    node.var = code_buffer.freshVar();
    code_buffer.emit(node.var + " = or i1 " + node.left->var + ", " + node.right->var + "\n" );
}

void LlvmVisitor::visit(ast::Type &node){
}


void LlvmVisitor::visit(ast::Cast &node){
}

void LlvmVisitor::visit(ast::Formals &node){
    for(const auto &formal : node.formals){
        formal->accept(*this);
        node.formal_list.append(formal->formal_buffer);
    }
    node.formal_list.pop_back();
}

void LlvmVisitor::visit(ast::Formal &node){
    std::string return_type_formal = node.type->toString();
    if(decl_formal == true){
        node.var= code_buffer.freshVar();
        node.formal_buffer = return_type_formal + " " + node.var + ",";
        
    }
    else{
        int offset_array = 50 + node.id->offset;
        if(node.type->type == ast::BuiltInType::BOOL ){
            std::string bool_to_i32 = code_buffer.freshVar();
            code_buffer.emit(bool_to_i32 + " = zext i1 " + node.var + " to i32\n");
            node.var = bool_to_i32;
        }
        code_buffer.emit("store i32 " + node.var + ", i32* getelementptr ([50 x i32], [50 x i32]* %Array,i32 0, i32" + std::to_string(offset_array) +")\n" );
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
    node.id->var = reg_ptr;
    code_buffer.emit(reg_ptr + " = getelementptr [50 x i32], [50 x i32]* %Array, i32 0, i32 " + std::to_string(node.id->offset) + "\n");
    if(node.init_exp != nullptr){
        node.init_exp->accept(*this);
        if(node.init_exp->type == ast::BuiltInType::BOOL){
            std::string bool_to_i32 = code_buffer.freshVar();
            code_buffer.emit(bool_to_i32 + " = zext i1 " + node.init_exp->var + " to i32\n");
            node.init_exp->var = bool_to_i32;
        }
        code_buffer.emit("store i32 " + node.init_exp->var +", i32* "+ reg_ptr + "\n");
    } else {
        if(node.return_type == ast::BuiltInType::INT || node.return_type == ast::BuiltInType::BYTE || node.return_type == ast::BuiltInType::BOOL){
            code_buffer.emit("store i32 0, i32*" + reg_ptr + "\n" );
        }
    }
    

}

void LlvmVisitor::visit(ast::Assign &node){
    std::string reg_ptr = code_buffer.freshVar();
    code_buffer.emit(reg_ptr + " = getelementptr [50 x i32], [50 x i32]* %Array, i32 0, i32 " + std::to_string(node.id->offset) + "\n");
    node.exp->accept(*this);
    if(node.exp->type == ast::BuiltInType::BOOL) {
            std::string bool_to_i32 = code_buffer.freshVar();
            code_buffer.emit(bool_to_i32 + " = zext i1 " + node.exp->var + " to i32\n");
            node.exp->var = bool_to_i32;
        }
    code_buffer.emit("store i32 " + node.exp->var +", i32* "+ reg_ptr + "\n");


}

void LlvmVisitor::visit(ast::Call &node){
    node.var = code_buffer.freshVar();
    node.args->accept(*this);
    std::string arguments;
    for(auto const &arg : node.args->args_list){
        arguments.append(arg);
    }
    arguments.pop_back();
    if(node.func_id->type == ast::BuiltInType::BOOL){
        code_buffer.emit(node.var + " = i1 @" + node.func_id->value + "("+ arguments +")\n");
    }
}

void LlvmVisitor::visit(ast::ExpList &node){
    for(const auto &exp : node.exps){
        exp->accept(*this);
        if(exp->type == ast::BuiltInType::BOOL){
            node.args_list.push_back("i1 "+ exp->var +",");
        }else{
            node.args_list.push_back("i32 "+ exp->var +",");
        }
    }
}

void LlvmVisitor::visit(ast::Return &node){
}

void LlvmVisitor::visit(ast::Break &node){
}

void LlvmVisitor::visit(ast::Continue &node){
}

void LlvmVisitor::visit(ast::If &node){
    node.condition->accept(*this);
    std::string reg_cond = node.condition->var;
    std::string if_true_label = code_buffer.freshLabel();
    std::string if_false_label = code_buffer.freshLabel();
    std::string if_end_label = code_buffer.freshLabel();
    code_buffer.emit("br i1 " + reg_cond + ", label " + if_true_label + ", label" + if_false_label + "\n");
    code_buffer.emitLabel(if_true_label);
    node.then->accept(*this);
    code_buffer.emit("br label " + if_end_label + "\n");
    code_buffer.emitLabel(if_false_label);
    if(node.otherwise){
        node.otherwise->accept(*this);
    }
    code_buffer.emit("br label " + if_end_label + "\n");

    code_buffer.emitLabel(if_end_label);
}

void LlvmVisitor::visit(ast::While &node){
}

void LlvmVisitor::visit(ast::FuncDecl &node){
    std::string return_type_func = node.return_type->toString();
    decl_formal = true;
    node.formals->accept(*this);
    code_buffer.emit("\ndefine "+ return_type_func +" @" + node.id->value + "(" + node.formals->formal_list + "){\n");
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



