#include "stdafx.h"
#include "TracksList.h"
#include "MusicPlayerCmdHelper.h"
#include "Player.h"

std::wstring UiElement::AbstractTracksList::GetItemText(int row, int col)
{
    if (row >= 0 && row < GetRowCount())
    {
        //序号
        if (col == COL_INDEX)
        {
            return std::to_wstring(row + 1);
        }
        //曲目
        if (col == COL_TRACK)
        {
            if (row >= 0 && row < GetSongListData()->GetSongCount())
            {
                return GetSongListData()->GetItem(row).name;
            }
        }
        //时间
        else if (col == COL_TIME)
        {
            if (row >= 0 && row < GetSongListData()->GetSongCount())
            {
                return GetSongListData()->GetItem(row).length.toString();
            }
        }
    }
    return std::wstring();
}

int UiElement::AbstractTracksList::GetRowCount()
{
    return GetSongListData()->GetSongCount();
}

int UiElement::AbstractTracksList::GetColumnCount()
{
    return COL_MAX;
}

int UiElement::AbstractTracksList::GetColumnWidth(int col, int total_width)
{
    const int index_width{ ui->DPI((std::max)(60, 60 + (EffectiveFontSize() - 20) * 5)) };
    const int time_width{ ui->DPI((std::max)(90, 90 + (EffectiveFontSize() - 20) * 4)) };
    if (col == COL_INDEX)
    {
        return index_width;
    }
    else if (col == COL_TIME)
    {
        return time_width;
    }
    else if (col == COL_TRACK)
    {
        return total_width - index_width - time_width;
    }
    return 0;
}

bool UiElement::AbstractTracksList::IsHighlightRow(int row)
{
    CUISongListMgr* ui_song_list_mgr = GetSongListData();
    if (ui_song_list_mgr != nullptr)
    {
        SongKey song_key = ui_song_list_mgr->GetItem(row).song_key;
        const auto& current_song = CPlayer::GetInstance().GetSafeCurrentSongInfo();
        if (!song_key.path.empty() && !current_song.IsEmpty())
            return (std::equal_to<SongKey>()(SongKey(current_song), song_key));
    }
    return false;
}

int UiElement::AbstractTracksList::GetColumnScrollTextWhenSelected()
{
    return COL_TRACK;
}

CMenu* UiElement::AbstractTracksList::GetContextMenu(bool item_selected)
{
    if (item_selected)
    {
        return theApp.m_menu_mgr.GetMenu(MenuMgr::LibRightMenu);
    }
    return nullptr;
}

void UiElement::AbstractTracksList::OnDoubleClicked()
{
    int item_selected = GetItemSelected();
    if (item_selected >= 0 && item_selected < GetRowCount())
    {
        const SongInfo& song{ GetSongListData()->GetSongInfo(item_selected) };
        CMusicPlayerCmdHelper helper;
        helper.OnPlayAllTrack(song);
    }
}

std::wstring UiElement::AbstractTracksList::GetEmptyString()
{
    if (GetSongListData()->IsLoading())
        return theApp.m_str_table.LoadText(L"UI_MEDIALIB_LIST_LOADING_INFO");
    else if (!GetSongListData()->IsInited())
        return theApp.m_str_table.LoadText(L"UI_MEDIALIB_LIST_UNINITED_INFO");
    else
        return theApp.m_str_table.LoadText(L"UI_MEDIALIB_LIST_EMPTY_INFO");
}

int UiElement::AbstractTracksList::GetHoverButtonCount(int row)
{
    return row >= 0 && row < GetRowCount() ? 1 : 0;
}

int UiElement::AbstractTracksList::GetHoverButtonColumn()
{
    return COL_TRACK;
}

IconMgr::IconType UiElement::AbstractTracksList::GetHoverButtonIcon(int index, int row)
{
    if (index == 0)
    {
        if (GetSongListData()->GetItem(row).is_favourite)
            return IconMgr::IT_Favorite_Off;
        else
            return IconMgr::IT_Favorite_On;
    }
    return IconMgr::IT_NO_ICON;
}

std::wstring UiElement::AbstractTracksList::GetHoverButtonTooltip(int index, int row)
{
    if (index == 0)
        return theApp.m_str_table.LoadText(L"UI_TIP_BTN_FAVOURITE");
    return std::wstring();
}

void UiElement::AbstractTracksList::OnHoverButtonClicked(int btn_index, int row)
{
    CMusicPlayerCmdHelper helper;
    //点击了“添加到我喜欢的音乐”按钮
    if (btn_index == 0)
    {
        const SongInfo& song{ GetSongListData()->GetSongInfo(row) };
        helper.OnAddRemoveFromFavourite(song);
        GetSongListData()->AddOrRemoveMyFavourite(row);        //更新UI中的显示
    }
}

int UiElement::AbstractTracksList::GetUnHoverIconCount(int row)
{
    //鼠标未指向的列，如果曲目在“我喜欢的音乐”中，则显示红心图标
    if (GetSongListData()->GetItem(row).is_favourite)
        return 1;
    else
        return 0;
}

IconMgr::IconType UiElement::AbstractTracksList::GetUnHoverIcon(int index, int row)
{
    if (index == 0)
    {
        return IconMgr::IT_Favorite_Off;
    }
    return IconMgr::IT_NO_ICON;
}

bool UiElement::AbstractTracksList::IsMultipleSelectionEnable()
{
    return true;
}
