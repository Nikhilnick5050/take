# Advanced C++ DSA Files: Plan

Goal: 10 standalone C++ files, each at least 400 lines, advanced data structures and algorithms. Each file has a header comment, compiles with `g++ -std=c++17 -Wall -Wextra`, and includes a `main()` with test cases.

Split:
- Helper 1: 01 to 04
- Helper 2: 05 to 08
- Me: 09 and 10

Files (saved in the project root under `dsa/`):
1. dsa/01_avl_tree.cpp: AVL tree with insert, erase, rotations, order statistics
2. dsa/02_segment_tree_lazy.cpp: segment tree with lazy propagation (range add, range sum, range min)
3. dsa/03_trie_suffix_array.cpp: trie, suffix array, LCP array
4. dsa/04_max_flow_dinic.cpp: Dinic's max flow and min cut
5. dsa/05_dijkstra_astar.cpp: Dijkstra with heap, A* search on a grid
6. dsa/06_advanced_dynamic_programming.cpp: knapsack variants, bitmask DP, digit DP, interval DP
7. dsa/07_hash_table_open_addressing.cpp: open addressing hash table with linear and double hashing, rehash
8. dsa/08_priority_structures.cpp: binary heap, leftist heap, binomial heap
9. dsa/09_graph_scc_bridges.cpp: Tarjan SCC, bridges, articulation points, topological sort
10. dsa/10_string_kmp_z.cpp: KMP, Z-function, Rabin-Karp, Aho-Corasick

Status:
- [ ] Helper 1 (01 to 04)
- [ ] Helper 2 (05 to 08)
- [ ] Me (09, 10)
- [ ] Compile check and line count check
