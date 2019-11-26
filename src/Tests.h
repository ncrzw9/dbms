/*
include schema in tree ctor
*/

//#ifndef TANE_TEST_H
//#define TANE_TESTS_H
//
//#include <random>
//#include <chrono>
//#include "Tree.h"
//
//const auto INORDER_COMPLETE = 100;
//const auto INORDER_COMPLETE_ORDER = 4;
//const auto TIMED_REGRESSION = 1000000;
//const auto TIMED_REGRESSION_ORDER = 100;
//
//const auto file_name = "file.db";
//
//class Timer {
//    using clock = std::chrono::high_resolution_clock;
//    std::chrono::time_point<clock> start;
//public:
//    Timer() : start(clock::now()) {}
//
//    void reset() { start = clock::now(); }
//
//    [[nodiscard]] double elapsed() const { return std::chrono::duration<double>(clock::now() - start).count(); }
//
//    [[nodiscard]] std::string elapsed_str() const { return std::to_string(elapsed()) + " sec"; }
//};
//
//template<typename U>
//bool auto_insert(const int _o, const U &_f, const U &_l, const std::string &_n) {
//    auto value = std::make_tuple<int>(0);
//    auto count = std::distance(_f, _l);
//    Tree<std::tuple<int>> tree(_o, file_name, count);
//    auto is_correct = true;
//    std::for_each(_f, _l, [&tree, &value, &is_correct](const auto &cfi) {
//        tree.insert(cfi, value);
//        auto test = tree.get_tree_keys();
////        std::copy(test.cbegin(), test.cend(), std::ostream_iterator<int>(std::cout, " "));
////        std::cout << '\n' << '\n';
//        if (!std::is_sorted(test.cbegin(), test.cbegin())) is_correct = false;
//    });
////    tree.tree_dump();
////    tree.leaf_dump();
//    is_correct ? std::cout << "PASS" : std::cout << "FAIL";
//    std::cout << ' ' << std::distance(_f, _l) << ' ' << _n << " -> ";
//
//    std::uintmax_t n = std::filesystem::remove(file_name);
//    std::cout << "Deleted " << n << " file\n";
//
//    return is_correct;
//}
//
//template<typename U>
//void auto_insert_time(const int _o, const U &_f, const U &_l) {
//    auto value = std::make_tuple<int>(0);
//    auto count = std::distance(_f, _l);
//    Tree<std::tuple<int>> tree(_o, file_name, count);
//    Timer timer;
//    std::for_each(_f, _l, [&tree, &value](const auto &cfi) {
////        std::cout << cfi << '\n';
//        tree.insert(cfi, value); });
//    std::cout << std::distance(_f, _l) << " in " << timer.elapsed_str() << '\n';
//
//    std::uintmax_t n = std::filesystem::remove(file_name);
//    std::cout << "Deleted " << n << " file\n";
//}
//
//void test_inserts() {
//    std::vector<int> ord_keys_inorder(INORDER_COMPLETE);
//    iota_step(ord_keys_inorder.begin(), ord_keys_inorder.end(), 0, 10);
//    std::vector<int> ord_keys_regression(TIMED_REGRESSION);
//    iota_step(ord_keys_regression.begin(), ord_keys_regression.end(), 0, 10);
//
//
//    if (auto_insert(INORDER_COMPLETE_ORDER, ord_keys_inorder.cbegin(), ord_keys_inorder.cend(),
//                         " INSERT ORDERED ASC")) {
////        auto_insert_time(TIMED_REGRESSION_ORDER, ord_keys_regression.cbegin(), ord_keys_regression.cend());
//    }
//
//    if (auto_insert(INORDER_COMPLETE_ORDER, ord_keys_inorder.crbegin(), ord_keys_inorder.crend(),
//                         " INSERT ORDERED DSC")) {
////        auto_insert_time(TIMED_REGRESSION_ORDER, ord_keys_regression.crbegin(), ord_keys_regression.crend());
//    }
//
//    auto seed_count = 4;
//    auto shf_keys_inorder = ord_keys_inorder;
//    auto shf_keys_regression = ord_keys_regression;
//    for (auto i = 0; i < seed_count; ++i) {
//        std::seed_seq seq{i};
//        std::shuffle(shf_keys_inorder.begin(), shf_keys_inorder.end(), std::mt19937{seq});
//        std::shuffle(shf_keys_regression.begin(), shf_keys_regression.end(), std::mt19937{seq});
//
//        if (auto_insert(INORDER_COMPLETE_ORDER, shf_keys_inorder.cbegin(), shf_keys_inorder.cend(),
//                             " INSERT SHUFFLE ASC SEED(" + std::to_string(i) + ')')) {
////            auto_insert_time(TIMED_REGRESSION_ORDER, shf_keys_regression.cbegin(), shf_keys_regression.cend());
//        }
//        if (auto_insert(INORDER_COMPLETE_ORDER, shf_keys_inorder.crbegin(), shf_keys_inorder.crend(),
//                             " INSERT SHUFFLE DSC SEED(" + std::to_string(i) + ')')) {
////            auto_insert_time(TIMED_REGRESSION_ORDER, shf_keys_regression.crbegin(), shf_keys_regression.crend());
//        }
//    }
//
//    auto rnd_keys_inorder = ord_keys_inorder;
//    auto rnd_keys_regression = ord_keys_regression;
//    for (auto i = 0; i < seed_count; ++i) {
//        std::seed_seq seq{i};
//        constexpr auto lo_dist{1}, up_dist{std::numeric_limits<int>::max()};
//        std::mt19937 mersenne_engine{seq};
//        std::uniform_int_distribution<int> dist{lo_dist, up_dist};
//
//        std::generate(rnd_keys_inorder.begin(), rnd_keys_inorder.end(),
//                      [&dist, &mersenne_engine]() { return dist(mersenne_engine); }
//        );
//        std::sort(rnd_keys_inorder.begin(), rnd_keys_inorder.end());
//        rnd_keys_inorder.erase(std::unique(rnd_keys_inorder.begin(), rnd_keys_inorder.end()), rnd_keys_inorder.end());
//
//        std::generate(rnd_keys_regression.begin(), rnd_keys_regression.end(),
//                      [&dist, &mersenne_engine]() { return dist(mersenne_engine); }
//        );
//        std::sort(rnd_keys_regression.begin(), rnd_keys_regression.end());
//        rnd_keys_regression.erase(std::unique(rnd_keys_regression.begin(), rnd_keys_regression.end()),
//                                  rnd_keys_regression.end());
//
//        if (auto_insert(INORDER_COMPLETE_ORDER, rnd_keys_inorder.cbegin(), rnd_keys_inorder.cend(),
//                             " INSERT RANDOM ASC SEED(" + std::to_string(i) + ')')) {
////            auto_insert_time(TIMED_REGRESSION_ORDER, rnd_keys_regression.cbegin(), rnd_keys_regression.cend());
//        }
//        if (auto_insert(INORDER_COMPLETE_ORDER, rnd_keys_inorder.crbegin(), rnd_keys_inorder.crend(),
//                             " INSERT RANDOM DSC SEED(" + std::to_string(i) + ')')) {
////            auto_insert_time(TIMED_REGRESSION_ORDER, rnd_keys_regression.crbegin(), rnd_keys_regression.crend());
//        }
//    }
//}
//
//template<typename T, typename U>
//bool auto_delete(const int _o, const T &_s, const T &_e, const U &_f, const U &_l, const std::string &_n) {
//    auto value = std::make_tuple<int>(0);
//    auto count = std::distance(_s, _e);
//    Tree<std::tuple<int>> tree(_o, file_name, count);
//    std::for_each(_s, _e, [&tree, &value](const auto &cfi) { tree.insert(cfi, value); });
//    auto is_correct = true;
//    std::for_each(_f, _l, [&_n, &tree, &is_correct](const auto &cfi) {
////        std::cout << "Delete " << cfi << '\n';
//        tree.remove(cfi);
////        tree.tree_dump();
//        auto test = tree.get_tree_keys();
//        if (!std::is_sorted(test.cbegin(), test.cbegin())) is_correct = false;
//    });
////    tree.tree_dump();
//    is_correct ? std::cout << "PASS" : std::cout << "FAIL";
//    std::cout << ' ' << std::distance(_f, _l) << ' ' << _n << " -> ";
//
//    std::uintmax_t n = std::filesystem::remove(file_name);
//    std::cout << "Deleted " << n << " file\n";
//
//    return is_correct;
//}
//
//template<typename T, typename U>
//void auto_delete_time(const int _o, const T &_s, const T &_e, const U &_f, const U &_l) {
//    auto value = std::make_tuple<int>(0);
//    auto count = std::distance(_s, _e);
//    Tree<std::tuple<int>> tree(_o, file_name, count);
//    std::for_each(_s, _e, [&tree, &value](const auto &cfi) { tree.insert(cfi, value); });
//    Timer timer;
//    std::for_each(_f, _l, [&tree](const auto &cfi) { tree.remove(cfi); });
//    std::cout << std::distance(_f, _l) << " in " << timer.elapsed_str() << '\n';
//
//    std::uintmax_t n = std::filesystem::remove(file_name);
//    std::cout << "Deleted " << n << " file\n";
//}
//
//void test_deletes() {
//    std::vector<int> ord_keys_inorder(INORDER_COMPLETE);
//    iota_step(ord_keys_inorder.begin(), ord_keys_inorder.end(), 0, 10);
//    std::vector<int> ord_keys_regression(TIMED_REGRESSION);
//    iota_step(ord_keys_regression.begin(), ord_keys_regression.end(), 0, 10);
//
//    if (auto_delete(INORDER_COMPLETE_ORDER, ord_keys_inorder.cbegin(), ord_keys_inorder.cend(),
//                         ord_keys_inorder.cbegin(),
//                         ord_keys_inorder.cend(), " DELETE FORWARD")) {
////        auto_delete_time(TIMED_REGRESSION_ORDER, ord_keys_regression.cbegin(), ord_keys_regression.cend(),
////                               ord_keys_regression.cbegin(),
////                               ord_keys_regression.cend());
//    }
//
//    if (auto_delete(INORDER_COMPLETE_ORDER, ord_keys_inorder.cbegin(), ord_keys_inorder.cend(),
//                         ord_keys_inorder.crbegin(),
//                         ord_keys_inorder.crend(), " DELETE BACKWARD")) {
////        auto_delete_time(TIMED_REGRESSION_ORDER, ord_keys_regression.cbegin(), ord_keys_regression.cend(),
////                               ord_keys_regression.crbegin(),
////                               ord_keys_regression.crend());
//    }
//
//    auto cmiddle_inorder = ord_keys_inorder.cbegin() + ord_keys_inorder.size() / 2;
//    auto cmiddle_regression = ord_keys_regression.cbegin() + ord_keys_regression.size() / 2;
//    if (auto_delete(INORDER_COMPLETE_ORDER, ord_keys_inorder.cbegin(), ord_keys_inorder.cend(), cmiddle_inorder,
//                         ord_keys_inorder.cend(),
//                         " DELETE MIDDLE FORWARD")) {
////        auto_delete_time(TIMED_REGRESSION_ORDER, ord_keys_regression.cbegin(), ord_keys_regression.cend(),
////                               cmiddle_regression,
////                               ord_keys_regression.cend());
//    }
//
//    auto crmiddle_inorder = ord_keys_inorder.crbegin() + ord_keys_inorder.size() / 2;
//    auto crmiddle_regression = ord_keys_regression.crbegin() + ord_keys_regression.size() / 2;
//    if (auto_delete(INORDER_COMPLETE_ORDER, ord_keys_inorder.cbegin(), ord_keys_inorder.cend(), crmiddle_inorder,
//                         ord_keys_inorder.crend(),
//                         " DELETE MIDDLE BACKWARD")) {
////        auto_delete_time(TIMED_REGRESSION_ORDER, ord_keys_regression.cbegin(), ord_keys_regression.cend(),
////                               crmiddle_regression,
////                               ord_keys_regression.crend());
//    }
//
//    auto shf_keys_inorder = ord_keys_inorder;
//    auto shf_keys_regression = ord_keys_regression;
//    auto shf_tests_count = 4;
//    for (auto i = 0; i < shf_tests_count; ++i) {
//        std::seed_seq seq{i};
//        std::shuffle(shf_keys_inorder.begin(), shf_keys_inorder.end(), std::mt19937{seq});
//        if (auto_delete(INORDER_COMPLETE_ORDER, shf_keys_inorder.cbegin(), shf_keys_inorder.cend(),
//                             shf_keys_inorder.cbegin(),
//                             shf_keys_inorder.cend(),
//                             " DELETE SHUFFLE SEED(" + std::to_string(i) + ')')) {
//            std::shuffle(shf_keys_regression.begin(), shf_keys_regression.end(), std::mt19937{seq});
////            auto_delete_time(TIMED_REGRESSION_ORDER, shf_keys_regression.cbegin(), shf_keys_regression.cend(),
////                                   shf_keys_regression.cbegin(), shf_keys_regression.cend());
//        }
//    }
//}
//
//void run(const std::string& _dbfile) {
//    auto record = std::make_tuple(1);
//    std::string input{};
//    int selection{};
//    const auto CLOSE = -99;
//    const auto prompt = "Select an option:\n"
//                        "\t0 - Exit\n"
//                        "\t1 - Create Tree\n"
//                        "\t2 - Auto Create Tree\n"
//                        "\t3 - Delete Element\n"
//                        "\t4 - Auto Delete Elements\n";
//    int run_order = 4;
//    int block_size = 0;
//    auto file_exists = std::filesystem::exists(_dbfile);
//    if (!file_exists) {
//        std::cout << "Enter number of blocks to create for the new file:";
//        std::cin >> block_size;
//    }
//    std::unique_ptr<Tree<std::tuple<int>>> tree(new Tree<std::tuple<int>>(run_order, _dbfile, block_size));
//    while (true) {
//        std::cout << std::endl << prompt;
//        std::cin >> selection;
//        if (selection == 0) {
//            break;
//        } else if (selection == 1) {
//            int input_key = 0;
//            while (true) {
//                const std::string input_prompt = "Input key to enter into B+ Tree:";
//                std::cout << input_prompt;
//                std::cin >> input_key;
//                if (input_key == CLOSE) break;
//                tree->insert(input_key, record);
//                tree->tree_dump();
//                tree->leaf_dump();
//            }
//        } else if (selection == 2) {
//            tree = std::make_unique<Tree<std::tuple<int>>>(run_order, _dbfile, block_size);
//            const std::string selection_prompt = "Input number of keys to enter into B+ Tree:";
//            std::cout << selection_prompt;
//            auto key_count = 0;
//            std::cin >> key_count;
//            std::vector<int> ord_keys(static_cast<unsigned long>(key_count));
//            iota_step(std::begin(ord_keys), std::end(ord_keys), 0, 10);
//            for (auto ri = std::cbegin(ord_keys); ri != std::cend(ord_keys); ++ri)
//                tree->insert(*ri, record);
//            tree->tree_dump();
//            tree->leaf_dump();
//        } else if (selection == 3) {
//            int input_key = 0;
//            while (true) {
//                const std::string input_prompt = "Input key to delete from B+ Tree:";
//                std::cout << input_prompt;
//                std::cin >> input_key;
//                if (input_key == CLOSE) break;
//                tree->remove(input_key);
//                tree->tree_dump();
//                tree->leaf_dump();
//            }
//        } else if (selection == 4) {
//            tree = std::make_unique<Tree<std::tuple<int>>>(run_order, _dbfile, block_size);
//            const std::string selection_prompt = "Input number of random keys to enter into B+ Tree:";
//            std::cout << selection_prompt;
//            auto key_count = 0;
//            std::cin >> key_count;
//            std::vector<int> shf_keys(static_cast<unsigned long>(key_count * 2));
//            std::seed_seq rnd_seq_count{1};
//            auto upper = key_count * key_count;
//            auto lo_dist{1}, up_dist{upper};
//            std::mt19937 mersenne_engine{rnd_seq_count};
//            std::uniform_int_distribution<int> dist{lo_dist, up_dist};
//            std::generate(shf_keys.begin(), shf_keys.end(),
//                          [&dist, &mersenne_engine]() { return dist(mersenne_engine); }
//            );
//            auto last = std::unique(shf_keys.begin(), shf_keys.end());
//            shf_keys.erase(last, shf_keys.end());
//            if (key_count < static_cast<int>(shf_keys.size())) {
//                shf_keys.erase(shf_keys.begin() + key_count, shf_keys.end());
//            }
//            std::for_each(shf_keys.cbegin(), shf_keys.cend(), [&tree, &record](const auto &cfi) {
//                tree->insert(cfi, record);
//            });
//            tree->tree_dump();
//            tree->leaf_dump();
//            std::for_each(shf_keys.cbegin(), shf_keys.cend(), [&tree](const auto &cfi) {
//                tree->remove(cfi);
//                tree->tree_dump();
//                tree->leaf_dump();
//            });
//        }
//    }
//}
//
//#endif