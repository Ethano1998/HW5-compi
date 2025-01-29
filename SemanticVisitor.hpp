#ifndef SVISITOR_HPP
#define SVISITOR_HPP

#include "symbolTable.hpp"
#include "output.hpp"

class SemanticVisitor : public Visitor {
private:
    /*
    std::vector<std::string> indents;
    std::vector<std::string> prefixes;

    /* Helper function to print a string with the current indentation 
    void print_indented(const std::string &str);

    /* Functions to manage the indentation level 
    void enter_child();

    void enter_last_child();

    void leave_child();
    */

public:

    bool from_funcdecl = false;

    enum class Context {
        DECLARATION,
        REFERENCE_FUNC,
        REFERENCE_VAR
    };

    bool declaration_function = false;

    Context current_context;

    output::ScopePrinter scopePrinter; 

    GlobalSymbolTable globalSymbolTable;

    void setContext(Context context){
        current_context = context;
    }

    std::string toString(ast::BuiltInType type) {
        switch (type) {
            case ast::BuiltInType::INT:
                return "INT";
            case ast::BuiltInType::BOOL:
                return "BOOL";
            case ast::BuiltInType::BYTE:
                return "BYTE";
            case ast::BuiltInType::VOID:
                return "VOID";
            case ast::BuiltInType::STRING:
                return "STRING";
            default:
                return "unknown";
        }
    }

    ast::BuiltInType toBuiltInType(const std::string &type) {
        if (type == "INT") {
            return ast::BuiltInType::INT;
        } else if (type == "BOOL") {
            return ast::BuiltInType::BOOL;
        } else if (type == "BYTE") {
            return ast::BuiltInType::BYTE;
        } else if (type == "VOID") {
            return ast::BuiltInType::VOID;
        } else if (type == "STRING") {
            return ast::BuiltInType::STRING;
        }
    }
    
    ast::BuiltInType check_assign(std::shared_ptr<ast::Exp> &node);

    void visit(ast::Num &node) override;

    void visit(ast::NumB &node) override;

    void visit(ast::String &node) override;

    void visit(ast::Bool &node) override;

    void visit(ast::ID &node) override;

    void visit(ast::BinOp &node) override;

    void visit(ast::RelOp &node) override;

    void visit(ast::Not &node) override;

    void visit(ast::And &node) override;

    void visit(ast::Or &node) override;

    void visit(ast::Type &node) override;

    void visit(ast::Cast &node) override;

    void visit(ast::ExpList &node) override;

    void visit(ast::Call &node) override;

    void visit(ast::Statements &node) override;

    void visit(ast::Break &node) override;

    void visit(ast::Continue &node) override;

    void visit(ast::Return &node) override;

    void visit(ast::If &node) override;

    void visit(ast::While &node) override;

    void visit(ast::VarDecl &node) override;

    void visit(ast::Assign &node) override;

    void visit(ast::Formal &node) override;

    void visit(ast::Formals &node) override;

    void visit(ast::FuncDecl &node) override;

    void visit(ast::Funcs &node) override;
};

#endif //SVISITOR_HPP