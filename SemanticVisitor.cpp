#include "output.hpp"
#include "SemanticVisitor.hpp"
#include <iostream>

ast::BuiltInType SemanticVisitor::check_assign(std::shared_ptr<ast::Exp> &node){
    return node->type;
}

void SemanticVisitor::visit(ast::Funcs &node){
    //appel du visitor sur toute les functions pour les declarer
    scopePrinter.emitFunc("print", ast::BuiltInType::VOID, {ast::BuiltInType::STRING});
    scopePrinter.emitFunc("printi", ast::BuiltInType::VOID, {ast::BuiltInType::INT});
    declaration_function = true;
    for (const auto &func : node.funcs){
        func->accept(*this);
    }
    std::shared_ptr<SymbolTable> functable = globalSymbolTable.getTable();
    std::shared_ptr<SymbolEntry> main = functable->findEntry("main");
    if(!main)
        output::errorMainMissing();
    else if(std::dynamic_pointer_cast<FuncSymbolEntry>(main)->returnType != "VOID")
        output::errorMainMissing();
    else if(std::dynamic_pointer_cast<FuncSymbolEntry>(main)->paramTypes.size() != 0)
        output::errorMainMissing();
    
    //continuation de la verification semantique apres avoir declaree toute les fonctions
    declaration_function = false;
    for (const auto &func : node.funcs){
        func->accept(*this);
    }
}

void SemanticVisitor::visit(ast::FuncDecl &node){
    if(declaration_function){


        //declaration des vecteurs pour les parametres
        std::vector<ast::BuiltInType> parameters;
        std::vector<std::string> parameters_name;

        //casting des parametres en string 
        for(const auto &params : node.formals->formals ){
        parameters.push_back(params->type->type);
        parameters_name.push_back(toString(params->type->type));
        }

        //casting du return type en string
        std::string returnType = toString(node.return_type->type);
     

        //definition du contexte
        setContext(Context::DECLARATION);

        //continuation du SemanticVisitor pour la verification de la function
        node.return_type->accept(*this);
        node.id->accept(*this);

        //recuperation de la table des functions
        std::shared_ptr<SymbolTable> functable = globalSymbolTable.getTable();  

        //ajout de la function a la symboltable des functions 
        functable->addFunc(node.id->value,parameters_name,returnType);

        node.formals->accept(*this);

          

        

        //emission de la function dans le scope global
        scopePrinter.emitFunc(node.id->value,node.return_type->type,parameters);

    }
    else {
    //debut de la scope de la function
    scopePrinter.beginScope();

    setContext(Context::REFERENCE_VAR);
    //creation de la table pour le scope de la function
    std::shared_ptr<SymbolTable> func_table = std::make_shared<SymbolTable>() ;
    globalSymbolTable.addTable(func_table);
    from_funcdecl = true;
    node.body->return_type= node.return_type->type;

    //route du visitor vers statement pour la scope de la function
    node.formals->accept(*this);
    node.body->accept(*this);

    //supression de la table et fin du scope de la function
    globalSymbolTable.popTable();
    scopePrinter.endScope();
    }
}

void SemanticVisitor::visit(ast::Type &node){
//verification du type
    auto check = toString(node.type);
    if(check == "unknown"){
        //printf("visit type");
        output::errorMismatch(node.line);
    }    
}

