#include "stdafx.h"
#include "MyFavouriteList.h"
#include "UiMediaLibItemMgr.h"
#include "MusicPlayerCmdHelper.h"
#include "CRecentList.h"
#include "Player.h"
#include "FilePathHelper.h"

std::wstring UiElement::MyFavouriteList::GetItemText(int row, int col)
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
            if (row >= 0 && row < CUiMyFavouriteItemMgr::Instance().GetSongCount())
            {
                const SongInfo& song_info{ CUiMyFavouriteItemMgr::Instance().GetSongInfo(row) };
                return CFilePathHelper(song_info.file_path).GetFileNameWithoutExtension();
            }
        }
        //时间
        else if (col == COL_TIME)
        {
            const SongInfo& song_info{ CUiMyFavouriteItemMgr::Instance().GetSongInfo(row) };
            return song_info.length().toString();
        }
    }
    return std::wstring();
}

int UiElement::MyFavouriteList::GetRowCount()
{
    return CUiMyFavouriteItemMgr::Instance().GetSongCount();
}

int UiElement::MyFavouriteList::GetColumnCount()
{
    return COL_MAX;
}

int UiElement::MyFavouriteList::GetColumnWidth(int col, int total_width)
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

bool UiElement::MyFavouriteList::IsHighlightRow(int row)
{
    if (CRecentList::Instance().IsPlayingSpecPlaylist(CRecentList::PT_FAVOURITE))
    {
        return CPlayer::GetInstance().GetIndex() == row;
    }
    return false;
}

int UiElement::MyFavouriteList::GetColumnScrollTextWhenSelected()
{
    return COL_TRACK;
}

CMenu* UiElement::MyFavouriteList::GetContextMenu(bool item_selected)
{
    if (item_selected)
    {
        return theApp.m_menu_mgr.GetMenu(MenuMgr::UiMyFavouriteMenu);
    }
    return nullptr;
}

void UiElement::MyFavouriteList::OnDoubleClicked()
{
    int item_selected = GetItemSelected();
    if (item_selected >= 0 && item_selected < GetRowCount())
    {
        CMusicPlayerCmdHelper helper;
        SongInfo song_info{ CUiMyFavouriteItemMgr::Instance().GetSongInfo(item_selected) };
        helper.OnPlayMyFavourite(song_info);
    }
}

std::wstring UiElement::MyFavouriteList::GetEmptyString()
{
    if (CUiMyFavouriteItemMgr::Instance().IsLoading())
        return theApp.m_str_table.LoadText(L"UI_MEDIALIB_LIST_LOADING_INFO");
    else if (!CUiMyFavouriteItemMgr::Instance().IsInited())
        return theApp.m_str_table.LoadText(L"UI_MEDIALIB_LIST_UNINITED_INFO");
    else
        return theApp.m_str_table.LoadText(L"UI_MEDIALIB_LIST_EMPTY_INFO");
}

int UiElement::MyFavouriteList::GetHoverButtonCount(int row)
{
    return 0;
}

int UiElement::MyFavouriteList::GetHoverButtonColumn()
{
    return COL_TRACK;
}

IconMgr::IconType UiElement::MyFavouriteList::GetHoverButtonIcon(int index, int row)
{
    switch (index)
    {
    case BTN_PLAY: return IconMgr::IT_Play;
    case BTN_ADD: return IconMgr::IT_Add;
    }
    return IconMgr::IT_NO_ICON;
}

std::wstring UiElement::MyFavouriteList::GetHoverButtonTooltip(int index, int row)
{
    switch (index)
    {
    case BTN_PLAY: return theApp.m_str_table.LoadText(L"UI_TIP_BTN_PLAY");
    case BTN_ADD: return theApp.m_str_table.LoadText(L"UI_TIP_BTN_ADD_TO_PLAYLIST");
    }
    return std::wstring();
}

void UiElement::MyFavouriteList::OnHoverButtonClicked(int btn_index, int row)
{
    CMusicPlayerCmdHelper helper;
    //点击了“播放”按钮
    if (btn_index == BTN_PLAY)
    {
        int item_selected = GetItemSelected();
        if (item_selected >= 0 && item_selected < GetRowCount())
        {
            CMusicPlayerCmdHelper helper;
            SongInfo song_info{ CUiMyFavouriteItemMgr::Instance().GetSongInfo(item_selected) };
            helper.OnPlayMyFavourite(song_info);
        }
    }
    //点击了“添加到播放列表”按钮
    else if (btn_index == BTN_ADD)
    {
        CMenu* menu = theApp.m_menu_mgr.GetMenu(MenuMgr::AddToPlaylistMenu);
        ShowContextMenu(menu, nullptr);
    }
}

bool UiElement::MyFavouriteList::IsMultipleSelectionEnable()
{
    return true;
}
