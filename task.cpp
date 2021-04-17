#include <vector>
#include <iostream>
#include <list>
#include <stdexcept>
template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
public:
    struct Node;

    struct iterator {
        std::list<int>::iterator it;
        std::vector<std::pair<Node, bool>>* arr;
        iterator() {}
        iterator(std::list<int>::iterator _it, std::vector<std::pair<Node, bool>>& _arr): it(_it), arr(&_arr){}
        std::pair<const KeyType, ValueType>& operator* () {
            return reinterpret_cast<std::pair<const KeyType, ValueType>&>((*arr)[*it].first.key_and_val);
        }
        std::pair<const KeyType, ValueType>* operator-> () {
            return reinterpret_cast<std::pair<const KeyType, ValueType>*>(&(((*arr)[*it].first.key_and_val)));
        }
        iterator& operator++ () {
            it--;
            return *this;
        }
        iterator operator++ (int) {
            iterator res = *this;
            it--;
            return res;
        }
        bool operator== (iterator other) const {
            return other.it == it;
        }
        bool operator!= (iterator other) const {
            return !(other.it == it);
        }
    };

    struct const_iterator {
        std::list<int>::iterator it;
        std::vector<std::pair<Node, bool>>* arr;
        const_iterator() {}
        const_iterator(iterator& _it): it(_it.it), arr(_it.arr){}
        const std::pair<const KeyType, ValueType>& operator* () const {
            return reinterpret_cast<const std::pair<const KeyType, ValueType>&>((*arr)[*it].first.key_and_val);
        }
        const std::pair<const KeyType, ValueType>* operator-> () const {
            return reinterpret_cast<const std::pair<const KeyType, ValueType>*>(&(((*arr)[*it].first.key_and_val)));
        }
        const_iterator& operator++ () {
            it--;
            return *this;
        }
        const_iterator operator++ (int) {
            const_iterator res = *this;
            it--;
            return res;
        }
        bool operator== (const_iterator other) const {
            return other.it == it;
        }
        bool operator!= (const_iterator other) const {
            return !(other.it == it);
        }
    };

    HashMap(Hash hasher = Hash()): get_hash(hasher) {
        buffer_size = default_size;
        _size = 0;
        size_all_non_nullptr = 0;
        arr.resize(buffer_size);
        _begin = _end = iterator(s.begin(), arr);
        _cbegin = _cend = const_iterator(_begin);
    }

    HashMap(HashMap& a): get_hash(a.get_hash) {
        buffer_size = default_size;
        _size = 0;
        size_all_non_nullptr = 0;
        arr.resize(buffer_size);
        _begin = _end = iterator(s.begin(), arr);
        _cbegin = _cend = const_iterator(_begin);
        for (auto it = a.begin(); it != a.end(); it++) {
            insert(*it);
        }
    }

    template<class T>
    HashMap(T begin, T end, Hash hasher = Hash()): get_hash(hasher) {
        std::vector<std::pair<KeyType, ValueType>> cur;
        while (begin != end) {
            cur.push_back(*begin);
            begin++;
        }
        buffer_size = cur.size();
        _size = 0;
        size_all_non_nullptr = 0;
        arr.resize(buffer_size);
        begin = end = iterator(s.begin(), arr);
        _cbegin = _cend = const_iterator(_begin);
        for (auto& t : cur) {
            insert(t);
        }
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> cur, Hash hasher = Hash()): get_hash(hasher) {
        buffer_size = cur.size();
        _size = 0;
        size_all_non_nullptr = 0;
        arr.resize(buffer_size);
        _begin = _end = iterator(s.begin(), arr);
        _cbegin = _cend = const_iterator(_begin);
        for (auto& t : cur) {
            insert(t);
        }
    }

    int size() const {
        return _size;
    }

    bool empty() const {
        return _size == 0;
    }

    Hash hash_function() const {
        return get_hash;
    }

    iterator begin() {
        return _end;
    }

    iterator end() {
        return _begin;
    }

    const_iterator begin() const {
        return _cend;
    }

    const_iterator end() const {
        return _cbegin;
    }

    iterator find(const KeyType key) {
        int ind = Find(key);
        if (ind != -1) {
            return arr[ind].first.it;
        }
        return _begin;
    }
    const_iterator find(const KeyType key) const {
        int ind = Find(key);
        if (ind != -1) {
            return arr[ind].first.cit;
        }
        return _cbegin;
    }

    void insert(std::pair<KeyType, ValueType> val) {
        if (Find(val.first) == -1) {
            Add(val.first, val.second);
        }
    }

    void erase(KeyType key) {
        if (Find(key) != -1) {
            Remove(key);
        }
    }

    ValueType& operator [](const KeyType& key) {
        int t = Find(key);
        if (t == -1) {
            t = Add(key, ValueType());
        }
        return arr[t].first.key_and_val.second;
    }
    const ValueType& at(const KeyType& key) const {
        int t = Find(key);
        if (t == -1) {
            throw std::out_of_range("std::out_of_range");
        }
        return arr[t].first.key_and_val.second;
    }
    void clear() {
        arr.clear();
        s.clear();
        buffer_size = default_size;
        _size = 0;
        size_all_non_nullptr = 0;
        arr.resize(buffer_size);
        _begin = _end = iterator(s.begin(), arr);
        _cbegin = _cend = const_iterator(_begin);
    }

    struct Node {
        std::pair<KeyType, ValueType> key_and_val;
        iterator it;
        const_iterator cit;
        bool flag;
        Node() {}
        Node(KeyType _key, ValueType _val, iterator _it):
                key_and_val(std::make_pair(_key, _val)), it(_it), flag(true) {}
    };

