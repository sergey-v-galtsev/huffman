#include <cassert>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <string>
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

bits_t encode(text_t const & text, code_t const & code)
{
    bits_t result;
    for (auto l : text)
    {
        auto c = code.find(l);
        if (c != code.end())
            result += c->second;
        else
            cerr << "Can not encode letter " << l << " ("
                << (is_char(l) ? string(1, letter2char(l)) : "not a char") << ")\n";
    }
    return result;
}

// FIXME: inoptimal, use trie instead
text_t decode(bits_t const & text, code_t const & code)
{
    text_t result;

    for (size_t i = 0; i < text.size(); )
    {
        letter_t l;
        size_t s = 0;

        for (auto const & c : code)
            if (c.second.size() <= text.size() - i and
                c.second == text.substr(i, c.second.size()))
            {
                l = c.first;
                s = c.second.size();
                break;
            }

        if (s)
            result.push_back(l);
        else
        {
            cerr << "Failed to decode bit sequence from position " << i << ": " << text.substr(i, 100) << "...\n";
            break;
        }

        i += s;
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

huffman_t merge(huffman_t a, huffman_t b)
{
    huffman_t result;

    result.count = a.count + b.count;

    a.prepend(int2bit(0));
    b.prepend(int2bit(1));
    result.code = a.code;
    result.code.insert(b.code.begin(), b.code.end());

    return result;
}

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
        huffman.push(merge(a, b));
    }

    return huffman.top().code;
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
    string result;

    bits = pad(bits, CHAR_BIT);

    for (size_t i = 0; i < bits.size(); )
    {
        uint8_t ch = 0;
        for (size_t j = 0; j < CHAR_BIT; ++j, ++i)
            ch = (ch << 1) | bit2int(bits[i]);
        result += static_cast<char>(ch);
    }

    return result;
}

bits_t unpack_bits(string in)
{
    string result;

    for (uint8_t ch : in)
        for (size_t j = CHAR_BIT; j--; )
            result += int2bit((ch >> j) & 1);

    return unpad(result);
}

int main()
{
    string input { istreambuf_iterator<char>(cin), istreambuf_iterator<char>() };

    text_t text = char2letter(input);
    text.push_back(sentinel);

    code_t code = build_code(text);

    cerr << "code table ************\n"
        << code << '\n';

    bits_t encoded_text = encode(text, code);

    string packed = pack_bits(encoded_text);
    cout.write(&packed[0], packed.size());

    bits_t unpacked = unpack_bits(packed);

    string decoded_text = letter2char(decode(unpacked, code));
    assert(decoded_text == input);

    assert(unpacked == encoded_text);

    cerr << "encoded size in bits: " << encoded_text.size() << '\n'
        << "average bits per char: " << float(encoded_text.size()) / decoded_text.size() << '\n'
        << "encoded ***************\n"
        << encoded_text << "\n\n"
        << "decoded size in chars: " << decoded_text.size() << '\n'
        << "decoded size in bits: " << decoded_text.size() * CHAR_BIT << '\n'
        << "decoded ***************\n"
        << decoded_text << '\n';

    return 0;
}
