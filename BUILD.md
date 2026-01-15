# Lab2 - Virtual File System

## Project Structure

- `vfs-app/` - VFS console application
- `tests/` - Unit tests for data structures (Google Test)
- `benchmarks/` - Performance benchmarks
- `test-main.cpp` - Development testing file
- `inc/` - Header files
- `src/` - Source files

## Prerequisites

Install Google Test:
```bash
# Ubuntu/Debian
sudo apt-get install libgtest-dev

# Or build from source
git clone https://github.com/google/googletest.git
cd googletest
mkdir build && cd build
cmake ..
make
sudo make install
```

## Build Instructions

### Build all targets:
```bash
mkdir build && cd build
cmake ..
make
```

### Run VFS Application:
```bash
./vfs-app
```

### Run Unit Tests:
```bash
./unit-tests
```

### Run Benchmarks:
```bash
./benchmarks
```

### Run Development Tests:
```bash
./test-lab2
```

## Test Coverage

Unit tests cover:
- **BTree**: Insert, remove, find, iterators, set mode, large datasets, stress tests
- **BPlusTree**: Insert, remove, find, iterators, set mode, sequential access, stress tests
- **Edge cases**: Empty trees, duplicates, non-existing keys
- **Performance**: Large datasets (1000+ elements), random operations

## VFS Commands

- `cd <path>` - Change directory
- `mkdir <path>` - Create directory
- `touch <path>` - Create empty file
- `attach <vpath> <ppath>` - Attach physical file to virtual path
- `rmdir <path>` - Remove directory
- `rm/remove <path>` - Remove file
- `mv/move <from> <to>` - Move file/directory
- `<path>` - Open file/directory
- `help/h` - Show manual
- `exit` - Exit application