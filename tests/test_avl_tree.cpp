/**
 * AVL tree unit tests.
 * Verifies AVL invariant after sequential and random insertions,
 * plus search, range query, and inorder traversal.
 */

#include <gtest/gtest.h>
#include "document_ingestion/avl_tree.hpp"
#include <random>
#include <set>
#include <sstream>

namespace {

using document_ingestion::AVLTree;

TEST(AVLTreeTest, UnitTest1_SequentialInsert_InvariantHolds) {
  AVLTree tree;
  const int N = 50000;
  for (int i = 0; i < N; ++i) {
    std::string key = "key_" + std::to_string(i);
    tree.insert(key, "doc_" + std::to_string(i));
    ASSERT_TRUE(tree.check_avl_invariant()) << "AVL invariant violated at insert " << i;
  }
  EXPECT_EQ(tree.size(), static_cast<size_t>(N));
}

TEST(AVLTreeTest, UnitTest2_RandomInsert_InvariantHolds) {
  AVLTree tree;
  std::mt19937 gen(42);
  std::uniform_int_distribution<> dis(0, 99999);
  std::set<std::string> inserted;
  for (int i = 0; i < 10000; ++i) {
    int val = dis(gen);
    std::string key = "val_" + std::to_string(val);
    tree.insert(key, "doc_" + std::to_string(i));
    inserted.insert(key);
    ASSERT_TRUE(tree.check_avl_invariant()) << "AVL invariant violated at insert " << i;
  }
  EXPECT_EQ(tree.size(), inserted.size());
}

TEST(AVLTreeTest, Search_ReturnsCorrectDocIds) {
  AVLTree tree;
  tree.insert("INV-2024-001", "doc1");
  tree.insert("INV-2024-002", "doc2");
  tree.insert("INV-2024-001", "doc3");  // Same key, different doc
  auto ids = tree.search("INV-2024-001");
  EXPECT_EQ(ids.size(), 2u);
  EXPECT_TRUE(ids.count("doc1"));
  EXPECT_TRUE(ids.count("doc3"));
  EXPECT_TRUE(tree.search("INV-2024-999").empty());
}

TEST(AVLTreeTest, RangeQuery_InclusiveBoundaries) {
  AVLTree tree;
  tree.insert("A", "d1");
  tree.insert("B", "d2");
  tree.insert("C", "d3");
  tree.insert("D", "d4");
  tree.insert("E", "d5");
  auto result = tree.range_query("B", "D");
  EXPECT_EQ(result.size(), 3u);
  EXPECT_TRUE(result.count("d2"));
  EXPECT_TRUE(result.count("d3"));
  EXPECT_TRUE(result.count("d4"));
}

TEST(AVLTreeTest, InorderTraversal_SortedKeys) {
  AVLTree tree;
  tree.insert("C", "c");
  tree.insert("A", "a");
  tree.insert("B", "b");
  tree.insert("E", "e");
  tree.insert("D", "d");
  auto items = tree.inorder_items();
  EXPECT_EQ(items.size(), 5u);
  std::vector<std::string> keys;
  for (const auto& [k, _] : items) keys.push_back(k);
  EXPECT_EQ(keys[0], "A");
  EXPECT_EQ(keys[1], "B");
  EXPECT_EQ(keys[2], "C");
  EXPECT_EQ(keys[3], "D");
  EXPECT_EQ(keys[4], "E");
}

TEST(AVLTreeTest, EmptyTree_InvariantHolds) {
  AVLTree tree;
  EXPECT_TRUE(tree.check_avl_invariant());
  EXPECT_EQ(tree.size(), 0u);
}

}  // namespace
