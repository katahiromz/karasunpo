////////////////////////////////////////////////////////////////////////////
// Shareware_inl.hpp -- MZC3 shareware maker for Win32
// This file is part of MZC3.  See file "ReadMe.txt" and "License.txt".
////////////////////////////////////////////////////////////////////////////

MZC_INLINE void SwMakeStaticHyperlink(
    HWND hwndParent, UINT idCtrl, LPCTSTR pszURL/* = NULL*/)
{
    SwMakeStaticHyperlink(::GetDlgItem(hwndParent, idCtrl), pszURL);
}

MZC_INLINE DWORD SW_Shareware::GetTrialDays() const
{
    return m_dwTrialDays;
}

MZC_INLINE bool SW_Shareware::IsRegistered() const
{
    return m_status == SW_Shareware::REGD;
}

MZC_INLINE bool SW_Shareware::IsInTrial() const
{
    return m_status == SW_Shareware::IN_TRIAL ||
           m_status == SW_Shareware::IN_TRIAL_FIRST_TIME;
}

MZC_INLINE bool SW_Shareware::IsOutOfTrial() const
{
    return m_status == SW_Shareware::OUT_OF_TRIAL;
}

MZC_INLINE /*virtual*/ void SW_Shareware::OnTrialFirstTime(HWND hwndParent)
{
}

MZC_INLINE /*virtual*/ void SW_Shareware::OnTrial(HWND hwndParent)
{
    UrgeRegister(hwndParent);
}

MZC_INLINE /*virtual*/ bool SW_Shareware::OnOutOfTrial(HWND hwndParent)
{
    return UrgeRegister(hwndParent);
}

MZC_INLINE /*virtual*/ void
SW_Shareware::ThisCommandRequiresRegistering(HWND hwndParent)
{
    ShowErrorMessage(hwndParent, 32737);
}

MZC_INLINE /*virtual*/ void SW_Shareware::ShowErrorMessage(
    HWND hwndParent, UINT uStringID)
{
    SwCenterMessageBox(hwndParent,
        SwLoadStringDx2(m_hInstance, uStringID),
        NULL, MB_ICONERROR);
}

////////////////////////////////////////////////////////////////////////////
