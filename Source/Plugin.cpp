#include <JuceHeader.h>
#include <array>
#include <cmath>

class MLOGChordLab final : public juce::AudioProcessor {
public:
    MLOGChordLab() : juce::AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "PARAMS", makeLayout()) {}
    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout() {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
        p.push_back(std::make_unique<juce::AudioParameterChoice>("preset", "Chord Style", juce::StringArray{"Rave Minor", "UKG Soul", "Warehouse", "Dream Pad", "Hard Stab", "Deep House"}, 0));
        p.push_back(std::make_unique<juce::AudioParameterFloat>("cutoff", "Brightness", 0.02f, 1.0f, 0.75f));
        p.push_back(std::make_unique<juce::AudioParameterFloat>("drive", "Drive", 0.0f, 1.0f, 0.25f));
        p.push_back(std::make_unique<juce::AudioParameterFloat>("release", "Release", 0.05f, 3.0f, 0.35f));
        p.push_back(std::make_unique<juce::AudioParameterFloat>("width", "Stereo Width", 0.0f, 1.0f, 0.5f));
        p.push_back(std::make_unique<juce::AudioParameterFloat>("level", "Output", 0.0f, 1.0f, 0.55f));
        return {p.begin(), p.end()};
    }
    const juce::String getName() const override { return "MLOG Chord Lab"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override { return new juce::GenericAudioProcessorEditor(*this); }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}
    bool isBusesLayoutSupported(const BusesLayout& l) const override { return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo(); }
    void prepareToPlay(double sr, int) override { sampleRate = sr; for (auto& v : voices) v = {}; }
    void releaseResources() override {}
    void getStateInformation(juce::MemoryBlock& out) override { auto xml = state.copyState().createXml(); copyXmlToBinary(*xml, out); }
    void setStateInformation(const void* data, int size) override { auto xml = getXmlFromBinary(data, size); if (xml && xml->hasTagName(state.state.getType())) state.replaceState(juce::ValueTree::fromXml(*xml)); }
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override {
        juce::ScopedNoDenormals noDenormals;
        buffer.clear();
        int preset = (int) state.getRawParameterValue("preset")->load();
        float bright = state.getRawParameterValue("cutoff")->load();
        float drive = state.getRawParameterValue("drive")->load();
        float release = state.getRawParameterValue("release")->load();
        float width = state.getRawParameterValue("width")->load();
        float level = state.getRawParameterValue("level")->load();
        const int n = buffer.getNumSamples();
        auto* L = buffer.getWritePointer(0); auto* R = buffer.getWritePointer(1);
        juce::MidiBuffer::Iterator it(midi); juce::MidiMessage message; int eventPos = 0;
        bool hasEvent = it.getNextEvent(message, eventPos);
        for (int i=0;i<n;++i) {
            while (hasEvent && eventPos <= i) {
                if (message.isNoteOn()) noteOn(message.getNoteNumber(), message.getFloatVelocity());
                else if (message.isNoteOff()) noteOff(message.getNoteNumber());
                else if (message.isAllNotesOff() || message.isAllSoundOff()) for(auto& v:voices) v.released=true;
                hasEvent=it.getNextEvent(message,eventPos);
            }
            float left=0,right=0;
            for(auto& v:voices) {
                if(!v.active) continue;
                const float attack = 1.0f - std::exp(-1.0f/(float)(sampleRate*0.008));
                v.env = v.released ? v.env * std::exp(-1.0f/(float)(sampleRate*release)) : v.env + (1.0f-v.env)*attack;
                if(v.released && v.env < 0.0002f) {v.active=false; continue;}
                static constexpr int intervals[6][4]={{0,3,7,10},{0,3,7,14},{0,7,10,15},{0,4,7,11},{0,3,6,10},{0,4,7,10}};
                float sumL=0,sumR=0;
                for(int c=0;c<4;++c) {
                    const double freq = 440.0*std::pow(2.0,(v.note+intervals[preset][c]-69.0)/12.0);
                    const double inc=freq/sampleRate;
                    v.phase[c] += inc;
                    if(v.phase[c]>=1.0) v.phase[c]-=std::floor(v.phase[c]);
                    const double p=v.phase[c];
                    const float saw=(float)(2.0*p-1.0);
                    const float sine=(float)std::sin(juce::MathConstants<double>::twoPi*p);
                    const float square=(p<0.5?1.0f:-1.0f);
                    float tone=0;
                    switch(preset) {
                        case 0: tone=0.7f*saw+0.3f*square; break;
                        case 1: tone=0.4f*saw+0.6f*sine; break;
                        case 2: tone=0.85f*saw+0.15f*square; break;
                        case 3: tone=0.25f*saw+0.75f*sine; break;
                        case 4: tone=0.8f*square+0.2f*saw; break;
                        default: tone=0.55f*saw+0.45f*sine;
                    }
                    float pan=(c%2==0?-1.0f:1.0f)*width*0.45f;
                    sumL+=tone*(1.0f-pan); sumR+=tone*(1.0f+pan);
                }
                sumL*=0.18f*v.env*v.velocity; sumR*=0.18f*v.env*v.velocity;
                const float a=std::clamp(0.002f+bright*bright*0.65f,0.002f,0.9f);
                v.filterL+=a*(sumL-v.filterL); v.filterR+=a*(sumR-v.filterR);
                left+=v.filterL; right+=v.filterR;
            }
            float gain=1.0f+drive*12.0f;
            L[i]=std::tanh(left*gain)*level;
            R[i]=std::tanh(right*gain)*level;
        }
    }
private:
    struct Voice {bool active=false,released=false; int note=0; float velocity=0,env=0,filterL=0,filterR=0; std::array<double,4> phase{};};
    std::array<Voice,16> voices{};
    double sampleRate=44100;
    juce::AudioProcessorValueTreeState state;
    void noteOn(int note,float vel) {
        Voice* chosen=nullptr;
        for(auto& v:voices) if(!v.active){chosen=&v;break;}
        if(!chosen) chosen=&voices[0];
        *chosen={}; chosen->active=true;chosen->note=note;chosen->velocity=vel;
    }
    void noteOff(int note) {for(auto& v:voices) if(v.active && v.note==note) v.released=true;}
};

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MLOGChordLab(); }
