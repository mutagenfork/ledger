/*
 * Copyright (c) 2003-2008, John Wiegley.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of New Artisans LLC nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file   utils.h
 * @author John Wiegley
 * @date   Sun May  6 21:20:00 2007
 *
 * @brief  This file contains general utility facilities used by Ledger.
 *
 * Ledger has need of the following utility code, which this file
 * provides or includes in:
 *
 * - system headers
 * - asserts
 * - verification (basically, "heavy asserts")
 * - tracing code
 * - debug logging code
 * - timing code
 * - current error context
 * - exception framework
 * - date/time type
 * - supports_flags<> for objects that use flags
 * - push_variable<> for restoring variable values
 */

#ifndef _UTILS_H
#define _UTILS_H

#if defined(DEBUG_MODE)
#define BOOST_MULTI_INDEX_ENABLE_SAFE_MODE 1
#define BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING 1
#endif

#include <system.hh>

/**********************************************************************
 *
 * Default values
 */

#if defined(DEBUG_MODE)
#define VERIFY_ON   1
#define TRACING_ON  1
#define DEBUG_ON    1
#define TIMERS_ON   1
#elif defined(NDEBUG)
#define NO_ASSERTS  1
#define NO_LOGGING  1
#else
#define VERIFY_ON   1		// compiled in, use --verify to enable
#define TRACING_ON  1		// use --trace X to enable
#define TIMERS_ON   1
#endif

/**********************************************************************
 *
 * Forward declarations
 */

namespace ledger {
  using namespace boost;

#if defined(VERIFY_ON)
  class string;
#else
  typedef std::string string;
#endif

  typedef std::list<string> strings_list;

  typedef posix_time::ptime	    ptime;
  typedef ptime::time_duration_type time_duration;
  typedef gregorian::date	    date;
  typedef gregorian::date_duration  date_duration;
  typedef posix_time::seconds	    seconds;

  typedef boost::filesystem::path	      path;
  typedef boost::filesystem::ifstream	      ifstream;
  typedef boost::filesystem::ofstream	      ofstream;
  typedef boost::filesystem::filesystem_error filesystem_error;
}

/**********************************************************************
 *
 * Assertions
 */

#ifdef assert
#undef assert
#endif

#if ! defined(NO_ASSERTS)
#define ASSERTS_ON 1
#endif
#if defined(ASSERTS_ON)

namespace ledger {
  void debug_assert(const string& reason, const string& func,
		    const string& file, unsigned long line);
}

#define assert(x)						\
  ((x) ? ((void)0) : debug_assert(#x, BOOST_CURRENT_FUNCTION,	\
				  __FILE__, __LINE__))

#else // ! ASSERTS_ON

#define assert(x)

#endif // ASSERTS_ON

/**********************************************************************
 *
 * Verification (basically, very slow asserts)
 */

#if defined(VERIFY_ON)

