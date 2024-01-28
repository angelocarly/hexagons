#include <iostream>

#include "burst/AssetLoader.h"
#include "burst/Engine.h"

#include "TestPresenter.h"

#include <imgui.h>
#include <ImGuiFileDialog.h>

class ExampleEngine
:
    public burst::Engine
{
    public:
        ExampleEngine( std::size_t inWidth, std::size_t inHeight, const char * inTitle )
        :
            burst::Engine( inWidth, inHeight, inTitle ),
            mEmptyPresenter()
        {
            std::uint16_t width = 512;
            std::uint16_t height = 512;
            auto data = std::vector< std::uint8_t >();
            data.resize( width * height * 4 );
            for( auto i = 0; i < width * height * 4; i += 4 )
            {
                data[ i + 0 ] = rand() % 255;
                data[ i + 1 ] = rand() % 255;
                data[ i + 2 ] = rand() % 255;
                data[ i + 3 ] = 255;
            }
            burst::ImageAsset asset = { width, height, data };
            mPresenter = std::make_shared< feedbackloop::TestPresenter >( GetPresentContext(), asset );
        }

        virtual void Update( float inDelta ) override
        {
            ImGui::BeginMainMenuBar();
            if (ImGui::MenuItem("Open Image.."))
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose Image", ".png", "/Users/angelo/Projects/nel.re/static/images/.");

            // display
            if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
            {
                // action if OK
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                    std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                    // Do something with the image
                    auto imageResource = burst::AssetLoader::LoadImage( filePathName );
                    mPresenter = std::make_shared< feedbackloop::TestPresenter >( GetPresentContext(), imageResource );
                }

                // close
                ImGuiFileDialog::Instance()->Close();
            }
            ImGui::EndMainMenuBar();

            if( mPresenter ) mPresenter->Update( inDelta );
        }

        virtual burst::Presenter & GetPresenter() const override
        {
            if( mPresenter ) return ( burst::Presenter & ) * mPresenter;
            return ( burst::Presenter & ) mEmptyPresenter;
        }

    private:
        std::shared_ptr< feedbackloop::TestPresenter > mPresenter;

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
    auto engine = ExampleEngine( 1920, 1080, "Pixel sort" );
    engine.Run();
}