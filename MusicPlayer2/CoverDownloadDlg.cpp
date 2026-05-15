// CoverDownloadDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "MusicPlayer2.h"
#include "Player.h"
#include "CoverDownloadDlg.h"
#include "SongDataManager.h"
#include "FilterHelper.h"
#include "CoverPreviewDlg.h"
#include "DrawCommon.h"
#include "AudioTag.h"
#include "PropertyDlgHelper.h"
#include "NeteaseLyricDownload.h"
#include "QQMusicLyricDownload.h"

namespace
{
    bool LoadImageFromMemory(const vector<unsigned char>& image_contents, CImage& image)
    {
        if (image_contents.empty())
            return false;

        HGLOBAL hBuffer = ::GlobalAlloc(GMEM_MOVEABLE, image_contents.size());
        if (hBuffer == NULL)
            return false;

        void* buffer = ::GlobalLock(hBuffer);
        if (buffer == nullptr)
        {
            ::GlobalFree(hBuffer);
            return false;
        }

        ::CopyMemory(buffer, image_contents.data(), image_contents.size());
        ::GlobalUnlock(hBuffer);

        IStream* pStream = nullptr;
        if (FAILED(::CreateStreamOnHGlobal(hBuffer, TRUE, &pStream)))
        {
            ::GlobalFree(hBuffer);
            return false;
        }

        image.Destroy();
        HRESULT hr = image.Load(pStream);
        pStream->Release();
        if (FAILED(hr))
        {
            image.Destroy();
            return false;
        }
        return !image.IsNull();
    }

    vector<unsigned char> StringToBytes(const string& data)
    {
        return vector<unsigned char>(data.begin(), data.end());
    }

    string BytesToString(const vector<unsigned char>& data)
    {
        return string(reinterpret_cast<const char*>(data.data()), data.size());
    }

    wstring ImageTypeToExtension(int image_type)
    {
        switch (image_type)
        {
        case 1: return L"png";
        case 2: return L"gif";
        default: return L"jpg";
        }
    }

    bool DownloadUrlToMemory(const wstring& url, vector<unsigned char>& data)
    {
        data.clear();
        CInternetSession session;
        CStdioFile* pFile{};
        try
        {
            pFile = session.OpenURL(url.c_str(), 1, INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE);
            BYTE buffer[8192]{};
            UINT read_size{};
            while ((read_size = pFile->Read(buffer, sizeof(buffer))) > 0)
                data.insert(data.end(), buffer, buffer + read_size);
            pFile->Close();
            delete pFile;
            session.Close();
            return !data.empty();
        }
        catch (CInternetException* e)
        {
            if (pFile != nullptr)
            {
                pFile->Close();
                delete pFile;
            }
            session.Close();
            e->Delete();
            data.clear();
            return false;
        }
    }
}


// CCoverDownloadDlg 对话框

IMPLEMENT_DYNAMIC(CCoverDownloadDlg, CBaseDialog)

CCoverDownloadDlg::CCoverDownloadDlg(CWnd* pParent /*=nullptr*/)
    : CBaseDialog(IDD_COVER_DOWNLOAD_DIALOG, pParent)
{

}

CCoverDownloadDlg::~CCoverDownloadDlg()
{
}

UINT CCoverDownloadDlg::SongSearchThreadFunc(LPVOID lpParam)
{
    CCommon::SetThreadLanguageList(theApp.m_str_table.GetLanguageTag());
    CCoverDownloadDlg* pThis = (CCoverDownloadDlg*)lpParam;
    wstring search_result;
    wstring m_search_url = pThis->m_search_url;
    int m_search_rtn = pThis->GetDownloadService()->RequestSearch(m_search_url, search_result);     //发送歌曲搜索请求
    if (theApp.m_cover_download_dialog_exit)
        return 0;
    pThis->m_search_rtn = m_search_rtn;
    pThis->m_search_result = search_result;
    ::PostMessage(pThis->m_hWnd, WM_SEARCH_COMPLATE, 0, 0);		//搜索完成后发送一个搜索完成的消息
    return 0;
}

void CCoverDownloadDlg::SetID(wstring id)
{
    SongInfo song_info_ori{ CSongDataManager::GetInstance().GetSongInfo3(m_song) };
    song_info_ori.SetSongId(id);
    CSongDataManager::GetInstance().AddItem(song_info_ori);
}

SongInfo CCoverDownloadDlg::GetSongInfo() const
{
    return CPlayer::GetInstance().GetCurrentSongInfo();
}

CString CCoverDownloadDlg::GetDialogName() const
{
    return _T("CoverDownloadDlg");
}

