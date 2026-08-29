#include "../Source/Audio/Harmony/Scale.h"
#include "../Source/Audio/Harmony/ChordProgression.h"
#include <iostream>
#include <cassert>
#include <array>
#include <set>
#include <string>

void testScaleQuantization() {
    // Test C Major
    Scale cMajor(0, Scale::Type::Major); // Root C (0)

    // Chromatic quantization in C Major
    // C=0, C#=1->C(0) or D(2), D=2, D#=3->E(4) or D(2), E=4, F=5, F#=6->G(7) or F(5), G=7, G#=8->A(9) or G(7), A=9, A#=10->B(11) or A(9), B=11
    
    assert(cMajor.quantize(60) == 60); // C4 -> C4
    int q61 = cMajor.quantize(61);
    assert(q61 == 60 || q61 == 62);    // C#4 -> C4 or D4
    assert(cMajor.quantize(62) == 62); // D4 -> D4
    int q63 = cMajor.quantize(63);
    assert(q63 == 62 || q63 == 64);    // D#4 -> D4 or E4
    assert(cMajor.quantize(64) == 64); // E4 -> E4
    assert(cMajor.quantize(65) == 65); // F4 -> F4
    int q66 = cMajor.quantize(66);
    assert(q66 == 65 || q66 == 67);    // F#4 -> F4 or G4
    assert(cMajor.quantize(67) == 67); // G4 -> G4
    
    // Test minor scale 
    Scale aMinor(9, Scale::Type::NaturalMinor); // Root A (9)
    assert(aMinor.quantize(69) == 69); // A4
    assert(aMinor.quantize(60) == 60); // C4 in A minor
    int q61am = aMinor.quantize(61);   // C# is not in A minor, closest is C(60) or D(62)
    assert(q61am == 60 || q61am == 62);

    std::cout << "Scale Quantization tests passed!\n";
}

void testChordProgression() {
    assert(ChordProgression::getNumProgressions() == 15);

    auto prog = ChordProgression::get(0); // Trance/Pop
    assert(prog.name == "Trance/Pop");
    assert(prog.degrees.size() == 4);
    assert(prog.degrees[0] == 0); // i
    assert(prog.degrees[1] == 5); // VI
    assert(prog.degrees[2] == 2); // III
    assert(prog.degrees[3] == 6); // VII

    auto latest = ChordProgression::get(14); // Tension Arc
    assert(latest.name == "Tension Arc");
    assert(latest.degrees[0] == 0);
    assert(latest.degrees[1] == 1);
    assert(latest.degrees[2] == 3);
    assert(latest.degrees[3] == 6);

    std::cout << "ChordProgression get logic passed!\n";
}

// ========================================================================
// P1: Expanded Harmony Tests
// ========================================================================

void testScaleDegreeWrapping() {
    Scale cMajor(0, Scale::Type::Major);

    // Degree 7 should wrap to degree 0 of next octave
    assert(cMajor.getDegree(0, 4) == 48);  // C4
    assert(cMajor.getDegree(7, 4) == 60);  // C5 (octave up)
    assert(cMajor.getDegree(8, 4) == 62);  // D5

    // Negative degree should wrap down
    assert(cMajor.getDegree(-1, 4) == 47); // B3
    assert(cMajor.getDegree(-2, 4) == 45); // A3

    // Large degree
    assert(cMajor.getDegree(14, 4) == 72); // C6
}

void testAllScaleTypes() {
    // Major
    Scale major(0, Scale::Type::Major);
    assert(major.getDegree(0, 4) == 48); // C
    assert(major.getDegree(1, 4) == 50); // D
    assert(major.getDegree(2, 4) == 52); // E
    assert(major.getDegree(3, 4) == 53); // F
    assert(major.getDegree(4, 4) == 55); // G
    assert(major.getDegree(5, 4) == 57); // A
    assert(major.getDegree(6, 4) == 59); // B

    // Natural Minor
    Scale minor(0, Scale::Type::NaturalMinor);
    assert(minor.getDegree(0, 4) == 48); // C
    assert(minor.getDegree(1, 4) == 50); // D
    assert(minor.getDegree(2, 4) == 51); // Eb
    assert(minor.getDegree(3, 4) == 53); // F
    assert(minor.getDegree(4, 4) == 55); // G
    assert(minor.getDegree(5, 4) == 56); // Ab
    assert(minor.getDegree(6, 4) == 58); // Bb

    // Harmonic Minor
    Scale harmMinor(0, Scale::Type::HarmonicMinor);
    assert(harmMinor.getDegree(0, 4) == 48); // C
    assert(harmMinor.getDegree(2, 4) == 51); // Eb
    assert(harmMinor.getDegree(6, 4) == 59); // B natural (raised 7th)

    // Dorian
    Scale dorian(0, Scale::Type::Dorian);
    assert(dorian.getDegree(0, 4) == 48); // C
    assert(dorian.getDegree(2, 4) == 51); // Eb
    assert(dorian.getDegree(5, 4) == 57); // A natural (raised 6th)

    // Test with different roots
    Scale dMajor(2, Scale::Type::Major);
    assert(dMajor.getDegree(0, 4) == 50); // D
    assert(dMajor.getDegree(4, 4) == 57); // A
}

