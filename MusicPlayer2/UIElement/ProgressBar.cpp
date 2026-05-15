#include "stdafx.h"
#include "ProgressBar.h"
#include "Player.h"
#include "TinyXml2Helper.h"
#include <cstdlib>

namespace
{
    bool ParseXmlColor(const std::string& str_color, COLORREF& color)
    {
        if (str_color.empty())
            return false;

        std::string color_text{ str_color };
        int base = 10;
        if (color_text[0] == '#')
        {
            color_text.erase(0, 1);
            base = 16;
        }
        else if (color_text.size() > 2 && color_text[0] == '0' && (color_text[1] == 'x' || color_text[1] == 'X'))
        {
            color_text.erase(0, 2);
            base = 16;
        }

        char* end_ptr{};
        unsigned long value = std::strtoul(color_text.c_str(), &end_ptr, base);
        if (end_ptr == color_text.c_str() || *end_ptr != '\0' || value > 0xFFFFFF)
            return false;

        color = RGB((value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF);
        return true;
    }

    bool ParseXmlColorAttribute(tinyxml2::XMLElement* xml_node, const char* attr_name, COLORREF& color)
    {
        return ParseXmlColor(CTinyXml2Helper::ElementAttribute(xml_node, attr_name), color);
    }
}

void UiElement::ProgressBar::Draw()
{
    CalculateRect();
    const COLORREF* custom_back_color{ !auto_color && progress_back_color.set ? &progress_back_color.color : nullptr };
    const COLORREF* custom_progress_color{ !auto_color && progress_color.set ? &progress_color.color : nullptr };
    if (show_play_time)
    {
        btn.rect = ui->DrawProgressBar(
            rect,
            play_time_both_side,
            ui->DPI(bar_height),
            custom_back_color,
            custom_progress_color,
            time_color.set ? &time_color.color : nullptr);
    }
    else
    {
        btn.rect = ui->DrawProgess(
            rect,
            ui->DPI(bar_height),
            custom_back_color,
            custom_progress_color);
    }
    Element::Draw();
}

bool UiElement::ProgressBar::LButtonUp(CPoint point)
{
    if (hover() && btn.rect.PtInRect(point))
    {
        int ckick_pos = point.x - btn.rect.left;
        double progress = static_cast<double>(ckick_pos) / btn.rect.Width();
        if (CPlayer::GetInstance().GetPlayStatusMutex().try_lock_for(std::chrono::milliseconds(1000)))
        {
            CPlayer::GetInstance().SeekTo(progress);
            CPlayer::GetInstance().GetPlayStatusMutex().unlock();
        }
        return true;
    }
    return false;
}

bool UiElement::ProgressBar::LButtonDown(CPoint point)
{
    return btn.hover;
}

bool UiElement::ProgressBar::RButtonUp(CPoint point)
{
    //进度条不弹出右键菜单
    return btn.rect.PtInRect(point);
}

bool UiElement::ProgressBar::MouseMove(CPoint point)
{
    btn.hover = btn.rect.PtInRect(point);
    if (last_hover && !btn.hover)
        HideTooltip();
    last_hover = btn.hover;

    if (btn.hover)
        HideTooltip();
    return false;
}

bool UiElement::ProgressBar::MouseLeave()
{
    HideTooltip();
    return false;
}

bool UiElement::ProgressBar::SetCursor()
{
    if (hover())
    {
        ::SetCursor(::LoadCursor(NULL, IDC_HAND));
        return true;
    }
    return false;
}

void UiElement::ProgressBar::HideTooltip()
{
    ui->UpdateMouseToolTipPosition(TooltipIndex::PROGRESS_BAR, CRect());
}

bool UiElement::ProgressBar::hover() const
{
    return btn.hover;
}

void UiElement::ProgressBar::FromXmlNode(tinyxml2::XMLElement* xml_node)
{
    Element::FromXmlNode(xml_node);
    std::string str_show_play_time = CTinyXml2Helper::ElementAttribute(xml_node, "show_play_time");
    show_play_time = CTinyXml2Helper::StringToBool(str_show_play_time.c_str());
    std::string str_play_time_both_side = CTinyXml2Helper::ElementAttribute(xml_node, "play_time_both_side");
    play_time_both_side = CTinyXml2Helper::StringToBool(str_play_time_both_side.c_str());
    std::string str_auto_color = CTinyXml2Helper::ElementAttribute(xml_node, "auto_color");
    if (str_auto_color.empty())
        str_auto_color = CTinyXml2Helper::ElementAttribute(xml_node, "auto_progress_color");
    auto_color = CTinyXml2Helper::StringToBool(str_auto_color.c_str());
    CTinyXml2Helper::GetElementAttributeInt(xml_node, "bar_height", bar_height);
    progress_back_color.set = ParseXmlColorAttribute(xml_node, "progress_back_color", progress_back_color.color);
    progress_color.set = ParseXmlColorAttribute(xml_node, "progress_color", progress_color.color);
    time_color.set = ParseXmlColorAttribute(xml_node, "time_color", time_color.color);
}
