#include <iostream>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

#ifdef CHECK_CONSTRAINTS
#include <math.h>
#include <set>
#endif

#include "RBTree.h"

#define PARENT(node) ((struct rb_node<K, V> *)((uintptr_t)node->parent & (UINTPTR_MAX - 1)))
#define SET_PARENT(node, pare) (node->parent = (struct rb_node<K, V> *)(((uintptr_t)(node->parent) & 1) | ((uintptr_t)(pare) & (UINTPTR_MAX - 1))));

#define GRANDPARENT(node) (PARENT(node) ? PARENT(PARENT(node)) : NULL)

#define IS_LEFT(node) (PARENT(node)->left == node)
#define IS_RIGHT(node) (PARENT(node)->right == node)

#define COLOR(node) (node == NULL ? 0 : (uintptr_t)(node->parent) & 1)
#define SET_COLOR(node, color) (node->parent = (struct rb_node<K, V> *)((uintptr_t)PARENT(node) | color))

#define SET_RED(node) (node->parent = (struct rb_node<K, V> *)((uintptr_t)(node->parent) | 1))
#define SET_BLACK(node) (node->parent = (struct rb_node<K, V> *)((uintptr_t)(node->parent) & (UINTPTR_MAX - 1)))

#define IS_RED(node) (node == NULL ? 0 : (uintptr_t)(node->parent) & 1)
#define IS_BLACK(node) (node == NULL ? 1 : !((uintptr_t)(node->parent) & 1))

#define SIBLING(node) (PARENT(node) == NULL ? NULL : PARENT(node)->left == node ? PARENT(node)->right : PARENT(node)->left)

template<typename K>
inline int rb_compare(K a, K b)
{
	return a == b ? 0 : a < b ? -1 : 1;
}

template<>
inline int rb_compare(const char *a, const char *b)
{
	return strcmp(a, b);
}

template<typename K, typename V>
void rb_tree_init(
	struct rb_tree<K, V> *tree)
{
	tree->root = NULL;
}

template<typename K, typename V>
struct rb_tree<K, V> *rb_tree_create()
{
	struct rb_tree<K, V> *tree =
		(struct rb_tree<K, V> *) malloc(sizeof(struct rb_tree<K, V>));
	rb_tree_init(tree);

	return tree;
}

template<typename K, typename V>
void rb_tree_destroy(
	struct rb_tree<K, V> *tree)
{
	free(tree);
}

template<typename K, typename V>
void rb_node_init(
	struct rb_node<K, V> *node,
	struct rb_node<K, V> *parent,
	K key,
	V value)
{
	node->key = key;
	node->value = value;
	node->parent = parent;
	node->left = NULL;
	node->right = NULL;

	SET_RED(node);
}

template<typename K, typename V>
struct rb_node<K, V> *rb_node_create(
	struct rb_node<K, V> *parent,
	K key,
	V value)
{
	struct rb_node<K, V> *node =
		(struct rb_node<K, V> *) malloc(sizeof(struct rb_node<K, V>));

	/* Allocations should be aligned on a word boundary */

	assert((uintptr_t)node % sizeof(uintptr_t) == 0);
	rb_node_init(node, parent, key, value);

	return node;
}

template<typename K, typename V>
void rb_node_destroy(
	struct rb_node<K, V> *node)
{
	free(node);
}

template<typename K, typename V>
struct rb_node<K, V> *rb_tree_rotate_right(
	struct rb_tree<K, V> *tree,
	struct rb_node<K, V> *node)
{
	struct rb_node<K, V> *a, *b;

	a = node;
	b = node->left;

	SET_PARENT(b, a->parent);
	if (PARENT(a))
	{
		if (PARENT(a)->left == a)
		{
			PARENT(a)->left = b;
		}
		else
		{
			PARENT(a)->right = b;
		}
	}
	else
	{
		tree->root = b;
	}

	a->left = b->right;
	if (a->left)
	{
		SET_PARENT(a->left, a);
	}

	b->right = a;
	SET_PARENT(a, b);

	return b;
}

template<typename K, typename V>
struct rb_node<K, V> *rb_tree_rotate_left(
	struct rb_tree<K, V> *tree,
	struct rb_node<K, V> *node)
{
	struct rb_node<K, V> *a, *b;

	a = node;
	b = node->right;