void testScaleQuantizeAllChromaticNotes() {
    Scale cMajor(0, Scale::Type::Major);

    // C Major scale degrees: C=0, D=2, E=4, F=5, G=7, A=9, B=11
    // Chromatic notes that are in-scale should quantize to themselves
    assert(cMajor.quantize(60) == 60); // C
    assert(cMajor.quantize(62) == 62); // D
    assert(cMajor.quantize(64) == 64); // E
    assert(cMajor.quantize(65) == 65); // F
    assert(cMajor.quantize(67) == 67); // G
    assert(cMajor.quantize(69) == 69); // A
    assert(cMajor.quantize(71) == 71); // B

    // Chromatic notes not in scale should quantize to nearest scale degree
    int q61 = cMajor.quantize(61); // C# -> C or D
    assert(q61 == 60 || q61 == 62);

    int q63 = cMajor.quantize(63); // D# -> D or E
    assert(q63 == 62 || q63 == 64);

    int q66 = cMajor.quantize(66); // F# -> F or G
    assert(q66 == 65 || q66 == 67);

    int q68 = cMajor.quantize(68); // G# -> G or A
    assert(q68 == 67 || q68 == 69);

    int q70 = cMajor.quantize(70); // A# -> A or B
    assert(q70 == 69 || q70 == 71);
}

void testChordTonesForAllScaleDegrees() {
    Scale cMajor(0, Scale::Type::Major);

    // I chord (C major): C, E, G
    auto i = cMajor.getChordTones(0, 4, false);
    assert(i.size() == 3);
    assert(i[0] == 48); // C
    assert(i[1] == 52); // E
    assert(i[2] == 55); // G

    // ii chord (D minor): D, F, A
    auto ii = cMajor.getChordTones(1, 4, false);
    assert(ii.size() == 3);
    assert(ii[0] == 50); // D
    assert(ii[1] == 53); // F
    assert(ii[2] == 57); // A

    // V chord (G major): G, B, D
    auto v = cMajor.getChordTones(4, 4, false);
    assert(v.size() == 3);
    assert(v[0] == 55); // G
    assert(v[1] == 59); // B
    assert(v[2] == 62); // D (next octave)
}

void testChordSeventhTonesForAllScaleDegrees() {
    Scale cMajor(0, Scale::Type::Major);

    // I7 (C major 7): C, E, G, B
    auto i7 = cMajor.getChordTones(0, 4, true);
    assert(i7.size() == 4);
    assert(i7[0] == 48); // C
    assert(i7[1] == 52); // E
    assert(i7[2] == 55); // G
    assert(i7[3] == 59); // B

    // ii7 (D minor 7): D, F, A, C
    auto ii7 = cMajor.getChordTones(1, 4, true);
    assert(ii7.size() == 4);
    assert(ii7[0] == 50); // D
    assert(ii7[1] == 53); // F
    assert(ii7[2] == 57); // A
    assert(ii7[3] == 60); // C (next octave)

    // V7 (G dominant 7): G, B, D, F
    auto v7 = cMajor.getChordTones(4, 4, true);
    assert(v7.size() == 4);
    assert(v7[0] == 55); // G
    assert(v7[1] == 59); // B
    assert(v7[2] == 62); // D
    assert(v7[3] == 65); // F
}

void testChordProgressionBounds() {
    // Valid indices
    assert(ChordProgression::isValidDisplayId(1));
    assert(ChordProgression::isValidDisplayId(15));

    // Invalid indices
    assert(!ChordProgression::isValidDisplayId(0));
    assert(!ChordProgression::isValidDisplayId(16));
    assert(!ChordProgression::isValidDisplayId(999));

    // Out-of-bounds get() should return first progression
    auto neg = ChordProgression::get(-1);
    assert(neg.name == "Trance/Pop");

    auto high = ChordProgression::get(100);
    assert(high.name == "Trance/Pop");

    // Valid get
    auto first = ChordProgression::get(0);
    assert(first.name == "Trance/Pop");

    auto last = ChordProgression::get(14);
    assert(last.name == "Tension Arc");
}

void testChordProgressionNames() {
    // All 15 progressions should have non-empty names
    for (int i = 1; i <= 15; ++i) {
        auto name = ChordProgression::getNameByDisplayId(static_cast<uint16_t>(i));
        assert(!name.empty());
    }

    // Names should be unique
    std::set<std::string> names;
    for (int i = 1; i <= 15; ++i) {
        auto name = ChordProgression::getNameByDisplayId(static_cast<uint16_t>(i));
        names.insert(std::string(name));
    }
    assert(names.size() == 15);
}

void testChordProgressionDegreeCount() {
    for (int i = 0; i < ChordProgression::getNumProgressions(); ++i) {
        const auto& prog = ChordProgression::get(i);
        assert(prog.degrees.size() >= 2);
        assert(prog.degrees.size() <= 8);
    }
}

void testChordProgressionGenreTags() {
    std::array<uint16_t, 8> genreCounts{0, 0, 0, 0, 0, 0, 0, 0};

    for (int i = 0; i < ChordProgression::getNumProgressions(); ++i) {
        const auto& prog = ChordProgression::get(i);
        assert(prog.genreTags != 0);
        assert(ChordProgression::getGenreMask(i) == prog.genreTags);

        for (uint8_t genreId = 1; genreId <= genreCounts.size(); ++genreId) {
            if (ChordProgression::hasGenre(i, genreId)) {
                ++genreCounts[genreId - 1];
            }
        }
    }

    assert(!ChordProgression::hasGenre(0, 0));
    assert(!ChordProgression::hasGenre(0, 33));

    for (uint16_t count : genreCounts) {
        assert(count > 0);
    }
}

int main() {
    testScaleQuantization();
    testChordProgression();

    // P1: Expanded tests
    testScaleDegreeWrapping();
    testAllScaleTypes();
    testScaleQuantizeAllChromaticNotes();
    testChordTonesForAllScaleDegrees();
    testChordSeventhTonesForAllScaleDegrees();
    testChordProgressionBounds();
    testChordProgressionNames();
    testChordProgressionDegreeCount();
    testChordProgressionGenreTags();

    std::cout << "All Harmony tests passed!\n";
    return 0;
}
