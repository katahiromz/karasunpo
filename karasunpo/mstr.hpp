// mstr.hpp --- C++ string manipulation library
// Copyright (C) 2019 Katayama Hirofumi MZ <katayama.hirofumi.mz@gmail.com>.
// This file is public domain software.

#ifndef MSTR_HPP_
#define MSTR_HPP_   13  // Version 13

#include <string>       // for std::string and std::wstring
#include <vector>       // for std::vector
#include <algorithm>    // for std::reverse
#include <cstdio>       // for std::printf
#include <cassert>      // for assert
#ifdef _WIN32
    #include <mbstring.h>   // for _mbsrchr
#endif

//////////////////////////////////////////////////////////////////////////////
// C-style functions

template <typename T_CHAR>
inline size_t mstrlen(const T_CHAR *str)
{
    return std::char_traits<T_CHAR>::length(str);
}

template <typename T_CHAR, size_t siz>
inline T_CHAR *mstrcpy(T_CHAR (&array)[siz], const T_CHAR *src)
{
    std::char_traits<T_CHAR>::copy(array, src, mstrlen(src) + 1);
    return array;
}

template <typename T_CHAR, size_t siz>
inline T_CHAR *mstrcpyn(T_CHAR (&array)[siz], const T_CHAR *src, size_t maxbuf)
{
    size_t len = mstrlen(src) + 1;
    if (len >= maxbuf)
        len = maxbuf;
    std::char_traits<T_CHAR>::copy(array, src, len);
    if (maxbuf)
        array[maxbuf - 1] = 0;
    return array;
}

template <typename T_CHAR>
inline T_CHAR *mstrrchr(T_CHAR *str, T_CHAR ch)
{
#if defined(_WIN32) && defined(_MBCS)
    if (sizeof(T_CHAR) == 1)
    {
        return (T_CHAR *)(_mbsrchr((unsigned char *)str, ch));
    }
#endif
    T_CHAR *ptr = NULL;
    assert(str != NULL);
    while (*str)
    {
        if (*str == ch)
            ptr = str;
        ++str;
    }
    return ptr;
}

template <typename T_CHAR>
inline const T_CHAR *mstrrchr(const T_CHAR *str, T_CHAR ch)
{
#if defined(_WIN32) && defined(_MBCS)
    if (sizeof(T_CHAR) == 1)
    {
        return (const T_CHAR *)(_mbsrchr((const unsigned char *)str, ch));
    }
#endif
    const T_CHAR *ptr = NULL;
    assert(str != NULL);
    while (*str)
    {
        if (*str == ch)
            ptr = str;
        ++str;
    }
    return ptr;
}

//////////////////////////////////////////////////////////////////////////////
// split and join

template <typename T_STR_CONTAINER>
inline size_t
mstr_split(T_STR_CONTAINER& container,
           const typename T_STR_CONTAINER::value_type& str,
           const typename T_STR_CONTAINER::value_type& sep)
{
    container.clear();
#ifndef MSTR_SPLIT_NO_SPECIAL_HANDLING
    if (sep.empty())
    {
        for (size_t i = 0; i < str.size(); ++i)
        {
            std::string t(1, str[i]);
            container.push_back(t);
        }
    }
    else
#endif
    {
        size_t i = 0, k = str.find(sep);
        while (k != T_STR_CONTAINER::value_type::npos)
        {
            container.push_back(str.substr(i, k - i));
            i = k + sep.size();
            k = str.find(sep, i);
        }
        container.push_back(str.substr(i));
    }
    return container.size();
}

