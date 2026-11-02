#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <stdexcept>

using namespace std;

map<string, pair<vector<string>, map<string, string>>> tables;

struct ParseTree {
    string val;
    vector<ParseTree*> children;
    string type = "";
};

static const set<string> terminals = {
    "NUM","ID","NULL","INT","WAIN","RETURN","SEMI","COMMA",
    "LPAREN","RPAREN","LBRACE","RBRACE","BOF","EOF","BECOMES",
    "PRINTLN","WHILE","IF","ELSE","LT","GT","LE","GE","NE",
    "EQ","PLUS","MINUS","STAR","SLASH","PCT","LBRACK","RBRACK","AMP","NEW","GETCHAR",
    "DELETE","PUTCHAR"
};

// Build parse tree from input
ParseTree* buildTree(istream& in) {
    string line;
    if (!getline(in, line)) return nullptr;

    ParseTree* node = new ParseTree{line, {}};
    istringstream iss(line);
    string lhs;
    iss >> lhs;

    // Check if terminal
    if (terminals.count(lhs)) {
        return node;
    }
    
    // Check if .EMPTY production
    string second;
    iss >> second;
    if (second == ".EMPTY") {
        return node;
    }

    // Non-terminal: collect RHS symbols
    vector<string> rhs;
    rhs.push_back(second);
    string sym;
    while (iss >> sym) {
        rhs.push_back(sym);
    }

    // Recursively build children
    for (const auto& s : rhs) {
        ParseTree* child = buildTree(in);
        if (child) {
            node->children.push_back(child);
        }
    }

    return node;
}

void delete_tree(ParseTree* n) {
    if (!n) return;
    for (auto* c : n->children) delete_tree(c);
    delete n;
}

void preorderTraversal(ParseTree* n) {
    if (!n) return;
    if (!n->type.empty()) cout << n->val << " : " << n->type << endl;
    else cout << n->val << endl;
    for (auto* c : n->children) preorderTraversal(c);
}

// getTypeFromTypeNode(ParseTree) determines the type of each node
// type -> INT STAR (2 children)
// type -> INT (1 child)
string getTypeFromTypeNode(ParseTree* node) {
    if (node->children.size() == 2) {
        return "int*";
    }
    return "int";
}

// error_and_clean(ParseTree, string) throws an error 
void error_and_clean(ParseTree* root, const string& msg) {
    cerr << "ERROR" << endl;
    delete_tree(root);
    throw runtime_error("semantic error");
}

// Forward declarations
void processParamlist(ParseTree* paramlist, const string& proc, ParseTree* root);
void collectArgTypes(ParseTree* arglist, vector<string>& types, const string& proc, ParseTree* root);
string annotateExpr(ParseTree* node, const string& proc, ParseTree* root);
void checkStatements(ParseTree* statements, const string& proc, ParseTree* root);
void checkStatement(ParseTree* statement, const string& proc, ParseTree* root);
void checkTest(ParseTree* test, const string& proc, ParseTree* root);

// processDcl - process a declaration
// - extracts type and name
// - checks for duplicates
// - adds to symbol table
pair<string,string> processDcl(ParseTree* dcl, const string& proc, ParseTree* root, bool isParam) {
    string type = getTypeFromTypeNode(dcl->children[0]);

    // extract the variable name (ID token)
    ParseTree* id = dcl->children[1];
    istringstream iss(id->val);
    string tok, name; 
    iss >> tok >> name;

    id->type = type;

    // check for duplicates
    if (tables[proc].second.count(name))
        error_and_clean(root, "duplicate variable " + name);

    // if parameter, add to signature
    if (isParam) tables[proc].first.push_back(type);
    tables[proc].second[name] = type;
    return {name, type};
}

// processDcls - handle variable declarations
// dcls → dcls dcl BECOMES NUM SEMI
// dcls → dcls dcl BECOMES NULL SEMI
// dcls → .EMPTY
void processDcls(ParseTree* node, const string& proc, ParseTree* root) {
    // base case
    if (node->val.find(".EMPTY") != string::npos) return;

    // recursive case
    processDcls(node->children[0], proc, root);

    ParseTree* dcl = node->children[1];
    ParseTree* rhs = node->children[3]; 
    string initType = (rhs->val.substr(0,3) == "NUM") ? "int" : "int*";

    auto [name, declType] = processDcl(dcl, proc, root, false);

    // check type match
    if (declType != initType)
        error_and_clean(root, "type mismatch in dcls for " + name);

    rhs->type = initType;
}

