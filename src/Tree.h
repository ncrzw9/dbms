#include <iostream>
#include <filesystem>
#include <map>

#ifndef TANE_TREE_H
#define TANE_TREE_H

#include "Leaf.h"
#include "Index.h"

template<typename V>
class Tree {
    std::shared_ptr<Io<V>> io;
    int order{};
    int max_keys{};
    int min_keys{};
    int root{};
    int first{};

    std::shared_ptr<Node<V>> get_leaf(std::shared_ptr<Node<V>>, int);

    bool split(std::shared_ptr<Node<V>>);

    void inorder(std::vector<int> &, std::shared_ptr<Node<V>>);

    void graft(std::shared_ptr<Node<V>>);

    std::pair<std::shared_ptr<Node<V>>, int> get_anchor(int, int);

public:
	std::map<int, int> schema;

    Tree(int, const std::string &, int, std::map<int, int>);

    bool insert(int, V);

    void remove(int);

    void tree_dump();

    void leaf_dump();

    std::vector<int> get_leaves_keys();

    std::vector<int> get_tree_keys();

	V get(int);
};

template<typename V>
Tree<V>::Tree(int _o, const std::string &_f, int _b, std::map<int, int> _schema)
        : io(std::make_shared<Io<V>>(_b, _f, _o, _schema)), order(_o), max_keys(order), min_keys(order / 2), schema(_schema) {
    io->io = io;
    root = io->get_root();
	io->set_root(root);
    first = io->first;
}

template<typename V>
std::shared_ptr<Node<V>> Tree<V>::get_leaf(std::shared_ptr<Node<V>> _node, int _key) {
    if (_node->is_leaf) return _node;
    auto ubound = _node->get_ubound(_key);
    if (ubound == _node->keys.begin()) {
        auto node = io->read(_node->get_rid(0));
        return get_leaf(node, _key);
    } else if (ubound == _node->keys.end()) {
        auto node = io->read(_node->get_rid(-1));
        return get_leaf(node, _key);
    }
    auto lbound = _node->get_lbound(ubound, *std::prev(ubound));
    auto d = static_cast<int>(lbound.second) + 1;
    auto node = io->read(_node->get_rid(d));
    return get_leaf(node, _key);
}

template<typename V>
bool Tree<V>::split(std::shared_ptr<Node<V>> _node) {
    if (io->empty()) return false;
    auto right_neighbor = _node->split();
    right_neighbor.first->left = _node->rid;
    if (_node->rid == root) {
        auto new_root = std::make_shared<Index<V>>(order, io, schema);
        new_root->rid = io->consume();
        new_root->keys.push_back(right_neighbor.second);
        new_root->rids.push_back(_node->rid);
        new_root->rids.push_back(right_neighbor.first->rid);
        right_neighbor.first->parent = _node->parent = new_root->rid;
        root = new_root->rid;
        io->set_root(root);
        io->write(right_neighbor.first);
        io->write(_node);
        io->write(new_root);
        return true;
    }
    auto parent_node = io->read(_node->parent);
    auto lbound = parent_node->get_lbound(parent_node->keys.end(), _node->keys.back());
    parent_node->keys.insert(lbound.first, right_neighbor.second);
    parent_node->move_rid(static_cast<int>(lbound.second) + 1, std::move(right_neighbor.first->rid));
    io->write(right_neighbor.first);
    io->write(_node);
    if (parent_node->keys.size() <= max_keys) {
        io->write(parent_node);
        return true;
    }
    split(parent_node);
    return true;
}

template<typename V>
void Tree<V>::inorder(std::vector<int> &_container, std::shared_ptr<Node<V>> _node) {
    if (!_node) return;
    if (_node->is_leaf) {
        for (const auto &key : _node->keys) _container.push_back(key);
        return;
    }
    for (int i = 0; i < static_cast<int>(_node->keys.size() + 1); ++i) {
        auto rid = _node->get_rid(i);
        if (rid == null_rid) return;
        auto child = io->read(rid);
        inorder(_container, child);
        if (i < static_cast<int>(_node->keys.size())) _container.push_back(_node->keys[i]);
    }
}

template<typename V>
void Tree<V>::graft(std::shared_ptr<Node<V>> _node) {
    if (_node->rid == root && _node->keys.empty() && !_node->is_leaf) {
        auto root_node = io->read(root);
        auto child = io->read(root_node->get_rid(0));
        child->parent = null_rid;
        io->produce(root);
        root = child->rid;
        io->set_root(root);
        io->write(root_node);
    }
    if (_node->left == null_rid && _node->right == null_rid) return;
    auto pnode = io->read(_node->parent);
    if (_node->left == null_rid) {
        auto rnode = io->read(_node->right);
        if (min_keys < rnode->keys.size()) {
            rnode->move_one_left(pnode);
            io->write(rnode);
        } else if (min_keys == rnode->keys.size()) {
            rnode->move_right_current(pnode);
            io->produce(rnode->rid);
        }
    } else if (_node->right == null_rid) {
        auto lnode = io->read(_node->left);
        if (min_keys < lnode->keys.size()) {
            lnode->move_one_right(pnode);
            io->write(lnode);
        } else if (min_keys == lnode->keys.size()) {
            lnode->move_current_left(pnode);
            io->produce(_node->rid);
            io->write(lnode);
        }
    } else {
        auto rnode = io->read(_node->right);
        auto lnode = io->read(_node->left);
        auto lanchor = get_anchor(lnode->parent, _node->parent);
        auto ranchor = get_anchor(_node->parent, rnode->parent);
        if ((rnode->keys.size() < lnode->keys.size() && min_keys < lnode->keys.size()) ||
            (min_keys < lnode->keys.size() && min_keys < rnode->keys.size() && ranchor.second < lanchor.second)) {
            lnode->move_one_right(lanchor.first);
            io->write(lnode);
            pnode = lanchor.first;
        }
        else if ((lnode->keys.size() < rnode->keys.size() && min_keys < rnode->keys.size()) ||
            (min_keys < rnode->keys.size() && min_keys < lnode->keys.size() && lanchor.second <= ranchor.second)) {
            rnode->move_one_left(ranchor.first);
            io->write(rnode);
            pnode = ranchor.first;
        }
        else if (min_keys == lnode->keys.size() && min_keys == rnode->keys.size() && ranchor.second <= lanchor.second) {
            lnode->move_current_left(lanchor.first);
            io->produce(_node->rid);
            io->write(lnode);
            pnode = lanchor.first;
        }
        else if (min_keys == lnode->keys.size() && min_keys == rnode->keys.size() && lanchor.second < ranchor.second) {
            rnode->move_right_current(ranchor.first);
            io->produce(rnode->rid);
            pnode = ranchor.first;
        }
    }
    pnode = io->read(pnode->rid);
    if (min_keys > pnode->keys.size()) {
        graft(pnode);
    }
}

