#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

using letter_t = size_t;
using text_t = vector<letter_t>;

letter_t char2letter(char ch)
{
    return static_cast<unsigned char>(ch);
}

char letter2char(letter_t l)
{
    return static_cast<unsigned char>(l);
}

letter_t const max_char = char2letter(numeric_limits<unsigned char>::max());
letter_t const sentinel = max_char + 1; /* Just for a demonstration purpose. */
letter_t const max_letter = numeric_limits<letter_t>::max() / 2;

size_t const bits_per_char = numeric_limits<unsigned char>::digits;

static_assert(sizeof(letter_t) > sizeof(char), "choose a wider type for letter_t so it can hold more than just a char");
static_assert(bits_per_char == 8, "bits_per_char != 8, pack_bits()/unpack_bits() have to be rewritten to be platform independent");

bool is_char(letter_t l)
{
    return l <= max_char;
}

text_t char2letter(string const & text)
{
    text_t result;
    for (auto ch : text)
        result.push_back(char2letter(ch));
    return result;
}

string letter2char(text_t const & text)
{
    string result;
    for (auto l : text)
        if (is_char(l))
            result += letter2char(l);
    return result;
}

using bit_t = char;
using bits_t = string;
using code_t = map<letter_t, bits_t>;

bit_t int2bit(uint8_t x)
{
    return '0' + (x & 1);
}

uint8_t bit2int(bit_t x)
{
    return (x - '0') & 1;
}

bits_t uint2unary(unsigned int n)
{
    bits_t result(n, int2bit(1));
    result += int2bit(0);
    return result;
}

unsigned int unary2uint(bits_t const & bits, size_t & position)
{
    unsigned int result = 0;
    for (; position < bits.size() and bits[position] == int2bit(1); ++position)
        ++result;

    if (position < bits.size())
        ++position;
    else
    {
        cerr << "Error: no terminating zero bit in a unary code.\n";
        abort();
    }

    return result;
}

unsigned int digits(unsigned int n)
{
    unsigned int result = 0;
    for (; n > 0; n >>= 1)
        ++result;
    return result;
}

bits_t uint2gamma(unsigned int n)
{
    assert(n > 0);

    unsigned int size = digits(n);
    bits_t result(size - 1, int2bit(0));

    for (unsigned int i = size; i-- > 0; )
        result += int2bit((n >> i) & 1);

    return result;
}

unsigned int gamma2uint(bits_t const & bits, size_t & position)
{
    unsigned int size = 1;
    for (; position < bits.size() and bits[position] == int2bit(0); ++position)
        ++size;

    if (bits.size() - position < size)
    {
        cerr << "Error: gamma code is truncated.\n";
        abort();
    }

    unsigned int result = 0;
    for (unsigned int i = 0; i < size; ++i, ++position)
        result = (result << 1) | bit2int(bits[position]);

    return result;
}

class decode_t
{
public:
    decode_t()
    {
    }

    explicit decode_t(code_t const & code)
    {
        for (auto const & c : code)
            push(c.second, c.first);
    }

    void push(bits_t const & bits, letter_t letter)
    {
        if (letter > max_letter)
        {
            cerr << "Error: letter value " << letter << " is greater than maximum " << max_letter << ".\n";
            abort();
        }

        if (bits.size() == 0)
        {
            cerr << "Error: code for letter=" << letter << " is empty.\n";
            abort();
        }

        node_index_t position = 0;
        node_index_t node = walk(bits, position);
        if (position >= bits.size())
        {
            cerr << "Error: code " << bits << " for letter=" << letter
                << " is a prefix of another code.\n";
            abort();
        }

        trie.reserve(trie.size() + bits.size() - 1 - position);
        for (; position < bits.size() - 1; ++position)
        {
            node_index_t next = trie.size();
            trie.push_back(node_t { });
            trie[node].set_child(bits[position], next);
            node = next;
        }

        trie[node].set_letter(bits[position], letter);
    }

    letter_t decode(bits_t const & bits, size_t & position) const
    {
        size_t p = position;
        auto const & node = trie[walk(bits, p)];
        if (not node.is_leaf(bits[p]))
            return letter_t { };
        position = p + 1;
        return node.get_letter(bits[p]);
    }