bool CCoverDownloadDlg::InitializeControls()
{
    SetIcon(IconMgr::IconType::IT_Album_Cover, FALSE);
    wstring temp;
    temp = theApp.m_str_table.LoadText(L"TITLE_COVER_DL");
    SetWindowTextW(temp.c_str());
    SetDlgItemTextW(IDC_CURRENT_COVER_STATIC, theApp.m_str_table.LoadText(L"TXT_COVER_DL_CURRENT_COVER").c_str());
    SetDlgItemTextW(IDC_ONLINE_COVER_STATIC, theApp.m_str_table.LoadText(L"TXT_COVER_DL_ONLINE_PREVIEW").c_str());
    temp = theApp.m_str_table.LoadText(L"TXT_LYRIC_DL_TITLE");
    SetDlgItemTextW(IDC_TXT_COVER_DL_TITLE_STATIC, temp.c_str());
    // IDC_TITLE_EDIT
    temp = theApp.m_str_table.LoadText(L"TXT_LYRIC_DL_SEARCH");
    SetDlgItemTextW(IDC_SEARCH_BUTTON, temp.c_str());
    temp = theApp.m_str_table.LoadText(L"TXT_LYRIC_DL_ARTIST");
    SetDlgItemTextW(IDC_TXT_COVER_DL_ARTIST_STATIC, temp.c_str());
    // IDC_ARTIST_EDIT
    temp = theApp.m_str_table.LoadText(L"TXT_LYRIC_DL_INFO");
    SetDlgItemTextW(IDC_STATIC_INFO, temp.c_str());
    temp = L"<a>" + theApp.m_str_table.LoadText(L"TXT_LYRIC_DL_UNLINK") + L"</a>";
    SetDlgItemTextW(IDC_UNASSOCIATE_LINK, temp.c_str());
    SetDlgItemTextW(IDC_COVER_DL_SERVICE_STATIC, theApp.m_str_table.LoadText(L"TXT_OPT_DATA_LYRICS_AND_COVER_DL_SERVICE").c_str());
    SetDlgItemTextW(IDC_COVER_DL_NETEASE_RADIO, theApp.m_str_table.LoadText(L"TXT_OPT_DATA_NETEASE_CLOUD_MUSIC").c_str());
    SetDlgItemTextW(IDC_COVER_DL_QQMUSIC_RADIO, theApp.m_str_table.LoadText(L"TXT_OPT_DATA_QQ_MUSIC").c_str());
    // IDC_COVER_DOWN_LIST
    temp = theApp.m_str_table.LoadText(L"TXT_LYRIC_DL_OPT");
    SetDlgItemTextW(IDC_DOWNLOAD_OPTION_GROUPBOX, temp.c_str());
    temp = theApp.m_str_table.LoadText(L"TXT_COVER_DL_LOCATION_SEL");
    SetDlgItemTextW(IDC_COVER_LOCATION_STATIC, temp.c_str());
    temp = theApp.m_str_table.LoadText(L"TXT_COVER_DL_LOCATION_FOLDER_SONG");
    SetDlgItemTextW(IDC_SAVE_TO_SONG_FOLDER2, temp.c_str());
    temp = theApp.m_str_table.LoadText(L"TXT_COVER_DL_LOCATION_FOLDER_COVER");
    SetDlgItemTextW(IDC_SAVE_TO_ALBUM_FOLDER2, temp.c_str());
    temp = theApp.m_str_table.LoadText(L"TXT_COVER_DL_SAVE_TO_AUDIO_FILE");
    SetDlgItemTextW(IDC_DOWNLOAD_SELECTED, temp.c_str());

    SetButtonIcon(IDC_SEARCH_BUTTON, IconMgr::IconType::IT_Find);
    SetButtonIcon(IDC_DOWNLOAD_SELECTED, IconMgr::IconType::IT_Save);

    RepositionTextBasedControls({
        { CtrlTextInfo::L1, IDC_TXT_COVER_DL_TITLE_STATIC },
        { CtrlTextInfo::C0, IDC_TITLE_EDIT },
        { CtrlTextInfo::L1, IDC_TXT_COVER_DL_ARTIST_STATIC },
        { CtrlTextInfo::C0, IDC_ARTIST_EDIT },
        { CtrlTextInfo::R1, IDC_SEARCH_BUTTON, CtrlTextInfo::W16 }
        }, CtrlTextInfo::W64);
    RepositionTextBasedControls({
        { CtrlTextInfo::C0, IDC_STATIC_INFO },
        { CtrlTextInfo::R1, IDC_UNASSOCIATE_LINK }
        }, CtrlTextInfo::W128);
    RepositionTextBasedControls({
        { CtrlTextInfo::R1, IDC_DOWNLOAD_SELECTED, CtrlTextInfo::W32 },
        { CtrlTextInfo::R2, IDCANCEL, CtrlTextInfo::W32 }
        });
    return true;
}

void CCoverDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
    CBaseDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COVER_DOWN_LIST, m_down_list_ctrl);
    DDX_Control(pDX, IDC_UNASSOCIATE_LINK, m_unassciate_lnk);
}

void CCoverDownloadDlg::ShowDownloadList()
{
    m_down_list_ctrl.DeleteAllItems();
    for (size_t i{}; i < m_down_list.size(); i++)
    {
        CString tmp;
        tmp.Format(_T("%d"), i + 1);
        m_down_list_ctrl.InsertItem(i, tmp);
        m_down_list_ctrl.SetItemText(i, 1, m_down_list[i].title.c_str());
        m_down_list_ctrl.SetItemText(i, 2, m_down_list[i].artist.c_str());
        m_down_list_ctrl.SetItemText(i, 3, m_down_list[i].album.c_str());
        m_down_list_ctrl.SetItemText(i, 4, CPlayTime(m_down_list[i].duration).toString().c_str());
    }
}

bool CCoverDownloadDlg::IsItemSelectedValid() const
{
    return (m_item_selected >= 0 && m_item_selected < static_cast<int>(m_down_list.size()));
}

CLyricDownloadCommon* CCoverDownloadDlg::GetDownloadService()
{
    if (m_dialog_download_service == GeneralSettingData::LDS_QQMUSIC)
        return m_qqmusic_download.get();
    return m_netease_download.get();
}

void CCoverDownloadDlg::LoadCurrentCoverPreview()
{
    if (!m_current_cover_img.IsNull())
        m_current_cover_img.Destroy();

    int image_type{};
    CAudioTag audio_tag(m_song.file_path);
    string cover_data{ audio_tag.GetAlbumCoverData(image_type) };
    LoadImageFromMemory(StringToBytes(cover_data), m_current_cover_img);
}

void CCoverDownloadDlg::LoadOnlineCoverPreview(const vector<unsigned char>& cover_data, const wstring& ext)
{
    if (!m_online_cover_img.IsNull())
        m_online_cover_img.Destroy();
    m_online_cover_data = cover_data;
    m_online_cover_ext = ext;
    LoadImageFromMemory(m_online_cover_data, m_online_cover_img);
    UpdateSaveButtonState();
    InvalidateRect(m_online_cover_rect);
}

void CCoverDownloadDlg::DrawCoverPreview(CDC& dc, const CRect& rect, CImage& image)
{
    dc.FillSolidRect(rect, RGB(245, 245, 245));
    dc.Draw3dRect(rect, RGB(180, 180, 180), RGB(180, 180, 180));
    if (!image.IsNull())
    {
        CRect image_rect{ rect };
        image_rect.DeflateRect(theApp.DPI(1), theApp.DPI(1));
        CDrawCommon draw;
        draw.Create(&dc);
        draw.DrawImage(image, image_rect.TopLeft(), image_rect.Size(), CDrawCommon::StretchMode::FIT);
    }
}

void CCoverDownloadDlg::UpdatePreviewRects()
{
    CWnd* pCurrent = GetDlgItem(IDC_CURRENT_COVER_PREVIEW);
    if (pCurrent != nullptr)
    {
        pCurrent->GetWindowRect(m_current_cover_rect);
        ScreenToClient(m_current_cover_rect);
    }
    CWnd* pOnline = GetDlgItem(IDC_ONLINE_COVER_PREVIEW);
    if (pOnline != nullptr)
    {
        pOnline->GetWindowRect(m_online_cover_rect);
        ScreenToClient(m_online_cover_rect);
    }
}

void CCoverDownloadDlg::UpdateSaveButtonState()
{
    CWnd* pSaveButton = GetDlgItem(IDC_DOWNLOAD_SELECTED);
    if (pSaveButton != nullptr)
        pSaveButton->EnableWindow(!m_online_cover_data.empty());
}

wstring CCoverDownloadDlg::GetCoverCacheKey(const wstring& song_id) const
{
    return std::to_wstring(static_cast<int>(m_dialog_download_service)) + L':' + song_id;
}

