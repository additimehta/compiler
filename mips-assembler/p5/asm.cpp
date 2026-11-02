#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cstdint>

using namespace std;

/** Prints an error to stderr with an "ERROR: " prefix, and newline suffix.
 *
 * @param message The error to print
 */
void formatError(const std::string & message)
{
    std::cerr << "ERROR: " << message << std::endl;
}

/** Convert a string representation of a number to an unsigned integer.
 *
 * If the string is "0", then 0 is returned.  If the string starts with "0x", the string is
 * interpreted as an unsigned hexidecimal number.  If the string starts with a "0", the string is
 * interpreted as an unsigned octal number.  Otherwise, the string is interpreted as a signed
 * decimal number.
 *
 * The function name is read as "string to uint64".
 *
 * @param s The string to parse
 * @return The uint64_t representation of the string
 */
uint64_t stouint64(const std::string & s)
{
    if(s == "0")
    {
        return 0;
    }
    if(s.starts_with("0x"))
    {
        return std::stol(s.substr(2), nullptr, 16);
    }

    if(s.starts_with("0"))
    {
        return std::stol(s.substr(1), nullptr, 8);
    }

    return std::stod(s);
}


/** For a given instruction, prints the instruction in the ascii representation of a call to
 *  compileLine().
 *
 * @param instruction The name of the instruction
 * @param one The value of the first parameter
 * @param two The value of the second parameter, 0 if the instruction has < 2 parameter
 * @param three The value of the third parameter, 0 if the instruction has < 3 parameter
 */
bool compileLine(const std::string & instruction,
                 uint32_t one,
                 uint32_t two,
                 uint32_t three)
{
    uint32_t values[3] = {one, two, three};

    std::cout << "compileLine(\"" << instruction << "\", ";
    for(size_t i = 0; i < 3; i += 1)
    {
        std::cout << (values[i] != 0 ? "0x" : "")
                  << std::hex << values[i] << (i < 2 ? ", " : "");
    }
    std::cout << ");" << std::endl;

    return true;
}



enum TokenType {
    // Not a real token type we output: a unique value for initializing a TokenType when the
    // actual value is unknown
    NONE,

    DIRECTIVE,
    LABEL,
    ID,
    HEXINT,
    REG,
    DEC,
    COMMA,
    LPAREN,
    RPAREN,
    NEWLINE,
};

class Token
{
public:
    const TokenType type;
    const std::string lexeme;

    Token(TokenType type, std::string lexeme);
};

Token::Token(TokenType t, std::string l)
  : type(t), lexeme(l)
{
    // Nothing
}




#define TOKEN_TYPE_PRINTER(t) case t: return #t
const char * tokenTypeString(TokenType t)
{
    switch(t)
    {
        TOKEN_TYPE_PRINTER(NONE);
        TOKEN_TYPE_PRINTER(LABEL);
        TOKEN_TYPE_PRINTER(DIRECTIVE);
        TOKEN_TYPE_PRINTER(ID);
        TOKEN_TYPE_PRINTER(HEXINT);
        TOKEN_TYPE_PRINTER(REG);
        TOKEN_TYPE_PRINTER(DEC);
        TOKEN_TYPE_PRINTER(COMMA);
        TOKEN_TYPE_PRINTER(LPAREN);
        TOKEN_TYPE_PRINTER(RPAREN);
    }

    // We will never get here
    return "";
}
#undef TOKEN_TYPE_PRINTER

#define TOKEN_TYPE_READER(s, t) if(s == #t) return t
TokenType stringToTokenType(const std::string & s)
{
    TOKEN_TYPE_READER(s, NONE);
    TOKEN_TYPE_READER(s, LABEL);
    TOKEN_TYPE_READER(s, DIRECTIVE);
    TOKEN_TYPE_READER(s, ID);
    TOKEN_TYPE_READER(s, HEXINT);
    TOKEN_TYPE_READER(s, REG);
    TOKEN_TYPE_READER(s, DEC);
    TOKEN_TYPE_READER(s, COMMA);
    TOKEN_TYPE_READER(s, LPAREN);
    TOKEN_TYPE_READER(s, RPAREN);
    return NONE;
}
#undef TOKEN_TYPE_READER

std::ostream & operator<<(std::ostream & out, const Token token)
{
    out << tokenTypeString(token.type) << " " << token.lexeme;
    return out;
}

/** Entrypoint for the assembler.  The first parameter (optional) is a mips assembly file to
 *  read.  If no parameter is specified, read assembly from stdin.  Prints machine code to stdout.
 *  If invalid assembly is found, prints an error to stderr, stops reading assembly, and return a
 *  non-0 value.
 *
 * If the file is not found, print an error and returns a non-0 value.
 *
 * @return 0 on success, non-0 on error
 */



