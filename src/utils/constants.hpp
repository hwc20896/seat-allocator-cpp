#pragma once

#include <QColor>

namespace Constants {
    inline constexpr auto DEFAULT_COLOR = "black";

    inline constexpr auto SHUFFLE_END_SOUND = "qrc:/effects/sounds/snd_pling.wav";
    inline constexpr auto SHUFFLE_START_SOUND = "qrc:/effects/sounds/snd_shuffle_start.wav";
    inline constexpr auto SHUFFLE_WHEEL_CLICK_SOUND = "qrc:/effects/sounds/snd_wheel_click.wav";

    inline constexpr auto DEFAULT_BACKGROUND = QColor(255, 255, 255);
    inline constexpr auto TAGGED_BACKGROUND = QColor(144,238, 144);
    inline constexpr auto SWAPPED_BACKGROUND = QColor(255, 255, 224);
    inline constexpr auto SHOW_ORIGINAL_BACKGROUND = QColor(207, 248, 248);

    inline constexpr auto MAX_ATTEMPTS = 1000;
    inline constexpr auto NUM_THREADS = 4;
    inline constexpr auto ATTEMPTS_PER_THREAD = MAX_ATTEMPTS / NUM_THREADS;
}