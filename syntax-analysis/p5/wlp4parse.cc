#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <utility>
#include <algorithm>
#include "wlp4data.h"

using namespace std;

struct ParseTree {
    string val;
    vector<ParseTree*> children;
};

void preorderTraverse(ParseTree* node) {
    if (!node) return;
    cout << node->val << endl;
    for (size_t i = 0; i < node->children.size(); ++i) {
        preorderTraverse(node->children[i]);
    }
}

void delete_tree(ParseTree* node) {
    if (!node) return;
    for (size_t i = 0; i < node->children.size(); ++i) {
        delete_tree(node->children[i]);
    }
    delete node;
}

struct Rule {
    string lhs;
    vector<string> rhs;
};

int main() {
    vector<Rule> rules;
    deque<pair<string, string>> input_seq;
    vector<pair<string, ParseTree*>> symbol_stack;
    vector<int> state_stack = {0};
    map<pair<int, string>, int> transitions;
    map<int, vector<pair<int, string>>> reductions;

    // --- Read CFG ---
    {
        istringstream cfgin(WLP4_CFG);
        string line;
        while (getline(cfgin, line)) {
            if (line.empty() || line[0] == '.') continue;
            istringstream iss(line);
            Rule r;
            iss >> r.lhs;
            string sym;
            while (iss >> sym) if (sym != ".EMPTY") r.rhs.push_back(sym);
            rules.push_back(r);
        }
    }

    // --- Read transitions ---
    {
        istringstream transin(WLP4_TRANSITIONS);
        string line;
        while (getline(transin, line)) {
            if (line.empty() || line[0] == '.') continue;
            istringstream iss(line);
            int from, to;
            string sym;
            iss >> from >> sym >> to;
            transitions[{from, sym}] = to;
        }
    }

    // --- Read reductions ---
    {
        istringstream redin(WLP4_REDUCTIONS);
        string line;
        while (getline(redin, line)) {
            if (line.empty() || line[0] == '.') continue;
            istringstream iss(line);
            int state, rule_num;
            string lookahead;
            iss >> state >> rule_num >> lookahead;
            reductions[state].emplace_back(rule_num, lookahead);
        }
    }

    // --- Read input tokens ---
    string kind, lexeme;
    input_seq.emplace_back("BOF", "BOF"); // BOF first
    while (cin >> kind >> lexeme) {
        input_seq.emplace_back(kind, lexeme);
    }
    input_seq.emplace_back("EOF", "EOF"); // EOF last

    int shifted = 0;

    while (true) {
        int curr_state = state_stack.back();

        // --- SHIFT ---
        if (!input_seq.empty()) {
            string tok_kind = input_seq.front().first;
            string tok_lex = input_seq.front().second;

            auto it = transitions.find({curr_state, tok_kind});
            if (it != transitions.end()) {
                ParseTree* node = new ParseTree{tok_kind + " " + tok_lex, {}};
                symbol_stack.emplace_back(tok_kind, node);
                state_stack.push_back(it->second);

                // Increment shifted only for real tokens (exclude BOF/EOF)
                if (tok_kind != "BOF" && tok_kind != "EOF") ++shifted;

                input_seq.pop_front();
                continue;
            }
        }

        // --- REDUCE ---
        bool reduced = false;
        string lookahead = input_seq.empty() ? "" : input_seq.front().first;
        if (reductions.count(curr_state)) {
            for (auto& [rule_num, expected] : reductions[curr_state]) {
                if (expected != lookahead && expected != ".ACCEPT") continue;

                const Rule& rule = rules[rule_num];

                // Build LHS node
                string full_rule = rule.lhs;
                if (rule.rhs.empty()) full_rule += " .EMPTY";
                for (const string& sym : rule.rhs) full_rule += " " + sym;

                ParseTree* node = new ParseTree{full_rule, {}};

                // Pop RHS nodes
                vector<ParseTree*> children;
                for (int j = rule.rhs.size() - 1; j >= 0; --j) {
                    children.push_back(symbol_stack.back().second);
                    symbol_stack.pop_back();
                    state_stack.pop_back();
                }

                // Reverse children for left-to-right order
                reverse(children.begin(), children.end());
                node->children = move(children);

                // Push LHS node
                symbol_stack.emplace_back(rule.lhs, node);
                int next_state = transitions[{state_stack.back(), rule.lhs}];
                state_stack.push_back(next_state);

                reduced = true;

                // --- ACCEPT ---
                if (expected == ".ACCEPT") {
                    ParseTree* top = symbol_stack.back().second;
                    preorderTraverse(top);
                    delete_tree(top);
                    for (auto& [_, ptr] : symbol_stack) {
                        if (ptr != top) delete_tree(ptr);
                    }
                    return 0;
                }

                break;
            }
        }

        // --- ERROR ---
        if (!reduced) {
            cerr << "ERROR at " << shifted + 1 << endl;
            for (auto& [_, ptr] : symbol_stack) delete_tree(ptr);
            break;
        }
    }

    return 0;
}