void SemanticVisitor::visit(ast::ID &node){
    switch(current_context){
        case Context::DECLARATION : {

            //verification si la function est deja declaree
            if(globalSymbolTable.findEntry(node.value))
                output::errorDef(node.line,node.value);
            break;
        }
        case Context::REFERENCE_FUNC :{
            //recuperation de la table dans symbol table
            std::shared_ptr<SymbolTable> Table = globalSymbolTable.getTable();

            //verification si la function est declaree en tant que variable
            if(std::dynamic_pointer_cast<VarSymbolEntry>(globalSymbolTable.findEntry(node.value)))
                output::errorDefAsVar(node.line,node.value);

            //recuperation de la table des functions
            std::shared_ptr<SymbolTable> functionTable = globalSymbolTable.getFunctionTable();
            
            //verification si la function est declaree
            if(!(functionTable->findEntry(node.value)))
                output::errorUndefFunc(node.line,node.value);
            break;
        }
        case Context::REFERENCE_VAR :{
            //recuperation de la table des functions
            std::shared_ptr<SymbolTable> functionTable = globalSymbolTable.getFunctionTable();
            
            //verification si la variable est declaree en tant que fonction
            if(std::dynamic_pointer_cast<FuncSymbolEntry>(functionTable->findEntry(node.value)))
                output::errorDefAsFunc(node.line,node.value);
            
          
            //verification si variable est declaree
            if(!(globalSymbolTable.findEntry(node.value)))
                output::errorUndef(node.line,node.value);
            else{
                auto get_type = std::dynamic_pointer_cast<VarSymbolEntry>(globalSymbolTable.findEntry(node.value))->type;
                node.type = toBuiltInType(get_type);
                node.offset = std::dynamic_pointer_cast<VarSymbolEntry>(globalSymbolTable.findEntry(node.value))->offset;
            }    
                
            break;    
        }       
    }
}

void SemanticVisitor::visit(ast::Formals &node){
    if(declaration_function){
        for(const auto &formal : node.formals){
            if(globalSymbolTable.findEntry(formal->id->value))
                output::errorDef(node.line,formal->id->value);
            std::string doublon = formal->id->value;
            for(const auto &check : node.formals){
                if(check->id->value == doublon && check != formal)
                    output::errorDef(check->id->line,doublon);
            }
            formal->type->accept(*this);
        }
    }
    else{
        for(const auto &formal : node.formals){
            formal->accept(*this);
        }
    }
}

void SemanticVisitor::visit(ast::Formal &node){
    auto table = globalSymbolTable.getTable();
    table->addParam(node.id->value,toString(node.type->type));
    scopePrinter.emitVar(node.id->value,node.type->type,table->param_offset);
}

void SemanticVisitor::visit(ast::Statements &node){
    if(from_funcdecl){
        from_funcdecl = false;
        for(const auto &statement : node.statements){
            statement->return_type = node.return_type;
            statement->accept(*this);
        }
    } else{
        scopePrinter.beginScope();
        std::shared_ptr<SymbolTable> table = std::make_shared<SymbolTable>() ;
        globalSymbolTable.addTable(table);
        for(const auto &statement : node.statements){
            statement->return_type = node.return_type;
            statement->accept(*this);
        }
        scopePrinter.endScope();
        globalSymbolTable.popTable();
    }    
}

void SemanticVisitor::visit(ast::VarDecl &node){
    current_context = Context::DECLARATION;
    node.id->accept(*this);
    node.type->accept(*this);
    if(node.init_exp){
        setContext(Context::REFERENCE_VAR);
        node.init_exp->accept(*this);
        ast::BuiltInType type_check = check_assign(node.init_exp);
        if(node.type->type == ast::BuiltInType::INT && !(type_check == ast::BuiltInType::INT || type_check == ast::BuiltInType::BYTE)){
            output::errorMismatch(node.line);
        }    
        else if(node.type->type != ast::BuiltInType::INT && (node.type->type != type_check)){
            output::errorMismatch(node.line);    
        }    
    }
    auto table = globalSymbolTable.getTable();
    scopePrinter.emitVar(node.id->value,node.type->type,table->get_offset());
    table->addVar(node.id->value,toString(node.type->type));
    node.id->offset = std::dynamic_pointer_cast<VarSymbolEntry>(globalSymbolTable.findEntry(node.id->value))->offset;
    setContext(Context::REFERENCE_VAR);
}

