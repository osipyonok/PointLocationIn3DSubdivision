#pragma once

#include <memory>

class QColor;

namespace Utilities
{
    QColor GenerateRandomColor();

    struct TimeMemoryLogger
    {
        TimeMemoryLogger();
        ~TimeMemoryLogger();

        double GetElapsedTimeSec() const;
        double GetMemoryDifferenceMb() const;

        void Start();
        void Stop();

    private:
        struct Impl;
        std::unique_ptr<Impl> mp_impl;
    };
}
