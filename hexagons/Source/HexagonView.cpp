#include "hexagons/HexagonView.h"

#include <glm/glm.hpp>

hex::HexagonView::HexagonView( const burst::PresentContext & inContext, glm::ivec2 inResolution )
:
    mImage( burst::ImageFactory().Create( inContext.mDevice, inResolution, vk::ImageLayout::eGeneral ) ),
    mImageInspector(
        "Hexagon View",
        vk::Extent2D( inResolution.x, inResolution.y ),
        {
            { mImage->GetSampler(), mImage->GetImageView() },
        },
        [this]( glm::vec2 inPos ){}
    )
{

}

hex::HexagonView::~HexagonView()
{

}

void
hex::HexagonView::Update( float inDelta )
{
    mImageInspector.Update( { true } );
}

void
hex::HexagonView::Compute( vk::CommandBuffer inCommandBuffer ) const
{

}

void
hex::HexagonView::Present( vk::CommandBuffer inCommandBuffer ) const
{

}
