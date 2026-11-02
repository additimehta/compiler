#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>

using namespace std;


// global table
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
    if (!getline(in, line)) return nullptr; //  if no input reutnr nothing
    ParseTree* node = new ParseTree{line, {}}; // create a new tree initalized to nothign
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

bool errorAndClean(ParseTree* root) {
    cerr << "ERROR" << endl;
    delete_tree(root);
    return true;
}

pair<string,string> processDcl(ParseTree* dcl, const string& proc, ParseTree* root, bool isParam) {
    string type = getTypeFromTypeNode(dcl->children[0]);
    ParseTree* id = dcl->children[1];
    istringstream iss(id->val);
    string token, name; iss >> token >> name;
    id->type = type;
    if (tables[proc].second.count(name)) {
        errorAndClean(root);
        exit(1);
    }
    if (isParam) tables[proc].first.push_back(type);
    tables[proc].second[name] = type;
    return {name, type};
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

string annotateExpr(ParseTree* node, const string& proc, ParseTree* root) {
    string rule = node->val;
    if (rule == "expr term") {
        string t = annotateExpr(node->children[0], proc, root); 
        node->type = t; 
        return t;
    } else if (rule == "term factor") {
        string t = annotateExpr(node->children[0], proc, root); 
        node->type = t; 
        return t;
    } else if (rule == "factor NUM") {
        node->type = "int";
         node->children[0]->type = "int"; 
         return "int";
    } else if (rule == "factor NULL") {
        node->type = "int*";
         node->children[0]->type = "int*"; 
         return "int*";
    } else if (rule == "factor LPAREN expr RPAREN") {
        string t = annotateExpr(node->children[1], proc, root);
         node->type = t; 
         return t;
    } else if (rule == "factor ID") {
        istringstream iss(node->children[0]->val);
        string token, name; iss >> token >> name;
        if (!tables[proc].second.count(name)) {
            errorAndClean(root);
            exit(1);
        }
        string t = tables[proc].second[name];
        node->type = t;
        node->children[0]->type = t;
        return t;
    }
    return "";
}

int main() {
    ParseTree* root = buildTree(cin);
    if (!root) { cerr << "ERROR" << endl; return 1; }
    // this is because every file when it starts should have a main and that cannot be less than 3
    if (root->children.size() < 3) return errorAndClean(root);

    // 
    ParseTree* procedures = root->children[1];
    if (procedures->children.empty()) return errorAndClean(root);
    ParseTree* mainNode = procedures->children[0];
    // must have main
    if (mainNode->val.substr(0, 4) != "main") return errorAndClean(root);


    // this is going to be useful for p4

    string procName;
    for (auto* c : mainNode->children) {
        if (c->val.substr(0, 4) == "WAIN" || c->val.substr(0, 2) == "ID") {
            istringstream iss(c->val);
            string tok, name; iss >> tok >> name;
            procName = name; break;
        }
    }

    
    if (procName.empty()) procName = "wain";
    tables[procName] = {{}, {}};

    ParseTree* dcl1 = mainNode->children[3];
    ParseTree* dcl2 = mainNode->children[5];
    auto [name1, type1] = processDcl(dcl1, procName, root, true);
    auto [name2, type2] = processDcl(dcl2, procName, root, true);
    if (name1 == name2 || type2 != "int") return errorAndClean(root);

    ParseTree* dcls = mainNode->children[8];
    processDcls(dcls, procName, root);

    ParseTree* expr = mainNode->children[11];
    string exprType = annotateExpr(expr, procName, root);
    if (exprType != "int") return errorAndClean(root);

    preorderTraversal(root);
    delete_tree(root);
    return 0;
}
