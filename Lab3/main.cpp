#include <iostream>
#include <vector>
#include <array>
#include <random>
#include <chrono>

using namespace std;
namespace ch = chrono;
using clockt = chrono::high_resolution_clock;


constexpr int CRC_WIDTH = 5;
constexpr uint8_t POLY = 0x05;    // x^5 + x^2 + 1
constexpr int K_BITS = 1000;
constexpr uint8_t CRC_WIDTH_MASK = (1 << CRC_WIDTH) - 1; //0d00..01.11



uint8_t reflect_bits(uint8_t v, int n) {
    uint8_t r = 0;
    for (int i = 0; i < n; ++i) if (v & (1 << i)) r |= 1 << (n - 1 - i);
    return r;
}

vector<uint8_t> gen_random_bits_packed(int bits, std::mt19937& rng) {
    int bytes = (bits + 7) / 8;
    vector<uint8_t> data(bytes, 0);
    uniform_int_distribution<int> d(0, 1);
    for (int i = 0; i < bits; ++i) {
        if (d(rng)) data[i / 8] |= (1 << (7 - (i % 8)));
    }
    return data;
}

inline uint8_t get_bit(const vector<uint8_t>& data, int i) {
	if (i / 8 >= data.size()) return 0;
    uint8_t b = data[i / 8];
    return (b >> (7 - (i % 8))) & 1;
}

uint8_t crc5_seq_msb(const vector<uint8_t>& data, int nbits) {
    uint8_t reg = 0;
    for (int i = 0; i < nbits + CRC_WIDTH; ++i) {
        int bit = get_bit(data, i);
        int msb = (reg >> (CRC_WIDTH - 1)) & 1;
        reg = ((reg << 1) & CRC_WIDTH_MASK) | bit;
        if (msb) reg ^= POLY;
    }
    return reg & CRC_WIDTH_MASK;
}

array<uint8_t, 256> build_table_msb() {
    array<uint8_t, 256> table;
    for (int b = 0; b < 256; ++b) {
        table[b] = crc5_seq_msb({ (uint8_t)b }, 8);
    }
    return table;
}

uint8_t crc5_table_msb(const vector<uint8_t>& data, int nbits, const array<uint8_t, 256>& table) {
    uint8_t reg = 0;
    int nbytes = (nbits + 7) / 8;
    for (int i = 0; i < nbytes; ++i) {
        reg = table[(reg << (8 - CRC_WIDTH)) ^ data[i]];
    }
    return reg & CRC_WIDTH_MASK;
}

uint8_t crc5_seq_lsb(const vector<uint8_t>& data, int nbits) {
    uint8_t reg = 0;
    uint8_t revpoly = reflect_bits(POLY, CRC_WIDTH);
    for (int i = 0; i < nbits + CRC_WIDTH; ++i) {
        int bit = get_bit(data, nbits - i - 1);
        int lsb = reg & 1;
        reg = (reg >> 1) | (bit << (CRC_WIDTH - 1));
        if (lsb) reg ^= revpoly;
    }
    return reg & CRC_WIDTH_MASK;
}

array<uint8_t, 256> build_table_lsb() {
    array<uint8_t, 256> table;
    for (int b = 0; b < 256; ++b) {
        table[b] = crc5_seq_lsb({ (uint8_t)b }, 8);
    }
    return table;
}

uint8_t crc5_table_lsb(const vector<uint8_t>& data, int nbits, const array<uint8_t, 256>& table) {
    uint8_t reg = 0;
    int nbytes = (nbits + 7) / 8;
    for (int i = 0; i < nbytes; ++i) {
        reg = table[reg ^ data[nbytes - i - 1]];
    }
    return reg & CRC_WIDTH_MASK;
}


template<typename F>
double time_many_runs(F f, int trials) {
    auto t0 = clockt::now();
    for (int i = 0; i < trials; ++i) f();
    auto t1 = clockt::now();
    double mili = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();
    return mili / trials;
}

int main() {
    std::mt19937 rng(1234567);
    vector<uint8_t> msg = gen_random_bits_packed(K_BITS, rng);

    auto table_msb = build_table_msb();
    auto table_lsb = build_table_lsb();

    const int TRIALS = 100000;

    uint8_t r1 = crc5_seq_msb(msg, K_BITS);
    uint8_t r2 = crc5_table_msb(msg, K_BITS, table_msb);
    uint8_t r3 = crc5_seq_lsb(msg, K_BITS);
    uint8_t r4 = crc5_table_lsb(msg, K_BITS, table_lsb);

    cout << " seq_msb : 0x" << hex << +r1 << dec << "\n";
    cout << " tab_msb : 0x" << hex << +r2 << dec << "\n";
    cout << " seq_lsb : 0x" << hex << +r3 << dec << "\n";
    cout << " tab_lsb : 0x" << hex << +r4 << dec << "\n\n";
    
    double t_seq_msb = time_many_runs([&]() { crc5_seq_msb(msg, K_BITS);  }, TRIALS);
    double t_tab_msb = time_many_runs([&]() { crc5_table_msb(msg, K_BITS, table_msb);  }, TRIALS);
    double t_seq_lsb = time_many_runs([&]() { crc5_seq_lsb(msg, K_BITS);  }, TRIALS);
    double t_tab_lsb = time_many_runs([&]() { crc5_table_lsb(msg, K_BITS, table_lsb);  }, TRIALS);

	double t_table_msb = time_many_runs(build_table_msb, 1);
	double t_table_lsb = time_many_runs(build_table_lsb, 1);

    cout << " seq_msb : " << t_seq_msb << " ms\n";
    cout << " tab_msb : " << t_tab_msb << " ms\n";
    cout << " seq_lsb : " << t_seq_lsb << " ms\n";
    cout << " tab_lsb : " << t_tab_lsb << " ms\n";
	cout << "\n";
	cout << " build_table_msb : " << t_table_msb << " ms\n";
	cout << " build_table_lsb : " << t_table_lsb << " ms\n";

    return 0;
}
