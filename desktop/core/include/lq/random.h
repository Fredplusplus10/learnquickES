#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

namespace lq {

// Deterministic generator ported from LQ.util.seeded, so the C++ demo learner matches the web app's.
// Exercise builders take one too: the app seeds it at random, tests with a fixed value.
class SeededRandom
{
public:
    explicit SeededRandom(std::string_view seed);
    explicit SeededRandom(std::uint32_t state) : m_state(state) {}
    double next();  // [0, 1)
    void skip(int count);

private:
    std::uint32_t m_state = 2166136261u;
};

// Fisher-Yates, drawing exactly as LQ.util.shuffle does.
template <typename T>
std::vector<T> shuffled(std::vector<T> list, SeededRandom &rnd)
{
    for (std::size_t i = list.size(); i-- > 1;) {
        const auto j = static_cast<std::size_t>(rnd.next() * static_cast<double>(i + 1));
        std::swap(list[i], list[j]);
    }
    return list;
}

// n distinct elements in random order (all of them when the list is shorter), as LQ.util.sample.
template <typename T>
std::vector<T> sampled(std::vector<T> list, std::size_t n, SeededRandom &rnd)
{
    list = shuffled(std::move(list), rnd);
    list.resize(std::min(n, list.size()));
    return list;
}

template <typename T>
const T &pickOne(const std::vector<T> &list, SeededRandom &rnd)
{
    return list[static_cast<std::size_t>(rnd.next() * static_cast<double>(list.size()))];
}

} // namespace lq
