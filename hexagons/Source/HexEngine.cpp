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

        ImGui::SliderFloat3( "A", & mColorFunction.mColorA[ 0 ],  0.0f, 1.0f, "%.2f" );
        ImGui::SliderFloat3( "B", & mColorFunction.mColorB[ 0 ],  0.0f, 1.0f, "%.2f" );
        ImGui::SliderFloat3( "C", & mColorFunction.mColorC[ 0 ],  0.0f, 1.0f, "%.2f" );
        ImGui::SliderFloat3( "D", & mColorFunction.mColorD[ 0 ],  0.0f, 1.0f, "%.2f" );

        ImGui::SliderFloat( "t scale", & mColorFunction.tScale, 0.0f, 1.0f );

        if( ImGui::Button( "Randomize" ) )
        {
            mColorFunction.mColorA = glm::vec4( ( rand() % 1000 ) / 1000.0f, ( rand() % 1000 ) / 1000.0f, ( rand() % 1000 ) / 1000.0f, 0.0f );
            mColorFunction.mColorB = glm::vec4( ( rand() % 1000 ) / 1000.0f, ( rand() % 1000 ) / 1000.0f, ( rand() % 1000 ) / 1000.0f, 0.0f );
            mColorFunction.mColorC = glm::vec4( ( rand() % 1000 ) / 1000.0f, ( rand() % 1000 ) / 1000.0f, ( rand() % 1000 ) / 1000.0f, 0.0f );
            mColorFunction.mColorD = glm::vec4( ( rand() % 1000 ) / 1000.0f, ( rand() % 1000 ) / 1000.0f, ( rand() % 1000 ) / 1000.0f, 0.0f );
        }

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

    mHexagonView->Compute( inCommandBuffer, mHexSize, elements, mColorFunction );
}

void
hex::Engine::Present( vk::CommandBuffer inCommandBuffer )
{
}
