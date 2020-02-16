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
