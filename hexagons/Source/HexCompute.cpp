//
// Created by magnias on 3/17/24.
//

#include <spdlog/spdlog.h>
#include "hexagons/HexCompute.h"
#include "hexagons/GridUtils.h"

void
hex::HexCompute::Step()
{
    if( mPoints.empty() )
    {
        mPoints = GridUtils::createHexagon( glm::ivec3( 0, 2, -2 ) );
        return;
    }

    spdlog::info( "Point count: {}", mPoints.size() );

    for( auto p : mPoints )
    {
        auto hexpoints = GridUtils::createHexagon( glm::ivec3( 2, -2, 0 ) );
        for( auto h : hexpoints )
        {
            glm::ivec3 pos = p * glm::ivec4( 0, 0, 0, 1 );
            if( mHexMap.Contains( pos ) && mHexMap[ pos ] < .5f )
            {
                mPoints.push_back( p * h );
            }
        }
    }
}

void
hex::HexCompute::Write()
{
    for( auto p : mPoints )
    {
        glm::ivec3 pos = p * glm::ivec4( 0, 0, 0, 1 );
        if( mHexMap.Contains( pos ) ) mHexMap[ pos ] += .4f;
    }
}
