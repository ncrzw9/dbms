#include <cassert>

#ifndef TANE_INDEX_H
#define TANE_INDEX_H

#include "Node.h"

template<typename V>
struct Index : public Node<V> {

    std::vector<int> rids{};

    Index(int, std::shared_ptr<Io<V>>, std::map<int, int>);

    std::stringstream &dump(int &, std::stringstream &) override;

    std::pair<std::shared_ptr<Node<V>>, int> split() override;

    void move_val(int, V &&) override {}

    void move_rid(int, int &&) override;

    V get_val(int) override { return V(); }

    int get_rid(int) override;

    void remove(int) override;

    void move_one_right(std::shared_ptr<Node<V>>) override;

    void move_one_left(std::shared_ptr<Node<V>>) override;

    void move_current_left(std::shared_ptr<Node<V>>) override;

    void move_right_current(std::shared_ptr<Node<V>>) override;

    void write() override;

    void read(int) override;

	V get(int) override { return V();  }
};

template<typename V>
Index<V>::Index(int _o, std::shared_ptr<Io<V>> _io, std::map<int, int> _schema) : Node<V>(_o, false, _io, _schema) {
    rids.reserve(Node<V>::order + 1);
}

template<typename V>
std::stringstream &Index<V>::dump(int &_d, std::stringstream &_sstream) {
    _sstream << std::string(_d++, '\t') << '(';
    auto key = Node<V>::keys.cbegin();
    auto key_end = Node<V>::keys.cend();
    auto rid_it = rids.cbegin();
    for (; key != key_end || rid_it != rids.cend();) {
        if (rid_it != rids.cend()) {
            if (rid_it != rids.cbegin()) _sstream << ',';
            _sstream << '*';
            ++rid_it;
        }
        if (key != key_end) {
            _sstream << ',' << *key;
            ++key;
        }
    }
    _sstream << ") " << Node<V>::rid /*<< ' ' << Node<V>::parent << ' ' << Node<V>::left << ' ' << Node<V>::right*/;
    _sstream << '\n';
    for (int i = 0; i < static_cast<int>(rids.size()); ++i) {
        if (i != 0) _sstream << '\n';
        // given rid -> read node from disk
        auto child = Node<V>::io->read(rids[i]);
        child->dump(_d, _sstream);
    }
    --_d;
    return _sstream;
}

template<typename V>
std::pair<std::shared_ptr<Node<V>>, int> Index<V>::split() {
    // move right min_max up above
    auto right_neighbor = std::make_shared<Index>(Node<V>::order, Node<V>::io, schema);
    right_neighbor->rid = Node<V>::io->consume();
    if (Node<V>::right != null_rid) {
        // given rid -> read node from disk
        auto rnode = Node<V>::io->read(Node<V>::right);
        rnode->left = right_neighbor->rid;
        // update original node to disk
        Node<V>::io->write(rnode);
    }
    right_neighbor->right = Node<V>::right;
    Node<V>::right = right_neighbor->rid;
    right_neighbor->parent = Node<V>::parent;
    auto keys_center = std::next(Node<V>::keys.begin(), Node<V>::keys.size() / 2);
    std::move(keys_center, Node<V>::keys.end(), std::back_inserter(right_neighbor->keys));
    Node<V>::keys.erase(keys_center, Node<V>::keys.end());
    auto min_max = right_neighbor->keys.front();
    right_neighbor->keys.erase(right_neighbor->keys.begin());
    auto rids_center = std::next(rids.begin(), rids.size() / 2);
    for (auto rid_it = rids_center; rid_it != rids.end(); ++rid_it) {
        // given rid -> read node from disk
        auto node = Node<V>::io->read(*rid_it);
        node->parent = right_neighbor->rid;
        // update original node to disk
        Node<V>::io->write(node);
    }
    std::move(rids_center, rids.end(), std::back_inserter(right_neighbor->rids));
    rids.erase(rids_center, rids.end());
    return std::make_pair<std::shared_ptr<Node<V>>, int>(right_neighbor, std::move(min_max));
}

