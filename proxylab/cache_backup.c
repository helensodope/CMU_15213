#include "cache.h"

size_t total_size;

cache_block *cache_inquiry(char *key, cache_block *cache)
{
    cache_block *temp = cache;
    cache_block *increment_ucount;
    while(temp != NULL)
    {
        if(strcmp(key, temp->key) == 0)
        {
            temp->ucount = 0;
            increment_ucount = temp->next;
            while(increment_ucount != NULL)
            {
                increment_ucount->ucount++;
                increment_ucount = increment_ucount->next;
            }
            return temp;
        }
        temp->ucount++;
        temp = temp->next;
    }
    return NULL;
}

cache_block *new_block(char *key, char *buf, size_t size)
{
    cache_block *newcache = malloc(sizeof(cache_block));
    
    newcache->key = malloc(strlen(key)+1);
    strcpy(newcache->key,key);
    
    newcache->buf = malloc(strlen(buf)+1);
    strcpy(newcache->buf, buf);
    
    newcache->size = size;
    newcache->ucount = 0;
    newcache->next = NULL;
    return newcache;
}

cache_block *cache_insert(char *key, char *buf, size_t size, cache_block *cache)
{
    cache_block *newcache;
    cache_block *temp = cache;
    cache_block *evict = cache;
    //no eviction
    if(total_size + size <= MAX_CACHE_SIZE)
    {
        newcache = new_block(key, buf, size);
        newcache->next = cache;
        //total cache size update
        total_size += size;
        return newcache;
    }
    //evict
    else
    {
        while(temp != NULL)
        {
            if(evict->ucount < temp->ucount)
            {
                evict = temp;
            }
            temp = temp->next;
        }

        free(evict->buf);
        free(evict->key);
        total_size = total_size - evict->size;

        evict->buf = malloc(strlen(buf)+1);
        strcpy(evict->buf, buf);
        evict->key = malloc(strlen(key)+1);
        strcpy(evict->key, key);
        
        evict->size = size;
        evict->ucount = 0;

        total_size += size;
        return cache;
    }
}
