#include "hexagons/HexEngine.h"

#include "hexagons/HexMap.h"

hex::Engine::Engine( std::size_t inWidth, std::size_t inHeight, const char * inTitle )
:
    burst::Engine( inWidth, inHeight, inTitle )
{
    mHexagonView = std::make_shared< hex::HexagonView< float, HexMapImpl::kElementCount > >( GetPresentContext(), glm::ivec2( 1024, 1024 ) );

    mHexMap[ { 0, 0 } ] = 1.0f;
}

void
hex::Engine::Update( float inDelta )
{
    ImGui::BeginMainMenuBar();
    ImGui::EndMainMenuBar();

    ImGui::Begin("Hex data" );
    {
        ImGui::SliderFloat( "Hex size", &mHexSize, 1.0f, 100.0f );
    }
    ImGui::End();

    mHexagonView->Update( inDelta );
}

void
hex::Engine::Compute( vk::CommandBuffer inCommandBuffer )
{
    auto elements = mHexMap.GetElements();
    mHexagonView->Compute( inCommandBuffer, mHexSize, elements );
}

void
hex::Engine::Present( vk::CommandBuffer inCommandBuffer )
{
}