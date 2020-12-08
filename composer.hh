// Copyright © 2020 Sam Varner
//
// This file is part of Composure.
//
// Composure is free software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// Composure is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with Composure.
// If not, see <http://www.gnu.org/licenses/>.

#include "phrase.hh"

/// Generate notes and add them to a Phrase.
/// @param phrase A set of notes to add to, using the notes at the end as a starting
///     point.  If phrase is empty, copies of tonic are used as the starting point.
/// @param tonic A the MIDI note number for the tonic of the key.  60 is middle C, 61 is a
///     half step higher, etc.  All generated notes are in the key.
/// @param voices The number of voices in the generated phrase.
/// @param max_range Note generation stops when the number of half steps between the
///    highest and lowest note in the voices exceeds this value.
/// @return A Phrase with generated notes.
Phrase compose(const Phrase& phrase, int tonic, int voices, int max_range);

/// A fitness function determines points of interest in the Phrase.  Sections that start
/// at those points are extracted, spliced and returned.  The sections may overlap.
Phrase edit(const Phrase& phrase);
