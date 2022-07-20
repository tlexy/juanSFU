#ifndef RTC_BASE_LOGGING_H_
#define RTC_BASE_LOGGING_H_

#include <iostream>  

#define LS_VERBOSE
#define LS_INFO
#define LS_ERROR

#define RTC_LOG(level) std::cout

#define RTC_LOG_F(level) std::cout

#define RTC_DLOG(level) std::cout

#define RTC_DLOG_F(level) std::cout

#endif