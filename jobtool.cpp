//ms docs here:
// https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-tsch/8a585672-81b8-4b3a-912a-310b01da3518
//some extra info here:
// https://web.archive.org/web/20190629040200/https://forensicswiki.org/wiki/Windows_Job_File_Format

#include <iostream>
#include <stdlib.h>  
#include <cstdio>
#include <string>
#include <objbase.h>
#include <mbstring.h>

using namespace std;


/*
params: 
filename e.g. name.job

idle deadline
Contains a maximum time in minutes to wait for the machine to become idle for Idle Wait minutes.

idle wait
Contains a value in minutes. The machine remains idle for this many minutes before it runs the task.

priority

maximum run time
number of milliseconds the server will wait for the task to complete

app name to execute

*/
int main(int argc, char* argv[])
{
    if (argc > 1) {
        std::cout << "writing hex to file";
        
        FILE* fp;

        if ((fopen_s( &fp, argv[1], "w+b")) == 0) {

            //version and product number - set to win2000 v1
            unsigned char buff[] = { 0x00, 0x05, 0x01, 0x00 };
            fwrite( buff, 1, sizeof(buff), fp);

            //unique id - using win32 api
            GUID g;
            CoCreateGuid(&g);
            fwrite(&g, 1, 16, fp);

            //application name size/length offset, relative from the start of the file - 2 bytes
            //offset in bytes within the .JOB file where the length of the application name is located.
            int name_offset = 70;
            fwrite(&name_offset, 1, 2, fp);

            //trigger offset, relative from the start of the file - 2 bytes
            int trig_offset = 224;
            fwrite(&trig_offset, 1, 2, fp);

            //error retry count (2bytes)
            //number of execute attempts that are attempted for the task if the task fails to start
            //set to zero
            unsigned char error_retry_count[2] = { 0x00, 0x00};
            fwrite(error_retry_count, 1, 2, fp);

            //error retry interval (2bytes)
            //interval, in minutes, between successive retries
            unsigned char error_retry_interval[2] = { 0x00, 0x00 };
            fwrite(error_retry_interval, 1, 2, fp);

            //idle deadline (2B)
            string dparam = argv[2];
            int idle_deadline = std::stoi(dparam);
            fwrite(&idle_deadline, 1, 2, fp);

            //idle wait (2B)
            string wparam = argv[3];
            int idle_wait = std::stoi(wparam);
            fwrite(&idle_wait, 1, 2, fp);

            //priority (4B)
            //was found at 32 (dec) or 0x20 0x00 0x00 0x00
            //doc state that 
            //0x02000000  is	IDLE_PRIORITY_CLASS 
            //others:
            //0x01000000 	HIGH_PRIORITY_CLASS
            //0x04000000 	NORMAL_PRIORITY_CLASS 
            //0x00800000 	REALTIME_PRIORITY_CLASS
            string pparam = argv[4];
            int priority = std::stoi(pparam);
            fwrite(&priority, 1, 4, fp);

            //max runtime (4B) ms allocated to starting task
            string rtime = argv[5];
            int runtime = std::stoi(rtime);
            fwrite(&runtime, 1, 4, fp);

            //exit code (4B)
            unsigned char exitcode[4] = { 0x00, 0x00, 0x00, 0x00 };
            fwrite(exitcode, 1, 4, fp);

            //status (4B)
            //set to zero according to docs, but gives possible values:
            //SCHED_S_TASK_READY
            //Task is not running but is scheduled to run at some time in the future.
            //0x00041300
            //unsigned char status[4] = { 0x00, 0x13, 0x04, 0x00 };
            //important: windows generated job file had status set to 03 13 04 00
            //0x00041303 (task has not yet run) - so this is probably safest, if 0 is unwanted
            unsigned char status[4] = { 0x00, 0x00, 0x00, 0x00 };
            fwrite(status, 1, 4, fp);

            //flags (4B) confused
            //found C0 00 80 21
            unsigned char flags[4] = { 0x00, 0x00, 0x00, 0x00 };
            fwrite(flags, 1, 4, fp);

            //sys time , according to docs: when creating a job, the field values are to be ignored upon receipt
            //seems to be 16B
            unsigned char systime[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            fwrite(systime, 1, 16, fp);

            //running instance count (2B)
            unsigned char rinst[2] = { 0x00, 0x00 };
            fwrite(rinst, 1, 2, fp);

            //-------VARIABLE LENGTH

            //application name to execute
            string anstr = argv[6];
            int applen = strlen(argv[6]) + 1;
            int wchars_num = MultiByteToWideChar(CP_UTF8, 0, anstr.c_str(), -1, NULL, 0);
            wchar_t* appname = new wchar_t[wchars_num];
            MultiByteToWideChar(CP_UTF8, 0, anstr.c_str(), -1, appname, wchars_num);
            //first 2B are length of string
            fwrite(&applen, 1, 2, fp);
            //length of single byte string is *2 to give length for double byte string
            fwrite(appname, 1, (applen*2), fp);

            //parameters - found like this - 2B as 0 indicates empty string (no following param str)
            unsigned char parameters[2] = { 0x00, 0x00 };
            fwrite(parameters, 1, 2, fp);

            //working directory
            string wdstr = argv[7];
            //adding 1 as strlen doesnt include nul term byte in its calculations
            int wdlen = strlen(argv[7])+1;
            int wdchars_num = MultiByteToWideChar(CP_UTF8, 0, wdstr.c_str(), -1, NULL, 0);
            wchar_t* wdir = new wchar_t[wdchars_num];
            MultiByteToWideChar(CP_UTF8, 0, wdstr.c_str(), -1, wdir, wdchars_num);
            //first 2B are length of string
            fwrite(&wdlen, 1, 2, fp);
            //length of single byte string is *2 to give length for double byte string
            fwrite(wdir, 1, (wdlen * 2), fp);

            //author
            //found as : Administrator
            string authstr = argv[8];
            //adding 1 as strlen doesnt include nul term byte in its calculations
            int authlen = strlen(argv[8]) + 1;
            int authchars_num = MultiByteToWideChar(CP_UTF8, 0, authstr.c_str(), -1, NULL, 0);
            wchar_t* auth = new wchar_t[authchars_num];
            MultiByteToWideChar(CP_UTF8, 0, authstr.c_str(), -1, auth, authchars_num);
            //first 2B are length of string
            fwrite(&authlen, 1, 2, fp);
            //length of single byte string is *2 to give length for double byte string
            fwrite(auth, 1, (authlen * 2), fp);

            //comment - found like this - 2B as 0 indicates empty string (no following param str)
            unsigned char comment[2] = { 0x00, 0x00 };
            fwrite(comment, 1, 2, fp);

            //user data - found like this - 2B as 0 indicates empty string (no following param str)
            unsigned char userdata[2] = { 0x00, 0x00 };
            fwrite(userdata, 1, 2, fp);

            //reserved data - found: (but docs say you can just set to zero) 10B
            unsigned char rdata[10] = { 0x08, 0x00, 0x03, 0x13, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00 };
            fwrite(rdata, 1, 10, fp);

            //--------------TRIGGERS

            //number of triggers (2B)
            //just support 1 for now 
            unsigned char trigno[2] = { 0x01, 0x00 };
            fwrite(trigno, 1, 2, fp);

            //trigger size (2B)
            //for now this is constant - unless we add extra options
            unsigned char trigsize[2] = { 0x30, 0x00 };
            fwrite(trigsize, 1, 2, fp);

            //reserved (2B)
            unsigned char reserved[2] = { 0x00, 0x00 };
            fwrite(reserved, 1, 2, fp);


            //-----> update correct position of trigger, at byte 22
            long cpos = ftell(fp);
            //ad two bytes to pos - seems to work
            int cintpos = ftell(fp)+2;
            fseek(fp, 22,0);
            int inject = cintpos;
            fwrite(&inject, 1, 2, fp);
            fseek(fp, cpos, 0);


            //begin year (2B)
            char* byr = argv[9];
            int byear = std::stoi(byr);
            fwrite(&byear, 1, 2, fp);
            //begin month (2B)  SHOULD be in the range of 1 to 12
            char* bm = argv[10];
            int bmonth = std::stoi(bm);
            fwrite(&bmonth, 1, 2, fp);
            //begin day (2B) SHOULD be in the range of 1 to the number of days in the month specified by the Begin Month field
            char* bday = argv[11];
            int beginday = std::stoi(bday);
            fwrite(&beginday, 1, 2, fp);

            //end year,month and day - ignore for now (6B)
            //from docs: These fields are ignored if the TASK_TRIGGER_FLAG_HAS_END_DATE bit is not set in the Flags field
            unsigned char enddate[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            fwrite(enddate, 1, 6, fp);

            //start hour (2B) Start Hour is in the range 0 to 23
            char* shour = argv[12];
            int start_hour = std::stoi(shour);
            fwrite(&start_hour, 1, 2, fp);
            //start minute (2B) Start Minute is in the range 0 to 59
            char* smin = argv[13];
            int start_minute = std::stoi(smin);
            fwrite(&start_minute, 1, 2, fp);

            //flags (4B) - ignore for now (set to zero)
            unsigned char tflags[4] = { 0x00, 0x00, 0x00, 0x00 };
            fwrite(tflags, 1, 4, fp);

            /*
            trigger type (4B)
            0x00000000 	ONCE
            0x00000001 	DAILY
            0x00000002 	WEEKLY
            0x00000003 	MONTHLYDATE
            0x00000004 	MONTHLYDOW
            0x00000005 	EVENT_ON_IDLE
            0x00000006 	EVENT_AT_SYSTEMSTART
            0x00000007 	EVENT_AT_LOGON
            */
            //for now leave at 'once' - this allows us to set the rest of the file to zero
            unsigned char restoffile[24] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            fwrite(restoffile, 1, 24, fp);


            fclose(fp);
        }
    }
    else {
        std::cout << "please enter params";
    }
    return 0;
}