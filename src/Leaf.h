#ifndef TANE_LEAF_H
#define TANE_LEAF_H

#include "Node.h"

template<typename V>
struct Leaf : public Node<V> {

    std::vector<V> vals{};

    Leaf(int, std::shared_ptr<Io<V>>, std::map<int, int>);

    std::stringstream &dump(int &, std::stringstream &) override;

    std::pair<std::shared_ptr<Node<V>>, int> split() override;

    void move_val(int, V &&) override;

    void move_rid(int, int &&) override {}

    V get_val(int) override;

    int get_rid(int) override { return int(); }

    void remove(int) override;

    void move_one_right(std::shared_ptr<Node<V>>) override;

    void move_one_left(std::shared_ptr<Node<V>>) override;

    void move_current_left(std::shared_ptr<Node<V>>) override;

    void move_right_current(std::shared_ptr<Node<V>>) override;

    void write() override;

    void read(int) override;

	V get(int) override;
};

template<typename V>
Leaf<V>::Leaf(int _o, std::shared_ptr<Io<V>> _io, std::map<int, int> _schema) : Node<V>(_o, true, _io, _schema) {
    vals.reserve(Node<V>::order + 1);
}

template<typename V>
std::stringstream &Leaf<V>::dump(int &_d, std::stringstream &_sstream) {
    _sstream << std::string(_d++, '\t') << '[';
    for (auto key = Node<V>::keys.cbegin(); key != Node<V>::keys.cend(); ++key) {
        if (key != Node<V>::keys.cbegin()) _sstream << ',';
        _sstream << *key;
    }
    _sstream << "] " << Node<V>::rid /*<< ' ' << Node<V>::parent << ' ' << Node<V>::left << ' ' << Node<V>::right*/;
    --_d;
    return _sstream;
}

template<typename V>
std::pair<std::shared_ptr<Node<V>>, int> Leaf<V>::split() {
    auto right_neighbor = std::make_shared<Leaf>(Node<V>::order, Node<V>::io, schema);
    right_neighbor->rid = Node<V>::io->consume();
    if (Node<V>::right != null_rid) {
        auto rnode = Node<V>::io->read(Node<V>::right);
        rnode->left = right_neighbor->rid;
        Node<V>::io->write(rnode);
    }
    right_neighbor->right = Node<V>::right;
    Node<V>::right = right_neighbor->rid;
    right_neighbor->parent = Node<V>::parent;
    // move keys
    auto keys_center = std::next(Node<V>::keys.begin(), Node<V>::keys.size() / 2);
    std::move(keys_center, Node<V>::keys.end(), std::back_inserter(right_neighbor->keys));
    Node<V>::keys.erase(keys_center, Node<V>::keys.end());
    auto min_max = right_neighbor->keys.front();
    // move vals
    auto vals_center = std::next(vals.begin(), vals.size() / 2);
    std::move(vals_center, vals.end(), std::back_inserter(right_neighbor->vals));
    vals.erase(vals_center, vals.end());
    return std::make_pair<std::shared_ptr<Node<V>>, int>(right_neighbor, std::move(min_max));
}

template<typename V>
void Leaf<V>::move_val(int _pos, V &&_val) {
//    if (vals.size() > Node<V>::order)
//        std::cout << Node<V>::rid << " too big " << vals.size() << '\n';
    if (_pos < 0) vals.push_back(std::move(_val));
    else vals.insert(vals.begin() + _pos, std::move(_val));
}

template<typename V>
V Leaf<V>::get_val(int _pos) {
    return vals[_pos];
}

template<typename V>
void Leaf<V>::remove(int _pos) {
    if (_pos < 0) {
        Leaf<V>::keys.pop_back();
        vals.pop_back();
    } else {
        Leaf<V>::keys.erase(Leaf<V>::keys.begin() + _pos);
        vals.erase(vals.begin() + _pos);
    }
}

template<typename V>
void Leaf<V>::move_one_right(std::shared_ptr<Node<V>> _anchor) {
//    std::cout << "Leaf<V>::move_one_right " << Node<V>::rid << " into " << Node<V>::right << "\n";
    auto new_right_first_key = Node<V>::keys.back();
    Node<V>::keys.pop_back();
    auto rnode = Node<V>::io->read(Node<V>::right);
    rnode->keys.insert(rnode->keys.begin(), new_right_first_key);
    rnode->move_val(0, std::move(vals.back()));
    vals.pop_back();
    auto pivot = _anchor->get_ubound(new_right_first_key);
    *pivot = new_right_first_key;
    Node<V>::io->write(_anchor);
    Node<V>::io->write(rnode);
}

