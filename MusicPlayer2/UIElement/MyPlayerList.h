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
        virtual bool IsHighlightRow(int row) override;
        virtual CMenu* GetContextMenu(bool item_selected) override;
        virtual void OnDoubleClicked() override;
        virtual void OnHoverButtonClicked(int btn_index, int row) override;
        virtual std::wstring GetEmptyString() override;
        virtual int GetSongListCachedItemHeight() const override;
        virtual void FromXmlNode(tinyxml2::XMLElement* xml_node) override;

    private:
        void NormalizeFolderTabs();
        std::wstring NormalizeFolderPath(const std::wstring& folder_path) const;
        std::wstring GetFolderTabName(int index) const;
        std::wstring GetTabName(int index) const;
        std::wstring GetFavouriteTabName() const;
        bool IsFavouriteTab(int index) const;
        bool IsFolderTab(int index) const;
        bool IsFavouriteTabSelected() const;
        void BuildTabOrder();
        void SaveTabOrder();
        int GetTabOrderPosition(int index) const;
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
        void ShowTabContextMenu(int index);
        bool SelectFolderPath(std::wstring& folder_path);
        void AddFolder();
        void RenameFolder(int index);
        void DeleteFolder(int index);
        void RefreshFolder(int index);
        void MoveFilesToFolderTab(int target_tab_index);
        void SaveFolderTabSettings();
        ListItem GetFolderListItem(int index) const;
        void SetFolderSongs(int index, bool force_refresh = false);
        void SetFavouriteSongs();
        bool LocatePlayingSongInCurrentTab();
        bool LoadFolderSongsFromCache(int index);
        void ApplyCacheTab(const CMyPlayerListCache::TabInfo& tab);
        CRect GetListRect() const;
        void EnsureHighlightItemVisible() override;

    private:
        CRect m_element_rect;
        std::vector<CRect> m_tab_rects;
        std::vector<int> m_tab_order;
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
        bool m_show_favourite_tab{ true };
    };
}
