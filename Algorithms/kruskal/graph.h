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

void initialize(int vertices) {
    parent.resize(vertices);

    for (int i = 0; i < vertices; i++) {
        parent[i] = i;
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

void sort_graph(Graph& graph){
    sort(graph.edges.begin(),
    graph.edges.end(),
    [](const Edge& a, const Edge& b) {return a.weight < b.weight;});
}

int find(int vertex){
    return parent[vertex];
}

void unite(int vertex1,int vertex2){
    if (parent[vertex1] != parent[vertex2]){
        if (parent[vertex1] <= parent[vertex2]){
            parent[vertex2] = parent[vertex1];
        }
        else {
            parent[vertex1] = parent[vertex2];
        }
    }
}

bool edge_exists(Graph& graph, int src, int dest) {
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

bool is_connected(Graph& graph){
    vector<int> unused=unuse_vertex(graph);
    if (unused.size()==0){
        return true;
    }
    else { return false;}

}
