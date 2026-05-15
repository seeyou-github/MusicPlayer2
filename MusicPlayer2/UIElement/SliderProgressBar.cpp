#include "stdafx.h"
#include "SliderProgressBar.h"
#include "Player.h"

void UiElement::SliderProgressBar::Draw()
{
    //设置进度条的位置
    SetRange(0, CPlayer::GetInstance().GetSongLength());
    if (!pressed)
        SetCurPos(CPlayer::GetInstance().GetCurrentPosition());

    Slider::Draw();

    //绘制AB重复标记
    ui->DrawABRepeat(rect);
}

void UiElement::SliderProgressBar::InitComplete()
{
    Slider::InitComplete();
    SetDragFinishTrigger([this](UiElement::Slider* sender) {
        int progress = sender->GetCurPos();
        if (CPlayer::GetInstance().GetPlayStatusMutex().try_lock_for(std::chrono::milliseconds(1000)))
        {
            CPlayer::GetInstance().SeekTo(progress);
            CPlayer::GetInstance().GetPlayStatusMutex().unlock();
        }
    });
}

bool UiElement::SliderProgressBar::MouseMove(CPoint point)
{
    if (Slider::MouseMove(point))
    {
        HideTooltip();
        return true;
    }

    bool hover = rect.PtInRect(point);
    if (last_hover && !hover)
        HideTooltip();
    last_hover = hover;

    return false;
}

bool UiElement::SliderProgressBar::MouseLeave()
{
    Slider::MouseLeave();
    HideTooltip();
    return false;
}

void UiElement::SliderProgressBar::HideTooltip()
{
    ui->UpdateMouseToolTipPosition(TooltipIndex::PROGRESS_BAR, CRect());
}

COLORREF UiElement::SliderProgressBar::GetBackColor(bool highlight_color)
{
    if (highlight_color)
        return ui->GetUIColors().color_spectrum;
    else
        return ui->GetUIColors().color_progress_back;
}

BYTE UiElement::SliderProgressBar::GetBackAlpha(bool highlight_color)
{
    if (highlight_color)
        return 255;
    else
        return ui->GetDefaultAlpha();
}
