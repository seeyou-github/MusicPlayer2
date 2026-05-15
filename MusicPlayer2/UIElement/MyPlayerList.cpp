#include "stdafx.h"
#include "MyPlayerList.h"
#include "FolderBrowserDlg.h"
#include "InputDlg.h"
#include "FilePathHelper.h"
#include "MusicPlayerDlg.h"
#include "TinyXml2Helper.h"
#include "AudioCommon.h"
#include "SongDataManager.h"
#include "MyPlayerListCache.h"
#include "Player.h"
#include "CRecentList.h"
#include "MusicPlayerCmdHelper.h"
#include <cstdlib>

namespace
{
    constexpr UINT TAB_MENU_COMMAND_BASE{ 0xE000 };
    constexpr int NO_TAB_INDEX{ -1 };
    constexpr int FAVOURITE_TAB_INDEX{ -2 };
    constexpr COLORREF FAVOURITE_TAB_HEART_COLOR{ RGB(220, 48, 72) };
    const wchar_t* FAVOURITE_TAB_ORDER_TOKEN{ L"<favourite>" };

    int DragDistance(CPoint a, CPoint b)
    {
        return (std::max)(std::abs(a.x - b.x), std::abs(a.y - b.y));
    }
}

void UiElement::MyPlayerList::Draw()
{
    if (!m_tabs_inited)
    {
        InitTabsWithoutLoading();
        m_tabs_inited = true;
    }

    CalculateRect();
    m_tab_height = ui->DPI(m_tab_height_config + m_tab_margin_top);
    m_element_rect = rect;
    if (IsFolderTab(m_selected_tab) && m_cached_list_font_size != theApp.m_app_setting_data.song_list_font_size)
    {
        if (!LoadFolderSongsFromCache(m_selected_tab))
        {
            m_cached_list_font_size = theApp.m_app_setting_data.song_list_font_size;
            m_cached_item_height = CalculateSongListItemHeight(theApp.m_app_setting_data.song_list_font_size);
        }
    }
    else if (IsFavouriteTabSelected() && m_cached_list_font_size != theApp.m_app_setting_data.song_list_font_size)
    {
        SetFavouriteSongs();
    }
    CRect list_rect{ GetListRect() };
    SetRect(list_rect);
    TrackList::Draw();
    rect = m_element_rect;
}

bool UiElement::MyPlayerList::LButtonDown(CPoint point)
{
    if (m_element_rect.PtInRect(point) && point.y < m_element_rect.top + m_tab_height)
    {
        if (HitTestTabAddButton(point))
            return true;

        if (HitTestTabMenuButton(point))
        {
            ShowHiddenTabsMenu();
            return true;
        }

        int index = HitTestTab(point);
        if (index != NO_TAB_INDEX)
        {
            m_tab_pressed = index;
            m_tab_drag_from = index;
            m_tab_drag_drop_index = GetTabOrderPosition(index);
            m_tab_drag_start_point = point;
            m_tab_dragging = false;
        }
        else
        {
            ResetTabDragState();
        }
        return true;
    }
    ResetTabDragState();
    CRect old_rect{ rect };
    SetRect(GetListRect());
    bool rtn = TrackList::LButtonDown(point);
    rect = old_rect;
    return rtn;
}

bool UiElement::MyPlayerList::RButtonUp(CPoint point)
{
    if (m_element_rect.PtInRect(point) && point.y < m_element_rect.top + m_tab_height)
    {
        int index = HitTestTab(point);
        ShowTabContextMenu(index);
        return true;
    }
    CRect old_rect{ rect };
    SetRect(GetListRect());
    bool rtn = TrackList::RButtonUp(point);
    rect = old_rect;
    return rtn;
}

bool UiElement::MyPlayerList::LButtonUp(CPoint point)
{
    if (m_tab_drag_from != NO_TAB_INDEX)
    {
        const int pressed_tab{ m_tab_pressed };
        const int drag_from{ m_tab_drag_from };
        const int drop_index{ m_tab_drag_drop_index };
        const bool dragging{ m_tab_dragging };
        ResetTabDragState();

        if (dragging)
        {
            ReorderFolderTab(drag_from, drop_index);
            return true;
        }

        if (pressed_tab != NO_TAB_INDEX && HitTestTab(point) == pressed_tab)
        {
            SelectFolderTab(pressed_tab, true);
            return true;
        }
    }

    if (m_element_rect.PtInRect(point) && point.y < m_element_rect.top + m_tab_height)
    {
        if (HitTestTabAddButton(point))
        {
            AddFolder();
            return true;
        }
        return true;
    }
    CRect old_rect{ rect };
    SetRect(GetListRect());
    bool rtn = TrackList::LButtonUp(point);
    rect = old_rect;
    return rtn;
}

bool UiElement::MyPlayerList::DoubleClick(CPoint point)
{
    if (m_element_rect.PtInRect(point) && point.y < m_element_rect.top + m_tab_height)
        return true;
    CRect old_rect{ rect };
    SetRect(GetListRect());
    bool rtn = TrackList::DoubleClick(point);
    rect = old_rect;
    return rtn;
}

bool UiElement::MyPlayerList::MouseMove(CPoint point)
{
    if (m_tab_drag_from != NO_TAB_INDEX)
    {
        if (!m_tab_dragging && DragDistance(point, m_tab_drag_start_point) >= ui->DPI(4))
            m_tab_dragging = true;
        if (m_tab_dragging)
        {
            m_tab_drag_drop_index = GetTabDropIndex(point);
            m_hover_tab = -1;
            return true;
        }
    }

    if (m_element_rect.PtInRect(point) && point.y < m_element_rect.top + m_tab_height)
    {
        m_hover_tab = (HitTestTabMenuButton(point) || HitTestTabAddButton(point)) ? -1 : HitTestTab(point);
        return true;
    }
    m_hover_tab = -1;
    CRect old_rect{ rect };
    SetRect(GetListRect());
    bool rtn = TrackList::MouseMove(point);
    rect = old_rect;
    return rtn;
}

bool UiElement::MyPlayerList::MouseLeave()
{
    m_hover_tab = -1;
    if (!m_tab_dragging)
        ResetTabDragState();
    return TrackList::MouseLeave();
}

