#pragma once

#include <string>
#include <vector>
#include <set>
#include <memory>
#include <utility>

namespace document_ingestion {

class AVLNode {
 public:
  std::string key;
  std::set<std::string> doc_ids;
  int height = 1;
  std::unique_ptr<AVLNode> left;
  std::unique_ptr<AVLNode> right;

  explicit AVLNode(std::string key, const std::string& doc_id);
};

class AVLTree {
 public:
  AVLTree() = default;

  void insert(const std::string& key, const std::string& doc_id);
  std::set<std::string> search(const std::string& key) const;
  std::set<std::string> range_query(const std::string& low, const std::string& high) const;
  std::vector<std::pair<std::string, std::vector<std::string>>> inorder_items() const;
  size_t size() const { return size_; }

  /** For testing: verifies AVL invariant (|balance_factor| <= 1) at every node. */
  bool check_avl_invariant() const;

 private:
  std::unique_ptr<AVLNode> root_;
  size_t size_ = 0;

  static int height(const AVLNode* node);
  static void update_height(AVLNode* node);
  static int balance_factor(const AVLNode* node);
  static std::unique_ptr<AVLNode> rotate_right(std::unique_ptr<AVLNode> y);
  static std::unique_ptr<AVLNode> rotate_left(std::unique_ptr<AVLNode> x);
  static std::unique_ptr<AVLNode> balance(std::unique_ptr<AVLNode> node);
  std::unique_ptr<AVLNode> insert_recursive(std::unique_ptr<AVLNode> node,
                                             const std::string& key,
                                             const std::string& doc_id);
  std::set<std::string> search_recursive(const AVLNode* node, const std::string& key) const;
  void range_query_recursive(const AVLNode* node, const std::string& low, const std::string& high,
                             std::set<std::string>& result) const;
  void inorder_recursive(const AVLNode* node,
                         std::vector<std::pair<std::string, std::vector<std::string>>>& result) const;
  bool check_avl_invariant_recursive(const AVLNode* node) const;
};

}  // namespace document_ingestion
