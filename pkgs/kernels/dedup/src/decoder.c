/*
 * Decoder for dedup files
 *
 * Copyright 2010 Princeton University.
 * All rights reserved.
 *
 * Written by Christian Bienia.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "decoder.h"
#include "dedupdef.h"
#include "config.h"
#include "util.h"
#include "hashtable.h"
#include "mbuffer.h"
#include "debug.h"

#ifdef ENABLE_GZIP_COMPRESSION
#include <zlib.h>
#endif //ENABLE_GZIP_COMPRESSION

#ifdef ENABLE_BZIP2_COMPRESSION
#include <bzlib.h>
#endif //ENABLE_BZIP2_COMPRESSION

#ifdef ENABLE_PARSEC_HOOKS
#include <hooks.h>
#endif //ENABLE_PARSEC_HOOKS



//The configuration block defined in main
config_t * conf;

//Hash table data structure & utility functions
struct hashtable *cache;

static unsigned int hash_from_key_fn( void *k ) {
  //NOTE: sha1 sum is integer-aligned
  return ((unsigned int *)k)[0];
}

static int keys_equal_fn ( void *key1, void *key2 ) {
  return (memcmp(key1, key2, SHA1_LEN) == 0);
}



/*
 * Helper function which reads the next chunk from the input file
 *
 * Returns the size of the data read
 */
static int read_chunk(int fd, chunk_t *chunk) {
  int r;

  assert(chunk!=NULL);
  assert(fd>=0);

  u_char type;
  r=xread(fd, &type, sizeof(type));
  if(r < 0) EXIT_TRACE("xread type fails\n")
  else if(r == 0) return 0;

  u_long len;
  r=xread(fd, &len, sizeof(len));
  if(r < 0) EXIT_TRACE("xread length fails\n")
  else if(r == 0) EXIT_TRACE("incomplete chunk\n");

  switch(type) {
  case TYPE_FINGERPRINT:
    if(len!=SHA1_LEN) EXIT_TRACE("incorrect size of SHA1 sum\n");
    r=xread(fd, (unsigned char *)(chunk->sha1), SHA1_LEN);
    if(r < 0) EXIT_TRACE("xread SHA1 sum fails\n")
    else if(r == 0) EXIT_TRACE("incomplete chunk\n");
    chunk->header.isDuplicate = TRUE;
    break;
  case TYPE_COMPRESS:
    if(len<=0) EXIT_TRACE("illegal size of data chunk\n");
    r = mbuffer_create(&chunk->compressed_data, len);
    if(r != 0) EXIT_TRACE("Creation of input buffer failed.\n");
    r=xread(fd, (unsigned char *)(chunk->compressed_data.ptr), chunk->compressed_data.n);
    if(r < 0) EXIT_TRACE("xread data chunk fails\n")
    else if(r == 0) EXIT_TRACE("incomplete chunk\n");
    chunk->header.isDuplicate = FALSE;
    break;
  default:
    EXIT_TRACE("unknown chunk type\n");
  }

  return len;
}

/* Helper function which uncompresses a data chunk
 *
 * Returns the size of the uncompressed data
 */
static int uncompress_chunk(chunk_t *chunk) {
  int r;

  assert(chunk!=NULL);
  assert(!chunk->header.isDuplicate);

  //uncompress the item
  switch (conf->compress_type) {
  case COMPRESS_NONE: {
    //Simply duplicate the data
    r = mbuffer_create(&chunk->uncompressed_data, chunk->compressed_data.n);
    if(r != 0) EXIT_TRACE("Creation of decompression buffer failed.\n");
    //copy the block
    memcpy(chunk->uncompressed_data.ptr, chunk->compressed_data.ptr, chunk->compressed_data.n);
    break;
  }
#ifdef ENABLE_GZIP_COMPRESSION
  case COMPRESS_GZIP: {
    unsigned long len_64 = UNCOMPRESS_BOUND;
    r = mbuffer_create(&chunk->uncompressed_data, len_64);
    if(r != 0) EXIT_TRACE("Creation of decompression buffer failed.\n");
    r = uncompress(chunk->uncompressed_data.ptr, &len_64, chunk->compressed_data.ptr, chunk->compressed_data.n);
    //TODO: Automatically enlarge buffer if return value is Z_BUF_ERROR
    if(r!=Z_OK) EXIT_TRACE("error uncompressing chunk data\n");
    //Shrink buffer to actual size
    if(len_64 < chunk->uncompressed_data.n) {
      r = mbuffer_realloc(&chunk->uncompressed_data, len_64);
      assert(r == 0);
    }
    break;
  }
#else
  case COMPRESS_GZIP: {
    EXIT_TRACE("Gzip compression used by input file not supported.\n");
    break;
  }
#endif //ENABLE_GZIP_COMPRESSION
#ifdef ENABLE_BZIP2_COMPRESSION
  case COMPRESS_BZIP2: {
    unsigned int len_32 = UNCOMPRESS_BOUND;
    r = mbuffer_create(&chunk->uncompressed_data, len_32);
    if(r != 0) EXIT_TRACE("Creation of decompression buffer failed.\n");
    r = BZ2_bzBuffToBuffDecompress(chunk->uncompressed_data.ptr, &len_32, chunk->compressed_data.ptr, chunk->compressed_data.n, 0, 0);
    //TODO: Automatically enlarge buffer if return value is BZ_OUTBUFF_FULL
    if(r!=BZ_OK) EXIT_TRACE("error uncompressing chunk data\n");
    //Shrink buffer to actual size
    if(len_32 < chunk->uncompressed_data.n) {
      r = mbuffer_realloc(&chunk->uncompressed_data, len_32);
      assert(r == 0);
    }
    break;
  }
#else
  case COMPRESS_BZIP2: {
    EXIT_TRACE("Bzip2 compression used by input file not supported.\n");
    break;
  }
#endif //ENABLE_BZIP2_COMPRESSION
  default: {
    EXIT_TRACE("unknown compression type\n");
    break;
  }
  }

  mbuffer_free(&chunk->compressed_data);
  return chunk->uncompressed_data.n;
}


