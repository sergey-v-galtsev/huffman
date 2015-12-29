#include <iostream>
#include <functional>
#include <limits>
#include <map>
#include <queue>
#include <string>
#include <vector>

using namespace std;

#define constexpr

using letter_t = size_t;

letter_t char2letter(char ch) constexpr {
    return ch - numeric_limits<char>::min();
}

char letter2char(letter_t l) constexpr {
    return static_cast<char>(l) + numeric_limits<char>::min();
}

letter_t const constexpr max_char = char2letter(numeric_limits<char>::max());
letter_t const constexpr sentinel = max_char + 1;
letter_t const constexpr alphabet_size = sentinel + 1;

bool is_char(letter_t l) {
    return l <= max_char;
}

vector<letter_t> char2letter(const string& input) {
    vector<letter_t> result;
    for (auto ch : input)
        result.push_back(char2letter(ch));
    return result;
}

string letter2char(const vector<letter_t>& input) {
    string result;
    for (auto l : input)
        if (is_char(l))
            result += letter2char(l);
    return result;
}

using bit_t = char;
using bits_t = string;
using code_t = map<letter_t, bits_t>;

class huffman_t {
public:
    size_t count;
    code_t code;

    huffman_t() {
    }

    huffman_t(size_t count, letter_t letter) :
    count { count },
    code { { letter, "" } }
    {
    }

    void prepend(bit_t bit) {
        for (auto& c : code)
            code[c.first] = bits_t(1, bit) + c.second;
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

bits_t encode(const vector<letter_t>& input, const code_t& code) {
    string result;
    for (auto l : input) {
        auto c = code.find(l);
        if (c != code.end()) {
            result += c->second;
            result += '\n';
        } else {
            cerr << "Can not encode letter " << l << " ("
                << (is_char(l) ? string(1, letter2char(l)) : "not a char") << ")\n";
        }
    }
    return result;
}

code_t build_code(const vector<letter_t>& input) {
    vector<size_t> count(alphabet_size);

    for (auto l : input)
        ++count[l];

    priority_queue<huffman_t, vector<huffman_t>, greater<huffman_t>> huffman;
    for (letter_t i = 0; i < count.size(); ++i)
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
            out << ", char=" << letter2char(c.first);
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

    vector<letter_t> input = char2letter(s);
    input.push_back(sentinel);

    code_t code = build_code(input);

    cerr << code;

    string output = encode(input, code);

    cout << output;

    return 0;
}