int UiElement::MyPlayerList::GetScrollAreaHeight()
{
    return TrackList::GetScrollAreaHeight();
}

int UiElement::MyPlayerList::GetSongListCachedItemHeight() const
{
    if (m_cached_item_height > 0 && m_cached_list_font_size == theApp.m_app_setting_data.song_list_font_size)
        return m_cached_item_height;
    return 0;
}

void UiElement::MyPlayerList::FromXmlNode(tinyxml2::XMLElement* xml_node)
{
    TrackList::FromXmlNode(xml_node);
    CTinyXml2Helper::GetElementAttributeInt(xml_node, "tab_height", m_tab_height_config);
    CTinyXml2Helper::GetElementAttributeInt(xml_node, "tab_margin_top", m_tab_margin_top);
    CTinyXml2Helper::GetElementAttributeInt(xml_node, "tab_margin_left", m_tab_margin_left);
    CTinyXml2Helper::GetElementAttributeInt(xml_node, "tab_margin_right", m_tab_margin_right);
    CTinyXml2Helper::GetElementAttributeInt(xml_node, "tab_padding", m_tab_padding_config);
    CTinyXml2Helper::GetElementAttributeInt(xml_node, "tab_selected_font_size", m_tab_selected_font_size);
    CTinyXml2Helper::GetElementAttributeInt(xml_node, "tab_unselected_font_size", m_tab_unselected_font_size);
    CTinyXml2Helper::GetElementAttributeBool(xml_node, "show_favourite_tab", m_show_favourite_tab);
    CCommon::SetNumRange(m_tab_height_config, 16, 96);
    CCommon::SetNumRange(m_tab_margin_top, 0, 96);
    CCommon::SetNumRange(m_tab_margin_left, 0, 256);
    CCommon::SetNumRange(m_tab_margin_right, 0, 256);
    CCommon::SetNumRange(m_tab_padding_config, 0, 96);
    if (m_tab_selected_font_size > 0)
        CCommon::SetNumRange(m_tab_selected_font_size, 5, 72);
    if (m_tab_unselected_font_size > 0)
        CCommon::SetNumRange(m_tab_unselected_font_size, 5, 72);
    ParseTabColor(xml_node, "tab_background_color", m_tab_background_color);
    ParseTabColor(xml_node, "tab_selected_background_color", m_tab_selected_background_color);
    ParseTabColor(xml_node, "tab_unselected_background_color", m_tab_unselected_background_color);
    ParseTabColor(xml_node, "tab_selected_text_color", m_tab_selected_text_color);
    ParseTabColor(xml_node, "tab_unselected_text_color", m_tab_unselected_text_color);
}

std::wstring UiElement::MyPlayerList::GetItemText(int row, int col)
{
    if (row < 0 || row >= GetSongListData()->GetSongCount())
        return std::wstring();

    if (col == COL_TRACK)
    {
        if (row >= 0 && row < static_cast<int>(m_cached_tracks.size()))
            return m_cached_tracks[row].display_name;
        return std::wstring();
    }
    else if (col == COL_TIME)
    {
        if (row >= 0 && row < static_cast<int>(m_cached_tracks.size()))
            return m_cached_tracks[row].duration_text;
        return std::wstring();
    }

    return AbstractTracksList::GetItemText(row, col);
}

bool UiElement::MyPlayerList::IsHighlightRow(int row)
{
    if (IsFavouriteTabSelected() && CRecentList::Instance().IsPlayingSpecPlaylist(CRecentList::PT_FAVOURITE))
        return CPlayer::GetInstance().GetIndex() == row;
    return AbstractTracksList::IsHighlightRow(row);
}

CMenu* UiElement::MyPlayerList::GetContextMenu(bool item_selected)
{
    if (IsFavouriteTabSelected())
        return nullptr;
    return AbstractTracksList::GetContextMenu(item_selected);
}

void UiElement::MyPlayerList::OnDoubleClicked()
{
    int item_selected = GetItemSelected();
    if (IsFavouriteTabSelected())
    {
        if (item_selected >= 0 && item_selected < CUiMyFavouriteItemMgr::Instance().GetSongCount())
        {
            CMusicPlayerCmdHelper helper;
            SongInfo song_info{ CUiMyFavouriteItemMgr::Instance().GetSongInfo(item_selected) };
            helper.OnPlayMyFavourite(song_info);
        }
        return;
    }
    TrackList::OnDoubleClicked();
}

void UiElement::MyPlayerList::OnHoverButtonClicked(int btn_index, int row)
{
    TrackList::OnHoverButtonClicked(btn_index, row);
    if (IsFavouriteTabSelected())
    {
        CUiMyFavouriteItemMgr::Instance().UpdateMyFavourite();
        SetFavouriteSongs();
    }
}

std::wstring UiElement::MyPlayerList::GetEmptyString()
{
    if (IsFavouriteTabSelected())
    {
        if (CUiMyFavouriteItemMgr::Instance().IsLoading())
            return theApp.m_str_table.LoadText(L"UI_MEDIALIB_LIST_LOADING_INFO");
        else if (!CUiMyFavouriteItemMgr::Instance().IsInited())
            return theApp.m_str_table.LoadText(L"UI_MEDIALIB_LIST_UNINITED_INFO");
        else
            return theApp.m_str_table.LoadText(L"UI_MEDIALIB_LIST_EMPTY_INFO");
    }
    return TrackList::GetEmptyString();
}

