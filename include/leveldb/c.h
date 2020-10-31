/* Copyright (c) 2011 The LevelDB Authors. All rights reserved.
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

typedef struct novelsm_t               novelsm_t;
typedef struct novelsm_cache_t         novelsm_cache_t;
typedef struct novelsm_comparator_t    novelsm_comparator_t;
typedef struct novelsm_env_t           novelsm_env_t;
typedef struct novelsm_filelock_t      novelsm_filelock_t;
typedef struct novelsm_filterpolicy_t  novelsm_filterpolicy_t;
typedef struct novelsm_iterator_t      novelsm_iterator_t;
typedef struct novelsm_logger_t        novelsm_logger_t;
typedef struct novelsm_options_t       novelsm_options_t;
typedef struct novelsm_randomfile_t    novelsm_randomfile_t;
typedef struct novelsm_readoptions_t   novelsm_readoptions_t;
typedef struct novelsm_seqfile_t       novelsm_seqfile_t;
typedef struct novelsm_snapshot_t      novelsm_snapshot_t;
typedef struct novelsm_writablefile_t  novelsm_writablefile_t;
typedef struct novelsm_writebatch_t    novelsm_writebatch_t;
typedef struct novelsm_writeoptions_t  novelsm_writeoptions_t;

/* DB operations */

extern novelsm_t* novelsm_open(
    const novelsm_options_t* options,
    const char* name,
    char** errptr);

extern void novelsm_close(novelsm_t* db);

extern void novelsm_put(
    novelsm_t* db,
    const novelsm_writeoptions_t* options,
    const char* key, size_t keylen,
    const char* val, size_t vallen,
    char** errptr);

extern void novelsm_delete(
    novelsm_t* db,
    const novelsm_writeoptions_t* options,
    const char* key, size_t keylen,
    char** errptr);

extern void novelsm_write(
    novelsm_t* db,
    const novelsm_writeoptions_t* options,
    novelsm_writebatch_t* batch,
    char** errptr);

/* Returns NULL if not found.  A malloc()ed array otherwise.
   Stores the length of the array in *vallen. */
extern char* novelsm_get(
    novelsm_t* db,
    const novelsm_readoptions_t* options,
    const char* key, size_t keylen,
    size_t* vallen,
    char** errptr);

extern novelsm_iterator_t* novelsm_create_iterator(
    novelsm_t* db,
    const novelsm_readoptions_t* options);

extern const novelsm_snapshot_t* novelsm_create_snapshot(
    novelsm_t* db);

extern void novelsm_release_snapshot(
    novelsm_t* db,
    const novelsm_snapshot_t* snapshot);

/* Returns NULL if property name is unknown.
   Else returns a pointer to a malloc()-ed null-terminated value. */
extern char* novelsm_property_value(
    novelsm_t* db,
    const char* propname);

extern void novelsm_approximate_sizes(
    novelsm_t* db,
    int num_ranges,
    const char* const* range_start_key, const size_t* range_start_key_len,
    const char* const* range_limit_key, const size_t* range_limit_key_len,
    uint64_t* sizes);

extern void novelsm_compact_range(
    novelsm_t* db,
    const char* start_key, size_t start_key_len,
    const char* limit_key, size_t limit_key_len);

/* Management operations */

extern void novelsm_destroy_db(
    const novelsm_options_t* options,
    const char* name,
    char** errptr);

extern void novelsm_repair_db(
    const novelsm_options_t* options,
    const char* name,
    char** errptr);

/* Iterator */

extern void novelsm_iter_destroy(novelsm_iterator_t*);
extern unsigned char novelsm_iter_valid(const novelsm_iterator_t*);
extern void novelsm_iter_seek_to_first(novelsm_iterator_t*);
extern void novelsm_iter_seek_to_last(novelsm_iterator_t*);
extern void novelsm_iter_seek(novelsm_iterator_t*, const char* k, size_t klen);
extern void novelsm_iter_next(novelsm_iterator_t*);
extern void novelsm_iter_prev(novelsm_iterator_t*);
extern const char* novelsm_iter_key(const novelsm_iterator_t*, size_t* klen);
extern const char* novelsm_iter_value(const novelsm_iterator_t*, size_t* vlen);
extern void novelsm_iter_get_error(const novelsm_iterator_t*, char** errptr);

/* Write batch */

