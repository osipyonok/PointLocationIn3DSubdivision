#include <Math.Core/Mesh.h>
#include <Math.Core/MeshPoint.h>
#include <Math.Core/MeshTriangle.h>
#include <Math.Core/TransformMatrix.h>

#include <Math.DataStructures/VoxelGrid.h>

#include <Math.Algos/Voxelizer.h>
#include <Math.Algos/PointLocalizerVoxelized.h>

#include <Math.IO/MeshIO.h>

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QString>

#include <memory>
#include <random>


#include <windows.h> 
#include <psapi.h> 

#ifdef max // surprise from minwindef.h
#undef max
#endif

#ifdef min
#undef min
#endif

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

// single thread testing application that helps to minimize unwanted influence on performance
int main(int argc, char** argv)
{
    QString stl_folder = "C:/3dData/mesh/a lot of parts/6";
   
    QDirIterator dir_iterator(stl_folder, QStringList() << "*.stl", QDir::Files);
    std::vector<std::unique_ptr<Mesh>> meshes;
    while (dir_iterator.hasNext())
    {
        meshes.emplace_back(std::make_unique<Mesh>());
        auto file = dir_iterator.next();
        bool result = ReadMesh(file, *meshes.back());
        if (!result)
            qDebug() << "Loading failed";
    }

    struct VoxelSize
    {
        double m_size_x = 0;
        double m_size_y = 0;
        double m_size_z = 0;
    };

    std::vector<VoxelSize> test_cases = 
    {
        { 0.1, 0.1, 0.1 },
        { 0.2, 0.2, 0.2 },
        { 0.3, 0.3, 0.3 },
        { 0.4, 0.4, 0.4 },
        { 0.5, 0.5, 0.5 },
        { 0.75, 0.75, 0.75 },
        { 1., 1., 1. },
        { 1.25, 1.25, 1.25 },
        { 1.5, 1.5, 1.5 },
        { 1.75, 1.75, 1.75 },
        { 2., 2., 2. }
    };

    std::srand(0);
    const size_t num_locations = 20000;
    std::vector<Point3D> points_to_locate;
    for (size_t i = 0; i < num_locations; ++i)
    {
        points_to_locate.emplace_back(3 + std::rand() % 40, 3 + std::rand() % 40, 3 + std::rand() % 23);
    }

    for (size_t i = 0; i < test_cases.size(); ++i)
    {
        const auto& test_case = test_cases[i];

        PointLocalizerVoxelized::Params params;
        params.m_voxel_size_x = test_case.m_size_x;
        params.m_voxel_size_y = test_case.m_size_y;
        params.m_voxel_size_z = test_case.m_size_z;
        
        PointLocalizerVoxelized localizer;
        for (const auto& p_mesh : meshes)
        {
            localizer.AddMesh(*p_mesh, TransformMatrix{});
        }

        {
        TimeMemoryLogger logger;
        logger.Start();
        localizer.Build(params);
        logger.Stop();

        qDebug() << "--------------------------------------------------";
        qDebug() << "Voxel size: " << test_case.m_size_x;
        qDebug() << "Time: " << logger.GetElapsedTimeSec();
        qDebug() << "Memory: " << logger.GetMemoryDifferenceMb();
        }

        {
        auto& grid = *localizer.GetCachedGrid().lock();
        auto voxels = grid.GetExistingVoxels();
        double num_triangles = 0;
        for (const auto& p_vox : voxels)
            num_triangles += p_vox->GetTriangles().size();

        qDebug() << "Avg triangles in voxel: " << QString::number(num_triangles / voxels.size(), 'f', 4).toStdString().c_str();
        }

        {
            double time = 0;

            for (size_t k = 0; k < num_locations; ++k)
            {
                TimeMemoryLogger logger;
                logger.Start();
                localizer.Localize(points_to_locate[k]);
                logger.Stop();
                time += logger.GetElapsedTimeSec();
            }

            qDebug() << "Query time: " << QString::number(time / num_locations, 'f', 8).toStdString().c_str();
        }
    }

    return 0;
}