void SemanticVisitor::visit(ast::Assign &node){
    current_context = Context::REFERENCE_VAR;
    node.id->accept(*this);
    node.exp->accept(*this);
    ast::BuiltInType type_check = check_assign(node.exp);
    std::string check = toString(type_check);
    auto variable = std::dynamic_pointer_cast<VarSymbolEntry>(globalSymbolTable.findEntry(node.id->value));
    if(variable->type == "INT" && !(type_check == ast::BuiltInType::INT || type_check == ast::BuiltInType::BYTE))
            output::errorMismatch(node.line);
    else if( variable->type != "INT" && (check != variable->type))
        output::errorMismatch(node.line);   
}

void SemanticVisitor::visit(ast::If &node){
    scopePrinter.beginScope();
    std::shared_ptr<SymbolTable> table = std::make_shared<SymbolTable>();
    globalSymbolTable.addTable(table);
    node.condition->accept(*this);  //need to check if there is no problem with the condition
    if(node.condition->type !=  ast::BuiltInType::BOOL){
        output::errorMismatch(node.condition->line);
    }
    node.then->return_type = node.return_type;
    node.then->accept(*this);
    scopePrinter.endScope();
    globalSymbolTable.popTable();
    if(node.otherwise){
        scopePrinter.beginScope();
        std::shared_ptr<SymbolTable> table = std::make_shared<SymbolTable>();
        globalSymbolTable.addTable(table);
        node.otherwise->return_type = node.return_type;
        node.otherwise->accept(*this);
        scopePrinter.endScope();
        globalSymbolTable.popTable();
    }
}

void SemanticVisitor::visit(ast::While &node){
    scopePrinter.beginScope();
    std::shared_ptr<SymbolTable> table = std::make_shared<SymbolTable>() ;
    globalSymbolTable.addTable(table);
    node.condition->accept(*this);  //need to check if there is no problem with the condition
    if(node.condition->type !=  ast::BuiltInType::BOOL)
        output::errorMismatch(node.condition->line);
    globalSymbolTable.is_loop++;
    node.body->return_type = node.return_type;
    node.body->accept(*this);
    scopePrinter.endScope();
    globalSymbolTable.popTable();
    globalSymbolTable.is_loop--;
}

void SemanticVisitor::visit(ast::Break &node){
    if(globalSymbolTable.is_loop == 0)
        output::errorUnexpectedBreak(node.line);
}

void SemanticVisitor::visit(ast::Continue &node){
    if(globalSymbolTable.is_loop == 0)
        output::errorUnexpectedContinue(node.line);
}

void SemanticVisitor::visit(ast::Call &node){
    current_context = Context::REFERENCE_FUNC;
    node.func_id->accept(*this);
    std::shared_ptr<SymbolTable> functionTable = globalSymbolTable.getFunctionTable();
    std::shared_ptr<FuncSymbolEntry> entry = std::dynamic_pointer_cast<FuncSymbolEntry>(functionTable->findEntry(node.func_id->value));
    std::string returnType = entry->returnType;
    node.type = toBuiltInType(returnType);
    //now we re checking the types of the arguments
    std::string name = node.func_id->value;
    std::vector<std::string> expectedtypes = entry->paramTypes;
    std::vector<std::shared_ptr<ast::Exp>> types = node.args->exps;
    if(expectedtypes.size() != types.size())
        output::errorPrototypeMismatch(node.line,node.func_id->value,expectedtypes);
    current_context = Context::REFERENCE_VAR;
    node.args->accept(*this);
    for(int i = 0; i < expectedtypes.size(); i++){
        if(name == "printi" && expectedtypes[i] == "INT" && types[i]->type == ast::BuiltInType::BYTE)
            continue;
        if(expectedtypes[i] != toString(types[i]->type))
            output::errorPrototypeMismatch(node.line,node.func_id->value,expectedtypes);
    }
}

void SemanticVisitor::visit(ast::ExpList &node){
    for(auto &exp : node.exps){
        exp->accept(*this);
    }
}

