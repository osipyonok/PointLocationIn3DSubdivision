#pragma once

#include <Math.IO/API.h>

#include <QString>

#include <memory>

class Mesh;

MATH_IO_API bool ReadMesh(const QString& i_src, Mesh& o_mesh);
