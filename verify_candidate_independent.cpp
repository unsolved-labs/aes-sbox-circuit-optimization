#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

struct Gate {
    std::string kind, out, left, right;
};

static const std::array<uint8_t, 256> AES_SBOX = {
0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};

std::vector<Gate> parse(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open circuit: " + path);
    std::vector<Gate> gates;
    bool active = false;
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream ss(line);
        std::string first;
        if (!(ss >> first)) continue;
        if (first == "%" || first[0] == '%') continue;
        if (line.find("begin SLP") != std::string::npos) { active = true; continue; }
        if (line.find("end SLP") != std::string::npos) { active = false; continue; }
        if (!active) continue;
        Gate g;
        g.kind = first;
        if (!(ss >> g.out >> g.left >> g.right)) throw std::runtime_error("malformed gate line: " + line);
        std::string extra;
        if (ss >> extra) throw std::runtime_error("extra token in gate line: " + line);
        if (g.kind != "XOR" && g.kind != "XNOR" && g.kind != "AND")
            throw std::runtime_error("unknown gate kind: " + g.kind);
        gates.push_back(g);
    }
    if (active) throw std::runtime_error("unterminated SLP");
    return gates;
}

uint8_t eval(const std::vector<Gate>& gates, uint8_t input) {
    std::unordered_map<std::string, bool> v;
    for (int i = 0; i < 8; ++i) v["U" + std::to_string(i)] = ((input >> (7 - i)) & 1) != 0;
    std::unordered_set<std::string> assigned;
    for (int i = 0; i < 8; ++i) assigned.insert("U" + std::to_string(i));
    for (const auto& g : gates) {
        if (assigned.count(g.out)) throw std::runtime_error("duplicate assignment: " + g.out);
        if (!v.count(g.left) || !v.count(g.right)) throw std::runtime_error("use before definition at: " + g.out);
        bool a = v.at(g.left), b = v.at(g.right), z;
        if (g.kind == "XOR") z = a ^ b;
        else if (g.kind == "XNOR") z = !(a ^ b);
        else z = a && b;
        v[g.out] = z;
        assigned.insert(g.out);
    }
    uint8_t out = 0;
    for (int i = 0; i < 8; ++i) {
        const std::string name = "S" + std::to_string(i);
        if (!v.count(name)) throw std::runtime_error("missing output: " + name);
        out |= static_cast<uint8_t>(v.at(name)) << (7 - i);
    }
    return out;
}

int main(int argc, char** argv) {
    try {
        const std::string path = argc > 1 ? argv[1] : "aes-sbox-fwd-a29-ad5-g139-gd36-xx110-13.ncff.txt";
        auto gates = parse(path);
        int n_and = 0, n_xor = 0, n_xnor = 0;
        std::unordered_map<std::string, int> depth, adepth;
        for (int i = 0; i < 8; ++i) { depth["U" + std::to_string(i)] = 0; adepth["U" + std::to_string(i)] = 0; }
        std::array<int, 6> profile{};
        std::unordered_set<std::string> seen;
        for (int i = 0; i < 8; ++i) seen.insert("U" + std::to_string(i));
        for (const auto& g : gates) {
            if (seen.count(g.out)) throw std::runtime_error("duplicate assignment: " + g.out);
            if (!depth.count(g.left) || !depth.count(g.right)) throw std::runtime_error("use before definition at: " + g.out);
            if (g.kind == "AND") ++n_and; else if (g.kind == "XOR") ++n_xor; else ++n_xnor;
            depth[g.out] = std::max(depth.at(g.left), depth.at(g.right)) + 1;
            adepth[g.out] = std::max(adepth.at(g.left), adepth.at(g.right)) + (g.kind == "AND" ? 1 : 0);
            if (g.kind == "AND") {
                if (adepth[g.out] < 1 || adepth[g.out] > 5) throw std::runtime_error("unexpected AND depth");
                ++profile[adepth[g.out]];
            }
            seen.insert(g.out);
        }
        if (gates.size() != 139 || n_and != 29 || n_xor != 97 || n_xnor != 13)
            throw std::runtime_error("gate tally mismatch");
        int gate_depth = 0, and_depth = 0;
        for (int i = 0; i < 8; ++i) {
            gate_depth = std::max(gate_depth, depth.at("S" + std::to_string(i)));
            and_depth = std::max(and_depth, adepth.at("S" + std::to_string(i)));
        }
        const std::array<int,5> expected_profile{9,1,2,2,15};
        for (int i = 1; i <= 5; ++i) if (profile[i] != expected_profile[i-1]) throw std::runtime_error("AND-depth profile mismatch");
        if (gate_depth != 36 || and_depth != 5) throw std::runtime_error("depth mismatch");
        for (int x = 0; x < 256; ++x) {
            auto got = eval(gates, static_cast<uint8_t>(x));
            if (got != AES_SBOX[x]) {
                std::cerr << "AES mismatch at 0x" << std::hex << x << ": got 0x" << int(got)
                          << ", expected 0x" << int(AES_SBOX[x]) << "\n";
                return 2;
            }
        }
        std::cout << "PASS independent C++ verifier\n";
        std::cout << "gates: 139 = 29 AND + 97 XOR + 13 XNOR\n";
        std::cout << "gate depth: 36\nAND-depth: 5\n";
        std::cout << "AND-depth profile: 9 1 2 2 15\n";
        std::cout << "FIPS 197 S-box table: 256/256 inputs matched\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << "\n";
        return 1;
    }
}
