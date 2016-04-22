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
        char log_msg[NUM_CHAR_LOG_MSG] = {0};
        count = 5000;
        ret = RET_SUCCESS;
        strncpy((char *)(&msg.function), __FUNCTION__,NUM_CHAR_LOG_FUN);
        msg.line = __LINE__;
        msg.lev = LOG_LEV_USUAL_MSG;
        init_log_dir();
        clear_expiry_log();
        for(i = 0; i < count;i++)
        {
                snprintf(log_msg,NUM_CHAR_LOG_MSG,"%d hello world !测试，Test 123 ！",i);
                LOGW(__FUNCTION__,__LINE__, LOG_LEV_USUAL_MSG,log_msg);


        }
        system("sync");
        return ret;
}



