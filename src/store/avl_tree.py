"""
Self-balancing AVL Tree implementation for indexing extracted field values.
Each node stores a key (normalized value) and a set of document IDs.

Time complexity:
- insert: O(log n)
- search: O(log n)
- range_query: O(log n + k) where k is number of results
- inorder_items: O(n)
"""

from typing import Optional, Set, List, Tuple


class AVLNode:
    """
    Node in an AVL tree.
    
    Attributes:
        key: The normalized value (string)
        doc_ids: Set of document IDs that have this value
        height: Height of the subtree rooted at this node
        left: Left child node
        right: Right child node
    """
    
    def __init__(self, key: str, doc_id: str):
        self.key = key
        self.doc_ids: Set[str] = {doc_id}
        self.height = 1
        self.left: Optional['AVLNode'] = None
        self.right: Optional['AVLNode'] = None


class AVLTree:
    """
    Self-balancing AVL Binary Search Tree.
    
    Maintains balance factor <= 1 for all nodes, ensuring O(log n) operations.
    Used for indexing extracted field values by normalized value.
    """
    
    def __init__(self):
        self.root: Optional[AVLNode] = None
        self._size = 0
    
    def _height(self, node: Optional[AVLNode]) -> int:
        """Get height of a node (0 if None)."""
        if node is None:
            return 0
        return node.height
    
    def _update_height(self, node: AVLNode) -> None:
        """Update height of a node based on children."""
        node.height = 1 + max(self._height(node.left), self._height(node.right))
    
    def _balance_factor(self, node: Optional[AVLNode]) -> int:
        """
        Calculate balance factor: height(left) - height(right).
        Should be in [-1, 0, 1] for balanced tree.
        """
        if node is None:
            return 0
        return self._height(node.left) - self._height(node.right)
    
    def _rotate_right(self, y: AVLNode) -> AVLNode:
        """
        Right rotation to fix left-heavy subtree.
        
        Time complexity: O(1)
        """
        x = y.left
        T2 = x.right
        
        # Perform rotation
        x.right = y
        y.left = T2
        
        # Update heights
        self._update_height(y)
        self._update_height(x)
        
        return x
    
    def _rotate_left(self, x: AVLNode) -> AVLNode:
        """
        Left rotation to fix right-heavy subtree.
        
        Time complexity: O(1)
        """
        y = x.right
        T2 = y.left
        
        # Perform rotation
        y.left = x
        x.right = T2
        
        # Update heights
        self._update_height(x)
        self._update_height(y)
        
        return y
    
    def _balance(self, node: AVLNode) -> AVLNode:
        """
        Balance a node by performing rotations if needed.
        
        Time complexity: O(1)
        """
        balance = self._balance_factor(node)
        
        # Left Left case
        if balance > 1 and self._balance_factor(node.left) >= 0:
            return self._rotate_right(node)
        
        # Right Right case
        if balance < -1 and self._balance_factor(node.right) <= 0:
            return self._rotate_left(node)
        
        # Left Right case
        if balance > 1 and self._balance_factor(node.left) < 0:
            node.left = self._rotate_left(node.left)
            return self._rotate_right(node)
        
        # Right Left case
        if balance < -1 and self._balance_factor(node.right) > 0:
            node.right = self._rotate_right(node.right)
            return self._rotate_left(node)
        
        return node
    
    def _insert_recursive(self, node: Optional[AVLNode], key: str, doc_id: str) -> AVLNode:
        """
        Recursively insert a key-value pair into the tree.
        
        Time complexity: O(log n)
        """
        # Step 1: Perform normal BST insert
        if node is None:
            self._size += 1
            return AVLNode(key, doc_id)
        
        if key < node.key:
            node.left = self._insert_recursive(node.left, key, doc_id)
        elif key > node.key:
            node.right = self._insert_recursive(node.right, key, doc_id)
        else:
            # Key exists: add doc_id to set
            node.doc_ids.add(doc_id)
            return node
        
        # Step 2: Update height
        self._update_height(node)
        
        # Step 3: Balance the node
        return self._balance(node)
    
    def insert(self, key: str, doc_id: str) -> None:
        """
        Insert a key-value pair into the tree.
        If key exists, adds doc_id to the existing set.
        
        Args:
            key: Normalized extracted value
            doc_id: Document ID that contains this value
        
        Time complexity: O(log n)
        """
        self.root = self._insert_recursive(self.root, key, doc_id)
    
    def _search_recursive(self, node: Optional[AVLNode], key: str) -> Optional[Set[str]]:
        """
        Recursively search for a key.
        
        Time complexity: O(log n)
        """
        if node is None:
            return None
        
        if key < node.key:
            return self._search_recursive(node.left, key)
        elif key > node.key:
            return self._search_recursive(node.right, key)
        else:
            return node.doc_ids
    
    def search(self, key: str) -> Set[str]:
        """
        Search for a key and return the set of document IDs.
        
        Args:
            key: Normalized value to search for
        
        Returns:
            Set of document IDs (empty set if not found)
        
        Time complexity: O(log n)
        """
        result = self._search_recursive(self.root, key)
        return result if result is not None else set()
    
    def _range_query_recursive(
        self, 
        node: Optional[AVLNode], 
        low: str, 
        high: str, 
        result: Set[str]
    ) -> None:
        """
        Recursively collect all doc_ids for keys in range [low, high].
        
        Time complexity: O(log n + k) where k is number of results
        """
        if node is None:
            return
        
        # If current node's key is in range, add its doc_ids
        if low <= node.key <= high:
            result.update(node.doc_ids)
        
        # Traverse left subtree if needed
        if low < node.key:
            self._range_query_recursive(node.left, low, high, result)
        
        # Traverse right subtree if needed
        if high > node.key:
            self._range_query_recursive(node.right, low, high, result)
    
    def range_query(self, low: str, high: str) -> Set[str]:
        """
        Find all document IDs for keys in the range [low, high] (inclusive).
        
        Args:
            low: Lower bound (inclusive)
            high: Upper bound (inclusive)
        
        Returns:
            Set of document IDs
        
        Time complexity: O(log n + k) where k is number of results
        """
        result: Set[str] = set()
        self._range_query_recursive(self.root, low, high, result)
        return result
    
    def _inorder_recursive(
        self, 
        node: Optional[AVLNode], 
        result: List[Tuple[str, List[str]]]
    ) -> None:
        """
        Recursively collect items in inorder traversal.
        
        Time complexity: O(n)
        """
        if node is None:
            return
        
        self._inorder_recursive(node.left, result)
        result.append((node.key, sorted(list(node.doc_ids))))
        self._inorder_recursive(node.right, result)
    
    def inorder_items(self) -> List[Tuple[str, List[str]]]:
        """
        Get all items in sorted order (inorder traversal).
        
        Returns:
            List of (key, doc_ids_list) tuples, sorted by key
        
        Time complexity: O(n)
        """
        result: List[Tuple[str, List[str]]] = []
        self._inorder_recursive(self.root, result)
        return result
    
    def size(self) -> int:
        """Get number of unique keys in the tree."""
        return self._size
