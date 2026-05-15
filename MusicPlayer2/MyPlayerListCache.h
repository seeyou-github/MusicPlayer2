#pragma once
#include "SongInfo.h"

class CMyPlayerListCache
{
public:
    static constexpr int CACHE_VERSION{ 1 };

    struct TrackInfo
    {
        std::wstring file_path;
        std::wstring display_name;
        std::wstring duration_text;
        unsigned long long modified_time{};
        unsigned long long file_size{};
    };

    struct TabInfo
    {
        std::wstring name;
        std::wstring folder_path;
        std::wstring cache_file;
        int sort_mode{};
        int list_font_size{};
        int item_height{};
        int cache_version{ CACHE_VERSION };
        bool include_sub_folder{ true };
        std::vector<TrackInfo> tracks;
    };

    static std::wstring GetTabsMetaPath();
    static std::wstring GetTabCacheFileName(const std::wstring& folder_path);
    static std::wstring GetTabCachePath(const std::wstring& cache_file);

    static bool LoadTabsMeta(std::vector<TabInfo>& tabs);
    static bool SaveTabsMeta(const std::vector<TabInfo>& tabs);
    static bool LoadTab(const std::wstring& folder_path, TabInfo& tab);
    static bool SaveTab(const TabInfo& tab);
    static bool DeleteTab(const std::wstring& folder_path);
    static void UpdateTabName(const std::wstring& folder_path, const std::wstring& name);
    static void UpdateAllTabMetrics(int list_font_size, int item_height);

    static TabInfo BuildTab(const std::wstring& folder_path, const std::wstring& name, int sort_mode, const std::vector<SongInfo>& song_list);

private:
    static std::wstring DataDir();
    static bool IsCacheValid(const TabInfo& tab, const std::wstring& folder_path);
    static void UpsertTabsMeta(const TabInfo& tab);
    static bool GetFileState(const std::wstring& file_path, unsigned long long& modified_time, unsigned long long& file_size);
};
