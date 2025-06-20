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


int init_drm() {
    int fd = -1;
    for (int i = 0; i < 10; i++) {
        char name[32];
        sprintf(name, "/dev/dri/card%d", i);
        fd = open(name, O_RDWR | O_CLOEXEC);
        if (fd != -1) {
            printf("Found and opened drm device \"%s\"\n", name);
            break;
        }
    }
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
        return -1;
    }

    return fd;
}

typedef struct {
    int count;
    drmModeConnector **connectors;
} connected_connectors;

connected_connectors enumerate_resources(int fd) {
    drmModeRes *resources = drmModeGetResources(fd);

    if (resources->count_connectors == 0) {
        return (connected_connectors){0, NULL};
    }

    int count_connectors = 0;
    drmModeConnector **connectors = NULL;

    for (int i = 0; i < resources->count_connectors; ++i) {
        drmModeConnector *conn = drmModeGetConnector(fd, resources->connectors[i]);
        if (conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0) {
            count_connectors++;
            connectors = realloc(connectors, count_connectors * sizeof(drmModeConnector*));
            connectors[count_connectors - 1] = conn;
        } else {
            drmModeFreeConnector(conn);
        }
    }

    for(int i = 0; i < count_connectors; i++) {
        drmModeConnector *conn = connectors[i];
        printf("Connector %d modes:\n", i);
        for (int j = 0; j < conn->count_modes; ++j) {
            printf("    Mode %d: %dx%d %dHz\n", j, conn->modes[j].hdisplay, conn->modes[j].vdisplay, conn->modes[j].vrefresh);
        }
    }

    printf("Number of connected connectors: %d\n", count_connectors);

    return (connected_connectors){count_connectors, connectors}; // WIP
}

typedef struct {
    drmModeCrtc *crtc;
    uint32_t handle;
    uint32_t pitch;
    uint64_t size;
    uint32_t fb_id;
    uint64_t offset;
    uint8_t *fb_ptr;
} framebuffer;

framebuffer setup_framebuffer(int fd, drmModeConnector *connector) {
    drmModeEncoder* encoder = drmModeGetEncoder(fd, connector->encoders[0]);

    if (encoder == NULL) {
        printf("Failed to get encoder\n");
        return (framebuffer){};
    }

    printf("Using encoder %d\n", encoder->encoder_id);

    drmModeCrtc* crtc = drmModeGetCrtc(fd, encoder->crtc_id);

    if (crtc == NULL) {
        printf("Failed to get crtc\n");
        return (framebuffer){};
    }

    printf("Using CRTC %d\n", crtc->crtc_id);

    uint32_t handle;
    uint32_t pitch;
    uint64_t size;
    if(drmModeCreateDumbBuffer(fd, connector->modes[0].hdisplay, connector->modes[0].vdisplay, 32,0 , &handle, &pitch, &size)){
        printf("Failed to create dumb buffer\n");
        return (framebuffer){};
    }

    uint32_t fb;
    if (drmModeAddFB(fd, connector->modes[0].hdisplay, connector->modes[0].vdisplay, 24, 32, pitch, handle, &fb)){
        printf("Failed to add framebuffer\n");
        return (framebuffer){};
    }

    uint64_t offset;
    if (drmModeMapDumbBuffer(fd, handle, &offset)) {
        printf("Failed to prepare dumb buffer for use\n");
        return (framebuffer){};
    }

    uint8_t *fb_ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
    if (fb_ptr == MAP_FAILED) {
        printf("Failed to mmap framebuffer\n");
        return (framebuffer){};
    }

    return (framebuffer){crtc,handle, pitch, size, fb, offset, fb_ptr};
}


int main() {

    int fd = init_drm();

    connected_connectors connectors = enumerate_resources(fd);

    if (connectors.count == 0) {
        printf("No connector found\n");
        return 1;
    }

    
    if (!drmIsMaster(fd)){
        printf("Process is not the DRM master\n");
        return 1;
    }
    
    for (int i = 0; i < connectors.count; i++) {
        drmModeConnector *connector = connectors.connectors[i];
        printf("Modesetting connector %d: %dx%d\n", connector->connector_id, connector->modes[0].hdisplay, connector->modes[0].vdisplay);

        framebuffer fb = setup_framebuffer(fd, connector);

        memset(fb.fb_ptr, 0xFF, fb.size);

        if(drmModeSetCrtc(fd, fb.crtc->crtc_id, fb.fb_id, 0, 0, &connector->connector_id, 1, &connector->modes[0])){
            printf("Failed to modeset crtc\n");
            return 1;
        }
    }
    
    //pause();

    return 0;

}
