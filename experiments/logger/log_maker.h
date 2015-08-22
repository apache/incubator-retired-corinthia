#ifndef LOG_MAKER_H
#define LOG_MAKER_H

#if _MSC_VER
#define snprintf _snprintf
#endif

#define LOG_ERR         "ERROR"
#define LOG_WARNING     "WARN"
#define LOG_NOTICE      "NOTICE"
#define LOG_INFO        "INFO"
#define LOG_DEBUG       "DEBUG"


// TODO
void set_log_output_function(char* func);

void log_init(void);
void set_log_dir(const char*, const char*);
void set_log_level(int level);
void close_log(void);

void log_msg_prefix(char* level, char* filename, int linenum, const char* function);
void log_msg(char* level, char *msg, ...);

#define filename_len 1024
char log_filename[filename_len];
int log_file_fd;

static int log_level_initialised = 0;

char logging_dir[filename_len];
char log_symlink[filename_len];

#define LOG_THIS(level, msg, ...) do {                                \
        log_msg_prefix(level, __FILE__, __LINE__, __FUNCTION__);      \
        log_msg(level, msg, __VA_ARGS__);                             \
    } while (0)


#define LOG_ERROR_MSG(msg, ...) LOG_THIS(LOG_ERR, msg, __VA_ARGS__);
#define LOG_WARNING_MSG(msg, ...) LOG_THIS(LOG_WARNING, msg, __VA_ARGS__);
#define LOG_NOTICE_MSG(msg, ...) LOG_THIS(LOG_NOTICE, msg, __VA_ARGS__);
#define LOG_INFO_MSG(msg, ...) LOG_THIS(LOG_INFO, msg, __VA_ARGS__);
#define LOG_DEBUG_MSG(msg, ...) LOG_THIS(LOG_DEBUG, msg, __VA_ARGS__);

#endif /* LOG_MAKER_H_H */
