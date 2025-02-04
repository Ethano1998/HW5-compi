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
    std::string copy;
    for (char ch : node.value) {
        if (ch == '\n') {
            copy += "\\n";
        }
        else if (ch == '\t') {
            copy += "\\t";
        }
        else if (ch == '\r') {
            copy += "\\r";
        }
        else{
            copy += ch;
        }
    }
    std::string var_str = code_buffer.emitString(copy);
    node.var = code_buffer.freshVar();
    code_buffer.emit(node.var + " = getelementptr [" + std::to_string(copy.length() + 1) + " x i8], [" + std::to_string(copy.length() + 1) + " x i8]* " + var_str + ", i32 0, i32 0");        
}

void LlvmVisitor::visit(ast::Bool &node){
    if(node.value){
        node.var = code_buffer.freshVar();
        code_buffer.emit(node.var + " = add i1 1, 0");
    }    
    else{
        node.var = code_buffer.freshVar();
        code_buffer.emit(node.var + " = add i1 0, 0");
    }    
}

void LlvmVisitor::visit(ast::ID &node){
    std::string ptr_reg = code_buffer.freshVar();
    code_buffer.emit(ptr_reg + " = getelementptr [50 x i32], [50 x i32]* %Array" + std::to_string(function_count) + ", i32 0, i32 " + node.offset_to_string());
    node.var = code_buffer.freshVar();
    code_buffer.emit( node.var + " = load i32, i32* " + ptr_reg);
    if(node.type == ast::BuiltInType::BOOL ){
        std::string reg_bool = code_buffer.freshVar();
        code_buffer.emit(reg_bool + " = trunc i32 " + node.var + " to i1");
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
        code_buffer.emit(node.var + " = add i32 " + node.left->var + ", " + node.right->var);
        break;
    case ast::BinOpType::SUB :
        code_buffer.emit(node.var + " = sub i32 " + node.left->var + ", " + node.right->var);
        break;
    case ast::BinOpType::MUL :
        code_buffer.emit(node.var + " = mul i32 " + node.left->var + ", " + node.right->var);
        break;     
    default:
        break;
    }
    if(node.op == ast::BinOpType::DIV){
        std::string reg_check = code_buffer.freshVar();
        code_buffer.emit(reg_check + " = icmp eq i32 0, " + node.right->var);
        std::string if_true_label = code_buffer.freshLabel();
        std::string if_false_label = code_buffer.freshLabel();
        std::string if_end_label = code_buffer.freshLabel();
        code_buffer.emit("br i1 " + reg_check + ", label " + if_true_label + ", label " + if_false_label);
        code_buffer.emitLabel(if_true_label);
        std::string string_var = code_buffer.freshVar();
        code_buffer.emit(string_var + " = getelementptr [23 x i8], [23 x i8]* @.str0, i32 0, i32 0");    
        code_buffer.emit("call void @print(i8* " + string_var + ")");
        code_buffer.emit("call void @exit(i32 1)");
        code_buffer.emit("br label " + if_end_label);
        code_buffer.emitLabel(if_false_label);
        code_buffer.emit("br label " + if_end_label);
        code_buffer.emitLabel(if_end_label);

        code_buffer.emit(node.var + " = sdiv i32 " + node.left->var + ", " + node.right->var);
    }
    if(node.type == ast::BuiltInType::BYTE){
        std::string trunc = code_buffer.freshVar();
        code_buffer.emit(trunc + " = trunc i32 " + node.var + " to i8");
        std::string byte_to_i32 = code_buffer.freshVar();
        code_buffer.emit(byte_to_i32 + " = zext i8 " + trunc + " to i32");
        node.var = byte_to_i32;
    }
}

