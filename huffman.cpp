#include <iostream>
#include <limits>
#include <string>
#include <vector>

using namespace std;

size_t char2index(char ch) constexpr {
    return ch - numeric_limits<char>::min();
}

char index2char(size_t i) constexpr {
    return i + numeric_limits<char>::min();
}

size_t const constexpr max_char = char2index(numeric_limits<char>::max());
size_t const constexpr sentinel = max_char + 1;

bool is_char(size_t i) {
    return i <= max_char;
}

vector<size_t> string2alphabet(const string& input) {
    vector<size_t> result;
    for (auto ch : input)
        result.push_back(char2index(ch));
    return result;
}

string alphabet2string (const vector<size_t>& input) {
    string result;
    for (auto ch : input)
        if (is_char(ch))
            result += index2char(ch);
    return result;
}

using code_t = map<size_t, string>;

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
    huffman_t result;

    result.count = a.count + b.count;

    a.prepend('0');
    b.prepend('1');
    result.code = a.code;
    result.code += b.code;

    return result;
}

string encode(const vector<size_t>& input, const code_t& code) {
    string result;
    for (auto ch : input) {
        result += code[input];
        result += '\n';
    }
    return result;
}

code_t build_code(const vector<size_t>& input) {
    vector<size_t> count(1 << numeric_limits<char>::bits);

    for (auto ch : input)
        ++count[ch];

    priority_queue<huffman_t> huffman;
    for (size_t i = 0; i < count.size(); ++i)
        huffman.push(huffman_t(count[i], i));

    while (huffman.size() > 1) {
        huffman_t a = huffman.pop();
        huffman_t b = huffman.pop();
        huffman.push(merge(a, b));
    }

    return huffman.code;
}

int main() {
    string s;
    cin >> s;

    vector<size_t> input = string2alphabet(s);
    input.push_back(sentinel);

    code_t code = build_code(input);

    string output = encode(input, code);

    cout << output;

    return 0;
}
