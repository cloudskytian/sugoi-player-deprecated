
#ifndef SUGOI_VERSION_H
#define SUGOI_VERSION_H

#ifndef _T
#if !defined(ISPP_INVOKED) && (defined(UNICODE) || defined(_UNICODE))
#define _T(text) L##text
#else
#define _T(text) text
#endif
#endif

#ifdef CI
#include "../ci_version.h"
#else
#define SUGOI_VERSION_MAJOR 1
#define SUGOI_VERSION_MINOR 1
#define SUGOI_VERSION_PATCH 0
#define SUGOI_VERSION_BUILD 0
#ifndef ISPP_INVOKED
#define SUGOI_VERSION_NUM   SUGOI_VERSION_MAJOR,SUGOI_VERSION_MINOR,SUGOI_VERSION_PATCH,SUGOI_VERSION_BUILD
#define SUGOI_VERSION_STR   _T("1.1.0.0")
#endif
#endif

#define SUGOI_APP_ID_32_STR        _T("{6067103B-4F27-4868-884E-61BE625B7C14}")
#define SUGOI_APP_ID_64_STR        _T("{CEBFDF18-02C1-4CA6-A6A9-F3BE8EB492E8}")
#define SUGOI_APP_NAME_STR         _T("Sugoi Player")
#define SUGOI_APP_DISPLAY_NAME_STR _T("Sugoi Player")
#define SUGOI_APP_MUTEX_32_STR     SUGOI_APP_ID_32_STR
#define SUGOI_APP_MUTEX_64_STR     SUGOI_APP_ID_64_STR
#define SUGOI_COMPANY_NAME_STR     _T("wangwenx190")
#define SUGOI_COMPANY_URL_STR      _T("https://github.com/wangwenx190/Sugoi-Player")
#define SUGOI_SUPPORT_URL_STR      SUGOI_COMPANY_URL_STR
#define SUGOI_UPDATE_URL_STR       SUGOI_COMPANY_URL_STR
#define SUGOI_CONTACT_STR          SUGOI_COMPANY_NAME_STR
#define SUGOI_SUPPORT_PHONE_STR    _T("10010001000")
#define SUGOI_README_URL_STR       _T("https://github.com/wangwenx190/Sugoi-Player/blob/master/README.md")
#define SUGOI_LICENSE_URL_STR      _T("https://github.com/wangwenx190/Sugoi-Player/blob/master/LICENSE.md")
#define SUGOI_BUG_REPORT_URL_STR   _T("https://github.com/wangwenx190/Sugoi-Player/issues")
#define SUGOI_COMMENTS_STR         _T("Multimedia player for Windows 7+ based on libmpv and Qt.")
#define SUGOI_COPYRIGHT_STR        _T("GPLv3")

#define LIBMPV_VERSION_STR           _T("2.0.0 (2017-12-25-dev)")

#if defined(_WIN64) || defined(x64)
#define SUGOI_APP_ID_STR    SUGOI_APP_ID_64_STR
#define SUGOI_APP_MUTEX_STR SUGOI_APP_MUTEX_64_STR
#else
#define SUGOI_APP_ID_STR    SUGOI_APP_ID_32_STR
#define SUGOI_APP_MUTEX_STR SUGOI_APP_MUTEX_32_STR
#endif

#endif
