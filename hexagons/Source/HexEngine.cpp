#include "hexagons/HexEngine.h"

#include "hexagons/HexMap.h"

hex::Engine::Engine( std::size_t inWidth, std::size_t inHeight, const char * inTitle )
:
    burst::Engine( inWidth, inHeight, inTitle )
{
    auto elementCount = mHexMap.kElementCount;
    mHexagonView = std::make_shared< hex::HexagonView >( GetPresentContext(), glm::ivec2( 2048, 2048) * 2, elementCount );
}

void
hex::Engine::Update( float inDelta )
{
    ImGui::BeginMainMenuBar();
    ImGui::EndMainMenuBar();

    ImGui::Begin("Hex" );
    {
        ImGui::SliderFloat( "Hex size", &mHexSize, 1.0f, 500.0f );

        ImGui::SliderFloat( "t scale", & mColorScale, 0.0f, 0.4f );
//        mColorScale = ( sin( GetRunTime() / 2 ) / 2.0f + 0.5f ) * 0.07f + 0.00005f;

        if( ImGui::Button( "Step" ) )
        {
            mHexCompute.Step();
            mHexCompute.Write( mShowPerLayer, mViewDepth );
        }

        ImGui::Checkbox("Per layer", & mShowPerLayer );
        int prevDepth = mViewDepth;
        ImGui::SliderInt( "depth", & mViewDepth, 0, 40);
        if( mShowPerLayer && prevDepth != mViewDepth )
        {
            mHexCompute.Write( mShowPerLayer, mViewDepth );
        }

    }
    ImGui::End();

    mHexagonView->Update( inDelta );
}

void
hex::Engine::Compute( vk::CommandBuffer inCommandBuffer )
{
    auto elements = mHexMap.GetElements();

    mHexagonView->Compute( inCommandBuffer, mHexSize, elements, mColorScale );
}

void
hex::Engine::Present( vk::CommandBuffer inCommandBuffer )
{
}