void LlvmVisitor::visit(ast::RelOp &node){
    node.left->accept(*this);
    node.right->accept(*this);
    node.var = code_buffer.freshVar();
    switch (node.op)
    {
    case ast::RelOpType::EQ :
        code_buffer.emit(node.var + " = icmp eq i32 " + node.left->var + ", " + node.right->var);
        break;
    case ast::RelOpType::GE :
        code_buffer.emit(node.var + " = icmp sge i32 " + node.left->var + ", " + node.right->var);
        break;    
    case ast::RelOpType::GT :
        code_buffer.emit(node.var + " = icmp sgt i32 " + node.left->var + ", " + node.right->var);
        break;
    case ast::RelOpType::LE :
        code_buffer.emit(node.var + " = icmp sle i32 " + node.left->var + ", " + node.right->var);
        break;
    case ast::RelOpType::LT :
        code_buffer.emit(node.var + " = icmp slt i32 " + node.left->var + ", " + node.right->var);
        break;
    case ast::RelOpType::NE :
        code_buffer.emit(node.var + " = icmp ne i32 " + node.left->var + ", " + node.right->var);
        break;        
    default:
        break;
    }
}

void LlvmVisitor::visit(ast::Not &node){
    node.exp->accept(*this);
    node.var = code_buffer.freshVar();
    code_buffer.emit(node.var + " = xor i1 " + node.exp->var + ", 1" );
}

void LlvmVisitor::visit(ast::And &node){
    node.left->accept(*this);
    std::string reg_cond = node.left->var;
    std::string if_true_label = code_buffer.freshLabel();
    std::string if_false_label = code_buffer.freshLabel();
    std::string both_true_label = code_buffer.freshLabel();
    std::string if_end_label = code_buffer.freshLabel();
    code_buffer.emit("br i1 " + reg_cond + ", label " + if_true_label + ", label " + if_false_label);
    code_buffer.emitLabel(if_true_label);
    node.right->accept(*this);
    reg_cond = node.right->var;
    code_buffer.emit("br i1 " + reg_cond + ", label " + both_true_label + ", label " + if_false_label);
    code_buffer.emitLabel(both_true_label);
    std::string truevar = code_buffer.freshVar();
    code_buffer.emit(truevar + " = add i1 1, 0");
    code_buffer.emit("br label " + if_end_label);
    code_buffer.emitLabel(if_false_label);
    std::string falsevar = code_buffer.freshVar();
    code_buffer.emit(falsevar + " = add i1 0, 0");
    code_buffer.emit("br label " + if_end_label);
    code_buffer.emitLabel(if_end_label);
    node.var = code_buffer.freshVar();
    code_buffer.emit(node.var + " = phi i1 [" + truevar + ", %" + both_true_label.substr(1) + "], [" + falsevar + ", %" + if_false_label.substr(1) + "]");
}

void LlvmVisitor::visit(ast::Or &node){
    node.left->accept(*this);
    std::string reg_cond = node.left->var;
    std::string if_true_label = code_buffer.freshLabel();
    std::string if_false_label = code_buffer.freshLabel();
    std::string both_false_label = code_buffer.freshLabel();
    std::string if_end_label = code_buffer.freshLabel();
    code_buffer.emit("br i1 " + reg_cond + ", label " + if_true_label + ", label " + if_false_label);
    code_buffer.emitLabel(if_true_label);
    std::string truevar = code_buffer.freshVar();
    code_buffer.emit(truevar + " = add i1 1, 0");
    code_buffer.emit("br label " + if_end_label);
    code_buffer.emitLabel(if_false_label);
    node.right->accept(*this);
    reg_cond = node.right->var;
    code_buffer.emit("br i1 " + reg_cond + ", label " + if_true_label + ", label " + both_false_label);
    code_buffer.emitLabel(both_false_label);
    std::string falsevar = code_buffer.freshVar();
    code_buffer.emit(falsevar + " = add i1 0, 0");
    code_buffer.emit("br label " + if_end_label);
    code_buffer.emitLabel(if_end_label);
    node.var = code_buffer.freshVar();
    code_buffer.emit(node.var + " = phi i1 [" + truevar + ", %" + if_true_label.substr(1) + "], [" + falsevar + ", %" + both_false_label.substr(1) + "]");
}

void LlvmVisitor::visit(ast::Type &node){
}


void LlvmVisitor::visit(ast::Cast &node){
    node.exp->accept(*this);
    node.var = node.exp->var;
}