// Extract parameters from params node
void processParams(ParseTree* params, const string& proc, ParseTree* root) {
    if (params->val.find(".EMPTY") != string::npos) return;
    
    // params → paramlist
    if (params->children.size() == 1) {
        processParamlist(params->children[0], proc, root);
    }
}

// Process paramlist recursively
void processParamlist(ParseTree* paramlist, const string& proc, ParseTree* root) {
    string rule = paramlist->val;
    
    if (rule == "paramlist dcl") {
        processDcl(paramlist->children[0], proc, root, true);
    } else if (rule == "paramlist dcl COMMA paramlist") {
        processDcl(paramlist->children[0], proc, root, true);
        processParamlist(paramlist->children[2], proc, root);
    }
}

// annotateExpr - annotate expression nodes with types
string annotateExpr(ParseTree* node, const string& proc, ParseTree* root) {
    string rule = node->val;
    
    // expr → term
    if (rule == "expr term") {
        string t = annotateExpr(node->children[0], proc, root);
        node->type = t; 
        return t;
    }

    // term → factor
    if (rule == "term factor") {
        string t = annotateExpr(node->children[0], proc, root);
        node->type = t; 
        return t;
    }

    // factor → NUM
    if (rule == "factor NUM") {
        node->type = "int"; 
        node->children[0]->type = "int"; 
        return "int";
    }

    // factor → NULL
    if (rule == "factor NULL") {
        node->type = "int*";
        node->children[0]->type = "int*"; 
        return "int*";
    }

    // factor → ID
    if (rule == "factor ID") {
        istringstream iss(node->children[0]->val);
        string tok, name; 
        iss >> tok >> name;

        if (!tables[proc].second.count(name))
            error_and_clean(root, "undeclared ID " + name);
        string t = tables[proc].second[name];
        node->type = t; 
        node->children[0]->type = t;
        return t;
    }

    // factor → ID LPAREN RPAREN (procedure call, no args)
    if (rule == "factor ID LPAREN RPAREN") {
        istringstream iss(node->children[0]->val);
        string tok, name;
        iss >> tok >> name;
        
        if (tables[proc].second.count(name)) {
            error_and_clean(root, "cannot call variable " + name);
        }
        
        if (!tables.count(name)) {
            error_and_clean(root, "undeclared procedure " + name);
        }
        
        if (tables[name].first.size() != 0) {
            error_and_clean(root, "wrong number of arguments for " + name);
        }
        
        node->type = "int";
        return "int";
    }

    // factor → ID LPAREN arglist RPAREN (procedure call with args)
    if (rule == "factor ID LPAREN arglist RPAREN") {
        istringstream iss(node->children[0]->val);
        string tok, name;
        iss >> tok >> name;
        
        if (tables[proc].second.count(name)) {
            error_and_clean(root, "cannot call variable " + name);
        }
        
        if (!tables.count(name)) {
            error_and_clean(root, "undeclared procedure " + name);
        }
        
        vector<string> argTypes;
        collectArgTypes(node->children[2], argTypes, proc, root);
        
        const vector<string>& signature = tables[name].first;
        if (argTypes.size() != signature.size()) {
            error_and_clean(root, "wrong number of arguments for " + name);
        }
        
        for (size_t i = 0; i < argTypes.size(); ++i) {
            if (argTypes[i] != signature[i]) {
                error_and_clean(root, "argument type mismatch for " + name);
            }
        }
        
        node->type = "int";
        return "int";
    }

    // factor → GETCHAR LPAREN RPAREN
    if (rule == "factor GETCHAR LPAREN RPAREN") {
        node->type = "int";
        return "int";
    }

    // factor → LPAREN expr RPAREN
    if (rule == "factor LPAREN expr RPAREN") {
        string t = annotateExpr(node->children[1], proc, root);
        node->type = t; 
        return t;
    }

    // factor → AMP lvalue
    if (rule == "factor AMP lvalue") { 
        string lt = annotateExpr(node->children[1], proc, root);
        if (lt != "int") error_and_clean(root, "address-of non-int");
        node->type = "int*"; 
        return "int*";
    }

    // factor → STAR factor
    if (rule == "factor STAR factor") {
        string ft = annotateExpr(node->children[1], proc, root);
        if (ft != "int*") error_and_clean(root, "deref of non-pointer");
        node->type = "int"; 
        return "int";
    }

    // factor → NEW INT LBRACK expr RBRACK
    if (rule == "factor NEW INT LBRACK expr RBRACK") {
        string et = annotateExpr(node->children[3], proc, root);
        if (et != "int") error_and_clean(root, "new array with non-int index");
        node->type = "int*"; 
        return "int*";
    }

    // expr → expr PLUS term
    if (rule == "expr expr PLUS term") {
        string lt = annotateExpr(node->children[0], proc, root);
        string rt = annotateExpr(node->children[2], proc, root);
        string result;
        if (lt == "int" && rt == "int") result = "int";
        else if ((lt == "int*" && rt == "int") || (lt == "int" && rt == "int*")) result = "int*";
        else error_and_clean(root, "invalid PLUS operands");
        node->type = result; 
        return result;
    }

    // expr → expr MINUS term
    if (rule == "expr expr MINUS term") {
        string lt = annotateExpr(node->children[0], proc, root);
        string rt = annotateExpr(node->children[2], proc, root);
        string result;
        if (lt == "int" && rt == "int") result = "int";
        else if (lt == "int*" && rt == "int") result = "int*";
        else if (lt == "int*" && rt == "int*") result = "int";
        else error_and_clean(root, "invalid MINUS operands");
        node->type = result; 
        return result;
    }

    // term → term STAR/SLASH/PCT factor
    if (rule == "term term STAR factor" ||
        rule == "term term SLASH factor" ||
        rule == "term term PCT factor") {
        string lt = annotateExpr(node->children[0], proc, root);
        string rt = annotateExpr(node->children[2], proc, root);
        if (lt != "int" || rt != "int")
            error_and_clean(root, "non-int arithmetic");
        node->type = "int"; 
        return "int";
    }

    // lvalue → ID
    if (rule == "lvalue ID") {
        istringstream iss(node->children[0]->val);
        string tok, name; 
        iss >> tok >> name;
        if (!tables[proc].second.count(name))
            error_and_clean(root, "undeclared ID " + name);
        string t = tables[proc].second[name];
        node->type = t; 
        node->children[0]->type = t; 
        return t;
    }

    // lvalue → STAR factor
    if (rule == "lvalue STAR factor") {
        string ft = annotateExpr(node->children[1], proc, root);
        if (ft != "int*") error_and_clean(root, "lvalue deref non-pointer");
        node->type = "int";
        return "int";
    }
    
    // lvalue → LPAREN lvalue RPAREN
    if (rule == "lvalue LPAREN lvalue RPAREN") {
        string lt = annotateExpr(node->children[1], proc, root);
        node->type = lt; 
        return lt;
    }

    error_and_clean(root, "unexpected rule: " + rule);
    return "";
}

