#pragma once

#include "game.h"

// Image
class Image
{
public:
    Image(const std::string& path)
    {
        
    }

private:
    void* data;
    qoi_desc info;
};
