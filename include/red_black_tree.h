#pragma once

// Use C++ STL
#include <concepts>
#include <iostream>
#include <memory>
#include <stack>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef __RED_BLACK_TEST__
#include <cassert>
#endif // __RED_BLACK_TEST__

// Do not populate default namespace
namespace red_black_tree_utility {


// Color alias
// Not an enum so the user can more easily use their own node for the tree if desired
using Color = bool;
inline constexpr Color kBlack = false;
inline constexpr Color kRed = true;

// Node with a parent and color
template <typename T>
struct ColoredTreeNodeWithParent {

    using data_type = T;

    ColoredTreeNodeWithParent<T>* left;
    ColoredTreeNodeWithParent<T>* right;
    ColoredTreeNodeWithParent<T>* parent;
    T val;
    Color color;

    // Constructor
    template <typename U>
    ColoredTreeNodeWithParent(ColoredTreeNodeWithParent* l, ColoredTreeNodeWithParent* r, ColoredTreeNodeWithParent* p, U&& v, Color c) : left(l), right(r), parent(p), val(std::forward<U>(v)), color(c) {}

    // Default construct which garbage initializes the data structure
    ColoredTreeNodeWithParent() {}
};

// Concepts to give more detailed error messages if bad template parameters are used

// Node is not a template template parameter since it is okay for specific nodes to be used as long as they have all requirements
template <typename Node, typename T>
concept ValidRedBlackTreeNode = requires(Node node) {
    { node.left } -> std::convertible_to<Node*>;
    { node.right } -> std::convertible_to<Node*>;
    { node.parent} -> std::convertible_to<Node*>;
    { node.val } -> std::convertible_to<T>;
    { node.color } -> std::convertible_to<bool>;
} && std::is_same_v<typename Node::data_type, T>;

template <typename Alloc, typename Datatype>
concept ValidAllocator = requires(Alloc a, Datatype* data, std::size_t n) {
    { a.allocate(n) } -> std::convertible_to<Datatype*>;
    { a.deallocate(data, n) };
};

template <typename Obj, typename T>
concept ValidBoolOperator = requires(Obj o, T a, T b) {
    { o(a, b) } -> std::convertible_to<bool>;
};

/// @brief Data structure represent the Red-Black Binary Search Tree with insertion, deletion, and traversal operations.
/// @tparam T exact datatype held by the tree.
/// @tparam duplicates whether the tree should allow duplicate elements.
/// @tparam Comparator comparator to compare two instances of `T`. Class should have an operator(const T& o1, const T& o2) returning a boolean true when o1 < o2 and false when o1 > o2.
/// @tparam Equal class with operator(const T& o1, const T& o2) that returns true iff o1 == o2.
/// @tparam Node template node class to use for `this`. The bool color should be false when black and true when red.
/// @tparam Allocator allocator class to allocate memory.
template <typename T, bool duplicates = false, class Comparator = std::less<T>, class Equal = std::equal_to<T>, class Node = ColoredTreeNodeWithParent<T>, class Allocator = std::allocator<Node>>
requires ValidRedBlackTreeNode<Node, T> && ValidBoolOperator<Comparator, T> && ValidBoolOperator<Equal, T> && ValidAllocator<Allocator, Node>
class RedBlackTree {

private:

    // Root of the tree
    Node* root_;
    // Sentienel NIL node
    Node* nil_;
    std::size_t size_;
    Comparator comparator_;
    Equal equal_;
    Allocator allocator_;

    // Cache minimum and maximums (especially important for O(1) begin())
    Node* minimum_;
    Node* maximum_;

public:

    // Define types for others to use (STL style)

    using value_type = typename Node::data_type;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
    using node_type = Node;

    

    // -------------------------- Constructors --------------------------


    /// @brief Constructs an empty Red-Black Tree
    RedBlackTree() : size_(0), comparator_(), equal_(), allocator_(), available_(), n_available_() {
        // Cannot be allocated in the constructor because of the order of object construction
        nil_ = allocator_.allocate(1);
        nil_->color = kBlack;
        nil_->left = nil_;
        nil_->right = nil_;
        nil_->parent = nil_;
        root_ = nil_;
        maximum_ = nil_;
        minimum_ = nil_;
    }

