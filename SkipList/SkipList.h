#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
#include <stdio.h>

#define STORE_FILE "store/dumpFile"

std::mutex mtx;
std::string delimiter = ":";


//class template for node
template<typename K, typename V>
class Node {
public:
	Node() = default;
	Node(K k, V v, int);
	~Node();

	K get_key() const;
	V get_value() const;

	void set_value(V);
	Node<K, V>** forward;
	Node<K, V>* backward;
	int node_level;

private:
	K key;
	V value;
};


//Constructor List Initialization
template<typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level) {
	this->key = k;
	this->value = v;
	this->node_level = level;

	//the index of array is from 0 ~ level
	this->forward = new Node<K, V>*[level + 1];
	this->backward = new Node<K, V>();

	//initial forward in memory
	memset(this->forward, 0, sizeof(Node<K, V>*) * (level + 1));
};

//desconstructor
template<typename K, typename V>
Node<K, V>::~Node() {
	delete[]forward;
	delete backward;
}


template<typename K, typename V>
K Node<K, V>::get_key() const {
	return key;
}

template<typename K, typename V>
V Node<K, V>::get_value() const {
	return value;
}


template<typename K, typename V>
void Node<K, V>::set_value(V value) {
	this->value = value;
}

//template for SkipList
/*
	+----------+
	| skiplist |
	+----------+

	+-----------+
	| max level |
	-------------
	| skiplist  |
	| level     |
	-------------
	| header    |
	-------------
	| tail      |
	-------------
	| elements  |
	| count     |
	+-----------+

*/
template<typename K, typename V>
class SkipList {
public:
	SkipList(int);
	~SkipList();

	int get_random_level();
	Node<K, V>* create_node(K, V, int);
	int insert_element(K, V);
	void display_list();
	bool search_element(K);
	void delete_element(K);
	void dump_file();
	void load_file();
	int size();

	void get_tail();
	void get_prev_element(K);

private:
	void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
	bool is_valid_string(const std::string& str);

	//maxium level of the SkipList
	int _max_level;

	//current level
	int _skip_list_level;

	//pointer to header node
	Node<K, V>* _header;

	//pointer to tailer
	Node<K, V>* _tailer;

	//file operator
	std::ofstream _file_writer;
	std::ifstream _file_reader;

	// the elements in SkipList
	int _element_count;
};

template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(K key, V value, int level) {
	Node<K, V>* node = new Node<K, V>(key, value, level);
	return node;
}


// Insert given key and value in skip list
// return 1 means element exists
// return 0 means insert successfully
/*
						   +------------+
						   |  insert 50 |
						   +------------+
level 4     +-->1+                                                      100
				 |
				 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
											   |    |
											   |    |
level 2         1          10         30       | 50 |          70       100
											   |    |
											   |    |
level 1         1    4     10         30       | 50 |          70       100
											   |    |
											   |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100

Notes:
  ** the random generated level is larger than skiplist-level
	1. Update skiplist-level
	2. Make the excess part linked to header

  ** if the new node is tail, update tailer
											   +----+
*/

template<typename K, typename V>
int SkipList<K, V>::insert_element(K key, V value) {
	mtx.lock();

	Node<K, V>* current = this->_header;

	//create update array and initialize it
	//update is array which put node that the node->forward[i] should be operated later	
	Node<K, V>** update = new Node<K, V>*[_max_level + 1];
	memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));

	//find the insertaion location. the condition : tail(NULL) or font < new < back
	// high level -> low level
	for (int i = _skip_list_level; i >= 0; --i) {

		while (current->forward[i] != NULL && current->forward[i]->get_key() < key) {
			current = current->forward[i];
		}
		update[i] = current;
	}

	Node<K, V>* prev = current;
	//reach level0 and forward pointer to right node, which is desired to insert key
	current = current->forward[0];

	//if the key exists, finish the insertation operation and return
	if (current != NULL && current->get_key() == key) {
		std::cout << "the value exist the SkipList" << std::endl;
		mtx.unlock();
		return 1;
	}

	//initialize overranging pointer to header
	if (current == NULL || current->get_key() != key) {
		int random_level = get_random_level();

		if (random_level > _skip_list_level) {
			for (int i = _skip_list_level + 1; i < random_level + 1; ++i) {
				update[i] = _header;
			}

			// update the maximun level of SkipList
			_skip_list_level = random_level;
		}

		Node<K, V>* insertedNode = create_node(key, value, random_level);

		insertedNode->backward = prev;
		if (current == NULL) {
			this->_tailer->forward[0] = insertedNode;
		}
		//do insertation operation
		for (int i = 0; i <= random_level; ++i) {
			insertedNode->forward[i] = update[i]->forward[i];
			update[i]->forward[i] = insertedNode;
		}

		std::cout << "Insert Successfully key: " << key << " Value: " << value << std::endl;
		++_element_count;
	}

	mtx.unlock();
	return 0;
}


template<typename K, typename V>
void SkipList<K, V>::display_list() {
	std::cout << "\n **********skip list*************" << "\n";

	//foreach each level
	for (int i = 0; i <= _skip_list_level; ++i) {
		Node<K, V>* node = this->_header->forward[i];
		std::cout << "level " << i << ":";
		while (node != NULL) {
			std::cout << node->get_key() << ":" << node->get_value() << ";";
			node = node->forward[i];
		}

		std::cout << std::endl;
	}
}

