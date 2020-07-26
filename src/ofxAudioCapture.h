#pragma once
#include "WavFile.h"  // wav recorder
//
#include "RtAudio.h"  // rt audio from oF
#include "ofMain.h"   // oF

//========================================================================

inline RtAudio::Api ofToRtAudio( ofSoundDevice::Api api )
{
	switch ( api ) {
		case ofSoundDevice::Api::ALSA:
			return RtAudio::Api::LINUX_ALSA;
		case ofSoundDevice::Api::PULSE:
			return RtAudio::Api::LINUX_PULSE;
		case ofSoundDevice::Api::OSS:
			return RtAudio::Api::LINUX_OSS;
		case ofSoundDevice::Api::JACK:
			return RtAudio::Api::UNIX_JACK;
#ifndef TARGET_LINUX
		case ofSoundDevice::Api::OSX_CORE:
			return RtAudio::Api::MACOSX_CORE;
		case ofSoundDevice::Api::MS_WASAPI:
			return RtAudio::Api::WINDOWS_WASAPI;
		case ofSoundDevice::Api::MS_ASIO:
			return RtAudio::Api::WINDOWS_ASIO;
		case ofSoundDevice::Api::MS_DS:
			return RtAudio::Api::WINDOWS_DS;
#endif
		default:
			return RtAudio::Api::UNSPECIFIED;
	}
}

//========================================================================

class ofxAudioCapture
{
public:
	enum class Mode
	{
		UNINITIALIZED = -1,
		READY,
		RECORDING
	};

	static void printDeviceList();

	bool setupAudioInputRecorder( const ofSoundStreamSettings& settings, int writerBitsPerSample = 16 );
	bool setupManualRecorder( int numInputChannels = 1, int sampleRate = 48000, int bitsPerSample = 16 );

	void beginRecording( const std::filesystem::path& wavFile );
	void endRecording();

	void audioIn( ofSoundBuffer& buffer );
	void audioIn( float* buffer, size_t size );

	const std::vector<float>& getVolumeRms() const { return m_volRms; }
	const std::vector<float>& getVolumeSmoothed() const { return m_volSmoothed; }

protected:
	Mode m_mode = Mode::UNINITIALIZED;
	ofSoundStream m_soundStream;
	ofSoundStreamSettings m_soundStreamSettings;
	WavFile m_wavWriter;
	std::filesystem::path m_wavFile;

	std::vector<float> m_volRms;  // size = num channels
	std::vector<float> m_volSmoothed;

	void updateVolumes( const ofSoundBuffer& buffer );

	template <typename T>
	static inline string padRight( T t, const int& width, const char& separator = ' ' )
	{
		stringstream ss;
		ss << left << setw( width ) << setfill( separator ) << t;
		return ss.str();
	}
};