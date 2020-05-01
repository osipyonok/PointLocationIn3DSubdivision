#include "Math.IO/MeshIO.h"

#include <Math.Core/Mesh.h>
#include <Math.Core/MeshPoint.h>
#include <Math.Core/MeshTriangle.h>
#include <Math.Core/Point3D.h>
#include <Math.Core/Triangle.h>
#include <Math.Core/Vector3D.h>

#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include <vector>

#include <OBJ_Loader.h>

#ifndef STLLOADER_IMPLEMENTATION
#define STLLOADER_IMPLEMENTATION
#include <stlloader.h>
#undef STLLOADER_IMPLEMENTATION
#endif // !STLLOADER_IMPLEMENTATION


namespace
{
    bool _LoadObj(const QString& i_src, Mesh& o_mesh)
    {
        try
        {
            objl::Loader loader;
            if (!loader.LoadFile(i_src.toStdString()))
                return false;

            o_mesh.SetName(QFileInfo(i_src).fileName());

            for (const auto& loaded_mesh : loader.LoadedMeshes)
            {
                std::vector<std::pair<Point3D, Vector3D>> point_normals;

                for (const auto& loaded_point : loaded_mesh.Vertices)
                {
                    Point3D point{ loaded_point.Position.X, loaded_point.Position.Y, loaded_point.Position.Z };
                    o_mesh.AddPoint(point);

                    point_normals.emplace_back(point, Vector3D{ loaded_point.Normal.X, loaded_point.Normal.Y, loaded_point.Normal.Z });
                }

                for (size_t i = 0; i < loaded_mesh.Indices.size(); i += 3)
                {
                    const Point3D& point1 = point_normals[loaded_mesh.Indices[i]].first;
                    const Point3D& point2 = point_normals[loaded_mesh.Indices[i + 1]].first;
                    const Point3D& point3 = point_normals[loaded_mesh.Indices[i + 2]].first;

                    o_mesh.AddTriangle(point1, point2, point3);
                }
            }

            return true;
        }
        catch (std::exception& e)
        {
            qDebug() << e.what();
        }
        catch (...)
        {
            qDebug() << "Unknown exception was thrown by 3rd party parser.";
        }

        return false;
    }

    bool _LoadStl(const QString& i_src, Mesh& o_mesh)
    {
        try
        {
            stlloader::Mesh stl;
            stlloader::parse_file(i_src.toStdString().c_str(), stl);

            o_mesh.SetName(QString::fromStdString(stl.name));

            for (const auto& facet : stl.facets)
            {
                auto point1 = o_mesh.AddPoint(facet.vertices[0].x, facet.vertices[0].y, facet.vertices[0].z);
                auto point2 = o_mesh.AddPoint(facet.vertices[1].x, facet.vertices[1].y, facet.vertices[1].z);
                auto point3 = o_mesh.AddPoint(facet.vertices[2].x, facet.vertices[2].y, facet.vertices[2].z);
                
                o_mesh.AddTriangle(*point1, *point2, *point3);
            }

            return true;
        }
        catch (std::exception& e)
        {
            qDebug() << e.what();
        }
        catch (...)
        {
            qDebug() << "Unknown exception was thrown by 3rd party parser.";
        }

        return false;
    }

    bool _WriteStl(const QString& i_dest, const Mesh& i_mesh)
    {
        try
        {
            stlloader::Mesh stl;
            stl.name = i_mesh.GetName().toStdString();
            
            for (size_t i = 0; i < i_mesh.GetTrianglesCount(); ++i)
            {
                auto triangle = i_mesh.GetTriangle(i).lock();
                
                stlloader::Facet facet;

                facet.vertices[0].x = static_cast<float>(triangle->GetPoint(0).GetX());
                facet.vertices[0].y = static_cast<float>(triangle->GetPoint(0).GetY());
                facet.vertices[0].z = static_cast<float>(triangle->GetPoint(0).GetZ());

                facet.vertices[1].x = static_cast<float>(triangle->GetPoint(1).GetX());
                facet.vertices[1].y = static_cast<float>(triangle->GetPoint(1).GetY());
                facet.vertices[1].z = static_cast<float>(triangle->GetPoint(1).GetZ());

                facet.vertices[2].x = static_cast<float>(triangle->GetPoint(2).GetX());
                facet.vertices[2].y = static_cast<float>(triangle->GetPoint(2).GetY());
                facet.vertices[2].z = static_cast<float>(triangle->GetPoint(2).GetZ());

                auto normal = triangle->GetNormal();
                facet.normal.x = static_cast<float>(normal.GetX());
                facet.normal.y = static_cast<float>(normal.GetY());
                facet.normal.z = static_cast<float>(normal.GetZ());

                stl.facets.emplace_back(facet);
            }

            stlloader::write_file(i_dest.toStdString().c_str(), stl, stlloader::Format::binary);
            return true;
        }
        catch (std::exception& e)
        {
            qDebug() << e.what();
        }
        catch (...)
        {
            qDebug() << "Unknown exception was thrown by 3rd party parser.";
        }

        return false;
    }
}

bool ReadMesh(const QString& i_src, Mesh& o_mesh)
{
    QFileInfo file_info(i_src);

    if (file_info.completeSuffix().toLower() == QStringLiteral("obj"))
    {
        return _LoadObj(i_src, o_mesh);
    }
    else if (file_info.completeSuffix().toLower() == QStringLiteral("stl"))
    {
        return _LoadStl(i_src, o_mesh);
    }
    else 
    {
        Q_ASSERT(false);
    }

    return false;
}

 bool WriteMesh(const QString& i_dest, const Mesh& i_mesh)
{
     QFileInfo file_info(i_dest);
     if (file_info.completeSuffix().toLower() != QStringLiteral("stl"))
     {
         qDebug() << "Only stl files are supported by this function";
         file_info.setFile(i_dest + QDir::separator() + QStringLiteral(".stl"));
     }

     return _WriteStl(file_info.absoluteFilePath(), i_mesh);
}