void UiElement::MyPlayerList::DrawScrollArea()
{
    CRect tab_source_rect{ m_element_rect };
    if (tab_source_rect.IsRectEmpty())
        tab_source_rect = rect;

    const int tab_gap = ui->DPI(2);
    const int tab_padding = GetTabPadding();
    const int min_tab_width = ui->DPI(80);

    CRect tab_bar_rect{ tab_source_rect };
    tab_bar_rect.bottom = tab_bar_rect.top + m_tab_height;
    CRect tab_content_rect{ tab_bar_rect };
    tab_content_rect.left += ui->DPI(m_tab_margin_left);
    tab_content_rect.right -= ui->DPI(m_tab_margin_right);
    if (tab_content_rect.right < tab_content_rect.left)
        tab_content_rect.right = tab_content_rect.left;

    BYTE background_alpha = static_cast<BYTE>(ui->IsDrawBackgroundAlpha() ? ALPHA_CHG(theApp.m_app_setting_data.background_transparency) / 2 : 255);
    ui->GetDrawer().FillAlphaRect(tab_bar_rect, m_tab_background_color.set ? m_tab_background_color.color : ui->GetUIColors().color_control_bar_back, background_alpha, true);

    m_tab_rects.clear();
    m_tab_indices.clear();
    m_hidden_tab_indices.clear();
    m_tab_menu_button_rect.SetRectEmpty();
    m_tab_add_button_rect.SetRectEmpty();
    m_tab_visible_rect = tab_content_rect;
    BuildTabOrder();

    const int tab_y = tab_bar_rect.top + ui->DPI(m_tab_margin_top);

    const int menu_button_size = ui->DPI((std::max)(20, m_tab_height_config));
    if (m_tab_order.empty())
    {
        m_tab_add_button_rect = tab_content_rect;
        m_tab_add_button_rect.left += ui->DPI(4);
        m_tab_add_button_rect.right = m_tab_add_button_rect.left + menu_button_size;
        m_tab_add_button_rect.top = tab_y;
        m_tab_add_button_rect.bottom = tab_y + ui->DPI(m_tab_height_config);
        if (m_tab_add_button_rect.right > tab_content_rect.right)
            m_tab_add_button_rect.right = tab_content_rect.right;

        if (!m_tab_add_button_rect.IsRectEmpty())
        {
            CPlayerUIBase::UIButton add_button;
            CPoint cursor_pos;
            GetCursorPos(&cursor_pos);
            if (ui->GetOwner() != nullptr)
                ui->GetOwner()->ScreenToClient(&cursor_pos);
            add_button.hover = m_tab_add_button_rect.PtInRect(cursor_pos) != FALSE;
            ui->DrawUIButton(m_tab_add_button_rect, add_button, IconMgr::IT_Add, false, std::wstring(), 9, false, Alignment::LEFT, true);
        }
    }

    std::vector<int> tab_widths;
    tab_widths.reserve(m_tab_order.size());
    m_tab_total_width = 0;
    for (int i{}; i < static_cast<int>(m_tab_order.size()); ++i)
    {
        int tab_index = m_tab_order[i];
        std::wstring name = GetTabName(tab_index);
        UiFontGuard font_guard(ui, GetTabFontSize(tab_index == m_selected_tab));
        int tab_width = ui->GetDrawer().GetTextExtent(name.c_str()).cx + tab_padding * 2;
        if (IsFavouriteTab(tab_index))
            tab_width += ui->DPI(18);
        tab_width = (std::max)(tab_width, min_tab_width);
        tab_widths.push_back(tab_width);
        m_tab_total_width += tab_width;
        if (i + 1 < static_cast<int>(m_tab_order.size()))
            m_tab_total_width += tab_gap;
    }

    const bool tabs_overflow = m_tab_total_width > tab_content_rect.Width();
    if (tabs_overflow)
    {
        m_tab_menu_button_rect = tab_content_rect;
        m_tab_menu_button_rect.left = (std::max)(m_tab_menu_button_rect.left, m_tab_menu_button_rect.right - menu_button_size);
        m_tab_menu_button_rect.top = tab_y;
        m_tab_menu_button_rect.bottom = tab_y + ui->DPI(m_tab_height_config);
        m_tab_visible_rect.right = (std::max)(m_tab_visible_rect.left, m_tab_menu_button_rect.left - tab_gap);
    }

    const int max_scroll_offset = (std::max)(0, m_tab_total_width - (m_tab_visible_rect.Width() - ui->DPI(4)));
    CCommon::SetNumRange(m_tab_scroll_offset, 0, max_scroll_offset);
    const int selected_order_pos = GetTabOrderPosition(m_selected_tab);
    if (selected_order_pos >= 0 && selected_order_pos < static_cast<int>(tab_widths.size()))
    {
        int selected_left = m_tab_visible_rect.left + ui->DPI(4) - m_tab_scroll_offset;
        for (int i{}; i < selected_order_pos; ++i)
            selected_left += tab_widths[i] + tab_gap;
        int selected_right = selected_left + tab_widths[selected_order_pos];
        if (selected_left < m_tab_visible_rect.left)
            m_tab_scroll_offset -= m_tab_visible_rect.left - selected_left;
        else if (selected_right > m_tab_visible_rect.right)
            m_tab_scroll_offset += selected_right - m_tab_visible_rect.right;
        CCommon::SetNumRange(m_tab_scroll_offset, 0, max_scroll_offset);
    }

    int tab_x = m_tab_visible_rect.left + ui->DPI(4) - m_tab_scroll_offset;
    for (int i{}; i < static_cast<int>(m_tab_order.size()); ++i)
    {
        int tab_index = m_tab_order[i];
        std::wstring name = GetTabName(tab_index);
        int tab_width = i < static_cast<int>(tab_widths.size()) ? tab_widths[i] : min_tab_width;
        CRect tab_rect{ tab_x, tab_y, tab_x + tab_width, tab_y + ui->DPI(m_tab_height_config) };
        m_tab_rects.push_back(tab_rect);
        m_tab_indices.push_back(tab_index);

        CRect draw_rect{ tab_rect };
        draw_rect &= m_tab_visible_rect;
        if (!draw_rect.IsRectEmpty())
        {
            COLORREF back_color{ GetTabBackColor(tab_index == m_selected_tab, tab_index == m_hover_tab || (m_tab_dragging && tab_index == m_tab_drag_from)) };

            BYTE tab_alpha = static_cast<BYTE>(ui->IsDrawBackgroundAlpha() ? ALPHA_CHG(theApp.m_app_setting_data.background_transparency) * 2 / 3 : 255);
            if (theApp.m_app_setting_data.button_round_corners)
                ui->GetDrawer().DrawRoundRect(draw_rect, back_color, ui->DPI(4), tab_alpha);
            else
                ui->GetDrawer().FillAlphaRect(draw_rect, back_color, tab_alpha, true);

            CRect text_rect{ tab_rect };
            text_rect.DeflateRect(tab_padding, 0);
            DrawAreaGuard guard(&ui->GetDrawer(), m_tab_visible_rect & text_rect);
            UiFontGuard font_guard(ui, GetTabFontSize(tab_index == m_selected_tab));
            if (IsFavouriteTab(tab_index))
            {
                const int heart_width = ui->DPI(14);
                CRect heart_rect{ text_rect };
                heart_rect.right = heart_rect.left + heart_width;
                ui->GetDrawer().DrawWindowText(heart_rect, L"\u2665", FAVOURITE_TAB_HEART_COLOR, Alignment::LEFT, true);
                text_rect.left += heart_width + ui->DPI(4);
            }
            ui->GetDrawer().DrawWindowText(text_rect, name.c_str(), GetTabTextColor(tab_index == m_selected_tab), Alignment::LEFT, true);
        }
        if (tabs_overflow && (tab_rect.left < m_tab_visible_rect.left || tab_rect.right > m_tab_visible_rect.right))
            m_hidden_tab_indices.push_back(tab_index);
        tab_x += tab_width + tab_gap;
    }

    if (tabs_overflow && !m_tab_menu_button_rect.IsRectEmpty())
    {
        CPlayerUIBase::UIButton menu_button;
        CPoint cursor_pos;
        GetCursorPos(&cursor_pos);
        if (ui->GetOwner() != nullptr)
            ui->GetOwner()->ScreenToClient(&cursor_pos);
        menu_button.hover = m_tab_menu_button_rect.PtInRect(cursor_pos) != FALSE;
        ui->DrawUIButton(m_tab_menu_button_rect, menu_button, IconMgr::IT_Menu, false, std::wstring(), 9, false, Alignment::LEFT, true);
    }

    if (m_tab_dragging && m_tab_drag_drop_index >= 0 && !m_tab_visible_rect.IsRectEmpty())
    {
        int marker_x = m_tab_visible_rect.right;
        for (int i{}; i < static_cast<int>(m_tab_rects.size()); ++i)
        {
            if (i == m_tab_drag_drop_index)
            {
                marker_x = m_tab_rects[i].left;
                break;
            }
        }
        const int marker_left = static_cast<int>(m_tab_visible_rect.left);
        const int marker_right = static_cast<int>(m_tab_visible_rect.right);
        marker_x = (std::max)(marker_left, (std::min)(marker_x, marker_right));
        ui->GetDrawer().DrawLine(CPoint(marker_x, tab_y), CPoint(marker_x, tab_y + ui->DPI(m_tab_height_config)), ui->GetUIColors().color_text, ui->DPI(2), false);
    }

    CRect old_rect{ rect };
    TrackList::DrawScrollArea();
    rect = old_rect;
}

