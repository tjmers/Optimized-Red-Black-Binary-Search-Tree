# Optimized Red-Black Binary Search Tree

Optimal red-black binary search tree implementation.

## Usage

To get the red-black tree into your project, simply copy the `include/red_black_tree.h` file and make sure you include it wherever necessary.

The red-black tree implementation is designed to be extremely flexible to fit the needs of any project

When instantiating a template of the red-black tree, the following paremeters are used:
1. (required) T - the data type held by the tree.
2. (optional) duplicates - whether the tree should be allowed to hold duplicate elements (false by default).
3. (optional) Comparator - comparator class to compare elements in the tree.
4. (optional) Equal - equal class to determine of two elements in the tree are equal.
5. (optional) Node - the type of node that should be used with the tree.
6. (optional) Allocator - the allocator to allocate memory for the tree nodes.

The main operations are

`template <typename U> bool insert(U)` - Inserts the given element into the tree.

`bool erase(T)` - Removes the given element from the tree.

`bool contains(T)` - Determines if the given element is in the tree.

Other operations that should be used include:

`void reserve_additional(std::size_t)` - Pre-allocates memory for additional nodes. Useful to avoid large numbers of heap allocations.

`std::size_t shrink_to_fit()` - This is the only way to release memory throughout the lifetime of the tree. Otherwise, nodes are only deleted in the destructor / when begin copied / moved into.

The copy/move constructors/assignment operators all function as intended, and the STL-style iterator allows for easy in-order traversal.


## Invarients
1. Every node is either red or black.
2. The root is always black.
3. All nil leaves are black.
4. If a node is red, both of its children are black.
5. For each node, all paths from it to its nil leaves contain the same number of black nodes.

## Asymptotic Time Complexities

### It will first be established that the height of the tree $\in \Theta(log(n))$

 - Define black-height (bh(x)): Number of black nodes from node x (excluding x) to any nil leaf node.

#### Part 1: Lower bound on subtree size

**Claim:** For any node x, the subtree rooted at x contains at least $2^{bh(x)} - 1$ internal nodes.

**Proof by induction:**

**Base case:** x is nil.

$$
bh(nil) = 0
$$

$$
Size = 0 >= 2^0 - 1 = 0
$$

**Inductive Step:** Assume the claim holds true for both the left and right subtrees of x.

Let:
- $n(x)$ be the number of internal nodes in the subtree rooted at x.
- $n(L)$ = size of the left subtree.
- $n(R)$ = size of the right subtree.

By definition of size, $n(x) = n(L) + n(R) + 1$

By inductive hypothesis:

$$
N(L) >= 2^{bh(L)} - 1
$$

$$
n(R) >= 2^{bh(R)} - 1
$$

**Case 1:** x is black.

$$
bh(L) = bh(R) = bh(x) - 1
$$

$$
n(x) >= (2^{bh(x) - 1} - 1) + (2^{bh(x) - 1} - 1) + 1
$$

$$
= 2 * 2^{bh(x) - 1} - 2 + 1
$$

$$
= 2^{bh(x)} - 1
$$

**Case 2:** x is red.

$$
bh(L) = bh(R) = bh(x)
$$

$$
n(x) >= (2^{bh(x)} - 1) + (2^{bh(x)} - 1) + 1
$$

$$
= 2^{bh(x) + 1} - 1
$$

Now use $2^{bh(x) + 1} - 1 > 2^{bh(x) - 1}$

$$
    >= 2^{bh(x)} - 1
$$

**In both cases, $n(x) >= 2^{bh(x)} - 1$**.

#### Part 2: Relate black-height to height

**Claim:** For any node x, $height(x) <= 2 * bh(x)$.

**Proof:**
- Consider any path from x to a nil leaf.
- By property 4, every red node has 2 black children.
- Therefore, on any path, at least half of the nodes must be black.
- The worst case here (least black-height) is when the nodes alternate black-red-black-red....
- So if there are $k$ black nodes on a path, there are at most $k - 1$ red nodes.
- Total nodes on that path are $k + (k - 1) = 2k - 1$.
- Since $bh(x) = k$, $height(x) <= 2 * bh(x)$.

#### Part 3: Upper bound on height

Let n be the number of internal nodes in the entire tree, and let h be its height.

From part 1 with x == root,

$$
n >= 2^{bh(root)} - 1
$$

$$
2^{bh(root)} <= n + 1
$$

$$
bh(root) <= log(n + 1)
$$

From part 2:

$$
h <= 2 * bh(root) <= 2 * log(n + 1)
$$

$$
\therefore h \in O(log(n))
$$

#### Part 4: Lower bound on height

Since the red-black tree is a binary search tree,

$$
n <= 2^{h + 1} - 1
$$

$$
2^{h + 1} >= n + 1
$$

$$
h >= log(n + 1) - 1
$$

$$
\therefore h \in \Omega(log(n))
$$

#### Conclusion:

Since $h \in O(log(n))$ and $h \in \Omega(log(n))$,

$$
h \in \Theta(log(n))
$$

### Searching Time Compexity: 

In the worst case, the element in not present in the tree.

That is, in the worst case, Searching $\in \Theta(h)$.

Without knowing any information about the elements that are begin searched for, it isn't possible to come up with an average case that is better than $\Theta(h)$.

Best case is when the element is the root, in which case Searching $\in \Theta(1)$.

But in the worst case and on average, since $h \in \Theta(log(n))$.

