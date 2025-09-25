#include <iostream>
#include <vector>
#include <forward_list>


struct StringSet {
    static const uint32_t base = 31;
	static const uint32_t table_size = 1.2e6;
    std::vector<std::forward_list<std::pair<std::string, int>>> table;

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
    
    std::vector<std::pair<std::string, int>> V1() {
        std::vector<std::pair<std::string, int>> result;
        for (const auto& bucket : table) {
            for (const auto& p : bucket) {
				if (p.second > 1)
                    result.push_back(p);
            }
        }
        return result;
	}

private:
	std::pair<std::string, int>* find(const std::string& s) {
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
};


int main()
{
	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);
	std::cout.tie(nullptr);
    StringSet set;
	char command = '0';
	std::string s;
    while (true)
    {
		std::cin >> command;
        switch (command)
        {
            case '#':
            {
                return 0;
            }
			case '+':
            {
                std::cin >> s;
                set.insert(s);
                break;
            }
			case '-':
            {
                std::cin >> s;
                set.erase(s);
                break;
            }
			case '?':
            {
                std::cin >> s;
                std::cout << (set.contains(s) ? "YES" : "NO") << std::endl;
                break;
            }
			case 'V':
			{
				auto v = set.V1();
				for (const auto& p : v) {
					std::cout << p.first << " " << p.second << std::endl;
				}
				break;
			}
        default:
            break;
        }
    }

}