void UiElement::MyPlayerList::NormalizeFolderTabs()
{
    auto& folders = theApp.m_media_lib_setting_data.folder_tab_paths;
    auto& names = theApp.m_media_lib_setting_data.folder_tab_names;
    if (folders.empty())
    {
        std::vector<CMyPlayerListCache::TabInfo> tabs;
        if (CMyPlayerListCache::LoadTabsMeta(tabs))
        {
            for (const auto& tab : tabs)
            {
                folders.push_back(tab.folder_path);
                names.push_back(tab.name);
            }
        }
    }

    if (names.size() < folders.size())
    {
        size_t old_size = names.size();
        names.resize(folders.size());
        for (size_t i{ old_size }; i < folders.size(); ++i)
            names[i] = CFilePathHelper(folders[i]).GetFolderName();
    }
    else if (names.size() > folders.size())
    {
        names.resize(folders.size());
    }

    for (size_t i{}; i < folders.size(); ++i)
    {
        folders[i] = NormalizeFolderPath(folders[i]);
        if (names[i].empty())
            names[i] = CFilePathHelper(folders[i]).GetFolderName();
    }
}

std::wstring UiElement::MyPlayerList::NormalizeFolderPath(const std::wstring& folder_path) const
{
    std::wstring normalized{ folder_path };
    CCommon::StringNormalize(normalized);
    if (!normalized.empty() && normalized.back() != L'\\' && normalized.back() != L'/')
        normalized.push_back(L'\\');
    return normalized;
}

std::wstring UiElement::MyPlayerList::GetFolderTabName(int index) const
{
    const auto& folders = theApp.m_media_lib_setting_data.folder_tab_paths;
    const auto& names = theApp.m_media_lib_setting_data.folder_tab_names;
    if (index < 0 || index >= static_cast<int>(folders.size()))
        return std::wstring();
    if (index < static_cast<int>(names.size()) && !names[index].empty())
        return names[index];

    std::wstring name = CFilePathHelper(folders[index]).GetFolderName();
    if (name.empty())
        name = folders[index];
    return name;
}

std::wstring UiElement::MyPlayerList::GetTabName(int index) const
{
    if (IsFavouriteTab(index))
        return GetFavouriteTabName();
    return GetFolderTabName(index);
}

std::wstring UiElement::MyPlayerList::GetFavouriteTabName() const
{
    return theApp.m_str_table.LoadText(L"TXT_MY_FAVOURITE_SHORT");
}

bool UiElement::MyPlayerList::IsFavouriteTab(int index) const
{
    return index == FAVOURITE_TAB_INDEX;
}

bool UiElement::MyPlayerList::IsFolderTab(int index) const
{
    const auto& folders = theApp.m_media_lib_setting_data.folder_tab_paths;
    return index >= 0 && index < static_cast<int>(folders.size());
}

bool UiElement::MyPlayerList::IsFavouriteTabSelected() const
{
    return IsFavouriteTab(m_selected_tab);
}

