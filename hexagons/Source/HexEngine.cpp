#include "hexagons/HexEngine.h"

hex::Engine::Engine( std::size_t inWidth, std::size_t inHeight, const char * inTitle )
:
    burst::Engine( inWidth, inHeight, inTitle )
{
    mHexagonView = std::make_shared< hex::HexagonView >( GetPresentContext(), glm::ivec2( 1024, 1024 ) );
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
hex::Engine::Compute( vk::CommandBuffer inCommandBuffer ) const
{
    mHexagonView->Compute( inCommandBuffer, mHexSize );
}

void
hex::Engine::Present( vk::CommandBuffer inCommandBuffer ) const
{
}
