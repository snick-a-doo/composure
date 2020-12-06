#include "composer.hh"
#include "doctest.h"

TEST_CASE("empty")
{
    Phrase phrase;
    CHECK(phrase.score(100) == "t 0 100\n");
    CHECK(phrase.score() == "");
}

TEST_CASE("table")
{
    std::vector<double> i_table = {10, 7, 5, 3};
    std::vector<double> d_table = {1.0, 0.7, 0.5, 0.3};

    Phrase phrase;
    phrase.set_notes(1, 0.5, {69}, 0.0);
    CHECK(phrase.score(i_table, 50) == "f1 0 4096 10 10 7 5 3\nt 0 50\ni1 0 1 0.5 69\n");
    CHECK(phrase.score(d_table, 50) == "f1 0 4096 10 1 0.7 0.5 0.3\nt 0 50\ni1 0 1 0.5 69\n");
}

TEST_CASE("chord")
{
    Phrase phrase;
    phrase.set_notes(0.1, 0.5, {60, 64, 67}, 0.0);
    CHECK(phrase.end_time() == 0.1);
    std::string out = "i1 0 0.1 0.5 60\ni1 0 0.1 0.5 64\ni1 0 0.1 0.5 67\n";
    CHECK(phrase.score() == out);

    phrase.set_notes(0.1, 0.5, {60, 64, 67}, 0.2);
    out += "i1 0.1 0.1 0.5 60\ni1 0.3 0.1 0.5 64\ni1 0.5 0.1 0.5 67\n";
    CHECK(phrase.score() == out);

    phrase.set_notes(0.1, 0.5, {60, 64, 67}, -0.5);
    out += "i1 0.2 0.1 0.5 60\ni1 -0.3 0.1 0.5 64\ni1 -0.8 0.1 0.5 67\n";
    CHECK(phrase.score() == out);
}