extern novelsm_writebatch_t* novelsm_writebatch_create();
extern void novelsm_writebatch_destroy(novelsm_writebatch_t*);
extern void novelsm_writebatch_clear(novelsm_writebatch_t*);
extern void novelsm_writebatch_put(
    novelsm_writebatch_t*,
    const char* key, size_t klen,
    const char* val, size_t vlen);
extern void novelsm_writebatch_delete(
    novelsm_writebatch_t*,
    const char* key, size_t klen);
extern void novelsm_writebatch_iterate(
    novelsm_writebatch_t*,
    void* state,
    void (*put)(void*, const char* k, size_t klen, const char* v, size_t vlen),
    void (*deleted)(void*, const char* k, size_t klen));

/* Options */

extern novelsm_options_t* novelsm_options_create();
extern void novelsm_options_destroy(novelsm_options_t*);
extern void novelsm_options_set_comparator(
    novelsm_options_t*,
    novelsm_comparator_t*);
extern void novelsm_options_set_filter_policy(
    novelsm_options_t*,
    novelsm_filterpolicy_t*);
extern void novelsm_options_set_create_if_missing(
    novelsm_options_t*, unsigned char);
extern void novelsm_options_set_error_if_exists(
    novelsm_options_t*, unsigned char);
extern void novelsm_options_set_paranoid_checks(
    novelsm_options_t*, unsigned char);
extern void novelsm_options_set_env(novelsm_options_t*, novelsm_env_t*);
extern void novelsm_options_set_info_log(novelsm_options_t*, novelsm_logger_t*);
extern void novelsm_options_set_write_buffer_size(novelsm_options_t*, size_t);
extern void novelsm_options_set_max_open_files(novelsm_options_t*, int);
extern void novelsm_options_set_cache(novelsm_options_t*, novelsm_cache_t*);
extern void novelsm_options_set_block_size(novelsm_options_t*, size_t);
extern void novelsm_options_set_block_restart_interval(novelsm_options_t*, int);

enum {
  novelsm_no_compression = 0,
  novelsm_snappy_compression = 1
};
extern void novelsm_options_set_compression(novelsm_options_t*, int);

/* Comparator */

extern novelsm_comparator_t* novelsm_comparator_create(
    void* state,
    void (*destructor)(void*),
    int (*compare)(
        void*,
        const char* a, size_t alen,
        const char* b, size_t blen),
    const char* (*name)(void*));
extern void novelsm_comparator_destroy(novelsm_comparator_t*);

/* Filter policy */

extern novelsm_filterpolicy_t* novelsm_filterpolicy_create(
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
extern void novelsm_filterpolicy_destroy(novelsm_filterpolicy_t*);

extern novelsm_filterpolicy_t* novelsm_filterpolicy_create_bloom(
    int bits_per_key);

/* Read options */

extern novelsm_readoptions_t* novelsm_readoptions_create();
extern void novelsm_readoptions_destroy(novelsm_readoptions_t*);
extern void novelsm_readoptions_set_verify_checksums(
    novelsm_readoptions_t*,
    unsigned char);
extern void novelsm_readoptions_set_fill_cache(
    novelsm_readoptions_t*, unsigned char);
extern void novelsm_readoptions_set_snapshot(
    novelsm_readoptions_t*,
    const novelsm_snapshot_t*);

/* Write options */

extern novelsm_writeoptions_t* novelsm_writeoptions_create();
extern void novelsm_writeoptions_destroy(novelsm_writeoptions_t*);
extern void novelsm_writeoptions_set_sync(
    novelsm_writeoptions_t*, unsigned char);

/* Cache */

extern novelsm_cache_t* novelsm_cache_create_lru(size_t capacity);
extern void novelsm_cache_destroy(novelsm_cache_t* cache);

/* Env */

extern novelsm_env_t* novelsm_create_default_env();
extern void novelsm_env_destroy(novelsm_env_t*);

/* Utility */

/* Calls free(ptr).
   REQUIRES: ptr was malloc()-ed and returned by one of the routines
   in this file.  Note that in certain cases (typically on Windows), you
   may need to call this routine instead of free(ptr) to dispose of
   malloc()-ed memory returned by this library. */
extern void novelsm_free(void* ptr);

/* Return the major version number for this release. */
extern int novelsm_major_version();

/* Return the minor version number for this release. */
extern int novelsm_minor_version();

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif  /* STORAGE_NOVELSM_INCLUDE_C_H_ */
