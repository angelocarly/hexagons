//
// Created by magnias on 3/17/24.
//

#include "hexagons/HexCompute.h"

#include "hexagons/GridUtils.h"
#include "hexagons/ForwardDecl.h"

#include "imgui.h"

#include <spdlog/spdlog.h>


hex::HexCompute::HexCompute( HexMapImpl & inMap )
: mHexMap( inMap )
{
    mOperations.push_back( Operation {
        GridUtils::createTriangle( glm::ivec3( 0, 1, -1 ) ),
        { 1, 1, 1, 1, 1, 1 }
    } );
    mOperations.push_back( Operation {
        GridUtils::createLine( glm::ivec3( 3, -3, 0 ) ),
        { 0, 0 }
    } );

    mNodes = Node {
        0,
        GridUtils::createTranslationMatrix( glm::ivec3( 0 ) ),
        false,
        std::vector< Node >(),
        & mOperations[ 0 ]
    };
}

void
hex::HexCompute::Step()
{
    int count = 0;
    NodeVisitor countVisitor( mNodes, [ & count ]( Node & node )
    {
        count += 1;
    } );
    countVisitor.Process();
    spdlog::info( "Node count: {}", count );

    NodeVisitor v( mNodes, [ this ]( Node & node )
    {
        // We don't have to add the children again
        if( node.processed ) return;

        auto childtransforms = node.operation->transforms;
        auto indices = node.operation->indices;
        for( int i = 0; i < childtransforms.size(); i++ )
        {
            auto t = childtransforms.at( i );

            node.children.push_back( Node {
                node.depth + 1,
                node.transform * t,
                false,
                std::vector< Node >(),
                & mOperations.at( indices.at( i ) )
            });
        }

        node.processed = true;
    } );
    v.Process();
}

void
hex::HexCompute::Write( bool inShowPerLayer, int inLayerDepth )
{
    for( auto & a : mHexMap.GetElements() )
    {
        a = 0.0f;
    }

    NodeVisitor v( mNodes, [ inShowPerLayer, inLayerDepth, this ]( Node & node )
    {
        glm::ivec3 pos = node.transform * glm::ivec4( 0, 0, 0, 1 );
        if( inShowPerLayer )
        {
            if( node.depth == inLayerDepth )
            {
                mHexMap[ pos ] += 10;
            }
        }
        else
        {
            mHexMap[ pos ] += node.depth;
        }
    } );
    v.Process();
}

hex::NodeVisitor::NodeVisitor( hex::Node & inNode, std::function< void( Node & ) > inFunction )
:
    mRoot( inNode ),
    mFunction( inFunction )
{
}

void
hex::NodeVisitor::Process()
{
    ProcessNode( mRoot );
}

void
hex::NodeVisitor::ProcessNode( hex::Node & inNode )
{
    // First process the children or else we get infinite recursion
    for( auto & c : inNode.children )
    {
        ProcessNode( c );
    }

    mFunction( inNode );
}

