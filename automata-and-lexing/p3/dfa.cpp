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

bool isChar(std::string s) {
  return s.length() == 1;
}
bool isRange(std::string s) {
  return s.length() == 3 && s[1] == '-';
}

using namespace std;
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
  while(in >> s) {
    string orginal = s;
    string input = (s == ".EMPTY") ? std::string{} : s;

    std::string state = inital_state;
    bool valid = true;

    for (char c : input) {
        auto stIt = transitions.find(state);
        if (stIt == transitions.end()) { valid = false; break; }

        auto symIt = stIt->second.find(c);
        if (symIt == stIt->second.end()) { valid = false; break; }

        state = symIt->second;
    }

    bool is_accepting =
        find(accept.begin(), accept.end(), state) != accept.end();

    // build display string using if/else
    string display_input;
    if (input.empty()) {
        display_input = ".EMPTY";
    } else {
        display_input = orginal;
    }

    string result;
    if (valid && is_accepting) {
        result = "true";
    } else {
        result = "false";
    }

    cout << display_input << ' ' << result << endl;
}







}