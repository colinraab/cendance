#pragma once
#include <ftxui/screen/color.hpp>

namespace Theme {
    inline const ftxui::Color Background = ftxui::Color::Default;
    inline const ftxui::Color Foreground = ftxui::Color::White;
    inline const ftxui::Color ModalForeground = ftxui::Color::Default;
    inline const ftxui::Color Highlight = ftxui::Color::Cyan;
    inline const ftxui::Color Active = ftxui::Color::Green;
    inline const ftxui::Color Inactive = ftxui::Color::GrayDark;
    inline const ftxui::Color Error = ftxui::Color::Red;
    inline const ftxui::Color TrackBorder = ftxui::Color::GrayDark;
    inline const ftxui::Color TrackSelected = ftxui::Color::Yellow;
    inline const ftxui::Color MeterLow = ftxui::Color::Green;
    inline const ftxui::Color MeterMid = ftxui::Color::Yellow;
    inline const ftxui::Color MeterHigh = ftxui::Color::Red;

    // Per-track note colors
    inline const ftxui::Color NoteDrums = ftxui::Color::RGB(0xFF, 0x6B, 0x6B);
    inline const ftxui::Color NoteBass = ftxui::Color::RGB(0xFF, 0xA9, 0x4D);
    inline const ftxui::Color NoteChords = ftxui::Color::RGB(0x69, 0xDB, 0x7C);
    inline const ftxui::Color NoteLead = ftxui::Color::RGB(0x74, 0xC0, 0xFC);

    inline const ftxui::Color TrackLabelDrums = NoteDrums;
    inline const ftxui::Color TrackLabelBass = NoteBass;
    inline const ftxui::Color TrackLabelChords = NoteChords;
    inline const ftxui::Color TrackLabelLead = NoteLead;

    inline const ftxui::Color NoteColors[] = {
        NoteDrums, NoteBass, NoteChords, NoteLead};
    inline const ftxui::Color TrackLabelColors[] = {
        TrackLabelDrums, TrackLabelBass, TrackLabelChords, TrackLabelLead};
}
