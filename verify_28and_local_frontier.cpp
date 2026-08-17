#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using U128 = unsigned __int128;

static int msb(U128 x) {
    const uint64_t hi = static_cast<uint64_t>(x >> 64);
    if (hi) return 64 + 63 - __builtin_clzll(hi);
    const uint64_t lo = static_cast<uint64_t>(x);
    return lo ? 63 - __builtin_clzll(lo) : -1;
}

struct TaggedBasis {
    std::array<U128, 66> vec{};
    std::array<uint32_t, 66> coord{};
    int rank = 0;

    bool add(U128 x, uint32_t c) {
        while (x) {
            int p = msb(x);
            if (vec[p]) { x ^= vec[p]; c ^= coord[p]; }
            else { vec[p] = x; coord[p] = c; ++rank; return true; }
        }
        return false;
    }

    bool coordinates(U128 x, uint32_t& c) const {
        c = 0;
        for (int p = 65; p >= 0; --p) {
            if (vec[p] && ((x >> p) & 1)) { x ^= vec[p]; c ^= coord[p]; }
        }
        return x == 0;
    }
};

static int rank24(const std::vector<uint32_t>& xs) {
    std::array<uint32_t, 24> basis{};
    int r = 0;
    for (uint32_t x : xs) {
        while (x) {
            const int p = 31 - __builtin_clz(x);
            if (basis[p]) x ^= basis[p];
            else { basis[p] = x; ++r; break; }
        }
    }
    return r;
}

struct Certificate {
    std::vector<std::pair<uint16_t,uint16_t>> generators;
    std::vector<uint32_t> target_rows;
};

static Certificate read_certificate(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open certificate: " + path);
    enum class Section { none, generators, target } section = Section::none;
    Certificate cert;
    std::string line;
    while (std::getline(in, line)) {
        const auto first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos || line[first] == '#') continue;
        if (line.compare(first, 12, "[GENERATORS]") == 0) { section = Section::generators; continue; }
        if (line.compare(first, 8, "[TARGET]") == 0) { section = Section::target; continue; }
        std::istringstream ss(line.substr(first));
        if (section == Section::generators) {
            std::string a,b,extra;
            if (!(ss >> a >> b) || (ss >> extra)) throw std::runtime_error("bad generator row: " + line);
            const auto u = static_cast<uint16_t>(std::stoul(a,nullptr,16));
            const auto v = static_cast<uint16_t>(std::stoul(b,nullptr,16));
            if (!u || !v || u >= 4096 || v >= 4096) throw std::runtime_error("generator outside F2^12");
            cert.generators.emplace_back(u,v);
        } else if (section == Section::target) {
            std::string q,extra;
            if (!(ss >> q) || (ss >> extra)) throw std::runtime_error("bad target row: " + line);
            const auto x = static_cast<uint32_t>(std::stoul(q,nullptr,16));
            if (x >= (1u<<24)) throw std::runtime_error("target coordinate outside F2^24");
            cert.target_rows.push_back(x);
        } else {
            throw std::runtime_error("data outside a certificate section");
        }
    }
    if (cert.generators.size() != 24 || cert.target_rows.size() != 12)
        throw std::runtime_error("certificate must contain 24 generators and 12 target rows");
    return cert;
}

