#include <iostream>
#include <queue>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <filesystem>

#ifndef TANE_IO_H
#define TANE_IO_H

struct List {
    const int PAGE_MODULUS = 10;
    std::vector<int> labels{};
    std::priority_queue<int, std::vector<int>, std::greater<>> free{};
    std::vector<char> tenure{};

    explicit List(int);

    std::stringstream &dump(std::stringstream &) const;

    bool empty() const;
};

List::List(int _c) : labels(_c, 0), tenure(_c, 0) {
    for (int i = 0; i < labels.size(); ++i) {
        labels[i] = i % PAGE_MODULUS;
        free.push(i);
    }
}

std::stringstream &List::dump(std::stringstream &_s) const {
    std::copy(labels.cbegin(), labels.cend(), std::ostream_iterator<int>(_s, ""));
    _s << '\n';
    std::copy(tenure.cbegin(), tenure.cend(), std::ostream_iterator<bool>(_s, ""));
    _s << '\n';
    return _s;
}

bool List::empty() const {
    return free.empty();
}

template<typename V>
struct Io : public List {
    static constexpr int ROOT_SIZE = sizeof(int);
    int free_size{};
    static constexpr int BYTE_SIZE = sizeof(bool);
    int nhead_size{};
	static constexpr int V20_SIZE = sizeof(V20);
	static constexpr int TUPLE_SIZE = 5 * V20_SIZE;
    int nunit_size{};

    std::string fname{};
    std::fstream stream;
    int root{};
    int first{};
    int order{};
    std::shared_ptr<Io> io;
    std::unordered_map<int, std::shared_ptr<Node<V>>> housing{};
	std::map<int, int> schema;

    Io(int, std::string, int, std::map<int, int>);

    std::shared_ptr<Node<V>> read(int);

    void write(std::shared_ptr<Node<V>>);

    void set_root(int);

    int get_root();

    int consume();

    void produce(int);

//    void generate(int);
};

template<typename V>
Io<V>::Io(int _c, std::string _n, int _o, std::map<int, int> _schema)
        : List(_c), free_size(_c),
          fname(std::move(_n)),
          order(_o),
          nhead_size(BYTE_SIZE + (ROOT_SIZE * 3) + ROOT_SIZE + (ROOT_SIZE * (_o + 1))),
          nunit_size(ROOT_SIZE + (TUPLE_SIZE * (_o + 2))),
          root(0), schema(_schema) {
    if (!std::filesystem::exists(fname)) {
//        std::cout << "CREATE NEW TREE\n";
        root = null_rid;
        stream.open(fname, std::ios::out | std::ios::binary);
        stream.write(reinterpret_cast<char *>(&root), ROOT_SIZE);
        stream.write(reinterpret_cast<char *>(&first), ROOT_SIZE);
        stream.write(reinterpret_cast<char *>(&free_size), ROOT_SIZE);
        stream.write(reinterpret_cast<char *>(&tenure[0]), free_size);
        std::vector<char> node_head(nhead_size, 0);
        std::vector<char> node_unit(nunit_size, 0);
        for (int i = 0; i < _c; ++i) {
            stream.write(reinterpret_cast<char *>(&node_head[0]), nhead_size);
            stream.write(reinterpret_cast<char *>(&node_unit[0]), nunit_size);
        }
        stream.close();
    } else {
//        std::cout << "USE EXISTING TREE\n";
        stream.open(fname, std::ios::in | std::ios::binary);
        stream.read(reinterpret_cast<char*>(&root), ROOT_SIZE);
        stream.read(reinterpret_cast<char*>(&first), ROOT_SIZE);
        stream.read(reinterpret_cast<char *>(&free_size), ROOT_SIZE);
        labels = std::vector<int>(free_size);
        tenure = std::vector<char>(free_size);
        stream.read(reinterpret_cast<char *>(&tenure[0]), free_size);
        stream.close();

        free = std::priority_queue<int, std::vector<int>, std::greater<>>();
        for (int i = 0; i < tenure.size(); ++i) if (tenure[i] == 0) produce(i);
        for (int i = 0; i < labels.size(); ++i) {
            labels[i] = i % PAGE_MODULUS;
        }

        std::stringstream sstream;
        std::copy(labels.cbegin(), labels.cend(), std::ostream_iterator<int>(sstream, "\t"));
        sstream << '\n';
        std::copy(tenure.cbegin(), tenure.cend(), std::ostream_iterator<bool>(sstream, "\t"));
        sstream << '\n';
    }
}

