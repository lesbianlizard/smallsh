// This file implements a simple dynamic array system for C strings

typedef struct {
  char** d;
  size_t allocated;
  size_t used;
} Strs;

// Initialize a Strs object
void initStrs(Strs* new_strs)
{
  const size_t initSize = 2;
  new_strs->d = malloc(initSize * sizeof(char*));
  new_strs->allocated = initSize;
  new_strs->used = 0;
}

// Free all memory allocated in a Strs
void deallocStrs(Strs* strs)
{
  size_t i;

  for (i = 0; i < strs->used; i++)
  {
    free(strs->d[i]);
  }

  free(strs->d);
}

// Double size of allocated memory
void _growStrs(Strs* strs)
{
  strs->d = realloc(strs->d, strs->allocated*2*sizeof(char*));
  strs->allocated *= 2;
}

// Add a new string to the end of a Strs array
void pushStrs(Strs* strs, char* new_string)
{
  if (!(strs->used < strs->allocated))
  {
    _growStrs(strs);
  }

  strs->d[strs->used] = new_string;
  strs->used++;
}

// Find the first occurence of search in strs and return its index.
// Return SIZE_MAX if search is not in strs.
size_t containsStrs(Strs* strs, char* search)
{
  size_t i;

  for (i = 0; i < strs->used; i++)
  {
    if (strcmp(search, strs->d[i]) == 0)
    {
      return i;
    }
  }
  
  return SIZE_MAX;
}

// Remove all elements of a Strs array at an index greater than or equal to idx
void truncateStrs(Strs* strs, size_t idx)
{
  size_t i;

  for (i = idx; i < strs->used; i++)
  {
    free(strs->d[i]);
    strs->d[i] = NULL;
  }

  strs->used = idx;
}
