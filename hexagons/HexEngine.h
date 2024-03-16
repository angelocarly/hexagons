//
// Created by magnias on 3/16/24.
//

#ifndef HEXAGONS_HEXENGINE_H
#define HEXAGONS_HEXENGINE_H

#include "burst/AssetLoader.h"
#include "burst/Engine.h"

#include "hexagons/HexagonView.h"

namespace hex
{
    class Engine
    :
        public burst::Engine,
        public burst::Presenter
    {
        public:
            Engine( std::size_t inWidth, std::size_t inHeight, const char * inTitle );

            void Compute( vk::CommandBuffer inCommandBuffer ) const override;
            void Present( vk::CommandBuffer inCommandBuffer ) const override;

            virtual void Update( float inDelta ) override;

        private:
            std::shared_ptr <hex::HexagonView> mHexagonView;

            float mHexSize = 15.0f;
    };
}

#endif //HEXAGONS_HEXENGINE_H
