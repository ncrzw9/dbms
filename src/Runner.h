
#ifndef ROHE_RUNNER_H
#define ROHE_RUNNER_H

#include <string>
#include <tuple>
#include <filesystem>
#include <iostream>
#include <vector>
#include <random>
#include "Tree.h"

struct Runner {
	const int CLOSE_SELECTION{ -99 };
	const std::string menu_prompt = "\nSelect an option:\n"
		"\t5 - GET\n"
		"\t1 - PUT\n"
		"\t3 - DELETE\n"
		"\t0 - EXIT\n";
	const std::string key_insert_prompt = "Input key to enter into B+ Tree:";
	const std::string keys_insert_prompt = "Input number of keys to enter into B+ Tree:";
	const std::string key_remove_prompt = "Input key to delete from B+ Tree:";
	const std::string keys_remove_prompt = "Input number of random keys to enter into B+ Tree:";
	const std::string key_get_prompt = "Input key to get tuple from B+ Tree:";
	using Tuple_t = std::tuple<V20, V20, V20, V20, V20>;
	const int order{ 4 };
	std::string filename{};
	std::string tableName{};
	int free_list_count{};
	std::string input{};
	int user_choice{};
	std::unique_ptr<Tree<Tuple_t>> tree{};
	std::map<int, std::string> attributes;
	std::map<int, int> schema;

	void run(const std::string&);

	void option_1();

	void option_2();

	void option_3();

	void option_4();

	void option_5();

	static std::vector<int> get_keys(int);
};

void readCSV(std::istream& input, std::vector<std::vector<std::string>>& output) {
	std::string csvLine;
	while (getline(input, csvLine)) {
		std::istringstream csvStream(csvLine);
		std::vector<std::string> csvColumn;
		std::string csvElement;
		while (getline(csvStream, csvElement, ',')) {
			csvColumn.push_back(csvElement);
		}
		output.push_back(csvColumn);
	}
}

void Runner::run(const std::string& _filename) {
	filename = _filename;
	if (!std::filesystem::exists(_filename)) {
		std::cout << "Enter number of free list blocks to create for the new file:";
		std::cin >> free_list_count;
	}

	// check if system catalog exists for filename passed through command line
	std::fstream tableFile("Table.cat", std::ios::in);
	if (!tableFile.is_open()) {
		std::cout << "System catalog not found!\n";
		return;
	}
	std::vector<std::vector<std::string>> csvCatalogData;
	readCSV(tableFile, csvCatalogData);
	bool isInSystemCatalog{ false };
	for (std::vector<std::vector<std::string>>::iterator i = csvCatalogData.begin(); i != csvCatalogData.end(); ++i) {
		for (std::vector<std::string>::iterator j = i->begin(); j != i->end(); ++j) {
			if (*j == _filename) {
				tableName = *(j - 1);
				isInSystemCatalog = true;
				break;
			}
		}
		if (isInSystemCatalog) {
			break;
		}
	}
	if (!isInSystemCatalog) {
		std::cout << _filename << " does not exist in system catalog!\n";
		return;
	}

	// check if attribute catalog exists for table name validated above
	std::fstream attributesFile("Attr.cat", std::ios::in);
	if (!attributesFile.is_open()) {
		std::cout << "Attributes catalog not found!\n";
		return;
	}
	std::vector<std::vector<std::string>> csvAttributesData;
	readCSV(attributesFile, csvAttributesData);
	for (std::vector<std::vector<std::string>>::iterator i = csvAttributesData.begin(); i != csvAttributesData.end(); ++i) {
		std::string attribute;
		int position = 0;
		for (std::vector<std::string>::iterator j = i->begin(); j != i->end(); ++j) {
			if (i->front() == tableName && *j != tableName && j != (i->end() - 1)) {
				attribute += *j + ',';
			}
			if (i->front() == tableName && *j != tableName && j == (i->end() - 1)) {
				attribute += *j;
				position = std::stoi(*j);
			}
		}
		if (position != 0) {
			attributes[position] = attribute;
		}
	}
	if (attributes.empty()) {
		std::cout << "No attribute exists for " << tableName << "!\n";
		return;
	}

	std::cout << "Attributes " << tableName << "\n";
	for (const auto& attr : attributes) {
		std::cout << ' ' << attr.second << '\n';
		if (attr.second.find(",Int,") != std::string::npos) {
			schema[attr.first] = 0;//Int
		} else {
			schema[attr.first] = 1;//V20
		}
	}

	tree = std::make_unique<Tree<Tuple_t>>(order, filename, free_list_count, schema);
	while (true) {
		std::cout << menu_prompt;
		std::cin >> user_choice;
		if (!user_choice) {
			break;
		}
		else if (user_choice == 1) {
			option_1();
		}
		else if (user_choice == 2) {
			//option_2();
		}
		else if (user_choice == 3) {
			option_3();
		}
		else if (user_choice == 4) {
			//option_4();
		}
		else if (user_choice == 5) {
			option_5();
		}
	}
}