	SET_PARENT(b, a->parent);
	if (PARENT(a))
	{
		if (PARENT(a)->left == a)
		{
			PARENT(a)->left = b;
		}
		else
		{
			PARENT(a)->right = b;
		}
	}
	else
	{
		tree->root = b;
	}

	a->right = b->left;
	if (a->right)
	{
		SET_PARENT(a->right, a);
	}

	b->left = a;
	SET_PARENT(a, b);

	return b;
}

template<typename K, typename V>
void rb_tree_balance(
	struct rb_tree<K, V> *tree,
	struct rb_node<K, V> *node)
{
	struct rb_node<K, V> *parent, *grandparent, *uncle;

	while (true)
	{
		parent = PARENT(node);
		grandparent = GRANDPARENT(node);

		if (PARENT(node) == NULL)
		{
			SET_BLACK(node);
			break;
		}

		if (IS_BLACK(parent))
		{
			break;
		}

		uncle = grandparent->left == PARENT(node) ?
			grandparent->right : grandparent->left;

		if (IS_RED(uncle))
		{
			SET_BLACK(parent);
			SET_BLACK(uncle);
			SET_RED(grandparent);

			node = grandparent;

			continue;
		}

		if (IS_RED(parent) && IS_BLACK(uncle))
		{
			if (IS_RIGHT(node) && IS_LEFT(parent))
			{
				rb_tree_rotate_left(tree, parent);
				node = node->left;
			}
			else if (IS_LEFT(node) && IS_RIGHT(parent))
			{
				rb_tree_rotate_right(tree, parent);
				node = node->right;
			}

			parent = PARENT(node);
			grandparent = GRANDPARENT(node);
			SET_BLACK(parent);
			SET_RED(grandparent);

			if (IS_LEFT(node) && IS_LEFT(parent))
			{
				rb_tree_rotate_right(tree, grandparent);
			}
			else if (IS_RIGHT(node) && IS_RIGHT(parent))
			{
				rb_tree_rotate_left(tree, grandparent);
			}
		}

		break;
	}
}

template<typename K, typename V>
bool rb_node_insert(
	struct rb_tree<K, V> *tree,
	struct rb_node<K, V> *node,
	K key,
	V value)
{
	while (node)
	{
		int result = rb_compare(node->key, key);

		switch (result > 0 ? 1 : (result < 0 ? -1 : result))
		{
		case 0:
			return false;

		case -1:
			if (node->right)
			{
				node = node->right;
				continue;
			}

			node->right = rb_node_create(
				node,
				key,
				value);

			rb_tree_balance(tree, node->right);
			return true;

		case 1:
			if (node->left)
			{
				node = node->left;
				continue;
			}

			node->left = rb_node_create(
				node,
				key,
				value);

			rb_tree_balance(tree, node->left);
			return true;
		}
	}

	return false;
}

template<typename K, typename V>
bool rb_tree_insert(
	struct rb_tree<K, V> *tree,
	K key,
	V value)
{
	if (!tree->root)
	{
		tree->root = rb_node_create(
			(struct rb_node<K, V> *) NULL,
			key,
			value);

		SET_BLACK(tree->root);

		return true;
	}

	return rb_node_insert(tree, tree->root, key, value);
}

template<typename K, typename V>
struct rb_node<K, V> *rb_node_internal_lookup(
	struct rb_tree<K, V> *tree,
	struct rb_node<K, V> *node,
	K key)
{
	while (node)
	{
		int result = rb_compare(node->key, key);

		switch (result > 0 ? 1 : (result < 0 ? -1 : result))
		{
		case 0:
			return node;
		case -1:
			node = node->right;
			break;
		case 1:
			node = node->left;
			break;
		}
	}

	return NULL;
}

template<typename K, typename V>
struct rb_node<K, V> *rb_tree_internal_lookup(
	struct rb_tree<K, V> *tree,
	K key)
{
	return tree->root ?
		rb_node_internal_lookup(tree, tree->root, key) :
		NULL;
}

template<typename K, typename V>
bool rb_tree_lookup(
	struct rb_tree<K, V> *tree,
	K key,
	V *value)
{
	struct rb_node<K, V> *node;

	if (!tree->root)
	{
		return NULL;
	}

	node = rb_tree_internal_lookup(tree, key);

	if (node)
	{
		*value = node->value;
		return true;
	}

	return false;
}

template<typename K, typename V>
struct rb_node<K, V> *rb_node_predecessor(
	struct rb_node<K, V> *node)
{
	node = node->left;

	while (node->right)
	{
		node = node->right;
	}

