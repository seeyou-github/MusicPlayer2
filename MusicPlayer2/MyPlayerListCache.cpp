#include "stdafx.h"
#include "MyPlayerListCache.h"
#include "MusicPlayer2.h"
#include "Common.h"
#include "FilePathHelper.h"
#include "md5.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace
{
    std::string ToUtf8(const std::wstring& str)
    {
        return CCommon::UnicodeToStr(str, CodeType::UTF8_NO_BOM);
    }

    std::wstring FromUtf8(const std::string& str)
    {
        return CCommon::StrToUnicode(str, CodeType::UTF8_NO_BOM);
    }

    std::wstring JsonWString(const json& data, const char* key)
    {
        if (!data.contains(key) || !data[key].is_string())
            return std::wstring();
        return FromUtf8(data[key].get<std::string>());
    }

    int JsonInt(const json& data, const char* key, int default_value = 0)
    {
        if (!data.contains(key) || !data[key].is_number_integer())
            return default_value;
        return data[key].get<int>();
    }

    unsigned long long JsonUInt64(const json& data, const char* key)
    {
        if (!data.contains(key) || !data[key].is_number_unsigned())
            return 0;
        return data[key].get<unsigned long long>();
    }

    bool JsonBool(const json& data, const char* key, bool default_value = false)
    {
        if (!data.contains(key) || !data[key].is_boolean())
            return default_value;
        return data[key].get<bool>();
    }

    json TrackToJson(const CMyPlayerListCache::TrackInfo& item)
    {
        return json{
            {"file_path", ToUtf8(item.file_path)},
            {"display_name", ToUtf8(item.display_name)},
            {"duration_text", ToUtf8(item.duration_text)},
            {"modified_time", item.modified_time},
            {"file_size", item.file_size}
        };
    }

    CMyPlayerListCache::TrackInfo TrackFromJson(const json& data)
    {
        CMyPlayerListCache::TrackInfo item;
        item.file_path = JsonWString(data, "file_path");
        item.display_name = JsonWString(data, "display_name");
        item.duration_text = JsonWString(data, "duration_text");
        item.modified_time = JsonUInt64(data, "modified_time");
        item.file_size = JsonUInt64(data, "file_size");
        return item;
    }

    json TabMetaToJson(const CMyPlayerListCache::TabInfo& tab)
    {
        return json{
            {"name", ToUtf8(tab.name)},
            {"folder_path", ToUtf8(tab.folder_path)},
            {"cache_file", ToUtf8(tab.cache_file)},
            {"sort_mode", tab.sort_mode},
            {"list_font_size", tab.list_font_size},
            {"item_height", tab.item_height},
            {"cache_version", tab.cache_version},
            {"include_sub_folder", tab.include_sub_folder}
        };
    }

    CMyPlayerListCache::TabInfo TabMetaFromJson(const json& data)
    {
        CMyPlayerListCache::TabInfo tab;
        tab.name = JsonWString(data, "name");
        tab.folder_path = JsonWString(data, "folder_path");
        tab.cache_file = JsonWString(data, "cache_file");
        tab.sort_mode = JsonInt(data, "sort_mode");
        tab.list_font_size = JsonInt(data, "list_font_size");
        tab.item_height = JsonInt(data, "item_height");
        tab.cache_version = JsonInt(data, "cache_version");
        tab.include_sub_folder = JsonBool(data, "include_sub_folder", true);
        return tab;
    }
}

std::wstring CMyPlayerListCache::DataDir()
{
    std::wstring data_dir{ theApp.m_data_dir };
    if (data_dir.empty())
        data_dir = CCommon::GetExePath() + L"AppData\\data\\";
    CCommon::CreateDir(CCommon::GetExePath() + L"AppData\\");
    CCommon::CreateDir(data_dir);
    return data_dir;
}

std::wstring CMyPlayerListCache::GetTabsMetaPath()
{
    return DataDir() + L"myplayerlist_tabs.dat";
}

