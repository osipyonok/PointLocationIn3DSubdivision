#include "Math.IO/MeshIO.h"

#include <Math.Core/Mesh.h>
#include <Math.Core/Point3D.h>
#include <Math.Core/Triangle.h>
#include <Math.Core/Vector3D.h>

#include <QFileInfo>

#include <vector>

#include <OBJ_Loader.h>

namespace
{
    bool _LoadObj(const QString& i_src, Mesh& o_mesh)
    {
        objl::Loader loader;
        if (!loader.LoadFile(i_src.toStdString()))
            return false;

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
                
                Point3D average_normal_pnt = (point_normals[loaded_mesh.Indices[i]].second
                                            + point_normals[loaded_mesh.Indices[i + 1]].second
                                            + point_normals[loaded_mesh.Indices[i + 2]].second) / 3;
                Vector3D normal = Vector3D(average_normal_pnt).Normalized();

                if (Triangle(point1, point2, point3).GetNormal() == normal)
                {
                    o_mesh.AddTriangle(point1, point2, point3);
                }
                else
                {
                    o_mesh.AddTriangle(point3, point2, point1);
                }
            }
        }

        return true;
    }
}

bool ReadMesh(const QString& i_src, Mesh& o_mesh)
{
    QFileInfo file_info(i_src);

    if (file_info.completeSuffix().toLower() == QStringLiteral("obj"))
    {
        return _LoadObj(i_src, o_mesh);
    }
    else
    {
        Q_ASSERT(false);
    }

    return false;
}
