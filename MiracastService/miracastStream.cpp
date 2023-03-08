#include <gst/audio/audio.h>
#include <gst/app/gstappsink.h>

#include <map>
#include <list>
#include <mutex>
#include <thread>
#include <condition_variable>

#include <curl/curl.h>
#include <unistd.h>
#include <regex>

#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

GMainLoop* gLoop;
GstElement *m_pipeline;
std::thread *m_gstThread;
int GstBusCallback(GstBus *, GstMessage *message, gpointer data);

// Logging - Start
void log(const char* func,
    const char* file,
    int line,
    int threadID,
    const char* format, ...)
{
    const short kFormatMessageSize = 4096;
    char formatted[kFormatMessageSize];

    va_list argptr;
    va_start(argptr, format);
    vsnprintf(formatted, kFormatMessageSize, format, argptr);
    va_end(argptr);

    char timestamp[0xFF] = {0};
    struct timespec spec;
    struct tm tm;

    clock_gettime(CLOCK_REALTIME, &spec);
    gmtime_r(&spec.tv_sec, &tm);
    long ms = spec.tv_nsec / 1.0e6;

    sprintf(timestamp, "%02d%02d%02d-%02d:%02d:%02d.%03ld",
        tm.tm_year % 100,
        tm.tm_mon + 1,
        tm.tm_mday,
        tm.tm_hour,
        tm.tm_min,
        tm.tm_sec,
        ms);

    if (threadID)
    {
        printf("%s [tid=%d] %s:%s:%d %s\n",
            timestamp,
            threadID,
            func, basename(file), line,
            formatted);
    }
    else
    {
        printf("%s %s:%s:%d %s\n",
            timestamp,
            func, basename(file), line,
            formatted);
    }

    fflush(stdout);
}

