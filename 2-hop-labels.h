#ifndef N_2_HOP_LABELS_H
#define N_2_HOP_LABELS_H

#include <iostream>
#include <fstream>

#include <vector>
#include <stack>
#include <map>
#include <queue>
#include <algorithm>
#include <set>

using namespace std;

struct Edge {
    int headVex, tailVex;
    Edge *headLink, *tailLink;

    Edge(int headVex, int tailVex) {
        this->headVex = headVex;
        this->tailVex = tailVex;
        this->headLink = this->tailLink = nullptr;
    }
};

struct Node {
    bool tarjan;
    bool bfs;
    bool reverse_bfs;
    int data;
    set<int> inNodes, outNodes;
    Edge *firIn, *firOut;

    Node(int data) {
        this->data = data;
        this->tarjan = this->bfs = this->reverse_bfs = false;
        this->firIn = this->firOut = nullptr;
    }
};

struct Graph {
    int nodeNum, edgeNum;
    map<int, Node*> nodes;
} graph;


void tarjan(Node*, map<int, int>&, map<int, int>&, stack<int>&, vector<vector<int>>&, map<int, bool>&);
void combine_scc_node(vector<vector<int>>&);
bool query(int outNodeNum, int inNodeNum);
void search_out_node(Node*);
void search_in_node(Node*);


inline void insert_edge(Node* node, Edge* edge)
{
    Edge *last_edge = node->firOut;
    if (last_edge == nullptr) {
        node->firOut = edge;
        return;
    }

    while (last_edge->headLink != nullptr) {
        last_edge = last_edge->headLink;
    }

    last_edge->headLink = edge;
}

inline void insert_reverse_edge(Node* node, Edge* edge)
{

    Edge *last_edge = node->firIn;
    if (last_edge == nullptr) {
        node->firIn = edge;
        return;
    }

    while (last_edge->tailLink != nullptr) {
        last_edge = last_edge->tailLink;
    }

    last_edge->tailLink = edge;
}

inline Node* init_inexistent_node(int nodeNum)
{
    if (graph.nodes[nodeNum] == nullptr) {
        graph.nodes[nodeNum] = new Node(nodeNum);
    }

    return graph.nodes[nodeNum];
}

#endif //N_2_HOP_LABELS_H
