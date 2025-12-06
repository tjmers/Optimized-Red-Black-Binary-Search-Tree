// Copyright (c) 2025 Jacob Myers
// Licensed under the MIT License. See LICENSE file in the project root for full license text.

#pragma once

// / @brief Verifys that unnecessary copy/moves are avoided when inserting. Note that the counters are shared between all template instantiation.
/// @tparam T underlying data type
template <typename T>
class tracker_wrapper {
    T val_;
    inline static int constructs_;
    inline static int destructs_;
    inline static int copies_;
    inline static int moves_;
    inline static int copy_constructors_;
    inline static int copy_assignments_;
    inline static int move_constructors_;
    inline static int move_assignments_;
public:

    static void init() {
        constructs_ = 0;
        destructs_ = 0;
        copies_ = 0;
        moves_ = 0;
        copy_constructors_ = 0;
        copy_assignments_ = 0;
        move_constructors_ = 0;
        move_assignments_ = 0;
    }

    static void reset() {
        init();
    }

    tracker_wrapper(const T& x) : val_(x) {
        ++constructs_;
    }

    tracker_wrapper() = delete;

    ~tracker_wrapper() {
        ++destructs_;
    }

    tracker_wrapper(const tracker_wrapper& other) : val_(other.val_) {
        ++copy_constructors_;
        ++copies_;
    }

    tracker_wrapper(tracker_wrapper&& other) : val_(std::move(other.val_)) {
        ++move_constructors_;
        ++moves_;
    }

    tracker_wrapper& operator=(const tracker_wrapper& other) {
        val_ = other.val_;
        ++copy_assignments_;
        ++copies_;
        return *this;
    }

    tracker_wrapper& operator=(tracker_wrapper&& other) {
        val_ = std::move(other.val_);
        ++move_assignments_;
        ++moves_;
        return *this;
    }

    // Getters
    static int constructs() {
        return constructs_;
    }

    static int destructs() {
        return destructs_;
    }

    static int copies() {
        return copies_;
    }

    static int moves() {
        return moves_;
    }

    static int copy_constructors() {
        return copy_constructors_;
    }

    static int copy_assignments() {
        return copy_assignments_;
    }

    static int move_constructors() {
        return move_constructors_;
    }

    static int move_assignments() {
        return move_assignments_;
    }

    // Conversion to T
    operator T() const {
        return val_;
    }

    // Operators
    bool operator<(const tracker_wrapper& other) const {
        return val_ < other.val_;
    }

    bool operator==(const tracker_wrapper& other) const {
        return val_ == other.val_;
    }

};

// Create template specializations for std::less and std::equal_to

// There are here so that the objects are passed in by reference not value
namespace std {
    template <typename T>
    struct less<tracker_wrapper<T>> {
        bool operator() (const tracker_wrapper<T>& t1, const tracker_wrapper<T>& t2) const {
            return t1 < t2;
        }
    };

    template <typename T>
    struct equal_to<tracker_wrapper<T>> {
        bool operator() (const tracker_wrapper<T>& t1, const tracker_wrapper<T>& t2) const {
            return t1 == t2;
        }
    };
}