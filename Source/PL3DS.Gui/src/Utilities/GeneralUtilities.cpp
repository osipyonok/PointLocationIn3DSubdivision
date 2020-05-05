#include "PL3DS.Gui/Utilities/GeneralUtilities.h"

#include <Math.Core/CommonUtilities.h>

#include <QColor>


#include <chrono>
#include <random>


#include <windows.h> 
#include <psapi.h> 

#ifdef max // surprise from minwindef.h
#undef max
#endif

#ifdef min
#undef min
#endif

namespace Utilities
{
    QColor GenerateRandomColor()
    {
        static constexpr int seed = 1337;
        static std::mt19937 random_generator(seed);

        return QColor(random_generator() % 256, random_generator() % 256, random_generator() % 256);
    }

    struct TimeMemoryLogger::Impl
    {
        bool m_started = false;
        bool m_stopped = false;

        SIZE_T m_memory_on_start;
        SIZE_T m_memory_on_stop;

        std::chrono::time_point<std::chrono::system_clock> m_time_on_start;
        std::chrono::time_point<std::chrono::system_clock> m_time_on_stop;
    };

    TimeMemoryLogger::TimeMemoryLogger()
        : mp_impl(std::make_unique<Impl>())
    {
    }

    TimeMemoryLogger::~TimeMemoryLogger() = default;

    double TimeMemoryLogger::GetElapsedTimeSec() const
    {
        assert(mp_impl->m_started && mp_impl->m_stopped);
        return std::chrono::duration<double>(mp_impl->m_time_on_stop - mp_impl->m_time_on_start).count();
    }

    double TimeMemoryLogger::GetMemoryDifferenceMb() const
    {
        assert(mp_impl->m_started && mp_impl->m_stopped);
        return (static_cast<long long>(mp_impl->m_memory_on_stop) - static_cast<long long>(mp_impl->m_memory_on_start)) / (1024.0 * 1024.0);
    }

    void TimeMemoryLogger::Start()
    {
        assert(!mp_impl->m_started);

        mp_impl->m_started = true;

        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc));

        mp_impl->m_memory_on_start = pmc.WorkingSetSize;

        mp_impl->m_time_on_start = std::chrono::system_clock::now();
    }

    void TimeMemoryLogger::Stop()
    {
        assert(mp_impl->m_started);
        assert(!mp_impl->m_stopped);

        mp_impl->m_stopped = true;

        mp_impl->m_time_on_stop = std::chrono::system_clock::now();

        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc));

        mp_impl->m_memory_on_stop = pmc.WorkingSetSize;
    }
}
