#include "two_edge_connected.h"
#include <tuple>

void TwoEdgeConnectivity::cover(int u, int v, int level) {
    TwoEdgeCluster *root = this->top_tree->expose(u, v);
    root->cover(level);
    this->top_tree->deexpose(u, v);
}

void TwoEdgeConnectivity::uncover(int u, int v, int level) {
    TwoEdgeCluster *root = this->top_tree->expose(u, v);
    root->uncover(level);
    this->top_tree->deexpose(u, v);
}

NewEdge* TwoEdgeConnectivity::insert(int u, int v) {
    //Try to link u,v in tree
    if (u == v) {
        return nullptr;
    }
    for (int i = 0; i < TwoEdgeCluster::get_l_max(); i++) {
        assert(this->find_size(u,u,i) <= this->vertex_labels.size() / (1 << i));
        assert(this->find_size(v,v,i) <= this->vertex_labels.size() / (1 << i));
    }

    NewEdge edge_data = NewEdge::tree_edge(u, v, -1, nullptr);
    TwoEdgeCluster* result = this->top_tree->link_leaf(u, v, edge_data); //TODO level = lmax?
    
    if (result) {

        //If successfull, try to assign vertex endpoints to new leaf
        if (!vertex_labels[u]->leaf_node) {
            result->assign_vertex(u, vertex_labels[u]);
            vertex_labels[u]->leaf_node = result;
        }
        if (!vertex_labels[v]->leaf_node) {
            result->assign_vertex(v, vertex_labels[v]);
            vertex_labels[v]->leaf_node = result;
        }
        result->full_splay();
        result->recompute_root_path();
        check_levels(u,v);
        return NewEdge::new_tree_edge(u, v, -1, result);

    }
    NewEdge* edge = NewEdge::new_non_tree_edge(u, v, 0);
    

    this->add_label(u, edge);
    this->add_label(v, edge);
    this->cover(u, v, 0);
    return edge;
}

NewEdge* TwoEdgeConnectivity::insert(int u, int v, int level) {
    TwoEdgeCluster* result = this->top_tree->link_leaf(u, v, NewEdge::tree_edge(u, v, -1, nullptr)); //TODO level = lmax?
    if (result) {
        return NewEdge::new_tree_edge(u, v, -1, result);
    }
    NewEdge* edge = NewEdge::new_non_tree_edge(u, v, level);
    this->add_label(u, edge);
    this->add_label(v, edge);
    this->cover(u, v, level);
    return edge;
}

void TwoEdgeConnectivity::add_label(int vertex, NewEdge* edge) {
    assert(edge->edge_type == NonTreeEdge);
    VertexLabel* vertex_label = this->vertex_labels[vertex];    
    int index = vertex_label->labels[edge->level].size();

    if (edge->endpoints[0] == vertex) {
        edge->extra_data.index[0] = index;
    } else {
        edge->extra_data.index[1] = index;
    }
    vertex_label->labels[edge->level].push_back(edge);
    
    assert(vertex_label->leaf_node);
    vertex_label->leaf_node->full_splay(); //depth <= 5
    vertex_label->leaf_node->recompute_root_path(); //takes O(depth) = O(1) time
}

void TwoEdgeConnectivity::remove_labels(NewEdge* edge) {
    assert(edge->edge_type == NonTreeEdge);
    int level = edge->level;
    for (int i = 0; i < 2; i++) {
        int ep = edge->endpoints[i];
        int ep_idx = edge->extra_data.index[i];

        VertexLabel* ep_label = this->vertex_labels[ep];
        
        NewEdge* last_label = ep_label->labels[level].back();
        assert(last_label->edge_type == NonTreeEdge);
        int ep_is_right_new = last_label->endpoints[1] == ep;

        ep_label->labels[level][ep_idx] = last_label;
        ep_label->labels[level][ep_idx]->extra_data.index[ep_is_right_new] = ep_idx;
        ep_label->labels[level].pop_back();

        if (ep_label->leaf_node) {
            ep_label->leaf_node->full_splay();
            ep_label->leaf_node->recompute_root_path();
        }
    }
}   

