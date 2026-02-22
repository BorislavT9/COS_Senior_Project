"""
Tests for AVL tree implementation.
"""

import pytest
from src.store.avl_tree import AVLTree, AVLNode


def test_avl_insert_and_search():
    """Test basic insert and search operations."""
    tree = AVLTree()
    
    tree.insert("apple", "doc1")
    tree.insert("banana", "doc2")
    tree.insert("cherry", "doc3")
    
    assert tree.search("apple") == {"doc1"}
    assert tree.search("banana") == {"doc2"}
    assert tree.search("cherry") == {"doc3"}
    assert tree.search("nonexistent") == set()


def test_avl_duplicate_keys():
    """Test that duplicate keys add doc_ids to the same node."""
    tree = AVLTree()
    
    tree.insert("apple", "doc1")
    tree.insert("apple", "doc2")
    tree.insert("apple", "doc3")
    
    result = tree.search("apple")
    assert result == {"doc1", "doc2", "doc3"}


def test_avl_balance_after_insert():
    """Test that tree remains balanced after many inserts."""
    tree = AVLTree()
    
    # Insert many keys in sorted order (worst case for BST, but AVL should balance)
    keys = [f"key_{i:03d}" for i in range(100)]
    for i, key in enumerate(keys):
        tree.insert(key, f"doc{i}")
    
    # Check that all keys are searchable
    for i, key in enumerate(keys):
        assert f"doc{i}" in tree.search(key)
    
    # Check tree height is logarithmic (AVL property: height <= 1.44 * log2(n))
    # For 100 nodes, max height should be around 7-8
    def get_height(node):
        if node is None:
            return 0
        return 1 + max(get_height(node.left), get_height(node.right))
    
    if tree.root:
        height = get_height(tree.root)
        # AVL tree with 100 nodes should have height <= 8
        assert height <= 8, f"Tree height {height} is too large for AVL tree"


def test_avl_range_query():
    """Test range query functionality."""
    tree = AVLTree()
    
    tree.insert("apple", "doc1")
    tree.insert("banana", "doc2")
    tree.insert("cherry", "doc3")
    tree.insert("date", "doc4")
    tree.insert("elderberry", "doc5")
    
    # Range query
    results = tree.range_query("banana", "date")
    assert "doc2" in results  # banana
    assert "doc3" in results   # cherry
    assert "doc4" in results   # date
    assert "doc1" not in results  # apple (before range)
    assert "doc5" not in results   # elderberry (after range)


def test_avl_inorder_traversal():
    """Test inorder traversal returns sorted items."""
    tree = AVLTree()
    
    keys = ["zebra", "apple", "banana", "cherry", "date"]
    for i, key in enumerate(keys):
        tree.insert(key, f"doc{i}")
    
    items = tree.inorder_items()
    sorted_keys = [key for key, _ in items]
    
    assert sorted_keys == sorted(keys)


def test_avl_empty_tree():
    """Test operations on empty tree."""
    tree = AVLTree()
    
    assert tree.search("anything") == set()
    assert tree.range_query("a", "z") == set()
    assert tree.inorder_items() == []
    assert tree.size() == 0


def test_avl_rotations():
    """Test that rotations maintain search property."""
    tree = AVLTree()
    
    # Insert in a way that triggers rotations
    tree.insert("c", "doc1")
    tree.insert("b", "doc2")
    tree.insert("a", "doc3")  # Should trigger right rotation
    
    # Verify all keys are still searchable
    assert tree.search("a") == {"doc3"}
    assert tree.search("b") == {"doc2"}
    assert tree.search("c") == {"doc1"}
    
    # Insert in reverse order (triggers left rotation)
    tree2 = AVLTree()
    tree2.insert("a", "doc1")
    tree2.insert("b", "doc2")
    tree2.insert("c", "doc3")  # Should trigger left rotation
    
    assert tree2.search("a") == {"doc1"}
    assert tree2.search("b") == {"doc2"}
    assert tree2.search("c") == {"doc3"}
