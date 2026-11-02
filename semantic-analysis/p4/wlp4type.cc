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
string currentProc = ""; // Track current procedure being analyzed

struct ParseTree {
    string val;
    vector<ParseTree*> children;
    string type = "";
};

static const set<string> terminals = {
    "NUM","ID","NULL","INT","WAIN","RETURN","SEMI","COMMA",
    "LPAREN","RPAREN","LBRACE","RBRACE","BOF","EOF","BECOMES",
    "PRINTLN","WHILE","IF","ELSE","LT","GT","LE","GE","NE",
    "EQ","PLUS","MINUS","STAR","SLASH","PCT","LBRACK","RBRACK","AMP","NEW","GETCHAR"
};


ParseTree* buildTree(istream& in) {
    string line;
    if (!getline(in, line)) return nullptr;

    ParseTree* node = new ParseTree{line, {}};
    istringstream iss(line);
    string lhs;
    iss >> lhs;

    // terminal or .EMPTY
    if (line.find(".EMPTY") != string::npos || terminals.count(lhs)) {
        return node;
    }

    // collect RHS symbols
    vector<string> rhs;
    string sym;
    while (iss >> sym) {
        if (sym != ".EMPTY") rhs.push_back(sym);
    }

    // recursively build each RHS child
    for (auto& s : rhs) {
        ParseTree* child = buildTree(in);
        if (child) node->children.push_back(child);
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
// this can easily be determiend as 
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


// dcl: pointer to the parse tree node which is the declaration
// proc: name of the procedure we are in like "main"
// root: root node of the entire program
// isParam: boolean value which indicate whether the dcl is a parameter or a varable
// processDcl(ParseTree, string, ParseTree, bool) is a function which takes in a dcl node and determines a few things
// - type
// - name
// - check for duplicates
// - adds it to the symbol table
pair<string,string> processDcl(ParseTree* dcl, const string& proc, ParseTree* root, bool isParam) {
    string type = getTypeFromTypeNode(dcl->children[0]);

    // extract the variable name
    // like ID a has the name a
    ParseTree* id = dcl->children[1];
    istringstream iss(id->val);
    string tok, name; 
    iss >> tok >> name;

    id->type = type;

    // look in the procedure and check if a variable like that already exists
    if (tables[proc].second.count(name))
        error_and_clean(root, "duplicate variable " + name);

    // if the dcl is a parameter then push it to the parameter which is the first part
    if (isParam) tables[proc].first.push_back(type);
    tables[proc].second[name] = type;
    return {name, type};
}


// processDcls handles dcls that are in the parse tree it recursively also cehcks for other dcls in the rule (left recursive)
// dcls → dcls dcl BECOMES NUM SEMI
// dcls → dcls dcl BECOMES NULL SEMI
// dcls → .EMPTY
void processDcls(ParseTree* node, const string& proc, ParseTree* root) {
    // base case
    if (node->val.find(".EMPTY") != string::npos) return;

    // otherwise recurse
    processDcls(node->children[0], proc, root);

    ParseTree* dcl = node->children[1];
    ParseTree* rhs = node->children[3]; 
    string initType = (rhs->val.substr(0,3) == "NUM") ? "int" : "int*";

    // for the dcl we extracted process that encounter
    auto [name, declType] = processDcl(dcl, proc, root, false);

    // types are not the same
    if (declType != initType)
        error_and_clean(root, "type mismatch in dcls for " + name);

    // assign the type for the
    rhs->type = initType;
}

// Forward declarations
void processParamlist(ParseTree* paramlist, const string& proc, ParseTree* root);
void collectArgTypes(ParseTree* arglist, vector<string>& types, const string& proc, ParseTree* root);
string annotateExpr(ParseTree* node, const string& proc, ParseTree* root);

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
        // Single parameter
        processDcl(paramlist->children[0], proc, root, true);
    } else if (rule == "paramlist dcl COMMA paramlist") {
        // Multiple parameters
        processDcl(paramlist->children[0], proc, root, true);
        processParamlist(paramlist->children[2], proc, root);
    }
}

