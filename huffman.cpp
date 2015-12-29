#include <iostream>
#include <limits>
#include <string>
#include <vector>

using namespace std;

size_t char2index(char ch) {
    return ch - numeric_limits<char>::min();
}

char index2char(size_t i) {
    return i + numeric_limits<char>::min();
}

class huffman_t {
public:
    size_t count;
    map<size_t, string> code;

    huffman_t(size_t count, size_t index) :
    count { count },
    code { { index, "" } }
    {
    }

    void prepend(char bit) {
        for (auto c : code)
            c.second = string(bit) + c.second;
    }
};

huffman_t merge(huffman_t a, huffman_t b) {
}

int main() {
    vector<size_t> count(1 << numeric_limits<char>::bits);
    for (char ch; cin >> ch; )
        ++vector[char2index(ch)];

    priority_queue<huffman_t> huffman;
    for (size_t i = 0; i < count.size(); ++i)
        huffman.push(huffman_t(count[i], i));

    while (huffman.size() > 1) {
        huffman_t a = huffman.pop();
        huffman_t b = huffman.pop();
        huffman.pu
    }

    return 0;
}
