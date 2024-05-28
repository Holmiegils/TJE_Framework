#ifndef SRENDERDATA_H
#define SRENDERDATA_H

#include <vector>
#include "graphics/material.h"
#include "framework/utils.h"

struct sRenderData {
    Material material;
    std::vector<Matrix44> models;
};

#endif // SRENDERDATA_H
