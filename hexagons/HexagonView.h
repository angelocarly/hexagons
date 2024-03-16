#ifndef HEXAGONS_HEXAGONVIEW_H
#define HEXAGONS_HEXAGONVIEW_H

#include "burst/Presenter.h"
#include "burst/Image.h"
#include "burst/Gui.h"

#include <glm/glm.hpp>

namespace hex
{
    class HexagonView
    :
        public burst::Presenter
    {
        public:
            HexagonView( const burst::PresentContext & inContext, glm::ivec2 inResolution );
            ~HexagonView();

            void Update( float inDelta );

            void Compute( vk::CommandBuffer inCommandBuffer ) const override;

            void Present( vk::CommandBuffer inCommandBuffer ) const override;

        private:
            std::shared_ptr< burst::Image > mImage;
            burst::gui::ImageInspector mImageInspector;
    };
}

#endif