    /// @brief Copy constructor. Deep copies `other` into `this`.
    /// @param other the other tree to copy
    RedBlackTree(const RedBlackTree& other) : root_(nullptr),
                                              nil_(nullptr),
                                              size_(other.size_),
                                              comparator_(other.comparator_),
                                              equal_(other.equal_),
                                              allocator_(other.allocator_),
                                              minimum_(nullptr),
                                              maximum_(nullptr) {
        root_ = deep_copy(other.root_, allocator_);
        nil_ = root_->parent;
        minimum_ = minimum(root_, nil_);
        maximum_ = maximum(root_, nil_);
    }


    /// @brief Move constructor.
    /// @param other the other tree to be moved
    RedBlackTree(RedBlackTree&& other) noexcept : root_(other.root_),
                                                  nil_(other.nil_),
                                                  size_(other.size_),
                                                  comparator_(std::move(other.comparator_)),
                                                  equal_(std::move(other.equal_)),
                                                  allocator_(std::move(other.allocator_)),
                                                  minimum_(other.minimum_),
                                                  maximum_(other.maximum_) {

        // Leave other in a valid but unspecified state
        other.nil_ = nullptr;
        other.root_ = nullptr;
    }

    /// @brief Copy assignment operator. Deep copies `other` into `this`.
    /// @param other tree to copy from.
    /// @return `this`
    RedBlackTree& operator=(const RedBlackTree& other) {
        // Clean up existing tree
        // Checks are needed since the trees may have been moved from
        if (root_ && root_ != nil_) {
            deallocate(root_, allocator_, nil_);
        }
        if (nil_) {
            std::destroy_at(nil_);
            allocator_.deallocate(nil_, 1);
        }

        allocator_ = other.allocator_;
        root_ = deep_copy(other.root_, allocator_);
        nil_ = root_->parent;
        size_ = other.size_;
        comparator_ = other.comparator_;
        equal_ = other.equal_;
        minimum_ = minimum(root_, nil_);
        maximum_ = maximum(root_, nil_);

        return *this;
    }

    /// @brief Move assignement operator.
    /// @param other The tree to move from.
    /// @return `this`.
    RedBlackTree& operator=(RedBlackTree&& other) noexcept {
        // Clean up existing 
        // Checks are needed since the trees may have been moved from
        // Do not add to available since different allocators are used
        if (root_ && root_ != nil_) {
            deallocate(root_, allocator_, nil_);
        }

        if (nil_) {
            std::destroy_at(nil_);
            allocator_.deallocate(nil_, 1);
        }

        root_ = other.root_;
        other.root_ = nullptr;
        nil_ = other.nil_;
        other.nil_ = nullptr;
        size_ = other.size_;
        comparator_ = std::move(other.comparator_);
        equal_ = std::move(other.equal_);
        allocator_ = std::move(other.allocator_);
        minimum_ = other.minimum_;
        maximum_ = other.maximum_;


        return *this;
    }

    // Constructor that inserts a list of elements
    template <typename... Args>
    explicit RedBlackTree(Args... args) : RedBlackTree() {
        // Use fold expression
        // It just forwards each arg to insert
        (insert(std::forward<Args>(args)), ...);
    }

    // -------------------------- Insertion --------------------------


