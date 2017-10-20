//
// Created by root on 10/19/17.
//

#include "../headerFiles/deletedIdBST.h"

ID_NODE* newNode(int id)
{
    ID_NODE* node = (ID_NODE*)
            malloc(sizeof(ID_NODE));
    node->id  = id;
    node->left  = NULL;
    node->right = NULL;

    return(node);
}

ID_NODE* insert(ID_NODE* root, int id)
{
    /* If the tree is empty, return a new,
        single node */
    if (root == NULL)
        return(newNode(id));
    else
    {
        /* Otherwise, recur down the tree */
        if (id <= root->id)
            root->left  = insert(root->left, id);
        else
            root->right = insert(root->right, id);

        /* return the (unchanged) node pointer */
        return root;
    }
}

ID_NODE * minValueNode(ID_NODE* root)
{
    //base case
    if(!root->left)
        return root;

    //recursion
    return minValueNode(root->left);

}

/* Given a binary search tree and a key, this function deletes the key
   and returns the new root */

ID_NODE* deleteNode(ID_NODE* root, int id)
{
    // base case
    if (root == NULL) return root;

    // If the key to be deleted is smaller than the root's key,
    // then it lies in left subtree
    if (id < root->id)
        root->left = deleteNode(root->left, id);

        // If the key to be deleted is greater than the root's key,
        // then it lies in right subtree
    else if (id > root->id)
        root->right = deleteNode(root->right, id);

        // if key is same as root's key, then This is the node
        // to be deleted
    else
    {
        // node with only one child or no child
        if (root->left == NULL)
        {
            ID_NODE* temp = root->right;
            free(root);
            return temp;
        }
        else if (root->right == NULL)
        {
            ID_NODE *temp = root->left;
            free(root);
            return temp;
        }

        // node with two children: Get the inorder successor (smallest
        // in the right subtree)
        ID_NODE* temp = minValueNode(root->right);

        // Copy the inorder successor's content to this node
        root->id= temp->id;

        // Delete the inorder successor
        root->right = deleteNode(root->right, temp->id);
    }
    return root;
}