void SemanticVisitor::visit(ast::BinOp &node){
    node.left->accept(*this); 
    node.right->accept(*this);
    if(!(node.left->type == ast::BuiltInType::INT || node.left->type == ast::BuiltInType::BYTE))
        output::errorMismatch(node.line);
    if(!(node.right->type == ast::BuiltInType::INT || node.right->type == ast::BuiltInType::BYTE))
        output::errorMismatch(node.line);
    if(node.left->type == ast::BuiltInType::INT || node.right->type == ast::BuiltInType::INT)
        node.type = ast::BuiltInType::INT;
    else
        node.type = ast::BuiltInType::BYTE;

}

void SemanticVisitor::visit(ast::Return &node){
    if(!node.exp){
        if(node.return_type != ast::BuiltInType::VOID){
            output::errorMismatch(node.line);
        }
    }
    else{
        node.exp->accept(*this);
        if(node.return_type == ast::BuiltInType::VOID){
            output::errorMismatch(node.line);
        }
        else if(node.return_type == ast::BuiltInType::INT){
            if(!(node.exp->type == ast::BuiltInType::INT || node.exp->type == ast::BuiltInType::BYTE))
                output::errorMismatch(node.line);
        }
        else if(node.return_type != check_assign(node.exp))
            output::errorMismatch(node.line);
    }
}

void SemanticVisitor::visit(ast::Num &node){
    node.type = ast::BuiltInType::INT;
}

void SemanticVisitor::visit(ast::NumB &node){
    if(node.value > 255)
        output::errorByteTooLarge(node.line,node.value);
    node.type = ast::BuiltInType::BYTE;
}

void SemanticVisitor::visit(ast::String &node){
    node.type = ast::BuiltInType::STRING;
}

void SemanticVisitor::visit(ast::Bool &node){
    node.type = ast::BuiltInType::BOOL;
}

void SemanticVisitor::visit(ast::RelOp &node){
    node.left->accept(*this);
    node.right->accept(*this);
    if(!(node.left->type == ast::BuiltInType::INT || node.left->type == ast::BuiltInType::BYTE)){
        output::errorMismatch(node.line);
    }
    if(!(node.right->type == ast::BuiltInType::INT || node.right->type == ast::BuiltInType::BYTE)){
        output::errorMismatch(node.line);
    }
    node.type = ast::BuiltInType::BOOL;
}

void SemanticVisitor::visit(ast::Cast &node){
    node.exp->accept(*this);
    node.target_type->accept(*this);
    if(!(node.exp->type == ast::BuiltInType::INT || node.exp->type == ast::BuiltInType::BYTE)){
        output::errorMismatch(node.line);
    }
    if(!(node.target_type->type == ast::BuiltInType::INT || node.target_type->type == ast::BuiltInType::BYTE)){
        output::errorMismatch(node.line);
    }
    node.type = node.target_type->type;
}

void SemanticVisitor::visit(ast::Not &node){
    node.exp->accept(*this);
    if(node.exp->type != ast::BuiltInType::BOOL){
        output::errorMismatch(node.line);
    }
    node.type = ast::BuiltInType::BOOL;
}

void SemanticVisitor::visit(ast::And &node){
    node.left->accept(*this);
    node.right->accept(*this);
    if(node.left->type != ast::BuiltInType::BOOL){
        output::errorMismatch(node.line);
    }
    if(node.right->type != ast::BuiltInType::BOOL){
        output::errorMismatch(node.line);
    }
    node.type = ast::BuiltInType::BOOL;
}

void SemanticVisitor::visit(ast::Or &node){
    node.left->accept(*this);
    node.right->accept(*this);
    if(node.left->type != ast::BuiltInType::BOOL){
        output::errorMismatch(node.line);
    }
    if(node.right->type != ast::BuiltInType::BOOL){
        output::errorMismatch(node.line);
    }
    node.type = ast::BuiltInType::BOOL;
}