template<typename V>
void Index<V>::move_rid(int _pos, int &&_rid) {
    if (_pos < 0) rids.push_back(std::move(_rid));
    else rids.insert(rids.begin() + _pos, std::move(_rid));
}

template<typename V>
int Index<V>::get_rid(int _pos) {
    if (_pos == static_cast<int>(rids.size())) return null_rid;
    else if (_pos < 0) return rids.back();
    else return rids[_pos];
}

template<typename V>
void Index<V>::remove(int _pos) {
    if (_pos < 0) rids.pop_back();
    else rids.erase(rids.begin() + _pos);
}

template<typename V>
void Index<V>::move_one_right(std::shared_ptr<Node<V>> _anchor) {
//    std::cout << "Index<V>::move_one_right " << Node<V>::rid << " into " << Node<V>::right << "\n";
    auto new_pivot_key = Node<V>::keys.back();
    Node<V>::keys.pop_back();
    auto pivot = _anchor->get_ubound(new_pivot_key);
    auto rnode = Node<V>::io->read(Node<V>::right);
    rnode->keys.insert(rnode->keys.begin(), *pivot);
    auto node = Node<V>::io->read(rids.back());
    node->parent = Node<V>::right;
    rnode->move_rid(0, std::move(rids.back()));
    rids.pop_back();
    *pivot = new_pivot_key;
    Node<V>::io->write(_anchor);
    Node<V>::io->write(rnode);
    Node<V>::io->write(node);
}

template<typename V>
void Index<V>::move_one_left(std::shared_ptr<Node<V>> _anchor) {
//    std::cout << "Index<V>::move_one_left " << Node<V>::rid << " into " << Node<V>::left << "\n";
    auto lnode = Node<V>::io->read(Node<V>::left);
    auto old_left_last_key = lnode->keys.back();
    auto pivot = _anchor->get_ubound(old_left_last_key);
    lnode->keys.push_back(*pivot);
    auto node = Node<V>::io->read(rids.front());
    node->parent = Node<V>::left;
    *pivot = Node<V>::keys.front();
    Node<V>::keys.erase(Node<V>::keys.begin());
    lnode->move_rid(-1, std::move(rids.front()));
    rids.erase(rids.begin());
    Node<V>::io->write(_anchor);
    Node<V>::io->write(lnode);
    Node<V>::io->write(node);
}

template<typename V>
void Index<V>::move_current_left(std::shared_ptr<Node<V>> _anchor) {
//    std::cout << "Index<V>::move_current_left " << Node<V>::right << " into " << Node<V>::rid << "\n";
    auto old_left_last_key = Node<V>::keys.back();
    auto pivot = _anchor->get_ubound(old_left_last_key);
    Node<V>::keys.push_back(*pivot);
    auto rnode = Node<V>::io->read(Node<V>::right);
    auto max_ptrs_size = static_cast<int>(rnode->keys.size() + 1 + 1);
    for (int i = 0; i < max_ptrs_size; ++i) {
        auto rid_it = rnode->get_rid(i);
        if (rid_it != null_rid) {
            auto node = Node<V>::io->read(rid_it);
            node->parent = rnode->left;
            rids.push_back(rid_it);
            Node<V>::io->write(node);
        }
    }
    std::move(rnode->keys.begin(), rnode->keys.end(), std::back_inserter(Node<V>::keys));
    rnode->keys.clear();
    // update neighbors
    if (rnode->right != null_rid) {
        // given rid -> read node from disk
        auto rrnode = Node<V>::io->read(rnode->right);
        rrnode->left = rnode->left;
        Node<V>::io->write(rrnode);
    }
    Node<V>::right = rnode->right;
    if (pivot == _anchor->keys.end()) {
        _anchor->remove(-1);
//        _anchor->keys.pop_back();
        _anchor->keys.pop_back();
    } else {
        _anchor->remove(static_cast<int>(pivot - _anchor->keys.begin() + 1));
        _anchor->keys.erase(pivot);
    }
    Node<V>::io->write(_anchor);
    Node<V>::io->write(rnode);
}

