#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <filesystem>
#include <algorithm>
#include <vector>
#include <chrono>
#include <fstream>
#include <random>
#include "BTree.hpp"
#include "BPlusTree.hpp"
#include "CRequirements.hpp"

namespace fs = std::filesystem;

template <
    template<class,class,ssize_t> class TTree
  , ssize_t Degree
  , class T = std::uint64_t
> requires CAssociative<TTree<T,T,Degree>,T,T>
class BTreeBenchmark
{
private:
    using tree = TTree<T,T,Degree>;
    using clock = std::chrono::steady_clock;
public:
    BTreeBenchmark( const std::string& folder ) : 
    _rng( std::random_device{}() )
    , _dist( std::uniform_int_distribution<T>(0, 1'000'000'000) )
    , _path( "../inc/Benchmark/results" ) {
        if (folder != "bplustree" && folder != "btree") {
            throw Exception( Exception::ErrorCode::INVALID_INPUT );
        }
        _path /= folder;
        fs::create_directories(_path);
    }

    ~BTreeBenchmark() = default;
public:
    void launchInsertions( const size_t count ) {
        std::ofstream csv(_path / "insert.csv", std::ios::trunc);
        if (!csv) { throw Exception( "Error. Creating a file failed."); }
        csv << "count,time_us\n";
        
        auto data = uniqueSet( count );

        for (size_t i = 1; i <= 10; i++) {
            tree t;
            auto n = (count * i) / 10;

            auto start = clock::now();

            for (size_t j = 0; j < n; j++) {
                t.insert( data[j] );
            }

            auto end = clock::now();

            auto time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            csv << n << "," << time << "\n";
        }
        csv.close();
    }

    void launchLookup( const size_t count ) {
        std::ofstream csv(_path / "lookup.csv", std::ios::trunc);
        if (!csv) { throw Exception( "Error. Creating a file failed."); }
        csv << "count,time_us\n";
    
        auto data = uniqueSet( count );

        for (size_t i = 1; i <= 10; i++) {
            tree t;
            auto n = (count * i) / 10;
            for (size_t j = 0; j < n; j++) { t.insert( data[j] ); }

            size_t queryCount = n < 100'000 ? n : 100'000;
            DynamicArray<T> queries( queryCount );
            std::uniform_int_distribution<T> queryDist(0, n-1);
            for (size_t j = 0; j < queryCount; j++) {
                queries.append(queryDist(_rng));
            }
            
            volatile size_t acc = 0;
            
            auto start = clock::now();
            for (size_t j = 0; j < queryCount; j++) {
                if ( t.contains( data[queries[j]] ) ) {
                    acc += 1;
                }
            }
            auto end = clock::now();

            auto time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            csv << n << "," << time << "\n";
        }
        csv.close();
    }

    void launchRemovals( const size_t count ) {
        std::ofstream csv(_path / "remove.csv", std::ios::trunc);
        if (!csv) { throw Exception( "Error. Creating a file failed."); }
        csv << "count,time_us\n";
    
        auto data = uniqueSet( count );

        for (size_t i = 1; i <= 10; i++) {
            tree t;
            auto n = (count * i) / 10;
            for (size_t j = 0; j < n; j++) { t.insert( data[j] ); }

            size_t queryCount = n < 100'000 ? n : 100'000;
            DynamicArray<T> queries( queryCount );
            std::uniform_int_distribution<T> queryDist(0, n-1);
            for (size_t j = 0; j < queryCount; j++) {
                queries.append(queryDist(_rng));
            }
            
            auto start = clock::now();
            for (size_t j = 0; j < queryCount; j++) {
                t.remove( data[queries[j]] );
            }            
            auto end = clock::now();

            auto time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            csv << n << "," << time << "\n";
        }
        csv.close();
    }

    void plot() {
        auto res = std::system("python3 ../inc/Benchmark/plot_results.py");
        if (res != 0) {
            throw Exception( "Error. Python plotter call failed." );
        }
    }
private:
    DynamicArray<T> dataset( const size_t count ) {
        auto arr = DynamicArray<T>(count);

        for (size_t i = 0; i < count; i++) {
            arr.append( _dist(_rng));
        }
        return arr;
    }
    
    std::vector<T> uniqueSet( const size_t count ) {
        std::vector<T> v(count);
        std::iota(v.begin(), v.end(), static_cast<T>(0));
        std::shuffle(v.begin(), v.end(), _rng);
        return v;
    }
private:
    std::mt19937 _rng;
    std::uniform_int_distribution<T> _dist;
    std::filesystem::path _path;
};

#endif // BENCHMARK_H