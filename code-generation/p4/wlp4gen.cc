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
int nonParamOffset = 0;

// Forward-declare the struct so it can be used in the function prototypes below.
struct ParseTree;

//forward declarations
void genExpr(ParseTree *expr);
void genTerm(ParseTree *term);
void genFactor(ParseTree *factor);
void processDcl(ParseTree *dcl, int offset);
void genStatements(ParseTree *statements);
void genStatement(ParseTree *statement);
void genTest(ParseTree *test);

void push(string reg = "$3") {
    cout << "sw " << reg << ", -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
}

void pop(string reg = "$5") {
    cout << "add $30, $30, $4" << endl;
    cout << "lw " << reg << ", -4($30)" << endl;  // FIXED: was "l2"
}

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
    cerr << "[DEBUG] buildTree line: " << line << endl;
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

void genStatements(ParseTree *statements) {
    cerr << "[DEBUG] genStatements: " << statements->val << endl;
    if(statements->children.empty())return;
    // statements -> statements statement
    genStatements(statements->children[0]);
    genStatement(statements->children[1]);
}

string resolveLvalue(ParseTree *lvalue) {
    cerr << "[DEBUG] resolveLvalue: " << lvalue->val << endl;
    // safety
    if (lvalue->children.empty()) {
        throw runtime_error("Invalid empty lvalue");
    }
    // lvalue → ID
    if (lvalue->children.size() == 1 &&
        lvalue->children[0]->val.substr(0, 2) == "ID") {
        string id = getLexeme(lvalue->children[0]);
        cerr << "[DEBUG] resolveLvalue base ID: " << id << endl;
        return id;
    }
    // lvalue → LPAREN lvalue RPAREN
    if (lvalue->children.size() == 3 &&
        lvalue->children[0]->val.substr(0, 6) == "LPAREN" &&
        lvalue->children[2]->val.substr(0, 6) == "RPAREN") {
        cerr << "[DEBUG] resolveLvalue nested LPAREN/RPAREN" << endl;
        return resolveLvalue(lvalue->children[1]);
    }
    cerr << "[DEBUG] Invalid lvalue structure caught: " << lvalue->val << endl;
    throw runtime_error("Invalid lvalue structure");
}

void genStatement(ParseTree *statement) {
    cerr << "[DEBUG] genStatement: " << statement->val << endl;
    // statement -> lvalue BECOMES expr SEMI
    if(statement->children[0]->val.substr(0,6) == "lvalue") {
        ParseTree *lvalue = statement->children[0];
        ParseTree *expr = statement->children[2];
        // lvalue ->ID or lvalue -> LPAREN lvalue RPAREN
        string varName = resolveLvalue(lvalue);
        cerr << "[DEBUG] Assigning to variable: " << varName << endl;
        genExpr(expr);
        int offset = symbolTable[varName];
        cerr << "[DEBUG] sw variable " << varName << " offset " << offset << endl;
        cout << "sw $3, " << offset << "($29)" << endl;
    }

    // statement → IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE
    else if (statement->children.size() == 11) {
        ParseTree *test = statement->children[2];
        ParseTree *trueBranch = statement->children[5];
        ParseTree *elseBranch = statement->children[9];

        static int ifCount = 0;
        string elseLabel = "else" + to_string(ifCount);
        string endLabel  = "endif" + to_string(ifCount);
        ifCount++;

        // Generate test (this will leave result in $3)
        genTest(test);

        // Jump to else if test result == 0
        cout << "beq $3, $0, " << elseLabel << endl;

        // Generate true branch
        genStatements(trueBranch);

        // Skip else after true branch
        cout << "beq $0, $0, " << endLabel << endl;

        // Else branch
        cout << elseLabel << ":" << endl;
        genStatements(elseBranch);

        // End of if
        cout << endLabel << ":" << endl;




   // statement → WHILE LPAREN test RPAREN LBRACE statements RBRACE
    }else if (statement->children.size() == 7 && statement->children[0]->val.substr(0, 5) == "WHILE") {
        static int loopCount = 0;
        string loopLabel = "loop" + to_string(loopCount);
        string endLabel = "endloop" + to_string(loopCount);
        loopCount++;

        ParseTree* testNode = statement->children[2];
        ParseTree* bodyNode = statement->children[5];

        cout << loopLabel << ":" << endl;

        genTest(testNode);  // ← This is what’s missing in your output
        cout << "beq $3, $0, " << endLabel << endl;

        genStatements(bodyNode);

        cout << "beq $0, $0, " << loopLabel << endl;
        cout << endLabel << ":" << endl;
    }

}

