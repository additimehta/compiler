#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <set>

using namespace std;

struct Rule {
    string lhs;
    vector<string> rhs;
};

void print_state(const vector<string>& symbol_stack, const deque<string>& input_seq) {
    for (const string& tok : symbol_stack) cout << tok << " ";
    cout << ".";
    for (const string& tok : input_seq) cout << " " << tok;
    cout << endl;
}

int main() {
    string line;
    vector<Rule> rules;
    deque<string> input_seq;
    vector<string> symbol_stack;
    vector<int> state_stack = {0};
    map<pair<int, string>, int> transitions;
    map<int, vector<pair<int, string>>> reductions;

    while (getline(cin, line)) {
        if (line == ".INPUT") break;
        if (line.empty() || line[0] == '.') continue;
        istringstream iss(line);
        Rule rule;
        iss >> rule.lhs;
        string sym;
        while (iss >> sym) {
            if (sym != ".EMPTY") rule.rhs.push_back(sym);
        }
        rules.push_back(rule);
    }

    while (getline(cin, line)) {
        if (line == ".TRANSITIONS") break;
        istringstream iss(line);
        string tok;
        while (iss >> tok) input_seq.push_back(tok);
    }

    while (getline(cin, line)) {
        if (line == ".REDUCTIONS") break;
        istringstream iss(line);
        int from, to;
        string sym;
        iss >> from >> sym >> to;
        transitions[{from, sym}] = to;
    }

    while (getline(cin, line)) {
        if (line == ".END") break;
        istringstream iss(line);
        int state, rule_num;
        string lookahead;
        iss >> state >> rule_num >> lookahead;
        reductions[state].emplace_back(rule_num, lookahead);
    }

    print_state(symbol_stack, input_seq);

    int shifted = 0;
    while (true) {
        int curr_state = state_stack.back();

        if (!input_seq.empty()) {
            string next_tok = input_seq.front();
            auto it = transitions.find({curr_state, next_tok});
            if (it != transitions.end()) {
                symbol_stack.push_back(next_tok);
                state_stack.push_back(it->second);
                input_seq.pop_front();
                ++shifted;
                print_state(symbol_stack, input_seq);
                continue;
            }
        }

        bool reduced = false;
        if (reductions.count(curr_state)) {
            string lookahead = input_seq.empty() ? "" : input_seq.front();
            for (auto& [rule_num, expected] : reductions[curr_state]) {
                if (expected == ".ACCEPT") {
                    const Rule& rule = rules[rule_num];
                    for (int i = 0; i < rule.rhs.size(); ++i) {
                        symbol_stack.pop_back();
                        state_stack.pop_back();
                    }
                    symbol_stack.push_back(rule.lhs);
                    cout << rule.lhs << " ." << endl;
                    return 0;
                }
                if (expected != lookahead) continue;
                const Rule& rule = rules[rule_num];
                for (int i = 0; i < rule.rhs.size(); ++i) {
                    symbol_stack.pop_back();
                    state_stack.pop_back();
                }
                symbol_stack.push_back(rule.lhs);
                int next_state = transitions[{state_stack.back(), rule.lhs}];
                state_stack.push_back(next_state);
                print_state(symbol_stack, input_seq);
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