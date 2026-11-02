#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include "wlp4data.h"
using namespace std;

// master symbol table will be useful for when we have procedures
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
    "EQ","PLUS","MINUS","STAR","SLASH","PCT","LBRACK","RBRACK","AMP"
};


ParseTree* buildTree(istream& in) {
    string line;
    if (!getline(in, line)) return nullptr;
    ParseTree* node = new ParseTree{line, {}};

    istringstream iss(line);
    string lhs; iss >> lhs;
    if (line.find(".EMPTY") != string::npos || terminals.count(lhs)) return node;

    string sym;
    while (iss >> sym) {
        if (sym != ".EMPTY") {
            ParseTree* child = buildTree(in);
            if (!child) return node;
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


string getTypeFromTypeNode(ParseTree* node) {
    return (node->children.size() == 2 ? "int*" : "int");
}

void processDcls(ParseTree* node, const string& proc, ParseTree* root) {
    if (node->val.find(".EMPTY") != string::npos) return;

    processDcls(node->children[0], proc, root);

    ParseTree* dcl = node->children[1];
    ParseTree* rhs = node->children[3];

    string initType = (rhs->val.substr(0,3) == "NUM") ? "int" : "int*";
    auto [name, declType] = processDcl(dcl, proc, root, false);
    if (declType != initType) { errorAndClean(root); exit(1); }

    rhs->type = initType;
}


string annotateExpr(ParseTree* node) {
    const string& rule = node->val;
    if (rule == "expr term") {
        string t = annotateExpr(node->children[0]); node->type = t; return t;
    } else if (rule == "term factor") {
        string t = annotateExpr(node->children[0]); node->type = t; return t;
    } else if (rule == "factor NUM") {
        node->type = "int"; node->children[0]->type = "int"; return "int";
    } else if (rule == "factor NULL") {
        node->type = "int*"; node->children[0]->type = "int*"; return "int*";
    } else if (rule == "factor LPAREN expr RPAREN") {
        string t = annotateExpr(node->children[1]); node->type = t; return t;
    }
    return "";
}


int main() {
    ParseTree* root = buildTree(cin);

    // validation checks
    if (!root) { 
        cerr << "ERROR" << endl; 
        return 1; 
    }

    if (root->children.size() < 3) { 
        cerr << "ERROR" << endl; 
        delete_tree(root); 
        return 1; 
    }

    ParseTree* procedures = root->children[1];
    if (procedures->children.empty()) { 
        cerr << "ERROR" << endl; 
        delete_tree(root); 
        return 1; 
    }

    ParseTree* mainNode = procedures->children[0];
    if (mainNode->val.substr(0, 4) != "main") { 
        cerr << "ERROR" << endl; 
        delete_tree(root); 
        return 1; 
    }

    string procName;
    for (auto* c : mainNode->children) {
        if (c->val.substr(0, 4) == "WAIN" || c->val.substr(0, 2) == "ID") {
            istringstream iss(c->val);
            string tok, name; iss >> tok >> name;
            procName = name; 
            break;
        }
    }
    if (procName.empty()) procName = "wain"; 
    tables[procName] = {{}, {}};


    if (mainNode->children.size() < 12) { 
        cerr << "ERROR" << endl; 
        delete_tree(root); 
        return 1; 
    }


    ParseTree* dcl1 = mainNode->children[3];
    ParseTree* dcl2 = mainNode->children[5];

    auto [name1, type1] = processDcl(dcl1, procName, true);
    auto [name2, type2] = processDcl(dcl2, procName, true);


    // check if the names are the same and if the type is the same 
    if (name1 == name2 || type2 != "int") {
        cerr << "ERROR" << endl; 
        delete_tree(root);
         return 1;
    }

    ParseTree* expr = mainNode->children[11];
    string exprType = annotateExpr(expr);
    if (exprType != "int") { 
        cerr << "ERROR" << endl; 
        delete_tree(root); 
        return 1; 
    }

    preorderTraversal(root);
    delete_tree(root);
    return 0;
}
