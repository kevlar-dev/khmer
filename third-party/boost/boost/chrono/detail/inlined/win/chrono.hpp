//  win/chrono.cpp  --------------------------------------------------------------//

//  Copyright Beman Dawes 2008
//  Copyright 2009-2010 Vicente J. Botet Escriba

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//----------------------------------------------------------------------------//
//                                Windows                                     //
//----------------------------------------------------------------------------//
#ifndef BOOST_CHRONO_DETAIL_INLINED_WIN_CHRONO_HPP
#define BOOST_CHRONO_DETAIL_INLINED_WIN_CHRONO_HPP

#include <boost/detail/win/time.hpp>
#include <boost/detail/win/timers.hpp>
#include <boost/detail/win/GetLastError.hpp>

namespace pkgboost
{
namespace chrono
{
namespace chrono_detail
{

  BOOST_CHRONO_INLINE double get_nanosecs_per_tic() BOOST_NOEXCEPT
  {
      pkgboost::detail::win32::LARGE_INTEGER_ freq;
      if ( !pkgboost::detail::win32::QueryPerformanceFrequency( &freq ) )
          return 0.0L;
      return double(1000000000.0L / freq.QuadPart);
  }

}

  steady_clock::time_point steady_clock::now() BOOST_NOEXCEPT
  {
    static double nanosecs_per_tic = chrono_detail::get_nanosecs_per_tic();

    pkgboost::detail::win32::LARGE_INTEGER_ pcount;
    if ( (nanosecs_per_tic <= 0.0L) ||
            (!pkgboost::detail::win32::QueryPerformanceCounter( &pcount )) )
    {
      BOOST_ASSERT(0 && "Boost::Chrono - Internal Error");
      return steady_clock::time_point();
    }

    return steady_clock::time_point(steady_clock::duration(
      static_cast<steady_clock::rep>((nanosecs_per_tic) * pcount.QuadPart)));
  }


#if !defined BOOST_CHRONO_DONT_PROVIDE_HYBRID_ERROR_HANDLING
  steady_clock::time_point steady_clock::now( system::error_code & ec )
  {
    static double nanosecs_per_tic = chrono_detail::get_nanosecs_per_tic();

    pkgboost::detail::win32::LARGE_INTEGER_ pcount;
    if ( (nanosecs_per_tic <= 0.0L)
            || (!pkgboost::detail::win32::QueryPerformanceCounter( &pcount )) )
    {
        pkgboost::detail::win32::DWORD_ cause =
            ((nanosecs_per_tic <= 0.0L)
                    ? ERROR_NOT_SUPPORTED
                    : pkgboost::detail::win32::GetLastError());
        if (BOOST_CHRONO_IS_THROWS(ec)) {
            pkgboost::throw_exception(
                    system::system_error(
                            cause,
                            BOOST_CHRONO_SYSTEM_CATEGORY,
                            "chrono::steady_clock" ));
        }
        else
        {
            ec.assign( cause, BOOST_CHRONO_SYSTEM_CATEGORY );
            return steady_clock::time_point(duration(0));
        }
    }

    if (!BOOST_CHRONO_IS_THROWS(ec))
    {
        ec.clear();
    }
    return time_point(duration(
      static_cast<steady_clock::rep>(nanosecs_per_tic * pcount.QuadPart)));
  }
#endif

  BOOST_CHRONO_INLINE
  system_clock::time_point system_clock::now() BOOST_NOEXCEPT
  {
    pkgboost::detail::win32::FILETIME_ ft;
  #if defined(UNDER_CE)
    // Windows CE does not define GetSystemTimeAsFileTime so we do it in two steps.
    pkgboost::detail::win32::SYSTEMTIME_ st;
    pkgboost::detail::win32::GetSystemTime( &st );
    pkgboost::detail::win32::SystemTimeToFileTime( &st, &ft );
  #else
    pkgboost::detail::win32::GetSystemTimeAsFileTime( &ft );  // never fails
  #endif
    return system_clock::time_point(
      system_clock::duration(
        ((static_cast<__int64>( ft.dwHighDateTime ) << 32) | ft.dwLowDateTime)
       -116444736000000000LL
      )
    );
  }

#if !defined BOOST_CHRONO_DONT_PROVIDE_HYBRID_ERROR_HANDLING
  BOOST_CHRONO_INLINE
  system_clock::time_point system_clock::now( system::error_code & ec )
  {
    pkgboost::detail::win32::FILETIME_ ft;
  #if defined(UNDER_CE)
    // Windows CE does not define GetSystemTimeAsFileTime so we do it in two steps.
    pkgboost::detail::win32::SYSTEMTIME_ st;
    pkgboost::detail::win32::GetSystemTime( &st );
    pkgboost::detail::win32::SystemTimeToFileTime( &st, &ft );
  #else
    pkgboost::detail::win32::GetSystemTimeAsFileTime( &ft );  // never fails
  #endif
    if (!BOOST_CHRONO_IS_THROWS(ec))
    {
        ec.clear();
    }
    return time_point(duration(
      (static_cast<__int64>( ft.dwHighDateTime ) << 32) | ft.dwLowDateTime));
  }
#endif

  BOOST_CHRONO_INLINE
  std::time_t system_clock::to_time_t(const system_clock::time_point& t) BOOST_NOEXCEPT
  {
      __int64 temp = t.time_since_epoch().count();

      temp /= 10000000;
      return static_cast<std::time_t>( temp );
  }

  BOOST_CHRONO_INLINE
  system_clock::time_point system_clock::from_time_t(std::time_t t) BOOST_NOEXCEPT
  {
      __int64 temp = t;
      temp *= 10000000;

      return time_point(duration(temp));
  }

}  // namespace chrono
}  // namespace pkgboost

#endif
