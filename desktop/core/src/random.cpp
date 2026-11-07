#include "lq/random.h"

namespace lq {

SeededRandom::SeededRandom(std::string_view seed)
{
    for (const unsigned char c : seed)
        m_state = (m_state ^ c) * 16777619u;
}

double SeededRandom::next()
{
    m_state += 0x6D2B79F5u;
    std::uint32_t t = m_state;
    t = (t ^ (t >> 15)) * (t | 1u);
    t ^= t + (t ^ (t >> 7)) * (t | 61u);
    return static_cast<double>(t ^ (t >> 14)) / 4294967296.0;
}

void SeededRandom::skip(int count)
{
    for (int i = 0; i < count; ++i)
        next();
}

} // namespace lq
