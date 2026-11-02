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
struct VarInfo {
    int offset;
    string type;
};
map<string, VarInfo> symbolTable;

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
    string type;
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
        if (sym == ":")  {
            string t;
            if(iss >> t) {
                node->type = t;
                break;
            }
        }
        rhs.push_back(sym);
    }
    // Recursively build children
    for (const auto& s : rhs) {
        ParseTree* child = buildTree(in);
        if (child) node->children.push_back(child);
    }
    return node;
}

void printTree(ParseTree* node, int depth = 0) {
    if (!node) return;
    // indent using spaces
    for (int i = 0; i < depth; i++) cerr << "  ";
    // print this node
    cerr << node->val << endl;
    // print children recursively
    for (auto child : node->children) {
        printTree(child, depth + 1);
    }
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

string getToken(ParseTree *node) {
    istringstream iss(node->val);
    string token, lexeme;
    iss >> token >> lexeme;
    return token;
}

void genStatements(ParseTree *statements) {
    if(statements->children.empty())return;
    // statements -> statements statement
    genStatements(statements->children[0]);
    genStatement(statements->children[1]);
}

string resolveLvalue(ParseTree *lvalue) {
    // lvalue → ID
    if (lvalue->children.size() == 1 &&
        getToken(lvalue->children[0]) == "ID") {
        return getLexeme(lvalue->children[0]);
    }
    // lvalue → LPAREN lvalue RPAREN
    if (lvalue->children.size() == 3 &&
        getToken(lvalue->children[0]) == "LPAREN" &&
        getToken(lvalue->children[2]) == "RPAREN") {
        return resolveLvalue(lvalue->children[1]);
    }
    // lvalue → STAR factor   (e.g., *p)
    if (lvalue->children.size() == 2 &&
        getToken(lvalue->children[0]) == "STAR") {
        return "*";  // special marker
    }
    throw runtime_error("Invalid lvalue structure");
}
 
void genStatement(ParseTree *statement) {
    // statement -> lvalue BECOMES expr SEMI
    if(statement->children[0]->val.substr(0,6) == "lvalue" && statement->children[1]->val.substr(0,7) == "BECOMES") {
        ParseTree *lvalue = statement->children[0];
        ParseTree *expr = statement->children[2];
        // lvalue → STAR factor (like *p = expr)
        if(lvalue->children.size() == 2 && lvalue->children[0]->val.substr(0,4) == "STAR") {
            ParseTree *factor = lvalue->children[1];
            genFactor(factor);
            push("$3");
            genExpr(expr);
            pop("$5");
            cout << "sw $3, 0($5)" << endl;
        }else {
            // lvalue ->ID or lvalue -> LPAREN lvalue RPAREN
            string varName = resolveLvalue(lvalue);
            genExpr(expr);
            int offset = symbolTable[varName].offset;
            cout << "sw $3, " << offset << "($29)" << endl;
        }
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
        genTest(testNode);
        cout << "beq $3, $0, " << endLabel << endl;
        genStatements(bodyNode);
        cout << "beq $0, $0, " << loopLabel << endl;
        cout << endLabel << ":" << endl;
    }
    // statement → PUTCHAR LPAREN expr RPAREN SEMI
    else if(statement->children.size() == 5 && statement->children[0]->val.substr(0,7) == "PUTCHAR") {
        ParseTree *expr = statement->children[2];
        genExpr(expr);
        // write to addres 0xffff000c
        cout << "lis $5" << endl;
        cout << ".word 0xffff000c" << endl;
        cout << "sw $3, 0($5)" << endl;
    }
    //statement → PRINTLN LPAREN expr RPAREN SEMI
    else if(statement->children.size() == 5 && statement->children[0]->val.substr(0,7) == "PRINTLN") {
        ParseTree *expr = statement->children[2];
        genExpr(expr);
        cout << "add $1, $3, $0" << endl;
        push("$31");
        // call print
        cout << "lis $5" << endl;
        cout << ".word print" << endl;
        cout << "jalr $5" << endl;
        // restore $31
        pop("$31");
    }
}

void processDcls(ParseTree *dcls) {
    // dcls →
    if (dcls->children.empty()) return;
    // dcls → dcls dcl BECOMES (NUM | NULL) SEMI
    ParseTree* prevDcls = dcls->children[0];
    ParseTree* dcl      = dcls->children[1];
    ParseTree* rhsNode  = dcls->children[3];  
    processDcls(prevDcls);
    processDcl(dcl, nonParamOffset);
    nonParamOffset -= 4;  
    string rhsToken  = getToken(rhsNode);   // "NUM" or "NULL"
    if (rhsToken == "NULL") {
        // dcls → dcls dcl BECOMES NULL SEMI
        cout << "lis $3" << endl;
        cout << ".word 1" << endl;
        cout << "sw $3, -4($30)" << endl;
    } else if (rhsToken == "NUM") {
        // dcls → dcls dcl BECOMES NUM SEMI
        string numVal = getLexeme(rhsNode); 
        if (numVal == "0") {
            cout << "sw $0, -4($30)" << endl;
        } else {
            cout << "lis $3" << endl;
            cout << ".word " << numVal << endl;
            cout << "sw $3, -4($30)" << endl;
        }
    }
    cout << "sub $30, $30, $4" << endl;
}

void processDcl(ParseTree* dcl, int offset) {
    // dcl -> type ID
    ParseTree *idNode = dcl->children[1];
    ParseTree *typeNode = dcl->children[0];
    string varName = getLexeme(idNode);
    
    // Build type from the type node structure
    string varType;
    if (typeNode->children.size() == 2) {
        // type -> INT STAR
        varType = "int*";
    } else {
        // type -> INT
        varType = "int";
    }
    
    symbolTable[varName] = {offset, varType};
}

void genFactor(ParseTree *factor) {
    // factor -> NUM   
    if (factor->children.size() == 1 && factor->children[0]->val.substr(0,3) == "NUM") {
        string numVal = getLexeme(factor->children[0]);
        cout << "lis $3" << endl;
        cout << ".word " << numVal << endl;
        factor->type = "int"; 
    }
    // factor -> ID (variable)
    else if (factor->children.size() == 1 && factor->children[0]->val.substr(0,2) == "ID") {
        string varName = getLexeme(factor->children[0]);
        int offset = symbolTable[varName].offset;
        cout << "lw $3, " << offset << "($29)" << endl;
        factor->type = symbolTable[varName].type;
    }
    // factor → GETCHAR LPAREN RPAREN
    else if(factor->children.size() == 3 && 
            factor->children[0]->val.substr(0,7) == "GETCHAR") {
        cout << "lis $5" << endl;
        cout << ".word 0xffff0004" << endl;
        cout << "lw $3, 0($5)" << endl; 
    }
    // factor → LPAREN expr RPAREN
    else if (factor->children.size() == 3 && 
             factor->children[0]->val.substr(0,6) == "LPAREN") {
        genExpr(factor->children[1]);
    }
    // factor → ID LPAREN RPAREN (procedure call, no args)
    else if(factor->children.size() == 3 &&
            factor->children[0]->val.substr(0,2) == "ID" &&
            factor->children[1]->val.substr(0,6) == "LPAREN") {  // ← CHECK children[1]!
        string proc = getLexeme(factor->children[0]);
        push("$29");
        push("$31");
        cout << "lis $5" << endl;
        cout << ".word P" << proc << endl;
        cout << "jalr $5" << endl;
        pop("$31");
        pop("$29");
    }
    // factor → ID LPAREN arglist RPAREN (procedure call with args)    
    else if(factor->children.size() == 4 &&
            factor->children[0]->val.substr(0,2) == "ID" &&
            factor->children[1]->val.substr(0,6) == "LPAREN") {  // ← CHECK children[1]!
        string proc = getLexeme(factor->children[0]);
        ParseTree *arglist = factor->children[2];
        vector<ParseTree*> args;
        ParseTree *curr = arglist;
        while(true) {
            args.push_back(curr->children[0]);
            if(curr->children.size() == 1) break;
            curr = curr->children[2];
        }
        push("$29");
        push("$31");
        for(ParseTree *exprNode : args) {
            genExpr(exprNode);
            push("$3");
        }
        cout << "lis $5" << endl;
        cout << ".word P" << proc << endl;
        cout << "jalr $5" << endl;
        // Pop arguments
        for(size_t i = 0; i < args.size(); i++) {
            cout << "add $30, $30, $4" << endl;
        }
        pop("$31");
        pop("$29");
    }
    // factor → NULL
    else if(factor->children.size() == 1 && factor->children[0]->val.substr(0,4) == "NULL") {
        cout << "lis $3" << endl;
        cout << ".word 1" << endl;
    }
    // factor → AMP lvalue
    else if (factor->children.size() == 2 && getToken(factor->children[0]) == "AMP") {
        ParseTree *lv = factor->children[1];
        // Case 1: lvalue → ID
        if (lv->children.size() == 1 &&
            getToken(lv->children[0]) == "ID") {
            string name = getLexeme(lv->children[0]);
            int offset = symbolTable[name].offset;
            cout << "lis $5" << endl;
            cout << ".word " << offset << endl;
            cout << "add $3, $29, $5" << endl;
        }
        // Case 2: lvalue → STAR factor
        else if (lv->children.size() == 2 &&
                    getToken(lv->children[0]) == "STAR") {
            // &(*E) = E
            ParseTree *innerFactor = lv->children[1];
            genFactor(innerFactor);   // this leaves E in $3
        }
        // Case 3: parentheses: (lvalue)
        else if (lv->children.size() == 3 &&
                    getToken(lv->children[0]) == "LPAREN") {
            ParseTree *inner = lv->children[1];
            // recursively apply AMP rule
            ParseTree fakeFactor;
            fakeFactor.children.push_back(new ParseTree{"AMP &", {}});
            fakeFactor.children.push_back(inner);
            genFactor(&fakeFactor);
        }
        else {
            throw runtime_error("Invalid AMP lvalue");
        }
    }
    // factor → STAR factor
    else if(factor->children.size() == 2 && factor->children[0]->val.substr(0,4) == "STAR") {
        ParseTree *innerfactor = factor->children[1];
        genFactor(innerfactor);
        // load from memory
        cout << "lw $3, 0($3)" << endl;
        if(innerfactor->type == "int*") {
            factor->type = "int";
        }else if(innerfactor->type == "int**") {
            factor->type = "int*";
        }else {
            factor->type = "int";
        }
     }
}

void genTest(ParseTree *test) {
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
    // term -> factor
    if(term->children.size() == 1 && term->children[0]->val.substr(0,6) == "factor") {
        genFactor(term->children[0]);
        term->type = term->children[0]->type;
    }
    // term -> term STAR factor
    else if(term->children.size() == 3 && term->children[1]->val.substr(0,4) == "STAR") {
        genTerm(term->children[0]);
        push();
        genFactor(term->children[2]);
        pop();
        cout << "mult $5, $3" << endl;
        cout << "mflo $3" << endl;
        term->type = "int";
    }
    // term -> term SLASH factor
    else if(term->children.size() == 3 && term->children[1]->val.substr(0,5) == "SLASH") {
        genTerm(term->children[0]);
        push();
        genFactor(term->children[2]);
        pop();
        cout << "div $5, $3" << endl;
        cout << "mflo $3" << endl;
        term->type = "int";
    }
    // term -> term PCT factor (% mod function)
    else if(term->children.size() == 3 && term->children[1]->val.substr(0,3) == "PCT") {
        genTerm(term->children[0]);
        push();
        genFactor(term->children[2]);
        pop();
        cout << "div $5, $3" << endl;
        cout << "mfhi $3" << endl;
        term->type = "int";
    }
}

void genExpr(ParseTree *expr) {
    // expr -> term
    if(expr->children.size() == 1 && expr->children[0]->val.substr(0,4) == "term") {
        genTerm(expr->children[0]);
        expr->type = expr->children[0]->type;
    }
    // expr->expr PLUS term 
    else if(expr->children.size() == 3 && expr->children[1]->val.substr(0,4) == "PLUS") {
        genExpr(expr->children[0]);
        push(); // save left to $3
        genTerm(expr->children[2]); // right = $3
        pop(); // left stored in $5
        // left in $5, right in $3
        string leftType = expr->children[0]->type;
        string rightType = expr->children[2]->type;
        
        // *(p+5)
        if(leftType == "int*" && rightType == "int") {
            cout << "mult $3, $4" << endl;
            cout << "mflo $3" << endl;
            expr->type = "int*";
        }
        // *(5 + p)
        else if(leftType == "int" && rightType == "int*") {
            cout << "mult $5, $4" << endl;
            cout << "mflo $5" << endl;
            expr->type ="int*";
        }else {
            expr->type = "int";
        }
        cout << "add $3, $5, $3" << endl; // left + right
    }
    // expr->expr MINUS term
    else if(expr->children.size() == 3 &&
        expr->children[1]->val.substr(0,5) == "MINUS") {
        genExpr(expr->children[0]);   // left goes to $3
        push();                       // save left
        genTerm(expr->children[2]);   // right in $3
        pop();                        // left in $5
        string leftType = expr->children[0]->type;
        string rightType = expr->children[2]->type;
        // left in $5, right in $3
        // Case 1: pointer - int
        if (leftType == "int*" && rightType == "int") {
            cout << "mult $3, $4" << endl;
            cout << "mflo $3" << endl;
            cout << "sub $3, $5, $3" << endl;
            expr->type = "int*";
        }
        // Case 2: pointer - pointer
        else if (leftType == "int*" && rightType == "int*") {
            cout << "sub $3, $5, $3" << endl;
            cout << "div $3, $4" << endl;
            cout << "mflo $3" << endl;
            expr->type = "int";
        }
        // Case 3: int - int
        else if (leftType == "int" && rightType == "int") {
            cout << "sub $3, $5, $3" << endl;
            expr->type = "int";
        }
    }
}

void genMain(ParseTree *main) {
    symbolTable.clear();
    nonParamOffset = 0;
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
    cout << "wain: " << endl;
    cout << "; begin prologue" << endl;
    // push 1st parameter of wain ($1)
    cout << "sw $1, -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
    // push 2nd parameter of wain ($2)
    cout << "sw $2, -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
    // set the frame pointer, frame pointer is below the stack pointer
    cout << "sub $29, $30, $4" << endl;
    cout << "; end prologue" << endl;
    processDcls(dcls);
    genStatements(statements);
    genExpr(expr);
    // epilogue
    int num_locals = (-nonParamOffset) / 4;
    int num_pops = num_locals + 2;
    cout << "; begin epilogue" << endl;
    for(int i = 0; i < num_pops; i++) {
        cout << "add $30, $30, $4" << endl;
    }
    cout << "jr $31" << endl;
}

int processParamList(ParseTree *paramlist) {
    vector<pair<string, string>> params; // store both name and type
    ParseTree *node = paramlist;
    
    while(true) {
        ParseTree *dcl = node->children[0];
        string name = getLexeme(dcl->children[1]);
        ParseTree *typeNode = dcl->children[0];
        
        // Build type from structure
        string type;
        if (typeNode->children.size() == 2) {
            type = "int*";
        } else {
            type = "int";
        }
        params.push_back({name, type});
        
        // paramlist → dcl
        if(node->children.size() == 1){
            break;
        }
        // paramlist → dcl COMMA paramlist
        node = node->children[2];
    }
    
    int num_params = params.size();
    int offset = 4 * num_params;
    
    for(auto &p : params) {
        symbolTable[p.first] = {offset, p.second}; // store VarInfo with offset and type
        offset -= 4;
    }
    
    return num_params;
}

int processParams(ParseTree *params) {
    // params →
    if(params->children.size() == 0){
        return 0;
    }
    // params → paramlist
    else {
        return processParamList(params->children[0]);
    }
}

void genProcedure(ParseTree *procedure) {
    // procedure → INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
    string procName = getLexeme(procedure->children[1]);
    cout << "P" << procName << ":" << endl;
    symbolTable.clear();
    nonParamOffset = 0;
    // handle parameters 
    ParseTree *params = procedure->children[3];
    int paramCount = processParams(params);
    // setup frame pointer after params 
    cout << "sub $29, $30, $4" << endl;
    // handle dcls
    ParseTree *dcls = procedure->children[6];
    processDcls(dcls);
    // handle statements
    ParseTree *statements = procedure->children[7];
    genStatements(statements);
    // handle expr
    ParseTree *expr = procedure->children[9];
    genExpr(expr);
    int numlocals = (-nonParamOffset) / 4;
    for(int i = 0; i < numlocals; i++) {
        cout << "add $30, $30, $4" << endl;
    }
    cout << "jr $31" << endl;
}

void genProcedures(ParseTree *procedures) {
    // procedures -> main
    if(procedures->children.size() == 1 && procedures->children[0]->val.substr(0,4) == "main"){
        ParseTree* main = procedures->children[0];
        genMain(main);
        return;
    }
    // procedures → procedure procedures
    else if (procedures->children.size() == 2 &&
        procedures->children[0]->val.substr(0,9) == "procedure" &&
        procedures->children[1]->val.substr(0,10) == "procedures") 
    {
        ParseTree *procedure = procedures->children[0];
        ParseTree *rest = procedures->children[1];
        genProcedure(procedure);
        genProcedures(rest);
    }
}

int main() {
    try { 
        ParseTree *root = buildTree(cin);
        if (!root) throw runtime_error("empty input");
        ParseTree *procedures = root->children[1];
        cout << ".import print" << endl;
        // const 4 stored at $4
        cout << "lis $4" << endl;
        cout << ".word 4" << endl;
        // const 1 stored at $11
        cout << "lis $11" << endl;
        cout << ".word 1" << endl;
        cout << "beq $0, $0, wain" << endl;
        // start → BOF procedures EOF
        genProcedures(procedures);
        delete_tree(root);
    } catch (exception& e) {
        cerr << "ERROR: " << e.what() << endl;
        return 1;
    }
    return 0;
}