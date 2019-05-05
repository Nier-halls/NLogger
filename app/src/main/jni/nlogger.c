/*
 * Created by fgd on 2019/4/28.
 *
 * 日志库主文件，带下划线前缀的未私有方法，功能尽量独立，不引用全局的数据
 */

#include <malloc.h>
#include <sys/stat.h>
#include <cJSON.h>
#include <sys/mman.h>
#include "nlogger.h"
#include "nlogger_android_log.h"
#include "nlogger_error_code.h"
#include "nlogger_constants.h"
#include "nlogger_file_utils.h"
#include "nlogger_cache.h"
#include "nlogger_json_util.h"
#include "nlogger_utils.h"


static nlogger *g_nlogger;
static char    *g_null_string = "NULL";

void _get_string_data(char *data, char **result) {
    if (is_empty_string(data)) {
        *result = g_null_string;
        return;
    }
    *result = data;
}

void _print_current_nlogger() {
    if (g_nlogger == NULL) {
        return;
    }
    char *result;
    LOGD("debug", ">>>>>>>>>>>>>>>>  g_nlogger  >>>>>>>>>>>>>>>>");
    LOGD("debug", "cache.cache_mod = %d", g_nlogger->cache.cache_mode);
    LOGD("debug", "cache.length = %d", g_nlogger->cache.length);
    LOGD("debug", "cache.content_length = %d", g_nlogger->cache.content_length);
    _get_string_data(g_nlogger->cache.p_file_path, &result);
    LOGD("debug", "cache.p_file_path = %s", result);
    _get_string_data(g_nlogger->cache.p_buffer, &result);
    LOGD("debug", "cache.p_buffer = %s", result);
    LOGD("debug", "================================================");
    LOGD("debug", "log.state = %d", g_nlogger->log.state);
    LOGD("debug", "log.file_length = %ld", g_nlogger->log.file_length);
    _get_string_data(g_nlogger->log.p_path, &result);
    LOGD("debug", "log.p_path = %s", result);
    _get_string_data(g_nlogger->log.p_name, &result);
    LOGD("debug", "log.p_name = %s", result);
    _get_string_data(g_nlogger->log.p_dir, &result);
    LOGD("debug", "log.p_dir = %s", result);
    LOGD("debug", "<<<<<<<<<<<<<<<<  g_nlogger  <<<<<<<<<<<<<<<<");
}

/**
 * 创建mmap缓存文件用的路径（包含缓存文件名）
 */
int _create_cache_file_path(const char *cache_file_dir, char **result_path) {
    if (is_empty_string(cache_file_dir)) {
        return ERROR_CODE_ILLEGAL_ARGUMENT;
    }
    int appendSeparate = 0;
    if (cache_file_dir[strlen(cache_file_dir)] != '/') {
        appendSeparate = 1;
    }
    size_t path_string_length = strlen(cache_file_dir) +
                                (appendSeparate ? strlen("/") : 0) +
                                strlen(NLOGGER_CACHE_DIR) +
                                strlen("/") +
                                strlen(NLOGGER_CACHE_FILE_NAME) +
                                strlen("\0");
    *result_path = malloc(path_string_length);
    if (*result_path == NULL) {
        return ERROR_CODE_MALLOC_CACHE_DIR_STRING;
    }
    memset(*result_path, 0, path_string_length);
    memcpy(*result_path, cache_file_dir, strlen(cache_file_dir));
    if (appendSeparate) {
        strcat(*result_path, "/");
    }
    strcat(*result_path, NLOGGER_CACHE_DIR);
    strcat(*result_path, "/");
    if (makedir_nlogger(*result_path) < 0) {
        return ERROR_CODE_CREATE_LOG_CACHE_DIR_FAILED;
    }
    LOGD("init", "create mmap cache dir_string >>> %s", *result_path);
    strcat(*result_path, NLOGGER_CACHE_FILE_NAME);
    return ERROR_CODE_OK;
}

/**
 * 创建日志文件存放的目录（不包含日志文件名）
 */
