#include <iostream>
#include <fstream>
#include <vector>
#include <forward_list>
#include <string>
#include <sstream>


struct StringSet {
    static const uint32_t base = 31;
	static const uint32_t table_size = 1.2e6;
    std::vector<std::forward_list<std::pair<std::string, size_t>>> table;

	StringSet() : table(table_size) {}

    static uint32_t hash(const std::string& s) {
        uint32_t h = 0;
        for (auto c = s.rbegin(); c != s.rend(); ++c) {
            h = (h * base + (*c - 'a'));
        }
        return h;
    }

    void insert(const std::string& s) {
        auto val = find(s);
        if (val == nullptr) {
            uint32_t h = hash(s);
            auto& bucket = get_bucket(s);
            bucket.emplace_front(std::make_pair(s, 1));
        }
        else
        {
			++val->second;
        }
    }

    void erase(const std::string& s) {
        uint32_t h = hash(s);
        auto& bucket = get_bucket(s);
        auto prev = bucket.before_begin();
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == s) {
                if (it->second > 1)
                    --it->second;
                else
                    bucket.erase_after(prev);
                return;
            }
            prev = it;
        }
    }

    bool contains(const std::string& s)  {
        return find(s) != nullptr;
    }
    
    std::vector<std::string> V2() {
        std::vector<std::string> result;
        for (const auto& bucket : table) {
            for (const auto& p : bucket) {
				if (is_polinom(p.first))
                    result.push_back(p.first);
            }
        }
        return result;
	}

private:
	std::pair<std::string, size_t>* find(const std::string& s) {
        auto& bucket = get_bucket(s);
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->first == s) {
                return &(*it);
            }
        }
        return nullptr;
    }

    decltype(table[0])& get_bucket(const std::string& s) {
        uint32_t h = hash(s);
        return table[h % table_size];
    }

    static bool is_polinom(const std::string& s) {
        for(size_t i = 0; i < s.size() / 2; ++i) {
            if (s[i] != s[s.size() - 1 - i]) {
                return false;
            }
		}
		return true;
    }
};


int main()
{
    StringSet set;
	std::string line;

    std::ifstream input("input.txt");
    std::ofstream output("output.txt");

    while (std::getline(input, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        char op;
        std::string str;

		iss >> op;
        if (op == '#') break;
		iss >> str;
        switch (op)
        {
			case '+':
            {
                set.insert(str);
                break;
            }
			case '-':
            {
                set.erase(str);
                break;
            }
			case '?':
            {
                output << (set.contains(str) ? "Y" : "N");
                break;
            }
        default:
            break;
        }
    }

    auto v = set.V2();
    for (const auto& p : v) {
        std::cerr << p << ' ';
    }
}
