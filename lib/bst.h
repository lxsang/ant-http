#ifndef BST_H
#define BST_H 1
#define bst_free(n) (bst_free_cb(n, NULL))
typedef struct _tree_node
{
    int key;
    void* data;
    struct _tree_node* left;
    struct _tree_node* right;
} bst_node_t;

void bst_free_cb(bst_node_t* root, void (*callback)(void*));
bst_node_t* bst_insert(bst_node_t* root, int key, void* data);
bst_node_t* bst_find_min(bst_node_t* root);
bst_node_t* bst_find_max(bst_node_t* root);
bst_node_t* bst_find(bst_node_t* root, int x);
bst_node_t* bst_delete(bst_node_t* root, int x);
void bst_for_each(bst_node_t* root, void (*callback)(bst_node_t*, void **, int), void** args, int argc);
#endif