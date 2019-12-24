//------------------------------------------------------------------------------
/// @brief Kernel Lab: Page Table Walk (rss.c)
/// 
/// @section license_section License
/// Copyright (c) 2018-2019, Computer Systems and Platforms Laboratory, SNU
/// All rights reserved.
///
/// Redistribution and use in source and binary forms,  with or without modifi-
/// cation, are permitted provided that the following conditions are met:
///
/// - Redistributions of source code must retain the above copyright notice,
///   this list of conditions and the following disclaimer.
/// - Redistributions in binary form must reproduce the above copyright notice,
///   this list of conditions and the following disclaimer in the documentation
///   and/or other materials provided with the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY  AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER  OR CONTRIBUTORS BE
/// LIABLE FOR ANY DIRECT,  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSE-
/// QUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF  SUBSTITUTE
/// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
/// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT
/// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY
/// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
/// DAMAGE.
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#define DBFS_FILE_PATH  "/sys/kernel/debug/ptrav/output"
#define PROC_FILE_PREFIX "/proc/"

int pid;

struct packet {
        pid_t pid;
        unsigned long pages_1g, pages_2m, pages_4k;
};


int main(int argc, char *argv[])
{
        int fd, ret, i;
        FILE *file;
        unsigned long *mem;
        char path[1024];
        char c;
        struct packet pckt;
        struct stat sts;

        if(argc != 2) {
                printf("Usage: ./app <pid>\n");
                exit(-1);
        }

        pid = atoi(argv[1]);

        memset(path, 0, sizeof(char) * 1024);
        strcat(path, PROC_FILE_PREFIX);
        strcat(path, argv[1]);

        if ((stat(path, &sts) == -1) && errno == ENOENT) {
                printf("Invalid PID!\n");
                exit(-1);
        }

        pckt.pid = pid;

        //Open debugfs
        fd = open(DBFS_FILE_PATH, O_RDWR);
        if (fd < 0) {
                printf("debugfs file open error\n");
                exit(-1);
        }

        //Read from debugfs
        ret = read(fd, &pckt, sizeof(struct packet));
        if (ret < 0) {
                printf("read failed\n");
                exit(-1);
        }

        printf("1G pages: %lu, 2M pages: %lu, 4K pages: %lu\n", pckt.pages_1g, pckt.pages_2m, pckt.pages_4k);
        printf("Resident Set Size: %lu kB\n", 1024*1024*pckt.pages_1g + 2048*pckt.pages_2m + 4*pckt.pages_4k);

        close(fd);

        return 0;
}