template<typename V>
bool Tree<V>::insert(int _key, V _value) {
    auto node = io->read(root);
    auto leaf = get_leaf(node, _key);
    auto exists = leaf->get_lbound(leaf->keys.end(), _key);
    if (exists.first != leaf->keys.end() && *exists.first == _key) {
        std::cout << _key << " already exists\n";
        return false;
    }
	//if (leaf->keys.size() == order && io->empty()) {
	//	std::cout << "Database is Full (insert causes split)\n";
	//	return false;
	//}
	if (leaf->keys.size() == order && io->free.size() == 1) {
		std::cout << "Database is Full (insert causes split)\n";
		return false;
	}
    leaf->keys.insert(exists.first, _key);
    leaf->move_val(static_cast<int>(exists.second), std::move(_value));
    if (leaf->keys.size() <= max_keys) {
        io->write(leaf);
        return true;
    }
    if (!split(leaf)) {
        std::cout << "Database is Full (unable to split)\n";
        return false;
    } else {
        return true;
    }
}

template<typename V>
void Tree<V>::tree_dump() {
    std::stringstream stream;
    io->dump(stream);
    int depth = 0;
    auto root_node = io->read(root);
    stream << "Dump of B+ Tree\n";
    root_node->dump(depth, stream);
    stream << '\n';
    std::cout << stream.str();
}

template<typename V>
void Tree<V>::leaf_dump() {
    std::stringstream stream;
    int depth = 0;
    stream << "Leaves of B+ Tree\n";
    for (int i = first; i != null_rid;) {
		auto leaf = io->read(i);
        leaf->dump(depth, stream);
        if (leaf->right != null_rid) stream << ',';
        stream << '\n';
		i = leaf->right;
    }
    stream << '\n';
    std::cout << stream.str();
}

template<typename V>
std::vector<int> Tree<V>::get_leaves_keys() {
    std::vector<int> leaves_keys;
    for (auto leaf = io->read(first); leaf; leaf = io->read(leaf->right))
        for (const auto &key : leaf->keys) leaves_keys.push_back(key);
    return leaves_keys;
}

template<typename V>
std::vector<int> Tree<V>::get_tree_keys() {
    std::vector<int> tree_keys;
    auto root_node = io->read(root);
    inorder(tree_keys, root_node);
    return tree_keys;
}

template<typename V>
V Tree<V>::get(int key) {
	auto node = io->read(root);
	auto leaf = get_leaf(node, key);
	auto exists = leaf->get_lbound(leaf->keys.end(), key);
	if (exists.first != leaf->keys.end() && *exists.first == key) {
		auto distance = std::distance(leaf->keys.begin(), exists.first);
		//return leaf->get(leaf->keys.end() - exists.first);
		return leaf->get(distance);
	}
	else {
		std::cout << "Key does not exist\n";
		return V();
	}
}

template<typename V>
void Tree<V>::remove(int _key) {
    auto root_node = io->read(root);
    auto leaf = get_leaf(root_node, _key);
    auto exists = leaf->get_lbound(leaf->keys.end(), _key);
    if (exists.first == leaf->keys.end() || *exists.first != _key) {
        std::cout << _key << " does not exist\n";
        return;
    }
    leaf->remove(static_cast<int>(exists.second));
    // write new node to disk
    io->write(leaf);
    if (leaf->keys.size() < min_keys) graft(leaf);
}

template<typename V>
std::pair<std::shared_ptr<Node<V>>, int>
Tree<V>::get_anchor(int left, int right) {
    auto pair = std::pair<std::shared_ptr<Node<V>>, int>();
    auto left_node = io->read(left);
    auto right_node = io->read(right);
    while (left_node->rid != right_node->rid) {
        left_node = io->read(left_node->parent);
        right_node = io->read(right_node->parent);
    }
    pair.first = left_node;
    while (left_node->parent != null_rid) {
        ++pair.second;
        left_node = io->read(left_node->parent);
    }
    return pair;
}

template<typename ForwardIt, typename T>
void iota_step(ForwardIt first, ForwardIt last, T value, T step) {
    while (first != last) {
        *first++ = static_cast<int>(value) * step;
        ++value;
    }
}

#endif