	return node;
}

template<typename K, typename V>
void rb_node_remove(
	struct rb_tree<K, V> *tree,
	struct rb_node<K, V> *node)
{
	struct rb_node<K, V> *parent, *sibling;

	while (true)
	{
		parent = PARENT(node);

		if (!parent)
		{
			break;
		}

		sibling = SIBLING(node);

		if (IS_RED(sibling))
		{
			SET_RED(parent);
			SET_BLACK(sibling);

			if (IS_LEFT(node))
			{
				rb_tree_rotate_left(tree, parent);
			}
			else
			{
				rb_tree_rotate_right(tree, parent);
			}
		}

		sibling = SIBLING(node);

		if (IS_BLACK(parent) &&
			IS_BLACK(sibling) &&
			IS_BLACK(sibling->left) &&
			IS_BLACK(sibling->right))
		{
			SET_RED(sibling);
			node = parent;
			continue;
		}

		if (IS_RED(parent) &&
			IS_BLACK(sibling) &&
			IS_BLACK(sibling->left) &&
			IS_BLACK(sibling->right))
		{
			SET_RED(sibling);
			SET_BLACK(parent);
			break;
		}

		if (IS_BLACK(sibling))
		{
			if (IS_LEFT(node) &&
				IS_RED(sibling->left) &&
				IS_BLACK(sibling->right))
			{
				SET_RED(sibling);
				SET_BLACK(sibling->left);
				rb_tree_rotate_right(tree, sibling);
			}
			else if (IS_RIGHT(node) &&
					 IS_BLACK(sibling->left) &&
					 IS_RED(sibling->right))
			{
				SET_RED(sibling);
				SET_BLACK(sibling->right);
				rb_tree_rotate_left(tree, sibling);
			}
		}
		else
		{
			break;
		}

		sibling = SIBLING(node);

		SET_COLOR(sibling, COLOR(parent));
		SET_BLACK(parent);

		if (IS_LEFT(node))
		{
			SET_BLACK(sibling->right);
			rb_tree_rotate_left(tree, parent);
		}
		else
		{
			SET_BLACK(sibling->left);
			rb_tree_rotate_right(tree, parent);
		}

		break;
	}
}

template<typename K, typename V>
bool rb_tree_remove(
	struct rb_tree<K, V> *tree,
	K key,
	K *nodeKey,
	V *value)
{
	struct rb_node<K, V> *node, *child;

	if (!tree->root)
	{
		return false;
	}

	node = rb_tree_internal_lookup(tree, key);

	if (!node)
	{
		return false;
	}

	if (nodeKey)
	{
		*nodeKey = node->key;
	}

	if (value)
	{
		*value = node->value;
	}

	if (node->left && node->right)
	{
		child = rb_node_predecessor(node);
		node->key = child->key;
		node->value = child->value;
		node = child;
	}

	child = node->left ? node->left : node->right;

	if (IS_BLACK(node))
	{
		SET_COLOR(node, COLOR(child));
		rb_node_remove(tree, node);
	}

	if (PARENT(node) == NULL)
	{
		tree->root = child;
	}
	else if (IS_LEFT(node))
	{
		PARENT(node)->left = child;
	}
	else
	{
		PARENT(node)->right = child;
	}

	if (child)
	{
		SET_PARENT(child, PARENT(node));
	}

	if (PARENT(node) == NULL && child != NULL)
	{
		SET_BLACK(child);
	}

	rb_node_destroy(node);
	return true;
}

#ifdef CHECK_CONSTRAINTS
template<typename K, typename V>
int rb_node_depth(
	struct rb_node<K, V> *node,
	int *nodes)
{
	if (nodes)
	{
		(*nodes)++;
	}

	return 1 + std::max(
		node->left ? rb_node_depth(node->left, nodes) : 0,
		node->right ? rb_node_depth(node->right, nodes) : 0);
}

template<typename K, typename V>
int rb_tree_depth(
	struct rb_tree<K, V> *tree,
	int *nodes)
{
	if (nodes)
	{
		*nodes = 0;
	}

	return tree->root ? rb_node_depth(tree->root, nodes) : 0;
}

