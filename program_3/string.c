typedef struct {
  char** d;
  size_t allocated;
  size_t used;
} Strs;

void initStrs(Strs* new_strs)
{
  const size_t initSize = 2;
  new_strs->d = malloc(initSize * sizeof(char*));
  new_strs->allocated = initSize;
  new_strs->used = 0;
}

void deallocStrs(Strs* strs)
{
  int i;

  for (i = 0; i < strs->used; i++)
  {
    free(strs->d[i]);
  }

  free(strs->d);
}

void growStrs(Strs* strs)
{
  strs->d = realloc(strs->d, strs->allocated*2*sizeof(char*));
  strs->allocated *= 2;
}

void pushStrs(Strs* strs, char* new_string)
{
  if (!(strs->used < strs->allocated))
  {
    growStrs(strs);
  }

  strs->d[strs->used] = new_string;
  strs->used++;
}

// Find the first occurence of search in strs and return its index.
// Return -1 if search is not in strs.
// FIXME: should return size_t, but that breaks comparisons with int
int containsStrs(Strs* strs, char* search)
{
  int i;

  for (i = 0; i < strs->used; i++)
  {
    if (strcmp(search, strs->d[i]) == 0)
    {
      //printf("[containsStrs] Found the string %s you're looking for at index %i\n", search, i);
      return i;
    }
  }
  
  //printf("[containsStrs] Didn't find the string %s you're looking\n", search);
  return -1;
}

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
