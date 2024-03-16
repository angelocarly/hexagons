#ifndef hexagons_TestPresenter_h
#define hexagons_TestPresenter_h

#include "burst/Presenter.h"
#include "burst/AssetLoader.h"
#include "burst/Gui.h"

#include "vkt/Device.h"
#include "vkt/ForwardDecl.h"
#include "vkt/Image.h"
#include "imgui.h"
#include "burst/ShaderEditor.h"

#include <glm/glm.hpp>

#include <chrono>

namespace hexagons
{
    class TestPresenter
    :
        public burst::Presenter
    {
        public:
            TestPresenter( burst::PresentContext const & inContext, burst::ImageAsset inImage );
            ~TestPresenter();

            void Update( float inDelta );
            void Compute( vk::CommandBuffer inCommandBuffer ) const override;
            void Present( vk::CommandBuffer inCommandBuffer ) const override;

        private:
            void WriteImage( burst::ImageAsset inImage );
            void ClearDrawImage();

            void PaintDrawImage( const glm::vec2 inpos );

            burst::PresentContext const & mContext;
            burst::ImageAsset mInitialImage;

            struct ImageData
            {
                vkt::ImagePtr mImage;
                vk::Sampler mSampler;
                vk::ImageView mImageView;
            };
            ImageData CreateImageData( vk::Extent2D inExtent );
            void DestroyImageData( ImageData const & inImageData );
            void SaveImage();

            ImageData mDisplayImage;
            ImageData mDrawImage;

            burst::gui::ImageInspector mDisplayInspector;

            std::shared_ptr< burst::ShaderEditor > mShaderEditor;

            vkt::DescriptorSetLayoutsPtr mComputeDescriptorSetLayout;
            vkt::ComputePipelinePtr mComputePipeline;

            vkt::DescriptorSetLayoutsPtr mGraphicsDescriptorSetLayout;
            vkt::GraphicsPipelinePtr mPipeline;

            bool mShouldCompute = false;
            bool mPlaying = false;
            float mTime = 0.0f;
            std::size_t mFrame = 0;
            std::size_t mTotalFrames = 0;
            int mFrameSpeed = 1;
            glm::vec2 mDir = glm::vec2( 0 );

            struct PushConstants
            {
                float mTime;
                std::uint32_t mWidth;
                std::uint32_t mHeight;
                float mSlider0;
            };

            bool mShowDraw = true;
            int mPencilSize = 20;
            float mSlider0 = 0.0f;

            glm::vec2 mLastMouse = glm::vec2( 0 );
    };
}

#endif
