#include <iostream>
#include <string>
#include <unordered_set>
#include <cctype>
#include <climits>
using namespace std;

// convert lowercase to uppercase
string toUpper(string s) {
    for (int i = 0; i < s.length(); i++) {
        if (s[i] >= 'a' && s[i] <= 'z') {
            s[i] = s[i] - 32; 
        }
    }
    return s;
}

// WLP4 reserved keywords
unordered_set<string> keywords = {
    "int", "wain", "return", "if", "else", "while",
    "println", "putchar", "delete", "new", "NULL"
};

int main() {
    string input = "";
    string line;

    // read all input into one string
    while (getline(cin, line)) {
        input += line;
        input += '\n';
    }

    int length = input.size();

    for (int i = 0; i < length; i++) {
        char c = input[i];

        // skip whitespace
        if (isspace(c)) {
            continue;
        }

        // skip comments
        else if (c == '/' && i + 1 < length && input[i + 1] == '/') {
            while (i < length && input[i] != '\n') {
                i++;
            }
            continue;
        }

        // identifiers and keywords
        else if (isalpha(c) || c == '_') {
            string lexeme = "";
            while (i < length && (isalnum(input[i]) || input[i] == '_')) {
                lexeme += input[i];
                i++;
            }
            i--; // backtrack one character
            if (keywords.find(lexeme) != keywords.end()) {
                cout << toUpper(lexeme) << " " << lexeme << endl;
            } else {
                cout << "ID " << lexeme << endl;
            }
        }

        // numbers
        else if (isdigit(c)) {
            string num = "";
            while (i < length && isdigit(input[i])) {
                num += input[i];
                i++;
            }
            i--;
            long long value = stoll(num);
            if (value > INT_MAX) {
                cerr << "ERROR" << endl;
                return 1;
            }
            cout << "NUM " << num << endl;
        }

        // two-character operators
        else if (c == '=' && i + 1 < length && input[i + 1] == '=') {
            cout << "EQ ==" << endl;
            i++;
        }
        else if (c == '!' && i + 1 < length && input[i + 1] == '=') {
            cout << "NE !=" << endl;
            i++;
        }
        else if (c == '<' && i + 1 < length && input[i + 1] == '=') {
            cout << "LE <=" << endl;
            i++;
        }
        else if (c == '>' && i + 1 < length && input[i + 1] == '=') {
            cout << "GE >=" << endl;
            i++;
        }

        // single-character tokens
        else if (c == '+') {
            cout << "PLUS +" << endl;
        }
        else if (c == '-') {
            cout << "MINUS -" << endl;
        }
        else if (c == '*') {
            cout << "STAR *" << endl;
        }
        else if (c == '/') {
            cout << "SLASH /" << endl;
        }
        else if (c == '%') {
            cout << "PCT %" << endl;
        }
        else if (c == '&') {
            cout << "AMP &" << endl;
        }
        else if (c == '=') {
            cout << "BECOMES =" << endl;
        }
        else if (c == '(') {
            cout << "LPAREN (" << endl;
        }
        else if (c == ')') {
            cout << "RPAREN )" << endl;
        }
        else if (c == '{') {
            cout << "LBRACE {" << endl;
        }
        else if (c == '}') {
            cout << "RBRACE }" << endl;
        }
        else if (c == '[') {
            cout << "LBRACK [" << endl;
        }
        else if (c == ']') {
            cout << "RBRACK ]" << endl;
        }
        else if (c == ';') {
            cout << "SEMI ;" << endl;
        }
        else if (c == ',') {
            cout << "COMMA ," << endl;
        }
        else if (c == '<') {
            cout << "LT <" << endl;
        }
        else if (c == '>') {
            cout << "GT >" << endl;
        }
        else {
            cerr << "ERROR" << endl;
            return 1;
        }
    }

    return 0;
}
