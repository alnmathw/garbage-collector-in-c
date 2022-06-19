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

    printf("------------------------------\n");

    size_t heap_ptrs_count = 0;
    for (size_t i = 0; i < alloced_chunks.count; ++i) {
        for (size_t j = 0; j < alloced_chunks.chunks[i].size; ++j) {
            uintptr_t *p = (uintptr_t*) alloced_chunks.chunks[i].start[j];
            if (heap <= p && p < heap + HEAP_CAP_WORDS) {
                printf("DETECTED HEAP POINTER: %p\n", (void*) p);
                heap_ptrs_count += 1;
            }
        }
    }

    printf("Detected %zu heap pointers\n", heap_ptrs_count);

    return 0;
}