# Optimized Red-Black Binary Search Tree

Optimal red-black binary search tree implementation


## Common Operations
1. Insertion
2. Deletion
3. Searching

## Asymptotic Time Complexities

### It will first be established that the height of the tree $\in \Omega(log(n))$



 - Recall that each red node must have two black children. 
 - Also recall that each path from a node to it's nil children must have the same number of black nodes.
 - Let the black height of a red-black binary search tree be the number of black nodes in any path from the root to the nil nodes
 - Let bh(x) be the black height of a red-black BST rooted at x.
 - Let size(x) be the number of nodes in the tree rooted at x.

Using the above properties, we get $2 * bh(x) >= height(x)$

Now, I claim that $size(x) >= 2 ^(bh(x)) - 1$ for any tree subrooted at x.
I will prove this claim by induction

Base case: Empty tree

$$
0 >= 2(0) - 1
0 >= 0
$$

Inductive Step:
Let that the left and right subtrees of x follow the property that $size(x) >= 2 ^ (bh(x)) - 1$

$$
size(x) = size(x.left) + size(x.right) + 1
size(x) <= 2 ^ (bh(x.left)) - 1 + 2 ^ (bh(x.right)) - 1 + 1
$$

The black height of a child is less than or equal to the black height of the root

$$
size(x) <= 2 ^ (bh(x)) - 1 + 2 ^ (bh(x)) - 1 + 1
size(x) <= 2 * 2 ^ (bh(x)) - 1
$$

Substitute $size >= 2 ^ (bh(x)) - 1$ from the original statement

$$
2 * 2 ^ (bh(x)) - 1 >= 2 ^ (bh(x)) + 1
$$

This is true since $bh(x) >= 1 => 2 ^ (bh(x)) >= 2$, therefore, $size(x) >= 2 ^ (bh(x)) - 1$ **is true**

Let $size(x) = n$

As shown above, $2 * bh(x) >= height(x)$

$$
n >= 2 ^ (bh(x)) - 1
n - 1 >= 2 ^ bh(x)
log(n - 1) >= bh(x)
bh(x) <= log(n - 1)
height(x) / 2 <= log(n - 1)
height(x) \in O(log(n))
$$




## Optimizations