// Copyright 2013 Dolphin Emulator Project / 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <cstdlib>
#include <type_traits>

namespace Common {

constexpr float PI = 3.1415926535f;

template <class T>
struct Rectangle {
    T left{};
    T top{};
    T right{};
    T bottom{};

    constexpr Rectangle() = default;

    constexpr Rectangle(T left, T top, T right, T bottom)
        : left(left), top(top), right(right), bottom(bottom) {}

    [[nodiscard]] T GetWidth() const {
        if constexpr (std::is_floating_point_v<T>) {
            return std::abs(right - left);
        } else {
            return std::abs(static_cast<std::make_signed_t<T>>(right - left));
        }
    }

    [[nodiscard]] T GetHeight() const {
        if constexpr (std::is_floating_point_v<T>) {
            return std::abs(bottom - top);
        } else {
            return std::abs(static_cast<std::make_signed_t<T>>(bottom - top));
        }
    }

    [[nodiscard]] Rectangle<T> TranslateX(const T x) const {
        return Rectangle{left + x, top, right + x, bottom};
    }

    [[nodiscard]] Rectangle<T> TranslateY(const T y) const {
        return Rectangle{left, top + y, right, bottom + y};
    }

    [[nodiscard]] Rectangle<T> Scale(const float s) const {
        return Rectangle{left, top, static_cast<T>(left + GetWidth() * s),
                         static_cast<T>(top + GetHeight() * s)};
    }
};

template <typename T>
Rectangle(T, T, T, T) -> Rectangle<T>;

} // namespace Common
