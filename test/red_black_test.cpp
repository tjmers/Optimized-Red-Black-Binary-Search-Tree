/**
 * Test cases for the Red Black Binary Search Tree
 * All test cases pass when compiled with g++ and MSVC, with the C++ version set to at least 20 (for concepts) on Windows 11
 * All profiling was done with MSVC performance profiler.
 * In terms of performance, the function that takes the most amount of time by far is the check_invariants function
 * This makes sense since it must traverse the entire tree is is invoked frequently.
 * Inside the insert function, the majority (93% of runtime) of the bottleneck for time comes from the memory allocation
 * Similarly for erasing, 97% of the runtime is spent in the allocator::deallocate function.
 * Since this program doesn't use a lot of memory, it is likely that no kernel functions are needed for extra memory
 * In that case, the bottleneck would be from the C++ STL memory allocations
 * Size of a red black node = 8 (left) + 8 (right) + 8 (parent) + 4 (int) + 1 (color) = 29 -> probably padded to 32 bytes on 64-bit systems
 * When 10,000 elements are inserted, 10,000 * 32 bytes = 312.5 kilo bytes allocated -> a significant amount
 * In order to make it more efficient, a system where memory is request in bigger chunks would be better
 * I tried to implement it but it really just turns into reinventing the heap and there would be a lot more overhead since nodes would have to be separeated based on how they're allocated
 * 
 */




#define __RED_BLACK_TEST__

#include "red_black_tree.h"
#include "tracker_wrapper.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <vector>

// Unit tests

void test_basic_insertion() {
    RedBlackTree<int> t;

    t.insert(10);
    t.insert(5);
    t.insert(15);

    assert(t.contains(10));
    assert(t.contains(5));
    assert(t.contains(15));

    t.check_invariants();
}

void test_duplicate_insertion() {
    RedBlackTree<int> t;

    assert(t.insert(10));
    assert(!t.insert(10));

    assert(t.size() == 1);

    t.check_invariants();

    RedBlackTree<int, true> t2;
    assert(t2.insert(10));
    assert(t2.insert(10));
    assert(t2.size() == 2);
}


void test_rotations() {
    RedBlackTree<int> t;

    // RR case (single left rotation)
    t.insert(1);
    t.insert(2);
    t.insert(3);

    t.check_invariants();

    // LL case (single right rotation)
    t.clear();
    t.insert(3);
    t.insert(2);
    t.insert(1);

    t.check_invariants();

    
    // LR case (left-right double rotation)
    t.clear();
    t.insert(3);
    t.insert(1);
    t.insert(2);
    
    t.check_invariants();
    
    // RL case (right-left double rotation)
    t.clear();
    t.insert(1);
    t.insert(3);
    t.insert(2);
    
    t.check_invariants();
}


void test_many_insertion() {
    RedBlackTree<int> t;

    for (int i = 1; i <= 1000; ++i) {
        t.insert(i);
        t.check_invariants();

    }

}

void test_random_insert() {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> dist(INT_MIN + 1, INT_MAX);

    RedBlackTree<int> t;

    for (int i = 0; i < 10000; ++i) {
        t.insert(dist(gen));
    }
    t.check_invariants();

    int prev_value = INT_MIN;
    for (const int& i : t) {
        assert(i > prev_value);
        prev_value = i;
    }
}


// Make sure that there are no unnecessary copies or moves
void test_copy_move_insert() {
    tracker_wrapper<int>::init();
    RedBlackTree<tracker_wrapper<int>> t;
    t.insert(10);

    tracker_wrapper<int> a(5);
    tracker_wrapper<int> b(20);

    t.insert(std::move(a));
    assert(tracker_wrapper<int>::constructs() == 3); // 1 from insert, 2 from construction above
    assert(tracker_wrapper<int>::copies() == 0);
    assert(tracker_wrapper<int>::moves() == 1);
    t.insert(b);
    assert(tracker_wrapper<int>::copies() == 1);
    assert(tracker_wrapper<int>::moves() == 1);


}

void test_delete_leaf() {
    RedBlackTree<int> t;

    t.insert(10);
    t.insert(5);
    t.insert(15);

    t.erase(5);

    assert(!t.contains(5));
    t.check_invariants();
}


void test_delete_one_child() {
    RedBlackTree<int> t;

    t.insert(10);
    t.insert(11);
    t.insert(8);
    t.check_invariants();
    t.insert(7);
    t.check_invariants();

    // Node with value 8 now has one left child (7)

    t.erase(8);

    assert(!t.contains(8));
    t.check_invariants();
}


