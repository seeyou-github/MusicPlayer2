#pragma once
#include "UIElement/TracksList.h"
#include "MyPlayerListCache.h"

namespace UiElement
{
    // Folder tab playlist.
    class MyPlayerList : public TrackList
    {
    public:
        virtual void Draw() override;
        virtual bool LButtonDown(CPoint point) override;
        virtual bool RButtonUp(CPoint point) override;
        virtual bool LButtonUp(CPoint point) override;
        virtual bool DoubleClick(CPoint point) override;
        virtual bool MouseMove(CPoint point) override;
        virtual bool MouseLeave() override;
        virtual int GetScrollAreaHeight() override;
        virtual void DrawScrollArea() override;
        virtual std::wstring GetItemText(int row, int col) override;
        virtual int GetSongListCachedItemHeight() const override;
        virtual void FromXmlNode(tinyxml2::XMLElement* xml_node) override;

    private:
        struct TabColor
        {
            bool set{};
            COLORREF color{};
        };

        void NormalizeFolderTabs();
        std::wstring NormalizeFolderPath(const std::wstring& folder_path) const;
        std::wstring GetFolderTabName(int index) const;
        void InitTabsWithoutLoading();
        void RefreshTabs(bool keep_selection);
        void SelectFolderTab(int index, bool refresh_list);
        int GetPlayingSongTabIndex() const;
        int HitTestTab(CPoint point) const;
        bool HitTestTabMenuButton(CPoint point) const;
        bool HitTestTabAddButton(CPoint point) const;
        void ShowHiddenTabsMenu();
        int GetTabDropIndex(CPoint point) const;
        void ReorderFolderTab(int from, int insert_before);
        void EnsureTabVisible(int index);
        void ResetTabDragState();
        int GetTabPadding() const;
        int GetTabFontSize(bool selected) const;
        COLORREF GetTabTextColor(bool selected) const;
        COLORREF GetTabBackColor(bool selected, bool hover) const;
        bool ParseTabColor(tinyxml2::XMLElement* xml_node, const char* attr_name, TabColor& color);
        void ShowTabContextMenu(int index);
        bool SelectFolderPath(std::wstring& folder_path);
        void AddFolder();
        void RenameFolder(int index);
        void DeleteFolder(int index);
        void RefreshFolder(int index);
        void SaveFolderTabSettings();
        ListItem GetFolderListItem(int index) const;
        void SetFolderSongs(int index, bool force_refresh = false);
        bool LocatePlayingSongInCurrentTab();
        bool LoadFolderSongsFromCache(int index);
        void ApplyCacheTab(const CMyPlayerListCache::TabInfo& tab);
        CRect GetListRect() const;

    private:
        CRect m_element_rect;
        std::vector<CRect> m_tab_rects;
        std::vector<int> m_tab_indices;
        std::vector<int> m_hidden_tab_indices;
        CRect m_tab_visible_rect;
        CRect m_tab_menu_button_rect;
        CRect m_tab_add_button_rect;
        int m_selected_tab{ -1 };
        int m_hover_tab{ -1 };
        int m_tab_height{};
        int m_tab_scroll_offset{};
        int m_tab_total_width{};
        int m_tab_pressed{ -1 };
        int m_tab_drag_from{ -1 };
        int m_tab_drag_drop_index{ -1 };
        CPoint m_tab_drag_start_point;
        bool m_tab_dragging{};
        bool m_tabs_inited{};
        int m_cached_item_height{};
        int m_cached_list_font_size{};
        std::vector<CMyPlayerListCache::TrackInfo> m_cached_tracks;
        int m_tab_height_config{ 32 };
        int m_tab_margin_top{};
        int m_tab_margin_left{};
        int m_tab_margin_right{};
        int m_tab_padding_config{ 18 };
        int m_tab_selected_font_size{};
        int m_tab_unselected_font_size{};
        TabColor m_tab_background_color;
        TabColor m_tab_selected_background_color;
        TabColor m_tab_unselected_background_color;
        TabColor m_tab_selected_text_color;
        TabColor m_tab_unselected_text_color;
    };
}