int main(int argc, char** argv) {
    try {
        const std::string path = argc > 1 ? argv[1] : "frontier_certificate.txt";
        const auto cert = read_certificate(path);

        int pair_index[12][12];
        std::fill(&pair_index[0][0], &pair_index[0][0] + 144, -1);
        int k = 0;
        for (int i=0;i<12;++i) for (int j=i+1;j<12;++j) pair_index[i][j] = k++;
        if (k != 66) throw std::runtime_error("internal pair-index error");

        static U128 wedge_basis[4096][12];
        for (int u=0;u<4096;++u) {
            for (int j=0;j<12;++j) {
                U128 w = 0;
                for (int i=0;i<12;++i) if (((u>>i)&1) && i!=j) {
                    const int lo = std::min(i,j), hi = std::max(i,j);
                    w ^= U128(1) << pair_index[lo][hi];
                }
                wedge_basis[u][j] = w;
            }
        }
        const auto wedge = [&](uint16_t u, uint16_t v) {
            U128 w = 0;
            while (v) {
                const int j = __builtin_ctz(v);
                v &= static_cast<uint16_t>(v-1);
                w ^= wedge_basis[u][j];
            }
            return w;
        };

        TaggedBasis P;
        for (size_t i=0;i<cert.generators.size();++i) {
            const auto [u,v] = cert.generators[i];
            if (!P.add(wedge(u,v), 1u << i)) throw std::runtime_error("outer generator dependence");
        }
        if (P.rank != 24) throw std::runtime_error("published outer span does not have rank 24");

        std::vector<uint32_t> decomposable_coordinates;
        uint64_t total = 0;
        for (uint16_t u=1;u<4096;++u) {
            for (uint16_t v=u+1;v<4096;++v) {
                const uint16_t z = u ^ v;
                if (!(v < z)) continue; // unique representative u < v < u+v for each 2-plane over F2
                ++total;
                uint32_t c = 0;
                if (P.coordinates(wedge(u,v), c)) decomposable_coordinates.push_back(c);
            }
        }
        std::sort(decomposable_coordinates.begin(), decomposable_coordinates.end());
        decomposable_coordinates.erase(std::unique(decomposable_coordinates.begin(), decomposable_coordinates.end()), decomposable_coordinates.end());

        std::vector<uint32_t> rr = cert.target_rows;
        std::vector<int> pivots;
        int r = 0;
        for (int col=23; col>=0 && r<12; --col) {
            int sel = -1;
            for (int i=r;i<12;++i) if ((rr[i]>>col)&1u) { sel=i; break; }
            if (sel < 0) continue;
            std::swap(rr[r], rr[sel]);
            for (int i=0;i<12;++i) if (i!=r && ((rr[i]>>col)&1u)) rr[i] ^= rr[r];
            pivots.push_back(col);
            ++r;
        }
        if (r != 12) throw std::runtime_error("target rows do not have rank 12");

        std::array<bool,24> is_pivot{};
        for (int p : pivots) is_pivot[p] = true;
        std::vector<uint32_t> annihilator_basis;
        for (int free_col=0; free_col<24; ++free_col) if (!is_pivot[free_col]) {
            uint32_t x = 1u << free_col;
            for (int i=0;i<12;++i) if ((rr[i]>>free_col)&1u) x |= 1u << pivots[i];
            annihilator_basis.push_back(x);
        }
        if (annihilator_basis.size() != 12) throw std::runtime_error("unexpected annihilator dimension");

        std::map<int,int> histogram;
        int best = 0;
        for (uint32_t mask=1; mask < (1u<<12); ++mask) {
            uint32_t functional = 0;
            for (int i=0;i<12;++i) if ((mask>>i)&1u) functional ^= annihilator_basis[i];
            std::vector<uint32_t> inside;
            for (uint32_t x : decomposable_coordinates)
                if ((__builtin_parity(x & functional)) == 0) inside.push_back(x);
            const int z = rank24(inside);
            ++histogram[z];
            best = std::max(best, z);
        }

        std::cout << "canonical decomposable bivectors: " << total << "\n";
        std::cout << "published outer-span rank: " << P.rank << "\n";
        std::cout << "decomposable points in published span: " << decomposable_coordinates.size() << "\n";
        std::cout << "target rank: 12\n";
        std::cout << "target-containing hyperplanes checked: 4095\n";
        std::cout << "decomposable-span rank histogram:";
        for (const auto& [rank,count] : histogram) std::cout << " " << rank << ":" << count;
        std::cout << "\nmaximum decomposable span rank: " << best << "\n";

        if (total != 2794155ULL || decomposable_coordinates.size() != 27 || best != 21)
            throw std::runtime_error("frozen local-frontier certificate totals changed");
        std::cout << "PASS: no 23-product outer realization exists entirely inside the frozen 24-product span.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << "\n";
        return 1;
    }
}