template <typename T_STR_CONTAINER>
inline typename T_STR_CONTAINER::value_type
mstr_join(const T_STR_CONTAINER& container,
          const typename T_STR_CONTAINER::value_type& sep)
{
    typename T_STR_CONTAINER::value_type result;
    typename T_STR_CONTAINER::const_iterator it, end;
    it = container.begin();
    end = container.end();
    if (it != end)
    {
        result = *it;
        for (++it; it != end; ++it)
        {
            result += sep;
            result += *it;
        }
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////////
// replace

template <typename T_STR>
inline size_t
mstr_replace(T_STR& str, const T_STR& from, const T_STR& to)
{
    size_t ret = 0, i = 0;
    for (;;)
    {
        i = str.find(from, i);
        if (i == T_STR::npos)
            break;
        ++ret;
        str.replace(i, from.size(), to);
        i += to.size();
    }
    return ret;
}

template <typename T_STR>
inline size_t
mstr_replace(T_STR& str,
             const typename T_STR::value_type *from,
             const typename T_STR::value_type *to)
{
    return mstr_replace(str, T_STR(from), T_STR(to));
}

//////////////////////////////////////////////////////////////////////////////
// trim

template <typename T_CHAR>
inline void
mstr_trim(std::basic_string<T_CHAR>& str, const std::basic_string<T_CHAR>& spaces)
{
    typedef std::basic_string<T_CHAR> string_type;
    size_t i = str.find_first_not_of(spaces);
    size_t j = str.find_last_not_of(spaces);
    if ((i != string_type::npos) && (j != string_type::npos))
        str = str.substr(i, j - i + 1);
    else
        str.clear();
}

template <typename T_CHAR>
inline void
mstr_trim(std::basic_string<T_CHAR>& str, const T_CHAR *spaces)
{
    mstr_trim(str, std::basic_string<T_CHAR>(spaces));
}

template <typename T_CHAR>
inline void
mstr_trim_left(std::basic_string<T_CHAR>& str, const std::basic_string<T_CHAR>& spaces)
{
    typedef std::basic_string<T_CHAR> string_type;
    size_t i = str.find_first_not_of(spaces);
    if (i != string_type::npos)
        str = str.substr(i);
    else
        str.clear();
}

template <typename T_CHAR>
inline void
mstr_trim_left(std::basic_string<T_CHAR>& str, const T_CHAR *spaces)
{
    mstr_trim_left(str, std::basic_string<T_CHAR>(spaces));
}

template <typename T_CHAR>
inline void
mstr_trim_right(std::basic_string<T_CHAR>& str, const std::basic_string<T_CHAR>& spaces)
{
    typedef std::basic_string<T_CHAR> string_type;
    size_t j = str.find_last_not_of(spaces);
    if (j != string_type::npos)
        str = str.substr(0, j + 1);
    else
        str.clear();
}

template <typename T_CHAR>
inline void
mstr_trim_right(std::basic_string<T_CHAR>& str, const T_CHAR *spaces)
{
    mstr_trim_right(str, std::basic_string<T_CHAR>(spaces));
}

//////////////////////////////////////////////////////////////////////////////
// escape and quote

template <typename T_STR>
inline void mstr_oct_ch(T_STR& str, unsigned char ch)
{
    typedef typename T_STR::value_type T_CHAR;
    str.clear();
    str += T_CHAR('0' + (ch & 7));
    ch >>= 7;
    str += T_CHAR('0' + (ch & 7));
    ch >>= 7;
    str += T_CHAR('0' + (ch & 7));
    std::reverse(str.begin(), str.end());
}

template <typename T_STR>
inline void mstr_hex_ch(T_STR& str, typename T_STR::value_type ch,
                        size_t len = sizeof(typename T_STR::value_type) * 2)
{
    typedef typename T_STR::value_type T_CHAR;
    T_CHAR value;
    str.clear();
    str.reserve(len);

    for (size_t i = 0; i < len; ++i)
    {
        value = (ch & 0xF);
        if (value >= 10)
            str += T_CHAR('A' + value - 10);
        else
            str += T_CHAR('0' + value);

        if (sizeof(T_CHAR) > 1)
            ch >>= 8;
    }

    std::reverse(str.begin(), str.end());
}

template <typename T_STR>
inline void
mstr_escape(T_STR& ret, const T_STR& src)
{
    typedef typename T_STR::value_type T_CHAR;
    ret.clear();
    ret.reserve(src.size());
    for (size_t i = 0; i < src.size(); ++i)
    {
        T_CHAR ch = src[i];
        switch (ch)
        {
        case T_CHAR('\"'): ret += T_CHAR('\"'); ret += T_CHAR('\"'); break;
        case T_CHAR('\\'): ret += T_CHAR('\\'); ret += T_CHAR('\\'); break;
        case T_CHAR('\0'): ret += T_CHAR('\\'); ret += T_CHAR('0'); break;
        case T_CHAR('\a'): ret += T_CHAR('\\'); ret += T_CHAR('a'); break;
        case T_CHAR('\b'): ret += T_CHAR('\\'); ret += T_CHAR('b'); break;
        case T_CHAR('\f'): ret += T_CHAR('\\'); ret += T_CHAR('f'); break;
        case T_CHAR('\n'): ret += T_CHAR('\\'); ret += T_CHAR('n'); break;
        case T_CHAR('\r'): ret += T_CHAR('\\'); ret += T_CHAR('r'); break;
        case T_CHAR('\t'): ret += T_CHAR('\\'); ret += T_CHAR('t'); break;
        case T_CHAR('\v'): ret += T_CHAR('\\'); ret += T_CHAR('v'); break;
        default:
            if (ch < 0x20
#ifdef MSTR_ESCAPE_NON_ASCII
                || ch >= 0x7F
#endif
            )
            {
                if (sizeof(T_CHAR) == 1)
                {
                    T_STR str;
                    mstr_oct_ch(str, (char)ch);
                    ret += T_CHAR('\\');
                    ret += str;
                }
                else if (sizeof(T_CHAR) == 2)
                {
                    ret += T_CHAR('\\');
                    ret += T_CHAR('u');
                    T_STR str;
                    mstr_hex_ch(str, ch);
                    ret += str;
                }
                else if (sizeof(T_CHAR) == 4)
                {
                    ret += T_CHAR('\\');
                    ret += T_CHAR('U');
                    T_STR str;
                    mstr_hex_ch(str, ch);
                    ret += str;
                }
                else
                {
                    assert(0);
                    ret += T_CHAR(ch);
                }
            }
            else
            {
                ret += T_CHAR(ch);
            }
        }
    }
}

template <typename T_CHAR>
inline std::basic_string<T_CHAR>
mstr_escape(const std::basic_string<T_CHAR>& src)
{
    std::basic_string<T_CHAR> ret;
    mstr_escape(ret, src);
    return ret;
}

template <typename T_CHAR>
inline std::basic_string<T_CHAR>
mstr_escape(const T_CHAR *src)
{
    typedef typename std::basic_string<T_CHAR> T_STR;
    T_STR ret;
    mstr_escape(ret, T_STR(src));
    return ret;
}

template <typename T_STR>
inline void
mstr_quote(T_STR& dest, const T_STR& str)
{
    typedef typename T_STR::value_type T_CHAR;
    dest.clear();
    dest += T_CHAR('\"');
    dest += mstr_escape(str);
    dest += T_CHAR('\"');
}

template <typename T_CHAR>
inline std::basic_string<T_CHAR>
mstr_quote(const std::basic_string<T_CHAR>& src)
{
    std::basic_string<T_CHAR> ret;
    mstr_quote(ret, src);
    return ret;
}

template <typename T_CHAR>
inline std::basic_string<T_CHAR>
mstr_quote(const T_CHAR *src)
{
    typedef typename std::basic_string<T_CHAR> T_STR;
    T_STR ret;
    mstr_quote(ret, T_STR(src));
    return ret;
}

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef MSTR_HPP_