template<typename V>
void Index<V>::move_right_current(std::shared_ptr<Node<V>> _anchor) {
//    std::cout << "Index<V>::move_right_current " << Node<V>::rid << " into " << Node<V>::left << "\n";
    auto lnode = Node<V>::io->read(Node<V>::left);
    auto old_left_last_key = lnode->keys.back();
    auto pivot = _anchor->get_ubound(old_left_last_key);
    lnode->keys.push_back(*pivot);
    // ensure 640 is moved into left keys
    // left keys aren't being written correctly to file
    std::move(Node<V>::keys.begin(), Node<V>::keys.end(), std::back_inserter(lnode->keys));
    Node<V>::keys.clear();
    for (auto &rid_it : rids) {
        auto node = Node<V>::io->read(rid_it);
        node->parent = Node<V>::left;
        lnode->move_rid(-1, std::move(rid_it));
        Node<V>::io->write(node);
    }
    rids.clear();
    if (Node<V>::right != null_rid) {
        auto rnode = Node<V>::io->read(Node<V>::right);
        rnode->left = Node<V>::left;
        Node<V>::io->write(rnode);
    }
    lnode->right = Node<V>::right;
    if (pivot == _anchor->keys.end()) {
        _anchor->remove(-1);
//        _anchor->keys.pop_back();
        _anchor->keys.pop_back();
    } else {
        _anchor->remove(static_cast<int>((pivot - _anchor->keys.begin() + 1)));
        _anchor->keys.erase(pivot);
    }
    Node<V>::io->write(_anchor);
    Node<V>::io->write(lnode);
}

template<typename V>
void Index<V>::write() {
//    std::cout << "Index write\n";
    std::stringstream s;
    auto node_bytes = Node<V>::io->nhead_size + Node<V>::io->nunit_size;
    std::vector<char> fill(node_bytes, 0);
    s.write(reinterpret_cast<char*>(&fill[0]), node_bytes);
    s.seekp(0);
    auto offset = Node<V>::write_node(s);
    s.seekp(Node<V>::io->nhead_size);
    std::vector<int> buf_rids((Node<V>::order + 2) + 1);
    buf_rids[0] = rids.size();
    for (int i = 0; i < buf_rids[0]; ++i) buf_rids[i + 1] = rids[i];
    s.write(reinterpret_cast<char*>(&buf_rids[0]), buf_rids.size() * sizeof(buf_rids[0]));
    Node<V>::io->stream.open(Node<V>::io->fname, std::ios::in | std::ios::out | std::ios::binary);
    Node<V>::io->stream.seekp(offset);
    Node<V>::io->stream << s.rdbuf();
    Node<V>::io->stream.close();
}

template<typename V>
void Index<V>::read(int _o) {
    auto offset = Node<V>::read_node(_o);
    std::vector<char> s(Node<V>::io->nunit_size);
    int rids_size{};
    Node<V>::io->stream.open(Node<V>::io->fname, std::ios::in | std::ios::binary);
    Node<V>::io->stream.seekg(offset);
    Node<V>::io->stream.read(reinterpret_cast<char*>(&s[0]), Node<V>::io->nunit_size);
    Node<V>::io->stream.close();
    std::stringstream ls;
	std::copy(s.begin(), s.end(), std::ostream_iterator<char>(ls));
	ls.read(reinterpret_cast<char*>(&rids_size), sizeof(int));
    for (int i = 0; i < rids_size; ++i) {
        int rid{};
        ls.read(reinterpret_cast<char*>(&rid), sizeof(int));
        rids.push_back(rid);
    }
}


#endif