int _create_log_file_dir(const char *log_file_dir, char **result_dir) {
    if (is_empty_string(log_file_dir)) {
        return ERROR_CODE_ILLEGAL_ARGUMENT;
    }
    int appendSeparate = 0;
    if (log_file_dir[strlen(log_file_dir)] != '/') {
        appendSeparate = 1;
    }
    size_t path_string_length = strlen(log_file_dir) +
                                (appendSeparate ? strlen("/") : 0) +
                                strlen(NLOGGER_CACHE_DIR) +
                                strlen("\0");
    *result_dir = malloc(path_string_length);
    if (*result_dir == NULL) {
        return ERROR_CODE_MALLOC_LOG_FILE_DIR_STRING;
    }
    memset(*result_dir, 0, path_string_length);
    memcpy(*result_dir, log_file_dir, strlen(log_file_dir));
    if (appendSeparate) {
        strcat(*result_dir, "/");
    }
    LOGD("init", "create log file dir >>> %s ", *result_dir);
    return makedir_nlogger(*result_dir);
}


/**
 * 初始化路径参数，并且创建文件
 *
 * @param log_file_path 日志文件的路径
 * @param cache_file_path 缓存文件的路径
 * @param encrypt_key 加密aes_key
 * @param encrypt_iv 加密偏移量
 *
 * @return error_code
 */
int init_nlogger(const char *log_file_dir, const char *cache_file_dir, const char *encrypt_key,
                 const char *encrypt_iv) {
    if (is_empty_string(log_file_dir) ||
        is_empty_string(cache_file_dir) ||
        is_empty_string(encrypt_key) ||
        is_empty_string(encrypt_iv)) {
        return ERROR_CODE_ILLEGAL_ARGUMENT;
    }
    //避免重复的去初始化
    if (g_nlogger != NULL && g_nlogger->state == NLOGGER_STATE_INIT) {
        return ERROR_CODE_OK;
    }

    if (g_nlogger != NULL) {
        //是不是需要加上对状态对判断
        free(g_nlogger);
    }
    g_nlogger = malloc(sizeof(nlogger));
    if (g_nlogger == NULL) {
        return ERROR_CODE_MALLOC_NLOGGER_STRUCT;
    }
    memset(g_nlogger, 0, sizeof(nlogger));
    //默认先把状态设置成error，稍后初始化成功以后再把状态设置到init
    g_nlogger->state = NLOGGER_STATE_ERROR;

    //todo 检查是否有原来的 结构体存在 考虑释放原来的结构体
    //创建mmap缓存文件的目录
    char *final_mmap_cache_path;
    _create_cache_file_path(cache_file_dir, &final_mmap_cache_path);
    LOGD("init", "finish create log cache path>>>> %s ", final_mmap_cache_path);
    g_nlogger->cache.p_file_path = final_mmap_cache_path;

    //创建日志文件的存放目录
    char *final_log_file_dir;
    _create_log_file_dir(log_file_dir, &final_log_file_dir);
    LOGD("init", "finish create log file dir >>> %s ", final_log_file_dir)
    g_nlogger->log.p_dir = final_log_file_dir;

    //配置加密相关的参数
    char *temp_encrypt_key = malloc(sizeof(char) * 16);
    memcpy(temp_encrypt_key, encrypt_key, sizeof(char) * 16);
    g_nlogger->data_handler.p_encrypt_key = temp_encrypt_key;
    char *temp_encrypt_iv = malloc(sizeof(char) * 16);
    memcpy(temp_encrypt_iv, encrypt_iv, sizeof(char) * 16);
    g_nlogger->data_handler.p_encrypt_iv = temp_encrypt_iv;

    //初始化缓存文件，首先采用mmap缓存，失败则用内存缓存
    int init_cache_result = init_cache_nlogger(g_nlogger->cache.p_file_path, &g_nlogger->cache.p_buffer);

    if (init_cache_result == NLOGGER_INIT_CACHE_FAILED) {
        g_nlogger->state = NLOGGER_STATE_ERROR;
    } else {
        //标记当前初始化阶段成功，避免重复初始化
        g_nlogger->state            = NLOGGER_STATE_INIT;
        g_nlogger->cache.cache_mode = init_cache_result;
    }

    LOGD("init", "init cache result >>> %d.", init_cache_result);

    //todo try flush cache log
    return ERROR_CODE_OK;
}

