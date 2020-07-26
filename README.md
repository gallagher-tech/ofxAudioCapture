# ofxAudioCapture

## capture audio from mic input
```c++
ofxAudioCapture audioCapture;

// list all audio devices
AudioCapture::printDeviceList();

// get a ofSoundStream device (using MS DirectSound API)
ofSoundStream soundStream;
auto devices = soundStream.getDeviceList( ofSoundDevice::Api::MS_DS );

// setup to capture from ofSoundStream / mic input:

ofSoundStreamSettings soundSettings;
soundSettings.setInDevice( devices[0] );    // use specific audio device
soundSettings.sampleRate        = 48000;    // hz, also 44100 is common
soundSettings.numOutputChannels = 0;        // no output, just input
soundSettings.numInputChannels  = 1;        // one channel input
soundSettings.bufferSize        = 512;      // ? seems to work
soundSettings.numBuffers        = 4;        // ^

int bitsPerSample = 16;     // bits per sample of audio writer, also 24 is common

audioCapture.setupAudioInputRecorder( soundSettings, bitsPerSample );

// begin capturing
auto path = ofToDataPath( "audio_capture.wav", true );  // absolute path
audioCapture.beginRecording( path );

// ... mic input is automatically piped to the audio capture writer ...

// later - end capture
audioCapture.endRecording();

// now WAV is saved to `bin/data/audio_capture.wav`
```

## capture audio manually
```c++
ofxAudioCapture audioCapture;

int numInputChannels    = 1;        // number of channels
int sampleRate          = 48000;    // Hz sample rate (e.g. 44100)
int bitsPerSample       = 16;       // bits per sample of audio writer (e.g. 24)

audioCapture.setupManualRecorder( numInputChannels, sampleRate, bitsPerSample );
std::vector<float> buffer;

// begin capturing
auto path = ofToDataPath( "audio_capture.wav", true );  // absolute path
audioCapture.beginRecording( path );

// fill buffer...
// add audio frames...
audioCapture.audioIn( buffer.data(), buffer.size() );

// later - end capture
audioCapture.endRecording();

// now WAV is saved to `bin/data/audio_capture.wav`
```