    /// @brief Inserts `element` into `this`.
    /// @tparam U the type of the element being inserted.
    /// @param element The element to add.
    /// @return true if the element was added and the violations were fixed false otherwise.
    /// @note If `!duplicates`, this function will always return true when returning normally.
    template <typename U>
    bool insert(U&& element) {

        // Always allocate memory regardless of if the insertion fails
        // This is necessary so that in equal_ and comparator_ the object does not get constructed each time
        Node* new_node = allocator_.allocate(1);
        new (new_node) Node(nil_, nil_, nil_, std::forward<U>(element), kRed);

        if (root_ == nil_) {
            // Root is always black
            new_node->color = kBlack;
            root_ = new_node;
            ++size_;
            minimum_ = new_node;
            maximum_ = new_node;
            return true;
        }

        // Insert where it belongs
        Node* current = root_;
        Node* prev = nil_;
        while (current != nil_) {
            // Do not add duplicates if specified
            if constexpr (!duplicates) {
                if (equal_(current->val, new_node->val)) {
                    std::destroy_at(new_node);
                    allocator_.deallocate(new_node, 1);
                    return false;
                }
            }

            prev = current;
            if (comparator_(new_node->val, current->val)) {
                // Go left
                current = current->left;
            } else {
                current = current->right;
            }
        }

        // Set the new nodes parent
        new_node->parent = prev;

        // Put node where it belongs
        if (comparator_(new_node->val, prev->val)) {
            prev->left = new_node;
        } else {
            prev->right = new_node;
        }

        if (new_node->parent->color == kRed) {
            // Violation has occured
            insert_fixup(new_node);
        }

        // Updates min and max
        if (comparator_(new_node->val, minimum_->val)) {
            minimum_ = new_node;
        }
        if (comparator_(maximum_->val, new_node->val)) {
            maximum_ = new_node;
        }
        ++size_;
        return true;
    }

private:

    /// @brief Finds the sibling of `node`.
    /// @param node The node to find the sibling of.
    /// @return Node->parent->left if node == node->parent->right else node->parent->left
    static Node* sibling_of(Node* node) {
        return node->parent->left == node ? node->parent->right : node->parent->left;
    }

    /// @brief Fixes the violations created from inserting the node
    /// @param node node that has the violation that needs to be fixed
    void insert_fixup(Node* node) {
        // Determine which fixup is needed
        // While statement since multiple fixups may need to happen
        while (node->color == kRed && node->parent->color == kRed) {
            Node* uncle = sibling_of(node->parent);
            if (uncle->color == kRed) {
                node = insert_fixup_case_one(node);
            } else {
                // If there is not a straight line from node to grandparent
                if ((node->parent->left == node) != (node->parent->parent->left == node->parent)) {
                    node = insert_fixup_case_two(node);
                }
                node = insert_fixup_case_three(node);
            }
        }
        root_->color = kBlack;
    }

    /// @brief Fixes the case where z has a red uncle (recolor)
    /// @param node z
    /// @return the root of the fixed violation
    static Node* insert_fixup_case_one(Node* node) {
        Node* uncle = sibling_of(node->parent);
        node->parent->color = kBlack;
        uncle->color = kBlack;
        node = node->parent->parent;
        node->color = kRed;
        return node;
    }

    /// @brief "Fixes" the case where z has a black uncle and the insert isn't a straight line (needs a case three fixup after)
    /// @param node z
    /// @return the root of the fixed violation
    Node* insert_fixup_case_two(Node* node) {
        if (node == node->parent->left) {
            rotate_right(node->parent);
            return node->right;
        } else {
            rotate_left(node->parent);
            return node->left;
        }
    }

    /// @brief Fixes the case where z has a black uncle and the insert is a straight line
    /// @param node z
    /// @return the root of the fixed violation
    Node* insert_fixup_case_three(Node* node) {
        Node* rotation_root;
        if (node == node->parent->left) {
            rotation_root = rotate_right(node->parent->parent);
        } else {
            rotation_root = rotate_left(node->parent->parent);
        }
        rotation_root->color = kBlack;
        rotation_root->left->color = kRed;
        rotation_root->right->color = kRed;
        return rotation_root;
    }

    /// @brief Helper function to rotate left (counter-clockwise) around a tree node.
    /// @param node node to rotate around
    /// @return the root of the rotation
    Node* rotate_left(Node* node) {
        Node* old_parent = node->parent;
        Node* new_head = node->right;
        Node* moving_part = new_head->left;

        node->right = moving_part;
        if (moving_part != nil_)
            moving_part->parent = node;

        new_head->left = node;
        node->parent = new_head;
        
        new_head->parent = old_parent;
        if (old_parent == nil_) {
            root_ = new_head;
        } else {
            (old_parent->left == node ? old_parent->left : old_parent->right) = new_head;
        }
        
        return new_head;
    }