private:
    static const int default_size = 47;
    constexpr static const double rehash_size = 0.75;
    size_t _size;
    size_t buffer_size;
    size_t size_all_non_nullptr;
    Hash get_hash;

    std::vector<std::pair<Node, bool>> arr;
    std::list<int> s;
    iterator _begin, _end;
    const_iterator _cbegin, _cend;

    void Resize() {
        int past_buffer_size = buffer_size;
        buffer_size *= 2;
        size_all_non_nullptr = 0;
        _size = 0;
        s.clear();
        _begin = _end = iterator(s.begin(), arr);
        _cbegin = _cend = const_iterator(_begin);
        std::vector<std::pair<Node, bool>> arr2(buffer_size);
        arr.swap(arr2);
        for (int i = 0; i < past_buffer_size; i++) {
            if (arr2[i].second && arr2[i].first.flag) {
                Add(arr2[i].first.key_and_val.first, arr2[i].first.key_and_val.second);
            }
        }
    }
    void Rehash() {
        int past_buffer_size = buffer_size;
        size_all_non_nullptr = 0;
        _size = 0;
        std::vector<std::pair<Node, bool>> arr2(buffer_size);
        s.clear();
        _begin = _end = iterator(s.begin(), arr);
        _cbegin = _cend = const_iterator(_begin);
        arr.swap(arr2);
        for (int i = 0; i < past_buffer_size; i++) {
            if (arr2[i].second && arr2[i].first.flag) {
                Add(arr2[i].first.key_and_val.first, arr2[i].first.key_and_val.second);
            }
        }
    }
    int Find(const KeyType& key) {
        for (size_t i = get_hash(key) % buffer_size, cnt = 0; cnt < buffer_size && arr[i].second; cnt++) {
            if (arr[i].second && arr[i].first.key_and_val.first == key && arr[i].first.flag == true) {
                return i;
            }
            if (i + 1 == arr.size()) {
                i = 0;
            } else {
                i++;
            }
        }
        return -1;
    }
    int Find(const KeyType& key) const {
        for (size_t i = get_hash(key) % buffer_size, cnt = 0; cnt < buffer_size && arr[i].second; cnt++) {
            if (arr[i].second && arr[i].first.key_and_val.first == key && arr[i].first.flag == true) {
                return i;
            }
            if (i + 1 == arr.size()) {
                i = 0;
            } else {
                i++;
            }
        }
        return -1;
    }

    bool Remove(const KeyType& key) {
        for (size_t i = get_hash(key) % buffer_size, cnt = 0; cnt < buffer_size && arr[i].second; cnt++) {
            if (arr[i].first.key_and_val.first == key && arr[i].first.flag == true) {
                arr[i].first.flag = false;
                if (arr[i].first.cit == _cend) {
                    ++_end;
                    ++_cend;
                }
                s.erase(arr[i].first.it.it);
                _size--;
                return i;
            }
            if (i + 1 == arr.size()) {
                i = 0;
            } else {
                i++;
            }
        }
        return false;
    }
    int Add(KeyType key, ValueType val) {
        if (_size + 1 > size_t(rehash_size * buffer_size)) {
            Resize();
        } else if (size_all_non_nullptr > 2 * _size) {
            Rehash();
        }
        bool fast_insert = false;
        size_t i;
        for (i = get_hash(key) % buffer_size; arr[i].second;) {
            if (arr[i].first.key_and_val.first == key) {
                fast_insert = true;
                if (arr[i].first.flag == true) {
                    _size--;
                }
                break;
            }
            if (i + 1 == arr.size()) {
                i = 0;
            } else {
                i++;
            }
        }
        if (fast_insert) {
            arr[i].first.key_and_val.second = val;
            arr[i].first.flag = true;
            arr[i].first.it = iterator(s.insert(s.end(), i), arr);
            arr[i].first.cit = const_iterator(arr[i].first.it);
            _end = arr[i].first.it;
            _cend = arr[i].first.cit;
        } else {
            size_all_non_nullptr++;
            arr[i].second = true;
            arr[i].first = Node(key, val, iterator(s.insert(s.end(), i), arr));
            arr[i].first.cit = const_iterator(arr[i].first.it);
            _end = arr[i].first.it;
            _cend = arr[i].first.cit;
        }
        _size++;
        return i;
    }
};
