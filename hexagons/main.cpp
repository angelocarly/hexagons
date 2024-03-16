#include <iostream>

#include "burst/AssetLoader.h"
#include "burst/Engine.h"

#include "hexagons/HexagonView.h"

#include "burst/Utils.h"

#include <glm/glm.hpp>
#include <imgui.h>

class Engine
:
    public burst::Engine
{
    public:
        Engine( std::size_t inWidth, std::size_t inHeight, const char * inTitle )
        :
            burst::Engine( inWidth, inHeight, inTitle ),
            mEmptyPresenter()
        {
            mPresenter = std::make_shared< hex::HexagonView >( GetPresentContext(), glm::ivec2( 1024, 1024 ) );
        }

        virtual void Update( float inDelta ) override
        {
            ImGui::BeginMainMenuBar();
            ImGui::EndMainMenuBar();

            if( mPresenter ) mPresenter->Update( inDelta );
        }

        virtual burst::Presenter & GetPresenter() const override
        {
            if( mPresenter ) return ( burst::Presenter & ) * mPresenter;
            return ( burst::Presenter & ) mEmptyPresenter;
        }

    private:
        std::shared_ptr< hex::HexagonView > mPresenter;

        class EmptyPresenter
        :
            public burst::Presenter
        {
            public:
                void Compute( vk::CommandBuffer inCommandBuffer ) const
                {}
                void Present( vk::CommandBuffer inCommandBuffer ) const
                {}
        } mEmptyPresenter;
};

int main()
{
    glm::ivec2 resolution = glm::ivec2( 1920, 1080 );
    if( burst::Utils::IsTargetApple() )
    {
        resolution = glm::ivec2( 1520, 780 );
    }
    auto engine = Engine( resolution.x, resolution.y, "Pixel sort" );
    engine.Run();
}