    /// @brief Helper function to rotate right (clockwise) around a tree node.
    /// @param node node to rotate around
    /// @return the root of the rotation
    Node* rotate_right(Node* node) {

        Node* old_parent = node->parent;
        Node* new_head = node->left;
        Node* moving_part = new_head->right;

        node->left = moving_part;
        if (moving_part != nil_)
            moving_part->parent = node;

        new_head->right = node;
        node->parent = new_head;
        
        new_head->parent = old_parent;
        if (old_parent == nil_) {
            root_ = new_head;
        } else {
            (old_parent->left == node ? old_parent->left : old_parent->right) = new_head;
        }

        // std::cout << "Done rotating right\n";
        return new_head;
    }

private:

    // -------------------------- Deletion --------------------------

    /// @brief Transplants node v into node u's place -- only changing the parent / root connections
    /// @param u "Old node"
    /// @param v "New node"
    /// @note updating any children of `u` or `v` is the responsibility of the callee
    void transplant(Node* u, Node* v) {
        if (u == root_) {
            root_ = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }

        v->parent = u->parent;
    }

public:

    /// @brief Removes an arbitrary occurence of `val` from `this`, if present
    /// @param val the value to erase
    /// @return true if the object was present and delted
    /// @note erasing a value invalidates only the iterator of that element
    bool erase(const_reference val) {
        // Find the node to be deleted

        Node* current = root_;
        while (current != nil_ && !equal_(current->val, val))
            current = (comparator_(val, current->val)) ? current->left : current->right;

        // Node was not found
        if (current == nil_) return false;

        Color original_color = current->color;
        Node* replacement;

        if (current->left == nil_) {
            // Case 1: Current has no left child
            // Solution: Replace with the right child
            // Still works if right child is nil
            replacement = current->right;
            
            transplant(current, replacement);

        } else if (current->right == nil_) {
            // Case 2: Current has no right child
            // Solution: Replace with the left child
            replacement = current->left;
            transplant(current, replacement);
        } else {
            // Case 3: current has both left and right children
            // Solution: Replace with the next element inorder (minimum of right subtree)
            // Cannot use the minimum member variable here since current->right is a subtree
            replacement = minimum(current->right, nil_);
            original_color = replacement->color;
            Node* replacement_right = replacement->right;
            // Handle case where next inorder is a child of this
            if (replacement->parent == current) {
                replacement_right->parent = replacement;
            } else {
                // Move subchildren of replacement node
                transplant(replacement, replacement_right);
                replacement->right = current->right;
                replacement->right->parent = replacement;
            }
            // Replace the node
            transplant(current, replacement);
            replacement->left = current->left;
            replacement->left->parent = replacement;
            replacement->color = current->color;

            // Fixup happens on the replacement_right
            replacement = replacement_right;
        }

        // Call destructor
        std::destroy_at(current);

        // Deallocate
        allocator_.deallocate(current, 1);

        // Fixup if necessary
        if (original_color == kBlack) {
            delete_fixup(replacement);
        }

        --size_;
        // Reset nil's parent in case it was temporarily updated
        nil_->parent = nil_;

        // Successful deletion
        return true;
    }

