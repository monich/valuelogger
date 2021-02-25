#ifndef DEBUGLOG_H
#define DEBUGLOG_H

#include <QDebug>

#ifndef LOG_DBG
#  define LOG_DBG 0
#endif // LOG_DBG

#ifndef LOG_VERBOSE
#  define LOG_VERBOSE 0
#endif // LOG_VERBOSE

#include <QDebug>

#if LOG_VERBOSE
#  define VERBOSE(x) qDebug() << x
#else
#  define VERBOSE(expr) ((void)0)
#endif

#if LOG_DBG
#  define DBG(x) qDebug() << x
#  define ASSERT(x) ((x) ? ((void)0) : qt_assert(#x,__FILE__,__LINE__))
#  define VERIFY(x) ASSERT(x)
#else
#  define DBG(expr) ((void)0)
#  define HASSERT(expr) ((void)0)
#  define HVERIFY(x) (x)
#endif // HARBOUR_DEBUG

#define WARN(x) qWarning() << x

#endif // DEBUGLOG_H
