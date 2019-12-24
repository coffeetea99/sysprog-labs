#include "cache.h"

/// you can use any data structures like linked-list, tree and hash table
int cache_size = 0;
cache_block *start = NULL;

// find cache block with uri, return NULL if none
cache_block *find_cache_block(char *uri)
{
  cache_block *ptr = start;
  while (ptr != NULL)
  {
    if (strcmp(uri, (*ptr).uri) == 0)
    {
      return ptr;
    }
    ptr = (*ptr).next;
  }
  return NULL;
}

void cache_replacement_policy()
{
  /*
 * cache_replacement_policy: 
 * 			delete the cached contents according to the cache replacement policy
 * params:
 *
 */

  // if the cache is too big, free some blocks
  cache_block *oldstart = start;
  while (cache_size > MAX_CACHE_SIZE)
  {
    start = (*start).next;

    int freeSize = sizeof(cache_block) + (*oldstart).contentLength;
    cache_size -= freeSize;

    free((*oldstart).content);
    free(oldstart);

    oldstart = start;
  }
}

int add_cache_block(char *uri, char *content, char *response, int contentLength)
{
  /*
 * add_cache_block: 
 *        add the uri information into the proxy cache 
 * params:
 *    - uri: uri string.
 *    - content: the content of uri
 *    - response: response header 
 *    - contentLength: byte length of the HTTP body
 * 
 */
  /// use cache replacement policy if the proxy cache is full.
  /// you can use any cache replacement policy such as FIFO, LRU

  int newSize = sizeof(cache_block) + contentLength;

  // too big!!
  if (newSize > MAX_OBJECT_SIZE)
  {
    return 0;
  }

  cache_block *ptr = start;
  cache_block *backPtr;
  while (ptr != NULL)
  {
    backPtr = ptr;
    ptr = (*ptr).next;
  }
  ptr = malloc(sizeof(cache_block));

  strcpy((*ptr).uri, uri);
  strcpy((*ptr).resp, response);
  (*ptr).content = malloc(sizeof(char) * contentLength);
  memcpy((*ptr).content, content, contentLength);
  (*ptr).contentLength = contentLength;
  (*ptr).next = NULL;

  cache_size += newSize;

  if (start == NULL)
  {
    start = ptr;
  }
  else
  {
    (*backPtr).next = ptr;
  }

  cache_replacement_policy();

  return 1;
}
