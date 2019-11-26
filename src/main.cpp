#include "Tree.h"
#include "Tests.h"
#include "Io.h"
#include "Runner.h"

int main(int argc, char **argv) {
//    test_inserts();
//    test_deletes();

    if (argc == 1) {
        std::cout << argv[0] << " <dbname>.db\n";
        return -1;
    }

	Runner runner;
	runner.run(argv[1]);

//    const auto COUNT = 20, ORDER = 4;
//    std::vector<int> keys(COUNT);
//    iota_step(keys.begin(), keys.end(), 0, 10);
//	auto file = "db.db";// argv[1];
//    int block_size = 0;
//    auto file_exists = std::filesystem::exists(file);
//    if (!file_exists) {
//        std::cout << "Enter number of blocks to create for the new file:";
//        std::cin >> block_size;
//    }
//    Tree<std::tuple<int>> tree(ORDER, file, block_size);
//
//    auto value = std::make_tuple<int>(5);
//    std::for_each(keys.cbegin(), keys.cend(), [&tree, &value](const auto &cfi) {
//        if (tree.insert(cfi, value)) {
////            std::cout << '\n' << cfi << '\n';tree.tree_dump();
//        }
//    });
//    tree.tree_dump();

//    std::for_each(keys.cbegin(), keys.cend(), [&tree](const auto &cfi) {
////        std::cout << '\n' << cfi << '\n';tree.tree_dump();
//        tree.remove(cfi);
//    });
//    tree.tree_dump();

}