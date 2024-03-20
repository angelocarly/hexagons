//
// Created by magnias on 3/17/24.
//

#ifndef HEXAGONS_FORWARDDECL_H
#define HEXAGONS_FORWARDDECL_H

#include "hexagons/HexMap.h"

namespace hex
{
    static const int kRadius = 50;
    using HexMapImpl = HexMap< float, kRadius >;
}

#endif //HEXAGONS_FORWARDDECL_H
