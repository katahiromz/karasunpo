//////////////////////////////////////////////////////////////////////////////
// karasunpo.h --- karasunpo
// Copyright (C) 2015 Katayama Hirofumi MZ. All Rights Reserved.
// See License.txt about licensing.
//////////////////////////////////////////////////////////////////////////////

#ifndef KARASUNPO_H
#define KARASUNPO_H

// resource IDs
#include "resource.h"

// tstring
#ifdef UNICODE
    typedef std::wstring tstring;
#else
    typedef std::string tstring;
#endif

// NOTE: Digital Mars C/C++ Compiler doesn't define INT_PTR type likely.
#ifdef __DMC__
    #define INT_PTR BOOL
#endif

#include "imaio.h"
#include "mstr.hpp"
#include "katahiromz_pdfium.h"

//////////////////////////////////////////////////////////////////////////////

#endif  // ndef KARASUNPO_H
