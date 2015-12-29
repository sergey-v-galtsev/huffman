#include <iostream>
#include <functional>
#include <limits>
#include <map>
#include <queue>
#include <string>
#include <vector>

using namespace std;

#define constexpr

size_t char2index(char ch) constexpr {
    return ch - numeric_limits<char>::min();
}

char index2char(size_t i) constexpr {
    return static_cast<char>(i) + numeric_limits<char>::min();
}

size_t const constexpr max_char = char2index(numeric_limits<char>::max());
size_t const constexpr sentinel = max_char + 1;
size_t const constexpr alphabet_size = sentinel + 1;

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
    code_t code;

    huffman_t() {
    }

    huffman_t(size_t count, size_t index) :
    count { count },
    code { { index, "" } }
    {
    }

    void prepend(char bit) {
        for (auto& c : code)
            code[c.first] = string(1, bit) + c.second;
    }

    bool operator>(const huffman_t& other) const {
        return count > other.count;
    }
};

huffman_t merge(huffman_t a, huffman_t b) {
    huffman_t result;

    result.count = a.count + b.count;

    a.prepend('0');
    b.prepend('1');
    result.code = a.code;
    result.code.insert(b.code.begin(), b.code.end());

    return result;
}

string encode(const vector<size_t>& input, const code_t& code) {
    string result;
    for (auto ch : input) {
        auto c = code.find(ch);
        if (c != code.end()) {
            result += c->second;
            result += '\n';
        } else {
            cerr << "Can not encode " << ch << " = ";
            if (is_char(ch))
               cerr << index2char(ch);
            else
               cerr << "not a char";
            cerr << '\n';
        }
    }
    return result;
}

code_t build_code(const vector<size_t>& input) {
    vector<size_t> count(alphabet_size);

    for (auto ch : input)
        ++count[ch];

    priority_queue<huffman_t, vector<huffman_t>, greater<huffman_t>> huffman;
    for (size_t i = 0; i < count.size(); ++i)
        if (count[i] > 0)
            huffman.push(huffman_t(count[i], i));

    while (huffman.size() > 1) {
        huffman_t a = huffman.top();
        huffman.pop();
        huffman_t b = huffman.top();
        huffman.pop();
        huffman.push(merge(a, b));
    }

    return huffman.top().code;
}

ostream& operator<<(ostream& out, const code_t& code) {
    for (const auto& c : code) {
        out << "letter=" << c.first;
        if (is_char(c.first))
            out << ", char=" << index2char(c.first);
        else
            out << ", not a char";
        out << ", code size=" << c.second.size()
            << ", code=" << c.second << "\n";
    }

    return out;
}

int main() {
    string s;
    cin >> s;

    vector<size_t> input = string2alphabet(s);
    input.push_back(sentinel);

    code_t code = build_code(input);

    cerr << code;

    string output = encode(input, code);

    cout << output;

    return 0;
}
