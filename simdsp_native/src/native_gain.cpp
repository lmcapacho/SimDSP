#include <algorithm>
#include <cstddef>

extern "C" int simdsp_native_gain_process(
    const float* input,
    float* output,
    std::size_t frames,
    std::size_t channels,
    float gain
) {
    if (input == nullptr || output == nullptr) {
        return -1;
    }

    const std::size_t count = frames * channels;
    for (std::size_t i = 0; i < count; ++i) {
        const float y = input[i] * gain;
        output[i] = std::max(-1.0f, std::min(1.0f, y));
    }
    return 0;
}
