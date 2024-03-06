#version 450

#define WORKGROUP_SIZE 16
layout ( local_size_x = WORKGROUP_SIZE, local_size_y = WORKGROUP_SIZE, local_size_z = 1 ) in;

layout(push_constant) uniform PushConstantsBlock
{
    float mTime;
    int mWidth;
    int mHeight;
    float mSlider0;
} PushConstants;

layout ( binding = 0, rgba8 ) uniform image2D outImage;
layout ( binding = 1, rgba8 ) uniform image2D maskImage;

#define hash(p)  fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453)
#define hash2(p) fract(sin((p)*mat2(127.1,311.7, 269.5,183.3)) *43758.5453123)
#define hash3(p) fract(sin((p)*mat3(127.1,311.7, 74.7,  269.5,183.3,246.1,  113.5,271.9,124.6))*43758.5453123)
#define hash2x3(p) fract(sin((p)*mat3x2(127.1,311.7,  269.5,183.3,  113.5,271.9))*43758.5453123)

float colorValue( vec3 c )
{
    return 0.2989f * c.r + 0.5870f * c.g + 0.1140f * c.b;
}

void main()
{
    ivec2 p = ivec2( gl_GlobalInvocationID.xy );
    if( p.x > PushConstants.mWidth || p.y > PushConstants.mHeight ) return;

    vec4 mask = imageLoad( maskImage, p );

    vec2 middle = vec2( PushConstants.mWidth / 2.0f, PushConstants.mHeight / 2.0f );
    vec2 dir = ( p - middle );
    dir = vec2( sin( dir.x ) - cos( dir.y ), cos( dir.x ) + sin( dir.y ) );

    if( length( dir ) > 1.0f ) dir = normalize( dir );

    // Normalize direction to [0.0 - 1.0f] range
    dir = ( dir + 1.0f ) / 2.0f;

    imageStore( maskImage, p, vec4( dir, 0.0f, 1.0f ) );
}