void LlvmVisitor::visit(ast::Formals &node){
    for(const auto &formal : node.formals){
        formal->accept(*this);
        node.formal_list.append(formal->formal_buffer);
    }
    if (!node.formal_list.empty()) {
    node.formal_list.pop_back();
    }
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
            code_buffer.emit(bool_to_i32 + " = zext i1 " + node.var + " to i32");
            node.var = bool_to_i32;
        }
        std::string reg_ptr = code_buffer.freshVar();
        code_buffer.emit(reg_ptr + " = getelementptr [50 x i32], [50 x i32]* %Array" + std::to_string(function_count) + ", i32 0, i32 " + std::to_string(offset_array));
        code_buffer.emit("store i32 " + node.var +", i32* "+ reg_ptr);
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
    code_buffer.emit(reg_ptr + " = getelementptr [50 x i32], [50 x i32]* %Array" + std::to_string(function_count) + ", i32 0, i32 " + node.id->offset_to_string());
    if(node.init_exp != nullptr){
        node.init_exp->accept(*this);
        if(node.init_exp->type == ast::BuiltInType::BOOL){
            std::string bool_to_i32 = code_buffer.freshVar();
            code_buffer.emit(bool_to_i32 + " = zext i1 " + node.init_exp->var + " to i32");
            node.init_exp->var = bool_to_i32;
        }
        if(node.init_exp->type == ast::BuiltInType::BYTE){
            std::string trunc = code_buffer.freshVar();
            code_buffer.emit(trunc + " = trunc i32 " + node.init_exp->var + " to i8");
            std::string byte_to_i32 = code_buffer.freshVar();
            code_buffer.emit(byte_to_i32 + " = zext i8 " + trunc + " to i32");
            node.init_exp->var = byte_to_i32;
        }
        code_buffer.emit("store i32 " + node.init_exp->var +", i32* "+ reg_ptr);
    } else {
            code_buffer.emit("store i32 0, i32*" + reg_ptr);
    }
}

void LlvmVisitor::visit(ast::Assign &node){
    std::string reg_ptr = code_buffer.freshVar();
    code_buffer.emit(reg_ptr + " = getelementptr [50 x i32], [50 x i32]* %Array" + std::to_string(function_count) + ", i32 0, i32 " + node.id->offset_to_string());
    node.exp->accept(*this);
    if(node.exp->type == ast::BuiltInType::BOOL) {
        std::string bool_to_i32 = code_buffer.freshVar();
        code_buffer.emit(bool_to_i32 + " = zext i1 " + node.exp->var + " to i32");
        node.exp->var = bool_to_i32;
    }
    if(node.exp->type == ast::BuiltInType::BYTE){
        std::string trunc = code_buffer.freshVar();
        code_buffer.emit(trunc + " = trunc i32 " + node.exp->var + " to i8");
        std::string byte_to_i32 = code_buffer.freshVar();
        code_buffer.emit(byte_to_i32 + " = zext i8 " + trunc + " to i32");
        node.exp->var = byte_to_i32;
        }
    code_buffer.emit("store i32 " + node.exp->var +", i32* "+ reg_ptr);


}

void LlvmVisitor::visit(ast::Call &node){
    node.var = code_buffer.freshVar();
    node.args->accept(*this);
    std::string arguments;
    for(auto const &arg : node.args->args_list){
        arguments.append(arg);
    }
    if(!arguments.empty()){
        arguments.pop_back();
    }
    if(node.func_id->type == ast::BuiltInType::BOOL){
        code_buffer.emit(node.var + " = call i1 @" + node.func_id->value + "("+ arguments +")");
    }
    else if(node.func_id->type == ast::BuiltInType::INT || node.func_id->type == ast::BuiltInType::BYTE){
        code_buffer.emit(node.var + " = call i32 @" + node.func_id->value + "("+ arguments +")");
    }
    else{
        code_buffer.emit("call void @" + node.func_id->value + "("+ arguments +")");
    }
}

