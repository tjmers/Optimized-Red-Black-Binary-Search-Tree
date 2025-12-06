# Optimized Red-Black Binary Search Tree

Optimal red-black binary search tree implementation

## Usage

## Invarients
1. Every node is either red or black
2. The root is always black
3. All nil leaves are black
4. If a node is red, both of its children are black
5. For each node, all paths from it to its nil leaves contain the same number of black nodes

## Asymptotic Time Complexities

### It will first be established that the height of the tree $\in \Theta(log(n))$



 - Define black-height (bh(x)): Number of black nodes from node x (excluding x) to any nil leaf node.

#### Part 1: Lower bound on subtree size

**Claim:** For any node x, the subtree rooted at x contains at least $2^{bh(x)} - 1$ internal nodes.

**Proof by induction:**

**Base case:** x is nil

$$
bh(nil) = 0
$$

$$
Size = 0 >= 2^0 - 1 = 0
$$

**Inductive Step:** Assume the claim holds true for both the left and right subtrees of x

Let:
- $n(x)$ be the number of internal nodes in the subtree rooted at x
- $n(L) = size of the left subtree
- $n(R) = size of the right subtree

By definition of size, $n(x) = n(L) + n(R) + 1$

By inductive hypothesis:

$$
N(L) >= 2^{bh(L)} - 1
$$

$$
n(R) >= 2^{bh(R)} - 1
$$

**Case 1:** x is black

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

**Case 2:** x is red

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

**In both cases, $n(x) >= 2^{bh(x)} - 1$.

#### Part 2: Relate black-height to height

**Claim:** For any node x, $height(x) <= 2 * bh(x)$

**Proof:**
- Consider any path from x to a nil leaf
- By property 4, every red node has 2 black children
- Therefore, on any path, at least half of the nodes must be black
- The worst case here (least black-height) is when the nodes alternate black-red-black-red....
- So if there are $k$ black nodes on a path, there are at most $k - 1$ red nodes
- Total nodes on that path are $k + (k - 1) = 2k - 1$
- Since $bh(x) = k$, $height(x) <= 2 * bh(x)$

#### Part 3: Upper bound on height

Let n be the number of internal nodes in the entire tree, and let h be its height.

From part 1 with x == root

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

That is, in the worst case, Searching $\in \Theta(h)$

Without knowing any information about the elements that are begin searched for, it isn't possible to come up with an average case that is better than $\Theta(h)$

Best case is when the element is the root, in which case Searching $\in \Theta(1)$

But in the worst case and on average, since $h \in \Theta(log(n))$

$$
Searching \in \Theta(log(n))
$$

### Inserting Time Complexity:




## Optimizations