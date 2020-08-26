#include "ofxAudioCapture.h"

/*----------------
  static
----------------*/

/**
    Print a list of all available audio devices (using RtAudio api)
 */
void ofxAudioCapture::printDeviceList()
{
	ofSoundStream soundstream;

	std::vector<RtAudio::Api> apis;
	RtAudio::getCompiledApi( apis );

	std::stringstream ss;

	ss << padRight( "AUDIO DRIVER", 20 ) << "| " << padRight( "DEVICE", 60 ) << "| DETAILS\n"
	   << padRight( "------------", 20 ) << "| " << padRight( "------", 60 ) << "| -------\n";

	for ( int i = ofSoundDevice::ALSA; i < ofSoundDevice::NUM_APIS; ++i ) {
		ofSoundDevice::Api api = ( ofSoundDevice::Api )i;
		if ( std::find( apis.begin(), apis.end(), ofToRtAudio( api ) ) != apis.end() ) {
			int j = 0;
			for ( auto& device : soundstream.getDeviceList( api ) ) {
				ss << padRight( toString( api ), 20 ) << "| "                             // driver
				   << padRight( "[" + ofToString( j ) + "] " + device.name, 60 ) << "| "  // device
				                                                                          // details:
				   << "in: " << padRight( device.inputChannels, 3 ) << ( device.isDefaultInput ? "* | " : "  | " )
				   << "out: " << padRight( device.outputChannels, 3 ) << ( device.isDefaultOutput ? "* | " : "  | " )
				   << "rates: ";
				for ( auto& rate : device.sampleRates ) {
					ss << rate << " ";
				}
				ss << "\n";
				++j;
			}
		}
	}

	ofLogNotice( "AudioCapture" ) << __FUNCTION__ << ":\n"
	                              << ss.str();
}

/*----------------
  public
----------------*/

/**
    Setup to record a mic input stream
 */
bool ofxAudioCapture::setupAudioInputRecorder( const ofSoundStreamSettings& settings, int writerBitsPerSample )
{
	// input stream
	m_soundStreamSettings = settings;
	m_soundStreamSettings.setInListener( this );
	if ( m_soundStream.setup( m_soundStreamSettings ) ) {
		ofLogNotice( "AudioCapture" ) << __FUNCTION__ << ": Successfully setup sound stream, device: " << toString( settings.getApi() ) << " [" << settings.getInDevice()->deviceID << "] " << settings.getInDevice()->name;
		// audio writer
		m_wavWriter.setFormat( settings.numInputChannels, settings.sampleRate, writerBitsPerSample );
		m_mode = Mode::READY;
		return true;
	} else {
		ofLogError( "AudioCapture" ) << __FUNCTION__ << ": Error setting up sound stream, device: " << toString( settings.getApi() )
		                             << ( settings.getInDevice() ? "[" + ofToString( settings.getInDevice()->deviceID ) + "] " + settings.getInDevice()->name : "invalid" ) << "!";
		m_mode = Mode::UNINITIALIZED;
		return false;
	}
}

/**
    Setup to manually add audio frames to the recorder.

    Add frames by calling ofxAudioCapture::audioIn( ofSoundBuffer& buffer ).
 */
bool ofxAudioCapture::setupManualRecorder( int numInputChannels, int sampleRate, int bitsPerSample )
{
	m_wavWriter.setFormat( numInputChannels, sampleRate, bitsPerSample );
	m_mode = Mode::READY;
	return true;
}