void processDcls(ParseTree *dcls) {
    cerr << "[DEBUG] processDcls: " << dcls->val << endl;
    // dcls .empty
    if(dcls->children.empty()) return;
    //dcls ->dcls dcls BECOMES NUM SEMI
    ParseTree* prevDcls = dcls->children[0];
    ParseTree* dcl = dcls->children[1];
    ParseTree* numNode = dcls->children[3];
    processDcls(prevDcls);
    string varName = getLexeme(dcl->children[1]);
    string numVal = getLexeme(numNode);
    nonParamOffset -=4;
    processDcl(dcl, nonParamOffset);
    cerr << "[DEBUG] Declared var: " << varName << " init=" << numVal << " offset=" << nonParamOffset << endl;
    if(numVal == "0") {
        cout << "sw $0, " << nonParamOffset << "($29)" << endl;
    }else {
        cout << "lis $3" << endl;
        cout << ".word " << numVal << endl;
        cout << "sw $3, " << nonParamOffset << "($29)" << endl;
    }


}

void processDcl(ParseTree* dcl, int offset) {
    cerr << "[DEBUG] processDcl: " << dcl->val << " offset=" << offset << endl;
    // dcl -> type ID
    ParseTree *idNode = dcl->children[1];
    string varName = getLexeme(idNode);
    symbolTable[varName] = offset;
}

void genFactor(ParseTree *factor) {
    cerr << "[DEBUG] genFactor: " << factor->val << endl;
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
        cerr << "[DEBUG] lw variable " << varName << " offset " << offset << endl;
        cout << "lw $3, " << offset << "($29)" << endl;
    }
    // factor -> LPAREN expr RPAREN
    else if (factor->children.size() == 3) {
        genExpr(factor->children[1]);
    }
}

void genTest(ParseTree *test) {
    cerr << "[DEBUG] genTest: " << test->val << endl;
    
    // Evaluate left expression
    genExpr(test->children[0]);
    push(); // push left result onto stack
    
    // Evaluate right expression
    genExpr(test->children[2]);
    pop(); // pop left result into $5
    
    // Now $5 has left operand, $3 has right operand
    // Extract just the operator token (first word of the line)
    istringstream iss(test->children[1]->val);
    string op;
    iss >> op;
    
    // test → expr EQ expr
    if (op == "EQ") {
        cout << "sub $3, $5, $3" << endl;     // diff = left - right
        cout << "sltu $6, $0, $3" << endl;    // $6 = 1 if diff != 0
        cout << "sub $3, $11, $6" << endl;    // invert → 1 if equal
    }
    // test → expr NE expr
    else if (op == "NE") {
        cout << "sub $3, $5, $3" << endl;     // diff = left - right
        cout << "sltu $3, $0, $3" << endl;    // $3 = 1 if not equal
    }
    // test → expr LT expr
    else if (op == "LT") {
        cout << "slt $3, $5, $3" << endl;     // $3 = 1 if left < right
    }
    // test → expr LE expr
    else if (op == "LE") {
        cout << "slt $6, $3, $5" << endl;     // $6 = 1 if right < left (i.e., left > right)
        cout << "sub $3, $11, $6" << endl;    // invert → 1 if left ≤ right
    }
    // test → expr GT expr
    else if (op == "GT") {
        cout << "slt $3, $3, $5" << endl;     // $3 = 1 if right < left (i.e., left > right)
    }
    // test → expr GE expr
    else if (op == "GE") {
        cout << "slt $6, $5, $3" << endl;     // $6 = 1 if left < right
        cout << "sub $3, $11, $6" << endl;    // invert → 1 if left ≥ right
    }
}