// Collect argument types from arglist
void collectArgTypes(ParseTree* arglist, vector<string>& types, const string& proc, ParseTree* root) {
    string rule = arglist->val;
    
    if (rule == "arglist expr") {
        string t = annotateExpr(arglist->children[0], proc, root);
        types.push_back(t);
    } else if (rule == "arglist expr COMMA arglist") {
        string t = annotateExpr(arglist->children[0], proc, root);
        types.push_back(t);
        collectArgTypes(arglist->children[2], types, proc, root);
    }
}

// Check test expressions (comparisons)
void checkTest(ParseTree* test, const string& proc, ParseTree* root) {
    // test → expr OP expr
    string leftType = annotateExpr(test->children[0], proc, root);
    string rightType = annotateExpr(test->children[2], proc, root);
    
    if (leftType != rightType) {
        error_and_clean(root, "comparison operands must have same type");
    }
}

// Check a single statement
void checkStatement(ParseTree* statement, const string& proc, ParseTree* root) {
    string rule = statement->val;
    
    // statement → lvalue BECOMES expr SEMI
    if (rule == "statement lvalue BECOMES expr SEMI") {
        string lvalueType = annotateExpr(statement->children[0], proc, root);
        string exprType = annotateExpr(statement->children[2], proc, root);
        
        if (lvalueType != exprType) {
            error_and_clean(root, "assignment type mismatch");
        }
    }
    // statement → IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE
    else if (rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
        checkTest(statement->children[2], proc, root);
        checkStatements(statement->children[5], proc, root);
        checkStatements(statement->children[9], proc, root);
    }
    // statement → WHILE LPAREN test RPAREN LBRACE statements RBRACE
    else if (rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
        checkTest(statement->children[2], proc, root);
        checkStatements(statement->children[5], proc, root);
    }
    // statement → PRINTLN LPAREN expr RPAREN SEMI
    else if (rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
        string exprType = annotateExpr(statement->children[2], proc, root);
        
        if (exprType != "int") {
            error_and_clean(root, "println requires int argument");
        }
    }
    // statement → PUTCHAR LPAREN expr RPAREN SEMI
    else if (rule == "statement PUTCHAR LPAREN expr RPAREN SEMI") {
        string exprType = annotateExpr(statement->children[2], proc, root);
        
        if (exprType != "int") {
            error_and_clean(root, "putchar requires int argument");
        }
    }
    // statement → DELETE LBRACK RBRACK expr SEMI
    else if (rule == "statement DELETE LBRACK RBRACK expr SEMI") {
        string exprType = annotateExpr(statement->children[3], proc, root);
        
        if (exprType != "int*") {
            error_and_clean(root, "delete requires int* argument");
        }
    }
}