void Runner::option_1() {
	int insert_key{};
	while (true) {
		Tuple_t tuple;
		for (const auto& attr : attributes) {
			V20 varchar;
			std::cout << attr.second << ':';
			if (!schema[attr.first]) {
				int integer;
				std::cin >> integer;
				while (std::cin.fail()) {
					std::cin.clear();
					std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
					std::cout << "Enter an INTEGER: ";
					std::cin >> integer;
				}
				if (attr.first == 1) {
					insert_key = integer;
					if (insert_key == CLOSE_SELECTION) break;
				}
				varchar.writeInt(integer);
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			} else {
				std::string str;
				std::getline(std::cin, str);
				while (str.size() > 20) {
					std::cout << "Enter an text NO LONGER THAN 20 CHARACTERS: ";
					std::getline(std::cin, str);
				}
				varchar.writeStr(str);
			}
			if (attr.first == 1) {
				std::get<0>(tuple) = varchar;
			}
			if (attr.first == 2) {
				std::get<1>(tuple) = varchar;
			}
			if (attr.first == 3) {
				std::get<2>(tuple) = varchar;
			}
			if (attr.first == 4) {
				std::get<3>(tuple) = varchar;
			}
			if (attr.first == 5) {
				std::get<4>(tuple) = varchar;
			}
		}

		if (insert_key == CLOSE_SELECTION) break;
		tree->insert(insert_key, tuple);
		tree->tree_dump();
		tree->leaf_dump();
	}
}

void Runner::option_2() {
	tree = std::make_unique<Tree<Tuple_t>>(order, filename, free_list_count, schema);
	std::cout << keys_insert_prompt;
	int key_inserts{};
	std::cin >> key_inserts;
	auto ord_keys = get_keys(key_inserts);
	Tuple_t tuple;
	std::for_each(ord_keys.cbegin(), ord_keys.cend(), [this, &tuple](const auto& fwd_it) {
		tree->insert(fwd_it, tuple);
		});
	tree->tree_dump();
	tree->leaf_dump();
}

void Runner::option_3() {
	int remove_key{};
	while (true) {
		std::cout << "Enter " << attributes[1].substr(0, attributes[1].find_first_of(',')) << ':';
		std::cin >> remove_key;
		if (remove_key == CLOSE_SELECTION) break;
		tree->remove(remove_key);
		tree->tree_dump();
		tree->leaf_dump();
	}
}

void Runner::option_4() {
	tree = std::make_unique<Tree<Tuple_t>>(order, filename, free_list_count, schema);
	std::cout << keys_remove_prompt;
	int key_removes{};
	std::cin >> key_removes;
	auto shf_keys = get_keys(key_removes);
	std::seed_seq rnds{ 1 };
	std::shuffle(shf_keys.begin(), shf_keys.end(), std::mt19937{ rnds });
	Tuple_t tuple;
	std::for_each(shf_keys.cbegin(), shf_keys.cend(), [this, &tuple](const auto& fwd_it) {
		tree->insert(fwd_it, tuple);
		});
	tree->tree_dump();
	tree->leaf_dump();
	std::for_each(shf_keys.cbegin(), shf_keys.cend(), [this](const auto& fwd_it) {
		tree->remove(fwd_it);
		tree->tree_dump();
		tree->leaf_dump();
		});
}

std::vector<int> Runner::get_keys(int _size) {
	std::vector<int> keys(_size);
	int gain{ 0 }, step{ 10 };
	auto first = keys.begin(), last = keys.end();
	while (first != last) {
		*first++ = gain * step;
		++gain;
	}
	return keys;
}

void Runner::option_5() {
	int get_key{};
	while (true) {
		std::cout << "Enter " << attributes[1].substr(0, attributes[1].find_first_of(',')) << ':';
		std::cin >> get_key;
		if (get_key == CLOSE_SELECTION) break;
		Tuple_t tuple = tree->get(get_key);
		for (const auto& attr : attributes)
			std::cout << attr.second.substr(0, attr.second.find_first_of(',')) << '\t';
		std::cout << std::endl;
		printElements(tuple);
		std::cout << std::endl;
	}
}

#endif //ROHE_RUNNER_H