bool CCoverDownloadDlg::DownloadSelectedCoverToPreview()
{
    if (!IsItemSelectedValid())
        return false;

    wstring song_id{ m_down_list[m_item_selected].id };
    if (song_id.empty())
        return false;

    CWaitCursor wait_cursor;
    wstring cache_key{ GetCoverCacheKey(song_id) };
    auto cache_iter = m_cover_cache.find(cache_key);
    if (cache_iter != m_cover_cache.end())
    {
        wstring ext{ L"jpg" };
        auto ext_iter = m_cover_cache_ext.find(cache_key);
        if (ext_iter != m_cover_cache_ext.end() && !ext_iter->second.empty())
            ext = ext_iter->second;
        LoadOnlineCoverPreview(cache_iter->second, ext);
        return true;
    }

    wstring cover_url{ GetDownloadService()->GetAlbumCoverURL(song_id) };
    if (cover_url.empty())
    {
        MessageBox(theApp.m_str_table.LoadText(L"MSG_NETWORK_COVER_DOWNLOAD_FAILED").c_str(), NULL, MB_ICONWARNING | MB_OK);
        LoadOnlineCoverPreview(vector<unsigned char>(), wstring());
        return false;
    }

    vector<unsigned char> data;
    if (!DownloadUrlToMemory(cover_url, data))
    {
        MessageBox(theApp.m_str_table.LoadText(L"MSG_NETWORK_COVER_DOWNLOAD_FAILED").c_str(), NULL, MB_ICONWARNING | MB_OK);
        LoadOnlineCoverPreview(vector<unsigned char>(), wstring());
        return false;
    }

    wstring ext{ CFilePathHelper(cover_url).GetFileExtension() };
    if (ext.empty())
        ext = L"jpg";
    m_cover_cache[cache_key] = data;
    m_cover_cache_ext[cache_key] = ext;
    LoadOnlineCoverPreview(data, ext);
    return true;
}


BEGIN_MESSAGE_MAP(CCoverDownloadDlg, CBaseDialog)
    ON_BN_CLICKED(IDC_SEARCH_BUTTON, &CCoverDownloadDlg::OnBnClickedSearchButton)
    ON_MESSAGE(WM_SEARCH_COMPLATE, &CCoverDownloadDlg::OnSearchComplate)
    ON_BN_CLICKED(IDC_DOWNLOAD_SELECTED, &CCoverDownloadDlg::OnBnClickedDownloadSelected)
    ON_NOTIFY(NM_CLICK, IDC_COVER_DOWN_LIST, &CCoverDownloadDlg::OnNMClickCoverDownList)
    ON_NOTIFY(NM_DBLCLK, IDC_COVER_DOWN_LIST, &CCoverDownloadDlg::OnNMDblclkCoverDownList)
    ON_NOTIFY(NM_RCLICK, IDC_COVER_DOWN_LIST, &CCoverDownloadDlg::OnNMRClickCoverDownList)
    ON_EN_CHANGE(IDC_TITLE_EDIT, &CCoverDownloadDlg::OnEnChangeTitleEdit)
    ON_EN_CHANGE(IDC_ARTIST_EDIT, &CCoverDownloadDlg::OnEnChangeArtistEdit)
    ON_NOTIFY(NM_CLICK, IDC_UNASSOCIATE_LINK, &CCoverDownloadDlg::OnNMClickUnassociateLink)
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_SAVE_TO_SONG_FOLDER2, &CCoverDownloadDlg::OnBnClickedSaveToSongFolder2)
    ON_BN_CLICKED(IDC_SAVE_TO_ALBUM_FOLDER2, &CCoverDownloadDlg::OnBnClickedSaveToAlbumFolder2)
    ON_COMMAND(ID_LD_LYRIC_DOWNLOAD, &CCoverDownloadDlg::OnLdCoverDownload)
    ON_COMMAND(ID_LD_LYRIC_SAVEAS, &CCoverDownloadDlg::OnLdCoverSaveas)
    ON_COMMAND(ID_LD_COPY_TITLE, &CCoverDownloadDlg::OnLdCopyTitle)
    ON_COMMAND(ID_LD_COPY_ARTIST, &CCoverDownloadDlg::OnLdCopyArtist)
    ON_COMMAND(ID_LD_COPY_ALBUM, &CCoverDownloadDlg::OnLdCopyAlbum)
    ON_COMMAND(ID_LD_COPY_ID, &CCoverDownloadDlg::OnLdCopyId)
    ON_COMMAND(ID_LD_VIEW_ONLINE, &CCoverDownloadDlg::OnLdViewOnline)
    ON_COMMAND(ID_LD_PREVIEW, &CCoverDownloadDlg::OnLdPreview)
    ON_COMMAND(ID_LD_RELATE, &CCoverDownloadDlg::OnLdRelate)
    ON_WM_PAINT()
    ON_BN_CLICKED(IDC_COVER_DL_NETEASE_RADIO, &CCoverDownloadDlg::OnBnClickedCoverDlNeteaseRadio)
    ON_BN_CLICKED(IDC_COVER_DL_QQMUSIC_RADIO, &CCoverDownloadDlg::OnBnClickedCoverDlQqmusicRadio)
