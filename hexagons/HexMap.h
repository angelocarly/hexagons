#ifndef HEXAGONS_HEXMAP_H
#define HEXAGONS_HEXMAP_H

#include <glm/glm.hpp>

#include <memory>
#include <unordered_map>
#include <stdexcept>
#include <span>

namespace hex
{
    template< typename T, int Radius >
    class HexMap
    {
        public:
            static const std::size_t kElementCount = 1 + 6 * Radius * (Radius + 1 ) / 2;

        private:

            std::size_t
            GetIndex( const glm::ivec2 & inPos ) const
            {
                glm::vec3 sig = glm::vec3( glm::sign( inPos.x ), glm::sign( inPos.y ), glm::sign( - inPos.y - inPos.x ) );
                glm::vec3 shiftedpos = glm::vec3(sig.z, sig.x, sig.y);
                glm::vec3 mask = glm::vec3(1) - max(sig * shiftedpos, glm::vec3( 0 ) );

                glm::vec3 u = mask * sig;

                glm::mat2 transform = glm::mat2(
                    u.x, u.y,
                    u.z, u.x
                );

                glm::vec2 subCoord = inverse( transform ) * inPos - glm::vec2(1, 0);
                int leg = 3 * int(u.x) + int(u.y);
                if (leg < 0) leg += 7;
                leg -= 1;

                if ( inPos == glm::ivec2( 0 ) ) return 0;
                int index = 1 + 6 * int(subCoord.x) * (int(subCoord.x) + 1) / 2 + leg * int(subCoord.x + 1.0) + int(subCoord.y);

                return index;
            }

        public:
            HexMap()
            {
            }

            ~HexMap() = default;

            T & operator[]( glm::ivec2 & inKey )
            {
                return elements[ GetIndex( inKey ) ];
            }

            T & operator[]( const glm::ivec2 & inKey )
            {
                return elements[GetIndex( inKey )];
            }

            std::span< T, kElementCount > GetElements()
            {
                return std::span< T, kElementCount >( elements );
            }

            bool Contains( const glm::ivec2 & inKey ) const
            {
                return GetIndex( inKey ) < kElementCount;
            }

        private:
            T elements[ kElementCount ] = {};
    };
}

#endif