void UiElement::MyPlayerList::BuildTabOrder()
{
    NormalizeFolderTabs();
    const auto& folders = theApp.m_media_lib_setting_data.folder_tab_paths;
    const auto& saved_order = theApp.m_media_lib_setting_data.my_player_list_tab_order;

    m_tab_order.clear();
    std::vector<bool> used_folders(folders.size());
    bool favourite_added{ false };

    for (const auto& item : saved_order)
    {
        if (item == FAVOURITE_TAB_ORDER_TOKEN)
        {
            if (m_show_favourite_tab && !favourite_added)
            {
                m_tab_order.push_back(FAVOURITE_TAB_INDEX);
                favourite_added = true;
            }
            continue;
        }

        std::wstring folder_path{ NormalizeFolderPath(item) };
        for (int i{}; i < static_cast<int>(folders.size()); ++i)
        {
            if (!used_folders[i] && CCommon::StringCompareNoCase(NormalizeFolderPath(folders[i]), folder_path))
            {
                m_tab_order.push_back(i);
                used_folders[i] = true;
                break;
            }
        }
    }

    if (m_show_favourite_tab && !favourite_added)
        m_tab_order.insert(m_tab_order.begin(), FAVOURITE_TAB_INDEX);

    for (int i{}; i < static_cast<int>(folders.size()); ++i)
    {
        if (!used_folders[i])
            m_tab_order.push_back(i);
    }
}

void UiElement::MyPlayerList::SaveTabOrder()
{
    auto& saved_order = theApp.m_media_lib_setting_data.my_player_list_tab_order;
    const auto& folders = theApp.m_media_lib_setting_data.folder_tab_paths;
    saved_order.clear();
    for (int tab_index : m_tab_order)
    {
        if (IsFavouriteTab(tab_index))
        {
            if (m_show_favourite_tab)
                saved_order.push_back(FAVOURITE_TAB_ORDER_TOKEN);
        }
        else if (tab_index >= 0 && tab_index < static_cast<int>(folders.size()))
        {
            saved_order.push_back(NormalizeFolderPath(folders[tab_index]));
        }
    }
}

int UiElement::MyPlayerList::GetTabOrderPosition(int index) const
{
    auto iter = std::find(m_tab_order.begin(), m_tab_order.end(), index);
    if (iter == m_tab_order.end())
        return -1;
    return static_cast<int>(iter - m_tab_order.begin());
}

void UiElement::MyPlayerList::InitTabsWithoutLoading()
{
    BuildTabOrder();
    m_selected_tab = m_tab_order.empty() ? NO_TAB_INDEX : m_tab_order.front();
    if (theApp.m_play_setting_data.remember_last_position)
    {
        int playing_tab = GetPlayingSongTabIndex();
        if (playing_tab != NO_TAB_INDEX)
            m_selected_tab = playing_tab;
    }
    if (m_selected_tab != NO_TAB_INDEX)
        SelectFolderTab(m_selected_tab, true);
    else
    {
        m_cached_tracks.clear();
        m_cached_item_height = 0;
        m_cached_list_font_size = 0;
        ClearListItem();
    }
}

void UiElement::MyPlayerList::RefreshTabs(bool keep_selection)
{
    int old_selected_tab{ m_selected_tab };
    BuildTabOrder();
    if (m_tab_order.empty())
    {
        m_selected_tab = NO_TAB_INDEX;
        m_cached_tracks.clear();
        m_cached_item_height = 0;
        m_cached_list_font_size = 0;
        ClearListItem();
        return;
    }

    if (!keep_selection || GetTabOrderPosition(old_selected_tab) < 0)
        m_selected_tab = m_tab_order.front();
    else
        m_selected_tab = old_selected_tab;
    SelectFolderTab(m_selected_tab, true);
}

void UiElement::MyPlayerList::SelectFolderTab(int index, bool refresh_list)
{
    if (GetTabOrderPosition(index) < 0)
        return;

    m_selected_tab = index;
    EnsureTabVisible(index);
    if (refresh_list)
    {
        if (IsFavouriteTab(index))
            SetFavouriteSongs();
        else
            SetFolderSongs(index);
    }
}

int UiElement::MyPlayerList::GetPlayingSongTabIndex() const
{
    if (m_show_favourite_tab && CRecentList::Instance().IsPlayingSpecPlaylist(CRecentList::PT_FAVOURITE))
        return FAVOURITE_TAB_INDEX;

    std::wstring song_path;
    const SongInfo& current_song = CPlayer::GetInstance().GetSafeCurrentSongInfo();
    if (!current_song.IsEmpty() && !current_song.file_path.empty())
    {
        song_path = current_song.file_path;
    }
    else
    {
        ListItem current_list = CRecentList::Instance().GetCurrentList();
        song_path = current_list.last_track.path;
    }

    if (song_path.empty())
        return NO_TAB_INDEX;

    CCommon::StringNormalize(song_path);
    std::replace(song_path.begin(), song_path.end(), L'/', L'\\');
    const auto& folders = theApp.m_media_lib_setting_data.folder_tab_paths;
    int best_index{ NO_TAB_INDEX };
    size_t best_length{};
    for (int i{}; i < static_cast<int>(folders.size()); ++i)
    {
        std::wstring folder_path{ NormalizeFolderPath(folders[i]) };
        std::replace(folder_path.begin(), folder_path.end(), L'/', L'\\');
        if (!folder_path.empty() && CCommon::StringFindNoCase(song_path, folder_path, 0) == 0 && folder_path.size() > best_length)
        {
            best_index = i;
            best_length = folder_path.size();
        }
    }
    return best_index;
}

int UiElement::MyPlayerList::HitTestTab(CPoint point) const
{
    if (!m_tab_visible_rect.IsRectEmpty() && !m_tab_visible_rect.PtInRect(point))
        return -1;
    for (int i{}; i < static_cast<int>(m_tab_rects.size()); ++i)
    {
        if (m_tab_rects[i].PtInRect(point))
            return i < static_cast<int>(m_tab_indices.size()) ? m_tab_indices[i] : -1;
    }
    return -1;
}

bool UiElement::MyPlayerList::HitTestTabMenuButton(CPoint point) const
{
    return !m_hidden_tab_indices.empty() && !m_tab_menu_button_rect.IsRectEmpty() && m_tab_menu_button_rect.PtInRect(point);
}

bool UiElement::MyPlayerList::HitTestTabAddButton(CPoint point) const
{
    return m_tab_order.empty() && !m_tab_add_button_rect.IsRectEmpty() && m_tab_add_button_rect.PtInRect(point);
}

