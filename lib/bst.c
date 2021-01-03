#include<stdio.h>
#include<stdlib.h>

#include "bst.h"

void bst_free_cb(bst_node_t* root, void (*cb)(void*))
{
    if(root != NULL)
    {
        bst_free(root->left);
        bst_free(root->right);
        if(root->data && cb)
        {
            cb(root->data);
        }
        free(root);
    }
}

bst_node_t* bst_insert(bst_node_t* root, int key, void* data)
{
    if(root == NULL)
    {
        root = malloc(sizeof(bst_node_t));
        root->key = key;
        root->data = data;
        root->left = root->right = NULL;
    }
    else if(key < root->key)
        root->left = bst_insert(root->left, key, data);
    else if(key > root->key)
        root->right = bst_insert(root->right, key, data);
    else
        root->data = data;
    return root;
}

bst_node_t* bst_find_min(bst_node_t* root)
{
    if(root == NULL)
    	return NULL;
    else if(root->left == NULL)
    	return root;
    else
    	return bst_find_min(root->left);
}

bst_node_t* bst_find_max(bst_node_t* root)
{
    if(root == NULL)
    	return NULL;
    else if(root->right == NULL)
    	return root;
    else
    	return bst_find_max(root->right);
}

bst_node_t* bst_find(bst_node_t* root, int x)
{
    if(root == NULL)
        return NULL;
    else if(x < root->key)
        return bst_find(root->left, x);
    else if(x > root->key)
        return bst_find(root->right, x);
    else
        return root;
}


bst_node_t* bst_delete(bst_node_t* root, int x)
{
    bst_node_t* temp;
    if(root == NULL)
        return NULL;
    else if(x < root->key)
        root->left = bst_delete(root->left, x);
    else if(x > root->key)
        root->right = bst_delete(root->right, x);
    else if(root->left && root->right)
    {
        temp = bst_find_min(root->right);
        root->key = temp->key;
        root->data = temp->data;
        root->right = bst_delete(root->right, root->key);
    }
    else
    {
        temp = root;
        if(root->left == NULL)
            root = root->right;
        else if(root->right == NULL)
            root = root->left;
        free(temp);
    }
    return root;
}

void bst_for_each(bst_node_t* root, void (*callback)(bst_node_t*, void **, int), void** args, int argc)
{
    if(root == NULL)
        return;
    bst_for_each(root->left, callback, args, argc);
    callback(root, args, argc);
    bst_for_each(root->right, callback, args, argc);
}