void Decode(config_t * _conf) {
  int fd_in;
  int fd_out;
  chunk_t *chunk=NULL;
  int r;

  conf = _conf;

  //Create chunk cache
  cache = hashtable_create(65536, hash_from_key_fn, keys_equal_fn, FALSE);
  if(cache == NULL) {
    printf("ERROR: Out of memory\n");
    exit(1);
  }

  mbuffer_system_init();

  //Open input & output files
  fd_in = open(conf->infile, O_RDONLY|O_LARGEFILE);
  if (fd_in < 0) {
    perror("infile open");
    exit(1);
  }
  byte compress_type;
  if (read_header(fd_in, &compress_type)) {
    EXIT_TRACE("Cannot read input file header.\n");
  }
  //Ignore any compression settings given at the command line, use type used during encoding
  conf->compress_type = compress_type;
  fd_out = open(conf->outfile, O_CREAT|O_WRONLY|O_TRUNC, ~(S_ISUID | S_ISGID |S_IXGRP | S_IXUSR | S_IXOTH));
  if (fd_out < 0) {
    perror("outfile open");
    close(fd_in);
    exit(1);
  }

#ifdef ENABLE_PARSEC_HOOKS
    __parsec_roi_begin();
#endif

  while(TRUE) {
    //chunks are 'consumed' if they are added to the hash table
    //only duplicate chunks can get reused, malloc a new one otherwise
    if(chunk==NULL) {
      chunk = (chunk_t *)malloc(sizeof(chunk_t));
      if(chunk==NULL) EXIT_TRACE("Memory allocation failed.\n");
    }

    //get input data
    r=read_chunk(fd_in, chunk);
    if(r<0) EXIT_TRACE("error reading from input file")
    else if(r==0) break;

    //process input data & assing chunk with corresponding uncompresse data to 'entry' variable
    chunk_t *entry;
    if(!chunk->header.isDuplicate) {
      //We got the compressed data, use it to get original data back
      r=uncompress_chunk(chunk);
      if(r<=0) EXIT_TRACE("error uncompressing data")
      //Compute SHA1 sum and add new chunk with uncompressed data to cache
      SHA1_Digest(chunk->uncompressed_data.ptr, chunk->uncompressed_data.n, (unsigned char *)(chunk->sha1));
      if(hashtable_insert(cache, (void *)(chunk->sha1), (void *)chunk) == 0) {
        EXIT_TRACE("hashtable_insert failed");
      }
      entry=chunk;
      chunk=NULL;
    } else {
      //We got a SHA1 key, use it to retrieve unique counterpart with uncompressed data
      entry=(chunk_t *)hashtable_search(cache, (void *)(chunk->sha1));
      if(entry==NULL) {
        EXIT_TRACE("Encountered a duplicate chunk in input file but not its unique counterpart. Maybe data is out of order?");
      }
    }
    //We now have the uncompressed data in 'entry', write uncompressed data to output file
    if(xwrite(fd_out, entry->uncompressed_data.ptr, entry->uncompressed_data.n)<entry->uncompressed_data.n) {
      EXIT_TRACE("error writing to output file");
    }
  }

#ifdef ENABLE_PARSEC_HOOKS
    __parsec_roi_end();
#endif

  close(fd_in);
  close(fd_out);

  free(chunk);
  mbuffer_system_destroy();
  //NOTE: Would have to iterate through hashtable and manually free all buffers. Calling
  //      hashtable_destroy will cause those buffers to be reported as leaked memory.
  //hashtable_destroy(cache, TRUE);
}

