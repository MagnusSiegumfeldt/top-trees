#include "top_tree.h"

template<class C, class E, class V>
LeafNode<C,E,V>::LeafNode(Edge<C,E,V>* e, int num_boundary) {
    this->parent = nullptr;
    this->edge = e;  
    this->num_boundary_vertices = num_boundary;
    this->merge();  
}   

template<class C, class E, class V>
bool LeafNode<C,E,V>::has_middle_boundary() {
    return false;
}

template<class C, class E, class V>
bool LeafNode<C,E,V>::has_left_boundary() {
    Vertex<C,E,V>* endpoint = this->get_endpoint(this->flipped);
    if (endpoint->get_id() == 9) {
        bool ost = this->get_endpoint(this->flipped)->is_exposed();
        ost = this->get_endpoint(this->flipped)->has_at_most_one_incident_edge();
        ost = false;
    }
    return endpoint->is_exposed() || !endpoint->has_at_most_one_incident_edge();
}

template<class C, class E, class V>
bool LeafNode<C,E,V>::has_right_boundary() {
    Vertex<C,E,V>* endpoint = this->get_endpoint(!this->flipped);
    return endpoint->is_exposed() || !endpoint->has_at_most_one_incident_edge();
}

template<class C, class E, class V>
bool LeafNode<C,E,V>::is_right_vertex(Vertex<C,E,V>* vertex) {
    return this->edge->is_right_vertex(vertex) != this->flipped;
}

template<class C, class E, class V>
Vertex<C,E,V>* LeafNode<C,E,V>::get_endpoint(int idx) {
    return this->edge->get_endpoint(idx);
}

template<class C, class E, class V>
void LeafNode<C,E,V>::merge() {
    E* edge = this->edge->get_data();
    V* left = this->edge->get_endpoint(this->flipped)->get_data();
    V* right = this->edge->get_endpoint(!this->flipped)->get_data();
    this->merge_leaf(edge, left, right);
    return;
}

template<class C, class E, class V>
void LeafNode<C,E,V>::split() {
    E* edge = this->edge->get_data();
    V* left = this->edge->get_endpoint(this->flipped)->get_data();
    V* right = this->edge->get_endpoint(!this->flipped)->get_data();
    this->split_leaf(edge, left, right);
    return;
}

template<class C, class E, class V>
void LeafNode<C,E,V>::print(int indent, bool flippe) {
    for (int i = 0; i < indent; i++) {
        std::cout << "    ";
    }
    this->print_data();
    std::cout << "endpoints:" <<
    this->edge->get_endpoint(this->flipped != flippe)->get_id() << "; " << 
    this->edge->get_endpoint(!this->flipped !=flippe)->get_id() << std::endl;


}
/*template<class C, class E, class V>
void LeafNode<C,E,V>::push_flip() {
    if (!this->flipped) {
        std::swap(this->edge->endpoints[0], this->edge->endpoints[1]);
    }
}*/