template<typename K, typename V>
void SkipList<K, V>::dump_file() {
	std::cout << "dump_file ------------" << std::endl;
	_file_writer.open(STORE_FILE);

	Node<K, V>* node = this->_header->forward[0];

	while (node != NULL) {
		_file_writer << node->get_key() << ":" << node->get_key() << "\n";
		_file_writer << node->get_value() << ":" << node->get_value() << "\n";
		node = node->forward[0];
	}

	_file_writer.flush();
	_file_writer.close();
	return;
}


template<typename K, typename V>
void SkipList<K, V>::load_file() {

	_file_reader.open(STORE_FILE);
	std::cout << "load file--------------" << std::endl;
	std::string line;
	std::string* key = new std::string();
	std::string* value = new std::string();
	while (std::string::getline(_file_reader, line)) {
		get_key_value_from_string(line, key, value);
		if (key->empty() || value->empty()) {
			continue;
		}
		insert_element(*key, *value);
		std::cout << "key: " << *key << " value: " << *value << std::endl;
	}
	_file_reader.close();
}


template<typename K, typename V>
int SkipList<K, V>::size() {
	return _element_count;
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value) {

	if (!is_valid_string(str)) {
		return;
	}

	*key = std.substr(0, str.find(delimiter));
	*value = std.substr(str.find(delimiter) + 1, str.length());
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str) {
	if (str.empty()) {
		return false;
	}
	if (str.find(delimiter) == std::string::npos) {
		return false;
	}
	return true;
}


template<typename K, typename V>
void SkipList<K, V>::delete_element(K key) {

	mtx.lock();
	Node<K, V>* current = this->_header;

	Node<K, V>** update = new Node<K, V>*[_max_level + 1];
	memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));

	for (int i = _skip_list_level; i >= 0; --i) {
		while (current->forward[i] != NULL && current->forward[i]->get_key() < key) {
			current = current->forward[i];
		}
		update[i] = current;
	}

	//locate at bottem level to delete the element
	current = current->forward[0];

	if (current != NULL && current->get_key() == key) {

		Node<K, V>* tail = this->_tailer->forward[0];

		if (tail->get_key() == key) {
			this->_tailer->forward[0] = current->backward;
		}

		//delete the point in every level 
		for (int i = 0; i <= _skip_list_level; ++i) {
			if (update[i]->forward[i] != current) {
				break;
			}

			update[i]->forward[i] = current->forward[i];
		}

		//if there is no elements in one level
		while (_skip_list_level > 0 && _header->forward[_skip_list_level] == 0) {
			--_skip_list_level;
		}

		std::cout << "delete key successfully: " << key << std::endl;
	}

	mtx.unlock();
	return;
}


template<typename K, typename V>
bool SkipList<K, V>::search_element(K key) {
	std::cout << "search element-----------------------" << std::endl;

	Node<K, V>* current = _header;

	//locate at the level
	for (int i = _skip_list_level; i >= 0; --i) {
		while (current->forward[i] != NULL && current->forward[i]->get_key() < key) {
			current = current->forward[i];
		}
	}

	//Go to bottom level
	current = current->forward[0];

	//look for needed key
	while (current and current->get_key() == key) {
		std::cout << "found key: " << key << " value " << current->get_value() << std::endl;
		return true;
	}

	std::cout << "Not Found Key:" << key << std::endl;
	return false;
}

template<typename K, typename V>
void SkipList<K, V>::get_tail() {
	Node<K, V>* tail = this->_tailer->forward[0];
	std::cout << "The final key:" << tail->get_key() << " value:" << tail->get_value() << std::endl;
}


template<typename K, typename V>
void SkipList<K, V>::get_prev_element(K key) {
	Node<K, V>* current = _header;

	//locate at the level
	for (int i = _skip_list_level; i >= 0; --i) {
		while (current->forward[i] != NULL && current->forward[i]->get_key() < key) {
			current = current->forward[i];
		}
	}

	//Go to bottom level
	current = current->forward[0];

	//look for needed key
	while (current and current->get_key() == key) {
		std::cout << key << "'s prev key: " << current->backward->get_key() << " value " << current->backward->get_value() << std::endl;
		return;
	}
}
template<typename K, typename V>
SkipList<K, V>::SkipList(int max_level) {
	this->_max_level = max_level;
	this->_skip_list_level = 0;
	this->_element_count = 0;

	K k;
	V v;
	this->_header = new Node<K, V>(k, v, _max_level);
	this->_tailer = new Node<K, V>(k, v, _max_level);
};

template<typename K, typename V>
SkipList<K, V>::~SkipList() {
	if (_file_writer.is_open()) {
		_file_writer.close();
	}
	if (_file_reader.is_open()) {
		_file_reader.close();
	}
	delete _header;
	delete _tailer;
}

template<typename K, typename V>
int SkipList<K, V>::get_random_level() {
	int k = 1;
	while (rand() % 2) {
		++k;
	}
	k = (k < _max_level) ? k : _max_level;
	return k;
}



