#pragma once

#ifdef HAVE_LIBODHERR_ODHERR_H

#include <rdk/libodherr/odherr.hpp>

#else

#define ODH_ERROR_REPORT_CTX_ERROR(e, msg, ...)
#define ODH_ERROR_REPORT_CTX_WARN(e, msg, ...)
#define ODH_ERROR_REPORT_CTX_CRITICAL(e, msg, ...)
#define ODH_ERROR_REPORT_ERROR_FORMAT_MSG(e, format, ...)
#define ODH_ERROR_REPORT_WARN_FORMAT_MSG(e, format, ...)
#define ODH_ERROR_REPORT_CRITICAL_FORMAT_MSG(e, format, ...)
#define ODH_ERROR_REPORT_DEINIT()

#endif