/**
 * 尝试在日志文件不存的时候创建日志文件，如果日志文件存在的话，则关闭之前的日志文件并且flush，
 *
 */
int _open_log_file(struct nlogger_log_struct *log) {
    if (!is_file_exist_nlogger(log->p_dir)) {
        makedir_nlogger(log->p_dir);
    }
    FILE *log_file = fopen(log->p_path, "ab+");
    if (log_file != NULL) {
        log->p_file = log_file;
        fseek(log_file, 0, SEEK_END);
        log->file_length = ftell(log_file);
        LOGD("check", "open log file %s  success.", log->p_path);
        log->state = NLOGGER_LOG_STATE_OPEN;
    } else {
        LOGW("check", "open log file %s failed.", log->p_path);
        log->state = NLOGGER_LOG_STATE_CLOSE;
        return ERROR_CODE_OPEN_LOG_FILE_FAILED;
    }
    return ERROR_CODE_OK;
}

int _set_log_file_name(struct nlogger_log_struct *log, const char *log_file_name) {
    size_t file_name_size = strlen(log_file_name);
    size_t file_dir_size  = strlen(log->p_dir);
    size_t end_tag_size   = 1;
    LOGD("check", "log_file_name >>> %s .", log->p_dir);

    LOGD("check", "log_file_name >>> %s .", log_file_name);

    LOGD("check", "file_name_size >>> %zd , file_dir_size >>> %zd , final_file_name >>> %zd.", file_name_size,
         file_dir_size, file_name_size + end_tag_size);

    log->p_name = malloc(file_name_size + end_tag_size);
    if (log->p_name != NULL) {
        memset(log->p_name, 0, file_name_size + end_tag_size);
        memcpy(log->p_name, log_file_name, file_name_size);
        LOGD("check", "### size of log->p_name >>> %zd .", strlen(log->p_name));

        LOGD("check", "### log->p_name >>> %s .", log->p_name);
    } else {
        return ERROR_CODE_MALLOC_LOG_FILE_NAME_STRING;
    }

    log->p_path = malloc(file_dir_size + file_name_size + end_tag_size);
    if (log->p_path != NULL) {
        memset(log->p_path, 0, file_dir_size + file_name_size + end_tag_size);
        memcpy(log->p_path, log->p_dir, file_dir_size);
        memcpy(log->p_path + file_dir_size, log->p_name, file_name_size);
    } else {
        return ERROR_CODE_MALLOC_LOG_FILE_PATH_STRING;
    }
    LOGD("check", "_config_log_file_name finish. log file name >>> %s, log file path >>> %s.", log->p_name,
         log->p_path);
    return ERROR_CODE_OK;
}

/**
 * 映射文件名到mmap文件中
 * 如果将文件名写入到mmap cache文件中失败，则采取内存映射（mmap的优势已经没有了，如果重新打开mmap中的内容不知道恢复到哪里去）
 */
int _map_log_name_to_mmap_cache(struct nlogger_cache_struct *cache, const char *log_file_name) {
    int result = ERROR_CODE_MAP_LOG_NAME_TO_CACHE_FAILED;
    if (cache->cache_mode == NLOGGER_MMAP_CACHE_MODE) {
        cJSON            *root        = cJSON_CreateObject();
        json_map_nlogger *map         = create_json_map_nlogger();
        char             *result_json = NULL;
        if (root != NULL) {
            if (map != NULL) {
                add_item_number_nlogger(map, NLOGGER_MMAP_CACHE_HEADER_KEY_VERSION, NLOGGER_VERSION);
                add_item_number_nlogger(map, NLOGGER_MMAP_CACHE_HEADER_KEY_DATE, system_current_time_millis_nlogger());
                add_item_string_nlogger(map, NLOGGER_MMAP_CACHE_HEADER_KEY_FILE, log_file_name);
                inflate_json_by_map_nlogger(root, map);
                result_json = cJSON_PrintUnformatted(root);
            }

            if (result_json != NULL) {
                LOGD("mmap_header", "build json finished. header >>> %s ", result_json);
                size_t content_length = strlen(result_json) + 1;
                char   *mmap_buffer   = cache->p_buffer;
                *mmap_buffer = NLOGGER_MMAP_CACHE_HEADER_HEAD_TAG;
                mmap_buffer++;
                *mmap_buffer = content_length;
                mmap_buffer++;
                *mmap_buffer = content_length >> 8;
                mmap_buffer++;
                memcpy(mmap_buffer, result_json, content_length);
                mmap_buffer = mmap_buffer + content_length;
                *mmap_buffer = NLOGGER_MMAP_CACHE_HEADER_TAIL_TAG;
                mmap_buffer++;
                cache->p_length = mmap_buffer;
                //这里是否有必要释放内存
                free(result_json);
                result = ERROR_CODE_OK;
            }
            cJSON_Delete(root);
        }
        if (map != NULL) {
            delete_json_map_nlogger(map);
        }
    }
    return result;
}

