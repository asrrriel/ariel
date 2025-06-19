#include <fcntl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main() {

    int fd = drmOpen(NULL, NULL);
    printf("fd: %d\n", fd);
    return 0;

}