// first pass will take in the tokenziation array, then it takes symbolTable to keep track of labels and their pc values
// optionally: added a symbolVec which just stores the labels in the correct order that we encountered them, useful for when printing
bool firstPass(const vector<Token>& tokens,map<string, uint32_t>& symbolTable, vector<pair<string, uint32_t>>& symbolVec){
    uint32_t PC = 0;
    for(int i = 0; i < tokens.size(); i++){
        const Token &t = tokens[i];


        // Labels take precedence so lets add that to the symbol table if we ever encounter one
        if(t.type == LABEL) {
            string name = t.lexeme.substr(0, t.lexeme.size() - 1);
            // check if the label already exists and if it does give an error
            if(symbolTable.count(name)) {
                formatError("duplicate label defintion");
                return false;

            }

            symbolTable[name] = PC;
            symbolVec.emplace_back(name, PC);


       }else if (t.type == ID || t.type == DIRECTIVE) {
            string inst = t.lexeme;

            if (inst == "beq" || inst == "bne") {
                if (i + 6 >= tokens.size() ||
                    tokens[i+1].type != REG || tokens[i+3].type != REG ||
                    tokens[i+2].type != COMMA || tokens[i+4].type != COMMA ||
                    (tokens[i+5].type != ID &&
                    tokens[i+5].type != DEC &&
                    tokens[i+5].type != HEXINT)) 
                {
                    formatError("invalid syntax for " + inst);
                    return false;
                }
                i += 5;
            }
            else if (inst == "add" || inst == "sub" || inst == "slt" || inst == "sltu") {
                if (i + 6 >= tokens.size() ||
                    tokens[i+1].type != REG || tokens[i+3].type != REG || tokens[i+5].type != REG ||
                    tokens[i+2].type != COMMA || tokens[i+4].type != COMMA) 
                {
                    formatError("invalid syntax for " + inst);
                    return false;
                }
                i += 5;
            }
            else if (inst == "mult" || inst == "multu" || inst == "div" || inst == "divu") {
                if (i + 4 >= tokens.size() ||
                    tokens[i+1].type != REG || tokens[i+2].type != COMMA ||
                    tokens[i+3].type != REG) 
                {
                    formatError("invalid syntax for " + inst);
                    return false;
                }
                i += 3;
            }
            else if (inst == "mfhi" || inst == "mflo" || inst == "lis" ||
                    inst == "jr"   || inst == "jalr") 
            {
                if (i + 2 >= tokens.size() || tokens[i+1].type != REG) {
                    formatError("invalid syntax for " + inst);
                    return false;
                }
                i += 1;
            }
            else if (inst == "lw" || inst == "sw") {
                if (i + 6 >= tokens.size() ||
                    tokens[i+1].type != REG || tokens[i+2].type != COMMA ||
                    (tokens[i+3].type != DEC && tokens[i+3].type != HEXINT) ||
                    tokens[i+4].type != LPAREN ||
                    tokens[i+5].type != REG || tokens[i+6].type != RPAREN) 
                {
                    formatError("invalid syntax for " + inst);
                    return false;
                }
                i += 6;
            }
            else if (inst == ".word") {
                if (i + 1 >= tokens.size() ||
                    !(tokens[i+1].type == HEXINT ||
                    tokens[i+1].type == DEC ||
                    tokens[i+1].type == ID)) 
                {
                    formatError("invalid .word operand");
                    return false;
                }
                i += 1;
            }
            else {
                formatError("unknown instruction: " + inst);
                return false;
            }

            PC += 4; 
        }

    }

    return true;
}