$$
Searching \in \Theta(log(n))
$$

### Inserting Time Complexity:

Insertion has three parts:
1. Find where the element should be inserted.
2. Insert the element.
3. Fix any invariant violations that may occur.

#### Step 1:

This is simply the worst case for searching, $\in \Theta(log(n))$

#### Step 2:

This step is simply constructing the node and updating pointers, $\in \Theta(1)$

#### Step 3:

This is an iterative step.

In the worst case, fixups are needed going all the way up frmo the inserted node to the root.

Since each fixup is individually $\in \Theta(1)$, there are at most $h$ fixups, since each fixup moves up the tree by at least one node.

So insertion fixups $\in \Theta(h) = \Theta(log(n))$.

Since this is the same as part 3, there is no need to explore average and worst cases.


$$
\therefore Insertion \in \Theta(log(n)) + \Theta(1) + \Theta(log(n)) = \Theta(log(n))
$$

#### Deletion Time Complexity:

Similar to insertion, deletion has 3 steps:
1. Find the node to delete.
2. Delete the node (normal BST).
3. Fix any invarient violations.

#### Step 1:

This is the same as searching for a node, $\in \Theta(log(n))$.

#### Step 2:

In the case that the node being deleted has no left and/or right child, this operation $\in \Theta(1)$, simply swapping around pointers.

When the node begin deleted has two children, the minimum of the right subtree must be found (the next inorder successor).

That operation, is $\in O(h) = O(log(n))$.

#### Step 3:

Similar to the worst case for the insertion fixup, the worst case for fixing invarient violations caused by element deletion results in a series of $\Theta(1)$ operations travelling all the way up the tree with height $h = log(n)$, making this step also $\in \Theta(log(n))$

$$
\therefore Deletion \in \Theta(log(n)) + O(log(n)) + \Theta(log(n)) = \Theta(log(n))
$$

### Comparison to other types of binary search trees

#### AVL Trees:

Due to the invarient that all balances factors <= 1, the AVL tree also has $height \in \Theta(log(n))$. This makes many operations in the AVL tree the same asymptotically as the red-black BST.

Searching: Since $h \in \Theta(log(n))$, this operation is $\in \Theta(log(n))$, since in the worst case when the element is not present all nodes must be traversed.

Insertion: AVL Trees have the same worst case insertion of $\Theta(log(n))$ as red-black trees, due to the limitation of traversing at $\Theta(log(n))$.

Deletion: AVL Trees have the same worst case deletion as the red-black trees, which is $\Theta(log(n))$, again dictated by the height of the tree.

#### Non-self-balancing binary search trees

This category of trees most importantly do not have any invariants that limit the height, leading the height to have a worst case of $\Theta(n)$, when the tree is in a linked-list fasion. When the data inserted is randomly generated, the average height is $\in \Theta(log(n))$, which is the same as the AVL and red-black trees. The complete breakdown is shown below:

Searching:

With a worst case height of $h \in \Theta(n)$, the worst case for searching $\in \Theta(n)$.

The average case height of $\Theta(log(n))$ keeps the average case for searching $\in \Theta(log(n))$.

Insertion:

The worst case still needs to traverse the tree to find where to insert the new element. Because of this, insertion has a worst case $\in \Theta(n)$.

In the average case, the tree is somewhat balanced, and the height $\in \Theta(log(n))$ makes the average case for insertion $\in \Theta(log(n))$

Deletion:

The worst case for deletion needs first find the node, which again has a worst case $\in \Theta(n)$ for when the tree is in a linked-list style

The average case, since height $\in \Theta(log(n))$, is $\in \Theta(log(n))$



## Optimizations

The greatest optimization came from how memory was allocated throughout the tree. When the first prototype of the class was created, the MSVC performance profiler was used and it was noticed that most of the time was spent allocating memory. In order to fix this, a system was created so that memory was allocated in larger chunks, rather than one for each inserted node. By default, the tree allocates memory for 32 nodes, then once all is used another 32. Deletion, instead of deallocating memory, simply moves the nodes to the "free" part of the memory (oversimplification - see `red_black_tree.h` for actual implementation) Additionally, a feature was added so that a larger chunks of memory could be allocated by request of the user. 

When the pre-allocation feature was used, it was observed that for inserting 10,000 elements, the MSVC's performance profiler (at 1000 samples / second) went from on average 9-15 profiles to 0-1 profiles - a speadup of over 5x.


## Additional notes:

It could be argued that the non-const iterator should be removed, since modifying the elements would very likely break the invarient of a normal BST.

Potential optimizations:

- Change the memory management system to work off of a fixed-size stack-allocated array, rather than the heap-allocated std::vector. Performance impact would have to be benchmarked since there really shouldn't be a lot of inserting / deleting from the std::vector anyways.

- Get rid of the `used` member variable from `MemoryBlock` and move it to be a member variable of `RedBlackTree`. This is possible since `memory_ (std::vector<MemoryBlock>)` holds the invarient that all `MemoryBlock`s before `memory_[active_block_]` are fully used and those after are completely empty. This would likely not impact performance very much, but could save memory if many elements are added without reserving the space.

- Add a function `compress`, which deallocates most of the current nodes, and re-allocates them in larger chunks, resulting in a lesser `memory_.size()`. This function should also have asynchrous support since it would be expensive compared to the other operations ($\in \Theta(n)$).

- Remove my invarient that nil's parent is nil. 