END_MESSAGE_MAP()


// CCoverDownloadDlg 消息处理程序


BOOL CCoverDownloadDlg::OnInitDialog()
{
    CBaseDialog::OnInitDialog();

    // TODO:  在此添加额外的初始化
    m_netease_download = std::make_unique<CNeteaseLyricDownload>();
    m_qqmusic_download = std::make_unique<CQQMusicLyricDownload>();
    m_dialog_download_service = GeneralSettingData::LDS_NETEASE;
    CheckRadioButton(IDC_COVER_DL_NETEASE_RADIO, IDC_COVER_DL_QQMUSIC_RADIO, IDC_COVER_DL_NETEASE_RADIO);

    m_song = GetSongInfo(); // 初始化复制Songinfo使用，防止随着播放GetSongInfo获取到另一首
    m_title = m_song.title;
    m_artist = m_song.artist;
    m_album = m_song.album;

    if (m_song.IsTitleEmpty())    // 如果没有标题信息，就把文件名设为标题
    {
        m_title = m_song.GetFileName();
        size_t index = m_title.rfind(L'.');
        m_title = m_title.substr(0, index);
    }
    if (m_song.IsArtistEmpty())	//没有艺术家信息，清空艺术家的文本
    {
        m_artist.clear();
    }
    if (m_song.IsAlbumEmpty())	//没有唱片集信息，清空唱片集的文本
    {
        m_album.clear();
    }
    m_file_name = m_song.GetFileName();

    SetDlgItemText(IDC_TITLE_EDIT, m_title.c_str());
    SetDlgItemText(IDC_ARTIST_EDIT, m_artist.c_str());

    ////设置列表控件主题颜色
    //m_down_list_ctrl.SetColor(theApp.m_app_setting_data.theme_color);

    //初始化搜索结果列表控件
    CRect rect;
    m_down_list_ctrl.GetWindowRect(rect);
    int width0, width1, width2, width3, width4;
    width0 = rect.Width() / 10;
    width1 = rect.Width() * 3 / 10;
    width2 = rect.Width() * 2 / 10;
    width4 = rect.Width() / 10;
    width3 = rect.Width() - theApp.DPI(20) - 1 - width0 - width1 - width2 - width4;

    m_down_list_ctrl.SetExtendedStyle(m_down_list_ctrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP);
    m_down_list_ctrl.InsertColumn(0, theApp.m_str_table.LoadText(L"TXT_SERIAL_NUMBER").c_str(), LVCFMT_LEFT, width0);
    m_down_list_ctrl.InsertColumn(1, theApp.m_str_table.LoadText(L"TXT_TITLE").c_str(), LVCFMT_LEFT, width1);
    m_down_list_ctrl.InsertColumn(2, theApp.m_str_table.LoadText(L"TXT_ARTIST").c_str(), LVCFMT_LEFT, width2);
    m_down_list_ctrl.InsertColumn(3, theApp.m_str_table.LoadText(L"TXT_ALBUM").c_str(), LVCFMT_LEFT, width3);
    m_down_list_ctrl.InsertColumn(4, theApp.m_str_table.LoadText(L"TXT_LENGTH").c_str(), LVCFMT_LEFT, width4);

    m_unassciate_lnk.ShowWindow(SW_HIDE);

    ShowDlgCtrl(IDC_DOWNLOAD_OPTION_GROUPBOX, false);
    ShowDlgCtrl(IDC_COVER_LOCATION_STATIC, false);
    ShowDlgCtrl(IDC_SAVE_TO_SONG_FOLDER2, false);
    ShowDlgCtrl(IDC_SAVE_TO_ALBUM_FOLDER2, false);
    UpdatePreviewRects();
    ShowDlgCtrl(IDC_CURRENT_COVER_PREVIEW, false);
    ShowDlgCtrl(IDC_ONLINE_COVER_PREVIEW, false);
    LoadCurrentCoverPreview();
    UpdateSaveButtonState();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
}


void CCoverDownloadDlg::OnBnClickedSearchButton()
{
    // TODO: 在此添加控件通知处理程序代码
    SetDlgItemText(IDC_STATIC_INFO, theApp.m_str_table.LoadText(L"TXT_LYRIC_DL_INFO_SEARCHING").c_str());    // 这里使用的是歌词下载对话框的字符串
    GetDlgItem(IDC_SEARCH_BUTTON)->EnableWindow(FALSE);		//点击“搜索”后禁用该按钮
    wstring keyword = CInternetCommon::URLEncode(m_artist + L' ' + m_title);	//搜索关键字为“艺术家 标题”，并将其转换成URL编码
    CString url = GetDownloadService()->GetSearchUrl(keyword, 30).c_str();
    m_search_url = url;
    theApp.m_cover_download_dialog_exit = false;
    m_pSearchThread = AfxBeginThread(SongSearchThreadFunc, this);
}


