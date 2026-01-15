#include <iostream>
#include "Benchmark.hpp"

int main() {
    BTreeBenchmark<BTree,1024> b1("btree");
    BTreeBenchmark<BPlusTree,1024> b2("bplustree");
    
    b1.launchInsertions( 1'000'000 );
    b1.launchLookup( 1'000'000 );
    b1.launchRemovals( 1'000'000 );

    std::cout << "btree done" << std::endl;
    
    b2.launchInsertions( 1'000'000 );
    b2.launchLookup( 1'000'000 );
    b2.launchRemovals( 1'000'000 );

    std::cout << "bplustree done" << std::endl;

    b1.plot();
}