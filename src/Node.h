#include <memory>
#include <utility>
#include <vector>
#include <iterator>
#include <sstream>
#include <array>
#include <charconv>

#ifndef TANE_NODE_H
#define TANE_NODE_H

class V20 {
	bool isInt{ false };
	std::array<char, 20> arr;
	void clear() {
		for (size_t i{ 0 }; i < arr.size(); ++i) {
			arr[i] = 0;
		}
	}
	int readInt() {
		int integer;
		std::from_chars(arr.data(), arr.data() + arr.size(), integer);
		return integer;
	}
	std::string readStr() {
		std::string str(arr.data(), arr.size());
		return str;
	}
public:
	void writeInt(int integer) {
		isInt = true;
		clear();
		std::to_chars(arr.data(), arr.data() + arr.size(), integer);
	}
	void writeStr(std::string str) {
		isInt = false;
		clear();
		str.copy(arr.data(), arr.size());
	}
	friend std::ostream& operator<<(std::ostream& os, V20& ar);
};

std::ostream& operator<<(std::ostream& os, V20& ar) {
	if (ar.isInt) {
		os << ar.readInt();
	}
	else {
		os << ar.readStr();
	}
	return os;
}

template <typename V>
struct Io;

constexpr int null_rid{std::numeric_limits<int>::max()};

template<typename V>
struct Node {
    int order{};
    bool is_leaf{};
    int rid{};
    int parent{ null_rid };
    int left{ null_rid };
    int right{ null_rid };
    std::shared_ptr<Io<V>> io;
    std::vector<int> keys{};
	std::map<int, int> schema;

    typename std::vector<int>::iterator get_ubound(int);

    std::pair<std::vector<int>::iterator, int> get_lbound(std::vector<int>::iterator, int);

    explicit Node(int, bool, std::shared_ptr<Io<V>>, std::map<int, int>);

    virtual ~Node() = 0;

    virtual std::stringstream &dump(int &, std::stringstream &) = 0;

    virtual std::pair<std::shared_ptr<Node>, int> split() = 0;

    virtual void move_val(int, V &&) = 0;

    virtual void move_rid(int, int &&) = 0;

    virtual V get_val(int) = 0;

    virtual int get_rid(int) = 0;

    virtual void remove(int) = 0;

    virtual void move_one_right(std::shared_ptr<Node>) = 0;

    virtual void move_one_left(std::shared_ptr<Node>) = 0;

    virtual void move_current_left(std::shared_ptr<Node>) = 0;

    virtual void move_right_current(std::shared_ptr<Node>) = 0;

    virtual void write() = 0;

    int write_node(std::stringstream&);

    int read_node(int);

    virtual void read(int) = 0;

	virtual V get(int) = 0;
};

template<typename V>
Node<V>::Node(const int _o, bool _is_leaf, std::shared_ptr<Io<V>> _io, std::map<int, int> _schema)
        : io(_io), is_leaf(_is_leaf), order(_o), schema(_schema){
    keys.reserve(order + 1);
}

template<typename V>
Node<V>::~Node() = default;

template<typename V> std::vector<int>::iterator Node<V>::get_ubound(int _key) {
    return std::upper_bound(keys.begin(), keys.end(), _key);
}

template<typename V>
std::pair<std::vector<int>::iterator, int> Node<V>::get_lbound(std::vector<int>::iterator _c, int _k) {
    auto lbound = std::lower_bound(keys.begin(), _c, _k);
    return std::pair<std::vector<int>::iterator, int>(std::move(lbound), lbound - keys.begin());
}

template<typename V>
int Node<V>::write_node(std::stringstream& _s) {
    auto tree_bytes = io->ROOT_SIZE + io->ROOT_SIZE + io->ROOT_SIZE + io->free_size;
    auto node_bytes = io->nhead_size + io->nunit_size;
    auto offset = tree_bytes + (node_bytes * rid);
    _s.write(reinterpret_cast<char*>(&is_leaf), sizeof(bool));
    std::vector<int> buf_neighbors{ parent, left, right };
    _s.write(reinterpret_cast<char*>(&buf_neighbors[0]), buf_neighbors.size() * sizeof(buf_neighbors[0]));
    int keys_size = keys.size();
    _s.write(reinterpret_cast<char*>(&keys_size), sizeof(int));
    int keys_bytes_size = keys_size * sizeof(int);
	if (keys_size) {
		_s.write(reinterpret_cast<char*>(&keys[0]), keys_bytes_size);
	}
    return offset;
}

template<typename V>
int Node<V>::read_node(int _o) {
    int keys_size{};
    io->stream.open(io->fname, std::ios::in | std::ios::binary);
    io->stream.seekg(_o);
    io->stream.read(reinterpret_cast<char*>(&parent), sizeof(int));
    io->stream.read(reinterpret_cast<char*>(&left), sizeof(int));
    io->stream.read(reinterpret_cast<char*>(&right), sizeof(int));
    io->stream.read(reinterpret_cast<char*>(&keys_size), sizeof(int));
    for (int i = 0; i < keys_size; ++i) {
        int key{};
        io->stream.read(reinterpret_cast<char*>(&key), sizeof(int));
        keys.push_back(key);
    }
    io->stream.close();
    int next_byte = (_o - 1) + io->nhead_size;
    return next_byte;
}

#endif