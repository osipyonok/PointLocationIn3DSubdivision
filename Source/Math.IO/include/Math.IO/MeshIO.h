#pragma once

#include <Math.IO/API.h>

#include <QString>

#include <memory>

class Mesh;

MATH_IO_API bool ReadMesh(const QString& i_src, Mesh& o_mesh);
MATH_IO_API bool WriteMesh(const QString& i_dest, const Mesh& i_mesh);
