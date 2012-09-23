#ifndef __RB_TREE_H
#define __RB_TREE_H

template<typename K, typename V>
struct rb_node
{
	K key;
	V value;
	struct rb_node<K, V> *parent, *left, *right;
};

template<typename K, typename V>
struct rb_tree
{
	struct rb_node<K, V> *root;
};

template<typename K, typename V>
void rb_tree_init(
	struct rb_tree<K, V> *tree);

template<typename K, typename V>
struct rb_tree<K, V> *rb_tree_create();

template<typename K, typename V>
void rb_tree_destroy(
	struct rb_tree<K, V> *tree);

template<typename K, typename V>
bool rb_tree_insert(
	struct rb_tree<K, V> *tree,
	K key,
	V value);

template<typename K, typename V>
bool rb_tree_lookup(
	struct rb_tree<K, V> *tree,
	K key,
	V *value);

template<typename K, typename V>
bool rb_tree_remove(
	struct rb_tree<K, V> *tree,
	K key,
	K *nodeKey,
	V *value);

#endif
