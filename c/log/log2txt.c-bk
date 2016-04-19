#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include "program_run_log.h"
#include "constant.h"
int main()
{
        int ret;
        char log_path[] = "./";
        char name_no_suffix[FILE_NAME_LEN] = {0};
        char log_time[TIME_STR_LEN] = {0};
        const char* pFormatLog = "%Y-%m-%d";
        int name_len;
        DIR *dir_log;
        struct dirent *entry;
        time_t time_log;

        ret = RET_SUCCESS;
        dir_log = opendir(log_path);

        while ((entry = readdir (dir_log)) != NULL) {

                name_len = strlen(entry->d_name);

                printf("%s(): Line : %d  %s %d\n", \
                                __FUNCTION__, __LINE__,entry->d_name,name_len);
                if ( (name_len > FILE_NAME_LEN) || \
                                (NULL == strstr(entry->d_name,".log") ) ) {
                        ret = -1;
                        continue;
                }
                strncpy(name_no_suffix,entry->d_name,name_len - strlen(".log\0"));
                strptime(log_time,pFormatLog,&time_log);
                strftime(log_time,TIME_STR_LEN,pFormatLog,&time_log);
                convert_log2txt(entry->d_name);
        }
        return ret;
}