void UiElement::MyPlayerList::ShowHiddenTabsMenu()
{
    if (m_hidden_tab_indices.empty())
        return;

    CMenu menu;
    menu.CreatePopupMenu();
    for (size_t i{}; i < m_hidden_tab_indices.size(); ++i)
    {
        int tab_index = m_hidden_tab_indices[i];
        menu.AppendMenuW(MF_STRING, TAB_MENU_COMMAND_BASE + static_cast<UINT>(i), GetTabName(tab_index).c_str());
    }

    CPoint point{ m_tab_menu_button_rect.left, m_tab_menu_button_rect.bottom };
    if (ui->GetOwner() != nullptr)
        ui->GetOwner()->ClientToScreen(&point);
    UINT command = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, point.x, point.y, theApp.m_pMainWnd);
    if (command >= TAB_MENU_COMMAND_BASE && command < TAB_MENU_COMMAND_BASE + m_hidden_tab_indices.size())
    {
        int tab_index = m_hidden_tab_indices[command - TAB_MENU_COMMAND_BASE];
        SelectFolderTab(tab_index, true);
    }
}

int UiElement::MyPlayerList::GetTabDropIndex(CPoint point) const
{
    if (m_tab_order.empty())
        return -1;

    for (int i{}; i < static_cast<int>(m_tab_rects.size()); ++i)
    {
        if (point.x < m_tab_rects[i].CenterPoint().x)
            return i;
    }
    return static_cast<int>(m_tab_order.size());
}

void UiElement::MyPlayerList::ReorderFolderTab(int from, int insert_before)
{
    BuildTabOrder();
    const int from_pos = GetTabOrderPosition(from);
    if (from_pos < 0)
        return;

    const int count = static_cast<int>(m_tab_order.size());
    CCommon::SetNumRange(insert_before, 0, count);
    if (insert_before == from_pos || insert_before == from_pos + 1)
        return;

    int selected_tab{ m_selected_tab };
    int tab_index{ m_tab_order[from_pos] };
    m_tab_order.erase(m_tab_order.begin() + from_pos);
    if (insert_before > from_pos)
        --insert_before;
    m_tab_order.insert(m_tab_order.begin() + insert_before, tab_index);
    m_selected_tab = selected_tab;
    SaveTabOrder();
    SaveFolderTabSettings();
    EnsureTabVisible(m_selected_tab);
}

void UiElement::MyPlayerList::EnsureTabVisible(int index)
{
    if (index == NO_TAB_INDEX || m_tab_visible_rect.IsRectEmpty())
        return;

    for (int i{}; i < static_cast<int>(m_tab_rects.size()); ++i)
    {
        if (i >= static_cast<int>(m_tab_indices.size()) || m_tab_indices[i] != index)
            continue;

        if (m_tab_rects[i].left < m_tab_visible_rect.left)
            m_tab_scroll_offset -= m_tab_visible_rect.left - m_tab_rects[i].left;
        else if (m_tab_rects[i].right > m_tab_visible_rect.right)
            m_tab_scroll_offset += m_tab_rects[i].right - m_tab_visible_rect.right;

        const int max_scroll_offset = (std::max)(0, m_tab_total_width - (m_tab_visible_rect.Width() - ui->DPI(4)));
        CCommon::SetNumRange(m_tab_scroll_offset, 0, max_scroll_offset);
        return;
    }
}

void UiElement::MyPlayerList::ResetTabDragState()
{
    m_tab_pressed = -1;
    m_tab_drag_from = -1;
    m_tab_drag_drop_index = -1;
    m_tab_dragging = false;
}

int UiElement::MyPlayerList::GetTabPadding() const
{
    return ui->DPI((std::max)(0, m_tab_padding_config));
}

int UiElement::MyPlayerList::GetTabFontSize(bool selected) const
{
    int font_size = selected ? m_tab_selected_font_size : m_tab_unselected_font_size;
    if (font_size <= 0)
        font_size = EffectiveFontSize();
    CCommon::SetNumRange(font_size, 5, 72);
    return font_size;
}

COLORREF UiElement::MyPlayerList::GetTabTextColor(bool selected) const
{
    const TabColor& color = selected ? m_tab_selected_text_color : m_tab_unselected_text_color;
    if (color.set)
        return color.color;
    return ListTextColor();
}

COLORREF UiElement::MyPlayerList::GetTabBackColor(bool selected, bool hover) const
{
    if (selected)
        return m_tab_selected_background_color.set ? m_tab_selected_background_color.color : ui->GetUIColors().color_list_selected;
    if (hover)
        return ui->GetUIColors().color_button_hover;
    return m_tab_unselected_background_color.set ? m_tab_unselected_background_color.color : ui->GetUIColors().color_back;
}

bool UiElement::MyPlayerList::ParseTabColor(tinyxml2::XMLElement* xml_node, const char* attr_name, TabColor& color)
{
    std::string str_color = CTinyXml2Helper::ElementAttribute(xml_node, attr_name);
    if (str_color.empty())
        return false;

    int base = 10;
    if (str_color[0] == '#')
    {
        str_color.erase(0, 1);
        base = 16;
    }
    else if (str_color.size() > 2 && str_color[0] == '0' && (str_color[1] == 'x' || str_color[1] == 'X'))
    {
        str_color.erase(0, 2);
        base = 16;
    }

    char* end_ptr{};
    unsigned long value = std::strtoul(str_color.c_str(), &end_ptr, base);
    if (end_ptr == str_color.c_str() || *end_ptr != '\0' || value > 0xFFFFFF)
        return false;

    color.color = RGB((value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF);
    color.set = true;
    return true;
}

void UiElement::MyPlayerList::ShowTabContextMenu(int index)
{
    if (index != NO_TAB_INDEX)
        SelectFolderTab(index, false);

    CMenu menu;
    menu.CreatePopupMenu();
    menu.AppendMenuW(MF_STRING, ID_FILE_OPEN_FOLDER, theApp.m_str_table.LoadText(L"TXT_FOLDER_TAB_ADD").c_str());
    if (IsFolderTab(index))
    {
        menu.AppendMenuW(MF_STRING, ID_RENAME, theApp.m_str_table.LoadText(L"TXT_FOLDER_TAB_RENAME").c_str());
        menu.AppendMenuW(MF_STRING, ID_RELOAD_PLAYLIST, theApp.m_str_table.LoadText(L"TXT_FOLDER_TAB_REFRESH").c_str());
        menu.AppendMenuW(MF_SEPARATOR);
        menu.AppendMenuW(MF_STRING, ID_DELETE_PATH, theApp.m_str_table.LoadText(L"TXT_FOLDER_TAB_DELETE").c_str());
    }

    CPoint point;
    GetCursorPos(&point);
    UINT command = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, point.x, point.y, theApp.m_pMainWnd);
    if (command == ID_FILE_OPEN_FOLDER)
        AddFolder();
    else if (command == ID_RENAME && IsFolderTab(index))
        RenameFolder(index);
    else if (command == ID_DELETE_PATH && IsFolderTab(index))
        DeleteFolder(index);
    else if (command == ID_RELOAD_PLAYLIST && IsFolderTab(index))
        RefreshFolder(index);
}