afx_msg LRESULT CCoverDownloadDlg::OnSearchComplate(WPARAM wParam, LPARAM lParam)
{
    //响应WM_SEARCH_CONPLATE消息
    GetDlgItem(IDC_SEARCH_BUTTON)->EnableWindow(TRUE);	//搜索完成之后启用该按钮
    switch (m_search_rtn)
    {
    case CInternetCommon::FAILURE:
    {
        const wstring& info = theApp.m_str_table.LoadText(L"MSG_NETWORK_SEARCH_FAILED");
        MessageBox(info.c_str(), NULL, MB_ICONWARNING);
        return 0;
    }
    case CInternetCommon::OUTTIME:
    {
        const wstring& info = theApp.m_str_table.LoadText(L"MSG_NETWORK_SEARCH_TIME_OUT");
        MessageBox(info.c_str(), NULL, MB_ICONWARNING);
        return 0;
    }
    default: break;
    }

    GetDownloadService()->DisposeSearchResult(m_down_list, m_search_result);		//处理返回的结果
    ShowDownloadList();			//将搜索的结果显示在列表控件中

    //计算搜索结果中最佳匹配项目
    int best_matched;
    bool id_releated{ false };
    std::wstring song_id;
    CSongDataManager::GetInstance().GetSongID(m_song, song_id);  // 从媒体库读取id
    m_song.SetSongId(song_id);
    if (!song_id.empty())        // 如果当前歌曲已经有关联的ID，则根据该ID在搜索结果列表中查找对应的项目
    {
        for (size_t i{}; i < m_down_list.size(); i++)
        {
            if (m_song.GetSongId() == m_down_list[i].id)
            {
                id_releated = true;
                best_matched = i;
                break;
            }
        }
    }
    if (!id_releated)
        best_matched = CLyricDownloadCommon::SelectMatchedItem(m_down_list, m_title, m_artist, m_album, m_file_name, true);
    wstring info;
    m_unassciate_lnk.ShowWindow(SW_HIDE);
    if (m_down_list.empty())
        info = theApp.m_str_table.LoadText(L"TXT_LYRIC_DL_INFO_SEARCH_NO_SONG");
    else if (best_matched == -1)
        info = theApp.m_str_table.LoadText(L"TXT_LYRIC_DL_INFO_SEARCH_NO_MATCHED");
    else if (id_releated)
    {
        info = theApp.m_str_table.LoadTextFormat(L"TXT_LYRIC_DL_INFO_SEARCH_RELATED", { best_matched + 1 });
        m_unassciate_lnk.ShowWindow(SW_SHOW);
    }
    else
        info = theApp.m_str_table.LoadTextFormat(L"TXT_LYRIC_DL_INFO_SEARCH_BEST_MATCHED", { best_matched + 1 });
    SetDlgItemText(IDC_STATIC_INFO, info.c_str());
    //自动选中列表中最佳匹配的项目
    if (best_matched >= 0)
    {
        m_down_list_ctrl.SetFocus();
        m_down_list_ctrl.SetItemState(best_matched, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);	//选中行
        m_down_list_ctrl.EnsureVisible(best_matched, FALSE);		//使选中行保持可见
    }
    m_item_selected = best_matched;
    LoadOnlineCoverPreview(vector<unsigned char>(), wstring());
    return 0;
}


void CCoverDownloadDlg::OnBnClickedDownloadSelected()
{
    if (m_online_cover_data.empty())
        return;

    if (!CPropertyDlgHelper::IsSongAlbumCoverWriteEnable(m_song))
    {
        MessageBox(theApp.m_str_table.LoadText(L"MSG_COVER_DL_WRITE_UNSUPPORTED").c_str(), NULL, MB_ICONWARNING | MB_OK);
        return;
    }

    CWaitCursor wait_cursor;
    if (IsItemSelectedValid())
        SetID(m_down_list[m_item_selected].id);
    CAudioTag audio_tag(m_song.file_path);
    if (!audio_tag.WriteAlbumCover(BytesToString(m_online_cover_data), m_online_cover_ext.empty() ? L"jpg" : m_online_cover_ext))
    {
        MessageBox(theApp.m_str_table.LoadText(L"MSG_COVER_DL_SAVE_TO_AUDIO_FILE_FAILED").c_str(), NULL, MB_ICONWARNING | MB_OK);
        return;
    }

    LoadCurrentCoverPreview();
    InvalidateRect(m_current_cover_rect);
    if (CPlayer::GetInstance().GetCurrentSongInfo() == m_song)
    {
        CPlayer::GetInstance().SearchOutAlbumCover();
        CPlayer::GetInstance().AlbumCoverGaussBlur();
    }
    MessageBox(theApp.m_str_table.LoadText(L"MSG_COVER_DL_SAVE_TO_AUDIO_FILE_COMPLETE").c_str(), NULL, MB_ICONINFORMATION | MB_OK);
}


