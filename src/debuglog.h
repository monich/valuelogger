/*
Copyright (c) 2021 Slava Monich <slava@monich.com>

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

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
#endif // LOG_DBG

#define WARN(x) qWarning() << x

#endif // DEBUGLOG_H