namespace ledger {

extern bool verify_enabled;

#define VERIFY(x)   (ledger::verify_enabled ? assert(x) : ((void)0))
#define DO_VERIFY() ledger::verify_enabled

void initialize_memory_tracing();
void shutdown_memory_tracing();

std::size_t current_memory_size();
std::size_t current_objects_size();

void trace_ctor_func(void * ptr, const char * cls_name, const char * args,
		     std::size_t cls_size);
void trace_dtor_func(void * ptr, const char * cls_name, std::size_t cls_size);

#define TRACE_CTOR(cls, args) \
  (DO_VERIFY() ? \
   ledger::trace_ctor_func(this, #cls, args, sizeof(cls)) : ((void)0))
#define TRACE_DTOR(cls) \
  (DO_VERIFY() ? \
   ledger::trace_dtor_func(this, #cls, sizeof(cls)) : ((void)0))

void report_memory(std::ostream& out, bool report_all = false);

/**
 * This string type is a wrapper around std::string that allows us to
 * trace constructor and destructor calls.
 */
class string : public std::string
{
public:
  string();
  string(const string& str);
  string(const std::string& str);
  string(const int len, char x);
  string(const char * str);
  string(const char * str, const char * end);
  string(const string& str, int x);
  string(const string& str, int x, int y);
  string(const char * str, int x);
  string(const char * str, int x, int y);
  ~string() throw();
};

inline string operator+(const string& __lhs, const string& __rhs)
{
  string __str(__lhs);
  __str.append(__rhs);
  return __str;
}

string operator+(const char* __lhs, const string& __rhs);
string operator+(char __lhs, const string& __rhs);

inline string operator+(const string& __lhs, const char* __rhs)
{
  string __str(__lhs);
  __str.append(__rhs);
  return __str;
}

inline string operator+(const string& __lhs, char __rhs)
{
  typedef string		__string_type;
  typedef string::size_type	__size_type;
  __string_type __str(__lhs);
  __str.append(__size_type(1), __rhs);
  return __str;
}

inline bool operator==(const string& __lhs, const string& __rhs)
{ return __lhs.compare(__rhs) == 0; }

inline bool operator==(const char* __lhs, const string& __rhs)
{ return __rhs.compare(__lhs) == 0; }

inline bool operator==(const string& __lhs, const char* __rhs)
{ return __lhs.compare(__rhs) == 0; }

inline bool operator!=(const string& __lhs, const string& __rhs)
{ return __rhs.compare(__lhs) != 0; }

inline bool operator!=(const char* __lhs, const string& __rhs)
{ return __rhs.compare(__lhs) != 0; }

inline bool operator!=(const string& __lhs, const char* __rhs)
{ return __lhs.compare(__rhs) != 0; }

} // namespace ledger

#else // ! VERIFY_ON

#define VERIFY(x)
#define DO_VERIFY() true
#define TRACE_CTOR(cls, args)
#define TRACE_DTOR(cls)

#endif // VERIFY_ON

extern ledger::string empty_string;

#define IF_VERIFY() if (DO_VERIFY())

/**********************************************************************
 *
 * Logging
 */

#if ! defined(NO_LOGGING)
#define LOGGING_ON 1
#endif
#if defined(LOGGING_ON)

namespace ledger {

enum log_level_t {
  LOG_OFF = 0,
  LOG_CRIT,
  LOG_FATAL,
  LOG_ASSERT,
  LOG_ERROR,
  LOG_VERIFY,
  LOG_WARN,
  LOG_INFO,
  LOG_EXCEPT,
  LOG_DEBUG,
  LOG_TRACE,
  LOG_ALL
};

extern log_level_t	  _log_level;
extern std::ostream *	  _log_stream;
extern std::ostringstream _log_buffer;

bool logger_func(log_level_t level);

#define LOGGER(cat) \
    static const char * const _this_category = cat

#if defined(TRACING_ON)

extern unsigned int _trace_level;

#define SHOW_TRACE(lvl) \
  (ledger::_log_level >= ledger::LOG_TRACE && lvl <= ledger::_trace_level)
#define TRACE(lvl, msg) \
  (SHOW_TRACE(lvl) ? \
   ((ledger::_log_buffer << msg), \
    ledger::logger_func(ledger::LOG_TRACE)) : false)

#else // TRACING_ON

#define SHOW_TRACE(lvl) false
#define TRACE(lvl, msg)

#endif // TRACING_ON

#if defined(DEBUG_ON)

extern optional<std::string> _log_category;

inline bool category_matches(const char * cat) {
  return _log_category && starts_with(cat, *_log_category);
}

#define SHOW_DEBUG(cat) \
  (ledger::_log_level >= ledger::LOG_DEBUG && ledger::category_matches(cat))
#define SHOW_DEBUG_() SHOW_DEBUG(_this_category)

#define DEBUG(cat, msg) \
  (SHOW_DEBUG(cat) ? \
   ((ledger::_log_buffer << msg), \
    ledger::logger_func(ledger::LOG_DEBUG)) : false)
#define DEBUG_(msg) DEBUG(_this_category, msg)

#else // DEBUG_ON

#define SHOW_DEBUG(cat) false
#define SHOW_DEBUG_()   false
#define DEBUG(cat, msg)
#define DEBUG_(msg)

#endif // DEBUG_ON

#define LOG_MACRO(level, msg) \
  (ledger::_log_level >= level ? \
   ((ledger::_log_buffer << msg), ledger::logger_func(level)) : false)

#define SHOW_INFO()     (ledger::_log_level >= ledger::LOG_INFO)
#define SHOW_WARN()     (ledger::_log_level >= ledger::LOG_WARN)
#define SHOW_ERROR()    (ledger::_log_level >= ledger::LOG_ERROR)
#define SHOW_FATAL()    (ledger::_log_level >= ledger::LOG_FATAL)
#define SHOW_CRITICAL() (ledger::_log_level >= ledger::LOG_CRIT)

#define INFO(msg)      LOG_MACRO(ledger::LOG_INFO, msg)
#define WARN(msg)      LOG_MACRO(ledger::LOG_WARN, msg)
#define ERROR(msg)     LOG_MACRO(ledger::LOG_ERROR, msg)
#define FATAL(msg)     LOG_MACRO(ledger::LOG_FATAL, msg)
#define CRITICAL(msg)  LOG_MACRO(ledger::LOG_CRIT, msg)
#define EXCEPTION(msg) LOG_MACRO(ledger::LOG_EXCEPT, msg)

} // namespace ledger

#else // ! LOGGING_ON

#define LOGGER(cat)

#define SHOW_TRACE(lvl) false
#define SHOW_DEBUG(cat) false
#define SHOW_DEBUG_()   false
#define SHOW_INFO()     false
#define SHOW_WARN()     false
#define SHOW_ERROR()    false
#define SHOW_FATAL()    false
#define SHOW_CRITICAL() false

#define TRACE(lvl, msg)
#define DEBUG(cat, msg)
#define DEBUG_(msg)
#define INFO(msg)
#define WARN(msg)
#define ERROR(msg)
#define FATAL(msg)
#define CRITICAL(msg)

#endif // LOGGING_ON

#define IF_TRACE(lvl) if (SHOW_TRACE(lvl))
#define IF_DEBUG(cat) if (SHOW_DEBUG(cat))
#define IF_DEBUG_()   if (SHOW_DEBUG_())
#define IF_INFO()     if (SHOW_INFO())
#define IF_WARN()     if (SHOW_WARN())
#define IF_ERROR()    if (SHOW_ERROR())
#define IF_FATAL()    if (SHOW_FATAL())
#define IF_CRITICAL() if (SHOW_CRITICAL())

/**********************************************************************
 *
 * Timers (allows log entries to specify cumulative time spent)
 */

#if defined(LOGGING_ON) && defined(TIMERS_ON)

namespace ledger {

void start_timer(const char * name, log_level_t lvl);
void stop_timer(const char * name);
void finish_timer(const char * name);

#if defined(TRACING_ON)
#define TRACE_START(name, lvl, msg) \
  (SHOW_TRACE(lvl) ? \
   ((ledger::_log_buffer << msg), \
    ledger::start_timer(#name, ledger::LOG_TRACE)) : ((void)0))
#define TRACE_STOP(name, lvl) \
  (SHOW_TRACE(lvl) ? ledger::stop_timer(#name) : ((void)0))
#define TRACE_FINISH(name, lvl) \
  (SHOW_TRACE(lvl) ? ledger::finish_timer(#name) : ((void)0))
#else
#define TRACE_START(name, lvl, msg)
#define TRACE_STOP(name)
#define TRACE_FINISH(name)
#endif

#if defined(DEBUG_ON)
#define DEBUG_START(name, cat, msg) \
  (SHOW_DEBUG(cat) ? \
   ((ledger::_log_buffer << msg), \
    ledger::start_timer(#name, ledger::LOG_DEBUG)) : ((void)0))
#define DEBUG_START_(name, msg) \
  DEBUG_START_(name, _this_category, msg)
#define DEBUG_STOP(name, cat) \
  (SHOW_DEBUG(cat) ? ledger::stop_timer(#name) : ((void)0))
#define DEBUG_STOP_(name) \
  DEBUG_STOP_(name, _this_category)
#define DEBUG_FINISH(name, cat) \
  (SHOW_DEBUG(cat) ? ledger::finish_timer(#name) : ((void)0))
#define DEBUG_FINISH_(name) \
  DEBUG_FINISH_(name, _this_category)
#else
#define DEBUG_START(name, cat, msg)
#define DEBUG_START_(name, msg)
#define DEBUG_STOP(name)
#define DEBUG_FINISH(name)
#endif

#define INFO_START(name, msg) \
  (SHOW_INFO() ? \
   ((ledger::_log_buffer << msg), \
    ledger::start_timer(#name, ledger::LOG_INFO)) : ((void)0))
#define INFO_STOP(name) \
  (SHOW_INFO() ? stop_timer(#name) : ((void)0))
#define INFO_FINISH(name) \
  (SHOW_INFO() ? finish_timer(#name) : ((void)0))

} // namespace ledger

#else // ! (LOGGING_ON && TIMERS_ON)

#define TRACE_START(lvl, msg, name)
#define TRACE_STOP(name)
#define TRACE_FINISH(name)

#define DEBUG_START(name, msg)
#define DEBUG_START_(name, cat, msg)
#define DEBUG_STOP(name)
#define DEBUG_FINISH(name)

#define INFO_START(name, msg)
#define INFO_STOP(name)
#define INFO_FINISH(name)

#endif // TIMERS_ON

/**********************************************************************
 *
 * Exception handling
 */

#include "error.h"

namespace ledger {

extern std::ostringstream _exc_buffer;

template <typename T>
inline void throw_func(const std::string& message) {
  _exc_buffer.str("");
  throw T(message);
}

#define throw_(cls, msg) \
  ((_exc_buffer << msg), throw_func<cls>(_exc_buffer.str()))

#if 0
inline void throw_unexpected_error(char c, char wanted) {
  if (c == -1) {
    if (wanted)
      throw new error(string("Missing '") + wanted + "'");
    else
      throw new error("Unexpected end of input");
  } else {
    if (wanted)
      throw new error(string("Invalid char '") + c +
		      "' (wanted '" + wanted + "')");
    else
      throw new error(string("Invalid char '") + c + "'");
  }
}
#else
inline void throw_unexpected_error(char, char) {
}
#endif

} // namespace ledger

/**********************************************************************
 *
 * Date/time support classes
 * General support for objects with "flags"
 * Support for scoped execution and variable restoration
 */

#include "times.h"
#include "flags.h"
#include "pushvar.h"

/**********************************************************************
 *
 * General utility functions
 */

#define foreach BOOST_FOREACH

namespace ledger {

template <typename T, typename U>
inline T& downcast(U& object) {
  return *polymorphic_downcast<T *>(&object);
}

path resolve_path(const path& pathname);

#ifdef HAVE_REALPATH
extern "C" char * realpath(const char *, char resolved_path[]);
#endif

inline const string& either_or(const string& first,
			       const string& second) {
  return first.empty() ? second : first;
}

} // namespace ledger

#endif // _UTILS_H
