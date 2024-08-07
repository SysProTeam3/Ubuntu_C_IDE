#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "errLog.h"

void start_logging() {
    int log_fd = open(LOG_FILE_NAME, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IRGRP);
    dup2(log_fd, 2);
    close(log_fd);
}

void end_logging() {
    close(2);
}