// annotateExpr handles adding the annotation for the type to the specific node
string annotateExpr(ParseTree* node, const string& proc, ParseTree* root) {
    // in our case the rule itself is the actual line that we consumed in when building the tree
    string rule = node->val;
    
    // expr's type will be the exact same as its term
    if (rule == "expr term") {
        // reursively find the type of the child node which is term
        string t = annotateExpr(node->children[0], proc, root);
        node->type = t; 
        return t;
    }

    // term will have the type of whatever factor is 
    if (rule == "term factor") {
        string t = annotateExpr(node->children[0], proc, root);
        node->type = t; 
        return t;
    }

    // factor will have type int so we assign that to the factor and NUM
    if (rule == "factor NUM") {
        node->type = "int"; 
        node->children[0]->type = "int"; 
        return "int";
    }

    // since null factor will have type int* so will its child
    if (rule == "factor NULL") {
        node->type = "int*";
        node->children[0]->type = "int*"; 
        return "int*";
    }

    // factor will have the type of whatever the ID type is
    if (rule == "factor ID") {
        istringstream iss(node->children[0]->val);
        string tok, name; 
        iss >> tok >> name;

        // we retrieev the ID
        if (!tables[proc].second.count(name))
            error_and_clean(root, "undeclared ID " + name);
        string t = tables[proc].second[name];
        // assign the types to factor, its child
        node->type = t; 
        node->children[0]->type = t;
        return t;
    }

    // Handle procedure call with no arguments: factor → ID LPAREN RPAREN
    if (rule == "factor ID LPAREN RPAREN") {
        istringstream iss(node->children[0]->val);
        string tok, name;
        iss >> tok >> name;
        
        // Check if ID is a variable (can't call a variable)
        if (tables[proc].second.count(name)) {
            error_and_clean(root, "cannot call variable " + name);
        }
        
        // Check if procedure exists
        if (!tables.count(name)) {
            error_and_clean(root, "undeclared procedure " + name);
        }
        
        // Check signature (should have 0 parameters)
        if (tables[name].first.size() != 0) {
            error_and_clean(root, "wrong number of arguments for " + name);
        }
        
        node->type = "int";
        // DO NOT annotate procedure name ID with type
        // node->children[0]->type stays empty
        return "int";
    }

    // Handle procedure call with arguments: factor → ID LPAREN arglist RPAREN
    if (rule == "factor ID LPAREN arglist RPAREN") {
        istringstream iss(node->children[0]->val);
        string tok, name;
        iss >> tok >> name;
        
        // Check if ID is a variable (can't call a variable)
        if (tables[proc].second.count(name)) {
            error_and_clean(root, "cannot call variable " + name);
        }
        
        // Check if procedure exists
        if (!tables.count(name)) {
            error_and_clean(root, "undeclared procedure " + name);
        }
        
        // Collect argument types
        vector<string> argTypes;
        collectArgTypes(node->children[2], argTypes, proc, root);
        
        // Check signature matches
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
        // DO NOT annotate procedure name ID with type
        // node->children[0]->type stays empty
        return "int";
    }

    // Handle getchar: factor → GETCHAR LPAREN RPAREN
    if (rule == "factor GETCHAR LPAREN RPAREN") {
        node->type = "int";
        return "int";
    }

    // factor will have the type of expr
    if (rule == "factor LPAREN expr RPAREN") {
        string t = annotateExpr(node->children[1], proc, root);
        node->type = t; 
        return t;
    }

    // factor will have the type int* if and only if the lvalue has type int otherwise error
    if (rule == "factor AMP lvalue") { 
        string lt = annotateExpr(node->children[1], proc, root);
        if (lt != "int") error_and_clean(root, "address-of non-int");
        node->type = "int*"; 
        return "int*";
    }

    // factor will have the type int if and only if factor has type int* otherwise its incorect to reference a int
    if (rule == "factor STAR factor") {
        string ft = annotateExpr(node->children[1], proc, root);
        if (ft != "int*") error_and_clean(root, "deref of non-pointer");
        node->type = "int"; 
        return "int";
    }

    // factor should have type int *, but we also need to annotate the inside expr and check if it is type int only then factor can be type *
    if (rule == "factor NEW INT LBRACK expr RBRACK") {
        string et = annotateExpr(node->children[3], proc, root);
        if (et != "int") error_and_clean(root, "new array with non-int index");
        node->type = "int*"; 
        return "int*";
    }

    // for this we need to check if the lhs and rhs
    // if lhs = int, rhs = int then result is int
    // if lhs = int*, rhs = int or visaversa then result is int*
    // otherwise error
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

    // same ideas + however if lhs and rhs are int* then the result is int
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

    // for this u have to make sure both lhs and rhs are ints
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

    // lvalue must be an *int otherwise invalid this is a simialr as anothe rule
    if (rule == "lvalue STAR factor") {
        string ft = annotateExpr(node->children[1], proc, root);
        if (ft != "int*") error_and_clean(root, "lvalue deref non-pointer");
        node->type = "int";
        return "int";
    }
    
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
        // Single argument
        string t = annotateExpr(arglist->children[0], proc, root);
        types.push_back(t);
    } else if (rule == "arglist expr COMMA arglist") {
        // Multiple arguments
        string t = annotateExpr(arglist->children[0], proc, root);
        types.push_back(t);
        collectArgTypes(arglist->children[2], types, proc, root);
    }
}

// Process a single procedure (main or procedure)
void processProcedure(ParseTree* procNode, ParseTree* root, bool isMain) {
    string procName;
    ParseTree *paramsNode, *dclsNode, *exprNode;
    
    if (isMain) {
        // main → INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
        procName = "wain";
        
        // Check for duplicate procedure
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
        
        // Check that second parameter is int type (CRITICAL requirement)
        if (type2 != "int")
            error_and_clean(root, "second parameter of wain must be int");
        
        dclsNode = procNode->children[8];
        exprNode = procNode->children[11];
        
    } else {
        // procedure → INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
        istringstream iss(procNode->children[1]->val);
        string tok;
        iss >> tok >> procName;
        
        // Check for duplicate procedure
        if (tables.count(procName)) {
            error_and_clean(root, "duplicate procedure " + procName);
        }
        
        tables[procName] = {{}, {}};
        
        // DO NOT annotate the procedure name ID with a type
        // procNode->children[1]->type = ""; // Procedure IDs don't get types
        
        paramsNode = procNode->children[3];
        processParams(paramsNode, procName, root);
        
        dclsNode = procNode->children[6];
        exprNode = procNode->children[9];
    }
    
    // Process declarations
    processDcls(dclsNode, procName, root);
    
    // Type check return expression
    string exprType = annotateExpr(exprNode, procName, root);
    if (exprType != "int")
        error_and_clean(root, "procedure must return int");
}

// Process all procedures
void processProcedures(ParseTree* procedures, ParseTree* root) {
    string rule = procedures->val;
    
    if (rule == "procedures main") {
        // Only wain
        processProcedure(procedures->children[0], root, true);
    } else if (rule == "procedures procedure procedures") {
        // procedure followed by more procedures
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
        
        // Process all procedures
        processProcedures(procedures, root);

        preorderTraversal(root);
        delete_tree(root);
    } catch (...) {
        return 1;
    }
    return 0;
}