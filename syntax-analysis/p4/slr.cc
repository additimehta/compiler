#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <deque>
#include <map>
using namespace std;

struct rule {
    string lhs;
    vector<string> rhs;
};

void print_trace(deque<string>& reduct_tokens, deque<string>& input_tokens) {
    for (auto &t : reduct_tokens) cout << t << " ";
    cout << ". ";
    for (auto &t : input_tokens) cout << t << " ";
    cout << endl;
}

int main() {
    string line;
    string section = "";
    vector<rule> cfg_rules;
    deque<string> input_tokens;
    deque<string> reduct_tokens;
    vector<int> state_stack = {0};
    map<pair<int, string>, int> transitions;
    map<int, vector<pair<int, string>>> reductions;

    while (getline(cin, line)) {
        if (line == ".END") break;

        if (line == ".CFG" || line == ".INPUT" || line == ".TRANSITIONS" || line == ".REDUCTIONS") {
            section = line;
            continue;
        }

        if (section == ".CFG") {
            istringstream iss(line);
            string lhs;
            iss >> lhs;
            vector<string> rhs;
            string sym;
            while (iss >> sym) {
                if (sym != ".EMPTY") rhs.push_back(sym);
            }
            cfg_rules.push_back({lhs, rhs});
        }
        else if (section == ".INPUT") {
            istringstream iss(line);
            string token;
            while (iss >> token) input_tokens.push_back(token);
        }
        else if (section == ".TRANSITIONS") {
            istringstream iss(line);
            int from, to;
            string sym;
            iss >> from >> sym >> to;
            transitions[{from, sym}] = to;
        }
        else if (section == ".REDUCTIONS") {
            istringstream iss(line);
            int state, rule_num;
            string lookahead;
            iss >> state >> rule_num >> lookahead;
            reductions[state].emplace_back(rule_num, lookahead);
        }
    }

    print_trace(reduct_tokens, input_tokens);

    int shifted = 0;

    while (true) {
        int curr_state = state_stack.back();
        bool reduced = false;

        // SHIFT STEP
        if (!input_tokens.empty()) {
            string curr_symbol = input_tokens.front();
            int next_state = -1;

            if (transitions.count({curr_state, curr_symbol})) {
                next_state = transitions[{curr_state, curr_symbol}];
            }

            if (next_state != -1) {
                reduct_tokens.push_back(curr_symbol);
                input_tokens.pop_front();
                state_stack.push_back(next_state);
                shifted += 1;
                print_trace(reduct_tokens, input_tokens);
                continue;
            }
        }

        // REDUCE STEP
        if (reductions.count(curr_state)) {
            string lookahead; 
            if (input_tokens.empty()) {
                lookahead = "";
            } else {
                lookahead = input_tokens.front();
            }

            for (auto &[rule_num, expected_token] : reductions[curr_state]) {

                if (expected_token == ".ACCEPT") {
                    const rule &r = cfg_rules[rule_num];
                    for (size_t i = 0; i < r.rhs.size(); ++i) {
                        reduct_tokens.pop_back();
                        state_stack.pop_back();
                    }
                    reduct_tokens.push_back(r.lhs);
                    cout << r.lhs << " ." << endl;
                    return 0;
                }
                
                if (expected_token != lookahead) {
                    continue;
                }

                const rule &r = cfg_rules[rule_num];
                for (size_t i = 0; i < r.rhs.size(); ++i) {
                    reduct_tokens.pop_back();
                    state_stack.pop_back();
                }

                reduct_tokens.push_back(r.lhs);
                int next_state = transitions[{state_stack.back(), r.lhs}];
                state_stack.push_back(next_state);
                print_trace(reduct_tokens, input_tokens);
                reduced = true;
                break;
            }
        }

        if (!reduced) {
            cerr << "ERROR at " << shifted + 1 << endl;
            break;
        }
    }

    return 0;
}
