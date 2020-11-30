#pragma once

template<typename T>
struct Array
{
	struct Iterator {
		T*  data = 0;
		int index = 0;

		Iterator(T* d, int i) : data(d), index(i) {}
		T&       operator*();
		bool     operator!=(const Iterator& other) const;
		Iterator operator++();
	};

	T* data = 0;

	inline T& operator[] (int i);
	inline const T& operator[] (int i) const;

	Iterator begin();
	Iterator end();

	#ifdef DEBUG
	~Array() {
		assert(data == 0);
	}
	#endif
};

template<typename K, typename V>
struct Hashmap
{
	struct Entry {
		K key;
		V value;
	};

	Entry* data = 0;

	#ifdef DEBUG
	~Hashmap() {
		assert(data == 0);
	}
	#endif
};

namespace array
{
	template<typename T> void uninit(Array<T>& arr);
	template<typename T> void clear(Array<T>& arr);

	template<typename T> umm  size(Array<T>& arr);
	template<typename T> umm  size_in_bytes(Array<T>& arr);
	template<typename T> void set_length(Array<T>& arr, int length);
	template<typename T> void ensure_capacity(Array<T>& arr, int capacity);
	template<typename T> umm  get_capacity(Array<T>& arr);

	template<typename T> umm  add(Array<T>& arr, const T& t); // returns the index
	template<typename T> void insert(Array<T>& arr, const T& t, int index);
	template<typename T> umm  reserve_before_insert(Array<T>& arr, int amount);
	template<typename T> umm  reserve_before_insert_at(Array<T>& arr, int amount, int index);
	template<typename T> void insert_reserved(Array<T>& arr, const T& t, umm reservedAt, int indexOffset);

	template<typename T> void remove(Array<T>& arr, int index);
	template<typename T> void remove_and_swap_last(Array<T>& arr, int index);
	template<typename T> void remove_many(Array<T>& arr, int index, int amount);
	template<typename T> void pop(Array<T>& arr);
}

namespace hashmap
{
	template<typename K, typename V> void  uninit(Hashmap<K,V>& hashmap);
	template<typename K, typename V> void  clear(Hashmap<K,V>& hashmap);
	template<typename K, typename V> umm   size(Hashmap<K,V>& hashmap);
	template<typename K, typename V> void  insert(Hashmap<K,V>& hashmap, K key, V value);
	template<typename K, typename V> bool  remove(Hashmap<K,V>& hashmap, K key);
	template<typename K, typename V> void  set_default(Hashmap<K,V>& hashmap, V value);
	template<typename K, typename V> void  set_default_entry(Hashmap<K,V>& hashmap, K key, V value);
	template<typename K, typename V> bool  contains(Hashmap<K,V>& hashmap, K key);
	template<typename K, typename V> V     get(Hashmap<K,V>& hashmap, K key);
	template<typename K, typename V> V     get_or_default(Hashmap<K,V>& hashmap, K key);
	template<typename K, typename V> int   get_index_of(Hashmap<K,V>& hashmap, K key); // returns -1 if not present
	template<typename K, typename V> auto* get_entry(Hashmap<K,V>& hashmap, K key); // returns Hashmap::Entry*
	template<typename K, typename V> auto* get_entry_or_default(Hashmap<K,V>& hashmap, K key); // returns Hashmap::Entry*

	// const char* hashmap
	template<typename V> void  uninit(Hashmap<const char*,V>& hashmap);
	template<typename V> void  clear(Hashmap<const char*,V>& hashmap);
	template<typename V> umm   size(Hashmap<const char*,V>& hashmap);
	template<typename V> void  insert(Hashmap<const char*,V>& hashmap, const char* key, V value);
	template<typename V> bool  remove(Hashmap<const char*,V>& hashmap, const char* key);
	template<typename V> void  set_default(Hashmap<const char*,V>& hashmap, V value);
	template<typename V> void  set_default_entry(Hashmap<const char*,V>& hashmap, const char* key, V value);
	template<typename V> bool  contains(Hashmap<const char*,V>& hashmap, const char* key);
	template<typename V> V     get(Hashmap<const char*,V>& hashmap, const char* key);
	template<typename V> V     get_or_default(Hashmap<const char*,V>& hashmap, const char* key);
	template<typename V> int   get_index_of(Hashmap<const char*,V>& hashmap, const char* key); // returns -1 if not present
	template<typename V> auto* get_entry(Hashmap<const char*,V>& hashmap, const char* key); // returns Hashmap::Entry*
	template<typename V> auto* get_entry_or_default(Hashmap<const char*,V>& hashmap, const char* key); // returns Hashmap::Entry*
}