template<typename V>
void Leaf<V>::move_one_left(std::shared_ptr<Node<V>> _anchor) {
//    std::cout << "Leaf<V>::move_one_left " << Node<V>::rid << " into " << Node<V>::left << "\n";
    auto lnode = Node<V>::io->read(Node<V>::left);
    auto old_left_last_key = lnode->keys.back();
    lnode->keys.push_back(std::move(Node<V>::keys.front()));
    Node<V>::keys.erase(Node<V>::keys.begin());
    lnode->move_val(-1, std::move(vals.front()));
    vals.erase(vals.begin());
    auto pivot = _anchor->get_ubound(old_left_last_key);
    *pivot = Node<V>::keys.front();
    Node<V>::io->write(_anchor);
    Node<V>::io->write(lnode);
}

template<typename V>
void Leaf<V>::move_current_left(std::shared_ptr<Node<V>> _anchor) {
//    std::cout << "Leaf<V>::move_current_left " << Node<V>::right << " into " << Node<V>::rid << "\n";
    auto pivot = _anchor->get_ubound(Node<V>::keys.back());
    auto distance = std::distance(_anchor->keys.begin(), pivot);
    auto rnode = Node<V>::io->read(Node<V>::right);
    for (int i = 0; i < static_cast<int>(rnode->keys.size()); ++i) vals.push_back(std::move(rnode->get_val(i)));
    std::move(rnode->keys.begin(), rnode->keys.end(), std::back_inserter(Node<V>::keys));
    if (rnode->right != null_rid) {
        auto rrnode = Node<V>::io->read(rnode->right);
        rrnode->left = rnode->left;
        Node<V>::io->write(rrnode);
    }
    Node<V>::right = rnode->right;
    if (pivot == _anchor->keys.end()) {
        _anchor->keys.pop_back();
        _anchor->remove(-1);
    } else {
        _anchor->keys.erase(pivot);
        _anchor->remove(static_cast<int>(distance + 1));
    }
    Node<V>::io->write(_anchor);
    Node<V>::io->write(rnode);
}

template<typename V>
void Leaf<V>::move_right_current(std::shared_ptr<Node<V>> _anchor) {
//    std::cout << "Leaf<V>::move_right_current " << Node<V>::rid << " into " << Node<V>::left << "\n";
    auto lnode = Node<V>::io->read(Node<V>::left);
    auto old_left_last_key = lnode->keys.back();
    auto pivot = _anchor->get_ubound(old_left_last_key);
    auto distance = std::distance(_anchor->keys.begin(), pivot);
    std::move(Node<V>::keys.begin(), Node<V>::keys.end(), std::back_inserter(lnode->keys));
    for (auto &val : vals) lnode->move_val(-1, std::move(val));
    if (Node<V>::right != null_rid) {
        auto rnode = Node<V>::io->read(Node<V>::right);
        rnode->left = Node<V>::left;
        Node<V>::io->write(rnode);
    }
    lnode->right = Node<V>::right;
    _anchor->keys.erase(pivot);
    _anchor->remove(static_cast<int>(distance + 1));
    Node<V>::io->write(_anchor);
    Node<V>::io->write(lnode);
}

template<std::size_t I = 0, typename... Tp, typename Buffer, typename Schema>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
writeElement(std::tuple<Tp...>& t, Buffer& buf, Schema& sch) { }

template<std::size_t I = 0, typename... Tp, typename Buffer, typename Schema>
inline typename std::enable_if < I < sizeof...(Tp), void>::type
	writeElement(std::tuple<Tp...> & t, Buffer & buf, Schema & sch) {
	buf.write(reinterpret_cast<char*>(&std::get<I>(t)), sizeof(std::get<I>(t)));
	writeElement<I + 1, Tp...>(t, buf, sch);
}

