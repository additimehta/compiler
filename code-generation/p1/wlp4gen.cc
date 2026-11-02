#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <stdexcept>     // <-- needed for runtime_error
using namespace std;

// offset from frame pointer ($29) 

// positive means it is a non-param
// negative means it is a param
map<string, int> symbolTable;

// Forward-declare the struct so it can be used in the function prototypes below.
struct ParseTree;

//forward declarations
void genExpr(ParseTree *expr);
void genTerm(ParseTree *term);
void genFactor(ParseTree *factor);

struct ParseTree {
    string val;
    vector<ParseTree*> children;
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

    // Stop recursion if this is a terminal or .EMPTY rule
    if (terminals.count(lhs) || line.find(".EMPTY") != string::npos) {
        return node;
    }

    // Collect RHS symbols, but stop before the colon (:) that introduces a type
    vector<string> rhs;
    string sym;
    while (iss >> sym) {
        if (sym == ":") break; // <-- ignore type annotation part
        rhs.push_back(sym);
    }

    // Recursively build children
    for (const auto& s : rhs) {
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

string getLexeme(ParseTree *node) {
    istringstream iss(node->val);
    string token, lexeme;
    iss >> token >> lexeme;
    return lexeme;
}

void processDcl(ParseTree* dcl, int offset) {
    // dcl -> type ID
    ParseTree *idNode = dcl->children[1];
    string varName = getLexeme(idNode);
    symbolTable[varName] = offset;
}

void genFactor(ParseTree *factor) {
    // factor -> NUM 
    if (factor->children.size() == 1 && factor->children[0]->val.substr(0,3) == "NUM") {
        string numVal = getLexeme(factor->children[0]);
        cout << "lis $3" << endl;
        cout << ".word " << numVal << endl;
    }
    // factor -> ID
    else if (factor->children.size() == 1 && factor->children[0]->val.substr(0,2) == "ID") {
        // non param
        string varName = getLexeme(factor->children[0]);
        int offset = symbolTable[varName];
        cout << "lw $3, " << offset << "($29)" << endl;
    }
    // factor -> LPAREN expr RPAREN
    else if (factor->children.size() == 3) {
        genExpr(factor->children[1]);
    }
}

void genTerm(ParseTree *term) {
    // term -> factor
    genFactor(term->children[0]);
}

void genExpr(ParseTree *expr) {
    // expr -> term
    genTerm(expr->children[0]);
}
void genMain(ParseTree *main) {
    // main -> INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
    // In your parse tree, expr is at index 11.
    ParseTree* dcl1 = main->children[3];
    ParseTree* dcl2 = main->children[5];
    ParseTree* expr = main->children[11];

    processDcl(dcl1, 8);
    processDcl(dcl2, 4);

    // prologue
    cout << "; begin prologue" << endl;
    cout << "lis $4" << endl;
    cout << ".word 4" << endl;
    cout << "sw $1, -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
    cout << "sw $2, -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
    cout << "sub $29, $30, $4" << endl;  // SET FRAME POINTER - THIS IS CRITICAL!
    cout << "; end prologue" << endl;

    genExpr(expr);

    // epilogue
    cout << "; begin epilogue" << endl;
    cout << "add $30, $30, $4" << endl;
    cout << "add $30, $30, $4" << endl;
    cout << "jr $31" << endl;
}



void generateCode(ParseTree *root) {
    // start -> BOF procedures EOF
    ParseTree* procedures = root->children[1];
    ParseTree* main = procedures->children[0];
    genMain(main);
}

int main() {
    try { 
        ParseTree *root = buildTree(cin);
        if (!root) throw runtime_error("empty input");
        
        generateCode(root);
        delete_tree(root);

        // good for debugging
        //preorderTraversal(root); 
        
    } catch (exception& e) {
        cerr << "ERROR: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
