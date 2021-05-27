////////////////////////////////////////////////////////////////////////////
// Shareware.hpp -- MZC3 shareware maker for Win32
// This file is part of MZC3.  See file "ReadMe.txt" and "License.txt".
////////////////////////////////////////////////////////////////////////////

#ifndef __MZC3_SHAREWARE_HPP__
#define __MZC3_SHAREWARE_HPP__
#ifndef MZC_NO_SHAREWARE

#include <string>

////////////////////////////////////////////////////////////////////////////

#define sw_shareware_max_password 256

////////////////////////////////////////////////////////////////////////////
// useful functions

LPTSTR SwLoadStringDx1(HINSTANCE hInstance, UINT uID);
LPTSTR SwLoadStringDx2(HINSTANCE hInstance, UINT uID);
void SwCenterDialog(HWND hwnd);
int SwCenterMessageBox(
    HWND hwndParent, LPCTSTR pszText, LPCTSTR pszCaption, UINT uMB_);
void SwMakeStaticHyperlink(HWND hwndCtrl, LPCTSTR pszURL = NULL);
void SwMakeStaticHyperlink(
    HWND hwndParent, UINT idCtrl, LPCTSTR pszURL = NULL);

////////////////////////////////////////////////////////////////////////////

#ifndef EXTENDS_MOBJECT
    #define EXTENDS_MOBJECT
#endif

class SW_Shareware EXTENDS_MOBJECT
{
public:
    // NOTE: pszCompanyKey is the name of the registry key of the company.
    // NOTE: pszAppKey is the name of the registry key of the application.
    // NOTE: dwTrialDays is the trial interval in days.
    // NOTE: parameter salt is the salt string.
    // NOTE: parameter new_version is the current version of this software.
    SW_Shareware(LPCTSTR pszCompanyKey,
                 LPCTSTR pszAppKey,
                 const char *pszSha256HashHexString,
                 DWORD dwTrialDays = 15,
                 const char *salt = "",
                 const char *new_version = "");
    SW_Shareware(LPCTSTR pszCompanyKey,
                 LPCTSTR pszAppKey,
                 const BYTE *pbHash32Bytes,
                 DWORD dwTrialDays = 15,
                 const char *salt = "",
                 const char *new_version = "");
    virtual ~SW_Shareware();

    // NOTE: SW_Shareware::Start must be called on start-up of the MZC3 shareware.
    // NOTE: SW_Shareware::Start returns false if the application cannot be used.
    virtual bool Start(HWND hwndParent = NULL);

    bool IsRegistered() const;
    bool IsInTrial() const;
    bool IsOutOfTrial() const;

    DWORD GetTrialDays() const;
    bool IsPasswordValid(const char *pszPassword) const;

    // NOTE: UrgeRegister show a dialog and returns true if registered.
    virtual bool UrgeRegister(HWND hwndParent = NULL);
    bool RegisterPassword(HWND hwndParent, const char *pszPassword);
    virtual void ShowErrorMessage(HWND hwndParent, UINT uStringID);
    virtual void ThisCommandRequiresRegistering(HWND hwndParent);
    bool CheckDate();

    virtual int CompareVersion(const char *old_ver, const char *new_ver);

public:
    HINSTANCE           m_hInstance;
    DWORDLONG           m_dwlTotalMinutesRemains;
    DWORD               m_dwTrialDays;
    FILETIME            m_ftStart;

    enum SHAREWARE_STATUS
    {
        IN_TRIAL_FIRST_TIME, IN_TRIAL, OUT_OF_TRIAL, REGD
    };
    SHAREWARE_STATUS    m_status;

public:
    #ifdef UNICODE
        typedef std::wstring tstring;
    #else
        typedef std::string tstring;
    #endif

protected:
    tstring             m_strCompanyKey;
    tstring             m_strAppKey;
    std::string         m_strSha256HashHexString;
    std::string         m_strSalt;
    std::string         m_strNewVersion;
    std::string         m_strOldVersion;

    bool CheckRegistry(HWND hwndParent);
    bool CheckAppKey(HWND hwndParent, HKEY hkeyApp);

    bool SetRegistryFirstTime(HWND hwndParent);

    DWORD GetUserCheckSum() const;
    virtual void OnTrialFirstTime(HWND hwndParent);
    virtual void OnTrial(HWND hwndParent);
    virtual bool OnOutOfTrial(HWND hwndParent);
    virtual void EncodePassword(void *pass, DWORD size) const;
    virtual void DecodePassword(void *pass, DWORD size) const;

private:
    SW_Shareware();
    // NOTE: SW_Shareware is not copyable.
    SW_Shareware(const SW_Shareware&);
    SW_Shareware& operator=(const SW_Shareware&);
};

////////////////////////////////////////////////////////////////////////////

#ifndef MZC_NO_INLINING
    #undef MZC_INLINE
    #define MZC_INLINE inline
    #include "Shareware_inl.hpp"
#endif

#endif  // ndef MZC_NO_SHAREWARE
#endif  // ndef __MZC3_SHAREWARE_HPP__
