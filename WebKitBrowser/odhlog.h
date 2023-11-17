/*
 * Helper functions to send messages to ODH.
 */
#pragma once

#ifdef USE_ODH_TELEMETRY

#include <rdk/libodherr/odherr.h>
#include <rdk/libodherr/odherr_ctx.h>

#define WPE_CONTEXT odh_ctx_create_json("wpe", "ss", "function", __PRETTY_FUNCTION__, "file", __FILE__)
#define WPE_CONTEXT_WITH_URL(url) odh_ctx_create_json("wpe", "sss", "function", __PRETTY_FUNCTION__, "file", __FILE__, "url", url)

#define ODH_REPORT_SEND(level, code, ctx, fmt, ...) do {                        \
    char *msg = odh_error_report_sprintf(fmt, ##__VA_ARGS__);                   \
    odh_error_report_send_v3(ODH_ERROR_REPORT_SENSITIVITY_NONSENSITIVE,         \
                             level,                                             \
                             code,                                              \
                             NULL,                                              \
                             msg,                                               \
                             ctx,                                               \
                             ODH_ERROR_REPORT_BACKTRACE,                        \
                             "browser");                                        \
    free(ctx);                                                                  \
    free(msg);                                                                  \
} while (0)

#define ODH_ERROR(code, ctx, fmt, ...) ODH_REPORT_SEND(ODH_ERROR_REPORT_LEVEL_ERROR, code, ctx, fmt, ##__VA_ARGS__)
#define ODH_WARNING(code, ctx, fmt, ...) ODH_REPORT_SEND(ODH_ERROR_REPORT_LEVEL_WARNING, code, ctx, fmt, ##__VA_ARGS__)
#define ODH_CRITICAL(code, ctx, fmt, ...) ODH_REPORT_SEND(ODH_ERROR_REPORT_LEVEL_CRITICAL, code, ctx, fmt, ##__VA_ARGS__)

#else /* USE_ODH_TELEMETRY */

#define WPE_CONTEXT
#define WPE_CONTEXT_WITH_URL(url)
#define ODH_ERROR(code, ctx, fmt, ...)
#define ODH_WARNING(code, ctx, fmt, ...)
#define ODH_CRITICAL(code, ctx, fmt, ...)

#endif /* USE_ODH_TELEMETRY */
