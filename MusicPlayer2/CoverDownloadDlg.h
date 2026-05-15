#pragma once
#include "ListCtrlEx.h"
#include "SongInfo.h"
#include "BaseDialog.h"
#include "LyricDownloadCommon.h"
#include "CommonData.h"

class CNeteaseLyricDownload;
class CQQMusicLyricDownload;

// CCoverDownloadDlg 对话框

class CCoverDownloadDlg : public CBaseDialog
{
    DECLARE_DYNAMIC(CCoverDownloadDlg)

public:
    CCoverDownloadDlg(CWnd* pParent = nullptr);   // 标准构造函数
    virtual ~CCoverDownloadDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_COVER_DOWNLOAD_DIALOG };
#endif

#define WM_SEARCH_COMPLATE (WM_USER+101)        //歌曲搜索完成消息
    //歌曲搜索线程函数
    static UINT SongSearchThreadFunc(LPVOID lpParam);

protected:
    CListCtrlEx m_down_list_ctrl;
    CLinkCtrl m_unassciate_lnk;

    SongInfo m_song;        // 此次下载封面的歌曲，窗口初始化时写入，之后不应再从播放实例获取
    wstring m_title;        //要查找歌词的歌曲的标题
    wstring m_artist;       //要查找歌词的歌曲的艺术家
    wstring m_album;        //要查找歌词的歌曲的唱片集
    wstring m_file_name;    //要查找歌词的歌曲的文件名

    vector<CLyricDownloadCommon::ItemInfo> m_down_list;  //搜索结果的列表
    int m_item_selected{ -1 };      //搜索结果列表中选中的项目

    wstring m_search_url;
    int m_search_rtn;
    wstring m_search_result;

    CWinThread* m_pSearchThread;        //搜索歌词的线程

    void SetID(wstring id);     // 保存指定id到媒体库内m_song.file_path对应条目

    virtual SongInfo GetSongInfo() const;
    virtual CString GetDialogName() const override;
    virtual bool InitializeControls() override;

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
    void ShowDownloadList();        //将搜索结果显示出来
    CLyricDownloadCommon* GetDownloadService();
    void LoadCurrentCoverPreview();
    void LoadOnlineCoverPreview(const vector<unsigned char>& cover_data, const wstring& ext);
    void DrawCoverPreview(CDC& dc, const CRect& rect, CImage& image);
    void UpdatePreviewRects();
    void UpdateSaveButtonState();
    wstring GetCoverCacheKey(const wstring& song_id) const;
    bool DownloadSelectedCoverToPreview();

    DECLARE_MESSAGE_MAP()

protected:
    bool IsItemSelectedValid() const;
    CImage m_current_cover_img;
    CImage m_online_cover_img;
    CRect m_current_cover_rect;
    CRect m_online_cover_rect;
    vector<unsigned char> m_online_cover_data;
    wstring m_online_cover_ext;
    std::map<wstring, vector<unsigned char>> m_cover_cache;
    std::map<wstring, wstring> m_cover_cache_ext;
    GeneralSettingData::LyricDownloadService m_dialog_download_service{ GeneralSettingData::LDS_NETEASE };
    std::unique_ptr<CNeteaseLyricDownload> m_netease_download;
    std::unique_ptr<CQQMusicLyricDownload> m_qqmusic_download;

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedSearchButton();
protected:
    afx_msg LRESULT OnSearchComplate(WPARAM wParam, LPARAM lParam);
public:
    afx_msg virtual void OnBnClickedDownloadSelected();
    afx_msg void OnNMClickCoverDownList(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMDblclkCoverDownList(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMRClickCoverDownList(NMHDR *pNMHDR, LRESULT *pResult);
    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnDestroy();
public:
    afx_msg void OnPaint();
    afx_msg void OnBnClickedCoverDlNeteaseRadio();
    afx_msg void OnBnClickedCoverDlQqmusicRadio();
    afx_msg void OnEnChangeTitleEdit();
    afx_msg void OnEnChangeArtistEdit();
    afx_msg void OnNMClickUnassociateLink(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedSaveToSongFolder2();
    afx_msg void OnBnClickedSaveToAlbumFolder2();
    afx_msg void OnLdCoverDownload();
    afx_msg void OnLdCoverSaveas();
    afx_msg void OnLdCopyTitle();
    afx_msg void OnLdCopyArtist();
    afx_msg void OnLdCopyAlbum();
    afx_msg void OnLdCopyId();
    afx_msg void OnLdViewOnline();
    afx_msg void OnLdPreview();
    afx_msg void OnLdRelate();

};
