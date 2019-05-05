//
// Created by Nie R on 2019/4/30.
//

#ifndef NLOGGER_NLOGGER_CONSTANTS_H
#define NLOGGER_NLOGGER_CONSTANTS_H

#endif //NLOGGER_NLOGGER_CONSTANTS_H
#define NLOGGER_CACHE_DIR  "nlogger_cache"
#define NLOGGER_CACHE_FILE_NAME "cache.mmap"

#define NLOGGER_MMAP_CACHE_HEADER_KEY_VERSION "version"
#define NLOGGER_MMAP_CACHE_HEADER_KEY_FILE "file"
#define NLOGGER_MMAP_CACHE_HEADER_KEY_DATE "date"

#define NLOGGER_MMAP_CACHE_HEADER_HEAD_TAG '^'
#define NLOGGER_MMAP_CACHE_HEADER_TAIL_TAG '$'


//(int cache_type, int flag, char *log_content, long long local_time,
//char *thread_name, long long thread_id,

#define NLOGGER_PROTOCOL_KEY_CACHE_TYPE "ct"
#define NLOGGER_PROTOCOL_KEY_FLAG "f"
#define NLOGGER_PROTOCOL_KEY_CONTENT "c"
#define NLOGGER_PROTOCOL_KEY_LOCAL_TIME "t"
#define NLOGGER_PROTOCOL_KEY_THREAD_NAME "tn"
#define NLOGGER_PROTOCOL_KEY_THREAD_ID "ti"

#define NLOGGER_VERSION 1

#define NLOGGER_CONTENT_LENGTH_BYTE_SIZE 3
