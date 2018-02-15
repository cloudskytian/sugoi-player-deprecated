
#ifndef SPLAYER_VERSION_H
#define SPLAYER_VERSION_H

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
#define SPLAYER_VERSION_MAJOR 1
#define SPLAYER_VERSION_MINOR 1
#define SPLAYER_VERSION_PATCH 0
#define SPLAYER_VERSION_BUILD 0
#ifndef ISPP_INVOKED
#define SPLAYER_VERSION_NUM   SPLAYER_VERSION_MAJOR,SPLAYER_VERSION_MINOR,SPLAYER_VERSION_PATCH,SPLAYER_VERSION_BUILD
#define SPLAYER_VERSION_STR   _T("1.1.0.0")
#endif
#endif

#define SPLAYER_APP_ID_32_STR        _T("{6067103B-4F27-4868-884E-61BE625B7C14}")
#define SPLAYER_APP_ID_64_STR        _T("{CEBFDF18-02C1-4CA6-A6A9-F3BE8EB492E8}")
#define SPLAYER_APP_NAME_STR         _T("SPlayer")
#define SPLAYER_APP_DISPLAY_NAME_STR _T("SPlayer")
#define SPLAYER_APP_MUTEX_32_STR     SPLAYER_APP_ID_32_STR
#define SPLAYER_APP_MUTEX_64_STR     SPLAYER_APP_ID_64_STR
#define SPLAYER_COMPANY_NAME_STR     _T("wangwenx190")
#define SPLAYER_COMPANY_URL_STR      _T("https://github.com/wangwenx190/SPlayer")
#define SPLAYER_SUPPORT_URL_STR      SPLAYER_COMPANY_URL_STR
#define SPLAYER_UPDATE_URL_STR       SPLAYER_COMPANY_URL_STR
#define SPLAYER_CONTACT_STR          SPLAYER_COMPANY_NAME_STR
#define SPLAYER_SUPPORT_PHONE_STR    _T("10010001000")
#define SPLAYER_README_URL_STR       _T("https://github.com/wangwenx190/SPlayer/blob/master/README.md")
#define SPLAYER_LICENSE_URL_STR      _T("https://github.com/wangwenx190/SPlayer/blob/master/LICENSE.md")
#define SPLAYER_COMMENTS_STR         _T("Multimedia player for Windows 7+ based on libmpv and Qt.")
#define SPLAYER_COPYRIGHT_STR        _T("GPLv3")

#if defined(_WIN64) || defined(x64)
#define SPLAYER_ARCH_STR      _T("x64")
#define SPLAYER_APP_ID_STR    SPLAYER_APP_ID_64_STR
#define SPLAYER_APP_MUTEX_STR SPLAYER_APP_MUTEX_64_STR
#else
#define SPLAYER_ARCH_STR      _T("x86")
#define SPLAYER_APP_ID_STR    SPLAYER_APP_ID_32_STR
#define SPLAYER_APP_MUTEX_STR SPLAYER_APP_MUTEX_32_STR
#endif

#endif
