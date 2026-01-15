#include <gtest/gtest.h>
#include <iostream>
#include "BPlusTree.hpp"
#include "BTree.hpp"
#include "Pair.hpp"

// BTree Tests
class BTreeTest : public ::testing::Test {
protected:
    BTree<int, std::string> tree;
    BTree<int, int> set_tree;
};

TEST_F(BTreeTest, EmptyTree) {
    EXPECT_TRUE(tree.isEmpty());
    EXPECT_EQ(tree.getSize(), 0);
    EXPECT_FALSE(tree.contains(1));
}

TEST_F(BTreeTest, SingleInsert) {
    tree.insert(Pair<int, std::string>(1, "one"));
    EXPECT_FALSE(tree.isEmpty());
    EXPECT_EQ(tree.getSize(), 1);
    EXPECT_TRUE(tree.contains(1));
    EXPECT_EQ(tree.get(1), "one");
}

TEST_F(BTreeTest, MultipleInserts) {
    tree.insert(Pair<int, std::string>(1, "one"));
    tree.insert(Pair<int, std::string>(2, "two"));
    tree.insert(Pair<int, std::string>(3, "three"));
    
    EXPECT_EQ(tree.getSize(), 3);
    EXPECT_TRUE(tree.contains(1));
    EXPECT_TRUE(tree.contains(2));
    EXPECT_TRUE(tree.contains(3));
    EXPECT_EQ(tree.get(1), "one");
    EXPECT_EQ(tree.get(2), "two");
    EXPECT_EQ(tree.get(3), "three");
}

TEST_F(BTreeTest, DuplicateKeyThrows) {
    tree.insert(Pair<int, std::string>(1, "one"));
    EXPECT_THROW(tree.insert(Pair<int, std::string>(1, "duplicate")), Exception);
}

TEST_F(BTreeTest, FindExisting) {
    tree.insert(Pair<int, std::string>(5, "five"));
    auto it = tree.find(5);
    EXPECT_NE(it, tree.end());
    EXPECT_EQ(*it, "five");
}

TEST_F(BTreeTest, FindNonExisting) {
    tree.insert(Pair<int, std::string>(5, "five"));
    auto it = tree.find(10);
    EXPECT_EQ(it, tree.end());
}

TEST_F(BTreeTest, RemoveSingle) {
    tree.insert(Pair<int, std::string>(1, "one"));
    tree.remove(1);
    EXPECT_TRUE(tree.isEmpty());
    EXPECT_FALSE(tree.contains(1));
}

TEST_F(BTreeTest, RemoveMultiple) {
    for (int i = 1; i <= 10; ++i) {
        tree.insert(Pair<int, std::string>(i, std::to_string(i)));
    }
    
    tree.remove(5);
    tree.remove(1);
    tree.remove(10);
    
    EXPECT_EQ(tree.getSize(), 7);
    EXPECT_FALSE(tree.contains(1));
    EXPECT_FALSE(tree.contains(5));
    EXPECT_FALSE(tree.contains(10));
    EXPECT_TRUE(tree.contains(2));
    EXPECT_TRUE(tree.contains(9));
}

TEST_F(BTreeTest, IteratorTraversal) {
    for (int i = 1; i <= 5; ++i) {
        tree.insert(Pair<int, std::string>(i, std::to_string(i)));
    }
    
    std::vector<std::string> values;
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        values.push_back(*it);
    }
    
    EXPECT_EQ(values.size(), 5);
    EXPECT_EQ(values[0], "1");
    EXPECT_EQ(values[4], "5");
}

TEST_F(BTreeTest, SetMode) {
    set_tree.insert(1);
    set_tree.insert(3);
    set_tree.insert(2);
    
    EXPECT_TRUE(set_tree.contains(1));
    EXPECT_TRUE(set_tree.contains(2));
    EXPECT_TRUE(set_tree.contains(3));
    EXPECT_EQ(set_tree.getSize(), 3);
}

TEST_F(BTreeTest, LargeDataSet) {
    const int N = 1000;
    for (int i = 0; i < N; ++i) {
        tree.insert(Pair<int, std::string>(i, std::to_string(i)));
    }
    
    EXPECT_EQ(tree.getSize(), N);
    for (int i = 0; i < N; ++i) {
        EXPECT_TRUE(tree.contains(i));
    }
    
    // Remove half
    for (int i = 0; i < N/2; ++i) {
        tree.remove(i);
    }
    
    EXPECT_EQ(tree.getSize(), N/2);
    for (int i = 0; i < N/2; ++i) {
        EXPECT_FALSE(tree.contains(i));
    }
    for (int i = N/2; i < N; ++i) {
        EXPECT_TRUE(tree.contains(i));
    }
}

// BPlusTree Tests
class BPlusTreeTest : public ::testing::Test {
protected:
    BPlusTree<int, std::string> tree;
    BPlusTree<int, int> set_tree;
};

TEST_F(BPlusTreeTest, EmptyTree) {
    EXPECT_TRUE(tree.isEmpty());
    EXPECT_EQ(tree.getSize(), 0);
    EXPECT_FALSE(tree.contains(1));
}

TEST_F(BPlusTreeTest, SingleInsert) {
    tree.insert(Pair<int, std::string>(1, "one"));
    EXPECT_FALSE(tree.isEmpty());
    EXPECT_EQ(tree.getSize(), 1);
    EXPECT_TRUE(tree.contains(1));
    EXPECT_EQ(tree.get(1), "one");
}

