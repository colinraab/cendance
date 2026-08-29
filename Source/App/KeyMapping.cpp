#include "KeyMapping.h"

std::optional<Command> mapKeyToCommand(const ftxui::Event& event, uint8_t selectedTrack) {
    if (event.is_character()) {
        char c = event.character()[0];
        
        switch (c) {
            case ' ':
                return Command{Command::Type::PlayStop, selectedTrack, 0, 0.0f};
            case 'h':
            case 'H':
                return Command{Command::Type::Stop, selectedTrack, 0, 0.0f};
            case '+':
            case '=':
                return Command{Command::Type::SetTempo, selectedTrack, 0, 1.0f}; // Increase BPM by 1
            case '-':
            case '_':
                return Command{Command::Type::SetTempo, selectedTrack, 0, -1.0f}; // Decrease BPM by 1
            case 'm':
            case 'M':
                return Command{Command::Type::ToggleMetronome, selectedTrack, 0, 0.0f};
            case 'b':
            case 'B':
                return Command{Command::Type::SpotEffectToggle,
                               selectedTrack,
                               static_cast<uint16_t>(Command::SpotEffectId::TapeBrake),
                               0.0f};
            case 'x':
            case 'X':
                return Command{Command::Type::SpotEffectToggle,
                               selectedTrack,
                               static_cast<uint16_t>(Command::SpotEffectId::Stutter),
                               0.0f};
            case 'n':
            case 'N':
                return Command{Command::Type::ToggleTrackMute, selectedTrack, 0, 0.0f};
            case '9':
                return Command{Command::Type::StepSynthPreset, selectedTrack, 0, -1.0f};
            case '0':
                return Command{Command::Type::StepSynthPreset, selectedTrack, 0, 1.0f};
            case ',':
            case '<':
                return Command{Command::Type::SetTone, selectedTrack, 0, -0.05f};
            case '.':
            case '>':
                return Command{Command::Type::SetTone, selectedTrack, 0, 0.05f};
            case ';':
            case ':':
                return Command{Command::Type::SetMotion, selectedTrack, 0, -0.05f};
            case '\'':
            case '"':
                return Command{Command::Type::SetMotion, selectedTrack, 0, 0.05f};
            case ']':
                return Command{Command::Type::SetTrackGain, selectedTrack, 0, 0.05f};
            case '[':
                return Command{Command::Type::SetTrackGain, selectedTrack, 0, -0.05f};
                
            case 'o': case 'O':
                return Command{Command::Type::StepAlgorithm, selectedTrack, 0, -1.0f};
            case 'p': case 'P':
                return Command{Command::Type::StepAlgorithm, selectedTrack, 0, 1.0f};
            case 'j': case 'J':
                return Command{Command::Type::StepArrangementSection, selectedTrack, 0, -1.0f};
            case 'u': case 'U':
                return Command{Command::Type::StepArrangementSection, selectedTrack, 0, 1.0f};
            case 'v': case 'V':
                return Command{Command::Type::StepArrangementMode, selectedTrack, 0, 1.0f};

            default:
                break;
        }
    }
    
    // Provide continuous mapping for arrows
    if (event == ftxui::Event::ArrowUp) {
        return Command{Command::Type::SetComplexity, selectedTrack, 0, 0.1f}; // increment
    }
    if (event == ftxui::Event::ArrowDown) {
        return Command{Command::Type::SetComplexity, selectedTrack, 0, -0.1f}; // decrement
    }
    if (event == ftxui::Event::ArrowRight) {
        return Command{Command::Type::SetDensity, selectedTrack, 0, 0.1f}; // increment
    }
    if (event == ftxui::Event::ArrowLeft) {
        return Command{Command::Type::SetDensity, selectedTrack, 0, -0.1f}; // decrement
    }
    
    return std::nullopt;
}