void LlvmVisitor::visit(ast::ExpList &node){
    for(const auto &exp : node.exps){
        exp->accept(*this);
        if(exp->type == ast::BuiltInType::BOOL){
            node.args_list.push_back("i1 "+ exp->var +",");
        }
        else if(exp->type == ast::BuiltInType::STRING){
            node.args_list.push_back("i8* "+ exp->var +",");
        }
        else{
            node.args_list.push_back("i32 "+ exp->var +",");
        }
    }
}

void LlvmVisitor::visit(ast::Return &node){
    if(node.exp != nullptr){
        node.exp->accept(*this);
        if(node.exp->type == ast::BuiltInType::BOOL){
            code_buffer.emit("ret i1 " + node.exp->var);
        }else{
            code_buffer.emit("ret i32 " + node.exp->var);
        }
    }
    else{
        code_buffer.emit("ret void");
    }
}

void LlvmVisitor::visit(ast::Break &node){
    std::string exit_label = exit_labels.back();
    code_buffer.emit("br label "+ exit_label);
}

void LlvmVisitor::visit(ast::Continue &node){
    std::string entry_label = entry_labels.back();
    code_buffer.emit("br label "+ entry_label);
}

void LlvmVisitor::visit(ast::If &node){
    node.condition->accept(*this);
    std::string reg_cond = node.condition->var;
    std::string if_true_label = code_buffer.freshLabel();
    std::string if_false_label = code_buffer.freshLabel();
    std::string if_end_label = code_buffer.freshLabel();
    code_buffer.emit("br i1 " + reg_cond + ", label " + if_true_label + ", label " + if_false_label);
    code_buffer.emitLabel(if_true_label);
    node.then->accept(*this);
    code_buffer.emit("br label " + if_end_label);
    code_buffer.emitLabel(if_false_label);
    if(node.otherwise){
        node.otherwise->accept(*this);
    }
    code_buffer.emit("br label " + if_end_label);

    code_buffer.emitLabel(if_end_label);
}

void LlvmVisitor::visit(ast::While &node){
    std::string whileEntry = code_buffer.freshLabel();
    std::string whileBody = code_buffer.freshLabel();
    std::string whileExit = code_buffer.freshLabel();
    exit_labels.push_back(whileExit);
    entry_labels.push_back(whileEntry);

    code_buffer.emit("br label " + whileEntry);
    code_buffer.emitLabel(whileEntry);
    node.condition->accept(*this);
    code_buffer.emit("br i1 " + node.condition->var + ", label " + whileBody + ", label " + whileExit);

    code_buffer.emitLabel(whileBody);
    node.body->accept(*this);
    code_buffer.emit("br label " + whileEntry);  

    code_buffer.emitLabel(whileExit);
    if(!exit_labels.empty()){
        exit_labels.pop_back();
    }
    if(!entry_labels.empty()){
        entry_labels.pop_back();
    }
}

void LlvmVisitor::visit(ast::FuncDecl &node){
    function_count++;
    std::string return_type_func = node.return_type->toString();
    decl_formal = true;
    node.formals->accept(*this);
    const char* f_list = node.formals->formal_list.c_str();
    code_buffer.emit("\ndefine "+ return_type_func +" @" + node.id->value + "(" + f_list + "){");
    code_buffer.emit("%Array" + std::to_string(function_count) + " = alloca [50 x i32]");
    decl_formal = false;
    node.formals->accept(*this);
    node.body->accept(*this);
    if(node.return_type->type == ast::BuiltInType::VOID){
        code_buffer.emit("ret void");
    }
    else if(node.return_type->type == ast::BuiltInType::BOOL){
        code_buffer.emit("ret i1 0");
    }else{
        code_buffer.emit("ret i32 0");
    }
    code_buffer.emit("}\n");
}

void LlvmVisitor::visit(ast::Funcs &node){
    std::ifstream file("print_functions.llvm");
    std::string line;
    while (std::getline(file, line)) {
        code_buffer.emit(line);
    }
    file.close();
    code_buffer.emitString("Error division by zero");

    for (auto &func : node.funcs) {
        func->accept(*this);
    }
}



