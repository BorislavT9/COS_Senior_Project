#include "document_ingestion/avl_tree.hpp"
#include <algorithm>

namespace document_ingestion {

AVLNode::AVLNode(std::string k, const std::string& doc_id) : key(std::move(k)) {
  doc_ids.insert(doc_id);
}

int AVLTree::height(const AVLNode* node) {
  return node ? node->height : 0;
}

void AVLTree::update_height(AVLNode* node) {
  if (node) {
    node->height = 1 + std::max(height(node->left.get()), height(node->right.get()));
  }
}

int AVLTree::balance_factor(const AVLNode* node) {
  return node ? height(node->left.get()) - height(node->right.get()) : 0;
}

std::unique_ptr<AVLNode> AVLTree::rotate_right(std::unique_ptr<AVLNode> y) {
  auto x = std::move(y->left);
  auto T2 = std::move(x->right);
  x->right = std::move(y);
  x->right->left = std::move(T2);
  update_height(x->right.get());
  update_height(x.get());
  return x;
}

std::unique_ptr<AVLNode> AVLTree::rotate_left(std::unique_ptr<AVLNode> x) {
  auto y = std::move(x->right);
  auto T2 = std::move(y->left);
  y->left = std::move(x);
  y->left->right = std::move(T2);
  update_height(y->left.get());
  update_height(y.get());
  return y;
}

std::unique_ptr<AVLNode> AVLTree::balance(std::unique_ptr<AVLNode> node) {
  int bf = balance_factor(node.get());
  if (bf > 1 && balance_factor(node->left.get()) >= 0) {
    return rotate_right(std::move(node));
  }
  if (bf < -1 && balance_factor(node->right.get()) <= 0) {
    return rotate_left(std::move(node));
  }
  if (bf > 1 && balance_factor(node->left.get()) < 0) {
    node->left = rotate_left(std::move(node->left));
    return rotate_right(std::move(node));
  }
  if (bf < -1 && balance_factor(node->right.get()) > 0) {
    node->right = rotate_right(std::move(node->right));
    return rotate_left(std::move(node));
  }
  return node;
}

std::unique_ptr<AVLNode> AVLTree::insert_recursive(std::unique_ptr<AVLNode> node,
                                                    const std::string& key,
                                                    const std::string& doc_id) {
  if (!node) {
    size_++;
    return std::make_unique<AVLNode>(key, doc_id);
  }
  if (key < node->key) {
    node->left = insert_recursive(std::move(node->left), key, doc_id);
  } else if (key > node->key) {
    node->right = insert_recursive(std::move(node->right), key, doc_id);
  } else {
    node->doc_ids.insert(doc_id);
    return node;
  }
  update_height(node.get());
  return balance(std::move(node));
}

void AVLTree::insert(const std::string& key, const std::string& doc_id) {
  root_ = insert_recursive(std::move(root_), key, doc_id);
}

std::set<std::string> AVLTree::search_recursive(const AVLNode* node, const std::string& key) const {
  if (!node) return {};
  if (key < node->key) return search_recursive(node->left.get(), key);
  if (key > node->key) return search_recursive(node->right.get(), key);
  return node->doc_ids;
}

std::set<std::string> AVLTree::search(const std::string& key) const {
  return search_recursive(root_.get(), key);
}

void AVLTree::range_query_recursive(const AVLNode* node, const std::string& low,
                                    const std::string& high,
                                    std::set<std::string>& result) const {
  if (!node) return;
  if (low <= node->key && node->key <= high) {
    for (const auto& id : node->doc_ids) result.insert(id);
  }
  if (low < node->key) range_query_recursive(node->left.get(), low, high, result);
  if (high > node->key) range_query_recursive(node->right.get(), low, high, result);
}

std::set<std::string> AVLTree::range_query(const std::string& low, const std::string& high) const {
  std::set<std::string> result;
  range_query_recursive(root_.get(), low, high, result);
  return result;
}

void AVLTree::inorder_recursive(const AVLNode* node,
                                std::vector<std::pair<std::string, std::vector<std::string>>>& result) const {
  if (!node) return;
  inorder_recursive(node->left.get(), result);
  std::vector<std::string> ids(node->doc_ids.begin(), node->doc_ids.end());
  std::sort(ids.begin(), ids.end());
  result.emplace_back(node->key, std::move(ids));
  inorder_recursive(node->right.get(), result);
}

std::vector<std::pair<std::string, std::vector<std::string>>> AVLTree::inorder_items() const {
  std::vector<std::pair<std::string, std::vector<std::string>>> result;
  inorder_recursive(root_.get(), result);
  return result;
}

}  // namespace document_ingestion