/**
 * 检查日志文件的状态，如果当次写入和上次的不一致则需要关闭原来的文件，创建新的文件流
 */
int _check_log_file_healthy(struct nlogger_log_struct *log, const char *log_file_name) {
    LOGD("check", "#### start check log file !!!");
//    这一步放在外面来判断，也就是write的地方
//    if (g_nlogger == NULL || g_nlogger->state == NLOGGER_STATE_ERROR) {
//        return ERROR_CODE_NEED_INIT_NLOGGER_BEFORE_ACTION;
//    }
    if (is_empty_string(log_file_name)) {
        return ERROR_CODE_ILLEGAL_ARGUMENT;
    }

    //检查之前保存的日志文件名
    if (is_empty_string(log->p_name)) {
        //第一次创建，之前没有使用过，这个时候应该在init的时候flush过一次，因此不再需要flush
//        size_t file_name_size = strlen(log_file_name) + 1;
//        log->p_name = malloc(file_name_size);
//        memcpy(log->p_name, log_file_name, file_name_size);

        int result_code = _set_log_file_name(log, log_file_name);
        //检查创建是否成功，如果path 或者 name创建失败则直接返回；
        if (result_code != ERROR_CODE_OK) {
            LOGW("check", "first malloc file string on error >>> %d .", result_code);
            return result_code;
        }
        LOGD("check", "first config log file success log file path >>> %s.", log->p_path);
    } else if (strcmp(log->p_name, log_file_name) != 0) {
        //非第一次创建，之前已经有一个已经打开的日志文件，并且日志文件是打开状态
        if (log->state == NLOGGER_LOG_STATE_OPEN &&
            log->p_file != NULL) {
            //如果当前是打开状态，则尝试关闭，这里是否需要flush
            //todo try flush cache log
            //释放资源
            fclose(log->p_file);
            free(log->p_name);
            free(log->p_path);
            log->state = NLOGGER_LOG_STATE_CLOSE;
        }
        //如果日志文件的目录都有问题说明需要重新初始化
        if (log->p_dir == NULL) {
            LOGW("check", "log file dir is empty on check.");
            return ERROR_CODE_NEED_INIT_NLOGGER_BEFORE_ACTION;
        }
        int result_code = _set_log_file_name(log, log_file_name);
        //检查创建是否成功，如果path 或者 name创建失败则直接返回；
        if (result_code != ERROR_CODE_OK) {
            LOGW("check", "malloc file string on error >>> %d .", result_code);
            return result_code;
        }
        LOGD("check", "close last once and new create log file, config success %s .", log->p_path);
    } else {
        //表示当前写入的日志文件和上次写入的日志文件是一致的
        if (!is_empty_string(log->p_path) &&
            is_file_exist_nlogger(log->p_path) &&
            log->p_file != NULL &&
            log->state == NLOGGER_LOG_STATE_OPEN) {
            //说明当前已经打开的日志文件和之前的是一致的
            //并且日志文件已经是打开状态，这个时候什么事都不用做
            LOGD("check", "same of last once log file config, success %s .", log->p_path);
            return ERROR_CODE_OK;
        } else {
            int temp_error_code = 0;
            if (log->p_name != NULL) {
                temp_error_code |= 0x0001;
                free(log->p_name);
            }
            if (log->p_path != NULL) {
                temp_error_code |= 0x0010;
                free(log->p_path);
            }
            if (log->p_file != NULL) {
                temp_error_code |= 0x0100;
                fclose(log->p_file);
            }
            if (log->state == NLOGGER_LOG_STATE_CLOSE) {
                temp_error_code |= 0x1000;
            }
            log->state = NLOGGER_LOG_STATE_CLOSE;
            LOGE("check", "on error log same as last once, but state invalid. state >>> %d", temp_error_code);
            //初始化状态重新调用一次，表示当前日志文件状态有问题重新配置日志文件
            _check_log_file_healthy(log, log_file_name);
        }
    }

    //如果是关闭状态，则检查是否需要创建目录，如果是打开状态则肯定已经做了日志文件长度这些状态的记录
    if (log->state == NLOGGER_LOG_STATE_CLOSE) {
        int result = _open_log_file(log);
        if (result == ERROR_CODE_OK) {
            //表示第一次打开日志文件
            return ERROR_CODE_NEW_LOG_FILE_OPENED;
        }
        //打开文件失败的情况
        return result;
    }

    return ERROR_CODE_OK;
}