void TwoEdgeConnectivity::reassign_vertices(TwoEdgeCluster* leaf_node) {
    leaf_node->full_splay();
    assert(!leaf_node->is_flipped());
    VertexLabel* old_labels[2] = {leaf_node->vertex[0],leaf_node->vertex[1]};
    leaf_node->vertex[0] = nullptr;
    leaf_node->vertex[1] = nullptr;
    leaf_node->recompute_root_path();
    for (int i = 0; i < 2; i++) {
        if (old_labels[i]) {
            //find new edge
            int id = leaf_node->get_endpoint_id(i);
            TwoEdgeCluster* replacement = this->top_tree->get_adjacent_leaf_node(id);
            if (replacement == leaf_node) {
                replacement = this->top_tree->get_adjacent_leaf_node(id, 1);
                if (!replacement) {
                    old_labels[i]->leaf_node = nullptr;
                    continue;
                }
            
            } 
            //std::cout << "found edge: (" << replacement->get_endpoint_id(0)  << "," << replacement->get_endpoint_id(1) << ")" << std::endl;
            replacement->full_splay();

            replacement->assign_vertex(id, old_labels[i]);
            old_labels[i]->leaf_node = replacement;

            replacement->recompute_root_path();
        }
    }
}

void TwoEdgeConnectivity::check_levels(int u, int v) {
    // for (int i = 0; i < TwoEdgeCluster::get_l_max(); i++) {
    //     int size_u = this->find_size(u,u,i);
    //     int size_v = this->find_size(v,v,i);
    //     int size_limit =  this->vertex_labels.size() / (1 << i);
    //     assert(size_u <= size_limit);
    //     assert(size_v <= size_limit);
    // }
}

void TwoEdgeConnectivity::remove(NewEdge* edge) {
    int u = edge->endpoints[0];
    int v = edge->endpoints[1];
    if (!this->top_tree->connected(u,v)) {
        this->vertex_labels[u]->print();
        this->vertex_labels[v]->print();
        std::cout << std::endl << std::endl << std::endl;

    }
    assert(this->top_tree->connected(u,v));

    for (int i = 0; i < TwoEdgeCluster::get_l_max(); i++) {
        int s = this->find_size(u,v,i);
        if (!(this->find_size(u,v,i) <= this->vertex_labels.size() / (1 << i))) {
            // this->expose(u,v)->print(0,false);
            // this->deexpose(u,v);
        }
        assert(this->find_size(u,u,i) <= this->vertex_labels.size() / (1 << i));
        assert(this->find_size(v,v,i) <= this->vertex_labels.size() / (1 << i));
    }

    int alpha = edge->level;
    if (alpha != -1) {
        assert(this->find_size(u,v,edge->level) <= this->vertex_labels.size() / (1 << edge->level));

    }

    int test = false;
    if (edge->edge_type == TreeEdge) {
        test = true;
        int cover_level = this->cover_level(u, v);
        alpha = cover_level;
        if (cover_level == -1) {
            //std::cout << "delete " << u << " " << v << std::endl;
            reassign_vertices(edge->extra_data.leaf_node);
            top_tree->cut_leaf(edge->extra_data.leaf_node);
            return;
        }
        //std::cout << "swap " << u << " " << v << std::endl;
        edge = this->swap(edge);
    } 
    assert(edge->edge_type == NonTreeEdge);
    this->remove_labels(edge);

    this->uncover(u, v, alpha);

    if (u == 9 && v == 10 && alpha == 1) {
        auto root = this->expose(v);
        for (int i = 0; i < root->size.size(); i++ ) {
            assert(root->size[i] <= this->vertex_labels.size() / (1 << i));
        }
        this->deexpose(v);
    }

    for (int i = alpha; i >= 0; i--) {
        this->recover(v, u, i);
    }

    for (int i = 0; i < TwoEdgeCluster::get_l_max(); i++) {
        int a = this->find_size(u,u,i);
        int b = this->find_size(v,v,i);
        if (!(b <= this->vertex_labels.size() / (1 << i))) {
            this->expose(v)->print(0,false);
        }
        assert(a <= this->vertex_labels.size() / (1 << i));
        assert(b <= this->vertex_labels.size() / (1 << i));
    }
    assert(this->top_tree->connected(u,v));
}

