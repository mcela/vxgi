#define STB_DS_IMPLEMENTATION
#define STBDS_NO_SHORT_NAMES
#include <stb/stb_ds.h> // dynamic vector, hashmap

template<typename T>
inline T& Array<T>::operator[] (int i) {
	assert(data);
	return data[i];
}
template<typename T>
inline const T& Array<T>::operator[] (int i) const {
	assert(data);
	return data[i];
}

template<typename T>
typename Array<T>::Iterator Array<T>::begin() {
	return Array<T>::Iterator(data, 0);
}
template<typename T>
typename Array<T>::Iterator Array<T>::end() {
	return Array<T>::Iterator(data, array::size(*this));
}

template<typename T>
typename Array<T>::Iterator Array<T>::Iterator::operator++() {
	index++;
	return *this;
}
template<typename T>
bool Array<T>::Iterator::operator!=(const Array<T>::Iterator& other) const {
	return index != other.index;
}
template<typename T>
T& Array<T>::Iterator::operator*() {
	return data[index];
}

namespace array
{
	template<typename T>
	void uninit(Array<T>& arr) {
		clear(arr);
	}
	template<typename T>
	void clear(Array<T>& arr) {
		if (arr.data)
			stbds_arrfree(arr.data);
		arr.data = 0;
	}

	template<typename T>
	umm size(Array<T>& arr) {
		return (umm) (stbds_arrlenu(arr.data));
	}
	template<typename T>
	umm size_in_bytes(Array<T>& arr) {
		return (umm) (stbds_arrlenu(arr.data) * sizeof(T));
	}
	template<typename T>
	void set_length(Array<T>& arr, int length) {
		stbds_arrsetlen(arr.data, length);
	}
	template<typename T>
	void ensure_capacity(Array<T>& arr, int capacity) {
		stbds_arrsetcap(arr.data, capacity);
	}
	template<typename T>
	umm get_capacity(Array<T>& arr) {
		return stbds_arrcap(arr.data);
	}

	template<typename T>
	umm add(Array<T>& arr, const T& t) {
		stbds_arrput(arr.data, t);
		return size(arr) - 1;
	}
	template<typename T>
	void insert(Array<T>& arr, const T& t, int index) {
		stbds_arrins(arr.data, index, t);
	}
	template<typename T>
	umm reserve_before_insert(Array<T>& arr, int amount) {
		return stbds_arraddnoff(arr.data, amount);
	}
	template<typename T>
	umm reserve_before_insert_at(Array<T>& arr, int amount, int index) {
		stbds_arrins(arr.data, index, amount);
		return index;
	}
	template<typename T>
	void insert_reserved(Array<T>& arr, const T& t, umm reservedAt, int indexOffset) {
		arr.data[reservedAt + indexOffset] = t;
	}

	template<typename T>
	void remove(Array<T>& arr, int index) {
		stbds_arrdel(arr.data, index);
	}
	template<typename T>
	void remove_and_swap_last(Array<T>& arr, int index) {
		stbds_arrdelswap(arr.data, index);
	}
	template<typename T>
	void remove_many(Array<T>& arr, int index, int amount) {
		stbds_arrdeln(arr.data, index, amount);
	}
	template<typename T>
	void pop(Array<T>& arr) {
		stbds_arrpop(arr.data);
	}
}

namespace hashmap
{
	template<typename K, typename V> void clear(Hashmap<K,V>& hashmap) {
		if (hashmap.data)
			stbds_hmfree(hashmap.data);
		hashmap.data = 0;
	}
	template<typename K, typename V> void uninit(Hashmap<K,V>& hashmap) {
		clear(hashmap);
	}
	template<typename K, typename V> umm size(Hashmap<K,V>& hashmap) {
		return stbds_hmlenu(hashmap.data);
	}
	template<typename K, typename V> void insert(Hashmap<K,V>& hashmap, K key, V value) {
		stbds_hmput(hashmap.data, key, value);
	}
	template<typename K, typename V> bool remove(Hashmap<K,V>& hashmap, K key) {
		int wasInHashmap = stbds_hmdel(hashmap.data, key);
		return (wasInHashmap == 1) ? true : false;
	}
	template<typename K, typename V> void set_default(Hashmap<K,V>& hashmap, V value) {
		stbds_hmdefault(hashmap.data, value);
	}
	template<typename K, typename V> void set_default_entry(Hashmap<K,V>& hashmap, K key, V value) {
		typename Hashmap<K,V>::Entry e = { key, value };
		stbds_hmdefaults(hashmap.data, e);
	}
	template<typename K, typename V> bool contains(Hashmap<K,V>& hashmap, K key) {
		return get_index_of(hashmap, key) != -1;
	}
	template<typename K, typename V> V get(Hashmap<K,V>& hashmap, K key) {
		return stbds_hmget(hashmap.data, key);
	}
	template<typename K, typename V> V get_or_default(Hashmap<K,V>& hashmap, K key) {
		return stbds_hmgetp(hashmap.data, key)->value;
	}
	template<typename K, typename V> int get_index_of(Hashmap<K,V>& hashmap, K key) {
		return stbds_hmgeti(hashmap.data, key);
	}
	template<typename K, typename V> auto* get_entry(Hashmap<K,V>& hashmap, K key) {
		return stbds_hmgetp_null(hashmap.data, key);
	}
	template<typename K, typename V> auto* get_entry_or_default(Hashmap<K,V>& hashmap, K key) {
		return stbds_hmgetp(hashmap.data, key);
	}

	template<typename V> void uninit(Hashmap<const char*,V>& hashmap) {
		clear(hashmap);
	}
	template<typename V> void clear(Hashmap<const char*,V>& hashmap) {
		if (hashmap.data)
			stbds_shfree(hashmap.data);
		hashmap.data = 0;
	}
	template<typename V> umm size(Hashmap<const char*,V>& hashmap) {
		return stbds_shlenu(hashmap.data);
	}
	template<typename V> void insert(Hashmap<const char*,V>& hashmap, const char* key, V value) {
		stbds_shput(hashmap.data, key, value);
	}
	template<typename V> bool remove(Hashmap<const char*,V>& hashmap, const char* key) {
		int wasInHashmap = stbds_shdel(hashmap.data, key);
		return (wasInHashmap == 1) ? true : false;
	}
	template<typename V> void set_default(Hashmap<const char*,V>& hashmap, V value) {
		stbds_shdefault(hashmap.data, value);
	}
	template<typename V> void set_default_entry(Hashmap<const char*,V>& hashmap, const char* key, V value) {
		typename Hashmap<const char*,V>::Entry e = { key, value };
		stbds_shdefaults(hashmap.data, e);
	}
	template<typename V> bool contains(Hashmap<const char*,V>& hashmap, const char* key) {
		return get_index_of(hashmap, key) != -1;
	}
	template<typename V> V get(Hashmap<const char*,V>& hashmap, const char* key) {
		return stbds_shget(hashmap.data, key);
	}
	template<typename V> V get_or_default(Hashmap<const char*,V>& hashmap, const char* key) {
		return stbds_shgetp(hashmap.data, key)->value;
	}
	template<typename V> int get_index_of(Hashmap<const char*,V>& hashmap, const char* key) {
		return stbds_shgeti(hashmap.data, key);
	}
	template<typename V> auto* get_entry(Hashmap<const char*,V>& hashmap, const char* key) {
		return stbds_shgetp_null(hashmap.data, key);
	}
	template<typename V> auto* get_entry_or_default(Hashmap<const char*,V>& hashmap, const char* key) {
		return stbds_shgetp(hashmap.data, key);
	}
}