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

mat2 bayer()
{
    return mat2( 0.f, 2.f, 3.f, 1.f );
}

mat4 bayer4()
{
    mat2 b2 = bayer();
    mat4 b4 = mat4( 1 );
    for( int x = 0; x < 2; x++ )
    {
        for( int y = 0; y < 2; y++ )
        {
            for( int a = 0; a < 2; a++ )
            {
                for( int b = 0; b < 2; b++ )
                {
                    b4[ x * 2 + a ][ y * 2 + b ] = 4.f * b2[ a ][ b ] + b2[ x ][ y ];
                }
            }
        }
    }

    float n = 2;
    return 1.0f / ( pow( 2, 2 * n + 2 ) ) * 1.3f * b4;
}

float bayer8( ivec2 p )
{
    int[ 64 ] b8 = int[](
        3, 32, 8, 40, 2, 34, 10, 42,
        48, 16, 56, 24, 50, 18, 58, 26,
        12, 44, 4, 36, 14, 46, 6, 38,
        60, 28, 52, 20, 62, 30, 54, 22,
        3, 35, 11, 43, 1, 33, 9, 41,
        51, 19, 59, 27, 49, 17, 57, 25,
        15, 47, 7, 39, 13, 45, 5, 37,
        63, 31, 55, 23, 61, 29, 53, 21
    );

    return b8[ p.y * 8 + p.x] / 64.0f - .0f;
}

vec3 dither( vec3 col )
{
    ivec2 p = ivec2( gl_GlobalInvocationID.xy );

    int n = 8;
    ivec2 bIndex = ivec2(p.x % n, p.y % n);
    
    float bElem = bayer8( bIndex );

    int levels = 6;
    float val = colorValue( col );
    for( int i = 0; i < 3; i++ )
    {
        val = col[ i ];
        if( val < bElem )
        {
            col[ i ] = floor( val * levels ) / float( levels );
        }
        else
        {
            col[ i ] = ceil( val * levels ) / float( levels );
        }
    }

    return col;
}

void main()
{
    ivec2 p = ivec2( gl_GlobalInvocationID.xy );
    if( p.x > PushConstants.mWidth || p.y > PushConstants.mHeight ) return;

    vec4 mask = imageLoad( maskImage, p );
    if( mask.a < .1f ) return;

    vec2 dir = normalize( mask.xy * 2.0f - 1.0f );

    ivec2 pos = ivec2( p );
    vec3 col = imageLoad( outImage, pos ).rgb;

    col = dither( col );

    imageStore( outImage, p, vec4( col, 1 ) );
}
