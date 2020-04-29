#include "PL3DS.Gui/Utilities/GeneralUtilities.h"

#include <Math.Core/CommonUtilities.h>

#include <QColor>

#include <random>

namespace Utilities
{
    QColor GenerateRandomColor()
    {
        static constexpr int seed = 1337;
        static std::mt19937 random_generator(seed);

        return QColor(random_generator() % 256, random_generator() % 256, random_generator() % 256);
    }
}