// Check all statements recursively
void checkStatements(ParseTree* statements, const string& proc, ParseTree* root) {
    string rule = statements->val;
    
    // Base case: statements → ε
    if (rule.find(".EMPTY") != string::npos) {
        return;
    }
    
    // Recursive case: statements → statements statement
    if (rule == "statements statements statement") {
        checkStatements(statements->children[0], proc, root);
        checkStatement(statements->children[1], proc, root);
    }
}

// Process a single procedure
void processProcedure(ParseTree* procNode, ParseTree* root, bool isMain) {
    string procName;
    ParseTree *paramsNode, *dclsNode, *statementsNode, *exprNode;
    
    if (isMain) {
        // main → INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
        procName = "wain";
        
        if (tables.count(procName)) {
            error_and_clean(root, "duplicate procedure " + procName);
        }
        
        tables[procName] = {{}, {}};
        
        ParseTree* dcl1 = procNode->children[3];
        ParseTree* dcl2 = procNode->children[5];
        
        auto [name1, type1] = processDcl(dcl1, procName, root, true);
        auto [name2, type2] = processDcl(dcl2, procName, root, true);
        
        if (name1 == name2)
            error_and_clean(root, "duplicate wain parameter name");
        
        if (type2 != "int")
            error_and_clean(root, "second parameter of wain must be int");
        
        dclsNode = procNode->children[8];
        statementsNode = procNode->children[9];
        exprNode = procNode->children[11];
        
    } else {
        // procedure → INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
        istringstream iss(procNode->children[1]->val);
        string tok;
        iss >> tok >> procName;
        
        if (tables.count(procName)) {
            error_and_clean(root, "duplicate procedure " + procName);
        }
        
        tables[procName] = {{}, {}};
        
        paramsNode = procNode->children[3];
        processParams(paramsNode, procName, root);
        
        dclsNode = procNode->children[6];
        statementsNode = procNode->children[7];
        exprNode = procNode->children[9];
    }
    
    processDcls(dclsNode, procName, root);
    checkStatements(statementsNode, procName, root);
    
    string exprType = annotateExpr(exprNode, procName, root);
    if (exprType != "int")
        error_and_clean(root, "procedure must return int");
}

// Process all procedures
void processProcedures(ParseTree* procedures, ParseTree* root) {
    string rule = procedures->val;
    
    if (rule == "procedures main") {
        processProcedure(procedures->children[0], root, true);
    } else if (rule == "procedures procedure procedures") {
        processProcedure(procedures->children[0], root, false);
        processProcedures(procedures->children[1], root);
    }
}

int main() {
    try {
        ParseTree* root = buildTree(cin);
        if (!root) throw runtime_error("empty input");
        
        if (root->children.size() < 3)
            error_and_clean(root, "root too small");

        ParseTree* procedures = root->children[1];
        if (procedures->children.empty())
            error_and_clean(root, "no procedures");
        
        processProcedures(procedures, root);

        preorderTraversal(root);
        delete_tree(root);
    } catch (...) {
        return 1;
    }
    return 0;
}