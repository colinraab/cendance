#include "../Source/App/KeyMapping.h"

#include <cassert>
#include <iostream>
#include <string>

void testGlobalMappings() {
    auto play = mapKeyToCommand(ftxui::Event::Character(" "), 0);
    assert(play.has_value());
    assert(play->type == Command::Type::PlayStop);

    auto stop = mapKeyToCommand(ftxui::Event::Character("h"), 0);
    assert(stop.has_value());
    assert(stop->type == Command::Type::Stop);

    auto metronome = mapKeyToCommand(ftxui::Event::Character("m"), 0);
    assert(metronome.has_value());
    assert(metronome->type == Command::Type::ToggleMetronome);

    auto tempoUp = mapKeyToCommand(ftxui::Event::Character("+"), 0);
    assert(tempoUp.has_value());
    assert(tempoUp->type == Command::Type::SetTempo);
    assert(tempoUp->value > 0.0f);

    auto tempoDown = mapKeyToCommand(ftxui::Event::Character("-"), 0);
    assert(tempoDown.has_value());
    assert(tempoDown->type == Command::Type::SetTempo);
    assert(tempoDown->value < 0.0f);
}

void testTrackMappings() {
    constexpr uint8_t selectedTrack = 2;

    auto spotTape = mapKeyToCommand(ftxui::Event::Character("b"), selectedTrack);
    assert(spotTape.has_value());
    assert(spotTape->type == Command::Type::SpotEffectToggle);
    assert(spotTape->paramId == static_cast<uint16_t>(Command::SpotEffectId::TapeBrake));

    auto spotStutter = mapKeyToCommand(ftxui::Event::Character("x"), selectedTrack);
    assert(spotStutter.has_value());
    assert(spotStutter->type == Command::Type::SpotEffectToggle);
    assert(spotStutter->paramId == static_cast<uint16_t>(Command::SpotEffectId::Stutter));

    auto mute = mapKeyToCommand(ftxui::Event::Character("n"), selectedTrack);
    assert(mute.has_value());
    assert(mute->type == Command::Type::ToggleTrackMute);
    assert(mute->trackIndex == selectedTrack);

    auto gainUp = mapKeyToCommand(ftxui::Event::Character("]"), selectedTrack);
    assert(gainUp.has_value());
    assert(gainUp->type == Command::Type::SetTrackGain);
    assert(gainUp->value > 0.0f);

    auto gainDown = mapKeyToCommand(ftxui::Event::Character("["), selectedTrack);
    assert(gainDown.has_value());
    assert(gainDown->type == Command::Type::SetTrackGain);
    assert(gainDown->value < 0.0f);

    auto prevSynth = mapKeyToCommand(ftxui::Event::Character("9"), selectedTrack);
    assert(prevSynth.has_value());
    assert(prevSynth->type == Command::Type::StepSynthPreset);
    assert(prevSynth->value < 0.0f);

    auto nextSynth = mapKeyToCommand(ftxui::Event::Character("0"), selectedTrack);
    assert(nextSynth.has_value());
    assert(nextSynth->type == Command::Type::StepSynthPreset);
    assert(nextSynth->value > 0.0f);

    auto toneDown = mapKeyToCommand(ftxui::Event::Character(","), selectedTrack);
    assert(toneDown.has_value());
    assert(toneDown->type == Command::Type::SetTone);
    assert(toneDown->value < 0.0f);

    auto toneUp = mapKeyToCommand(ftxui::Event::Character("."), selectedTrack);
    assert(toneUp.has_value());
    assert(toneUp->type == Command::Type::SetTone);
    assert(toneUp->value > 0.0f);

    auto motionDown = mapKeyToCommand(ftxui::Event::Character(";"), selectedTrack);
    assert(motionDown.has_value());
    assert(motionDown->type == Command::Type::SetMotion);
    assert(motionDown->value < 0.0f);

    auto motionUp = mapKeyToCommand(ftxui::Event::Character("'"), selectedTrack);
    assert(motionUp.has_value());
    assert(motionUp->type == Command::Type::SetMotion);
    assert(motionUp->value > 0.0f);

    auto prevAlgo = mapKeyToCommand(ftxui::Event::Character("o"), selectedTrack);
    assert(prevAlgo.has_value());
    assert(prevAlgo->type == Command::Type::StepAlgorithm);
    assert(prevAlgo->value < 0.0f);

    auto nextAlgo = mapKeyToCommand(ftxui::Event::Character("p"), selectedTrack);
    assert(nextAlgo.has_value());
    assert(nextAlgo->type == Command::Type::StepAlgorithm);
    assert(nextAlgo->value > 0.0f);

    auto prevSection = mapKeyToCommand(ftxui::Event::Character("j"), selectedTrack);
    assert(prevSection.has_value());
    assert(prevSection->type == Command::Type::StepArrangementSection);
    assert(prevSection->value < 0.0f);

    auto nextSection = mapKeyToCommand(ftxui::Event::Character("u"), selectedTrack);
    assert(nextSection.has_value());
    assert(nextSection->type == Command::Type::StepArrangementSection);
    assert(nextSection->value > 0.0f);

    auto stepMode = mapKeyToCommand(ftxui::Event::Character("v"), selectedTrack);
    assert(stepMode.has_value());
    assert(stepMode->type == Command::Type::StepArrangementMode);

    auto algorithmSelectorKey = mapKeyToCommand(ftxui::Event::Character("a"), selectedTrack);
    assert(!algorithmSelectorKey.has_value());

    auto soundSelectorKey = mapKeyToCommand(ftxui::Event::Character("s"), selectedTrack);
    assert(!soundSelectorKey.has_value());

    auto chordSelectorKey = mapKeyToCommand(ftxui::Event::Character("c"), selectedTrack);
    assert(!chordSelectorKey.has_value());

    auto arrangementEditorKey = mapKeyToCommand(ftxui::Event::Character("r"), selectedTrack);
    assert(!arrangementEditorKey.has_value());

    auto arrangementEditorKeyUpper = mapKeyToCommand(ftxui::Event::Character("R"), selectedTrack);
    assert(!arrangementEditorKeyUpper.has_value());
}

void testArrowMappings() {
    constexpr uint8_t selectedTrack = 1;

    auto up = mapKeyToCommand(ftxui::Event::ArrowUp, selectedTrack);
    assert(up.has_value());
    assert(up->type == Command::Type::SetComplexity);
    assert(up->value > 0.0f);

    auto down = mapKeyToCommand(ftxui::Event::ArrowDown, selectedTrack);
    assert(down.has_value());
    assert(down->type == Command::Type::SetComplexity);
    assert(down->value < 0.0f);

    auto right = mapKeyToCommand(ftxui::Event::ArrowRight, selectedTrack);
    assert(right.has_value());
    assert(right->type == Command::Type::SetDensity);
    assert(right->value > 0.0f);

    auto left = mapKeyToCommand(ftxui::Event::ArrowLeft, selectedTrack);
    assert(left.has_value());
    assert(left->type == Command::Type::SetDensity);
    assert(left->value < 0.0f);
}

void testUnmappedKey() {
    auto none = mapKeyToCommand(ftxui::Event::Character("!"), 0);
    assert(!none.has_value());
}

int main() {
    testGlobalMappings();
    testTrackMappings();
    testArrowMappings();
    testUnmappedKey();

    std::cout << "KeyMapping tests passed!\n";
    return 0;
}