    /// @brief Fixes violations that come from standard BST deletion
    /// @param node where the fix is needed
    void delete_fixup(Node* node) {
        while (node != root_ && node->color == kBlack) {
            if (node == node->parent->left) {
                Node* sibling = node->parent->right;
                
                if (sibling->color == kRed) {
                    // Case 1: Sibling is red
                    sibling->color = kBlack;
                    node->parent->color = kRed;
                    rotate_left(node->parent);
                    sibling = node->parent->right;
                }
                
                if (sibling->left->color == kBlack && sibling->right->color == kBlack) {
                    // Case 2: Both of sibling's children are black
                    sibling->color = kRed;
                    node = node->parent;
                } else {
                    if (sibling->right->color == kBlack) {
                        // Case 3: Sibling's right child is black, left is red
                        sibling->left->color = kBlack;
                        sibling->color = kRed;
                        rotate_right(sibling);
                        sibling = node->parent->right;
                    }
                    // Case 4: Sibling's right child is red
                    sibling->color = node->parent->color;
                    node->parent->color = kBlack;
                    sibling->right->color = kBlack;
                    rotate_left(node->parent);
                    node = root_;
                }
            } else {
                // Mirror cases (node is right child)
                Node* sibling = node->parent->left;
                
                if (sibling->color == kRed) {
                    // Case 1: Sibling is red
                    sibling->color = kBlack;
                    node->parent->color = kRed;
                    rotate_right(node->parent);
                    sibling = node->parent->left;
                }
                
                if (sibling->left->color == kBlack && sibling->right->color == kBlack) {
                    // Case 2: Both of sibling's children are black
                    sibling->color = kRed;
                    node = node->parent;
                } else {
                    if (sibling->left->color == kBlack) {
                        // Case 3: Sibling's left child is black, right is red
                        sibling->right->color = kBlack;
                        sibling->color = kRed;
                        rotate_left(sibling);
                        sibling = node->parent->left;
                    }
                    // Case 4: Sibling's left child is red
                    sibling->color = node->parent->color;
                    node->parent->color = kBlack;
                    sibling->left->color = kBlack;
                    rotate_right(node->parent);
                    node = root_;
                }
            }
        }
        node->color = kBlack;
    }

    
public:

    // -------------------------- Searching --------------------------
    
    /// @brief Determines if `element` is in `this`
    /// @param element the object to search for
    /// @return true if `element` is in `this` false otherwise
    [[nodiscard("Does nothing")]] bool contains(const_reference element) const noexcept(noexcept(equal_(element, root_->val)) && noexcept(comparator_(element, root_->val))) {
        Node* current = root_;
        while (current != nil_) {
            if (equal_(current->val, element)) {
                return true;
            }

            if (comparator_(element, current->val)) {
                current = current->left;
            } else {
                current = current->right;
            }
        }
        return false;
    }

    /// @brief Gets the element if present
    /// @param element the element to do the lookup for
    /// @return The pointer to the element if present otherwise nullptr
    /// @warning If the element returns is modified, the tree will have undefined behavior.
    [[nodiscard("Does nothing")]] const pointer get(const_reference element) const noexcept(noexcept(equal_(element, root_->val)) && noexcept(comparator_(element, root_->val))) {
        Node* current = root_;
        while (current != nil_ && !equal_(current->val, element)) {
            current = comparator_(element, current->val) ? current->left : current->right;
        }
        return current == nil_ ? nullptr : std::addressof(current->val);
    }

    
public:

    /// @brief Gets the root of `this`.
    /// @return Root of `this`.
    inline Node* root() const noexcept {
        return root_;
    }

    /// @brief Gets |this|.
    /// @return |this|.
    [[nodiscard("Does nothing")]] inline std::size_t size() const noexcept {
        return size_;
    }

    /// @brief Determines if |this| == 0
    /// @return |this| == 0
    [[nodiscard("Does nothing")]] inline bool empty() const noexcept {
        return size_ == 0;
    }

    /// @brief Gets the minimum value in `this`.
    /// @return The minimum value in `this`.
    const T& minimum_value() const noexcept {
        return minimum_->val;
    }

    /// @brief Gets the maximum value in `this`.
    /// @return The maximum value in `this`.
    const T& maximum_value() const noexcept {
        return maximum_->val;
    }

    /// @brief Destroys each element in `this` and deallocates the tree
    ~RedBlackTree() noexcept {
        // Deallocate each node
        destroy_cached_memory();
        
        // This is so that if this object is moved from it will not break
        // Using the allocator to deallocate nullptr is undefined
        if (root_ == nullptr) return;

        // Do not deallocate nil if the root is nil, the other statement will do it
        // Absence of the if statement makes nil_ get deallocated twice
        bool root_nil = root_ == nil_;
        deallocate(root_, allocator_, nil_);
        if (!root_nil) {
            // Do not std::destroy_at(nil_) because nil was never actually constructed
            allocator_.deallocate(nil_, 1);
        }
    }


