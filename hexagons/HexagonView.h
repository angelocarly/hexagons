#ifndef HEXAGONS_HEXAGONVIEW_H
#define HEXAGONS_HEXAGONVIEW_H

#include "burst/Presenter.h"
#include "burst/Image.h"
#include "burst/Gui.h"

#include "vkt/Buffer.h"
#include "vkt/DescriptorSetLayout.h"
#include "vkt/ComputePipeline.h"

#include "HexMap.h"
#include "vkt/Shader.h"

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
            struct ColorFunction
            {
                glm::vec4 mColorA;
                glm::vec4 mColorB;
                glm::vec4 mColorC;
                glm::vec4 mColorD;
                float tScale;
            };

            HexagonView( const burst::PresentContext & inContext, glm::ivec2 inResolution, std::size_t inBufferElements );
            ~HexagonView();

            void Update( float inDelta );
            void Compute( vk::CommandBuffer inCommandBuffer, float inHexSize, std::span< float > inData, ColorFunction const & inColorFunction );

        private:
            vkt::Device const & mDevice;
            std::shared_ptr< burst::Image > mImage;
            burst::gui::ImageInspector mImageInspector;
            const std::size_t mElementCount;

            struct PushConstants
            {
                int width;
                int height;
                float hexSize;
                float tScale;
                glm::vec4 mColorA;
                glm::vec4 mColorB;
                glm::vec4 mColorC;
                glm::vec4 mColorD;
            };

            vkt::DescriptorSetLayoutsPtr mComputeDescriptorSetLayout;
            vkt::ComputePipelinePtr mComputePipeline;

            vkt::BufferPtr mBuffer;

    };
}

#endif
