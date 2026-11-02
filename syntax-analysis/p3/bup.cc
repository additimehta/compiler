#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <deque>
using namespace std;

struct rule {
    string lhs;
    vector<string> rhs;
};

int main() {
    string line;
    string section = "";
    vector<rule> cfg_rules;
    deque<string> input_tokens;
    deque<string> reduct_tokens;
    vector<string> actions;

    while (getline(cin, line)) {
        if (line == ".END") break;

        if (line == ".CFG" || line == ".INPUT" || line == ".ACTIONS") {
            section = line;
            continue;
        }

        if (section == ".CFG") {
            istringstream iss(line);
            string lhs;
            iss >> lhs;
            vector<string> rhs;
            string symbol;
            while (iss >> symbol) {
                if (symbol != ".EMPTY") {
                    rhs.push_back(symbol);
                }
            }
            cfg_rules.push_back({lhs, rhs});
        } else if (section == ".INPUT") {
            istringstream iss(line);
            string token;
            while (iss >> token) {
                input_tokens.push_back(token);
            }
        } else if (section == ".ACTIONS") {
            actions.push_back(line);
        }
    }

    // Execute actions
    for (string action_line : actions) {
        istringstream iss(action_line);
        string command;
        iss >> command;

        if (command == "print") {
            for (auto &t : reduct_tokens) cout << t << " ";
            cout << ". ";
            for (auto &t : input_tokens) cout << t << " ";
            cout << endl;
        }

        else if (command == "shift") {
            if (!input_tokens.empty()) {
                reduct_tokens.push_back(input_tokens.front());
                input_tokens.pop_front();
            }
        }

        else if (command == "reduce") {
            int n;
            iss >> n;
            rule &r = cfg_rules[n];

            for (int i = (int)r.rhs.size() - 1; i >= 0; i--) {
                if (reduct_tokens.empty() || reduct_tokens.back() != r.rhs[i]) {
                    cerr << "Error: RHS does not match reduction sequence" << endl;
                    return 1;
                }
                reduct_tokens.pop_back();
            }
            reduct_tokens.push_back(r.lhs);
        }
    }

    return 0;
}
