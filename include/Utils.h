#pragma once

#include <random>
#include <chrono>

namespace Utils
{
    using ID = std::uint64_t;

    inline ID GenerateId()
    {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<ID> dis(
            std::numeric_limits<ID>::min(),
            std::numeric_limits<ID>::max()
        );
        return dis(mt);
    }

    inline std::chrono::time_point<std::chrono::system_clock> TimeNow()
    {
        return std::chrono::system_clock::now();
    };

    inline std::string TimePointAsString(const std::chrono::system_clock::time_point& tp) {
        std::time_t t = std::chrono::system_clock::to_time_t(tp);
        std::string ts = std::ctime(&t);
        ts.resize(ts.size()-1);
        return ts;
    }

    inline double SnapDownPriceToTick(const double& price, const double& tick)
    {
        double remainder = std::fmod(price, tick);
        if (std::abs(tick - remainder) < 1e-9 || remainder < 1e-9) {
            remainder = 0;
        }
        return price - remainder;
    }
}