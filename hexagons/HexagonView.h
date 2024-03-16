#ifndef HEXAGONS_HEXAGONVIEW_H
#define HEXAGONS_HEXAGONVIEW_H

#include "burst/Presenter.h"

namespace hex
{
    class HexagonView
    :
        public burst::Presenter
    {
        public:
            HexagonView( burst::PresentContext const & inContext );
            ~HexagonView();

            void Update( float inDelta );

            void Compute( vk::CommandBuffer inCommandBuffer ) const override;

            void Present( vk::CommandBuffer inCommandBuffer ) const override;
    };
}

#endif