#define _LOG(FORMAT, ...)          \
    log(__func__, __FILE__, __LINE__, syscall(__NR_gettid), \
         FORMAT,                          \
         ##__VA_ARGS__)

#define MIRACASTLOG_TRACE(FMT, ...)   _LOG(FMT, ##__VA_ARGS__)
#define MIRACASTLOG_VERBOSE(FMT, ...) _LOG(FMT, ##__VA_ARGS__)
#define MIRACASTLOG_INFO(FMT, ...)    _LOG(FMT, ##__VA_ARGS__)
#define MIRACASTLOG_WARNING(FMT, ...) _LOG(FMT, ##__VA_ARGS__)
#define MIRACASTLOG_ERROR(FMT, ...)   _LOG(FMT, ##__VA_ARGS__)
#define MIRACASTLOG_FATAL(FMT, ...)   _LOG(FMT, ##__VA_ARGS__)
// Logging - End

static void on_pad_added(GstElement *gstelement, GstPad *pad, gpointer user_data)
{
    bool result = TRUE;

    if(!result) {
        MIRACASTLOG_ERROR("failed to link elements!");
        gst_object_unref(m_pipeline);
        m_pipeline = NULL;
        return;
    }
    GstCaps *caps = gst_pad_get_caps(pad);
    GstStructure *s = gst_caps_get_structure(caps, 0);

    MIRACASTLOG_INFO("on_pad_added %s", gst_structure_get_name(s));

    if (strcmp (gst_structure_get_name(s), "audio") >= 0)
    {
        result &= gst_element_link_many (m_aQueue, m_audioFilter, m_audioDecoder, m_audioSink, NULL);
        GstElement *sink = (GstElement *)user_data;
        GstPad* sinkpad = gst_element_get_static_pad (sink, "sink");
        bool linked = GST_PAD_LINK_SUCCESSFUL(gst_pad_link (pad, sinkpad));
        if (!linked)
        {
            MIRACASTLOG_ERROR("Failed to link demux and audio");
        }
        else
        {
            MIRACASTLOG_INFO("Configured audio");
        }
        gst_object_unref (sinkpad);
    }
    else if (strcmp (gst_structure_get_name(s), "video") >= 0)
    {
        result &= gst_element_link_many (m_vQueue, m_videoFilter, m_videoDecoder, m_videoSink, NULL);

        GstElement *sink = (GstElement *)user_data;
        GstPad* sinkpad = gst_element_get_static_pad (sink, "queue");
        bool linked = GST_PAD_LINK_SUCCESSFUL(gst_pad_link (pad, sinkpad));
        if (!linked)
        {
            MIRACASTLOG_ERROR("Failed to link demux and video");
        }
        else
        {
            MIRACASTLOG_INFO("Configured video");
        }
        gst_object_unref (sinkpad);
    }

}

void createPipeline() {
    MIRACASTLOG_TRACE("Creating Pipeline...");
    m_pipeline = gst_pipeline_new(NULL);
    if (!m_pipeline) {
        MIRACASTLOG_ERROR("Failed to create gstreamer pipeline");
        return;
    }

    GstElement *m_source = gst_element_factory_make("udpsrc", NULL);
    g_object_set(mSource, "port", "1991", NULL);
    GstCaps *caps = gst_caps_new_simple("application/x-rtp", "media", G_TYPE_STRING, "video", NULL); 
    g_object_set(mSource, "caps", caps, NULL);

    GstElement *m_rtpDePayloader = gst_element_factory_make("rtpmp2tdepay", NULL);
    GstElement *m_demux = gst_element_factory_make("tsdemux", NULL);

    GstElement *m_vQueue = gst_element_factory_make("queue", NULL);
    g_object_set(m_vQueue, "max-size-buffers", 0, NULL);
    g_object_set(m_vQueue, "max-size-time", 0, NULL);

    GstElement *m_videoFilter = gst_element_factory_make("brcmvidfilter", NULL);
    GstElement *m_videoDecoder = gst_element_factory_make("brcmvideodecoder", NULL);
    GstElement *m_videoSink = gst_element_factory_make("brcmvideosink", NULL);

    GstElement *m_aQueue = gst_element_factory_make("queue", NULL);
    g_object_set(m_aQueue, "max-size-buffers", 0, NULL);
    g_object_set(m_aQueue, "max-size-time", 0, NULL);

    GstElement *m_audioFilter = gst_element_factory_make("brcmaudfilter", NULL);
    GstElement *m_audioDecoder = gst_element_factory_make("brcmaudiodecoder", NULL);
    GstElement *m_audioSink = gst_element_factory_make("brcmaudiosink", NULL);


    gst_bin_add_many(GST_BIN(m_pipeline), m_source, m_rtpDePayloader, m_demux, m_vQueue, m_videoFilter, m_videoDecoder, m_videoSink, m_audioSink, m_aQueue, m_audioFilter, m_audioDecoder, m_audioSink, NULL);

    g_signal_connect(m_demux, "pad-added", G_CALLBACK(on_pad_added), );

    GstBus *bus = gst_element_get_bus(m_pipeline);
    gst_bus_add_watch(bus, GstBusCallback, (gpointer)(NULL));
    gst_object_unref(bus);

    gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
}

void GStreamerThreadFunc(void *) {
    MIRACASTLOG_INFO("Starting GStreamerThread");
    createPipeline();

    if(!m_pipeline)
        return;

    gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    sleep(3);
    gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
    sleep(1);
    gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
}

int GstBusCallback(GstBus *, GstMessage *message, gpointer) {
    GError* error = NULL;
    gchar* debug = NULL;

    if(!m_pipeline) {
        MIRACASTLOG_WARNING("NULL Pipeline");
        return false;
    }

    switch (GST_MESSAGE_TYPE(message)){
        case GST_MESSAGE_ERROR:
            gst_message_parse_error(message, &error, &debug);
            MIRACASTLOG_ERROR("error! code: %d, %s, Debug: %s", error->code, error->message, debug);
            GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(m_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "error-pipeline");
            break;

        case GST_MESSAGE_WARNING:
            gst_message_parse_warning(message, &error, &debug);
            MIRACASTLOG_WARNING("warning! code: %d, %s, Debug: %s", error->code, error->message, debug);
            break;

        case GST_MESSAGE_EOS:
            MIRACASTLOG_INFO("EOS message received");
            break;

        case GST_MESSAGE_STATE_CHANGED:
            gchar* filename;
            GstState oldstate, newstate, pending;
            gst_message_parse_state_changed (message, &oldstate, &newstate, &pending);

            MIRACASTLOG_VERBOSE("%s old_state %s, new_state %s, pending %s",
                    GST_MESSAGE_SRC_NAME(message) ? GST_MESSAGE_SRC_NAME(message) : "",
                    gst_element_state_get_name (oldstate), gst_element_state_get_name (newstate), gst_element_state_get_name (pending));

            if (GST_ELEMENT(GST_MESSAGE_SRC(message)) != m_pipeline)
                break;

            filename = g_strdup_printf("%s-%s", gst_element_state_get_name(oldstate), gst_element_state_get_name(newstate));
            GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(m_pipeline), GST_DEBUG_GRAPH_SHOW_ALL, filename);
            g_free(filename);
            break;

        default:
            MIRACASTLOG_WARNING("Msg Src=%s, Type=%s", GST_MESSAGE_SRC_NAME(message), GST_MESSAGE_TYPE_NAME(message));
            break;
    }

    if(error)
        g_error_free(error);

    if(debug)
        g_free(debug);

    return true;
}

int main() {
    gLoop = g_main_loop_new(g_main_context_default(), FALSE);
    gst_init(NULL, NULL);

#if 1
    g_timeout_add_seconds(0, [](void *) -> int {
            createPipeline(); return G_SOURCE_REMOVE;
        }, NULL);
    g_timeout_add_seconds(3, [](void *) -> int {
            gst_element_set_state(m_pipeline, GST_STATE_PLAYING); return G_SOURCE_REMOVE;
        }, NULL);
//    g_timeout_add_seconds(8, [](void *) -> int {
//            gst_element_set_state(m_pipeline, GST_STATE_PAUSED); return G_SOURCE_REMOVE;
//        }, NULL);
//    g_timeout_add_seconds(10, [](void *) -> int {
//            gst_element_set_state(m_pipeline, GST_STATE_PLAYING); return G_SOURCE_REMOVE;
//        }, NULL);
#else
    m_gstThread = new std::thread([]{
            GStreamerThreadFunc(NULL);
    });
#endif

    g_main_loop_run(gLoop);
}

