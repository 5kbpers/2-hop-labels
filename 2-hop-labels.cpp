#include "2-hop-labels.h"


int main(int argc, char *argv[])
{
    if (argv[1] == nullptr) {
        cout << "Usage: ./a.out <filename>" << endl;
        exit(1);
    }

    ifstream fin(argv[1]);
    cout << "Loading data from file " << argv[1] << "..." << endl;
    fin >> graph.nodeNum >> graph.edgeNum;

    int inNodeNum, outNodeNum;
    Node *inNode = nullptr, *outNode = nullptr;
    for (int j = 0; j < graph.edgeNum; j++) {
        fin >> outNodeNum >> inNodeNum;
        outNode = init_inexistent_node(outNodeNum);
        inNode = init_inexistent_node(inNodeNum);
        Edge *edge = new Edge(outNodeNum, inNodeNum);

        insert_edge(outNode, edge);
        insert_reverse_edge(inNode, edge);
    }

    cout << "Finding scc..." << endl;
    map<int, int> def;
    map<int, int> low;
    map<int, bool> stack_sign;
    stack<int> st;
    vector<vector<int>> scc;

    for (auto beg = graph.nodes.begin(); beg != graph.nodes.end(); ++beg) {
        if (!beg->second->tarjan) {
            tarjan(beg->second, def, low, st, scc, stack_sign);
        }
    }

    for (auto beg = scc.begin(); beg != scc.end(); ++beg) {
        cout << "scc1 :";
        for (int r : *beg) {
            cout << r << " ";
        }
        cout << endl;
    }

    combine_scc_node(scc);

    cout << "Building 2-hops-label's data structure..." << endl;
    for (auto beg = graph.nodes.begin(); beg != graph.nodes.end(); ++beg) {
        search_out_node((*beg).second);
        search_in_node((*beg).second);
    }

    cout << "Please enter query data(e.g. from 1 to 2 => 1 2)" << endl;
    while (cin >> outNodeNum >> inNodeNum) {
        cout << query(outNodeNum,inNodeNum) << endl;
        cout << "Please enter query data(e.g. from 1 to 2 => 1 2)" << endl;
    }

    return 0;
}

void tarjan(Node *node, map<int, int> &def, map<int, int> &low, stack<int> &st,
            vector<vector<int>> &scc, map<int, bool> &stack_sign)
{
    if (node == nullptr) {
        cout << "In tarjan function: the variable 'node' is nullptr" << endl;
        exit(1);
    }

    static int index = 0;
    def[node->data] = low[node->data] = ++index;
    st.push(node->data);
    stack_sign[node->data] = true;
    node->tarjan = true;

    Edge *edge = node->firOut;
    while (edge != nullptr) {
        Node *next_node = graph.nodes[edge->tailVex];
        if (!next_node->tarjan) {
            tarjan(next_node, def, low, st, scc, stack_sign);
            low[node->data] = min(low[node->data], low[next_node->data]);
        }
        else if(stack_sign[next_node->data]) {
            low[node->data] = min(low[node->data], def[next_node->data]);
        }

        edge = edge->headLink;
    }

    if (def[node->data] == low[node->data]) {
        vector<int> new_scc;
        while (1) {
            int top_data = st.top();
            stack_sign[top_data] = false;
            new_scc.push_back(top_data);
            st.pop();
            if (top_data == node->data) {
                break;
            }
        }
        scc.push_back(new_scc);
    }
}

void combine_scc_node(vector<vector<int>> &scc)
{
    for (auto s : scc) {
        int first = *(s.begin());
        for (auto beg = s.begin(); beg != s.end(); ++beg) {
            if (beg == s.begin()) {
                continue;
            }
            int r = *beg;
            for (Edge *edge = graph.nodes[r]->firOut; edge != nullptr; edge = edge->headLink) {
                if (edge->tailVex != first) {
                    edge->headVex = first;
                    insert_edge(graph.nodes[first], edge);
                }
            }

            for (Edge *edge = graph.nodes[r]->firIn; edge != nullptr; edge = edge->tailLink) {
                if (edge->headVex != first) {
                    edge->tailVex = first;
                    insert_reverse_edge(graph.nodes[first], edge);
                }
            }
            graph.nodes[r]->data = first;
            graph.nodes[r]->firIn = nullptr;
            graph.nodes[r]->firOut = nullptr;
        }
    }
}

void search_out_node(Node *node)
{
    queue<int> q;
    map<int, bool> accessed;
    Node *cur_node = node;

    for (Edge *edge = cur_node->firOut; edge != nullptr; edge = edge->headLink)
        q.push(edge->tailVex);

    while (!q.empty()) {
        cur_node = graph.nodes[q.front()];
        q.pop();

        if (!cur_node->bfs && !query(node->data, cur_node->data)) {
            for (Edge *edge = cur_node->firOut; edge != nullptr; edge = edge->headLink) {
                if (!accessed[edge->tailVex]) {
                    q.push(edge->tailVex);
                }
            }
        }

        node->outNodes.insert(cur_node->data);
        accessed[cur_node->data] = true;
    }

    node->bfs = true;
}

void search_in_node(Node *node)
{
    queue<int> q;
    map<int, bool> accessed;
    Node *cur_node = node;

    for (Edge *edge = cur_node->firIn; edge != nullptr; edge = edge->tailLink)
        q.push(edge->headVex);

    while (!q.empty()) {
        cur_node = graph.nodes[q.front()];
        q.pop();

        if (!cur_node->reverse_bfs && !query(node->data, cur_node->data)) {
            for (Edge *edge = cur_node->firIn; edge != nullptr; edge = edge->tailLink) {
                if (!accessed[edge->headVex]) {
                    q.push(edge->headVex);
                }
            }
        }

        node->inNodes.insert(cur_node->data);
        accessed[cur_node->data] = true;
    }

    node->reverse_bfs = true;
}

bool query(int outNodeNum, int inNodeNum)
{
    Node *outNode = graph.nodes[ graph.nodes[outNodeNum]->data ];
    Node *inNode = graph.nodes[ graph.nodes[inNodeNum]->data ];

    set<int> out_set(outNode->outNodes);
    for (auto o : outNode->outNodes) {
        Node *n = graph.nodes[o];
        set_union(n->outNodes.begin(), n->outNodes.end(),
                  out_set.begin(), out_set.end(),
                  inserter(out_set, out_set.end()));
    }
    out_set.insert(outNodeNum);

    set<int> in_set(inNode->inNodes);
    for (auto o : inNode->inNodes) {
        Node *n = graph.nodes[o];
        set_union(n->inNodes.begin(), n->inNodes.end(),
                  in_set.begin(), in_set.end(),
                  inserter(in_set, in_set.end()));
    }
    in_set.insert(inNodeNum);

    set<int> result;

    set_intersection(in_set.begin(), in_set.end(),
                     out_set.begin(), out_set.end(),
                     inserter(result, result.begin()));

    return !result.empty();
}

