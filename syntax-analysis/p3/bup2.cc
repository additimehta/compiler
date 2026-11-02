#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <deque>

using namespace std;

struct Rule {
    string lhs;
    vector<string> rhs;
};

int main() {
    string line;
    vector<Rule> rules;
    deque<string> input_seq;
    deque<string> actions;
    vector<string> reduct_seq;

    // Read CFG rules
    while (getline(cin, line)) {
        if (line == ".INPUT") break;
        if (line.empty() || line[0] == '.') continue;
        istringstream iss(line);
        Rule rule;
        iss >> rule.lhs;
        string sym;
        while (iss >> sym) {
            if (sym != ".EMPTY") {
                rule.rhs.push_back(sym);
            }
        }
        rules.push_back(rule);
    }

    // Read input sequence
    while (getline(cin, line)) {
        if (line == ".ACTIONS") break;
        istringstream iss(line);
        string tok;
        while (iss >> tok) {
            input_seq.push_back(tok);
        }
    }

    // Read actions
    while (getline(cin, line)) {
        if (line == ".END") break;
        if (!line.empty()) {
            actions.push_back(line);
        }
    }

    // Perform actions
    for (string action : actions) {
        if (action == "shift") {
            if (!input_seq.empty()) {
                reduct_seq.push_back(input_seq.front());
                input_seq.pop_front();
            }
        } else if (action.substr(0, 6) == "reduce") {
            int rule_num = stoi(action.substr(7));
            const Rule& rule = rules[rule_num];
            const vector<string>& rhs = rule.rhs;

            if (!rhs.empty()) {
                if (reduct_seq.size() < rhs.size()) {
                    cerr << "Error: not enough symbols on stack to reduce\n";
                    return 1;
                }

                bool match = true;
                for (size_t i = 0; i < rhs.size(); ++i) {
                    if (reduct_seq[reduct_seq.size() - rhs.size() + i] != rhs[i]) {
                        match = false;
                        break;
                    }
                }

                if (!match) {
                    cerr << "Error: RHS does not match top of reduction stack\n";
                    return 1;
                }

                for (size_t i = 0; i < rhs.size(); ++i) {
                    reduct_seq.pop_back();
                }
            }

            reduct_seq.push_back(rule.lhs);
        }else if (action == "print") {
            for (string tok : reduct_seq)
                cout << tok << " ";
            cout << ".";
            for (string tok : input_seq)
                cout << " " << tok;
            cout << endl;
        }
    }

    return 0;
}