template<typename V>
std::shared_ptr<Node<V>> Io<V>::read(int _r) {
    auto tree_bytes = io->ROOT_SIZE + io->ROOT_SIZE + io->ROOT_SIZE + io->free_size;
    auto node_bytes = io->nhead_size + io->nunit_size;
    auto offset = tree_bytes + (node_bytes * _r);
    bool type{};
    stream.open(fname, std::ios::in | std::ios::binary);
    stream.seekg(offset);
    stream.read(reinterpret_cast<char*>(&type), sizeof(bool));
    stream.close();

    std::shared_ptr<Node<V>> node;
    if (type == true) {
        node = std::make_shared<Leaf<V>>(order, io, schema);
    } else {
        node = std::make_shared<Index<V>>(order, io, schema);
    }
    node->read(offset + 1);
    node->rid = _r;

    return node;
//    return housing.find(_o)->second;
}

template<typename V>
void Io<V>::write(std::shared_ptr<Node<V>> _n) {
    _n->write();
//    auto node = housing.find(_n->rid);
//    if (node == housing.end()) housing[_n->rid] = _n;
//    else node->second = _n;
}

template<typename V>
void Io<V>::set_root(int _r) {
//    std::cout << "set root\n";
    root = _r;
    stream.open(fname, std::ios::in | std::ios::out | std::ios::binary);
    stream.seekp(std::ios::beg);
    stream.write(reinterpret_cast<char *>(&root), ROOT_SIZE);
    stream.write(reinterpret_cast<char *>(&first), ROOT_SIZE);
    stream.close();
}

template<typename V>
int Io<V>::get_root() {
//    std::cout << "ROOT=" << root << '\n';
    if (root == null_rid) {
        root = io->consume();
        auto root_node = std::make_shared<Leaf<V>>(order, io, schema);
        root_node->rid = root;
        io->write(root_node);
        return root_node->rid;
    } else {
        auto root_node = std::make_shared<Leaf<V>>(order, io, schema);
        root_node->rid = root;
        housing[root] = root_node;
        return root_node->rid;
    }
    return root;
}

template<typename V>
int Io<V>::consume() {
    int block = List::free.top();
    List::tenure[block] = 1;
    List::free.pop();
    stream.open(fname, std::ios::in | std::ios::out | std::ios::binary);
    stream.seekg(ROOT_SIZE, std::ios::beg);
    stream.write(reinterpret_cast<char *>(&first), ROOT_SIZE);
    auto tenure_size = tenure.size();
    stream.write(reinterpret_cast<char *>(&tenure_size), ROOT_SIZE);
    stream.write(reinterpret_cast<char *>(&tenure[0]), free_size);
    stream.close();
    return block;
}

template<typename V>
void Io<V>::produce(int _f) {
    List::tenure[_f] = 0;
    List::free.push(_f);
    stream.open(fname, std::ios::in | std::ios::out | std::ios::binary);
    stream.seekg(ROOT_SIZE, std::ios::beg);
    stream.write(reinterpret_cast<char *>(&first), ROOT_SIZE);
    auto tenure_size = tenure.size();
    stream.write(reinterpret_cast<char *>(&tenure_size), ROOT_SIZE);
    stream.write(reinterpret_cast<char *>(&tenure[0]), free_size);
    stream.close();
}

#endif