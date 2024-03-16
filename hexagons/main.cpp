#include <iostream>

#include "burst/Utils.h"
#include "HexEngine.h"

#include <glm/glm.hpp>
#include <imgui.h>


int main()
{
    glm::ivec2 resolution = glm::ivec2( 1920, 1080 );
    if( burst::Utils::IsTargetApple() )
    {
        resolution = glm::ivec2( 1520, 780 );
    }
    auto engine = hex::Engine( resolution.x, resolution.y, "Pixel sort" );
    engine.Run();
}