    void dump(ostream & out, string const & label) const
    {
        static size_t const max_label = 80;

        out << "digraph {\n  label=\"";
        for (auto const & c : label.substr(0, max_label))
            out << escape(c);
        out << "\"\n  fontsize=32\n";

        for (node_index_t i = 0; i < trie.size(); ++i)
        {
            out << "  node_" << i << " [label = \"\", shape = point]\n";
            auto const & node = trie[i];
            for (int j = 0; j <= 1; ++j)
            {
                if (node.is_leaf(j))
                {
                    letter_t l = node.get_letter(j);
                    out << "  node_" << i << '_' << j << " [label = \"";
                    if (is_char(l))
                        out << escape(letter2char(l));
                    else
                        out << "letter=" << l;
                    out << "\", shape = box]\n"
                        << "  node_" << i << " -> node_" << i << '_' << j;
                }
                else if (node.is_nil(j))
                    out << "  node_" << i << '_' << j
                        << " [label = \"nil\", shape = circle]\n"
                        << "  node_" << i << " -> node_" << i << '_' << j;
                else
                    out << "  node_" << i << " -> node_" << node.get_child(j);
                out << " [label = \"" << j << "\"]\n";
            }
        }

        out << "}\n";
    }

private:
    using node_index_t = size_t;

    static string escape(char c)
    {
        if (not isprint(c))
        {
            ostringstream result;
            result << "\\\\x" << hex << setfill('0') << setw(2)
                << static_cast<int>(static_cast<unsigned char>(c));
            return result.str();
        }
        else if (c == '"' or c == '\\')
            return string { "\\" } + c;
        else
            return string { 1, c };
    }

    class node_t
    {
    public:
        bool is_leaf(bit_t bit) const
        {
            return child[bit2int(bit)] > max_node;
        }

        bool is_nil(bit_t bit) const
        {
            return child[bit2int(bit)] == nil;
        }

        bool is_child(bit_t bit) const
        {
            node_index_t node = child[bit2int(bit)];
            return node <= max_node and node != nil;
        }

        letter_t get_letter(bit_t bit) const
        {
            assert(is_leaf(bit));
            return letters_top - child[bit2int(bit)];
        }

        void set_letter(bit_t bit, letter_t letter)
        {
            assert(letters_top - letter > max_node);
            assert(is_nil(bit));
            child[bit2int(bit)] = letters_top - letter;
        }

        node_index_t get_child(bit_t bit) const
        {
            assert(is_child(bit));
            return child[bit2int(bit)];
        }

        void set_child(bit_t bit, node_index_t node)
        {
            assert(is_nil(bit));
            child[bit2int(bit)] = node;
        }

    private:
        static_assert(sizeof(node_index_t) >= sizeof(letter_t), "letter_t variables can not fit in node_index_t, use a larger type for trie node index");

        static node_index_t const letters_top = numeric_limits<node_index_t>::max();
        static node_index_t const max_node = numeric_limits<node_index_t>::max() / 2;

        static_assert(letters_top - max_letter > max_node, "letter and node index ranges can not be combined into node_index_t, reduce these ranges or do not store them in one variable.");

        static node_index_t const nil = 0;

        node_index_t child[2] { nil, nil };
    };

    node_index_t walk(bits_t const & bits, node_index_t & position) const
    {
        node_index_t node = 0;
        for (; position < bits.size() and trie[node].is_child(bits[position]); ++position)
        {
            node_index_t next = trie[node].get_child(bits[position]);
            assert(node < next and next < trie.size());
            node = next;
        }
        return node;
    }

    vector<node_t> trie { node_t { } };
};

bits_t encode(text_t const & text, code_t const & code)
{
    bits_t result;
    for (auto l : text)
    {
        auto c = code.find(l);
        if (c != code.cend())
            result += c->second;
        else
        {
            cerr << "Can not encode letter " << l << " ("
                << (is_char(l) ? string(1, letter2char(l)) : "not a char") << ")\n";
            abort();
        }
    }
    return result;
}

text_t decode(bits_t const & text, decode_t const & code)
{
    text_t result;

    for (size_t i = 0; i < text.size(); )
    {
        size_t position = i;
        letter_t l = code.decode(text, position);

        if (i < position)
            result.push_back(l);
        else
        {
            cerr << "Failed to decode bit sequence from position " << i << ": " << text.substr(i, 100) << "...\n";
            abort();
        }

        i = position;
    }

    return result;
}

class huffman_t
{
public:
    size_t count;
    code_t code;

    huffman_t()
    {
    }

    huffman_t(size_t count, letter_t letter) :
        count { count },
        code { { letter, "" } }
    {
        if (letter > max_letter)
        {
            cerr << "Error: letter value " << letter << " is greater than maximum " << max_letter << ".\n";
            abort();
        }
    }

    huffman_t(huffman_t && a, huffman_t && b) :
        count { a.count + b.count }
    {
        a.prepend(int2bit(0));
        b.prepend(int2bit(1));

        swap(code, a.code);
        code.insert(b.code.cbegin(), b.code.cend());
    }

