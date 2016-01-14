huffman: huffman.cpp
	clang++ -std=c++14 -Wall $< -o $@
	./$@ --test < $< 2>/dev/null
