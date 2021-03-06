//
// Created by fgd on 2019/5/18.
//

#include <string.h>
#include "nlogger_log_file_handler.h"
#include "../utils/nlogger_android_log.h"
#include "../nlogger_error_code.h"
#include "../utils/nlogger_utils.h"
#include "../utils/nlogger_file_utils.h"
#include "../nlogger_constants.h"


/**
 * 尝试在日志文件不存的时候创建日志文件，如果日志文件存在的话，则关闭之前的日志文件并且flush，
 *
 */
int open_log_file(struct nlogger_log_struct *log) {
    if (!is_file_exist_nlogger(log->p_dir)) {
        make_dir_nlogger(log->p_dir);
    }
    FILE *log_file = fopen(log->p_path, "ab+");
    if (log_file != NULL) {
        log->p_file = log_file;
        fseek(log_file, 0, SEEK_END);
        log->current_file_size = ftell(log_file);
        LOGD("check", "open log file %s  success.", log->p_path);
        log->state = NLOGGER_LOG_STATE_OPEN;
    } else {
        LOGW("check", "open log file %s failed.", log->p_path);
        log->state = NLOGGER_LOG_STATE_CLOSE;
        return ERROR_CODE_OPEN_LOG_FILE_FAILED;
    }
    return ERROR_CODE_OK;
}

/**
 * 释放日志文件资源
 *
 * @param log
 * @return
 */
int release_log_file(struct nlogger_log_struct *log) {
    //释放资源
    if (log->state == NLOGGER_LOG_STATE_OPEN) {
        if (log->p_file != NULL) {
            fclose(log->p_file);
            log->p_file = NULL;
        }
        free(log->p_name);
        log->p_name = NULL;
        free(log->p_path);
        log->p_path = NULL;
        log->state  = NLOGGER_LOG_STATE_CLOSE;
    }
    return ERROR_CODE_OK;
}

/**
 * 为文件名malloc内存保存并且更新日志文件的path
 *
 * @param log
 * @param log_file_name
 * @return
 */
int _set_log_file_name(struct nlogger_log_struct *log, const char *log_file_name) {
    if (log->p_dir == NULL) {
        return ERROR_CODE_NEED_INIT_NLOGGER_BEFORE_ACTION;
    }

    size_t file_name_size = strlen(log_file_name);
    size_t file_dir_size  = strlen(log->p_dir);
    size_t end_tag_size   = 1;
    LOGD("check", "log_file_name >>> %s .", log->p_dir);

    LOGD("check", "log_file_name >>> %s .", log_file_name);

    LOGD("check", "file_name_size >>> %zd , file_dir_size >>> %zd , final_file_name >>> %zd.", file_name_size,
         file_dir_size, file_name_size + end_tag_size);
    //设置日志文件的文件名
    log->p_name = malloc(file_name_size + end_tag_size);
    if (log->p_name != NULL) {
        memset(log->p_name, 0, file_name_size + end_tag_size);
        memcpy(log->p_name, log_file_name, file_name_size);
        LOGD("check", "### size of log->p_name >>> %zd .", strlen(log->p_name));

        LOGD("check", "### log->p_name >>> %s .", log->p_name);
    } else {
        return ERROR_CODE_MALLOC_LOG_FILE_NAME_STRING;
    }
    //设置日志文件的完整路径(包括文件名)
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
 * 检查日志文件的状态
 *
 * 如果参数文件名对应的Log File没有打开，则直接open file
 * 如果参数文件名和当前文件名不一致则需要关闭原来的文件，从新open file创建新的文件流
 *
 * 最后会统一再检查一次日志文件是否已经超过大小
 *
 * @return 小于0表示存在异常，1 代表文件存在，并且是打开状态，2 代表文件不存在但是重新创建了一个并且打开了文件流
 */
int check_log_file_healthy(struct nlogger_log_struct *log, const char *log_file_name) {
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
        /*
         * 第一次创建，之前没有使用过，
         */
        int result_code = _set_log_file_name(log, log_file_name);
        //检查创建是否成功，如果path 或者 name创建失败则直接返回；
        if (result_code != ERROR_CODE_OK) {
            LOGW("check", "first malloc file string on error >>> %d .", result_code);
            return result_code;
        }
        LOGD("check", "first config log file success log file path >>> %s.", log->p_path);
    } else if (strcmp(log->p_name, log_file_name) != 0) {
        /* 非第一次创建，之前已经有一个已经打开的日志文件，并且日志文件是打开状态 */
        //把这块逻辑放到外部去实现
//        if (log->state == NLOGGER_LOG_STATE_OPEN && log->p_file != NULL) {
//            // todo try flush cache log 如果当前是打开状态，则尝试关闭，这里是否需要flush，切换日志文件时是否有缓存日志在？
//            LOGW("check", "do flush on new log file open.")
//
//
//            //释放资源
//            fclose(log->p_file);
//            free(log->p_name);
//            free(log->p_path);
//            log->state = NLOGGER_LOG_STATE_CLOSE;
//        }
//        //如果日志文件的目录都有问题说明需要重新初始化
//        if (log->p_dir == NULL) {
//            LOGW("check", "log file dir is empty on check.");
//            return ERROR_CODE_NEED_INIT_NLOGGER_BEFORE_ACTION;
//        }
//        check_log_file_healthy(log, log_file_name);
//        int result_code = _set_log_file_name(log, log_file_name);
//        //检查创建是否成功，如果path字符 或者 name字符创建失败则直接返回；
//        if (result_code != ERROR_CODE_OK) {
//            LOGW("check", "malloc file string on error >>> %d .", result_code);
//            return result_code;
//        }
//        LOGD("check", "close last once and new create log file, config success %s .", log->p_path);
        LOGE("write_nlogger", "_check_log_file_healthy already an exist disfferent log file (%s)", log->p_path)
        return ERROR_CODE_ALREADY_AN_DIFFERENT_LOG_FILE_ON_OPEN;
    } else {
        /*表示当前写入的日志文件和上次写入的日志文件是一致的*/
        if (!is_empty_string(log->p_path) &&
            is_file_exist_nlogger(log->p_path) &&
            log->p_file != NULL &&
            log->state == NLOGGER_LOG_STATE_OPEN) {
            //说明当前已经打开的日志文件和之前的是一致的
            //并且日志文件已经是打开状态，这个时候什么事都不用做

            //检查是否超过日志最大大小
            if (log->current_file_size >= log->max_file_size) {
                LOGE("check", "log file size large than max size.");
                return ERROR_CODE_LOG_FILE_ON_MAX_SIZE;

            }
            LOGD("check", "same of last once log file config, success %s .", log->p_path);

            return ERROR_CODE_OK;
        } else {
            // 异常情况，重新进行初始化
            int temp_error_code = 0;
            if (log->p_name != NULL) {
                temp_error_code |= 0x0001;
            }
            if (log->p_path != NULL) {
                temp_error_code |= 0x0010;
            }
            if (log->p_file != NULL) {
                temp_error_code |= 0x0100;
            }
            if (log->state == NLOGGER_LOG_STATE_CLOSE) {
                temp_error_code |= 0x1000;
            }
            release_log_file(log);
            LOGE("check", "on error log same as last once, but state invalid. state >>> %d", temp_error_code);
            //初始化状态重新调用一次，表示当前日志文件状态有问题重新配置日志文件
            check_log_file_healthy(log, log_file_name);
        }
    }

    //如果是关闭状态，则检查是否需要创建日志文件的目录并且记录日志文件长度这些状态
    if (log->state == NLOGGER_LOG_STATE_CLOSE) {
        int result = open_log_file(log);
        if (result == ERROR_CODE_OK) {
            //检查是否超过日志最大大小
            if (log->current_file_size >= log->max_file_size) {
                LOGE("check", "log file size large than max size.");
                return ERROR_CODE_LOG_FILE_ON_MAX_SIZE;
            }
            //表示第一次打开日志文件
            return ERROR_CODE_NEW_LOG_FILE_OPENED;
        }
        //打开文件失败的情况
        return result;
    }

    return ERROR_CODE_OK;
}

