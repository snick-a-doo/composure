#include "compose.hh"
#include "doctest.h"

bool operator== (const Note& n1, const Note& n2)
{
    return n1.time == n2.time
        && n1.duration == n2.duration
        && n1.volume == n2.volume
        && n1.pitch == n2.pitch;
}

TEST_CASE("notes")
{
    Phrase phrase(60);
    SUBCASE("empty")
    {
        CHECK(phrase.notes().empty());
    }
    SUBCASE("add")
    {
        phrase.set_notes(1, 0.5, {69, 70, 71}, 0.5);
        const auto& n = phrase.notes();
        CHECK(n.size() == 3);
        CHECK(n[0] == Note(0.0, 1.0, 0.5, 69));
        CHECK(n[1] == Note(0.5, 1.0, 0.5, 70));
        CHECK(n[2] == Note(1.0, 1.0, 0.5, 71));
    }
    SUBCASE("negative")
    {
        phrase.set_notes(1, 0.5, {69, 70, 71}, -0.5);
        const auto& n = phrase.notes();
        CHECK(n.size() == 3);
        CHECK(n[0] == Note(0.0, 1.0, 0.5, 69));
        CHECK(n[1] == Note(-0.5, 1.0, 0.5, 70));
        CHECK(n[2] == Note(-1.0, 1.0, 0.5, 71));
    }
}