bool secondPass(const vector<Token> &tokens, const map<string, uint32_t>& symbolTable) {
    uint32_t PC = 0;  
    for (int i = 0; i < tokens.size(); i++) {
        const Token &t = tokens[i];

        if (t.type == ID) {
            string inst = t.lexeme;

            // ===== beq / bne =====
            if (inst == "beq" || inst == "bne") {

                // too large of an instruction
                if (i + 5 >= tokens.size()) {
                    formatError("invalid instruction: " + inst);
                    return false;
                }

                // if in good size check both registers

                if (tokens[i+1].type != REG || tokens[i+3].type != REG) {
                    formatError("registers are not in right format" + inst);
                    return false;
                }

                uint32_t reg1 = stouint64(tokens[i + 1].lexeme.substr(1));
                uint32_t reg2 = stouint64(tokens[i + 3].lexeme.substr(1));

                const Token& op3 = tokens[i + 5];
                int32_t branchOffset = 0;

                if (op3.type == ID) {
                    string label = op3.lexeme;
                    if (!symbolTable.count(label)) {
                        formatError("label does not exist on symbol table " + label);
                        return false;
                    }

                    // calculation of branch offset (labelAdderess - PC) / 4
                    branchOffset = (int32_t(symbolTable.at(label)) - int32_t(PC + 4)) / 4;
                } else {
                    // not a label so just a number
                    branchOffset = (int32_t)stouint64(op3.lexeme);
                }

                cout << inst << " " << reg1 << " " << reg2 << " "
                     << static_cast<uint16_t>(branchOffset) << endl;


                // move up
                i += 5;
            }

            // ===== add / sub / slt / sltu =====
            else if (inst == "add" || inst == "sub" || inst == "slt" || inst == "sltu") {
                if (i + 5 >= tokens.size()) {
                    formatError("not an r-format instruction " + inst);
                    return false;
                }

                uint32_t dest = stouint64(tokens[i + 1].lexeme.substr(1));
                uint32_t  reg1 = stouint64(tokens[i + 3].lexeme.substr(1));
                uint32_t reg2 = stouint64(tokens[i + 5].lexeme.substr(1));

                cout << inst << " " << dest << " " << reg1 << " " << reg2 << endl;
                i += 5;
            }

            // ===== mult / div / variants =====
            else if (inst == "mult" || inst == "multu" || inst == "div" || inst == "divu") {
                if (i + 3 >= tokens.size()) {
                    formatError("not right format" + inst);
                    return false;
                }

                uint32_t s  = stouint64(tokens[i + 1].lexeme.substr(1));
                uint32_t t_ = stouint64(tokens[i + 3].lexeme.substr(1));

                cout << inst << " " << s << " " << t_ << " 0" << endl;
                i += 3;
            }

            // ===== mfhi / mflo / lis =====
            else if (inst == "mfhi" || inst == "mflo" || inst == "lis") {
                if (i + 1 >= tokens.size()) {
                    formatError("format is incorrect " + inst);
                    return false;
                }

                uint32_t d = stouint64(tokens[i + 1].lexeme.substr(1));
                cout << inst << " " << d << " 0 0" << endl;
                i += 1;
            }

            // ===== lw / sw =====
            else if (inst == "lw" || inst == "sw") {
                if (i + 6 >= tokens.size()) {
                    formatError("format is incorrect" + inst);
                    return false;
                }

                uint32_t t_  = stouint64(tokens[i + 1].lexeme.substr(1));
                uint32_t imm = stouint64(tokens[i + 3].lexeme);
                uint32_t s   = stouint64(tokens[i + 5].lexeme.substr(1));

                cout << inst << " " << t_ << " " << s << " " << imm << endl;
                i += 6;
            }

            // ===== jr / jalr =====
            else if (inst == "jr" || inst == "jalr") {
                if (i + 1 >= tokens.size()) {
                    formatError("incomplete " + inst);
                    return false;
                }

                uint32_t s = stouint64(tokens[i + 1].lexeme.substr(1));
                cout << inst << " " << s << " 0 0" << endl;
                i += 1;
            }

            // ===== unknown =====
            else {
                formatError("unknown instruction: " + inst);
                return false;
            }

            PC += 4;
        }

        // ===== .word directive =====
        else if (t.type == DIRECTIVE && t.lexeme == ".word") {
            if (i + 1 >= tokens.size()) {
                formatError("missing operand for .word");
                return false;
            }

            string val = tokens[i + 1].lexeme;
            uint32_t num = 0;

            if (symbolTable.count(val)) {
                num = symbolTable.at(val);
            } else {
                num = stouint64(val);
            }

            cout << "word " << num << " 0 0" << endl;
            i += 1;
            PC += 4;
        }
    }

    return true;
}

int main(int argc, char * argv[])
{
    if(argc > 2)
    {
        std::cerr << "Usage:" << std::endl
                  << "\tasm [$FILE]" << std::endl
                  << std::endl
                  << "If $FILE is unspecified or if $FILE is `-`, read the assembly from standard "
                  << "in. Otherwise, read the assembly from $FILE." << std::endl;
        return 1;
    }

    std::ifstream fp;
    std::istream &in =
        (argc > 1 && std::string(argv[1]) != "-")
      ? [&]() -> std::istream& {
            fp.open(argv[1]);
            return fp;
        }()
      : std::cin;

    if(!fp && argc > 1)
    {
        formatError((std::stringstream() << "file '" << argv[1] << "' not found!").str());
        return 1;
    }

    std::vector<Token> tokens;
    std::map<std::string, uint32_t> symbolTable;
    vector<pair<string, uint32_t>> symbolVec;


    while(!in.eof())
    {
        std::string line;
        std::getline( in, line );
	if(line == "")
	{
            continue;
	}

        std::string tokenType;
        std::string lexeme;

        std::stringstream lineParser(line);
        lineParser >> tokenType;
        if(tokenType != "NEWLINE")
        {
            lineParser >> lexeme;
        }

        tokens.push_back(Token(stringToTokenType(tokenType), lexeme));
    }


    if(!firstPass(tokens, symbolTable, symbolVec)) {
        return 1;
    }

    if(!secondPass(tokens, symbolTable)) {
        return 1;
    }

    // print symbol table in right order
    for (const auto& entry : symbolVec) {
        cerr << entry.first << " " << entry.second << endl;
    }


    /*
    for(Token token : tokens)
    {
        std::cout << token << std::endl;
    }

    */

    return 0;
}