int _init_cache_on_new_log_file_opened(const char *log_file_name, struct nlogger_cache_struct *cache) {
    if (cache->cache_mode == NLOGGER_MMAP_CACHE_MODE) {
        //写入日志头，指定当前mmap文件映射的log file name
        int map_result = _map_log_name_to_mmap_cache(cache, log_file_name);
        //写入失败则切换成Memory缓存模式
        //（不能建立关系就失去了意外中断导致日志丢失的优势，这和内存缓存没有区别）
        if (map_result != ERROR_CODE_OK) {
            if (cache->p_buffer != NULL) {
                munmap(cache->p_buffer, NLOGGER_MMAP_CACHE_SIZE);
            }
            //路径传空，强制Memory模式缓存
            if (NLOGGER_INIT_CACHE_FAILED == init_cache_nlogger(NULL, &cache->p_buffer)) {
                return ERROR_CODE_INIT_CACHE_FAILED;
            }
        }
    }
    //mmap模式会在完成映射关系（写入file name）后指定代表content长度指针的位置
    //memory模式需要手动指定代表长度的指针，也就是内存缓存的开头地址
    if (cache->cache_mode == NLOGGER_MEMORY_CACHE_MODE) {
        cache->p_length = cache->p_buffer;
    }

    cache->p_next_write   = cache->p_length + NLOGGER_CONTENT_LENGTH_BYTE_SIZE;
    cache->length         = 0;
    cache->content_length = 0;

    return ERROR_CODE_OK;
}

int _check_cache_healty(struct nlogger_cache_struct *cache) {
    int result = ERROR_CODE_OK;
    if (cache->cache_mode == NLOGGER_MMAP_CACHE_MODE &&
        !is_file_exist_nlogger(cache->p_file_path)) {
        //mmap缓存文件不见了，这个时候主动切换成Memory缓存
        if (cache->p_buffer != NULL) {
            munmap(cache->p_buffer, NLOGGER_MMAP_CACHE_SIZE);
        }
        //路径传空，强制Memory缓存
        if (NLOGGER_INIT_CACHE_FAILED == init_cache_nlogger(NULL, &cache->p_buffer)) {
            return ERROR_CODE_INIT_CACHE_FAILED;
        }
        cache->p_length       = cache->p_buffer;
        cache->p_next_write   = cache->p_length + NLOGGER_CONTENT_LENGTH_BYTE_SIZE;
        cache->length         = 0;
        cache->content_length = 0;
        result = ERROR_CODE_CHANGE_CACHE_MODE_TO_MEMORY;
    }

    //最后检查一下缓存指针是否存在，如果不存在则直接报错返回
    if (cache->p_buffer == NULL) {
        result = ERROR_CODE_INIT_CACHE_FAILED;
    }

    return result;
}