/**
 * 检查当前文件名是否存在
 *
 * @param log
 * @return
 */
int is_log_file_name_valid(struct nlogger_log_struct *log) {
    if (!is_empty_string(log->p_name) && !is_empty_string(log->p_dir)) {
        return ERROR_CODE_OK;
    }
    return ERROR_CODE_LOG_FILE_NAME_NOT_CONFIG;
}

/**
 * 创建日志文件存放的目录（不包含日志文件名）
 */
int _create_log_file_dir(const char *log_file_dir, char **result_dir) {
    if (is_empty_string(log_file_dir)) {
        return ERROR_CODE_ILLEGAL_ARGUMENT;
    }
    //判断末尾有没有反斜杠
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
    return make_dir_nlogger(*result_dir);
}

/**
 * 为日志文件路径申请内存并且保存
 *
 * @param log
 * @param dir
 * @return
 */
int init_log_file_config(struct nlogger_log_struct *log, const char *dir, const long long max_file_size) {
    char *final_log_file_dir;
    if (max_file_size <= 0) {
        return ERROR_CODE_ILLEGAL_ARGUMENT;
        LOGE("init", "invalid max log file size >>> %llu ", max_file_size)
    }
    log->max_file_size = max_file_size;
    //malloc是否应该放在外层
    int result = _create_log_file_dir(dir, &final_log_file_dir);
    if (result != ERROR_CODE_OK) {
        return result;
    }
    LOGD("init", "finish create log file dir >>> %s ", final_log_file_dir)
    log->p_dir = final_log_file_dir;
    return ERROR_CODE_OK;
}

/**
 * 获取当前的文件名
 *
 * @param log
 * @return
 */
char *current_log_file_name(struct nlogger_log_struct *log) {
    return log->p_name;
}

/**
 * 将日缓存写入到日志文件中
 *
 */
int flush_cache_to_log_file(struct nlogger_log_struct *log, char *cache, size_t cache_length) {
    int result = ERROR_CODE_OK;
    //重新检查一下需要写入到文件是否健康
    if (check_log_file_healthy(log, log->p_name) < 0) {
        return ERROR_CODE_BAD_LOG_FILE_STATE_ON_FLUSH;
    }
    fwrite(cache, sizeof(char), cache_length, log->p_file);
    fflush(log->p_file);
//    fclose(log->p_file);
    log->current_file_size += cache_length;

    return result;
}