TEST_F(BPlusTreeTest, MultipleInserts) {
    tree.insert(Pair<int, std::string>(1, "one"));
    tree.insert(Pair<int, std::string>(2, "two"));
    tree.insert(Pair<int, std::string>(3, "three"));
    
    EXPECT_EQ(tree.getSize(), 3);
    EXPECT_TRUE(tree.contains(1));
    EXPECT_TRUE(tree.contains(2));
    EXPECT_TRUE(tree.contains(3));
    EXPECT_EQ(tree.get(1), "one");
    EXPECT_EQ(tree.get(2), "two");
    EXPECT_EQ(tree.get(3), "three");
}

TEST_F(BPlusTreeTest, DuplicateKeyThrows) {
    tree.insert(Pair<int, std::string>(1, "one"));
    EXPECT_THROW(tree.insert(Pair<int, std::string>(1, "duplicate")), Exception);
}

TEST_F(BPlusTreeTest, FindExisting) {
    tree.insert(Pair<int, std::string>(5, "five"));
    auto it = tree.find(5);
    EXPECT_NE(it, tree.end());
    EXPECT_EQ(*it, "five");
}

TEST_F(BPlusTreeTest, FindNonExisting) {
    tree.insert(Pair<int, std::string>(5, "five"));
    auto it = tree.find(10);
    EXPECT_EQ(it, tree.end());
}

TEST_F(BPlusTreeTest, RemoveSingle) {
    tree.insert(Pair<int, std::string>(1, "one"));
    tree.remove(1);
    EXPECT_TRUE(tree.isEmpty());
    EXPECT_FALSE(tree.contains(1));
}

TEST_F(BPlusTreeTest, RemoveMultiple) {
    for (int i = 1; i <= 10; ++i) {
        tree.insert(Pair<int, std::string>(i, std::to_string(i)));
    }
    
    tree.remove(5);
    tree.remove(1);
    tree.remove(10);
    
    EXPECT_EQ(tree.getSize(), 7);
    EXPECT_FALSE(tree.contains(1));
    EXPECT_FALSE(tree.contains(5));
    EXPECT_FALSE(tree.contains(10));
    EXPECT_TRUE(tree.contains(2));
    EXPECT_TRUE(tree.contains(9));
}

TEST_F(BPlusTreeTest, IteratorTraversal) {
    for (int i = 1; i <= 5; ++i) {
        tree.insert(Pair<int, std::string>(i, std::to_string(i)));
    }
    
    std::vector<std::string> values;
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        values.push_back(*it);
    }
    
    EXPECT_EQ(values.size(), 5);
    EXPECT_EQ(values[0], "1");
    EXPECT_EQ(values[4], "5");
}

TEST_F(BPlusTreeTest, SetMode) {
    set_tree.insert(1);
    set_tree.insert(3);
    set_tree.insert(2);
    
    EXPECT_TRUE(set_tree.contains(1));
    EXPECT_TRUE(set_tree.contains(2));
    EXPECT_TRUE(set_tree.contains(3));
    EXPECT_EQ(set_tree.getSize(), 3);
}

TEST_F(BPlusTreeTest, LargeDataSet) {
    const int N = 1000;
    for (int i = 0; i < N; ++i) {
        tree.insert(Pair<int, std::string>(i, std::to_string(i)));
    }
    
    EXPECT_EQ(tree.getSize(), N);
    for (int i = 0; i < N; ++i) {
        EXPECT_TRUE(tree.contains(i));
    }
    
    // Remove half
    for (int i = 0; i < N/2; ++i) {
        tree.remove(i);
    }
    
    EXPECT_EQ(tree.getSize(), N/2);
    for (int i = 0; i < N/2; ++i) {
        EXPECT_FALSE(tree.contains(i));
    }
    for (int i = N/2; i < N; ++i) {
        EXPECT_TRUE(tree.contains(i));
    }
}

TEST_F(BPlusTreeTest, SequentialAccess) {
    for (int i = 1; i <= 100; ++i) {
        tree.insert(Pair<int, std::string>(i, std::to_string(i)));
    }
    
    // Test forward iteration
    int count = 1;
    for (auto it = tree.begin(); it != tree.end(); ++it, ++count) {
        EXPECT_EQ(*it, std::to_string(count));
    }
    EXPECT_EQ(count, 101);
}

// Stress Tests
TEST(StressTest, BTreeRandomOperations) {
    BTree<int, int> tree;
    std::set<int> reference;
    
    // Random insertions
    for (int i = 0; i < 500; ++i) {
        int key = rand() % 1000;
        if (reference.find(key) == reference.end()) {
            tree.insert(Pair<int, int>(key, key));
            reference.insert(key);
        }
    }
    
    // Verify all elements
    for (int key : reference) {
        EXPECT_TRUE(tree.contains(key));
    }
    
    // Random deletions
    auto it = reference.begin();
    for (int i = 0; i < 100 && it != reference.end(); ++i) {
        int key = *it;
        tree.remove(key);
        it = reference.erase(it);
    }
    
    // Verify remaining elements
    for (int key : reference) {
        EXPECT_TRUE(tree.contains(key));
    }
}

TEST(StressTest, BPlusTreeRandomOperations) {
    BPlusTree<int, int> tree;
    std::set<int> reference;
    
    // Random insertions
    for (int i = 0; i < 500; ++i) {
        int key = rand() % 1000;
        if (reference.find(key) == reference.end()) {
            tree.insert(Pair<int, int>(key, key));
            reference.insert(key);
        }
    }
    
    // Verify all elements
    for (int key : reference) {
        EXPECT_TRUE(tree.contains(key));
    }
    
    // Random deletions
    auto it = reference.begin();
    for (int i = 0; i < 100 && it != reference.end(); ++i) {
        int key = *it;
        tree.remove(key);
        it = reference.erase(it);
    }
    
    // Verify remaining elements
    for (int key : reference) {
        EXPECT_TRUE(tree.contains(key));
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}