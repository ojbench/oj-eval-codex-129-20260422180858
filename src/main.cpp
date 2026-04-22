#include <bits/stdc++.h>
using namespace std;

// Scope simulator per Scope-2025 specification

struct Value {
    // 0 = int, 1 = string
    int type = -1;
    long long iv = 0; // use long long to avoid overflow during ops, but input guarantees in int range
    string sv;
};

struct VarEntry {
    string name;
    Value val;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    // Scopes: each level maintains name->Value map and a stack of declared names for quick deletion
    vector<unordered_map<string, Value>> scopes;
    vector<vector<string>> declared;
    scopes.emplace_back();
    declared.emplace_back();

    auto resolve = [&](const string &name) -> optional<Value> {
        for (int i = (int)scopes.size() - 1; i >= 0; --i) {
            auto it = scopes[i].find(name);
            if (it != scopes[i].end()) return it->second;
        }
        return nullopt;
    };

    auto resolve_ref = [&](const string &name) -> pair<int, unordered_map<string, Value>::iterator> {
        for (int i = (int)scopes.size() - 1; i >= 0; --i) {
            auto it = scopes[i].find(name);
            if (it != scopes[i].end()) return {i, it};
        }
        return {-1, scopes[0].end()};
    };

    auto invalid = [&]() {
        cout << "Invalid operation\n";
    };

    string op;
    string tmp;
    string line;
    // We'll parse per token basis using >>, with care for string literal values surrounded by quotes
    for (int i = 0; i < n; ++i) {
        if (!(cin >> op)) break;
        if (op == "Indent") {
            scopes.emplace_back();
            declared.emplace_back();
        } else if (op == "Dedent") {
            if (scopes.size() <= 1) {
                invalid();
                continue;
            }
            scopes.pop_back();
            declared.pop_back();
        } else if (op == "Declare") {
            string type, var;
            if (!(cin >> type >> var)) { invalid(); continue; }
            // Parse value: if string type, next token starts with quote; may contain spaces? Spec implies string literal is any characters inside quotes, but input is tokenized by spaces; typical OJ provides it as one token with quotes and no spaces. However we handle spaces by reading using std::getline from remaining stream.
            // We'll read next token as raw, if it starts with '"' and not ended, keep reading tokens and appending with space until ending quote encountered.
            string valueToken;
            if (!(cin >> valueToken)) { invalid(); continue; }

            Value v;
            if (type == "int") {
                // valueToken must be integer
                bool ok = true;
                // Allow leading + or -
                int idx = 0;
                if (valueToken[idx] == '+' || valueToken[idx] == '-') idx++;
                for (; idx < (int)valueToken.size(); ++idx) if (!isdigit((unsigned char)valueToken[idx])) { ok = false; break; }
                if (!ok) { invalid(); continue; }
                long long val = 0;
                try {
                    val = stoll(valueToken);
                } catch (...) {
                    invalid(); continue;
                }
                v.type = 0; v.iv = val;
            } else if (type == "string") {
                // Need to capture full quoted string
                string s = valueToken;
                if (s.size() >= 1 && s.front() == '"') {
                    if (s.size() == 1 || s.back() != '"') {
                        // keep reading tokens until we see ending quote
                        string more;
                        while (cin >> more) { s += ' '; s += more; if (!more.empty() && more.back() == '"') break; }
                    }
                }
                // Validate starts and ends with quotes
                if (s.size() < 2 || s.front() != '"' || s.back() != '"') { invalid(); continue; }
                // no escape processing required per spec
                v.type = 1; v.sv = s.substr(1, s.size() - 2);
            } else {
                invalid(); continue;
            }

            // Check: variable name must not already exist in current scope
            if (scopes.back().count(var)) { invalid(); continue; }
            scopes.back()[var] = v;
            declared.back().push_back(var);
        } else if (op == "Add") {
            string res, v1, v2;
            if (!(cin >> res >> v1 >> v2)) { invalid(); continue; }
            auto rref = resolve_ref(res);
            auto a1 = resolve(v1);
            auto a2 = resolve(v2);
            if (rref.first < 0 || !a1 || !a2) { invalid(); continue; }
            if (rref.second->second.type != a1->type || a1->type != a2->type) { invalid(); continue; }
            if (a1->type == 0) {
                long long sum = a1->iv + a2->iv;
                rref.second->second.iv = sum;
            } else {
                string sum = a1->sv + a2->sv;
                rref.second->second.sv = sum;
            }
        } else if (op == "SelfAdd") {
            string var; 
            if (!(cin >> var)) { invalid(); continue; }
            auto rref = resolve_ref(var);
            if (rref.first < 0) { invalid(); continue; }
            Value &target = rref.second->second;
            if (target.type == 0) {
                string token;
                if (!(cin >> token)) { invalid(); continue; }
                bool ok = true; int idx = 0; if (token[idx] == '+' || token[idx] == '-') idx++;
                for (; idx < (int)token.size(); ++idx) if (!isdigit((unsigned char)token[idx])) { ok = false; break; }
                if (!ok) { invalid(); continue; }
                long long addv = 0; try { addv = stoll(token); } catch (...) { invalid(); continue; }
                target.iv += addv;
            } else if (target.type == 1) {
                string token; if (!(cin >> token)) { invalid(); continue; }
                string s = token;
                if (s.size() >= 1 && s.front() == '"') {
                    if (s.size() == 1 || s.back() != '"') {
                        string more;
                        while (cin >> more) { s += ' '; s += more; if (!more.empty() && more.back() == '"') break; }
                    }
                }
                if (s.size() < 2 || s.front() != '"' || s.back() != '"') { invalid(); continue; }
                string add = s.substr(1, s.size() - 2);
                target.sv += add;
            } else {
                invalid(); continue;
            }
        } else if (op == "Print") {
            string var; if (!(cin >> var)) { invalid(); continue; }
            auto v = resolve(var);
            if (!v) { invalid(); continue; }
            if (v->type == 0) {
                cout << var << ":" << (long long)v->iv << '\n';
            } else if (v->type == 1) {
                cout << var << ":" << v->sv << '\n';
            } else {
                invalid();
            }
        } else {
            // unknown op
            invalid();
        }
    }

    return 0;
}