    /// @brief Clears `this`. 
    void clear() {
        if (root_ == nil_) return;
        deallocate(root_, allocator_, nil_);
        root_ = nil_; 
        size_ = 0;
        // Reset in case its needed (probably not but should have negligable performance)
        allocator_ = Allocator();
        equal_ = Equal();
        comparator_ = Comparator();
        maximum_ = nil_;
        minimum_ = nil_;
    }


public:

    // -------------------------- Traversal --------------------------

    /// @brief In-Order iterator class for the tree.
    /// @note Insertion and deletion invalidates these.
    class iterator {
        Node* current_;
        const Node* nil_;

    public:

        using value_type = T;
        using reference = value_type&;
        using pointer = value_type*;

        // Since this operation is in O(log(n)) time, it is the users responsibility to cache the begin iterator for performence.
        iterator(Node* start, Node* nil) : current_(start), nil_(nil) {}
        iterator(const iterator& other) : current_(other.current_), nil_(other.nil_) {}
        
        reference operator*() const {
            return current_->val;
        }
        
        iterator& operator++() {
            if (current_ == nil_) return *this; // already at end

            if (current_->right != nil_) {
                // Go right once, then all the way left
                current_ = minimum(current_->right, nil_);
            } else {
                // Go up until we've come from a left child
                Node* p = current_->parent;
                while (p != nil_ && current_ == p->right) {
                    current_ = p;
                    p = p->parent;
                }
                current_ = p; // Will be nil if we're done
            }

            return *this;
        }
        
        bool operator==(iterator other) const {
            return current_ == other.current_ && nil_ == other.nil_;
        }
        
        bool operator!=(iterator other) const {
            return current_ != other.current_ && nil_ == other.nil_;
        }

        // Many common iterator operators are not implemented since they make no sense in these context or would lead to confusion
        
    };


    /// @brief Constant In-Order iterator class for the tree.
    /// @note Insertion and deletion invalidates these.
    class const_iterator {
        const Node* current_;
        const Node* nil_;

    public:

        using value_type = T;
        using reference = value_type&;
        using const_reference = const reference;
        using pointer = value_type*;

        // Since this operation is in O(log(n)) time, it is the users responsibility to cache the begin iterator for performence.
        const_iterator(Node* start, Node* nil) : current_(start), nil_(nil) {}
        const_iterator(const const_iterator& other) : current_(other.current_), nil_(other.nil_) {}
        
        const_reference operator*() const {
            return current_->val;
        }
        
        iterator& operator++() {
            if (current_ == nil_) return *this; // already at end

            if (current_->right != nil_) {
                // Go right once, then all the way left
                current_ = minimum(current_->right, nil_);
            } else {
                // Go up until we've come from a left child
                Node* p = current_->parent;
                while (p != nil_ && current_ == p->right) {
                    current_ = p;
                    p = p->parent;
                }
                current_ = p; // Will be nil if we're done
            }

            return *this;
        }
        
        bool operator==(iterator other) const {
            return current_ == other.current_ && nil_ == other.nil_;
        }
        
        bool operator!=(iterator other) const {
            return current_ != other.current_ && nil_ == other.nil_;
        }

        // Many common iterator operators are not implemented since they make no sense in these context or would lead to confusion
        
    };

    
    /// @brief The iterator to the beginning of the tree. 
    /// @return Iterator to the beginning of the tree.
    iterator begin() { return iterator(minimum_, nil_); }

    /// @brief The iterator to the end of the tree. 
    /// @return Iterator to the end of the tree.
    iterator end() { return iterator(nil_, nil_); }

    /// @brief The constant iterator to the beginning of the tree. 
    /// @return Constant iterator to the beginning of the tree.
    const_iterator begin() const { return const_iterator(minimum_, nil_); }


    /// @brief The constant iterator to the end of the tree. 
    /// @return Constant iterator to the end of the tree.
    const_iterator end() const { return const_iterator(nil_, nil_); }



