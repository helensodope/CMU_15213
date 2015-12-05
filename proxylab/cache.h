#include "csapp.h"

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

struct cache_block{
    size_t size;
    size_t ucount;
    struct cache_block* next;
    char *key;
    char *buf;
};
typedef struct cache_block cache_block;

cache_block *cache_inquiry(char *key, cache_block *cache);
cache_block *newblock(char *key, char *buf, size_t size);
cache_block *cache_insert(char *key, char *buf, size_t size, cache_block *cache);
