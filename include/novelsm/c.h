/* Copyright (c) 2011 The NOVELSM Authors. All rights reserved.
  Use of this source code is governed by a BSD-style license that can be
  found in the LICENSE file. See the AUTHORS file for names of contributors.

  C bindings for novelsm.  May be useful as a stable ABI that can be
  used by programs that keep novelsm in a shared library, or for
  a JNI api.

  Does not support:
  . getters for the option types
  . custom comparators that implement key shortening
  . custom iter, db, env, cache implementations using just the C bindings

  Some conventions:

  (1) We expose just opaque struct pointers and functions to clients.
  This allows us to change internal representations without having to
  recompile clients.

  (2) For simplicity, there is no equivalent to the Slice type.  Instead,
  the caller has to pass the pointer and length as separate
  arguments.

  (3) Errors are represented by a null-terminated c string.  NULL
  means no error.  All operations that can raise an error are passed
  a "char** errptr" as the last argument.  One of the following must
  be true on entry:
     *errptr == NULL
     *errptr points to a malloc()ed null-terminated error message
       (On Windows, *errptr must have been malloc()-ed by this library.)
  On success, a novelsm routine leaves *errptr unchanged.
  On failure, novelsm frees the old value of *errptr and
  set *errptr to a malloc()ed error message.

  (4) Bools have the type unsigned char (0 == false; rest == true)

  (5) All of the pointer arguments must be non-NULL.
*/

#ifndef STORAGE_NOVELSM_INCLUDE_C_H_
#define STORAGE_NOVELSM_INCLUDE_C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/* Exported types */

typedef struct NOVELSM_t               NOVELSM_t;
typedef struct NOVELSM_cache_t         NOVELSM_cache_t;
typedef struct NOVELSM_comparator_t    NOVELSM_comparator_t;
typedef struct NOVELSM_env_t           NOVELSM_env_t;
typedef struct NOVELSM_filelock_t      NOVELSM_filelock_t;
typedef struct NOVELSM_filterpolicy_t  NOVELSM_filterpolicy_t;
typedef struct NOVELSM_iterator_t      NOVELSM_iterator_t;
typedef struct NOVELSM_logger_t        NOVELSM_logger_t;
typedef struct NOVELSM_options_t       NOVELSM_options_t;
typedef struct NOVELSM_randomfile_t    NOVELSM_randomfile_t;
typedef struct NOVELSM_readoptions_t   NOVELSM_readoptions_t;
typedef struct NOVELSM_seqfile_t       NOVELSM_seqfile_t;
typedef struct NOVELSM_snapshot_t      NOVELSM_snapshot_t;
typedef struct NOVELSM_writablefile_t  NOVELSM_writablefile_t;
typedef struct NOVELSM_writebatch_t    NOVELSM_writebatch_t;
typedef struct NOVELSM_writeoptions_t  NOVELSM_writeoptions_t;

/* DB operations */

extern NOVELSM_t* NOVELSM_open(
    const NOVELSM_options_t* options,
    const char* name,
    char** errptr);

extern void NOVELSM_close(NOVELSM_t* db);

extern void NOVELSM_put(
    NOVELSM_t* db,
    const NOVELSM_writeoptions_t* options,
    const char* key, size_t keylen,
    const char* val, size_t vallen,
    char** errptr);

extern void NOVELSM_delete(
    NOVELSM_t* db,
    const NOVELSM_writeoptions_t* options,
    const char* key, size_t keylen,
    char** errptr);

extern void NOVELSM_write(
    NOVELSM_t* db,
    const NOVELSM_writeoptions_t* options,
    NOVELSM_writebatch_t* batch,
    char** errptr);

/* Returns NULL if not found.  A malloc()ed array otherwise.
   Stores the length of the array in *vallen. */
extern char* NOVELSM_get(
    NOVELSM_t* db,
    const NOVELSM_readoptions_t* options,
    const char* key, size_t keylen,
    size_t* vallen,
    char** errptr);

extern NOVELSM_iterator_t* NOVELSM_create_iterator(
    NOVELSM_t* db,
    const NOVELSM_readoptions_t* options);

extern const NOVELSM_snapshot_t* NOVELSM_create_snapshot(
    NOVELSM_t* db);

extern void NOVELSM_release_snapshot(
    NOVELSM_t* db,
    const NOVELSM_snapshot_t* snapshot);

/* Returns NULL if property name is unknown.
   Else returns a pointer to a malloc()-ed null-terminated value. */
extern char* NOVELSM_property_value(
    NOVELSM_t* db,
    const char* propname);

extern void NOVELSM_approximate_sizes(
    NOVELSM_t* db,
    int num_ranges,
    const char* const* range_start_key, const size_t* range_start_key_len,
    const char* const* range_limit_key, const size_t* range_limit_key_len,
    uint64_t* sizes);

extern void NOVELSM_compact_range(
    NOVELSM_t* db,
    const char* start_key, size_t start_key_len,
    const char* limit_key, size_t limit_key_len);

/* Management operations */

extern void NOVELSM_destroy_db(
    const NOVELSM_options_t* options,
    const char* name,
    char** errptr);

extern void NOVELSM_repair_db(
    const NOVELSM_options_t* options,
    const char* name,
    char** errptr);

/* Iterator */