template<typename V>
void Leaf<V>::write() {
	//auto offset = Node<V>::read_node(_o);
	//int vals_size{};
	//Node<V>::io->stream.open(Node<V>::io->fname, std::ios::in | std::ios::binary);
	//Node<V>::io->stream.seekg(offset);
	//Node<V>::io->stream.read(reinterpret_cast<char*>(&vals_size), sizeof(int));
	//if (vals_size == 0) {
	//	++vals_size;
	//}
	//auto valsBufferBlockSize = Node<V>::io->nunit_size * vals_size;
	//std::vector<char> valsBufferBlock(valsBufferBlockSize);
	//Node<V>::io->stream.read(reinterpret_cast<char*>(&valsBufferBlock[0]), valsBufferBlockSize);
	//Node<V>::io->stream.close();
	//std::stringstream valsBufferStream;
	//std::copy(valsBufferBlock.begin(), valsBufferBlock.end(), std::ostream_iterator<char>(valsBufferStream));
	//vals = std::vector<V>(vals_size);
	//for (int i = 0; i < vals_size; ++i) {
	//	readElement(vals[i], valsBufferStream, Node<V>::schema);
	//	auto tup = vals[i];
	//	auto test = tup;
	//}


    std::stringstream s;
	auto vals_size = Node<V>::keys.size();
    auto node_bytes = Node<V>::io->nhead_size + Node<V>::io->nunit_size;
    std::vector<char> fill(node_bytes, 0);
    s.write(reinterpret_cast<char *>(&fill[0]), node_bytes);
    s.seekp(0);
    auto offset = Node<V>::write_node(s);
    s.seekp(Node<V>::io->nhead_size);
    s.write(reinterpret_cast<char *>(&vals_size), sizeof(int));
    for (auto &val : vals) writeElement(val, s, Node<V>::schema);
    Node<V>::io->stream.open(Node<V>::io->fname, std::ios::in | std::ios::out | std::ios::binary);
    Node<V>::io->stream.seekp(offset);
    Node<V>::io->stream << s.rdbuf();
    Node<V>::io->stream.close();
}


template<std::size_t I = 0, typename... Tp, typename Buffer, typename Schema>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
readElement(std::tuple<Tp...>& t, Buffer& buf, Schema& sch) { }

template<std::size_t I = 0, typename... Tp, typename Buffer, typename Schema>
inline typename std::enable_if < I < sizeof...(Tp), void>::type
	readElement(std::tuple<Tp...> & t, Buffer & buf, Schema & sch) {
	buf.read(reinterpret_cast<char*>(&std::get<I>(t)), sizeof(std::get<I>(t)));
	readElement<I + 1, Tp...>(t, buf, sch);
}

template<typename V>
void Leaf<V>::read(int _o) {
	auto offset = Node<V>::read_node(_o);
	int vals_size{};
	Node<V>::io->stream.open(Node<V>::io->fname, std::ios::in | std::ios::binary);
	Node<V>::io->stream.seekg(offset);
	Node<V>::io->stream.read(reinterpret_cast<char*>(&vals_size), sizeof(int));
	if (vals_size == 0) {
		++vals_size;
	}
	auto valsBufferBlockSize = Node<V>::io->nunit_size * vals_size;
	std::vector<char> valsBufferBlock(valsBufferBlockSize);
	Node<V>::io->stream.read(reinterpret_cast<char*>(&valsBufferBlock[0]), valsBufferBlockSize);
	Node<V>::io->stream.close();
	std::stringstream valsBufferStream;
	std::copy(valsBufferBlock.begin(), valsBufferBlock.end(), std::ostream_iterator<char>(valsBufferStream));
	vals = std::vector<V>(vals_size);
	for (int i = 0; i < vals_size; ++i) {
		readElement(vals[i], valsBufferStream, Node<V>::schema);
		auto tup = vals[i];
		auto test = tup;
	}
	//auto offset = Node<V>::read_node(_o);
	//std::vector<char> s(Node<V>::io->nunit_size);
	//int vals_size{};
	//Node<V>::io->stream.open(Node<V>::io->fname, std::ios::in | std::ios::binary);
	//Node<V>::io->stream.seekg(offset);
	//Node<V>::io->stream.read(reinterpret_cast<char*>(&s[0]), Node<V>::io->nunit_size);
	//Node<V>::io->stream.close();
	//std::stringstream ls;
	//std::copy(s.begin(), s.end(), std::ostream_iterator<char>(ls));
	//ls.read(reinterpret_cast<char*>(&vals_size), sizeof(int));
	//vals = std::vector<V>(vals_size);
	//for (int i = 0; i < vals_size; ++i) {
	//	readElement(vals[i], ls, Node<V>::schema);
	//	auto tup = vals[i];
	//	auto test = tup;
	//}
}

template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
printElements(std::tuple<Tp...>& t) { }

template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if < I < sizeof...(Tp), void>::type
	printElements(std::tuple<Tp...> & t) {
	std::cout << std::get<I>(t) << '\t';
	printElements<I + 1, Tp...>(t);
}

template<typename V>
V Leaf<V>::get(int _index) {
	return vals[_index];
}

#endif