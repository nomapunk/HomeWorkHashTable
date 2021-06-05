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
        iterator() = default;
        iterator(std::list<int>::iterator _it, std::vector<std::pair<Node, bool>>& _arr): it(_it), arr(&_arr){}
        std::pair<const KeyType, ValueType>& operator* () {
            return reinterpret_cast<std::pair<const KeyType, ValueType>&>((*arr)[*it].first.entry);
        }
        std::pair<const KeyType, ValueType>* operator-> () {
            return reinterpret_cast<std::pair<const KeyType, ValueType>*>(&(((*arr)[*it].first.entry)));
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
            return reinterpret_cast<const std::pair<const KeyType, ValueType>&>((*arr)[*it].first.entry);
        }
        const std::pair<const KeyType, ValueType>* operator-> () const {
            return reinterpret_cast<const std::pair<const KeyType, ValueType>*>(&(((*arr)[*it].first.entry)));
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
        factor = 0;
        size_all_non_nullptr = 0;
        array.resize(buffer_size);
        _begin = _end = iterator(s.begin(), array);
        _cbegin = _cend = const_iterator(_begin);
    }

    HashMap(HashMap& a): get_hash(a.get_hash) {
        buffer_size = default_size;
        factor = 0;
        size_all_non_nullptr = 0;
        array.resize(buffer_size);
        _begin = _end = iterator(s.begin(), array);
        _cbegin = _cend = const_iterator(_begin);
        for (auto it = a.begin(); it != a.end(); it++) {
            insert(*it);
        }
    }

    template<class T>
    HashMap(T begin, T end, Hash hasher = Hash()): get_hash(hasher) {
        clear();
        for (auto it = begin; it != end; it++) {
            insert(*it);
        }
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> cur, Hash hasher = Hash()): get_hash(hasher) {
        buffer_size = cur.size();
        factor = 0;
        size_all_non_nullptr = 0;
        array.resize(buffer_size);
        _begin = _end = iterator(s.begin(), array);
        _cbegin = _cend = const_iterator(_begin);
        for (auto& t : cur) {
            insert(t);
        }
    }

    int size() const {
        return factor;
    }

    bool empty() const {
        return factor == 0;
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

    iterator find(const KeyType& key) {
        int ind = Find(key);
        if (ind != -1) {
            return array[ind].first.it;
        }
        return _begin;
    }

    const_iterator find(const KeyType key) const {
        int ind = Find(key);
        if (ind != -1) {
            return array[ind].first.const_iterator;
        }
        return _cbegin;
    }

    void insert(const std::pair<KeyType, ValueType>& val) {
        if (Find(val.first) == -1) {
            Add(val.first, val.second);
        }
    }

    void erase(const KeyType& key) {
        if (Find(key) != -1) {
            Remove(key);
        }
    }

    ValueType& operator [](const KeyType& key) {
        int t = Find(key);
        if (t == -1) {
            t = Add(key, ValueType());
        }
        return array[t].first.entry.second;
    }
    const ValueType& at(const KeyType& key) const {
        int t = Find(key);
        if (t == -1) {
            throw std::out_of_range("std::out_of_range");
        }
        return array[t].first.entry.second;
    }
    void clear() {
        array.clear();
        s.clear();
        buffer_size = default_size;
        factor = 0;
        size_all_non_nullptr = 0;
        array.resize(buffer_size);
        _begin = _end = iterator(s.begin(), array);
        _cbegin = _cend = const_iterator(_begin);
    }

    struct Node {
        std::pair<KeyType, ValueType> entry;
        iterator it;
        const_iterator const_iterator;
        bool flag;
        Node() = default;
        Node(KeyType _key, ValueType _val, iterator _it):
                entry(std::pair<KeyType, ValueType>(_key, _val)), it(_it), flag(true) {}
    };

private:
    constexpr static const int default_size = 47;
    constexpr static const double rehash_size = 0.75;
    size_t factor;
    size_t buffer_size;
    size_t size_all_non_nullptr;
    Hash get_hash;

    std::vector<std::pair<Node, bool>> array;
    std::list<int> s;
    iterator _begin, _end;
    const_iterator _cbegin, _cend;

    void Resize() {
        size_t past_buffer_size = buffer_size;
        buffer_size *= 2;
        size_all_non_nullptr = 0;
        factor = 0;
        s.clear();
        _begin = _end = iterator(s.begin(), array);
        _cbegin = _cend = const_iterator(_begin);
        std::vector<std::pair<Node, bool>> tmp(buffer_size);
        array.swap(tmp);
        for (size_t i = 0; i < past_buffer_size; i++) {
            if (tmp[i].second && tmp[i].first.flag) {
                Add(tmp[i].first.entry.first, tmp[i].first.entry.second);
            }
        }
    }
    void Rehash() {
        size_t past_buffer_size = buffer_size;
        size_all_non_nullptr = 0;
        factor = 0;
        std::vector<std::pair<Node, bool>> arr2(buffer_size);
        s.clear();
        _begin = _end = iterator(s.begin(), array);
        _cbegin = _cend = const_iterator(_begin);
        array.swap(arr2);
        for (size_t i = 0; i < past_buffer_size; i++) {
            if (arr2[i].second && arr2[i].first.flag) {
                Add(arr2[i].first.entry.first, arr2[i].first.entry.second);
            }
        }
    }
    int Find(const KeyType& key) {
        for (size_t i = get_hash(key) % buffer_size, cnt = 0; cnt < buffer_size && array[i].second; cnt++) {
            if (array[i].second && array[i].first.entry.first == key && array[i].first.flag == true) {
                return i;
            }
            if (i + 1 == array.size()) {
                i = 0;
            } else {
                i++;
            }
        }
        return -1;
    }
    int Find(const KeyType& key) const {
        for (size_t i = get_hash(key) % buffer_size, cnt = 0; cnt < buffer_size && array[i].second; cnt++) {
            if (array[i].second && array[i].first.entry.first == key && array[i].first.flag == true) {
                return i;
            }
            if (i + 1 == array.size()) {
                i = 0;
            } else {
                i++;
            }
        }
        return -1;
    }

    bool Remove(const KeyType& key) {
        for (size_t i = get_hash(key) % buffer_size, cnt = 0; cnt < buffer_size && array[i].second; cnt++) {
            if (array[i].first.entry.first == key && array[i].first.flag == true) {
                array[i].first.flag = false;
                if (array[i].first.const_iterator == _cend) {
                    ++_end;
                    ++_cend;
                }
                s.erase(array[i].first.it.it);
                factor--;
                return true;
            }
            if (i + 1 == array.size()) {
                i = 0;
            } else {
                i++;
            }
        }
        return false;
    }
    int Add(KeyType key, ValueType val) {
        if (factor + 1 > size_t(rehash_size * buffer_size)) {
            Resize();
        } else if (size_all_non_nullptr > 2 * factor) {
            Rehash();
        }
        bool exists_element = false;
        size_t i;
        for (i = get_hash(key) % buffer_size; array[i].second;) {
            if (array[i].first.entry.first == key) {
                exists_element = true;
                if (array[i].first.flag == true) {
                    factor--;
                }
                break;
            }
            if (i + 1 == array.size()) {
                i = 0;
            } else {
                i++;
            }
        }
        if (exists_element) {
            array[i].first.entry.second = val;
            array[i].first.flag = true;
            array[i].first.it = iterator(s.insert(s.end(), i), array);
            array[i].first.const_iterator = const_iterator(array[i].first.it);
            _end = array[i].first.it;
            _cend = array[i].first.const_iterator;
        } else {
            size_all_non_nullptr++;
            array[i].second = true;
            array[i].first = Node(key, val, iterator(s.insert(s.end(), i), array));
            array[i].first.const_iterator = const_iterator(array[i].first.it);
            _end = array[i].first.it;
            _cend = array[i].first.const_iterator;
        }
        factor++;
        return i;
    }
};