    // -------------------------- Debugging --------------------------

#ifdef __RED_BLACK_TEST__
public:
    // Prints the tree
    // (Used for debugging)
    void pretty_print() const {
        
        // Put these in strings to make stdout make it that color
        // Ex: "\033[31mThis text is red\033[0mNow this text is white"
        // Would print This text is redNow this text is white
        // First half is red second half is white
        static constexpr std::string_view red = "\033[31m";
        static constexpr std::string_view normal = "\033[0m";

        int h = height(root_, nil_);
        // Assume each node takes 2 spots on the console
        int total_nodes = 1;
        for (int i = 0; i < h; ++i) {
            total_nodes <<= 1;
        }
        int total_spots = total_nodes * 4 - 4;

        std::vector<Node*> current_level;
        current_level.push_back(root_);
        bool non_null = true;
        while (non_null) {
            int n_intervals = current_level.size();
            int interval_spacing = total_spots / n_intervals;
            std::string padding;

            for (int i = 0; i < interval_spacing / 2; ++i) {
                std::cout << ' ';
            }

            for (int i = 0; i < interval_spacing; ++i) {
                padding.push_back(' ');
            }
            std::vector<Node*> next_level;
            non_null = false;

            for (Node* n : current_level) {
                if (n == nil_) {
                    std::cout << "NL";
                    next_level.push_back(nil_);
                    next_level.push_back(nil_);
                    std::cout << padding;
                    continue;
                }
                if (n->color == kRed) {
                    std::cout << red;
                }
                std::cout << n->val;
                if (n->color == kRed) {
                    std::cout << normal;
                } 
                non_null = true;
                next_level.push_back(n->left);
                next_level.push_back(n->right);
                std::cout << padding;
            }

            std::cout << '\n';

            current_level = std::move(next_level);
        }
    }

    /// @brief Helper debug function to print information about `this`.
    void print_info() const {
        std::cout << "Size: " << size_ << '\n';
        std::cout << "Root: " << root_->val << '\n';
    }

    // Test case helpers

    /// @brief Makes sure that all invariants of the red black tree are held
    void check_invariants() const {

        // Verify that the NIL node is still valid
        assert(nil_);
        assert(nil_->left == nil_);
        assert(nil_->right == nil_);
        assert(nil_->parent == nil_);
        assert(nil_->color == kBlack);

        // Root is always black
        assert(root_);
        assert(root_->color == kBlack);

        // All red nodes have black children
        check_no_red_red(root_);

        // All paths from one node to its nil leafs have the same number of black nodes
        check_black_height(root_);

        // Make sure each left node is less than node and each right node is greater
        check_bst(root_);
    }

private:

    /// @brief Helper function for `check_invariants` for ensuring that all red nodes have black children
    /// @param node the tree / subtree to verify
    void check_no_red_red(Node* node) const {
        assert(node);
        // Base case
        if (node == nil_) return;

        assert(!(node->color == kRed && (node->left->color == kRed || node->right->color == kRed)));
        check_no_red_red(node->left);
        check_no_red_red(node->right);
    }

    /// @brief Helper function for `check_invariants` for ensuring that each path has the same number of black nodes
    /// @param node the tree to verify
    /// @return the black black height of the tree rooted at `node` 
    int check_black_height(Node* node) const {
        assert(node);
        if (node == nil_) return 0;
        int left = check_black_height(node->left);
        int right = check_black_height(node->right);
        assert(left == right);
        return left + (node->color == kBlack);
    }

