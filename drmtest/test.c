#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main() {

    int fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if (fd == -1) {
        printf("Failed to open drm device\n");
        return 1;
    }
    drmVersionPtr version = drmGetVersion(fd);
    if (version) {
        printf("Driver version: %d.%d.%d\n",version->version_major, version->version_minor, version->version_patchlevel);
        printf("Driver name: %s\n", version->name);
        printf("Driver date: %s\n", version->date);
        printf("Driver description: %s\n", version->desc);
        drmFreeVersion(version);
    }

    uint64_t cap;
    int ret = drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &cap);
    if (ret != 0 || cap == 0) {
        printf("Dumb buffer unsupported\n");
        return 1;
    }
    printf("Dumb buffer supported\n");

    drmModeConnector *connector = NULL;

    drmModeRes *resources = drmModeGetResources(fd);
    for (int i = 0; i < resources->count_connectors; ++i) {
        drmModeConnector *conn = drmModeGetConnector(fd, resources->connectors[i]);
        if (conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0) {
            printf("Connector %d: %dx%d\n", i, conn->modes[0].hdisplay, conn->modes[0].vdisplay);
            connector = conn;
            break;
        }
    }

    if (connector == NULL) {
        printf("No connector found\n");
        return 1;
    }

    printf("Using connector %d: %dx%d\n", connector->connector_id, connector->modes[0].hdisplay, connector->modes[0].vdisplay);

    drmModeEncoder* encoder = drmModeGetEncoder(fd, connector->encoders[0]);

    if (encoder == NULL) {
        printf("Failed to get encoder\n");
        return 1;
    }

    printf("Using encoder %d\n", encoder->encoder_id);

    drmModeCrtc* crtc = drmModeGetCrtc(fd, encoder->crtc_id);

    if (crtc == NULL) {
        printf("Failed to get crtc\n");
        return 1;
    }

    printf("Using CRTC %d\n", crtc->crtc_id);

    uint32_t handle;
    uint32_t pitch;
    uint64_t size;
    if(drmModeCreateDumbBuffer(fd, connector->modes[0].hdisplay, connector->modes[0].vdisplay, 32,0 , &handle, &pitch, &size)){
        printf("Failed to create dumb buffer\n");
        return 1;
    }

    uint32_t fb;
    if (drmModeAddFB(fd, connector->modes[0].hdisplay, connector->modes[0].vdisplay, 24, 32, pitch, handle, &fb)){
        printf("Failed to add framebuffer\n");
        return 1;
    }

    uint64_t offset;
    if (drmModeMapDumbBuffer(fd, handle, &offset)) {
        printf("Failed to prepare dumb buffer for use\n");
        return 1;
    }

    uint8_t *fb_ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
    if (fb_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // fill it with white
    memset(fb_ptr, 0xFF, size);

    if(drmModeSetCrtc(fd, crtc->crtc_id, fb, 0, 0, &connector->connector_id, 1, &connector->modes[0])){
        printf("Failed to modeset crtc\n");
        return 1;
    }

    return 0;

}
