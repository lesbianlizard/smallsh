// This file implements a simple dynamic array system for an arbitrary data type DTYPE
#define NAME Strs
#define DTYPE char*

typedef struct {
  DTYPE *d;
  size_t allocated;
  size_t used;
} NAME;

// Initialize a NAME object
void initNAME(NAME* new_arr)
{
  const size_t initSize = 2;
  new_arr->d = malloc(initSize * sizeof(DTYPE));
  new_arr->allocated = initSize;
  new_arr->used = 0;
}

// Free all memory allocated in a NAME
void deallocNAME(NAME* arr)
{
  size_t i;

  for (i = 0; i < arr->used; i++)
  {
    free(arr->d[i]);
  }

  free(arr->d);
}

// Double size of allocated memory
void _growNAME(NAME* arr)
{
  arr->d = realloc(arr->d, arr->allocated*2*sizeof(char*));
  arr->allocated *= 2;
}

// Add a new string to the end of a NAME array
void pushNAME(NAME* arr, char* new_string)
{
  if (!(arr->used < arr->allocated))
  {
    _growNAME(arr);
  }

  arr->d[arr->used] = new_string;
  arr->used++;
}

// Find the first occurence of search in arr and return its index.
// Return SIZE_MAX if search is not in arr.
size_t containsNAME(NAME* arr, char* search)
{
  size_t i;

  for (i = 0; i < arr->used; i++)
  {
    if (strcmp(search, arr->d[i]) == 0)
    {
      return i;
    }
  }
  
  return SIZE_MAX;
}

// Remove all elements of a NAME array at an index greater than or equal to idx
void truncateNAME(NAME* arr, size_t idx)
{
  size_t i;

  for (i = idx; i < arr->used; i++)
  {
    free(arr->d[i]);
    arr->d[i] = NULL;
  }

  arr->used = idx;
}
