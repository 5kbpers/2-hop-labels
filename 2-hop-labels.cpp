#include "2-hop-labels.h"


int main(int argc, char *argv[])
{
    if (argv[1] == nullptr) {
        cout << "Usage: ./a.out <filename>" << endl;
        exit(1);
    }

    ifstream fin(argv[1]);
    ofstream sccout("scc-out");
    ofstream labelout("label-out");

    cout << "Loading data from file " << argv[1] << "..." << endl;
    fin >> graph.nodeNum >> graph.edgeNum;

    int numberCount = 0;
    string inNodeID, outNodeID;
    int inNodeNum, outNodeNum;
    Node *inNode = nullptr, *outNode = nullptr;

    clock_t start = clock();

    for (int j = 0; j < graph.edgeNum; j++) {
        fin >> outNodeID >> inNodeID;

        if (graph.numberMap[outNodeID] == 0) {
            outNodeNum = graph.numberMap[outNodeID] = ++numberCount;
            graph.nameMap[outNodeNum] = outNodeID;
        }
        else {
            outNodeNum = graph.numberMap[outNodeID];
        }

        if (graph.numberMap[inNodeID] == 0) {
            inNodeNum = graph.numberMap[inNodeID] = ++numberCount;
            graph.nameMap[inNodeNum] = inNodeID;
        }
        else {
            inNodeNum = graph.numberMap[inNodeID];
        }

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

    for (auto &s: scc) {
        for (auto &i : s) {
            sccout << i << " ";
        }
        sccout << endl;
    }

    cout << "Combining scc..." << endl;
    combine_scc_node(scc);

    cout << "Building 2-hops-label's data structure..." << endl;
    for (auto beg = graph.nodes.begin(); beg != graph.nodes.end(); ++beg) {
        if (beg->first == beg->second->data) {
            search_out_node(beg->second);
            search_in_node(beg->second);

            labelout << graph.nameMap[beg->first] << endl;
            for (auto i : beg->second->outNodes) {
                labelout << graph.nameMap[i] << " ";
            }
            labelout << endl;

            for (auto i : beg->second->inNodes) {
                labelout << graph.nameMap[i] << " ";
            }
            labelout << endl;
        }
    }

    clock_t finish = clock();

    cout << "Total time: " << (finish-start)/CLOCKS_PER_SEC << " seconds" << endl;

    cout << "Please enter query data(e.g. from 1 to 2 => 1 2)" << endl;
    while (cin >> outNodeID >> inNodeID) {
        outNodeNum = graph.numberMap[outNodeID];
        inNodeNum = graph.numberMap[inNodeID];

        cout << query(outNodeNum, inNodeNum) << endl;
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
    for (auto &s : scc) {
        if (s.size() < 2) {
            continue;
        }
        int first = *(s.begin());
        map<int, bool> scc_sign;
        map<int, bool> accessed;
        for (auto &i: s)
            scc_sign[i] = true;

        for (auto beg = s.begin(); beg != s.end(); ++beg) {
            if (beg == s.begin()) {
                continue;
            }
            int r = *beg;
            for (Edge *edge = graph.nodes[r]->firOut; edge != nullptr;) {
                auto temp = edge;
                edge = edge->headLink;
                if (!scc_sign[temp->tailVex] && !accessed[temp->tailVex]) {
                    temp->headVex = first;
                    insert_edge(graph.nodes[first], temp);
                    accessed[temp->tailVex] = true;
                }
            }

            for (Edge *edge = graph.nodes[r]->firIn; edge != nullptr;) {
                auto temp = edge;
                edge = edge->tailLink;
                if (!scc_sign[temp->headVex] && !accessed[temp->headVex]) {
                    temp->tailVex = first;
                    insert_reverse_edge(graph.nodes[first], temp);
                    accessed[temp->headVex] = true;
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

        if (node->data != cur_node->data && !cur_node->bfs && !query(node->data, cur_node->data)) {
            for (Edge *edge = cur_node->firOut; edge != nullptr; edge = edge->headLink) {
                if (!accessed[edge->tailVex]) {
                    q.push(edge->tailVex);
                }
            }

            if (!accessed[cur_node->data]) {
                node->outNodes.push_back(cur_node->data);
                cur_node->inNodes.push_back(node->data);
                accessed[cur_node->data] = true;
            }
        }
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

        if (node->data != cur_node->data && !cur_node->reverse_bfs && !query(node->data, cur_node->data)) {
            for (Edge *edge = cur_node->firIn; edge != nullptr; edge = edge->tailLink) {
                if (!accessed[edge->headVex]) {
                    q.push(edge->headVex);
                }
            }

            if (!accessed[cur_node->data]) {
                node->inNodes.push_back(cur_node->data);
                cur_node->outNodes.push_back(node->data);
                accessed[cur_node->data] = true;
            }
        }
    }

    node->reverse_bfs = true;
}

bool query(int outNodeNum, int inNodeNum)
{
    Node *outNode = graph.nodes[ graph.nodes[outNodeNum]->data ];
    Node *inNode = graph.nodes[ graph.nodes[inNodeNum]->data ];

    vector<int> out_vec(outNode->outNodes);
    out_vec.push_back(graph.nodes[outNodeNum]->data);
    sort(out_vec.begin(), out_vec.end());

    vector<int> in_vec(inNode->inNodes);
    in_vec.push_back(graph.nodes[inNodeNum]->data);
    sort(in_vec.begin(), in_vec.end());

    vector<int> result;

    set_intersection(in_vec.begin(), in_vec.end(),
                     out_vec.begin(), out_vec.end(),
                     back_inserter(result));

    return !result.empty();
}

