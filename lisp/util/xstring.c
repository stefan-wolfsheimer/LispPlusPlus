#include "xstring.h"
#include "xmalloc.h"
#include <string.h>
#include <stdio.h>

char * alloc_strcpy(const char * str)
{
  if(str == NULL) 
  {
    return NULL;
  }
  else 
  {
    size_t len = strlen(str);
    char * tmp = MALLOC(len + 1);
    if(tmp != NULL) 
    {
      strcpy(tmp, str);
    }
    return tmp;
  }
}

char * alloc_sprintf(const char * fmt, ...)
{
  int       size;
  va_list   ap1, ap2;
  char    * ret;
  va_start(ap1, fmt);
  va_copy(ap2, ap1);
  size = vsnprintf(NULL, 0, fmt, ap1);
  va_end(ap1);
  ret = MALLOC( sizeof(char) * (size+1) );
  if(ret) 
  {
    vsprintf(ret, fmt, ap2);  
  }
  va_end(ap2);
  return ret;
}
