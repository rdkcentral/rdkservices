/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
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
**/

#include "ScreenCapture.h"

#include "UtilsJsonRpc.h"

#ifdef PLATFORM_BROADCOM
#include <nexus_config.h>
#include <nxclient.h>
#endif

#include <png.h>
#include <curl/curl.h>
#include <base64.h>

#ifdef HAS_FRAMEBUFFER_API_HEADER
extern "C" {
#include "framebuffer-api.h"
#include "framebuffer-serverapi.h"
}
#endif

// Methods
#define METHOD_UPLOAD "uploadScreenCapture"

// Events
#define EVT_UPLOAD_COMPLETE "uploadComplete"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework
{

    namespace {

        static Plugin::Metadata<Plugin::ScreenCapture> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    namespace Plugin
    {
        SERVICE_REGISTRATION(ScreenCapture, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

        ScreenCapture::ScreenCapture()
            : PluginHost::JSONRPC()
#if defined(PLATFORM_AMLOGIC)
        , m_RDKShellRef(nullptr)
        , m_captureRef(nullptr)
        , m_screenCaptureStore(this)
#endif
        {
            #ifdef PLATFORM_BROADCOM
            inNexus = false;
            #endif

            Register(METHOD_UPLOAD, &ScreenCapture::uploadScreenCapture, this);
        }

        ScreenCapture::~ScreenCapture()
        {
        }

        /* virtual */ const string ScreenCapture::Initialize(PluginHost::IShell* service)
        {
#if defined(PLATFORM_AMLOGIC)
            m_RDKShellRef = service->QueryInterfaceByCallsign<PluginHost::IPlugin>("org.rdk.RDKShell");

            if(nullptr != m_RDKShellRef)
            {
                m_captureRef = m_RDKShellRef->QueryInterface<Exchange::ICapture>();
                if (nullptr == m_captureRef)
                {
                    LOGERR("Can't get Exchange::ICapture interface");
                    m_RDKShellRef->Release();
                    m_RDKShellRef = nullptr;
                }
            }
            else
            {
                LOGERR("Can't get RDKShell interface");
            }
#else
            screenShotDispatcher = new WPEFramework::Core::TimerType<ScreenShotJob>(64 * 1024, "ScreenCaptureDispatcher");
#endif
            return { };
        }

        void ScreenCapture::Deinitialize(PluginHost::IShell* /* service */)
        {

#if defined(PLATFORM_AMLOGIC)
            if (nullptr != m_RDKShellRef)
            {
                m_RDKShellRef->Release();
                m_RDKShellRef = nullptr;
            }

            if (nullptr != m_captureRef)
            {
                m_captureRef->Release();
                m_captureRef = nullptr;
            }
#else
            delete screenShotDispatcher;
#endif
        }

#if defined(PLATFORM_AMLOGIC)
        void ScreenCapture::onScreenCaptureData(const unsigned char* buffer, const unsigned int width, const unsigned int height)
        {
            // flip the image
            uint32_t *decodedImageRGBA = (uint32_t *)buffer;
            for(size_t row = 0; row < height / 2; row++)
            {
                for(size_t col = 0; col < width; col++)
                {
                    uint32_t p = decodedImageRGBA[row * width + col];

                    decodedImageRGBA[row * width + col] = decodedImageRGBA[(height - 1 - row) * width + col];
                    decodedImageRGBA[(height - 1 - row) * width + col] = p;
                }
            }

            std::vector<unsigned char> png_out_data;
            if(!saveToPng((unsigned char *)buffer, width, height, png_out_data))
            {
                LOGERR("Failed to convert ScreenShot data to png");
                return;
            }

            doUploadScreenCapture(png_out_data, true);
        }
#endif

        uint32_t ScreenCapture::uploadScreenCapture(const JsonObject& parameters, JsonObject& response)
        {
            
            std::lock_guard<std::mutex> guard(m_callMutex);

            LOGINFOMETHOD();

            if(!parameters.HasLabel("url"))
            {
                response["message"] = "Upload url is not specified";

                returnResponse(false);
            }

            url = parameters["url"].String();

            if(parameters.HasLabel("callGUID"))
              callGUID = parameters["callGUID"].String();
              
#if defined(PLATFORM_AMLOGIC)

            if (!m_captureRef)
            {
                LOGERR("No access to Exchange::ICapture");
                returnResponse(false);
            }

            m_captureRef->Capture(m_screenCaptureStore);
#else
            screenShotDispatcher->Schedule( Core::Time::Now().Add(0), ScreenShotJob( this) );
#endif

            returnResponse(true);
        }

#if defined(PLATFORM_AMLOGIC)
        bool ScreenCaptureStore::R8_G8_B8_A8(const unsigned char* buffer, const unsigned int width, const unsigned int height)
        {
            m_screenCapture->onScreenCaptureData(buffer, width, height);

            return true;
        }
#else
        uint64_t ScreenShotJob::Timed(const uint64_t scheduledTime)
        {
            if(!m_screenCapture)
            {
                LOGERR("!m_screenCapture");
                return 0;
            }

            m_screenCapture->getScreenShot();

            return 0;
        }
#endif

        bool ScreenCapture::getScreenShot()
        {
            std::vector<unsigned char> png_data;
            bool got_screenshot = false;

            #ifdef PLATFORM_BROADCOM
            got_screenshot = getScreenshotNexus(png_data);
            #endif

            #ifdef PLATFORM_INTEL
            got_screenshot = getScreenshotIntel(png_data);
            #endif

            #ifdef HAS_FRAMEBUFFER_API_HEADER
            got_screenshot = getScreenshotRealtek(png_data);
            #endif

            return doUploadScreenCapture(png_data, got_screenshot);
        }

        bool ScreenCapture::doUploadScreenCapture(const std::vector<unsigned char> &png_data, bool got_screenshot)
        {
            if(got_screenshot)
            {
                std::string error_str;

                LOGWARN("uploading %d of png data to '%s'", png_data.size(), url.c_str() );

                if(uploadDataToUrl(png_data, url.c_str(), error_str))
                {
                    JsonObject params;
                    params["status"] = true;
                    params["message"] = "Success";
                    params["call_guid"] = callGUID;

                    sendNotify(EVT_UPLOAD_COMPLETE, params);

                    return true;
                }
                else
                {
                    JsonObject params;
                    params["status"] = false;
                    params["message"] = std::string("Upload Failed: ") + error_str;
                    params["call_guid"] = callGUID;

                    sendNotify(EVT_UPLOAD_COMPLETE, params);

                    return false;
                }
            }
            else
            {
                LOGERR("Error: could not get the screenshot");

                JsonObject params;
                params["status"] = false;
                params["message"] = "Failed to get screen data";
                params["call_guid"] = callGUID;

                sendNotify(EVT_UPLOAD_COMPLETE, params);

                return false;
            }
        }

#ifdef PLATFORM_INTEL
        bool ScreenCapture::getScreenshotIntel(std::vector<unsigned char> &png_out_data)
        {
            int i;
            char *filename = "/proc/gdl/dump/wbp";    //both video and guide graphics, potentially at lower 720x480
//             char *filename = "/proc/gdl/dump/upp_d"; //graphics only, normally at higher 1280x720
//             char *filename = "/proc/gdl/dump/upp_a"; //video only, normally at higher 1280x720

            FILE* fp = fopen(filename, "rb");

            unsigned char info[56];
            fread(info, sizeof(unsigned char), 56, fp); // read the 54-byte header

            if(!fp)
            {
                LOGERR("Error: could not open image file '%s'", filename);
                return false;
            }

            // extract image height and width from header
            int w = abs(*(int*)&info[18]);
            int h = abs(*(int*)&info[22]);

            int size = 4 * w * h;

            LOGWARN("intel screenshot capture of size w:%d h:%d loaded", w, h);

            if(size < 1)
            {
                LOGERR("Error: png data size < 1");
                return false;
            }

            std::vector<unsigned char> data_v(size);
            std::vector<unsigned char> new_data_v(size);

            unsigned char* data = &data_v[0];
            unsigned char* new_data = &new_data_v[0];

            fread(data, sizeof(unsigned char), size, fp); // read the rest of the data at once
            fclose(fp);

            for(i = 0; i < size; i += 4)
            {
                //r and b need swapped?
                new_data[i+0] = data[i+2];
                new_data[i+1] = data[i+1];
                new_data[i+2] = data[i+0];
                new_data[i+3] = data[i+3];
            }

            //convert to png
            saveToPng(new_data, w, h, png_out_data);

            return true;
        }
#endif

#ifdef PLATFORM_BROADCOM
        bool ScreenCapture::joinNexus()
        {
            if(inNexus) return true;

            NxClient_JoinSettings joinSettings;

            NxClient_GetDefaultJoinSettings(&joinSettings);

            snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", "wpeframework");

            NEXUS_Error rc = NxClient_Join(&joinSettings);

            if (!(( rc == NEXUS_SUCCESS )))
            {
                LOGERR("could not join Nexus");
                return false;
            }

            LOGWARN("Nexus Joined");

            inNexus = true;

            return true;
        }

        bool ScreenCapture::getScreenshotNexus(std::vector<unsigned char> &png_out_data)
        {
            if(!joinNexus())
            {
                LOGERR("could not join Nexus");
                return false;
            }

            NEXUS_Error err = NEXUS_SUCCESS;
            bool res = true;

            NxClient_ScreenshotSettings screenshotSettings;
//             NxClient_GetDefaultScreenshotSettings(&screenshotSettings);
            memset(&screenshotSettings, 0, sizeof(screenshotSettings));

            #ifdef SCREENCAP_SVP_ENABLED
            screenshotSettings.screenshotWindow = NxClient_ScreenshotWindow_eGraphics;
            LOGWARN("[SCREENCAP]: Using NxClient_ScreenshotWindow_eGraphics (graphics only, no video)");
            #else
            screenshotSettings.screenshotWindow = NxClient_ScreenshotWindow_eAll;
            LOGWARN("[SCREENCAP]: Using NxClient_ScreenshotWindow_eAll (graphics including video)");
            #endif

            NEXUS_SurfaceCreateSettings defSurfSettings;
            memset(&defSurfSettings, 0, sizeof(defSurfSettings));
            NEXUS_Surface_GetDefaultCreateSettings( &defSurfSettings );

            defSurfSettings.width = 1280;
            defSurfSettings.height = 720;
            //defSurfSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
            defSurfSettings.pixelFormat = NEXUS_PixelFormat_eA8_B8_G8_R8;
            int bytesPerPixel = 4;
            std::vector<unsigned char> data_v(1280 * 720 * 4);
            unsigned char *bytes = &data_v[0];
//             unsigned char bytes[1280 * 720 * 4];


            NEXUS_SurfaceHandle surface = NEXUS_Surface_Create( &defSurfSettings );

            err = NxClient_Screenshot(&screenshotSettings, surface);

            if (err != NEXUS_SUCCESS)
            {
                LOGERR("[SCREENCAP]: Failed to get screenshot");
                res = false;
                goto do_destroy_surface;
            }

            NEXUS_SurfaceMemoryProperties properties;
            NEXUS_Surface_GetMemoryProperties(surface, &properties);

            void* pSurfaceMemory;
            err = NEXUS_Surface_Lock(surface, &pSurfaceMemory);

            if (err != NEXUS_SUCCESS)
            {
                LOGERR("[SCREENCAP]: Failed to lock surface");
                res = false;
                goto do_destroy_surface;
            }
            else
            {
                LOGWARN("[SCREENCAP]: locked surface (pSurfaceMemory:%p pixelMemoryOffset:%d w:%d h:%d bpp:%d)",
                        pSurfaceMemory, properties.pixelMemoryOffset, defSurfSettings.width, defSurfSettings.height, bytesPerPixel);
            }

            memcpy(bytes, (const char*) pSurfaceMemory + properties.pixelMemoryOffset, defSurfSettings.width * defSurfSettings.height * bytesPerPixel);

            NEXUS_Surface_Unlock( surface );

            LOGWARN("[SCREENCAP]: unlocked surface");

            do_destroy_surface:
            NEXUS_Surface_Destroy( surface );

            if(!res)
            {
                LOGERR("could not get screenshot from Nexus");
                return false;
            }

            if(!saveToPng(bytes, defSurfSettings.width, defSurfSettings.height, png_out_data))
            {
                LOGERR("could not convert Nexus screenshot to png");
                return false;
            }
            else
                return true;
        }
#endif

#ifdef HAS_FRAMEBUFFER_API_HEADER
        static vnc_bool_t FakeVNCServerFramebufferUpdateReady(void* ctx) {
            LOGWARN("FakeVNCServerFramebufferUpdateReady called");
            return vnc_true;
        }

        static void FakeVNCServerFramebufferDetailsChanged(void* ctx, vnc_uint8_t* fb, vnc_uint16_t width, vnc_uint16_t height, vnc_uint16_t stride, PixelFormat* pf) {
            LOGWARN("FakeVNCServerFramebufferDetailsChanged called");
        }

        static void FakeVNCServerPaletteChanged(void* ctx, Palette* palette) {
            LOGWARN("FakeVNCServerPaletteChanged called");
        }

        static void FakeVNCServerLogMessage(void* ctx_, const char* fmt, ...) {
            LOGWARN("VNCServerLogMessage called");
        }

        bool ScreenCapture::getScreenshotRealtek(std::vector<unsigned char> &png_out_data)
        {
            ErrCode err;
            vnc_bool_t result;
            vnc_uint8_t* buffer; 
            VncServerFramebufferAPI api;

            api.framebufferUpdateReady = FakeVNCServerFramebufferUpdateReady;
            api.framebufferDetailsChanged = FakeVNCServerFramebufferDetailsChanged;
            api.paletteChanged = FakeVNCServerPaletteChanged;
            api.logMsg = FakeVNCServerLogMessage;

            FBContext *context = NULL;
            err = fbCreate(&context);
            if (err != ErrNone) {
                LOGERR("fbCreate fail");
                return false;
            }

            err = fbInit(context, &api, NULL);
            if (err != ErrNone) {
                LOGERR("fbInit fail");
                fbDestroy(context);
                return false;
            }

            PixelFormat *pf = fbGetPixelFormat(context);
            if(pf) {
                LOGINFO("fbGetPixelFormat:");
                LOGINFO("\tbitsPerPixel=%d", pf->bitsPerPixel );
                LOGINFO("\tdepth=%d", pf->depth);
                LOGINFO("\tbigEndian=%s", (pf->bigEndian)?"true":"false");
                LOGINFO("\ttrueColour=%s", (pf->trueColour)?"true":"false");
                LOGINFO("\tredMax=%d", pf->redMax);
                LOGINFO("\tgreenMax=%d", pf->greenMax);
                LOGINFO("\tblueMax=%d", pf->blueMax);
                LOGINFO("\tredShift=%d", pf->redShift);
                LOGINFO("\tgreenShift=%d", pf->greenShift);
                LOGINFO("\tblueShift=%d", pf->blueShift);
            }

            if (32 != pf->bitsPerPixel)
            {
                LOGERR("Unsupported bits per pixel: %d", pf->bitsPerPixel);
                fbDestroy(context);
                return false;
            }

            vnc_uint16_t w = fbGetWidth(context);
            LOGINFO("fbGetWidth=%d", w);
            vnc_uint16_t h = fbGetHeight(context);
            LOGINFO("fbGetHeight=%d", h);
            vnc_uint16_t s = fbGetStride(context);
            LOGINFO("fbGetStride=%d", s);

            buffer = fbGetFramebuffer(context);

            if(buffer) {
                LOGINFO("fbGetFramebuffer=ok"); 

                for(unsigned int n = 0; n < h; n++)
                {
                    for(unsigned int i = 0; i < w; i++)
                    {
                        unsigned char *color = buffer + n * s + i * 4;

                        unsigned char blue = color[0];
                        color[0] =  color[2];
                        color[2] = blue;
                    }
                }

                if(!saveToPng(buffer, w, h, png_out_data))
                {
                    LOGERR("could not convert Nexus screenshot to png");
                    fbDestroy(context);
                    return false;
                }
                LOGINFO("[Done]");

            } else {
                LOGERR("fbGetFramebuffer=null");
                fbDestroy(context);
                return false;
            }

            err = fbDestroy(context);
            if (err != ErrNone)
                LOGERR("fbDestroy fail");

            return true;
        }
#endif

        static void PngWriteCallback(png_structp  png_ptr, png_bytep data, png_size_t length)
        {
            std::vector<unsigned char> *p = (std::vector<unsigned char>*)png_get_io_ptr(png_ptr);
            p->insert(p->end(), data, data + length);
        }

        bool ScreenCapture::uploadDataToUrl(const std::vector<unsigned char> &data, const char *url, std::string &error_str)
        {
            CURL *curl;
            CURLcode res;
            bool call_succeeded = true;

            if(!url || !strlen(url))
            {
                LOGERR("no url given");
                return false;
            }

            LOGWARN("uploading png data of size %u to '%s'", data.size(), url);

            //init curl
            curl_global_init(CURL_GLOBAL_ALL);
            curl = curl_easy_init();

            if(!curl)
            {
                LOGERR("could not init curl\n");
                return false;
            }

            //create header
            struct curl_slist *chunk = NULL;
            chunk = curl_slist_append(chunk, "Content-Type: image/png");

            //set url and data
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, &data[0]);

            //perform blocking upload call
            res = curl_easy_perform(curl);

            //output success / failure log
            if(CURLE_OK == res)
            {
                long response_code;

                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

                if(600 > response_code && response_code >= 400)
                {
                    LOGERR("uploading failed with response code %ld\n", response_code);
                    error_str = std::string("response code:") + std::to_string(response_code);
                    call_succeeded = false;
                }
                else
                    LOGWARN("upload done");
            }
            else
            {
                LOGERR("upload failed with error %d:'%s'", res, curl_easy_strerror(res));
                error_str = std::to_string(res) + std::string(":'") + std::string(curl_easy_strerror(res)) + std::string("'");
                call_succeeded = false;
            }

            //clean up curl object
            curl_easy_cleanup(curl);
            curl_slist_free_all(chunk);

            return call_succeeded;
        }

        bool ScreenCapture::saveToPng(unsigned char *data, int width, int height, std::vector<unsigned char> &png_out_data)
        {
            int bitdepth = 8;
            int colortype = PNG_COLOR_TYPE_RGBA;
            int pitch = 4 * width;
            int transform = PNG_TRANSFORM_IDENTITY;

            int i = 0;
            int r = 0;
            png_structp png_ptr = NULL;
            png_infop info_ptr = NULL;
            png_bytep* row_pointers = NULL;

            if (NULL == data)
            {
                LOGERR("Error: failed to save the png because the given data is NULL.");
                r = -1;
                goto error;
            }

            if (0 == pitch)
            {
                LOGERR("Error: failed to save the png because the given pitch is 0.");
                r = -3;
                goto error;
            }

            png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            if (NULL == png_ptr)
            {
                LOGERR("Error: failed to create the png write struct.");
                r = -5;
                goto error;
            }

            info_ptr = png_create_info_struct(png_ptr);
            if (NULL == info_ptr)
            {
                LOGERR("Error: failed to create the png info struct.");
                r = -6;
                goto error;
            }

            png_set_IHDR(png_ptr,
                            info_ptr,
                            width,
                            height,
                            bitdepth,                 /* e.g. 8 */
                            colortype,                /* PNG_COLOR_TYPE_{GRAY, PALETTE, RGB, RGB_ALPHA, GRAY_ALPHA, RGBA, GA} */
                            PNG_INTERLACE_NONE,       /* PNG_INTERLACE_{NONE, ADAM7 } */
                            PNG_COMPRESSION_TYPE_BASE,
                            PNG_FILTER_TYPE_BASE);

            row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);

            for (i = 0; i < height; ++i)
                row_pointers[i] = data + i * pitch;

            png_set_write_fn(png_ptr, &png_out_data, PngWriteCallback, NULL);
            png_set_rows(png_ptr, info_ptr, row_pointers);
            png_write_png(png_ptr, info_ptr, transform, NULL);

            error:

            if (NULL != png_ptr)
            {

                if (NULL == info_ptr)
                {
                    LOGERR("Error: info ptr is null. not supposed to happen here.\n");
                }

                png_destroy_write_struct(&png_ptr, &info_ptr);
                png_ptr = NULL;
                info_ptr = NULL;
            }

            if (NULL != row_pointers)
            {
                free(row_pointers);
                row_pointers = NULL;
            }

            return (r==0);
        }

    } // namespace Plugin
} // namespace WPEFramework
