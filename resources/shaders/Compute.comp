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

float colorValue( vec4 c )
{
    return 0.2989f * c.r + 0.5870f * c.g + 0.1140f * c.b;
}

void main()
{
    ivec2 p = ivec2( gl_GlobalInvocationID.xy );
    if( p.x > PushConstants.mWidth || p.y > PushConstants.mHeight ) return;

    vec4 mask = imageLoad( maskImage, p );
    vec2 dir = normalize( mask.xy * 2.0f - 1.0f );
    if( mask.a < .1f ) return;

    float time = PushConstants.mTime;
    float t = sin( time );

    vec2 rand = hash2( p );
//    rand = vec2( 0 );
    ivec2 pos = ivec2( p + dir * 6.5f * PushConstants.mSlider0  + rand );
    vec3 col = imageLoad( outImage, pos ).rgb;

    imageStore( outImage, p, vec4( col, 1 ) );
}