void ofxAudioCapture::beginRecording( const std::filesystem::path& wavFile )
{
	if ( m_mode == Mode::UNINITIALIZED ) {
		ofLogError( "AudioCapture" ) << __FUNCTION__ << ": Can't start recording, AudioCapture uninitialized!";
		return;
	}
	if ( m_mode == Mode::RECORDING ) {
		ofLogWarning( "AudioCapture" ) << __FUNCTION__ << ": Recording started, but already recording! Closing previous recording at [" << m_wavFile.string() << "] and starting new one at [" << wavFile.string() << "]";
		endRecording();
	}
	if ( m_wavWriter.open( wavFile.string(), WAVFILE_WRITE ) ) {
		m_wavFile = wavFile;
		m_mode    = Mode::RECORDING;
		ofLogNotice( "AudioCapture" ) << __FUNCTION__ << ": Starting recording to file [" << m_wavFile.string() << "]";
	} else {
		ofLogError( "AudioCapture" ) << __FUNCTION__ << ": Unable to open wav file for recording [" << wavFile.string() << "]!";
		m_mode = Mode::READY;
	}
}

void ofxAudioCapture::endRecording()
{
	if ( m_mode == Mode::RECORDING ) {
		m_wavWriter.close();
		m_mode = Mode::READY;
		ofLogNotice( "AudioCapture" ) << __FUNCTION__ << ": Ended recording to file [" << m_wavFile.string() << "]";
	} else {
		ofLogError( "AudioCapture" ) << __FUNCTION__ << ": Unable to end recording - " << ( m_mode == Mode::READY ? "recording not started" : "audio capture uninitialized" ) << "!";
	}
}

void ofxAudioCapture::audioIn( ofSoundBuffer& buffer )
{
	updateVolumes( buffer );

	// recording
	if ( m_mode == Mode::RECORDING ) {
		// if ( buffer.size() != m_soundStreamSettings.bufferSize ) {
		// 	// this shouldn't happen
		// 	ofLogError( "AudioCapture" ) << __FUNCTION__ << ": Expected audio input buffer size [" << m_soundStreamSettings.bufferSize << "] but the input stream has buffer size [" << buffer.size() << "]";
		// 	return;
		// }

		if ( buffer.size() ) {
			if ( !m_wavWriter.write( buffer.getBuffer().data(), buffer.size() ) ) {
				// unable to write
				ofLogError( "AudioCapture" ) << __FUNCTION__ << ": Unable to write audio to " << m_wavFile << "!";
				endRecording();
			}

		} else {
			ofLogError( "AudioCapture" ) << __FUNCTION__ << ": Audio input buffer empty!";
		}
	}
}

void ofxAudioCapture::audioIn( float* buffer, size_t size )
{
	// recording
	if ( m_mode == Mode::RECORDING ) {

		//if(buffer.size()){
		if ( buffer ) { /// TODO: E0153 expression "buffer" must have a class type -- fixed? test
			if ( !m_wavWriter.write( buffer, size ) ) {
				// unable to write
				ofLogError( "AudioCapture" ) << __FUNCTION__ << ": Unable to write audio to " << m_wavFile << "!";
				endRecording();
			}

		} else {
			ofLogError( "AudioCapture" ) << __FUNCTION__ << ": Audio input buffer empty!";
		}
	}
}

/*----------------
  protected
----------------*/

void ofxAudioCapture::updateVolumes( const ofSoundBuffer& buffer )
{
	auto nCh = buffer.getNumChannels();

	if ( nCh != m_volRms.size() ) {
		m_volRms.resize( nCh );
	}
	if ( nCh != m_volSmoothed.size() ) {
		m_volSmoothed.resize( nCh );
	}

	// root mean square volumes
	for ( auto& vol : m_volRms ) vol = 0;
	for ( size_t f = 0; f < buffer.getNumFrames(); f++ ) {

		for ( size_t ch = 0; ch < nCh; ch++ ) {
			// square
			m_volRms[ch] += buffer[f * nCh + ch] * buffer[f * nCh + ch];
		}
	}
	for ( auto& vol : m_volRms ) {
		vol /= float( buffer.getNumFrames() * nCh );  // mean
		vol = sqrt( vol );                            // root
	}

	// smoothed volumes
	for ( int ch = 0; ch < nCh; ch++ ) {
		m_volSmoothed[ch] *= 0.93;
		m_volSmoothed[ch] += 0.07 * m_volRms[ch];
	}
}