void test_delete_two_children() {
    RedBlackTree<int> t;

    t.insert(10);
    t.insert(5);
    t.insert(15);
    t.insert(13);
    t.insert(17);

    t.erase(10);  // root with two children

    assert(!t.contains(10));
    t.check_invariants();
}

void ultimate_test() {
    {
        // Add a bunch of random elements and remove them randomly
        // Keep track of what should be in there using STL
        std::vector<int> nums;
    
        // Allow duplicates for the first test
        RedBlackTree<int, true> t;

        std::random_device rd;
    
        std::mt19937 gen(rd());
    
        std::uniform_int_distribution<> dist(INT_MIN, INT_MAX);
    
        constexpr int N_INSERT = 10000;
    
        nums.reserve(N_INSERT);
    
        for (int i = 0; i < N_INSERT; ++i) {
            int num = dist(gen);
            nums.push_back(num);
            assert(t.insert(num));
            t.check_invariants(); 
        }
    
        assert(t.size() == nums.size());
        for (int i : nums) {
            assert(t.contains(i));
        }
    
        for (int i : nums) {
            // Remove all numbers
            assert(t.erase(i));
            t.check_invariants();
        }
    
        
        assert(t.size() == 0);
    }
    {
        // Now try with std::string
        RedBlackTree<std::string> t;

        assert(t.insert("Hello")); // Becomes root
        assert(t.insert("Foundations")); // Becomes left child of Hello
        assert(t.insert("2")); // Becomes left most child of everyting -- left rotation on root: Foundations is root with Hello on right and 2 on left
        assert(t.insert("Final")); // Right child of 2

        assert(t.size() == 4);
        assert(t.contains("Hello"));
        assert(!t.contains("hello"));
        assert(!t.erase("hello"));
        assert(t.erase("Hello"));
        assert(t.size() == 3);
    }

}

void test_tree_copy() {
    tracker_wrapper<int>::init();
    RedBlackTree<tracker_wrapper<int>> t;
    t.insert(30);
    t.insert(40);
    t.insert(10);
    t.insert(20);

    assert(tracker_wrapper<int>::constructs() == 4);
    assert(tracker_wrapper<int>::copies() == 0);
    assert(tracker_wrapper<int>::moves() == 0);

    RedBlackTree<tracker_wrapper<int>> t2(t);
    t.check_invariants();
    t2.check_invariants();

    assert(tracker_wrapper<int>::copies() == 4);
}

void test_tree_move() {
    tracker_wrapper<int>::init();
    RedBlackTree<tracker_wrapper<int>> t;
    t.insert(30);
    t.insert(40);
    t.insert(10);
    t.insert(20);

    assert(tracker_wrapper<int>::constructs() == 4);
    assert(tracker_wrapper<int>::copies() == 0);
    assert(tracker_wrapper<int>::moves() == 0);

    RedBlackTree<tracker_wrapper<int>> t2(std::move(t));
    t2.check_invariants();

    assert(tracker_wrapper<int>::copies() == 0);
    t = std::move(t2);
    t.check_invariants();
}

void test_iterator() {
    // Add elements from 0 to 100
    RedBlackTree<int> t;

    constexpr int N = 100;

    for (int i = 0; i < N; ++i) {
        t.insert(i);
    }

    std::vector<bool> seen(N, false);

    assert(t.size() == N);
    int n_seen = 0;


    for (int i : t) {
        seen[i] = true;
        ++n_seen;
    }

    assert(n_seen == N);
    for (bool b : seen) {
        assert(b);
    }
}

void test_const_iterator() {
    // Add elements from 0 to 100
    RedBlackTree<int> t;

    constexpr int N = 100;

    for (int i = 0; i < N; ++i) {
        t.insert(i);
    }

    std::vector<bool> seen(N, false);

    assert(t.size() == N);
    int n_seen = 0;


    for (const int& i : t) {
        seen[i] = true;
        ++n_seen;
    }

    assert(n_seen == N);
    for (bool b : seen) {
        assert(b);
    }
}


int main() {
    test_basic_insertion();
    test_duplicate_insertion();
    test_rotations();
    test_many_insertion();
    test_random_insert();

    test_copy_move_insert();

    test_delete_leaf();
    test_delete_one_child();
    test_delete_two_children();

    ultimate_test();

    test_tree_copy();
    test_tree_move();

    test_iterator();
    test_const_iterator();

    std::cout << "All tests passed.\n";
}
