/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licenses for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licensing:
*
*   http://www.hise.audio/
*
*   HISE is based on the JUCE library,
*   which must be separately licensed for closed source applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

#ifndef AUDIOLOOPER_H_INCLUDED
#define AUDIOLOOPER_H_INCLUDED
#include "InterpolationTables.h"

namespace hise { using namespace juce;

class AudioLooper;

class AudioLooperSound : public ModulatorSynthSound
{
public:
	AudioLooperSound() {}

	bool appliesToNote(int /*midiNoteNumber*/) override   { return true; }
	bool appliesToChannel(int /*midiChannel*/) override   { return true; }
	bool appliesToVelocity(int /*midiChannel*/) override  { return true; }
};

class AudioLooperVoice : public ModulatorSynthVoice
{
public:

	AudioLooperVoice(ModulatorSynth *ownerSynth);;

	bool canPlaySound(SynthesiserSound *) override
	{
		return true;
	};

	void startNote(int midiNoteNumber, float /*velocity*/, SynthesiserSound*, int /*currentPitchWheelPosition*/) override;

	void calculateBlock(int startSample, int numSamples) override;;

	void resetVoice() override;
	

private:

	friend class AudioLooper;

	time_stretcher stretcher;

	Random r;

};

/** A simple, one-file sample player with looping facitilies.
	@ingroup synthTypes

	Whenever you don't need a fully fledged streaming sampler, you can use
	this class to play a single sample that the user can change using
	a AudioDisplayWaveform component.
*/
class AudioLooper : public ModulatorSynth,
					public AudioSampleProcessor,
					public TempoListener,
					public MultiChannelAudioBuffer::Listener
{
public:

	SET_PROCESSOR_NAME("AudioLooper", "Audio Loop Player", "");

	static ProcessorMetadata createMetadata();

	enum SampleInterpolation
	{
		NearestNeighbor = 0,
		Linear,
		SNESGaussian,
		Cubic,
		PS1Gaussian,
		GCPolyphase,
		numInterpolationModes
	};
	enum SpecialParameters
	{
		SyncMode = ModulatorSynth::numModulatorSynthParameters, 
		LoopEnabled,
		PitchTracking,
		RootNote,
		SampleStartMod,
		Reversed,
		InterpolationMode,
		ResampleRate,
		numLooperParameters
	};
	//public getters
	SampleInterpolation getInterpolationMode() const noexcept { return interpolationMode; }
	const AudioSampleBuffer* getResampledBuffer() const noexcept { return targetSampleRateIndex == 0 ? nullptr : &resampledBuffer; }
	int getTargetSampleRateIndex() const noexcept { return targetSampleRateIndex; }
	double getResampleRatio() const noexcept
{
    static const double sampleRates[] = { 0, 48000, 44100, 32000, 22050, 16000, 11025, 8000, 4000 };
    if (targetSampleRateIndex == 0) return 1.0;
    const double fileSampleRate = getSampleRateForLoadedFile();
    if (fileSampleRate <= 0.0) return 1.0;
    return sampleRates[targetSampleRateIndex] / fileSampleRate;
}
	AudioLooper(MainController *mc, const String &id, int numVoices);

	~AudioLooper() override;

	void restoreFromValueTree(const ValueTree &v) override;

	ValueTree exportAsValueTree() const override;

	void tempoChanged(double /*newTempo*/) override
	{
		setSyncMode(syncMode);
	};

	float getAttribute(int parameterIndex) const override;;

	void setInternalAttribute(int parameterIndex, float newValue) override;

	void prepareToPlay(double sampleRate, int samplesPerBlock) override
	{
		ModulatorSynth::prepareToPlay(sampleRate, samplesPerBlock);
		refreshSyncState();
	}

	void bufferWasLoaded() override;

	void bufferWasModified() override;

	ProcessorEditorBody* createEditor(ProcessorEditor *parentEditor) override;
	void setSyncMode(int newSyncMode);

	void setUseLoop(bool shouldBeEnabled)
	{
		loopEnabled = shouldBeEnabled;
	}

	bool isUsingLoop() const { return loopEnabled; }

	void refreshSyncState();

private:

	HeapBlock<float> resampleBuffer;
	double resampleRatio;
	int numResampleBuffer;

	UpdateMerger inputMerger;


	bool loopEnabled = false;
	bool reversed = false;
	bool pitchTrackingEnabled;
	AudioLooper::SampleInterpolation interpolationMode = SampleInterpolation::NearestNeighbor;
	int rootNote;
	int targetSampleRateIndex = 0; // 0 = Native (no resampling)
	AudioSampleBuffer resampledBuffer;
	void rebuildResampledBuffer();

	int sampleStartMod = 0;

	friend class AudioLooperVoice;

	scriptnode::core::stretch_player<1>::tempo_syncer syncer;
	double numQuarters = 0.0;
	AudioSampleProcessor::SyncToHostMode syncMode;
	
};






} // namespace hise

#endif  // AUDIOLOOPER_H_INCLUDED
