#include "log_maker.h"
#include <stdio.h>
#include <unistd.h>

#define LOG_USE_LOGMAKER_MSG(msg, ...) LOG_THIS("LOG_MAKER", msg, __VA_ARGS__);

void function_one(void)
{
    LOG_THIS(LOG_WARNING,"LOG_THIS = %d, %d, %d,%d, %d, %d, %d, %d, %d, %d\n",
             1,2,3,4,5,6,7,8,9,10);
    LOG_ERROR_MSG("this is a LOG_ERROR_MSG = %d\n",1);
    LOG_WARNING_MSG("this is a LOG_WARNING_MSG = %d\n",2);
    LOG_NOTICE_MSG("this is a LOG_NOTICE_MSG = %d\n",3);
    LOG_INFO_MSG("this is a LOG_INFO_MSG = %d\n",4);
    LOG_DEBUG_MSG("this is a LOG_DEBUG_MSG = %d\n",5);
    return;
}

void function_two(void)
{
    LOG_THIS(LOG_NOTICE,"variable two = %d, %d, %d,%d, %d, %d, %d, %d, %d, %d "
                  "%d, %d, %d,%d, %d, %d, %d, %d, %d, %d\n", 
                  1,2,3,4,5,6,7,8,9,10,
                  1,2,3,4,5,6,7,8,9,10);
    LOG_USE_LOGMAKER_MSG("this is a LOG_USE_LOGMAKER_MSG = %s\n","LOG_MAKER");
    return;
}

int main(int argc, char *argv[])
{
    // set_log_level(LOG_DEBUG);
    set_log_dir("/tmp/foo/bar/","/home/g/cor-logs/incubator-corinthia/experiments/logger/");
    log_init();

    function_one();
    function_two();

    close_log();
    return 0;
}