    void prepend(bit_t bit)
    {
        for (auto & c : code)
            code[c.first] = bits_t(1, bit) + c.second;
    }

    bool operator>(huffman_t const & other) const
    {
        return count > other.count;
    }
};

code_t build_code(text_t const & text)
{
    vector<size_t> count;

    for (auto l : text)
    {
        if (l >= count.size())
            count.resize(l + 1);
        ++count[l];
    }

    priority_queue<huffman_t, vector<huffman_t>, greater<huffman_t>> huffman;
    for (letter_t i = 0; i < count.size(); ++i)
        if (count[i] > 0)
            huffman.push(huffman_t(count[i], i));

    while (huffman.size() > 1)
    {
        huffman_t a = huffman.top();
        huffman.pop();
        huffman_t b = huffman.top();
        huffman.pop();
        huffman.push(huffman_t(move(a), move(b)));
    }

    if (huffman.empty())
        return code_t { };

    code_t result = huffman.top().code;
    huffman.pop();

    if (result.size() == 1 and result.cbegin()->second.empty())
        result.begin()->second = int2bit(0);

    return result;
}

ostream & operator<<(ostream & out, code_t const & code)
{
    for (auto const & c : code)
    {
        out << "letter=" << c.first;
        if (is_char(c.first))
        {
            if (isprint(letter2char(c.first)))
                out << ", char=" << letter2char(c.first);
            else
                out << ", ascii=" << static_cast<int>(static_cast<unsigned char>(letter2char(c.first)));
        } else
            out << ", not a char";
        out << ", code size=" << c.second.size()
            << ", code=" << c.second << "\n";
    }

    return out;
}

bits_t pack_code(code_t const & code)
{
    bits_t result = uint2gamma(code.size() + 1);

    letter_t previous_letter = -1;

    for (auto const & c : code)
    {
        assert(previous_letter == -1 or previous_letter < c.first);
        result += uint2gamma(c.first - previous_letter);
        previous_letter = c.first;

        result += uint2gamma(c.second.size());
        result += c.second;
    }

    return result;
}

decode_t unpack_code(bits_t const & bits, size_t & position)
{
    decode_t result;

    letter_t letter = -1;

    unsigned int entries = gamma2uint(bits, position) - 1;
    unsigned int i;

    for (i = 0; position < bits.size() and i < entries; ++i)
    {
        letter += gamma2uint(bits, position);

        unsigned int code_size = gamma2uint(bits, position);
        if (bits.size() - position < code_size)
        {
            cerr << "Error: code table is truncated.\n";
            abort();
        }
        bits_t code = bits.substr(position, code_size);
        position += code_size;

        result.push(code, letter);
    }

    if (i != entries)
    {
        cerr << "Error: failed to read code table. Found only " << i << " of " << entries << " entries.\n";
        abort();
    }

    return result;
}

bits_t pad(bits_t const & bits, size_t boundary)
{
    assert(boundary != 0);

    size_t pad_size = boundary - (bits.size() % boundary);
    assert(pad_size > 0);

    bits_t result = uint2unary(pad_size - 1) + bits;

    assert(result.size() % boundary == 0);
    assert(result.size() - bits.size() <= boundary);

    return result;
}

bits_t unpad(bits_t const & bits)
{
    size_t position = 0;
    unary2uint(bits, position);
    return bits.substr(position);
}

string pack_bits(bits_t bits)
{
    bits = pad(bits, bits_per_char);

    string result(bits.size() / bits_per_char, '\0');

    for (size_t i = 0, j = 0; i < result.size(); ++i)
    {
        unsigned char ch = 0;
        for (size_t k = 0; k < bits_per_char; ++j, ++k)
            ch = (ch << 1) | bit2int(bits[j]);
        result[i] = ch;
    }

    return result;
}

bits_t unpack_bits(string const & in)
{
    string result(in.size() * bits_per_char, '\0');

    for (size_t i = 0, j = 0; i < in.size(); ++i)
    {
        unsigned char ch = in[i];
        for (size_t k = bits_per_char; k-- > 0; ++j)
            result[j] += int2bit((ch >> k) & 1);
    }

    return unpad(result);
}

void test_gamma()
{
    for (unsigned int i = 1; i < 1000; ++i)
    {
        bits_t gamma = uint2gamma(i);
        size_t position = 0;
        unsigned int decoded_i = gamma2uint(gamma, position);
        cerr << i << ' ' << gamma << ' ' << decoded_i << '\n';
        assert(decoded_i == i);
    }
}

