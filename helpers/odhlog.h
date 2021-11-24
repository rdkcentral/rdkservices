/*
 * Helper functions to send messages to ODH.
 */
#pragma once

#include <rdk/libodherr/odherr.h>

#define ODH_ERROR(fmt, ...) do { char *msg = odh_error_report_sprintf(fmt, ##__VA_ARGS__); ODH_ERROR_REPORT_SRC_ERROR_V3("0", NULL, msg, NULL, NULL); free(msg);} while (0)
#define ODH_WARNING(fmt, ...) do { char *msg = odh_error_report_sprintf(fmt, ##__VA_ARGS__); ODH_ERROR_REPORT_SRC_WARNING_V3("0", NULL, msg, NULL, NULL); free(msg);} while (0)
#define ODH_CRITICAL(fmt, ...) do { char *msg = odh_error_report_sprintf(fmt, ##__VA_ARGS__); ODH_ERROR_REPORT_SRC_CRITICAL_V3("0", NULL, msg, NULL, NULL); free(msg);} while (0)