bool UiElement::MyPlayerList::SelectFolderPath(std::wstring& folder_path)
{
    CFolderBrowserDlg dlg(theApp.m_pMainWnd->GetSafeHwnd());
    dlg.SetInfo(theApp.m_str_table.LoadText(L"TITLE_FOLDER_BROWSER_SONG_SOURCE").c_str());
    if (dlg.DoModal() != IDOK)
        return false;

    folder_path = NormalizeFolderPath(dlg.GetPathName().GetString());
    if (folder_path.empty())
        return false;
    return true;
}

void UiElement::MyPlayerList::AddFolder()
{
    std::wstring folder_path;
    if (!SelectFolderPath(folder_path))
        return;

    auto& media_folders = theApp.m_media_lib_setting_data.media_folders;
    if (std::find(media_folders.begin(), media_folders.end(), folder_path) == media_folders.end())
        media_folders.push_back(folder_path);

    auto& tab_paths = theApp.m_media_lib_setting_data.folder_tab_paths;
    auto& tab_names = theApp.m_media_lib_setting_data.folder_tab_names;
    auto iter = std::find(tab_paths.begin(), tab_paths.end(), folder_path);
    if (iter == tab_paths.end())
    {
        tab_paths.push_back(folder_path);
        tab_names.push_back(CFilePathHelper(folder_path).GetFolderName());
        m_selected_tab = static_cast<int>(tab_paths.size()) - 1;
    }
    else
    {
        m_selected_tab = static_cast<int>(iter - tab_paths.begin());
    }

    BuildTabOrder();
    if (GetTabOrderPosition(m_selected_tab) < 0)
        m_tab_order.push_back(m_selected_tab);
    SaveTabOrder();
    SaveFolderTabSettings();
    SetFolderSongs(m_selected_tab, true);
}

void UiElement::MyPlayerList::RenameFolder(int index)
{
    auto& folders = theApp.m_media_lib_setting_data.folder_tab_paths;
    if (index < 0 || index >= static_cast<int>(folders.size()))
        return;

    NormalizeFolderTabs();
    CInputDlg dlg(theApp.m_pMainWnd);
    dlg.SetTitle(theApp.m_str_table.LoadText(L"TXT_FOLDER_TAB_RENAME").c_str());
    dlg.SetInfoText(theApp.m_str_table.LoadText(L"TXT_FOLDER_TAB_NAME").c_str());
    dlg.SetEditText(GetFolderTabName(index).c_str());
    if (dlg.DoModal() != IDOK)
        return;

    std::wstring name = dlg.GetEditText().GetString();
    CCommon::StringNormalize(name);
    if (name.empty())
        return;

    theApp.m_media_lib_setting_data.folder_tab_names[index] = name;
    CMyPlayerListCache::UpdateTabName(folders[index], name);
    SaveFolderTabSettings();
}

void UiElement::MyPlayerList::DeleteFolder(int index)
{
    auto& paths = theApp.m_media_lib_setting_data.folder_tab_paths;
    auto& names = theApp.m_media_lib_setting_data.folder_tab_names;
    if (index < 0 || index >= static_cast<int>(paths.size()))
        return;

    std::wstring info = theApp.m_str_table.LoadTextFormat(L"MSG_DELETE_MEDIALIB_FOLDER_INQUIRY", { GetFolderTabName(index) });
    if (AfxMessageBox(info.c_str(), MB_ICONQUESTION | MB_YESNO) != IDYES)
        return;

    std::wstring folder_path{ paths[index] };
    paths.erase(paths.begin() + index);
    if (index < static_cast<int>(names.size()))
        names.erase(names.begin() + index);
    CMyPlayerListCache::DeleteTab(folder_path);
    if (m_selected_tab == index)
        m_selected_tab = NO_TAB_INDEX;
    else if (m_selected_tab > index)
        --m_selected_tab;
    BuildTabOrder();
    if (m_selected_tab == NO_TAB_INDEX && !m_tab_order.empty())
        m_selected_tab = m_tab_order.front();
    SaveTabOrder();
    SaveFolderTabSettings();

    if (m_tab_order.empty())
    {
        m_selected_tab = NO_TAB_INDEX;
        m_cached_tracks.clear();
        m_cached_item_height = 0;
        m_cached_list_font_size = 0;
        ClearListItem();
    }
    else
    {
        RefreshTabs(true);
    }
}

void UiElement::MyPlayerList::RefreshFolder(int index)
{
    if (IsFolderTab(index))
        SetFolderSongs(index, true);
}

void UiElement::MyPlayerList::SaveFolderTabSettings()
{
    SaveTabOrder();
    CMusicPlayerDlg* pMainWnd = CMusicPlayerDlg::GetInstance();
    if (pMainWnd != nullptr)
        pMainWnd->SaveConfigNow();
}

ListItem UiElement::MyPlayerList::GetFolderListItem(int index) const
{
    ListItem list_item;
    const auto& folders = theApp.m_media_lib_setting_data.folder_tab_paths;
    if (index >= 0 && index < static_cast<int>(folders.size()))
    {
        list_item = ListItem{ LT_FOLDER, folders[index] };
        list_item.contain_sub_folder = true;
    }
    return list_item;
}

