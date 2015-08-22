#define _GNU_SOURCE 1

#include "log_maker.h"
#include <assert.h>
#include <error.h>
#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int get_time_string(char** time_buf, int time_buf_len) 
{
    time_t t;
    struct tm *tm_tmp;

    memset(*time_buf, 0, time_buf_len);

    t = time(NULL);
    tm_tmp = localtime(&t);
    if (NULL == tm_tmp) {
        // FIXME:  there is 'utassrt' but I'm not sure how to get ahold of it right now.
        perror("localtime error");
        abort();
    }

    strftime(*time_buf, time_buf_len, "%Y%m%d-%H%M%S", tm_tmp);

    return strlen(*time_buf);
}

void set_log_output_function(char* func)
{
    // TODO
}

/* The format of file name is 
     <binary_name>.<hostname>.<date_string> 
   Note: Open question: is it worth also adding username to this
         string? 
*/


void log_init(void) 
{
    if (log_level_initialised)
        return;

    int hostname_len = 80;
    char hostname_buf[hostname_len];
    gethostname((char*)hostname_buf, hostname_len);

    char* progname = basename(getenv("_"));

    int time_buf_len = 16;
    char *time_buf = malloc(time_buf_len);

    get_time_string(&time_buf, time_buf_len);
    
    /* FIXME:
 
      Take DFFormatString and copy it into your code, adapting it as
      necessary.

    */
    snprintf(log_filename, filename_len, "%s%s.%s.%s",
             logging_dir,progname,hostname_buf,time_buf);

    log_file_fd = open(log_filename, 
                  O_CREAT | O_TRUNC | O_WRONLY, 
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (log_file_fd == -1) {
        fprintf(stderr, "FATAL: couldn't open file descriptor\n");
        abort();
    }

    symlink(log_filename, log_symlink);

    free(time_buf);
    log_level_initialised = 1;
}


void set_log_dir(const char* dir, const char *sdir) 
{
    if (log_level_initialised) {
        fprintf(stderr, "Cannot reset logging level after initialisation\n");
        abort();
    }

    errno = 0;

    DIR* has_dir = opendir(dir);

    int this_error = errno;

    // check the actual log file dir
    if (has_dir) {
        closedir(has_dir);
        snprintf(logging_dir, filename_len, "%s", dir);
    }
    else {
        // FIXME
        perror(strerror(this_error));
        abort();
        return;
    }

    // check the symlink, remove if it exists
    snprintf(log_symlink, filename_len, "%scurrent.log", sdir);

    struct stat statbuf;
    
    if (-1 == lstat(log_symlink, &statbuf)) 
        return;
    else 
        unlink(log_symlink);

    return;
}

void close_log(void) 
{
    close(log_file_fd);
    printf("Logfile created in %s\nSee %s to view it\n", log_filename, log_symlink);
}

void log_msg_prefix(char *level, char* filename, int linenum, const char* function)
{
    if (! log_level_initialised) {
        fprintf(stderr, "ERROR: Trying to log before initialisation is complete.\n");
        exit(1);
    }
    dprintf(log_file_fd, "%-15s %s:%d %s() ", level, filename, linenum, function);
}


void log_msg(char *level, char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    assert(log_file_fd > -1);
    vdprintf(log_file_fd, fmt, argp);
    va_end(argp);
 }

