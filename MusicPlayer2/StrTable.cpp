#include "stdafx.h"
#include "StrTable.h"
#include "Common.h"
#include "IniHelper.h"
#include "resource.h"

StrTable::StrTable()
{
}

StrTable::~StrTable()
{
}

const wstring StrTable::error_str = { L"<error>" };

bool StrTable::Init(wstring& language_tag_setting)
{
    bool expected = false;
    if (!initialized.compare_exchange_strong(expected, true))
        return false;

    auto InitMapFromIniHelper = [this](const CIniHelper& ini)
    {
        static const wstring MenuAppName = L"menu.";
        ini.GetAllKeyValues(L"text", m_text_string_table);
        ini.GetAllKeyValues(L"scintlla", m_scintilla_string_table);
        const auto& list = ini.GetAllAppName(MenuAppName);
        for (const auto& item : list)
            ini.GetAllKeyValues(MenuAppName + item, m_menu_string_table[item]);
    };

    CIniHelper default_ini(IDR_STRING_TABLE);
    InitMapFromIniHelper(default_ini);

    LanguageInfo zh_cn;
    zh_cn.display_name = default_ini.GetString(L"general", L"DISPLAY_NAME", L"简体中文");
    zh_cn.bcp_47 = default_ini.GetString(L"general", L"BCP_47", L"zh-CN");
    zh_cn.default_font_name = default_ini.GetString(L"general", L"DEFAULT_FONT", L"");
    default_ini.GetStringList(L"general", L"TRANSLATOR", zh_cn.translator, vector<wstring>{ L"MusicPlayer2" });
    m_default_font_name = zh_cn.default_font_name;
    m_language_tag.push_back(zh_cn.bcp_47);
    m_language_list.push_back(std::move(zh_cn));

    if (!language_tag_setting.empty() && language_tag_setting != m_language_tag.front())
        language_tag_setting.clear();

    // TODO: 检查系统是否已安装此字体（未测试：我担心其中使用的字体枚举API当系统字体非常多时出现严重的效率问题）
    if (m_default_font_name.empty() || m_default_font_name.size() > LF_FACESIZE - 1/* || !CCommon::IsFontInstalled(m_default_font_name)*/)
        m_default_font_name = CCommon::GetSystemDefaultUIFont();
    return true;
}

const wstring& StrTable::LoadText(const wstring& key) const
{
    // 查找key而不是使用[]是为了避免发生任何写入，这样不使用读写锁也有线程安全
    auto iter = m_text_string_table.find(key);
    if (iter != m_text_string_table.end())
        return iter->second;
    else    // 程序中试图读取不存在于<language>.ini中的键或当前还未进行初始化
    {
        std::lock_guard<std::mutex> lock(error_mutex);
        m_unknown_key.insert(key);
        return error_str;
    }
}

wstring StrTable::LoadTextFormat(const wstring& key, const std::initializer_list<CVariant>& paras) const
{
    // 查找key而不是使用[]是为了避免发生任何写入，这样不使用读写锁也有线程安全
    auto iter = m_text_string_table.find(key);
    if (iter == m_text_string_table.end())  // 程序中试图读取不存在于<language>.ini中的键或当前还未进行初始化
    {
        std::lock_guard<std::mutex> lock(error_mutex);
        m_unknown_key.insert(key);
        return error_str;
    }
    wstring str{ iter->second };    // 复制以避免原始字符串修改
    int index{ 1 };
    for (const auto& para : paras)
    {
        wstring format_str{ L"<%" + std::to_wstring(index) + L"%>" };
        if (!CCommon::StringReplace(str, format_str, para.ToString().GetString()))
        {
            // 当前取得的翻译字符串中缺少paras指定的<%序号%>占位符
            std::lock_guard<std::mutex> lock(error_mutex);
            m_error_para_key.insert(key);
            continue;
        }
        ++index;
    }
    return str;
}

const wstring& StrTable::LoadMenuText(const wstring& menu_name, const wstring& key) const
{
    // 查找key而不是使用[]是为了避免发生任何写入，这样不使用读写锁也有线程安全
    auto iter_name = m_menu_string_table.find(menu_name);
    if (iter_name == m_menu_string_table.end())
    {
        std::lock_guard<std::mutex> lock(error_mutex);
        m_unknown_key.insert(menu_name);
        return error_str;
    }
    const auto& key_map = iter_name->second;
    auto iter_key = key_map.find(key);
    if (iter_key == key_map.end())
    {
        std::lock_guard<std::mutex> lock(error_mutex);
        m_unknown_key.insert(key);
        return error_str;
    }
    return iter_key->second;
}

const std::map<wstring, wstring>& StrTable::GetScintillaStrMap() const
{
    return m_scintilla_string_table;
}
