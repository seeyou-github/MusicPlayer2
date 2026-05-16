#include "stdafx.h"
#include "ProgressBar.h"
#include "Player.h"
#include "TinyXml2Helper.h"

void UiElement::ProgressBar::Draw()
{
    CalculateRect();
    const COLORREF* custom_back_color{ !auto_color ? &theApp.m_app_setting_data.progress_back_color : nullptr };
    const COLORREF* custom_progress_color{ !auto_color ? &theApp.m_app_setting_data.progress_color : nullptr };
    if (show_play_time)
    {
        btn.rect = ui->DrawProgressBar(
            rect,
            play_time_both_side,
            ui->DPI(bar_height),
            custom_back_color,
            custom_progress_color,
            &theApp.m_app_setting_data.progress_time_color);
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
}
