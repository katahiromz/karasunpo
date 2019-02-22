//////////////////////////////////////////////////////////////////////////////
// stdafx.h
// Copyright (C) 2015 Katayama Hirofumi MZ. All Rights Reserved.
// See License.txt about licensing.
//////////////////////////////////////////////////////////////////////////////

// We must do it strictly!!!
#define STRICT 1

#ifdef __GNUC__
    // NOTE: g++'s sprintf_s is buggy (2015.09.19).
    #define sprintf_s sprintf
#endif

// the target versioning header
#include "targetver.h"

#ifdef _MSC_VER
    // for detecting memory leak (MSVC only)
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
#endif

// Windows headers
#include <windows.h>
#include <commctrl.h>
#include <dlgs.h>

// C runtime headers
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cmath>
#include <tchar.h>

// C++ headers
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <new>

// private headers
#include "karasunpo.h"

#ifdef _MSC_VER
    // for detecting memory leak (MSVC only)
    #define new ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

//////////////////////////////////////////////////////////////////////////////
