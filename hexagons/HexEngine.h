//
// Created by magnias on 3/16/24.
//

#ifndef HEXAGONS_HEXENGINE_H
#define HEXAGONS_HEXENGINE_H

#include "hexagons/HexagonView.h"
#include "hexagons/ForwardDecl.h"

#include "burst/AssetLoader.h"
#include "burst/Engine.h"
#include "HexCompute.h"

namespace hex
{
    class Engine
    :
        public burst::Engine,
        public burst::Presenter
    {
        public:
            Engine( std::size_t inWidth, std::size_t inHeight, const char * inTitle );

            void Compute( vk::CommandBuffer inCommandBuffer ) override;
            void Present( vk::CommandBuffer inCommandBuffer ) override;

            virtual void Update( float inDelta ) override;

        private:
            HexMapImpl mHexMap;

            HexCompute mHexCompute = HexCompute( mHexMap );

            hex::HexagonView::ColorFunction mColorFunction =
            {
                glm::vec4( 0.5, 0.5, 0.5, 0.0 ),
                glm::vec4( 0.5, 0.5, 0.5, 0.0 ),
                glm::vec4( 1.0, 1.0, 1.0, 0.0 ),
                glm::vec4( 0.3, 0.2, 0.2, 0.0 ),
                1.0f
            };

            bool mShowPerLayer = false;
            int mViewDepth;

            float mHexSize = 15.0f;
            std::shared_ptr< hex::HexagonView > mHexagonView;
    };
}

#endif //HEXAGONS_HEXENGINE_H
