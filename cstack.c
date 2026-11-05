#include "cstack.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct node
{
    const struct node* prev;
    unsigned int size;
    char data[];
};

typedef struct node* stack_t;

struct stack_entry
{
    int reserved;
    stack_t stack;
};

typedef struct stack_entry stack_entry_t;

struct stack_entries_table
{
    unsigned int size;
    stack_entry_t* entries;
};

struct stack_entries_table g_table = {0u, NULL};

hstack_t stack_new(void)
{
    unsigned int new_size = g_table.size + 1;
    stack_entry_t* new_entries =
        realloc(g_table.entries, new_size * sizeof(stack_entry_t));
    if (new_entries == NULL)
        return -1;
    g_table.entries = new_entries;
    g_table.size = new_size;
    int new_handler = (int)(g_table.size - 1);
    g_table.entries[new_handler].reserved = 1;
    g_table.entries[new_handler].stack = NULL;
    return new_handler;
}

int stack_valid_handler(const hstack_t hstack)
{
    if (hstack >= 0 && (unsigned int)hstack < g_table.size &&
        g_table.entries[hstack].reserved == 1)
        return 1;
    return 0;
}

void stack_free(const hstack_t hstack)
{
    if (stack_valid_handler(hstack))
    {
        stack_t cur = g_table.entries[hstack].stack;
        while (cur != NULL)
        {
            stack_t next = (stack_t)(cur->prev);
            free(cur);
            cur = next;
        }
        g_table.entries[hstack].stack = NULL;
        g_table.entries[hstack].reserved = 0;
    }
}

void stack_free_all(void)
{
    if (g_table.entries != NULL && g_table.size != 0)
    {
        for (unsigned int index = 0; index < g_table.size; ++index)
        {
            stack_free((hstack_t)index);
        }
    }
}

unsigned int stack_size(const hstack_t hstack)
{
    if (stack_valid_handler(hstack))
    {
        unsigned int count = 0;
        stack_t cur = g_table.entries[hstack].stack;
        while (cur != NULL)
        {
            ++count;
            cur = (stack_t)(cur->prev);
        }
        return count;
    }
    return 0;
}

void
stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    if (data_in != NULL && size != 0 && stack_valid_handler(hstack))
    {
        stack_t new_node = malloc(sizeof(struct node) + size);
        if (new_node != NULL)
        {
            new_node->prev = g_table.entries[hstack].stack;
            new_node->size = size;
            memcpy(new_node->data, data_in, size);
            g_table.entries[hstack].stack = new_node;
        }
    }
}

unsigned int
stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    if (data_out != NULL && stack_valid_handler(hstack))
    {
        stack_t top_node = g_table.entries[hstack].stack;
        if (top_node != NULL && size >= top_node->size)
        {
            unsigned int bytes = top_node->size;
            memcpy(data_out, top_node->data, bytes);
            g_table.entries[hstack].stack = (stack_t)(top_node->prev);
            free(top_node);
            return bytes;
        }
        return 0;
    }
    return 0;
}

unsigned stack_top_size(const hstack_t hstack)
{
    if (stack_valid_handler(hstack))
    {
        stack_t top_node = g_table.entries[hstack].stack;
        return (top_node != NULL) ? top_node->size : 0;
    }
    return 0;
}

hstack_t stack_get_all(void)
{
    unsigned int initial_size = g_table.size;
    hstack_t result = stack_new();
    if (stack_valid_handler(result))
    {
        for (unsigned int index = 0; index < initial_size; ++index)
        {
            hstack_t cur = (hstack_t)index;
            if (stack_valid_handler(cur))
                stack_push(result, &cur, sizeof(hstack_t));
        }
        return result;
    }
    return -1;
}
