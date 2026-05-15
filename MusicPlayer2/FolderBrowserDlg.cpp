#include "stdafx.h"
#include "FolderBrowserDlg.h"
#include "MusicPlayer2.h"
#include <atlbase.h>
#include <shobjidl.h>


CFolderBrowserDlg::CFolderBrowserDlg(HWND hParent)
	: m_hParent{ hParent }
{
}


CFolderBrowserDlg::~CFolderBrowserDlg()
{
}

int CFolderBrowserDlg::DoModal()
{
    struct OleInitGuard
    {
        OleInitGuard() : hr(OleInitialize(nullptr)) {}
        ~OleInitGuard()
        {
            if (SUCCEEDED(hr))
                OleUninitialize();
        }
        HRESULT hr{};
    } ole_init_guard;

    CComPtr<IFileOpenDialog> file_dialog;
    HRESULT hr = file_dialog.CoCreateInstance(CLSID_FileOpenDialog);
    if (SUCCEEDED(hr) && file_dialog != nullptr)
    {
        DWORD options{};
        file_dialog->GetOptions(&options);
        file_dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
        if (!m_info.IsEmpty())
            file_dialog->SetTitle(m_info);

        hr = file_dialog->Show(m_hParent);
        if (SUCCEEDED(hr))
        {
            CComPtr<IShellItem> shell_item;
            hr = file_dialog->GetResult(&shell_item);
            if (SUCCEEDED(hr) && shell_item != nullptr)
            {
                PWSTR path{};
                hr = shell_item->GetDisplayName(SIGDN_FILESYSPATH, &path);
                if (SUCCEEDED(hr) && path != nullptr)
                {
                    m_path = path;
                    CoTaskMemFree(path);
                    return IDOK;
                }
                if (path != nullptr)
                    CoTaskMemFree(path);
            }
        }
        else if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
        {
            return IDCANCEL;
        }
    }

	TCHAR szPath[MAX_PATH];		//存放选择的目录路径
	CString str;

	BROWSEINFO bi;
	bi.hwndOwner = m_hParent;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = szPath;
	bi.lpszTitle = m_info;
	bi.ulFlags = 0;
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = 0;
	//弹出选择目录对话框
	browse:
	LPITEMIDLIST lp = SHBrowseForFolder(&bi);

	if (lp)
	{
		if (!SHGetPathFromIDList(lp, szPath))
		{
            static const wstring& info = theApp.m_str_table.LoadText(L"MSG_FOLDER_BROWSER_INVALID_DIR_WARNING");
            AfxMessageBox(info.c_str(), MB_ICONWARNING | MB_OK);
			goto browse;
		}
		m_path = szPath;
		return IDOK;
	}
	return IDCANCEL;
}
