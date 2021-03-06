//
// Created by fgd on 2019/4/28.
//

#ifndef NLOGGER_NLOGGER_ERROR_CODE_H
#define NLOGGER_NLOGGER_ERROR_CODE_H

#endif //NLOGGER_NLOGGER_ERROR_CODE_H
#define ERROR_CODE_CHANGE_CACHE_MODE_TO_MEMORY 3
#define ERROR_CODE_NEW_LOG_FILE_OPENED 2
#define ERROR_CODE_OK 1
#define ERROR_CODE_ILLEGAL_ARGUMENT (-1)
#define ERROR_CODE_MALLOC_NLOGGER_STRUCT (-2)
#define ERROR_CODE_MALLOC_CACHE_DIR_STRING (-3)
#define ERROR_CODE_CREATE_LOG_CACHE_DIR_FAILED (-4)
#define ERROR_CODE_MALLOC_LOG_FILE_DIR_STRING (-5)
#define ERROR_CODE_NEED_INIT_NLOGGER_BEFORE_ACTION (-6)

#define ERROR_CODE_MALLOC_LOG_FILE_PATH_STRING (-7)
#define ERROR_CODE_MALLOC_LOG_FILE_NAME_STRING (-8)

#define ERROR_CODE_OPEN_LOG_FILE_FAILED (-9)

#define ERROR_CODE_MAP_LOG_NAME_TO_CACHE_FAILED (-10)

#define ERROR_CODE_INIT_CACHE_FAILED (-11)

#define ERROR_CODE_BUILD_LOG_BLOCK_FAILED (-12)

#define ERROR_CODE_INIT_HANDLER_MALLOC_STREAM_FAILED (-13)
#define ERROR_CODE_INIT_HANDLER_INIT_STREAM_FAILED (-14)

#define ERROR_CODE_COMPRESS_FAILED (-15)

#define ERROR_CODE_INVALID_BUFFER_POINT_ON_PARSE_MMAP_HEADER (-16)

#define ERROR_CODE_INVALID_HEADER_LENGTH_ON_PARSE_MMAP_HEADER (-17)

#define ERROR_CODE_INVALID_HEAD_OR_TAIL_TAG_ON_PARSE_MMAP_HEADER (-18)

#define ERROR_CODE_BAD_HEADER_CONTENT_ON_PARSE_MMAP_HEADER (-19)

#define ERROR_CODE_UNEXPECT_STATE_WHEN_ON_FLUSH (-20)

#define ERROR_CODE_CAN_NOT_PARSE_ON_MEMORY_CACHE_MODE (-21)

#define ERROR_CODE_BUILD_LOG_JSON_DATA_FAILED (-22)

#define ERROR_CODE_BUILD_CACHE_HEADER_JSON_DATA_FAILED (-23)

#define ERROR_CODE_MALLOC_FAILED_ON_PARSE_MMAP_HEADER (-24)

#define ERROR_CODE_LOG_FILE_NAME_NOT_CONFIG (-25)

#define ERROR_CODE_BAD_LOG_FILE_STATE_ON_FLUSH (-26)

#define ERROR_CODE_CREATE_CACHE_FAILED (-27)

#define ERROR_CODE_RESET_DATA_HANDLER_FAILED (-28)

#define ERROR_CODE_LOG_FILE_ON_MAX_SIZE (-29)

#define ERROR_CODE_ALREADY_AN_DIFFERENT_LOG_FILE_ON_OPEN (-30)
