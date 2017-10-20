//
// Created by root on 10/19/17.
//

#ifndef headerIncluded
#include "headers.h"
#endif

#ifndef structIncluded
#include "types.h"
#endif

//create new id node
ID_NODE* newNode(int id);

//insert ID_NODE to deleted list
ID_NODE* insert(ID_NODE* root, int id);

//return the node with minimum value
ID_NODE * minValueNode(ID_NODE* root);

//delete the node that has given id
ID_NODE* deleteNode(ID_NODE* root, int id);
