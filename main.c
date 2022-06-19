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
    stack_base = (const uintptr_t*)__builtin_frame_address(0);

    for (size_t i = 0; i < 10; ++i) {
        heap_alloc(i);
    }

    Node *root = generate_tree(0, 3);

    printf("root: %p\n", (void*)root);

    alan alan = {
        .sink = stdout,
        .write = (alan_Write) fwrite,
    };

    print_tree(root, &alan);

    printf("\n------------------------------\n");
    heap_collect();
    chunk_list_dump(&alloced_chunks, "Alloced");
    chunk_list_dump(&freed_chunks, "Freed");
    printf("------------------------------\n");
    root = NULL;
    heap_collect();
    chunk_list_dump(&alloced_chunks, "Alloced");
    chunk_list_dump(&freed_chunks, "Freed");

    return 0;
}