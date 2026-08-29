#include "MelodicSampleLibrary.h"

#include "MelodicSampleBinaryData.h"

#include "../Audio/Synths/BassEngine.h"
#include "../Audio/Synths/ChordEngine.h"
#include "../Audio/Synths/LeadEngine.h"

#include <algorithm>
#include <sstream>

namespace {

template <typename Callback>
void forEachRegion(Callback&& callback) {
    for (const auto& instrument : {MelodicSampleCatalog::kSpectrum,
                                   MelodicSampleCatalog::kGrandPiano,
                                   MelodicSampleCatalog::kFluteC3}) {
        for (const auto& region : instrument.regions) {
            if (!region.resourceName.empty()) {
                callback(region);
            }
        }
    }

    auto visitImported = [&](const auto& instruments) {
        for (const auto& instrument : instruments) {
            for (const auto& region : instrument.regions) {
                if (!region.resourceName.empty()) {
                    callback(region);
                }
            }
        }
    };

    visitImported(MelodicSampleCatalog::kImportedBassInstruments);
    visitImported(MelodicSampleCatalog::kImportedChordInstruments);
    visitImported(MelodicSampleCatalog::kImportedLeadInstruments);
}

} // namespace

bool MelodicSampleLibrary::preloadEmbeddedSamples(std::string& error) {
    error.clear();
    formatManager.registerBasicFormats();

    size_t nextIndex = 0;
    bool hadFailure = false;
    std::ostringstream failures;

    forEachRegion([&](const MelodicSampleCatalog::RegionDefinition& region) {
        if (nextIndex >= sampleStorage.size()) {
            hadFailure = true;
            failures << "Melodic sample registry is full before " << region.resourceName << "\n";
            return;
        }

        std::shared_ptr<MelodicSampleData> loaded;
        std::string loadError;
        if (!loadEmbeddedSample(region, loaded, loadError)) {
            hadFailure = true;
            failures << "Failed to load embedded melodic sample " << region.resourceName
                     << ": " << loadError << "\n";
            return;
        }

        sampleStorage[nextIndex] = std::move(loaded);
        resourceNames[nextIndex] = region.resourceName;
        ++nextIndex;
    });

    if (hadFailure) {
        error = failures.str();
    }

    return !hadFailure;
}

bool MelodicSampleLibrary::loadEmbeddedSample(const MelodicSampleCatalog::RegionDefinition& region,
                                              std::shared_ptr<MelodicSampleData>& outData,
                                              std::string& error) const {
    error.clear();

    int dataSize = 0;
    const void* data = MelodicSampleBinaryData::getNamedResource(region.resourceName.data(), dataSize);
    if (data == nullptr || dataSize <= 0) {
        error = "Missing embedded resource.";
        return false;
    }

    auto memoryStream = std::make_unique<juce::MemoryInputStream>(data, static_cast<size_t>(dataSize), false);
    juce::OggVorbisAudioFormat oggFormat;
    std::unique_ptr<juce::AudioFormatReader> reader(oggFormat.createReaderFor(memoryStream.release(), true));
    if (!reader) {
        error = "Failed to decode embedded OGG sample.";
        return false;
    }

    if (reader->lengthInSamples <= 0) {
        error = "Sample source is empty.";
        return false;
    }

    const int channelCount = std::max(1, std::min<int>(2, static_cast<int>(reader->numChannels)));
    const int sampleCount = static_cast<int>(
        std::min<int64_t>(reader->lengthInSamples, static_cast<int64_t>(30 * reader->sampleRate)));

    auto sampleData = std::make_shared<MelodicSampleData>();
    sampleData->audio.setSize(channelCount, sampleCount, false, true, true);
    if (!reader->read(&sampleData->audio, 0, sampleCount, 0, true, true)) {
        error = "Failed reading sample data.";
        return false;
    }

    sampleData->sourceSampleRate = reader->sampleRate;
    sampleData->path = "embedded://" + std::string(region.resourceName);
    sampleData->name = std::string(region.displayName);
    outData = std::move(sampleData);
    return true;
}

const MelodicSampleData* MelodicSampleLibrary::getSample(std::string_view resourceName) const {
    for (size_t i = 0; i < resourceNames.size(); ++i) {
        if (resourceNames[i] == resourceName) {
            return sampleStorage[i].get();
        }
    }

    return nullptr;
}

void MelodicSampleLibrary::clearTrack(uint8_t trackIndex,
                                      BassEngine* bassEngine,
                                      ChordEngine* chordEngine,
                                      LeadEngine* leadEngine) const {
    if (trackIndex == 1 && bassEngine != nullptr) {
        bassEngine->clearSampleRegions();
    } else if (trackIndex == 2 && chordEngine != nullptr) {
        chordEngine->clearSampleRegions();
    } else if (trackIndex == 3 && leadEngine != nullptr) {
        leadEngine->clearSampleRegions();
    }
}

void MelodicSampleLibrary::setRegion(uint8_t trackIndex,
                                     uint8_t regionIndex,
                                     const MelodicSamplerEngine::Region& region,
                                     BassEngine* bassEngine,
                                     ChordEngine* chordEngine,
                                     LeadEngine* leadEngine) const {
    if (trackIndex == 1 && bassEngine != nullptr) {
        bassEngine->setSampleRegion(regionIndex, region);
    } else if (trackIndex == 2 && chordEngine != nullptr) {
        chordEngine->setSampleRegion(regionIndex, region);
    } else if (trackIndex == 3 && leadEngine != nullptr) {
        leadEngine->setSampleRegion(regionIndex, region);
    }
}

bool MelodicSampleLibrary::configurePreset(uint8_t trackIndex,
                                           uint8_t presetId,
                                           BassEngine* bassEngine,
                                           ChordEngine* chordEngine,
                                           LeadEngine* leadEngine,
                                           std::string& error) const {
    error.clear();
    clearTrack(trackIndex, bassEngine, chordEngine, leadEngine);

    const auto* instrument = MelodicSampleCatalog::getInstrumentForPreset(trackIndex, presetId);
    if (instrument == nullptr) {
        return true;
    }

    bool hadFailure = false;
    std::ostringstream failures;

    for (uint8_t i = 0; i < instrument->regions.size(); ++i) {
        const auto& definition = instrument->regions[i];
        if (definition.resourceName.empty()) {
            continue;
        }

        const MelodicSampleData* sample = getSample(definition.resourceName);
        if (sample == nullptr) {
            hadFailure = true;
            failures << "Missing melodic sample " << definition.resourceName << "\n";
            continue;
        }

        MelodicSamplerEngine::Region region;
        region.audio = &sample->audio;
        region.sourceSampleRate = sample->sourceSampleRate;
        region.rootNote = definition.rootNote;
        region.lowNote = definition.lowNote;
        region.highNote = definition.highNote;
        region.gain = definition.gain;
        region.tuneSemitones = definition.tuneSemitones;
        region.startOffset = definition.startOffset;
        region.endOffset = definition.endOffset;
        region.loop = definition.loop;
        region.loopStart = definition.loopStart;
        region.loopEnd = definition.loopEnd;
        setRegion(trackIndex, i, region, bassEngine, chordEngine, leadEngine);
    }

    if (hadFailure) {
        error = failures.str();
    }

    return !hadFailure;
}
