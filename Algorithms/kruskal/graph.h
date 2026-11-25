#pragma once

#include <vector>
#include <algorithm>


using namespace std;

struct Edge {
    int src;
    int dest;
    int weight;
};

struct Graph {
    int V;
    vector<Edge> edges;
};

struct DSU {
    vector<int> parent;
    vector<int> dsu_rank;
};

struct KruskalStep {
    Edge edge;
    int status;   // 1 = accepted, 0 = rejected
};


void initialize(DSU& dsu, int vertices) {
    dsu.parent.resize(vertices);
    dsu.dsu_rank.assign(vertices, 0);

    for (int i = 0; i < vertices; i++) {
        dsu.parent[i] = i;
    }
}

Graph create_graph(int number){
    Graph graph;
    graph.V=number;
    return graph;
}

void add_edge(Graph& graph , int src , int dest , int weight){
    Edge edge;

    edge.src=src;
    edge.dest=dest;
    edge.weight=weight;

    graph.edges.push_back(edge);
}

bool edge_exists(const Graph& graph, int src, int dest) {
    for (const Edge& edge : graph.edges) {
        if ((edge.src == src && edge.dest == dest) ||
            (edge.src == dest && edge.dest == src)) {
            return true;
        }
    }

    return false;
}

vector<int> unuse_vertex(Graph& graph){
    vector<int> unuse;
    for (int i=0;i<graph.V;i++){
        bool add=false;
        for (const Edge& edge : graph.edges){    
            if (i==edge.src || i==edge.dest){
                add = true;
                break;    
            }    
        }
        if(add==false){
            unuse.push_back(i);
        }  
    }
    return unuse;
}



int findd(DSU& dsu, int vertex) {
    if (dsu.parent[vertex] != vertex) {
        dsu.parent[vertex] = findd(dsu, dsu.parent[vertex]);
    }

    return dsu.parent[vertex];
}

void unite(DSU& dsu, int vertex1, int vertex2) {
    int root1 = findd(dsu, vertex1);
    int root2 = findd(dsu, vertex2);

    if (root1 == root2) {
        return;
    }

    if (dsu.dsu_rank[root1] < dsu.dsu_rank[root2]) {
        dsu.parent[root1] = root2;
    }
    else if (dsu.dsu_rank[root1] > dsu.dsu_rank[root2]) {
        dsu.parent[root2] = root1;
    }
    else {
        dsu.parent[root2] = root1;
        dsu.dsu_rank[root1]++;
    }
}

bool is_connected(const Graph& graph) {
    if (graph.V == 0) {
        return false;
    }

    DSU dsu;
    initialize(dsu, graph.V);

    for (const Edge& edge : graph.edges) {
        unite(dsu, edge.src, edge.dest);
    }

    int root = findd(dsu, 0);

    for (int i = 1; i < graph.V; i++) {
        if (findd(dsu, i) != root) {
            return false;
        }
    }

    return true;
}

vector<KruskalStep> kruskal(const Graph& graph) {
    vector<Edge> sorted_edges = graph.edges;
    vector<KruskalStep> steps;
    int accept=0;
    sort(sorted_edges.begin(), sorted_edges.end(),
        [](const Edge& a, const Edge& b) {
            return a.weight < b.weight;
        });

    DSU dsu;
    initialize(dsu, graph.V);

    for (const Edge& edge : sorted_edges) {

        if (findd(dsu, edge.src) != findd(dsu, edge.dest)) {
            steps.push_back({edge, 1});
            unite(dsu, edge.src, edge.dest);
            accept++;
        }
        else{
            steps.push_back({edge, 0});
        }

        if (accept == graph.V - 1) {
            break;
        }
    }

    return steps;
}




