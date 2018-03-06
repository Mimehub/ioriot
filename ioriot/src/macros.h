// Copyright 2018 Mimecast Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef MACROS_H
#define MACROS_H

#define Cleanup(code) ret = code; goto cleanup
#define Cleanup_unless(expr, code) \
    if (expr != code) { ret = code; goto cleanup; }

// String helpers
#define Clone(str) notnull(strdup(str),__FILE__,__LINE__,0)
#define Eq(str1,str2) strcmp(str1,str2) == 0

// Number helpers
#define Abs(num) num >= 0 ? num : -num
#define Readhex(str) strtol(str, NULL, 16)
#define Perc(a, b) a > b ? b/(a/100.) : a/(b/100.)

// Bitwise helpers
#define Has(flags, what) (flags & (what)) == (what)
#define Hasnt(flags, what) (flags & (what)) != (what)

// Memory helpers
#define Malloc(what) \
    notnull(malloc(sizeof(what)),__FILE__,__LINE__,1)
#define Calloc(count,what) \
    notnull(calloc(count,sizeof(what)),__FILE__,__LINE__,count)
#define Mset(where,value,count,what) \
    memset(where,value,count*sizeof(what))

// Open helpers
#define Fopen(path, mode) fnotnull(fopen(path, mode), path, __FILE__, __LINE__)

// Mmap helpers
#define Mmapshared(what) \
  mmapok(mmap(NULL, sizeof(what), \
              PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0), \
      __FILE__,__LINE__)
#define Cmapshared(count,what) \
  mmapok(mmap(NULL, count*sizeof(what), \
              PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0), \
      __FILE__,__LINE__)

// Printing messages
#define Out(...) \
  fprintf(stdout, __VA_ARGS__); \
  fflush(stdout);
#define Put(...) \
  fprintf(stdout, __VA_ARGS__); \
  fprintf(stdout, "\n"); \
  fflush(stdout);

// Printing debug messages
#define Debug(...) \
  fprintf(stderr, "%s:%d DEBUG: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  fflush(stderr);

// Printing error messages
#define Error(...) \
  fprintf(stderr, "%s:%d ERROR: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr,"\n"); \
  fflush(stdout); \
  fflush(stderr); \
  exit(ERROR);

#define Error_if(expr, ...) if (expr) { Error(__VA_ARGS__); }

#define Errno(...) \
  fprintf(stderr, "%s:%d ERROR: %s (%d). ", __FILE__, __LINE__, \
      strerror(errno), errno); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr,"\n"); \
  fflush(stdout); \
  fflush(stderr); \
  exit(ERROR);

#define Errno_if(expr, ...) if (expr) { Errno(__VA_ARGS__); }

#define Segfault(...) \
  fprintf(stderr, "%s:%d ERROR: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr,"\n"); \
  fflush(stdout); \
  fflush(stderr); \
  *(int*)0 = 0;

// Printing warn messages
#define Warn(...) \
  fprintf(stderr, "WARN: "); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr,"\n"); \
  fflush(stdout); \
  fflush(stderr);

#define Warn_if(expr, ...) if (expr) { Warn(__VA_ARGS__); }

// Other helpers
#define Fill_with_stuff(buf, len) \
  for (int i = 0; i<len-1; ++i) { buf[i] = 'X'; }

#endif // MACROS_H