int TwoEdgeConnectivity::cover_level(int u, int v) {
    TwoEdgeCluster* root = this->top_tree->expose(u, v);
    int cover_level = root->get_cover_level();
    this->top_tree->deexpose(u, v);
    return cover_level;    
}

NewEdge* TwoEdgeConnectivity::swap(NewEdge* tree_edge) {
    assert(tree_edge->edge_type == TreeEdge);

    int u = tree_edge->endpoints[0];
    int v = tree_edge->endpoints[1];
    
    assert(this->top_tree->connected(u,v));
    int cover_level = this->cover_level(u, v);

    assert(cover_level > -1);

    reassign_vertices(tree_edge->extra_data.leaf_node);
    auto root = this->top_tree->cut_leaf(tree_edge->extra_data.leaf_node);
    assert(!this->top_tree->connected(u,v));

    NewEdge* non_tree_edge = find_replacement(u, v, cover_level);
    int x = non_tree_edge->endpoints[0];
    int y = non_tree_edge->endpoints[1];
    this->remove_labels(non_tree_edge);
    
    NewEdge e = NewEdge::tree_edge(x,y,-1,nullptr);
    TwoEdgeCluster* new_leaf = this->top_tree->link_leaf(x, y, e);
    assert(new_leaf != nullptr);
    if (new_leaf) {
        //If successfull, try to assign vertex endpoints to new leaf
        new_leaf->full_splay();
        if (!vertex_labels[x]->leaf_node) {
            //std::cout << "taking " << x << std::endl;
            new_leaf->assign_vertex(x, vertex_labels[x]);
            vertex_labels[x]->leaf_node = new_leaf;
        }
        if (!vertex_labels[y]->leaf_node) {
            //std::cout << "taking " << y << std::endl;
            new_leaf->assign_vertex(y, vertex_labels[y]);
            vertex_labels[y]->leaf_node = new_leaf;
        }
        new_leaf->recompute_root_path();
    }
    //std::cout << "replacing (" << u << "," << v << ") with (" << x << "," << y << ")" << std::endl; 
    // TODO comment
    tree_edge->endpoints[0] = -1;
    tree_edge->endpoints[1] = -1;
    NewEdge::swap(non_tree_edge,NewEdge::new_tree_edge(x,y,-1,new_leaf));

    assert(this->top_tree->connected(u,v));
    NewEdge* edge = NewEdge::new_non_tree_edge(u, v, cover_level);
    this->add_label(u, edge);
    this->add_label(v, edge);
    this->cover(u, v, cover_level);
    return edge;
}

int TwoEdgeConnectivity::find_size(int u, int v, int cover_level) {
    TwoEdgeCluster* root = this->top_tree->expose(u);
    if (u != v) {
        root = this->top_tree->expose(v);
    }
    int size;
    if (cover_level >= TwoEdgeCluster::get_l_max()) {
        size = INT32_MAX;
    } else if (root) {
        size = root->get_size(cover_level);
        // if (u == 2 && v == 1 && cover_level == 0) {
        //     root->print(0,false);
        // }
        //root->recompute_tree();
        // if (u == 2 && v == 1 && cover_level == 0) {
        //     root->print(0,false);
        // }
        //assert(root->get_size(cover_level) == size);
    } else {
        size = 1;
    }
    // if (!(size <= this->vertex_labels.size() / (1<<cover_level))) {
    //     std::cerr << "size: " << size << std::endl;
    //     std::cerr << "size limit: " << this->vertex_labels.size() / (1<<cover_level) << std::endl;
        
    //     root->print(0,false);
    // }
    //assert(size <= this->vertex_labels.size() / (1<<cover_level));
    this->top_tree->deexpose(u);
    if (u != v) {
        this->top_tree->deexpose(v);
    }
    return size;
}

