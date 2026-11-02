#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>
#include <sstream>
#include <string>

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
    std::map<std::string, uint32_t> labelMap;

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

    for(Token token : tokens)
    {
        std::cout << token << std::endl;
    }

    return 0;
}