extern void NOVELSM_iter_destroy(NOVELSM_iterator_t*);
extern unsigned char NOVELSM_iter_valid(const NOVELSM_iterator_t*);
extern void NOVELSM_iter_seek_to_first(NOVELSM_iterator_t*);
extern void NOVELSM_iter_seek_to_last(NOVELSM_iterator_t*);
extern void NOVELSM_iter_seek(NOVELSM_iterator_t*, const char* k, size_t klen);
extern void NOVELSM_iter_next(NOVELSM_iterator_t*);
extern void NOVELSM_iter_prev(NOVELSM_iterator_t*);
extern const char* NOVELSM_iter_key(const NOVELSM_iterator_t*, size_t* klen);
extern const char* NOVELSM_iter_value(const NOVELSM_iterator_t*, size_t* vlen);
extern void NOVELSM_iter_get_error(const NOVELSM_iterator_t*, char** errptr);

/* Write batch */

extern NOVELSM_writebatch_t* NOVELSM_writebatch_create();
extern void NOVELSM_writebatch_destroy(NOVELSM_writebatch_t*);
extern void NOVELSM_writebatch_clear(NOVELSM_writebatch_t*);
extern void NOVELSM_writebatch_put(
    NOVELSM_writebatch_t*,
    const char* key, size_t klen,
    const char* val, size_t vlen);
extern void NOVELSM_writebatch_delete(
    NOVELSM_writebatch_t*,
    const char* key, size_t klen);
extern void NOVELSM_writebatch_iterate(
    NOVELSM_writebatch_t*,
    void* state,
    void (*put)(void*, const char* k, size_t klen, const char* v, size_t vlen),
    void (*deleted)(void*, const char* k, size_t klen));

/* Options */

extern NOVELSM_options_t* NOVELSM_options_create();
extern void NOVELSM_options_destroy(NOVELSM_options_t*);
extern void NOVELSM_options_set_comparator(
    NOVELSM_options_t*,
    NOVELSM_comparator_t*);
extern void NOVELSM_options_set_filter_policy(
    NOVELSM_options_t*,
    NOVELSM_filterpolicy_t*);
extern void NOVELSM_options_set_create_if_missing(
    NOVELSM_options_t*, unsigned char);
extern void NOVELSM_options_set_error_if_exists(
    NOVELSM_options_t*, unsigned char);
extern void NOVELSM_options_set_paranoid_checks(
    NOVELSM_options_t*, unsigned char);
extern void NOVELSM_options_set_env(NOVELSM_options_t*, NOVELSM_env_t*);
extern void NOVELSM_options_set_info_log(NOVELSM_options_t*, NOVELSM_logger_t*);
extern void NOVELSM_options_set_write_buffer_size(NOVELSM_options_t*, size_t);
extern void NOVELSM_options_set_max_open_files(NOVELSM_options_t*, int);
extern void NOVELSM_options_set_cache(NOVELSM_options_t*, NOVELSM_cache_t*);
extern void NOVELSM_options_set_block_size(NOVELSM_options_t*, size_t);
extern void NOVELSM_options_set_block_restart_interval(NOVELSM_options_t*, int);

enum {
  NOVELSM_no_compression = 0,
  NOVELSM_snappy_compression = 1
};
extern void NOVELSM_options_set_compression(NOVELSM_options_t*, int);

/* Comparator */

extern NOVELSM_comparator_t* NOVELSM_comparator_create(
    void* state,
    void (*destructor)(void*),
    int (*compare)(
        void*,
        const char* a, size_t alen,
        const char* b, size_t blen),
    const char* (*name)(void*));
extern void NOVELSM_comparator_destroy(NOVELSM_comparator_t*);

/* Filter policy */

extern NOVELSM_filterpolicy_t* NOVELSM_filterpolicy_create(
    void* state,
    void (*destructor)(void*),
    char* (*create_filter)(
        void*,
        const char* const* key_array, const size_t* key_length_array,
        int num_keys,
        size_t* filter_length),
    unsigned char (*key_may_match)(
        void*,
        const char* key, size_t length,
        const char* filter, size_t filter_length),
    const char* (*name)(void*));
extern void NOVELSM_filterpolicy_destroy(NOVELSM_filterpolicy_t*);

extern NOVELSM_filterpolicy_t* NOVELSM_filterpolicy_create_bloom(
    int bits_per_key);

/* Read options */

extern NOVELSM_readoptions_t* NOVELSM_readoptions_create();
extern void NOVELSM_readoptions_destroy(NOVELSM_readoptions_t*);
extern void NOVELSM_readoptions_set_verify_checksums(
    NOVELSM_readoptions_t*,
    unsigned char);
extern void NOVELSM_readoptions_set_fill_cache(
    NOVELSM_readoptions_t*, unsigned char);
extern void NOVELSM_readoptions_set_snapshot(
    NOVELSM_readoptions_t*,
    const NOVELSM_snapshot_t*);

/* Write options */

extern NOVELSM_writeoptions_t* NOVELSM_writeoptions_create();
extern void NOVELSM_writeoptions_destroy(NOVELSM_writeoptions_t*);
extern void NOVELSM_writeoptions_set_sync(
    NOVELSM_writeoptions_t*, unsigned char);

/* Cache */

extern NOVELSM_cache_t* NOVELSM_cache_create_lru(size_t capacity);
extern void NOVELSM_cache_destroy(NOVELSM_cache_t* cache);

/* Env */

extern NOVELSM_env_t* NOVELSM_create_default_env();
extern void NOVELSM_env_destroy(NOVELSM_env_t*);

/* Utility */

/* Calls free(ptr).
   REQUIRES: ptr was malloc()-ed and returned by one of the routines
   in this file.  Note that in certain cases (typically on Windows), you
   may need to call this routine instead of free(ptr) to dispose of
   malloc()-ed memory returned by this library. */
extern void NOVELSM_free(void* ptr);

/* Return the major version number for this release. */
extern int NOVELSM_major_version();

/* Return the minor version number for this release. */
extern int NOVELSM_minor_version();

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif  /* STORAGE_NOVELSM_INCLUDE_C_H_ */
