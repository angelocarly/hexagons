#ifndef HEXAGONS_HEXCOMPUTE_H
#define HEXAGONS_HEXCOMPUTE_H

#include "hexagons/ForwardDecl.h"

#include <vector>

namespace hex
{
    class HexCompute
    {
        public:
            HexCompute( HexMapImpl & inMap )
            : mHexMap( inMap )
            {
            }

            void Step();
            void Write();

            ~HexCompute() = default;

        private:
            HexMapImpl & mHexMap;

            std::vector< glm::mat4 > mPoints;
    };
}


#endif //HEXAGONS_HEXCOMPUTE_H
