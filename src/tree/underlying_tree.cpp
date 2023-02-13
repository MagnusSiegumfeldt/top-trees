#include "underlying_tree.h"
#include <iostream>

using namespace std;


Tree::Tree(int num_vertices) {
    this->num_vertices = num_vertices;
    vector<Vertex> vertices;
    for (int i = 0; i < num_vertices; i++) {
        vertices.push_back(Vertex(i));
    }
    this->vertices = vertices;
}

Tree::~Tree() {
    for (int i = 0; i < this->vertices.size(); i++) {
        Edge* current = vertices[i].get_first_edge();
        while (current) {
            this->del_edge(current);
            current = vertices[i].get_first_edge();
        }
    }
}
Edge::Edge(Vertex* left, Vertex* right, int weight) {
    this->endpoints[0] = left;
    this->endpoints[1] = right;
    
    this->weight = weight;
    this->user_data = nullptr;
    for (int i = 0; i < 2; i++) {
        this->prev[i] = nullptr;
        this->next[i] = nullptr;
    }
}

Edge* Tree::add_edge(int u, int v, int weight) {
    Vertex* left = &this->vertices[u];
    Vertex* right = &this->vertices[v];
    struct Edge* next[2] = {left->get_first_edge(), right->get_first_edge()};

    //Construct edge and set next
    Edge* edge = new Edge(left, right, weight);
    edge->next[0] = next[0],
    edge->next[1] = next[1];

    left->set_first_edge(edge);
    right->set_first_edge(edge);
    if (next[0]) {
        int vertex_is_right = next[0]->vertex_is_right(left);
        next[0]->prev[vertex_is_right] = edge;
    }
    if (next[1]) {
        int vertex_is_right = next[1]->vertex_is_right(right);
        next[1]->prev[vertex_is_right] = edge;
    }
    return edge;
}

void Tree::del_edge(Edge* edge) {
    del_edge_inner(edge->endpoints[0], edge->prev[0], edge->next[0]);
    del_edge_inner(edge->endpoints[1], edge->prev[1], edge->next[1]);

    delete edge;
}

void Tree::del_edge_inner(Vertex* vertex, Edge* prev, Edge* next) {
    if (prev) {
        int vertex_is_right = prev->vertex_is_right(vertex);
        prev->next[vertex_is_right] = next;
    } else {
        vertex->set_first_edge(next);
    }
    if (next) {
        int vertex_is_right = next->vertex_is_right(vertex);
        next->prev[vertex_is_right] = prev;
    }
}

void Tree::print_tree() {
    for (int i = 0; i < this->num_vertices; i++) {
        cout << i << ": "; 
        print_edges(&(this->vertices[i]));
        cout << endl;
    }
}
void Tree::print_edges(Vertex *vertex) {
    Edge *current = vertex->get_first_edge();
    while (current) {
        cout << "(" << current->endpoints[0]->id << "," << current->endpoints[1]->id << ") ";
        int vertex_is_right = current->vertex_is_right(vertex);
        current = current->next[vertex_is_right];
    }
}


bool Vertex::has_at_most_one_incident_edge() {
    Edge *first_edge = this->get_first_edge();
    if (first_edge) {
        int vertex_is_right = first_edge->vertex_is_right(this);
        return first_edge->next[vertex_is_right] == NULL;
    } else {
        return true;
    }
}

int Edge::vertex_is_right(Vertex* vertex) {
    return this->endpoints[1] == vertex;
}