template<typename K, typename V>
void rb_node_check_constraints(
	struct rb_node<K, V> *node,
	int *blacks)
{
	if (IS_RED(node))
	{
		assert(IS_BLACK(node->left) && IS_BLACK(node->right));
	}

	/* Every path from the root to a leaf node has the same
	 * number of black nodes. */

	if (node->left == NULL && node->right == NULL)
	{
		int nodes = 0;
		struct rb_node<K, V> *ptr = node;

		while (ptr)
		{
			if (IS_BLACK(ptr))
			{
				nodes++;
			}

			ptr = PARENT(ptr);
		}

		if (*blacks == -1)
		{
			*blacks = nodes;
		}
		else
		{
			assert(*blacks == nodes);
		}
	}

	if (node->left)
	{
		rb_node_check_constraints(node->left, blacks);
	}

	if (node->right)
	{
		rb_node_check_constraints(node->right, blacks);
	}
}

template<typename K, typename V>
void rb_tree_check_constraints(
	struct rb_tree<K, V> *tree)
{
	int nodes = -1;
	int blacks = -1;
	int depth = -1;

	/* The height of a red black tree can't exceed 2 * log2(n + 1)
	 * where n is the number of nodes in the tree. */

	depth = rb_tree_depth(tree, &nodes);
	assert(depth <= ceil(log2(nodes + 1) * 2));

	if (tree->root)
	{
		rb_node_check_constraints(tree->root, &blacks);
	}
}

template<typename T, unsigned int N>
void randomize(T (&values)[N])
{
	std::set<T> uniques;
	static bool init = false;

	if (!init)
	{
		srand(time(0));
		init = true;
	}

	int count = 0;
	while (true)
	{
		int random = rand() % sizeof(values) / sizeof(T);

		if (uniques.find(random) != uniques.end())
		{
			continue;
		}

		uniques.insert(random);
		values[count] = random;
		count++;

		if (count == sizeof(values) / sizeof(T))
		{
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	{
		struct rb_tree<int, int> tree;
		int values[1000];
		int nodes = -1;

		rb_tree_init(&tree);

		randomize<int>(values);

		for (unsigned int i = 0; i < sizeof(values) / sizeof(int); i++)
		{
			rb_tree_insert(&tree, values[i], values[i]);
			rb_tree_check_constraints(&tree);
		}

		rb_tree_depth(&tree, &nodes);
		assert(nodes == sizeof(values) / sizeof(int));

		for (unsigned int i = 0; i < sizeof(values) / sizeof(int); i++)
		{
			int value = -1;
			assert(rb_tree_lookup(&tree, (int)i, &value));
			assert(value == (int)i);
		}

		randomize<int>(values);

		for (unsigned int i = 0; i < sizeof(values) / sizeof(int); i++)
		{
			int key, value;
			assert(rb_tree_remove(&tree, values[i], &key, &value));
			assert(key == values[i]);
			rb_tree_check_constraints(&tree);
		}

		rb_tree_depth(&tree, &nodes);
		assert(nodes == 0);
	}

	{
		struct rb_tree<const char *, int> tree;
		char buffer[4096];
		int nodes = -1;

		rb_tree_init(&tree);
		memset(buffer, 0xff, sizeof(buffer));

		for (unsigned int i = 1; i <= 1000; i++)
		{
			unsigned int j;
			for (j = 0; j < i; j++)
			{
				buffer[j] = 'a';
			}
			buffer[j] = '\0';

			rb_tree_insert(
				&tree,
				(const char *)strdup(buffer),
				(int)i);

			rb_tree_check_constraints(&tree);
		}

		rb_tree_depth(&tree, &nodes);
		assert(nodes == 1000);

		for (unsigned int i = 1; i <= 1000; i++)
		{
			unsigned int j;
			for (j = 0; j < i; j++)
			{
				buffer[j] = 'a';
			}
			buffer[j] = '\0';

			int value = -1;

			assert(
				rb_tree_lookup(
					&tree,
					(const char *)buffer,
					&value));

			assert(strlen(buffer) == (unsigned int)value);
		}

		for (unsigned int i = 1; i <= 1000; i++)
		{
			unsigned int j;
			for (j = 0; j < i; j++)
			{
				buffer[j] = 'a';
			}
			buffer[j] = '\0';

			const char *key;
			int value;

			assert(rb_tree_remove(
				&tree,
				(const char *)buffer,
				&key,
				&value));
			assert(!strcmp(key, (const char *)buffer));
			assert(strlen(buffer) == (unsigned int)value);

			free((void *)key);
			rb_tree_check_constraints(&tree);
		}

		rb_tree_depth(&tree, &nodes);
		assert(nodes == 0);
	}

	return 0;
}
#endif
