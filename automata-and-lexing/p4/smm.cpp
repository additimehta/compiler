#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
const std::string ALPHABET    = ".ALPHABET";
const std::string STATES      = ".STATES";
const std::string TRANSITIONS = ".TRANSITIONS";
const std::string INPUT       = ".INPUT";
const std::string EMPTY       = ".EMPTY";

using namespace std;
bool isChar(std::string s) {
  return s.length() == 1;
}
bool isRange(std::string s) {
  return s.length() == 3 && s[1] == '-';
}


char Peek(string x) {
    char first = x[0];
    return first;
}

bool goesTo(const unordered_map<string, unordered_map<char, string>>& transitions,
            const string& from,
            char c,
            const string& to) {
    auto itFrom = transitions.find(from);
    if (itFrom == transitions.end()) return false;          
    auto itChar = itFrom->second.find(c);
    if (itChar == itFrom->second.end()) return false;        
    return itChar->second == to; 
}


// Locations in the program that you should modify to store the
// DFA information have been marked with four-slash comments:
//// (Four-slash comment)
int main() {
  std::istream& in = std::cin;
  std::string s;
  vector<char> alpha;

  // --- ALPHABET SECTION ------

  std::getline(in, s); // Alphabet section (skip header)
  // Read characters or ranges separated by whitespace
  while(in >> s) {
    if (s == STATES) { 
      break; 
    } else {
      if (isChar(s)) {
        //// Variable 's[0]' is an alphabet symbol
        char symbol = s[0];
        if(count(alpha.begin(), alpha.end(), symbol) == 0){
            // symbol was not found
            alpha.push_back(symbol);
            
        }

      } else if (isRange(s)) {
        for(char c = s[0]; c <= s[2]; ++c) {
          //// Variable 'c' is an alphabet symbol
          for(char c = s[0]; c <= s[2]; c++){
            bool exists = false;
            for(char x : alpha) {
                if(x == c) {
                    exists = c;
                    break;
                }
            }
            if(!exists) {
                alpha.push_back(c);
            }
        
          }
        }


      } 
    }
  }


  // ------STATES-----------
  vector<string> states;
  string inital_state;
  vector<string> accept;

  std::getline(in, s); // States section (skip header)
  // Read states separated by whitespace
  while(in >> s) {
    if (s == TRANSITIONS) { 
      break; 
    } else {
      static bool initial = true;
      bool accepting = false;
      if (s.back() == '!' && !isChar(s)) {
        accepting = true;
        s.pop_back();
      }
      //// Variable 's' contains the name of a state
      states.push_back(s);
      if (initial) {
        //// The state is initial
        inital_state = s;
        initial = false;
      }
      if (accepting) {
        //// The state is accepting
        accept.push_back(s);
      }
    }
  }


  //// ----------- TRANSITIONS ---------
  unordered_map<string, unordered_map<char, string>>transitions;

  std::getline(in, s); // Transitions section (skip header)
  // Read transitions line-by-line
  while(std::getline(in, s)) {
    if (s == INPUT) { 
      // Note: Since we're reading line by line, once we encounter the
      // input header, we will already be on the line after the header
      break; 
    } else {
      std::string fromState, symbols, toState;
      std::istringstream line(s);
      std::vector<std::string> lineVec;
      while(line >> s) {
        lineVec.push_back(s);
      }
      fromState = lineVec.front();
      toState = lineVec.back();
      for(int i = 1; i < lineVec.size()-1; ++i) {
        std::string s = lineVec[i];
        if (isChar(s)) {
          symbols += s;
        } else if (isRange(s)) {
          for(char c = s[0]; c <= s[2]; ++c) {
            symbols += c;
          }
        }
      }
      for ( char c : symbols ) {
        //// There is a transition from 'fromState' to 'toState' on 'c'
        transitions[fromState][c] = toState;

      }
    }
  }


  /// ----- INPUT HEADER ---------
  // Input section (already skipped header)
  // Read and tokenize every input string after .INPUT

  in >> s;
  string text = s;
  size_t start = 0;

  // helper: check accepting quickly
  auto accepts = [&](const string& st) {
    return find(accept.begin(), accept.end(), st) != accept.end();
  };

  while (start < text.size()) {
    string state = inital_state;   // reset DFA to start
    string lexeme;                  // grows as we walk
    string best;                    // longest accepting lexeme from 'start'
    size_t best_end = start;        // end index of 'best'

    // walk forward from 'start' as long as transitions exist
    for (size_t j = start; j < text.size(); ++j) {
        char ch = text[j];

        // safe lookup: don't create entries accidentally
        auto itFrom = transitions.find(state);
        if (itFrom == transitions.end()) break;
        auto itEdge = itFrom->second.find(ch);
        if (itEdge == itFrom->second.end()) break;

        state = itEdge->second;      // follow edge
        lexeme.push_back(ch);        // tentatively consume

        if (accepts(state)) {        // remember last accepting point
            best = lexeme;
            best_end = j + 1;
        }
    }

    if (best.empty()) {
        cerr << "ERROR" << endl;
        return 1;
    }

    cout << best << endl;            // emit token
    start = best_end;                // continue from end of best match
  }
}