void test_prefix_code(code_t const & code)
{
    vector<bits_t> code_words;
    for (auto const & c : code)
        code_words.push_back(c.second);

    sort(code_words.begin(), code_words.end());

    for (size_t i = 0; i + 1 < code_words.size(); ++i)
    {
        cerr << '"' << code_words[i] << "\" \"" << code_words[i + 1] << "\"\n";
        assert(code_words[i + 1].substr(0, code_words[i].size()) != code_words[i]);
    }
}

void test_encode_decode(bool print, string const & input, text_t suffix = text_t { })
{
    text_t text = char2letter(input);
    text.insert(text.end(), suffix.cbegin(), suffix.cend());

    code_t code = build_code(text);
    test_prefix_code(code);
    cerr << "code table ************\n"
        << code << '\n';

    bits_t packed_code = pack_code(code);
    cerr << "packed code ***********\n"
        << "size = " << packed_code.size() << '\n'
        << packed_code << '\n';

    bits_t encoded_text = encode(text, code);

    string packed = pack_bits(packed_code + encoded_text);

    bits_t unpacked = unpack_bits(packed);

    size_t position = 0;
    decode_t unpacked_code = unpack_code(unpacked, position);

    if (print)
    {
        cerr << "code trie *************\n";
        unpacked_code.dump(cerr, input);
    }

    unpacked = unpacked.substr(position);
    assert(unpacked == encoded_text);

    text_t decoded_text = decode(unpacked, unpacked_code);
    assert(decoded_text == text);

    string decoded_input = letter2char(decoded_text);
    assert(decoded_input == input);

    cerr << "encoded size in bits: " << packed_code.size() << " + " << encoded_text.size()
        << " = " << packed_code.size() + encoded_text.size() << '\n'
        << "average bits per char: " << float(packed_code.size() + encoded_text.size()) / decoded_input.size() << '\n'
        << "encoded ***************\n"
        << packed_code + encoded_text << "\n\n"
        << "decoded size in chars: " << decoded_input.size() << '\n'
        << "decoded size in bits: " << decoded_input.size() * bits_per_char << '\n'
        << "decoded ***************\n"
        << decoded_input << '\n'
        << "pack ratio ************\n"
        << float(packed.size()) / decoded_input.size() << '\n';
}

string test(string const & input, bool print)
{
    test_gamma();

    vector<string> test_data {
        "",
        "0",
        "01",
        "abracadabra",
        "banana",
        "pizza",
        "to be or not to be?",
        "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
    };

    for (auto const & text : test_data)
    {
        test_encode_decode(print, text);
        test_encode_decode(print, text, { sentinel });
    }

    test_encode_decode(print, input);

    return "Tests passed.\n";
}

string compress(string const & input, bool print)
{
    text_t text = char2letter(input);
    code_t code = build_code(text);

    if (print)
    {
        decode_t decode { code };
        decode.dump(cerr, input);
    }

    bits_t packed_code = pack_code(code);
    bits_t encoded_text = encode(text, code);
    return pack_bits(packed_code + encoded_text);
}

string decompress(string const & packed, bool print)
{
    bits_t unpacked = unpack_bits(packed);
    size_t position = 0;
    decode_t unpacked_code = unpack_code(unpacked, position);
    unpacked = unpacked.substr(position);
    string result = letter2char(decode(unpacked, unpacked_code));

    if (print)
        unpacked_code.dump(cerr, result);

    return result;
}

bool if_option(int argc, char ** argv, const string & a, const string & b)
{
    char ** argv_begin = argv + 1;
    char ** argv_end = argv + argc;

    return
        find(argv_begin, argv_end, a) != argv_end or
        find(argv_begin, argv_end, b) != argv_end;
}

int main(int argc, char ** argv)
{
    if (argc == 1 or if_option(argc, argv, "-h", "--help"))
    {
        cout << argv[0] << " [{-c|--compress} | {-d|--decompress} | {-t|--test}] [{-p|--print}] < input_file > output_file\n"
            << "    -c, --compress      compress stdin to stdout\n"
            << "    -d, --decompress    decompress stdin to stdout\n"
            << "    -t, --test          run tests, including compression of stdin\n"
            << "    -p, --print         also print code trie(s) in dot format on stderr\n";
        return 0;
    }

    bool print = if_option(argc, argv, "-p", "--print");

    auto function = compress;

    if (if_option(argc, argv, "-c", "--compress"))
        function = compress;
    else if (if_option(argc, argv, "-d", "--decompress"))
        function = decompress;
    else if (if_option(argc, argv, "-t", "--test"))
        function = test;
    else
    {
        cerr << "See help: " << argv[0] << " [-h|--help]\n";
        return 1;
    }

    string input { istreambuf_iterator<char>(cin), istreambuf_iterator<char>() };
    string output = function(input, print);

    cout.write(&output[0], output.size());

    return 0;
}