std::wstring CMyPlayerListCache::GetTabCacheFileName(const std::wstring& folder_path)
{
    MD5 md5;
    md5.Update(folder_path);
    md5.Finalize();
    return L"myplayerlist_" + FromUtf8(md5.HexDigest()) + L".dat";
}

std::wstring CMyPlayerListCache::GetTabCachePath(const std::wstring& cache_file)
{
    return DataDir() + cache_file;
}

bool CMyPlayerListCache::LoadTabsMeta(std::vector<TabInfo>& tabs)
{
    tabs.clear();
    const std::wstring file_path{ GetTabsMetaPath() };
    if (!CCommon::FileExist(file_path))
        return false;

    try
    {
        std::ifstream stream{ file_path.c_str(), std::ios::binary };
        json data = json::parse(stream, nullptr, false);
        if (data.is_discarded() || !data.contains("tabs") || !data["tabs"].is_array())
            return false;

        for (const auto& item : data["tabs"])
        {
            TabInfo tab{ TabMetaFromJson(item) };
            if (!tab.folder_path.empty())
            {
                if (tab.cache_file.empty())
                    tab.cache_file = GetTabCacheFileName(tab.folder_path);
                tabs.push_back(tab);
            }
        }
        return true;
    }
    catch (...)
    {
        tabs.clear();
        return false;
    }
}