void CCoverDownloadDlg::OnNMClickCoverDownList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码
    m_item_selected = pNMItemActivate->iItem;
    DownloadSelectedCoverToPreview();
    *pResult = 0;
}


void CCoverDownloadDlg::OnNMDblclkCoverDownList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码
    m_item_selected = pNMItemActivate->iItem;
    if (m_item_selected >= 0 && m_item_selected < static_cast<int>(m_down_list.size()))
    {
        DownloadSelectedCoverToPreview();
    }
    *pResult = 0;
}


void CCoverDownloadDlg::OnNMRClickCoverDownList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码
    m_item_selected = pNMItemActivate->iItem;

    if (IsItemSelectedValid())
    {
        //弹出右键菜单
        CMenu* pContextMenu = theApp.m_menu_mgr.GetMenu(MenuMgr::LdListMenu);
        m_down_list_ctrl.ShowPopupMenu(pContextMenu, pNMItemActivate->iItem, this);
    }

    *pResult = 0;
}


void CCoverDownloadDlg::OnOK()
{
    // TODO: 在此添加专用代码和/或调用基类
    theApp.m_cover_download_dialog_exit = true;
    if (m_pSearchThread != nullptr)
        WaitForSingleObject(m_pSearchThread->m_hThread, 1000);	//等待线程退出
    CBaseDialog::OnOK();
}


void CCoverDownloadDlg::OnCancel()
{
    // TODO: 在此添加专用代码和/或调用基类
    theApp.m_cover_download_dialog_exit = true;
    if (m_pSearchThread != nullptr)
        WaitForSingleObject(m_pSearchThread->m_hThread, 1000);	//等待线程退出
    CBaseDialog::OnCancel();
}


void CCoverDownloadDlg::OnDestroy()
{
    m_cover_cache.clear();
    m_cover_cache_ext.clear();
    CBaseDialog::OnDestroy();
}

void CCoverDownloadDlg::OnPaint()
{
    CPaintDC dc(this);
    UpdatePreviewRects();
    DrawCoverPreview(dc, m_current_cover_rect, m_current_cover_img);
    DrawCoverPreview(dc, m_online_cover_rect, m_online_cover_img);
}

void CCoverDownloadDlg::OnBnClickedCoverDlNeteaseRadio()
{
    m_dialog_download_service = GeneralSettingData::LDS_NETEASE;
    m_down_list.clear();
    m_down_list_ctrl.DeleteAllItems();
    m_item_selected = -1;
    LoadOnlineCoverPreview(vector<unsigned char>(), wstring());
}

void CCoverDownloadDlg::OnBnClickedCoverDlQqmusicRadio()
{
    m_dialog_download_service = GeneralSettingData::LDS_QQMUSIC;
    m_down_list.clear();
    m_down_list_ctrl.DeleteAllItems();
    m_item_selected = -1;
    LoadOnlineCoverPreview(vector<unsigned char>(), wstring());
}


void CCoverDownloadDlg::OnEnChangeTitleEdit()
{
    // TODO:  如果该控件是 RICHEDIT 控件，它将不
    // 发送此通知，除非重写 CBaseDialog::OnInitDialog()
    // 函数并调用 CRichEditCtrl().SetEventMask()，
    // 同时将 ENM_CHANGE 标志“或”运算到掩码中。

    // TODO:  在此添加控件通知处理程序代码
    CString tmp;
    GetDlgItemText(IDC_TITLE_EDIT, tmp);
    m_title = tmp;
}


void CCoverDownloadDlg::OnEnChangeArtistEdit()
{
    // TODO:  如果该控件是 RICHEDIT 控件，它将不
    // 发送此通知，除非重写 CBaseDialog::OnInitDialog()
    // 函数并调用 CRichEditCtrl().SetEventMask()，
    // 同时将 ENM_CHANGE 标志“或”运算到掩码中。

    // TODO:  在此添加控件通知处理程序代码
    CString tmp;
    GetDlgItemText(IDC_ARTIST_EDIT, tmp);
    m_artist = tmp;
}


