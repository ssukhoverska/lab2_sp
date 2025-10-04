#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.h"
using namespace std;

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string input;
    if (argc >= 2) {
        ifstream f(argv[1]);
        if (!f) { cerr << "Cannot open file\n"; return 1; }
        stringstream ss; ss << f.rdbuf(); input = ss.str();
    }
    else {
        stringstream ss; ss << cin.rdbuf(); input = ss.str();
    }

    Lexer lexer(input);
    auto tokens = lexer.tokenize();
    for (auto& t : tokens) {
        cout << "<" << t.lexeme << ", " << tokenTypeToString(t.type) << ">\n";
    }
    return 0;
}