void genTerm(ParseTree *term) {

    cerr << "[DEBUG] genTerm: " << term->val << endl;
    // term -> factor
    if(term->children.size() == 1 && term->children[0]->val.substr(0,6) == "factor") {
        genFactor(term->children[0]);
    }
    // term -> term STAR factor
    else if(term->children.size() == 3 && term->children[1]->val.substr(0,4) == "STAR") {
        genTerm(term->children[0]);
        push();
        genFactor(term->children[2]);
        pop();
        cout << "mult $5, $3" << endl;
        cout << "mflo $3" << endl;
    }
    // term -> term SLASH factor
    else if(term->children.size() == 3 && term->children[1]->val.substr(0,5) == "SLASH") {
        genTerm(term->children[0]);
        push();
        genFactor(term->children[2]);
        pop();
        cout << "div $5, $3" << endl;
        cout << "mflo $3" << endl;
    }
    // term -> term PCT factor (% mod function)
    else if(term->children.size() == 3 && term->children[1]->val.substr(0,3) == "PCT") {
        genTerm(term->children[0]);
        push();
        genFactor(term->children[2]);
        pop();
        cout << "div $5, $3" << endl;
        cout << "mfhi $3" << endl;
    }
}

void genExpr(ParseTree *expr) {
    cerr << "[DEBUG] genExpr: " << expr->val << endl;
    // expr -> term
    if(expr->children.size() == 1 && expr->children[0]->val.substr(0,4) == "term") {
        genTerm(expr->children[0]);
    }
    // expr->expr PLUS term 
    else if(expr->children.size() == 3 && expr->children[1]->val.substr(0,4) == "PLUS") {
        genExpr(expr->children[0]);
        push();
        genTerm(expr->children[2]);
        pop();
        cout << "add $3, $5, $3" << endl; // left + right
    }
    // expr->expr MINUS term
    else if(expr->children.size() == 3 && expr->children[1]->val.substr(0,5) == "MINUS") {
        genExpr(expr->children[0]);
        push();
        genTerm(expr->children[2]);
        pop();
        cout << "sub $3, $5, $3" << endl; // left - right
    } 
}

void genMain(ParseTree *main) {
    cerr << "[DEBUG] genMain children size: " << main->children.size() << endl;
    for(size_t i = 0; i < main->children.size(); ++i)
        cerr << "[DEBUG] child[" << i << "]: " << main->children[i]->val << endl;
    // main -> INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
    // In your parse tree, expr is at index 11.
    ParseTree* dcl1 = main->children[3];
    ParseTree* dcl2 = main->children[5];
    ParseTree* expr = main->children[11];
    ParseTree* dcls = main->children[8];
    ParseTree *statements = main->children[9];
    processDcl(dcl1, 8);
    processDcl(dcl2, 4);
    // prologue
    cout << "; begin prologue" << endl;
    cout << "lis $4" << endl;
    cout << ".word 4" << endl;
    cout << "lis $11" << endl;
    cout << ".word 1" << endl;
    cout << "sw $1, -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
    cout << "sw $2, -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
    cout << "sub $29, $30, $4" << endl;  // SET FRAME POINTER - THIS IS CRITICAL!
    cout << "; end prologue" << endl;
    processDcls(dcls);
    genStatements(statements);
    genExpr(expr);
    // epilogue
    cout << "; begin epilogue" << endl;
    cout << "add $30, $30, $4" << endl;
    cout << "add $30, $30, $4" << endl;
    cout << "jr $31" << endl;
}

void generateCode(ParseTree *root) {
    cerr << "[DEBUG] generateCode: " << root->val << endl;
    // start -> BOF procedures EOF
    ParseTree* procedures = root->children[1];
    ParseTree* main = procedures->children[0];
    genMain(main);
}

int main() {
    try { 
        ParseTree *root = buildTree(cin);
        if (!root) throw runtime_error("empty input");
        
        cerr << "[DEBUG] Tree built successfully" << endl;
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