void CCoverDownloadDlg::OnNMClickUnassociateLink(NMHDR* pNMHDR, LRESULT* pResult)
{
    // TODO: 在此添加控件通知处理程序代码
    SetID(wstring());
    m_unassciate_lnk.ShowWindow(SW_HIDE);

    *pResult = 0;
}



void CCoverDownloadDlg::OnBnClickedSaveToSongFolder2()
{
    // TODO: 在此添加控件通知处理程序代码
}


void CCoverDownloadDlg::OnBnClickedSaveToAlbumFolder2()
{
    // TODO: 在此添加控件通知处理程序代码
}

void CCoverDownloadDlg::OnLdCoverDownload()
{
    OnBnClickedDownloadSelected();
}

void CCoverDownloadDlg::OnLdCoverSaveas()
{
    if (!IsItemSelectedValid())
        return;

    //获取选中项的歌曲id
    wstring song_id = m_down_list[m_item_selected].id;
    if (song_id.empty())
        return;

    wstring cover_url = GetDownloadService()->GetAlbumCoverURL(song_id);
    wstring cover_ext = CFilePathHelper(cover_url).GetFileExtension();

    //构造保存文件对话框
    std::wstring filter = FilterHelper::GetImageFileFilter();
    CFileDialog fileDlg(FALSE, cover_ext.c_str(), m_down_list[m_item_selected].album.c_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter.c_str(), this);

    //显示保存文件对话框
    if (IDOK == fileDlg.DoModal())
    {
        wstring file_path = fileDlg.GetPathName().GetString();

        //下载专辑封面
        CWaitCursor wait_cursor;
        if (!cover_ext.empty())
        {
            CFilePathHelper path_helper(file_path);
            file_path = path_helper.ReplaceFileExtension(cover_ext.c_str());
        }
        HRESULT hr = URLDownloadToFile(0, cover_url.c_str(), file_path.c_str(), 0, NULL);
        if (hr == S_OK)
            MessageBox(theApp.m_str_table.LoadText(L"MSG_NETWORK_DOWNLOAD_COMPLETE").c_str(), NULL, MB_ICONINFORMATION | MB_OK);
        else
            MessageBox(theApp.m_str_table.LoadText(L"MSG_NETWORK_COVER_DOWNLOAD_FAILED").c_str(), NULL, MB_ICONWARNING | MB_OK);
    }
}

void CCoverDownloadDlg::OnLdCopyTitle()
{
    if (IsItemSelectedValid())
    {
        if (!CCommon::CopyStringToClipboard(m_down_list[m_item_selected].title))
            MessageBox(theApp.m_str_table.LoadText(L"MSG_COPY_CLIPBOARD_FAILED").c_str(), NULL, MB_ICONWARNING);
    }
}

void CCoverDownloadDlg::OnLdCopyArtist()
{
    if (IsItemSelectedValid())
    {
        if (!CCommon::CopyStringToClipboard(m_down_list[m_item_selected].artist))
            MessageBox(theApp.m_str_table.LoadText(L"MSG_COPY_CLIPBOARD_FAILED").c_str(), NULL, MB_ICONWARNING);
    }
}

void CCoverDownloadDlg::OnLdCopyAlbum()
{
    if (IsItemSelectedValid())
    {
        if (!CCommon::CopyStringToClipboard(m_down_list[m_item_selected].album))
            MessageBox(theApp.m_str_table.LoadText(L"MSG_COPY_CLIPBOARD_FAILED").c_str(), NULL, MB_ICONWARNING);
    }
}

void CCoverDownloadDlg::OnLdCopyId()
{
    if (IsItemSelectedValid())
    {
        if (!CCommon::CopyStringToClipboard(m_down_list[m_item_selected].id))
            MessageBox(theApp.m_str_table.LoadText(L"MSG_COPY_CLIPBOARD_FAILED").c_str(), NULL, MB_ICONWARNING);
    }
}

void CCoverDownloadDlg::OnLdViewOnline()
{
    if (IsItemSelectedValid())
    {
        //获取该歌曲的在线接听网址
        wstring song_url{ GetDownloadService()->GetOnlineUrl(m_down_list[m_item_selected].id) };
        //打开超链接
        ShellExecute(NULL, _T("open"), song_url.c_str(), NULL, NULL, SW_SHOW);
    }
}

void CCoverDownloadDlg::OnLdPreview()
{
    DownloadSelectedCoverToPreview();
}

void CCoverDownloadDlg::OnLdRelate()
{
    if (m_item_selected >= 0 && m_item_selected < static_cast<int>(m_down_list.size()))
    {
        SetID(m_down_list[m_item_selected].id);     // 将选中项目的歌曲ID关联到歌曲
    }
}