    void check_bst(Node* node) const {
        assert(node);
        if (node == nil_) return;
        if (node->left != nil_) {
            assert(comparator_(node->left->val, node->val));
            check_bst(node->right);
        }

        if (node->right != nil_) {
            assert(comparator_(node->val, node->right->val));
            check_bst(node->right);
        }
    }
    
#endif // __RED_BLACK_TEST__
};

// These are declared outside of the class since they do not need all template parameters and do not directly affect `this`


/// @brief Destroys and deallocates all nodes subrooted at `root` including `root`.
/// @tparam Node the type of node to be deallocated.
/// @tparam Allocator the allocated type used to deallocate the nodes.
/// @param root Root of the tree / subtree to be deallcoated.
/// @param alloc Allocator object used to deallocate the nodes.
template <typename Node, typename Allocator>
static void deallocate(Node* root, Allocator& alloc, const Node* nil = nullptr) noexcept {
    if (root->left != nil) {
        deallocate(root->left, alloc, nil);
    }
    if (root->right != nil) {
        deallocate(root->right, alloc, nil);
    }

    std::destroy_at(root);
    alloc.deallocate(root, 1);
}

/// @brief Finds the node with the minimum value of the BST rooted at `root`.
/// @tparam Node the type of node to find the minimum of.
/// @param root the root of the tree to find the minimum of.
template <typename Node>
requires ValidRedBlackTreeNode<Node, typename Node::data_type>
static Node* minimum(Node* root, const Node* nil = nullptr) {
    while (root->left != nil) root = root->left;
    return root;
}

/// @brief Finds the node with the maximum value of the BST rooted at `root`.
/// @tparam Node the type of node to find the maximum of.
/// @param root the root of the tree to find the maximum of.
template <typename Node>
requires ValidRedBlackTreeNode<Node, typename Node::data_type>
static Node* maximum(Node* root, const Node* nil = nullptr) {
    while (root->right != nil) root = root->right;
    return root;
}

template <typename Node>
static std::size_t height(const Node* root, const Node* nil = nullptr) {
    if (root == nil) return 0;
    return 1 + std::max(height(root->left, nil), height(root->right, nil));
}


/// @brief Creates a deep copy of the tree rooted at `root`, with a new nil node too.
/// @tparam Node the node type to make a copy of.
/// @param root The root of the tree that a copy is made of. Parent must be the nil node
/// @param alloc Allocator to allocate memory for the tree copy
/// @return The root of the tree copy.
template <typename Node, typename Allocator>
requires ValidRedBlackTreeNode<Node, typename Node::data_type> && ValidAllocator<Allocator, Node>
[[nodiscard("Dynamically allocated nodes must be deallocated")]]
static Node* deep_copy(Node* root, Allocator& alloc) {
    // Make sure that a new nil is used (could go wrong with multithreading when nil's parent is changed in deletion_fixup and when one object goes out of scope and nil_ is deallocated)
    Node* new_nil = alloc.allocate(1);
    new_nil->parent = new_nil;
    new_nil->left = new_nil;
    new_nil->right = new_nil;
    new_nil->color = kBlack;

    Node* new_root;
    if (root == root->parent) {
        // Root is nil
        new_root = new_nil;
        return new_root;
    }

    // Set up new root
    new_root = alloc.allocate(1);
    new (new_root) Node(nullptr, nullptr, new_nil, root->val, root->color);


    struct CopyStep {
        Node* old_node;
        Node* new_node;
    };


    std::stack<CopyStep> dfs;
    dfs.push({root, new_root});

    while (!dfs.empty()) {
        CopyStep current = dfs.top();
        dfs.pop();

        // Copy the children and add to the dfs if necessary

        if (current.old_node->left == root->parent) {
            // If the old node's left child was nil
            current.new_node->left = new_nil;
        } else {
            current.new_node->left = alloc.allocate(1);
            new (current.new_node->left) Node(nullptr, nullptr, current.new_node, current.old_node->left->val, current.old_node->left->color);
            dfs.push({current.old_node->left, current.new_node->left});
        }

        if (current.old_node->right == root->parent) {
            // The old node's right child was nil
            current.new_node->right = new_nil;
        } else {
            current.new_node->right = alloc.allocate(1);
            new (current.new_node->right) Node(nullptr, nullptr, current.new_node, current.old_node->right->val, current.old_node->right->color);
            dfs.push({current.old_node->right, current.new_node->right});
        }
    }

    return new_root;
}

} // namespace red_black_tree_utility

// Bring RedBlackTree into the default namespace
using red_black_tree_utility::RedBlackTree;