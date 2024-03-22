#ifndef HEXAGONS_HEXCOMPUTE_H
#define HEXAGONS_HEXCOMPUTE_H

#include "hexagons/ForwardDecl.h"

#include <vector>
#include <functional>

namespace hex
{
    struct Operation
    {
        std::vector< glm::mat4 > transforms;
        std::vector< int > indices;
    };

    struct Node
    {
        int depth;
        glm::mat4 transform;
        bool processed = false;
        std::vector< Node > children;
        Operation * operation;
    };

    class HexCompute
    {
        public:
            HexCompute( HexMapImpl & inMap );
            ~HexCompute() = default;

            void Step();
            void Write( bool inShowPerLayer, int inLayerDepth );


        private:
            HexMapImpl & mHexMap;
            std::vector< Operation > mOperations;
            Node mNodes;
    };

    class NodeVisitor
    {
        public:
            NodeVisitor( hex::Node & inNode, std::function< void( Node & ) > inFunction );
            ~NodeVisitor() = default;

            void Process();

        private:
            void ProcessNode( Node & inNode );

            Node & mRoot;
            std::function< void( Node & ) > mFunction;
    };
}


#endif //HEXAGONS_HEXCOMPUTE_H
