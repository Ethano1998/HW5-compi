#ifndef SYMBOLTABLE_HPP
#define SYMBOLTABLE_HPP

#include <vector>
#include <memory>
#include <string>


class SymbolEntry {
    public:
    virtual ~SymbolEntry() = default;
};

class VarSymbolEntry : public SymbolEntry {
public:
    std::string name;
    std::string type;
    int offset;
    VarSymbolEntry(std::string name, std::string type, int offset) : name(name), type(type), offset(offset) {}
};

class FuncSymbolEntry : public SymbolEntry {
public:
    std::string name;
    std::vector<std::string> paramTypes;
    std::string returnType;
    FuncSymbolEntry(std::string name,std::vector<std::string> paramTypes ,std::string returnType) : name(name), paramTypes(paramTypes), returnType(returnType) {}
};

class SymbolTable {
    int global_offset = 0;

    public:
    std::vector<std::shared_ptr<SymbolEntry>> entries;
    int param_offset = 0;
    int scope_offset = 0;

    SymbolTable() = default; 
    
    std::shared_ptr<SymbolEntry> findEntry(std::string name) {
        for (auto entry : entries) {
            if (std::dynamic_pointer_cast<VarSymbolEntry>(entry)) {
                if (std::dynamic_pointer_cast<VarSymbolEntry>(entry)->name == name) {
                    return entry;
                }
            } else if (std::dynamic_pointer_cast<FuncSymbolEntry>(entry)) {
                if (std::dynamic_pointer_cast<FuncSymbolEntry>(entry)->name == name) {
                    return entry;
                }
            }
        }
        return nullptr;
    }

    void addParam(std::string name, std::string type){
        entries.push_back(std::make_shared<VarSymbolEntry>(name, type, param_offset-1));
        param_offset --;
    }

    int get_offset(){
        return global_offset;
    }

    void set_offset(int offset){
        global_offset = global_offset + offset;
    }

    void addVar(std::string name, std::string type) {
        entries.push_back(std::make_shared<VarSymbolEntry>(name, type, global_offset));
        global_offset++;
        scope_offset++;
    }

    void addFunc(std::string name, std::vector<std::string> paramTypes, std::string returnType) {
        entries.push_back(std::make_shared<FuncSymbolEntry>(name, paramTypes, returnType));
    }
};

class GlobalSymbolTable {
    std::vector<std::shared_ptr<SymbolTable>> tables;
    public:
    int is_loop = 0;
    GlobalSymbolTable(){
        std::shared_ptr<SymbolTable> table = std::make_shared<SymbolTable>();
        table->addFunc("print", std::vector<std::string>{"STRING"}, "void");
        table->addFunc("printi", std::vector<std::string>{"INT"}, "void");
        tables.push_back(table);
    };

    void addTable(std::shared_ptr<SymbolTable>(table)) {
        std::shared_ptr<SymbolTable> precedent_table = tables.back();
        int old_offset = precedent_table->get_offset();
        table->set_offset(old_offset);
        tables.push_back(table);
    }
    
    std::shared_ptr<SymbolTable> getFunctionTable(){
        return tables.front();
    }

    std::shared_ptr<SymbolTable> getTable() {
        return tables.back();
    }

    std::shared_ptr<SymbolTable> popTable() {
        auto table = tables.back();
        int sub_offset = table->scope_offset;
        tables.pop_back();
        auto dad_table= tables.back();
        dad_table->scope_offset = dad_table->scope_offset - sub_offset;
        return table;
    }

    std::shared_ptr<SymbolEntry> findEntry(std::string name) {
        for (auto table : tables) {
            auto entry = table->findEntry(name);
            if (entry) {
                return entry;
            }
        }
        return nullptr;
    }
};



#endif //SYMBOLTABLE_HPP