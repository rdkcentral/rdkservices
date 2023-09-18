/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <string>
#include <iostream>
#include <vector>
#include <cstring>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "kms.h"

#define MEM_PROCFS "/proc/meminfo"
#define TOTAL_MEM_PARAM_STR  "MemTotal:"
#define FREE_MEM_PARAM_STR  "MemFree:"
#define DEFAULT_DEVICE "/dev/dri/card0"

using namespace std;

static uint64_t parseLine(const char * line);
static uint64_t getMemInfo(const char * param);
static void getPrimaryPlane(int drm_fd, kms_ctx *kms, drmModePlane **plane);
static void getGraphicSize(uint32_t &w, uint32_t &h);

static uint64_t parseLine(const char * line)
{
    string str(line);
    uint64_t val = 0;
    size_t begin = str.find_first_of("0123456789");
    size_t end = string::npos;

    if (string::npos != begin)
        end = str.find_first_not_of("0123456789", begin);

    if (string::npos != begin && string::npos != end)
    {

        str = str.substr(begin, end);
        val = strtoul(str.c_str(), NULL, 10);

    }
    else
    {
        cout << "Failed to parse: " << line << endl;
    }

    return val;
}

static uint64_t getMemInfo(const char * param)
{
    FILE *meminfoFile = fopen(MEM_PROCFS, "r");
    uint64_t memVal = 0;
    if (NULL == meminfoFile)
    {
        cout << "Failed to open " << MEM_PROCFS << ", error: " << strerror(errno) << endl;
    }
    else
    {
        vector <char> buf;
        buf.resize(1024);

        while (fgets(buf.data(), buf.size(), meminfoFile))
        {
             if ( strstr(buf.data(), param ) == buf.data())
             {
                 memVal = parseLine(buf.data()) * 1000;
                 break;
             }
        }

        fclose(meminfoFile);
    }
    return memVal;
}

static void getPrimaryPlane(int drm_fd, kms_ctx *kms, drmModePlane **plane)
{
    kms_get_plane(drm_fd, kms);
    cout << "Primary Plane ID : "<< kms->primary_plane_id << endl;
    *plane = drmModeGetPlane(drm_fd, kms->primary_plane_id );
    if(*plane)
        printf("fb id : %d\n", (*plane)->fb_id);
}

static void getGraphicSize(uint32_t &w, uint32_t &h)
{
    int drm_fd;
    kms_ctx *kms = NULL;
    drmModePlane *plane = NULL;
    int trytimes = 0;

    do {
        /* Setup buffer information */
        drm_fd = open( DEFAULT_DEVICE, O_RDWR);

        /* Setup KMS */
        kms = kms_setup(drm_fd);
        if(!kms || !kms->crtc ) {
            cout << "[Realtek] kms_setup fail" << endl;
            break;
        }

        /* Get primary buffer */
        getPrimaryPlane(drm_fd, kms, &plane);
        if( !plane) {
            cout << "[Realtek] fail to getPrimaryPlane" << endl;
            break;
        }

        /* get fb */
        drmModeFB *fb = drmModeGetFB(drm_fd, plane->fb_id);
        while(!fb) {
            getPrimaryPlane(drm_fd, kms, &plane);
            fb = drmModeGetFB(drm_fd, plane->fb_id);
            if (trytimes++ > 100) {
                cout << "[Realtek] fail to getPrimaryPlane" << endl;
                break;
            }
        }

        /* Get the width and height */
        if(fb) {
            w = fb->width;
            h = fb->height;
            drmModeFreeFB(fb);
        }
    } while(0);

    /* release */
    /* Cleanup buffer info */
    if(kms) {
        kms_cleanup_context(kms);
        free(kms);
    }

    cout << "[getGraphicSize] width : " << w << endl;
    cout << "[getGraphicSize] height : " << h << endl;
}


uint64_t SoC_GetTotalGpuRam()
{
    return getMemInfo(TOTAL_MEM_PARAM_STR);
}

uint64_t SoC_GetFreeGpuRam()
{
    return getMemInfo(FREE_MEM_PARAM_STR);
}

uint32_t SoC_GetGraphicsWidth()
{
    uint32_t w, h;
    getGraphicSize(w, h);
    return w;
}

uint32_t SoC_GetGraphicsHeight()
{
    uint32_t w, h;
    getGraphicSize(w, h);
    return h;
}
