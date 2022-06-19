#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "./heap.h"

#define alan_IMPLEMENTATION
#include "alan.h"

typedef struct Node Node;

struct Node {
    char x;
    Node *left;
    Node *right;
};

Node *generate_tree(size_t level_cur, size_t level_max)
{
    if (level_cur < level_max) {
        Node *root = heap_alloc(sizeof(*root));
        assert((char) level_cur - 'a' <= 'z');
        root->x = level_cur + 'a';
        root->left = generate_tree(level_cur + 1, level_max);
        root->right = generate_tree(level_cur + 1, level_max);
        return root;
    } else {
        return NULL;
    }
}

void print_tree(Node *root, alan *alan)
{
    if (root != NULL) {
        alan_object_begin(alan);

        alan_member_key(alan, "value");
        alan_string_sized(alan, &root->x, 1);

        alan_member_key(alan, "left");
        print_tree(root->left, alan);

        alan_member_key(alan, "right");
        print_tree(root->right, alan);

        alan_object_end(alan);
    } else {
        alan_null(alan);
    }
}

#define N 10

void *ptrs[N] = {0};

int main()
{
    Node *root = generate_tree(0, 3);

    alan alan = {
        .sink = stdout,
        .write = (alan_Write) fwrite,
    };

    print_tree(root, &alan);

    return 0;
}