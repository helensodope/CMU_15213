#include "cache.h"

//global variable to keep track of the cache size
size_t total_size;

//based on key, which is going to be the uri in this project,
//cache returns the block with the matching key
cache_block *cache_inquiry(char *key, cache_block *cache)
{
    cache_block *temp = cache;
    cache_block *increment_ucount;
    while(temp != NULL)
    {
        //found matching key
        if(strcmp(key, temp->key) == 0)
        {
            //ucount is for keeping track of LRU
            //most recently used, so set to zero
            temp->ucount = 0;
            increment_ucount = temp->next;
            while(increment_ucount != NULL)
            {
                //increment ucount of other elements
                increment_ucount->ucount++;
                increment_ucount = increment_ucount->next;
            }
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

//make and initialize a new block
cache_block *new_block(char *key, char *buf, size_t size)
{
    cache_block *newcache = malloc(sizeof(cache_block));
    
    newcache->key = Malloc(strlen(key)+1);
    strcpy(newcache->key,key);
    
    newcache->buf = Malloc(size);
    strcpy(newcache->buf, buf);
    
    newcache->size = size;
    newcache->ucount = 0;
    //newcache->next = NULL;
    return newcache;
}

//insert element to cache
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
        //get block to evict based on ucount (greatest)
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
        //update total cache size
        total_size = total_size - evict->size;

        evict->buf = malloc(size);
        strcpy(evict->buf, buf);
        evict->key = malloc(strlen(key)+1);
        strcpy(evict->key, key);
        
        evict->size = size;
        evict->ucount = 0;

        //update total cache size
        total_size += size;
        return cache;
    }
}
