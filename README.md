# 魔改版

 ![loading-ag-214](.\pic\a.jpg)

![loading-ag-216](.\pic\b.jpg)



### 修改

###### my_ui.xml 新增功能：

播放详情页歌词封面 圆角：radius= 0-height/2 
<albumCover square="true" margin="52" width="420" height="420" radius="64"/>

进度条： 

<!--progressBar高度：height="13" bar_height="13" 需要同时设置 -->

        <progressBar show_play_time="true" height="13" bar_height="10" auto_color="false"  progress_back_color="#333333" progress_color="#777777" time_color="#eeeeee"/>

lyrics歌词：
  <lyrics font_size="14" text_color="#9AA0A6" playing_text_color="#FFFFFF"/>
  也兼容：
  <lyrics font_size="14" text_color="0x9AA0A6" current_text_color="#FFFFFF"/>
  改动点：

- font_size 现在不需要再配 use_default_font="true" 也会生效。

- text_color 控制普通歌词文字颜色。

- playing_text_color 控制正在播放歌词颜色。

- current_text_color 作为正在播放颜色的别名。

- 单行、多行、双行、卡拉 OK 高亮、翻译歌词都接入了这套颜色。
  
  <bottomLyrics margin-left="14" height="54" max-width="560" font_size="20" single_line="true" played_text_color="#0f91a" unplayed_text_color="#eeeeee" next_text_color="#aaaaaa"/>

自定义播放列表界面：
<myPlayerList item_height="40" tab_margin_top="0" tab_margin_left="0" tab_margin_right="0" tab_height="36" tab_padding="18"
                            tab_background_color="#2D3037" tab_selected_background_color="#20222A" tab_unselected_background_color="#2D3037"
                            tab_selected_text_color="#eeeeee" tab_unselected_text_color="#aaaaaa" tab_selected_font_size="16" tab_unselected_font_size="16"/>

###### 其他修改

启动时扫描 exe\skins\background\ 下的 jpg/jpeg/png，按自然排序生成背景列表  

- 设置 -> 外观设置 新增：
  - 歌曲列表文字大小，范围 8-16
  - 歌曲列表文字颜色，可选择自定义颜色，也可恢复为“跟随主题”
- 设置会保存到 config.ini：
  - song_list_font_size
  - song_list_custom_text_color
  - song_list_text_color
- 已应用到自绘歌曲列表：
  - myplayerlist
  - 我喜欢的音乐
  - 播放队列/播放列表类界面
- 文字大小变化时同步调整：
  - 列表行高
  - 序号列/时长列宽度
  - 播放按钮、收藏按钮、列表图标、迷你频谱图标尺寸
- 原生播放列表控件也同步更新字体、行高、列宽和文字颜色。

###### 第三方lib

bass.dll ：BASS 音频引擎主库。项目用它做音频播放、流媒体/本地文件解码、设备输出、EQ/混响等基础播放能力。 
bass_fx.dll：BASS 的 FX 扩展库。 用于 BASS_FX_TempoCreate、播放速度不变调、变调等功能：
    官网：https://www.un4seen.com/bass.html