/**
 * 申请内存并且组装一个json数据
 *
 * @param cache_type 缓存的类型 1:MMAP 2:MEMORY
 * @param flag 日志类型
 * @param log_content 日志内容
 * @param local_time 日志生成的时间
 * @param thread_name 线程的名称
 * @param thread_id 线程id
 * @param result_json_data 用于返回数据的指针
 *
 * @return 数据的长度
 */
size_t _malloc_and_build_json_data(int cache_type, int flag, char *log_content, long long local_time,
                                   char *thread_name, long long thread_id, char **result_json_data) {
    size_t result_size = 0;

    cJSON            *root        = cJSON_CreateObject();
    json_map_nlogger *map         = create_json_map_nlogger();
    char             *result_json = NULL;
    if (root != NULL) {
        if (map != NULL) {
            add_item_number_nlogger(map, NLOGGER_PROTOCOL_KEY_CACHE_TYPE, cache_type);
            add_item_number_nlogger(map, NLOGGER_PROTOCOL_KEY_FLAG, flag);
            add_item_string_nlogger(map, NLOGGER_PROTOCOL_KEY_CONTENT, log_content);
            add_item_number_nlogger(map, NLOGGER_PROTOCOL_KEY_LOCAL_TIME, local_time);
            add_item_string_nlogger(map, NLOGGER_PROTOCOL_KEY_THREAD_NAME, thread_name);
            add_item_number_nlogger(map, NLOGGER_PROTOCOL_KEY_THREAD_ID, thread_id);
            inflate_json_by_map_nlogger(root, map);
            result_json = cJSON_PrintUnformatted(root);
        }

        if (result_json != NULL) {
            size_t content_length = strlen(result_json) + 1; //加上末尾的换行符\n
            char   *log_data      = malloc(content_length);
            memset(log_data, 0, content_length);
            memcpy(log_data, result_json, strlen(result_json));
            *result_json_data = log_data;
            //这里是否有必要释放内存
            free(result_json);
            result_size = content_length;
        }
        cJSON_Delete(root);
    }
    if (map != NULL) {
        delete_json_map_nlogger(map);
    }

    return result_size;
}

int write_nlogger(const char *log_file_name, int flag, char *log_content, long long local_time, char *thread_name,
                  long long thread_id, int is_main) {
    if (g_nlogger == NULL || g_nlogger->state == NLOGGER_STATE_ERROR) {
        return ERROR_CODE_NEED_INIT_NLOGGER_BEFORE_ACTION;
    }

    //step1 检查日志文件
    int result = _check_log_file_healthy(&g_nlogger->log, log_file_name);

    if (result <= 0) {
        return result;
    }

    //第一次创建或者打开日志文件，需要重新指定当前mmap缓存指向的日志文件，方便下次启动将缓存flush到指定的日志文件中
    if (result == ERROR_CODE_NEW_LOG_FILE_OPENED) {
        int init_cache_result = _init_cache_on_new_log_file_opened(log_file_name, &g_nlogger->cache);
        if (init_cache_result != ERROR_CODE_OK) {
            return init_cache_result;
        }
    }

    //step2 检查缓存状态
    int check_cache_result = _check_cache_healty(&g_nlogger->cache);
    if (check_cache_result <= 0) {
        return check_cache_result;
    }

    //mmap缓存有问题，避免remain中缓存数据影响新插入的日志，先把它清空
    if (check_cache_result == ERROR_CODE_CHANGE_CACHE_MODE_TO_MEMORY) {
        g_nlogger->data_handler.remain_data_length = 0;
    }

    //test debug
    _print_current_nlogger();

    //step3 组装日志
    char   *result_json_log;
    size_t data_size       = _malloc_and_build_json_data(g_nlogger->cache.cache_mode, flag, log_content, local_time,
                                                         thread_name, thread_id, &result_json_log);
    LOGD("write", "_malloc_and_build_json_data data size >>> %zd ", data_size);
    LOGD("write", "_malloc_and_build_json_data resutl data >>> %s ", result_json_log)

    return ERROR_CODE_OK;
}

