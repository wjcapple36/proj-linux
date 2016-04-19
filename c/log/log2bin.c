#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include "program_run_log.h"
#include "constant.h"
int main()
{
        int ret, i,count;
        _tagLogMsg msg;
        count = 6000;
        ret = RET_SUCCESS;
        strncpy((char *)(&msg.function), __FUNCTION__,NUM_CHAR_LOG_FUN);
        msg.line = __LINE__;
        for(i = 0; i < count;i++)
        {
                snprintf((char *)(&msg.log_msg), NUM_CHAR_LOG_MSG,\
                                "%d  hello word ，你好，测试，Test 123！",i);
                outoutLog(msg);
                system("sync");

        }
        return ret;
}



