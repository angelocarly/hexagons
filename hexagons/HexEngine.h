//
// Created by magnias on 3/16/24.
//

#ifndef HEXAGONS_HEXENGINE_H
#define HEXAGONS_HEXENGINE_H

#include "burst/AssetLoader.h"
#include "burst/Engine.h"

#include "hexagons/HexagonView.h"
#include "HexMap.h"

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
            static const int kRadius = 10;
            using HexMapImpl = HexMap< float, kRadius >;
            HexMapImpl mHexMap;

            float mHexSize = 15.0f;
            std::shared_ptr< hex::HexagonView< float, HexMapImpl::kElementCount > > mHexagonView;
    };
}

#endif //HEXAGONS_HEXENGINE_H
