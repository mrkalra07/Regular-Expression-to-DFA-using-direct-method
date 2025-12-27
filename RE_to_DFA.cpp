#include <bits/stdc++.h>

using namespace std;

struct Node {
    char symbol;
    set<int> firstpos;
    set<int> lastpos;
    bool nullable;
    Node* left;
    Node* right;
    int position; 

    Node(char symbol) : symbol(symbol), nullable(false), left(nullptr), right(nullptr), position(-1) {}
};

Node* createLeaf(char symbol, int pos) {
    Node* node = new Node(symbol);
    node->position = pos;
    node->firstpos.insert(pos);
    node->lastpos.insert(pos);
    node->nullable = false;
    return node;
}

Node* createInternalNode(char symbol, Node* left, Node* right) {
    Node* node = new Node(symbol);
    node->left = left;
    node->right = right;
    return node;
}

void computeFollowpos(Node* root, map<int, set<int>>& followpos) {
    if (!root) return;

    if (root->symbol == '.') {
        for (int i : root->left->lastpos) {
            followpos[i].insert(root->right->firstpos.begin(), root->right->firstpos.end());
        }
    } else if (root->symbol == '*') {
        for (int i : root->lastpos) {
            followpos[i].insert(root->firstpos.begin(), root->firstpos.end());
        }
    }

    computeFollowpos(root->left, followpos);
    computeFollowpos(root->right, followpos);
}

Node* buildSyntaxTree(const string& postfix, map<int, char>& positions) {
    stack<Node*> stk;
    int pos = 1;

    for (char ch : postfix) {
        if (isalnum(ch) || ch == '#') {
            Node* leaf = createLeaf(ch, pos);
            stk.push(leaf);
            positions[pos] = ch;
            pos++;
        } else {
            if (ch == '*') {
                Node* node = stk.top(); stk.pop();
                Node* starNode = createInternalNode(ch, node, nullptr);

                starNode->nullable = true;
                starNode->firstpos = node->firstpos;
                starNode->lastpos = node->lastpos;

                stk.push(starNode);
            } else if (ch == '|') {
                Node* right = stk.top(); stk.pop();
                Node* left = stk.top(); stk.pop();
                Node* orNode = createInternalNode(ch, left, right);

                orNode->nullable = left->nullable || right->nullable;
                orNode->firstpos = left->firstpos;
                orNode->firstpos.insert(right->firstpos.begin(), right->firstpos.end());
                orNode->lastpos = left->lastpos;
                orNode->lastpos.insert(right->lastpos.begin(), right->lastpos.end());

                stk.push(orNode);
            } else if (ch == '.') {
                Node* right = stk.top(); stk.pop();
                Node* left = stk.top(); stk.pop();
                Node* concatNode = createInternalNode(ch, left, right);

                concatNode->nullable = left->nullable && right->nullable;
                concatNode->firstpos = left->firstpos;
                if (left->nullable) {
                    concatNode->firstpos.insert(right->firstpos.begin(), right->firstpos.end());
                }
                concatNode->lastpos = right->lastpos;
                if (right->nullable) {
                    concatNode->lastpos.insert(left->lastpos.begin(), left->lastpos.end());
                }

                stk.push(concatNode);
            }
        }
    }

    return stk.top();
}

string infixToPostfix(const string& regex) {
    string postfix;
    stack<char> ops;
    map<char, int> precedence = {{'|', 1}, {'.', 2}, {'*', 3}};

    for (char ch : regex) {
        if (isalnum(ch) || ch == '#') {
            postfix += ch;
        } else if (ch == '(') {
            ops.push(ch);
        } else if (ch == ')') {
            while (!ops.empty() && ops.top() != '(') {
                postfix += ops.top();
                ops.pop();
            }
            ops.pop();
        } else {
            while (!ops.empty() && precedence[ops.top()] >= precedence[ch]) {
                postfix += ops.top();
                ops.pop();
            }
            ops.push(ch);
        }
    }

    while (!ops.empty()) {
        postfix += ops.top();
        ops.pop();
    }

    return postfix;
}

void printSet(const set<int>& s) {
    cout << "{";
    for (auto it = s.begin(); it != s.end(); ++it) {
        cout << *it;
        if (next(it) != s.end()) cout << ",";
    }
    cout << "}";
}

void createDFA(map<int,char> positions, map<int, set<int>> followpos, set<int> start){
    set<char> inputs;
    int finalPos=-1;

    for (const auto& pos : positions){
        if (pos.second == '#'){
            finalPos = pos.first;
        } else{
            inputs.insert(pos.second);
        }
    }
    
    set<int> finalStates;
    map<set<int>,int> states;
    queue<set<int>> newStates;

    newStates.push(start);
    states[start] = 0;
    cout << "Start State: Q0\n\nTransitions:\n";

    int index = 1;
    while(!newStates.empty()){
        set<int> s=newStates.front(); newStates.pop();
        
        if (s.count(finalPos)){
            finalStates.insert(states[s]);
        }
        for (char c: inputs){
            set<int> nextState;

            for (int pos: s){
                if (positions[pos] == c){
                    nextState.insert(followpos[pos].begin(), followpos[pos].end());
                }
            }
            if (!nextState.empty()){
                if (!states.count(nextState)){
                    newStates.push(nextState);
                    states[nextState] = index++;
                }
                cout << " \t • Q" << states[s] << " --" << c << "--> " << "Q" << states[nextState] << endl;
            }
        }
    }
    cout << "\nFinal State(s): ";
    for (int i: finalStates){
        cout << "Q" << i << " ";
    }
}
int main() {
    string regex;
    cout << "Enter regular expression: ";
    cin >> regex;
    string modifiedRegex;
    for (size_t i = 0; i < regex.length(); ++i) {
        modifiedRegex += regex[i];
        if (i + 1 < regex.length() &&
            (isalnum(regex[i]) || regex[i] == '*' || regex[i] == ')') &&
            (isalnum(regex[i+1]) || regex[i+1] == '(')) {
            modifiedRegex += '.';
        }
    }
    modifiedRegex += '.';
    modifiedRegex += '#';

    string postfix = infixToPostfix(modifiedRegex);

    map<int, char> positions;
    Node* root = buildSyntaxTree(postfix, positions);

    map<int, set<int>> followpos;
    computeFollowpos(root, followpos);

    cout << "i \tSymbols \tfirstpos(n)\tlastpos(n)\tfollowpos(i)" << endl;
    for (auto& entry : positions) {
        int pos = entry.first;
        char symbol = entry.second;
        cout << pos <<"\t";
        cout << symbol << "\t\t{";
        cout << pos << "}\t\t";
        cout << "{" << pos << "}\t\t";

        if (followpos.find(pos) != followpos.end()) {
            printSet(followpos[pos]);
        } else {
            cout << "-";
        }
        cout << endl;
    }

    stack<Node*> nodes;
    nodes.push(root);
    while (!nodes.empty()) {
        Node* node = nodes.top();
        nodes.pop();

        if (!node) continue;
        if (node->symbol == '|' || node->symbol == '.' || node->symbol == '*') {
            cout << "-\t" << node->symbol << "\t\t";
            printSet(node->firstpos);
            cout << "\t\t";
            printSet(node->lastpos);
            cout << "\t\t-\n";
        }
        nodes.push(node->right);
        nodes.push(node->left);
    }

    createDFA(positions, followpos, root->firstpos);
    return 0;
}