bool CMyPlayerListCache::SaveTabsMeta(const std::vector<TabInfo>& tabs)
{
    try
    {
        json data;
        data["cache_version"] = CACHE_VERSION;
        data["tabs"] = json::array();
        for (const auto& tab : tabs)
            data["tabs"].push_back(TabMetaToJson(tab));

        std::ofstream stream{ GetTabsMetaPath().c_str(), std::ios::binary | std::ios::trunc };
        stream << data.dump(2);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool CMyPlayerListCache::LoadTab(const std::wstring& folder_path, TabInfo& tab)
{
    tab = TabInfo{};
    const std::wstring cache_file{ GetTabCacheFileName(folder_path) };
    const std::wstring file_path{ GetTabCachePath(cache_file) };
    if (!CCommon::FileExist(file_path))
        return false;

    try
    {
        std::ifstream stream{ file_path.c_str(), std::ios::binary };
        json data = json::parse(stream, nullptr, false);
        if (data.is_discarded())
            return false;

        tab = TabMetaFromJson(data);
        if (tab.cache_file.empty())
            tab.cache_file = cache_file;
        if (!IsCacheValid(tab, folder_path))
            return false;

        if (data.contains("tracks") && data["tracks"].is_array())
        {
            tab.tracks.reserve(data["tracks"].size());
            for (const auto& item : data["tracks"])
                tab.tracks.push_back(TrackFromJson(item));
        }
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool CMyPlayerListCache::SaveTab(const TabInfo& tab)
{
    try
    {
        TabInfo tab_to_save{ tab };
        if (tab_to_save.cache_file.empty())
            tab_to_save.cache_file = GetTabCacheFileName(tab_to_save.folder_path);

        json data = TabMetaToJson(tab_to_save);
        data["tracks"] = json::array();
        for (const auto& item : tab_to_save.tracks)
            data["tracks"].push_back(TrackToJson(item));

        std::ofstream stream{ GetTabCachePath(tab_to_save.cache_file).c_str(), std::ios::binary | std::ios::trunc };
        stream << data.dump(2);
        UpsertTabsMeta(tab_to_save);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool CMyPlayerListCache::DeleteTab(const std::wstring& folder_path)
{
    std::vector<TabInfo> tabs;
    LoadTabsMeta(tabs);
    const std::wstring cache_file{ GetTabCacheFileName(folder_path) };
    tabs.erase(std::remove_if(tabs.begin(), tabs.end(), [&](const TabInfo& tab) {
        return tab.folder_path == folder_path;
    }), tabs.end());
    SaveTabsMeta(tabs);
    ::DeleteFileW(GetTabCachePath(cache_file).c_str());
    return true;
}

void CMyPlayerListCache::UpdateTabName(const std::wstring& folder_path, const std::wstring& name)
{
    TabInfo tab;
    if (LoadTab(folder_path, tab))
    {
        tab.name = name;
        SaveTab(tab);
        return;
    }

    std::vector<TabInfo> tabs;
    if (LoadTabsMeta(tabs))
    {
        for (auto& item : tabs)
        {
            if (item.folder_path == folder_path)
                item.name = name;
        }
        SaveTabsMeta(tabs);
    }
}

void CMyPlayerListCache::UpdateAllTabMetrics(int list_font_size, int item_height)
{
    std::vector<TabInfo> tabs;
    if (!LoadTabsMeta(tabs))
        return;

    bool meta_changed{};
    for (auto& meta : tabs)
    {
        if (meta.list_font_size != list_font_size || meta.item_height != item_height)
        {
            meta.list_font_size = list_font_size;
            meta.item_height = item_height;
            meta_changed = true;
        }

        TabInfo tab;
        if (LoadTab(meta.folder_path, tab))
        {
            tab.list_font_size = list_font_size;
            tab.item_height = item_height;
            SaveTab(tab);
        }
    }

    if (meta_changed)
        SaveTabsMeta(tabs);
}

CMyPlayerListCache::TabInfo CMyPlayerListCache::BuildTab(const std::wstring& folder_path, const std::wstring& name, int sort_mode, const std::vector<SongInfo>& song_list)
{
    TabInfo tab;
    tab.name = name;
    tab.folder_path = folder_path;
    tab.cache_file = GetTabCacheFileName(folder_path);
    tab.sort_mode = sort_mode;
    tab.list_font_size = theApp.m_app_setting_data.song_list_font_size;
    tab.item_height = CalculateSongListItemHeight(theApp.m_app_setting_data.song_list_font_size);
    tab.cache_version = CACHE_VERSION;
    tab.include_sub_folder = true;
    tab.tracks.reserve(song_list.size());

    for (const auto& song : song_list)
    {
        TrackInfo item;
        item.file_path = song.file_path;
        item.display_name = CFilePathHelper(song.file_path).GetFileNameWithoutExtension();
        item.duration_text = song.length().toString();
        GetFileState(song.file_path, item.modified_time, item.file_size);
        tab.tracks.push_back(item);
    }
    return tab;
}

bool CMyPlayerListCache::IsCacheValid(const TabInfo& tab, const std::wstring& folder_path)
{
    return tab.cache_version == CACHE_VERSION
        && tab.include_sub_folder
        && tab.folder_path == folder_path
        && !tab.cache_file.empty();
}

void CMyPlayerListCache::UpsertTabsMeta(const TabInfo& tab)
{
    std::vector<TabInfo> tabs;
    LoadTabsMeta(tabs);
    auto iter = std::find_if(tabs.begin(), tabs.end(), [&](const TabInfo& item) {
        return item.folder_path == tab.folder_path;
    });

    TabInfo meta{ tab };
    meta.tracks.clear();
    if (iter == tabs.end())
        tabs.push_back(meta);
    else
        *iter = meta;
    SaveTabsMeta(tabs);
}

bool CMyPlayerListCache::GetFileState(const std::wstring& file_path, unsigned long long& modified_time, unsigned long long& file_size)
{
    WIN32_FILE_ATTRIBUTE_DATA file_data{};
    if (!::GetFileAttributesExW(file_path.c_str(), GetFileExInfoStandard, &file_data))
        return false;

    ULARGE_INTEGER modified{};
    modified.LowPart = file_data.ftLastWriteTime.dwLowDateTime;
    modified.HighPart = file_data.ftLastWriteTime.dwHighDateTime;
    modified_time = modified.QuadPart;

    ULARGE_INTEGER size{};
    size.LowPart = file_data.nFileSizeLow;
    size.HighPart = file_data.nFileSizeHigh;
    file_size = size.QuadPart;
    return true;
}