void UiElement::MyPlayerList::SetFolderSongs(int index, bool force_refresh)
{
    const auto& folders = theApp.m_media_lib_setting_data.folder_tab_paths;
    if (index < 0 || index >= static_cast<int>(folders.size()))
    {
        return;
    }

    m_list_item = GetFolderListItem(index);
    m_selected_tab = index;
    SelectNone();

    if (!force_refresh && LoadFolderSongsFromCache(index))
    {
        scroll_offset = 0;
        LocatePlayingSongInCurrentTab();
        return;
    }

    std::vector<SongInfo> song_list;
    CAudioCommon::GetAudioFiles(folders[index], song_list, MAX_SONG_NUM, true);
    int update_cnt{};
    bool exit_flag{};
    int process_percent{};
    CAudioCommon::GetAudioInfo(song_list, update_cnt, exit_flag, process_percent, MR_MIN_REQUIRED);

    auto sort_fun = SongInfo::GetSortFunc(m_list_item.sort_mode == SM_UNSORT ? SM_U_FILE : m_list_item.sort_mode);
    std::stable_sort(song_list.begin(), song_list.end(), sort_fun);

    for (auto& cur_song : song_list)
        cur_song = CSongDataManager::GetInstance().GetSongInfo3(cur_song);

    CMyPlayerListCache::TabInfo tab = CMyPlayerListCache::BuildTab(folders[index], GetFolderTabName(index), m_list_item.sort_mode, song_list);
    CMyPlayerListCache::SaveTab(tab);
    ApplyCacheTab(tab);
    scroll_offset = 0;
    LocatePlayingSongInCurrentTab();
}

void UiElement::MyPlayerList::SetFavouriteSongs()
{
    m_list_item = CRecentList::Instance().GetSpecPlaylist(CRecentList::PT_FAVOURITE);
    m_selected_tab = FAVOURITE_TAB_INDEX;
    SelectNone();

    std::vector<SongInfo> song_list;
    CUiMyFavouriteItemMgr::Instance().GetSongList(song_list);

    m_cached_tracks.clear();
    m_cached_tracks.reserve(song_list.size());
    std::vector<CUISongListMgr::UTrackInfo> track_list;
    track_list.reserve(song_list.size());
    for (const auto& song_info : song_list)
    {
        CMyPlayerListCache::TrackInfo cached_track;
        cached_track.file_path = song_info.file_path;
        cached_track.display_name = CFilePathHelper(song_info.file_path).GetFileNameWithoutExtension();
        cached_track.duration_text = song_info.length().toString();
        m_cached_tracks.push_back(cached_track);

        CUISongListMgr::UTrackInfo track;
        track.song_key = SongKey(song_info);
        track.name = cached_track.display_name;
        track.length = song_info.length();
        track.is_favourite = true;
        track_list.push_back(track);
    }

    m_cached_item_height = CalculateSongListItemHeight(theApp.m_app_setting_data.song_list_font_size);
    m_cached_list_font_size = theApp.m_app_setting_data.song_list_font_size;
    m_ui_song_list->UpdateCached(track_list);
    scroll_offset = 0;
    LocatePlayingSongInCurrentTab();
}

bool UiElement::MyPlayerList::LocatePlayingSongInCurrentTab()
{
    SongKey playing_key;
    const SongInfo& current_song = CPlayer::GetInstance().GetSafeCurrentSongInfo();
    if (!current_song.IsEmpty() && !current_song.file_path.empty())
        playing_key = SongKey(current_song);
    else
        playing_key = CRecentList::Instance().GetCurrentList().last_track;

    int row_count = GetRowCount();
    for (int i{}; i < row_count; ++i)
    {
        bool is_playing_row{ IsHighlightRow(i) };
        if (!is_playing_row && !playing_key.path.empty())
        {
            CUISongListMgr* song_list = GetSongListData();
            if (song_list != nullptr && i < song_list->GetSongCount())
                is_playing_row = std::equal_to<SongKey>()(song_list->GetItem(i).song_key, playing_key);
        }
        if (is_playing_row)
        {
            CRect old_rect{ rect };
            CRect list_rect{ GetListRect() };
            if (!list_rect.IsRectEmpty())
                SetRect(list_rect);
            scroll_offset = i * ItemHeight();
            RestrictOffset();
            SetItemSelected(i);
            rect = old_rect;
            return true;
        }
    }
    return false;
}

bool UiElement::MyPlayerList::LoadFolderSongsFromCache(int index)
{
    const auto& folders = theApp.m_media_lib_setting_data.folder_tab_paths;
    if (index < 0 || index >= static_cast<int>(folders.size()))
        return false;

    CMyPlayerListCache::TabInfo tab;
    if (!CMyPlayerListCache::LoadTab(folders[index], tab))
        return false;

    const int item_height = CalculateSongListItemHeight(theApp.m_app_setting_data.song_list_font_size);
    if (tab.list_font_size != theApp.m_app_setting_data.song_list_font_size || tab.item_height != item_height)
    {
        tab.list_font_size = theApp.m_app_setting_data.song_list_font_size;
        tab.item_height = item_height;
        CMyPlayerListCache::SaveTab(tab);
    }

    ApplyCacheTab(tab);
    return true;
}

void UiElement::MyPlayerList::ApplyCacheTab(const CMyPlayerListCache::TabInfo& tab)
{
    m_cached_tracks.clear();
    m_cached_tracks.reserve(tab.tracks.size());
    for (const auto& item : tab.tracks)
    {
        if (!item.file_path.empty())
            m_cached_tracks.push_back(item);
    }
    m_cached_item_height = tab.item_height;
    m_cached_list_font_size = tab.list_font_size;

    std::vector<CUISongListMgr::UTrackInfo> track_list;
    track_list.reserve(m_cached_tracks.size());
    for (const auto& item : m_cached_tracks)
    {
        CUISongListMgr::UTrackInfo track;
        SongInfo song;
        song.file_path = item.file_path;
        track.song_key = SongKey(song);
        track.name = item.display_name;
        track.is_favourite = CUiMyFavouriteItemMgr::Instance().Contains(song);
        track_list.push_back(track);
    }
    m_ui_song_list->UpdateCached(track_list);
}

CRect UiElement::MyPlayerList::GetListRect() const
{
    CRect list_rect{ m_element_rect };
    list_rect.top += m_tab_height + ui->DPI(4);
    return list_rect;
}
