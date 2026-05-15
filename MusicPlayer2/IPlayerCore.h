#pragma once

struct MidiInfo
{
    int midi_position;
    int midi_length;
    int speed;		//速度，bpm
    int tempo;		//每个四分音符的微秒数
    float ppqn;
};

enum PlayerCoreType
{
    PT_BASS,
    PT_MCI,
    PT_FFMPEG,
};

enum PlayingState       //正在播放标志
{
    PS_STOPED,          //已停止
    PS_PAUSED,          //已暂停
    PS_PLAYING          //正在播放
};

#define MAX_PLAY_SPEED 4.0f
#define MIN_PLAY_SPEED 0.1f
#define MAX_PLAY_PITCH 12
#define MIN_PLAY_PITCH -12

/////////////////////////////////////////////////////////////////////////////////////////////////

class IPlayerCore
{
public:
    struct AudioInfo
    {
        int length{};       //时长，单位为毫秒
        int bitrate{};      //比特率，单位为kbps
        int freq{};         //采样频率，单位为Hz
        int bits{};         //位深度
        int channels{};     //声道数
    };

    struct AudioTag
    {
        std::wstring title;
        std::wstring artist;
        std::wstring album;
        std::wstring comment;
        std::wstring genre;
        unsigned short year{};
        int track{};
    };

    virtual ~IPlayerCore() {}

    virtual void InitCore() = 0;
    virtual void UnInitCore() = 0;

    virtual std::wstring GetAudioType() = 0;    //获取音频格式的类型，如果返回空字符串，则会显示为文件的扩展名
    virtual int GetChannels() = 0;      //获取声道数
    virtual int GetFReq() = 0;          //获取采样频率，单位为Hz
    virtual int GetBitrate() = 0;       //获取比特率，单位为kbps
    virtual std::wstring GetSoundFontName() = 0;    //播放midi音乐时，获取midi音色库的名称

    virtual void Open(const wchar_t* file_path) = 0;
    virtual void Close() = 0;
    virtual void Play() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;
    virtual void SetVolume(int volume) = 0;
    virtual void SetSpeed(float speed) = 0;         //设置播放速度（1为原速）
    virtual void SetPitch(int pitch) = 0;           //设置播放变调，半音为一个单位，[-12, 12]，0为原调
    virtual bool IsSpeedAvailable() = 0;            //设置播放速度是否可用
    virtual bool IsPitchAvailable() = 0;            //设置播放变调是否可用
    virtual bool SongIsOver() = 0;                  //曲目是否播放完毕

    virtual int GetCurPosition() = 0;               //获取当前播放进度，单位为毫秒
    virtual int GetSongLength() = 0;                //获取歌曲长度，单位为毫秒
    virtual void SetCurPosition(int position) = 0;  //设置播放进度，单位为毫秒

    /**
     * @brief   获取一个音频文件的音频信息和标签信息（需要支持并发且不影响当前播放）
     * @param[in]   file_path 文件路径
     * @param[out]   audio_info 保存音频信息
     * @param[out]   audio_tag 保存标签信息（主程序会优先通过Taglib获取标签信息，在Taglib无法获取到标签信息的情况下，才会使用这里的值）
     */
    virtual void GetAudioInfo(const wchar_t* file_path, AudioInfo* audio_info, AudioTag* audio_tag) = 0;

    virtual bool IsMidi() = 0;
    virtual bool IsMidiConnotPlay() = 0;
    virtual MidiInfo GetMidiInfo() = 0;
    virtual std::wstring GetMidiInnerLyric() = 0;
    virtual bool MidiNoLyric() = 0;
    virtual PlayingState GetPlayingState() = 0;

    virtual void ApplyEqualizer(int channel, int gain) = 0; //设置均衡器（channel为均衡器通道，取值为0~9，gain为增益，取值为-15~15）
    virtual void SetReverb(int mix, int time) = 0;		//设置混响（mix为混响强度，取值为0~100，time为混响时间，取值为1~300，单位为10ms）
    virtual void ClearReverb() = 0;			//关闭混响
    virtual void GetFFTData(float fft_data[FFT_SAMPLE]) = 0;       //获取频谱分析数据

    virtual int GetErrorCode() = 0;                         //获取错误代码
    virtual std::wstring GetErrorInfo(int error_code) = 0;  //根据错误代码获取错误信息
    virtual std::wstring GetErrorInfo() = 0;  //获取错误信息

    virtual PlayerCoreType GetCoreType() = 0;

    virtual bool IsVolumeFadingOut() { return false; }    //是否处于音量淡出状态
};