NewEdge* TwoEdgeConnectivity::find_replacement(int u, int v, int cover_level) {
    int size_u = this->find_size(u, u, cover_level);
    int size_v = this->find_size(v, v, cover_level);
    if (!(size_u+size_v <= this->vertex_labels.size() / (1 << cover_level))) {
        // this->expose(u)->print(0,false);
        // this->expose(v)->print(0,false);
    }
    assert(size_u+size_v <= this->vertex_labels.size() / (1 << cover_level));
    if (size_u <= size_v) {
        //assert(size_u <= this->vertex_labels.size() / (1 << (cover_level+1)));
        //assert(size_v <= this->vertex_labels.size() / (1 << (cover_level)));
        return this->recover_phase(u, u, cover_level, size_u);
    } else {
        //assert(size_v <= this->vertex_labels.size() / (1 << (cover_level+1)));
        //assert(size_u <= this->vertex_labels.size() / (1 << (cover_level)));
        return this->recover_phase(v, v, cover_level, size_v);
    }
}

NewEdge* TwoEdgeConnectivity::find_first_label(int u, int v, int cover_level) {
    TwoEdgeCluster* root = this->top_tree->expose(u);
    NewEdge* res;
    std::tuple<TwoEdgeCluster*,VertexLabel*> result;
    VertexLabel* label;
    TwoEdgeCluster* label_leaf;

    if (u != v) {
        root = this->top_tree->expose(v);
    }
    if (!root) { // if no root, no tree edges. Look for label in vertex only.
        assert(u == v);

        label = vertex_labels[u];
        if (label->labels[cover_level].size() > 0) {
            res = label->labels[cover_level].back();
        } else {
            res = nullptr;
        }
        goto out;
    }
    root->push_flip();
    result = root->find_first_label(u,v,cover_level);
    label_leaf = std::get<0>(result);
    label = std::get<1>(result);
    if (!label_leaf) {
        res = nullptr;
        goto out;
    }
    label_leaf->full_splay();
    if (!label) {
        res = nullptr;
        goto out;
    }
    res = label->labels[cover_level].back();

    out:
    this->top_tree->deexpose(u);
    if (u != v) {
        this->top_tree->deexpose(v);
    }
    return res;
}

void TwoEdgeConnectivity::recover(int u, int v, int cover_level) {
    int this_size = this->find_size(u,v,cover_level);
    assert(this_size <= this->vertex_labels.size() / (1 << (cover_level)) );
    int size = this->find_size(u,v,cover_level) / 2;
    this->recover_phase(u, v, cover_level, size);
    this->recover_phase(v, u, cover_level, size);

}

NewEdge* TwoEdgeConnectivity::recover_phase(int u, int v, int cover_level, int size) {
    NewEdge* label = this->find_first_label(u, v, cover_level);
    int i = 0;
    while (label) {
        int q = label->endpoints[0];
        int r = label->endpoints[1];
        if (!this->top_tree->connected(q, r)) {
            return label;
        }
        // //std::cout << "found (" << q << ", " << r << ")" << std::endl;
        assert(size <= this->vertex_labels.size() / (1 << (cover_level+1)));
        if (this->find_size(q, r, cover_level + 1) <= size) {
            this->remove_labels(label);
            label->level = cover_level + 1;
            this->add_label(q, label);
            this->add_label(r, label);
            this->cover(q, r, cover_level + 1);
        } else {
            this->cover(q, r, cover_level); 
            assert(u != v);
            return nullptr;
        }
        label = this->find_first_label(u, v, cover_level);
    }
    return nullptr;
}

bool TwoEdgeConnectivity::two_edge_connected(int u, int v) {
    if (u == v) {
        return true; //TODO true?
    }
    return (this->top_tree->connected(u,v) && (this->cover_level(u,v) >= 0));
}

NewEdge* TwoEdgeConnectivity::find_bridge(int u, int v) {
    TwoEdgeCluster* root = this->top_tree->expose(u,v);
    NewEdge* bridge;
    if (root->cover_level == -1) {
        bridge = root->min_path_edge;
    } else {
        bridge = nullptr;
    }
    this->top_tree->deexpose(u,v);
    return bridge;
}