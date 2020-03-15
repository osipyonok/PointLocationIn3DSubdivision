#include "Rendering.Core/RenderingUtilities.h"

#include <Math.Core/Mesh.h>
#include <Math.Core/MeshPoint.h>
#include <Math.Core/MeshTriangle.h>
#include <Math.Core/Point3D.h>
#include <Math.Core/Vector3D.h>

#include <QByteArray>

#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QGeometry>
#include <Qt3dRender/QBuffer>

#include <map>
#include <vector>

namespace
{
    std::vector<Vector3D> _GetPointNormals(const Mesh& i_mesh)
    {
        std::vector<Vector3D> normals(i_mesh.GetPointsCount());
        
        for (size_t i = 0; i < i_mesh.GetPointsCount(); ++i)
        {
            for (auto p_tr : i_mesh.GetPoint(i)->GetTriangles())
            {
                normals[i] += p_tr->GetNormal();
            }

            normals[i].Normalize();
        }

        return std::move(normals);
    }

    template<typename TData>
    void _FillTriangles(const Mesh& i_mesh, QByteArray& i_index_bytes, const std::map<Point3D, size_t>& i_point_index_map)
    {
        Q_ASSERT(i_mesh.GetTrianglesCount() <= std::numeric_limits<TData>::max());

        i_index_bytes.resize(3 * i_mesh.GetTrianglesCount() * sizeof(TData));
        auto p_raw_index_data = reinterpret_cast<TData*>(i_index_bytes.data());
        for (size_t i = 0; i < i_mesh.GetTrianglesCount(); ++i)
        {
            const auto p_tr = i_mesh.GetTriangle(i);
            //todo: fix this
            Q_ASSERT(i_point_index_map.find(p_tr->GetPoint(0)) != i_point_index_map.end());
            Q_ASSERT(i_point_index_map.find(p_tr->GetPoint(1)) != i_point_index_map.end());
            Q_ASSERT(i_point_index_map.find(p_tr->GetPoint(2)) != i_point_index_map.end());

            *p_raw_index_data++ = static_cast<TData>(i_point_index_map.at(p_tr->GetPoint(0)));
            *p_raw_index_data++ = static_cast<TData>(i_point_index_map.at(p_tr->GetPoint(1)));
            *p_raw_index_data++ = static_cast<TData>(i_point_index_map.at(p_tr->GetPoint(2)));
        }
    }
}

namespace Rendering
{
    namespace Utilities
    {
        std::unique_ptr<Qt3DRender::QGeometry> Mesh2QGeometry(const Mesh& i_mesh)
        {
            auto point_normals = _GetPointNormals(i_mesh);

            auto p_geomerty = std::make_unique<Qt3DRender::QGeometry>();

          
            const size_t points_count = i_mesh.GetPointsCount();
            const size_t element_size = 3 + (point_normals.size() > 0 ? 3 : 0);
            const size_t step = element_size * sizeof(float);

            QByteArray buffer_bytes;
            buffer_bytes.resize(points_count * step);

            auto p_raw_data = reinterpret_cast<float*>(buffer_bytes.data());

            std::map<Point3D, size_t> point_index_map;

            for (size_t i = 0; i < points_count; ++i)
            {
                point_index_map[*i_mesh.GetPoint(i)] = i;

                *p_raw_data++ = static_cast<float>(i_mesh.GetPoint(i)->GetX());
                *p_raw_data++ = static_cast<float>(i_mesh.GetPoint(i)->GetY());
                *p_raw_data++ = static_cast<float>(i_mesh.GetPoint(i)->GetZ());

                if (!point_normals.empty())
                {
                    *p_raw_data++ = static_cast<float>(point_normals[i].GetX());
                    *p_raw_data++ = static_cast<float>(point_normals[i].GetY());
                    *p_raw_data++ = static_cast<float>(point_normals[i].GetZ());
                }
            }

            auto p_buf = new Qt3DRender::QBuffer;
            p_buf->setType(Qt3DRender::QBuffer::VertexBuffer);
            p_buf->setData(buffer_bytes);

            size_t offset = 0;

            auto p_pos_attribute = new Qt3DRender::QAttribute(p_buf, Qt3DRender::QAttribute::defaultPositionAttributeName(), Qt3DRender::QAttribute::Float, 3, points_count, offset, step);
            p_geomerty->addAttribute(p_pos_attribute);
            offset += 3 * sizeof(float);

            if (point_normals.size())
            {
                auto p_normal_attribute = new Qt3DRender::QAttribute(p_buf, Qt3DRender::QAttribute::defaultNormalAttributeName(), Qt3DRender::QAttribute::Float, 3, points_count, offset, step);
                p_geomerty->addAttribute(p_normal_attribute);
                offset += 3 * sizeof(float);
            }

            QByteArray index_bytes;
            Qt3DRender::QAttribute::VertexBaseType vertex_type;
            if (i_mesh.GetTrianglesCount() <= std::numeric_limits<quint16>::max())
            {
                vertex_type = Qt3DRender::QAttribute::UnsignedShort;
                _FillTriangles<quint16>(i_mesh, index_bytes, point_index_map);
            }
            else
            {
                vertex_type = Qt3DRender::QAttribute::UnsignedInt;
                _FillTriangles<quint32>(i_mesh, index_bytes, point_index_map);
            }

            auto p_index_buffer = new Qt3DRender::QBuffer;
            p_index_buffer->setType(Qt3DRender::QBuffer::IndexBuffer);
            p_index_buffer->setData(index_bytes);

            auto p_index_attribute = new Qt3DRender::QAttribute(p_index_buffer, vertex_type, 1, i_mesh.GetTrianglesCount());
            p_index_attribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);
            p_geomerty->addAttribute(p_index_attribute);

            return std::move(p_geomerty);
        }
    }
}