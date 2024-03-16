#ifndef HEXAGONS_HEXAGONVIEW_H
#define HEXAGONS_HEXAGONVIEW_H

#include "burst/Presenter.h"
#include "burst/Image.h"
#include "burst/Gui.h"
#include "vkt/DescriptorSetLayout.h"

#include <glm/glm.hpp>

namespace hex
{
    /**
     * HexagonView, responsible for rendering the hexagons themselves.
     * Maintains a single image with the hexagons, and an imgui display to view it.
     */
    class HexagonView
    {
        public:
            HexagonView( const burst::PresentContext & inContext, glm::ivec2 inResolution );
            ~HexagonView();

            void Update( float inDelta );

            void Compute( vk::CommandBuffer inCommandBuffer, float inHexSize ) const;

        private:
            vkt::Device const & mDevice;
            std::shared_ptr< burst::Image > mImage;
            burst::gui::ImageInspector mImageInspector;

            struct PushConstants
            {
                int width;
                int height;
                float hexSize;
            };

            vkt::DescriptorSetLayoutsPtr mComputeDescriptorSetLayout;
            vkt::ComputePipelinePtr mComputePipeline;
    };
}

#endif
