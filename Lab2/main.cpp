#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <bitset>
#include <unordered_set>
#include <chrono>


namespace ch = std::chrono;

template <typename Func, typename... Args>
ch::milliseconds measure_time(Func func, Args... args) {
    auto start_at = ch::high_resolution_clock::now();
    func(args...);
    auto finish_at = ch::high_resolution_clock::now();
    return ch::duration_cast<ch::milliseconds>(finish_at - start_at);
}

class BloomFilter {
private:
    std::vector<bool> bits;
    size_t m;
    size_t k;
   
    static size_t hash1(const std::string& str) { //djb2
        unsigned long hash = 5381;
        for (char c : str) {
            hash = ((hash << 5) + hash) + c;
        }
        return hash;
    }

    static size_t hash2(const std::string& str) { //sdbm
        unsigned long hash = 0;
        for (char c : str) {
            hash = c + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }

public:
    BloomFilter(size_t bits, size_t hash_count)
        : m(bits), k(hash_count), bits(bits, false) {
    }

    void insert(const std::string& s) {
        size_t h1 = hash1(s) % m;
        size_t h2 = hash2(s) % m;
        for (size_t i = 0; i < k; ++i) {
            size_t hi = (h1 + i * h2) % m;
            bits[hi] = true;
        }
    }

    bool contains(const std::string& s) const {
        size_t h1 = hash1(s) % m;
        size_t h2 = hash2(s) % m;
        for (size_t i = 0; i < k; ++i) {
            size_t hi = (h1 + i * h2) % m;
            if (!bits[hi]) return false;
        }
        return true;
    }

    size_t bit_count() const {
        size_t count = 0;
        for (bool b : bits) if (b) count++;
        return count;
    }
};

std::string testBF(const std::vector<std::pair<char, std::string>>& operations, size_t m, size_t k) {
    BloomFilter bf(m, k);
	std::string result;

    for (const auto& x : operations) {
        char op = x.first;
        const std::string& str = x.second;
        switch (op)
        {
        case '+':
            bf.insert(str);
            break;
        case '?':
        {
            result += (bf.contains(str) ? 'Y' : 'N');
        }
        break;
        default:
            break;
        }
    }

	return result;
}

std::string testHT(const std::vector<std::pair<char, std::string>>& operations) {
    std::unordered_set<std::string> hash_table;
    std::string result;
    for (const auto& x : operations) {
        char op = x.first;
        const std::string& str = x.second;
        switch (op)
        {
        case '+':
            hash_table.insert(str);
            break;
        case '?':
        {
            result += (hash_table.find(str) != hash_table.end() ? 'Y' : 'N');
        }
        break;
        default:
            break;
        }
    }

    return result;
}


int main() {
    constexpr size_t n = 1e6;
    constexpr double p = 0.01;
    const size_t m = (size_t)(-1.0 * n * log(p) / (log(2) * log(2)));
    const size_t k = (size_t)(log(2) * m / n);
	std::cerr << "m: " << m << ", k: " << k << '\n';

	std::vector<std::pair<char, std::string>> operations;
    std::string line;

    std::ifstream input("input.txt");
	std::ofstream output("output.txt");

    while (std::getline(input, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        char op;
        iss >> op;
		if (op == '#') break;
        std::string str;
        iss >> str;
        operations.push_back({ op, str });
    }

	std::string bf_result, ht_result;

    auto bf_time = measure_time([&]() {
        bf_result = testBF(operations, m, k);
		});
    auto ht_time = measure_time([&]() {
        ht_result = testHT(operations);
		});
	std::cerr << "Bloom Filter time: " << bf_time.count() << " ms" << '\n';
	std::cerr << "Hash Table time: " << ht_time.count() << " ms" << '\n';
    
	size_t count = 0, mistake = 0;

    for(int i = 0; i < bf_result.size(); ++i) {
        if(ht_result[i] == 'Y') {
            ++count;
            if(ht_result[i] != 'Y') {
                ++mistake;
            }
        }
	}
	std::cerr << "Total 'Y' in Hash Table: " << count << '\n';
	std::cerr << "False positives in Bloom Filter: " << mistake << '\n';
	std::cerr << "False positive rate: " << (double)mistake / count * 100 << " %" << '\n';

	output << bf_result << '\n';
}
