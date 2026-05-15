#include "stdafx.h"
#include "TracksList.h"
#include "MusicPlayerCmdHelper.h"
#include "MediaLibHelper.h"
#include "AudioCommon.h"
#include "Playlist.h"
#include "SongDataManager.h"
#include "CRecentList.h"
#include "Player.h"

namespace
{
    bool IsSamePlayableList(const ListItem& list_item)
    {
        if (list_item.empty() || !CRecentList::Instance().IsCurrentList(list_item))
            return false;

        ListItem current_list{ CRecentList::Instance().GetCurrentList() };
        if (list_item.type == LT_FOLDER && current_list.contain_sub_folder != list_item.contain_sub_folder)
            return false;

        return true;
    }

    bool PlayFromCurrentPlaylist(const SongInfo& song, int fallback_track)
    {
        int track = CPlayer::GetInstance().IsSongInPlayList(song);
        if (track < 0)
            track = fallback_track;
        if (track < 0 || track >= CPlayer::GetInstance().GetSafeSongNum())
            return false;

        CMusicPlayerCmdHelper helper;
        helper.OnPlayTrack(track);
        return true;
    }

    void PlayListItemSong(ListItem& list_item, const SongInfo& song, int row)
    {
        if (IsSamePlayableList(list_item) && PlayFromCurrentPlaylist(song, row))
            return;

        list_item.SetPlayTrack(song);
        CMusicPlayerCmdHelper helper;
        helper.OnListItemSelected(list_item, true, true);
    }
}

//////////////////////////////////////////////////////////////////
UiElement::TrackList::TrackList()
{
    m_ui_song_list = std::make_unique<CUISongListMgr>();
}

void UiElement::TrackList::SetListItem(const ListItem& list_item)
{
    m_list_item = list_item;

    std::vector<SongInfo> song_list;
    if (list_item.type == LT_MEDIA_LIB)
    {
        if (list_item.medialib_type != ListItem::ClassificationType::CT_NONE)
        {
            CMediaClassifier classifer(list_item.medialib_type);
            classifer.ClassifyMedia();
            song_list = classifer.GetMeidaList()[list_item.path];
        }

    }
    else if (list_item.type == LT_FOLDER)
    {
        CAudioCommon::GetAudioFiles(list_item.path, song_list, MAX_SONG_NUM, list_item.contain_sub_folder);
        //展开cue
        int cnt{};
        bool flag{};
        CAudioCommon::GetCueTracks(song_list, cnt, flag, MR_MIN_REQUIRED);
        //排序
        auto sort_fun = SongInfo::GetSortFunc(list_item.sort_mode == SM_UNSORT ? SM_U_FILE : list_item.sort_mode);
        std::stable_sort(song_list.begin(), song_list.end(), sort_fun);
        //获取曲目信息
        for (auto& cur_song : song_list)
        {
            cur_song = CSongDataManager::GetInstance().GetSongInfo3(cur_song);
        }
    }
    else if (list_item.type == LT_PLAYLIST)
    {
        CPlaylistFile playlist;
        playlist.LoadFromFile(list_item.path);
        playlist.MoveToSongList(song_list);
        //获取曲目信息
        for (auto& cur_song : song_list)
        {
            cur_song = CSongDataManager::GetInstance().GetSongInfo3(cur_song);
        }
    }
    m_ui_song_list->Update(song_list);
    scroll_offset = 0;
    EnsureHighlightItemVisible();
}

void UiElement::TrackList::ClearListItem()
{
    m_ui_song_list->Update(std::vector<SongInfo>());
    scroll_offset = 0;
}

std::wstring UiElement::TrackList::GetEmptyString()
{
    if (GetSongListData()->IsLoading())
        return theApp.m_str_table.LoadText(L"UI_MEDIALIB_LIST_LOADING_INFO");
    else
        return std::wstring();
}

CUISongListMgr* UiElement::TrackList::GetSongListData()
{
    //如果显示的列表为所有曲目，则直接使用CUiAllTracksMgr的数据
    if (m_list_item.type == LT_MEDIA_LIB && m_list_item.medialib_type == ListItem::ClassificationType::CT_NONE)
        return &CUiAllTracksMgr::Instance();
    else
        return m_ui_song_list.get();
}

void UiElement::TrackList::OnHoverButtonClicked(int btn_index, int row)
{
    AbstractTracksList::OnHoverButtonClicked(btn_index, row);
}

void UiElement::TrackList::OnDoubleClicked()
{
    int item_selected = GetItemSelected();
    if (item_selected >= 0 && item_selected < GetRowCount())
    {
        const SongInfo& song{ GetSongListData()->GetSongInfo(item_selected) };
        PlayListItemSong(m_list_item, song, item_selected);
    }
}
