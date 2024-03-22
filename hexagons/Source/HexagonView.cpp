#include "hexagons/HexagonView.h"

#include "vkt/Shader.h"
#include "vkt/ComputePipeline.h"

#include <glm/glm.hpp>

namespace
{
    std::uint8_t HexToInt( std::string inHex )
    {
        std::stringstream ss;
        ss << std::hex << inHex;
        int decimalValue;
        ss >> decimalValue;
        return decimalValue;
    }

    std::array< std::uint8_t, 4 > HexToColor( std::string inColor )
    {
        std::array< std::uint8_t, 4 > out;
        out[ 0 ] = HexToInt( inColor.substr( 0, 2 ) );
        out[ 1 ] = HexToInt( inColor.substr( 2, 2 ) );
        out[ 2 ] = HexToInt( inColor.substr( 4, 2 ) );
        out[ 3 ] = 255;

        return out;
    }

    void FillBuffer( std::span< std::uint8_t > inSpan, std::vector< std::string > inColors )
    {
        for( int i = 0; i < inColors.size(); i++ )
        {
            auto c = HexToColor( inColors.at( i ) );
            inSpan[ i * 4 + 0 ] = c[ 0 ];
            inSpan[ i * 4 + 1 ] = c[ 1 ];
            inSpan[ i * 4 + 2 ] = c[ 2 ];
            inSpan[ i * 4 + 3 ] = c[ 3 ];
        }
    }
}

hex::HexagonView::HexagonView( const burst::PresentContext & inContext, glm::ivec2 inResolution, std::size_t inBufferElements )
    :
    mDevice( inContext.mDevice ),
    mElementCount( inBufferElements ),
    mImage( burst::ImageFactory().Create( inContext.mDevice, inResolution, vk::ImageLayout::eGeneral ) ),
    mImageInspector(
        "Hexagon View",
        vk::Extent2D( inResolution.x, inResolution.y ),
        {
            { mImage->GetSampler(), mImage->GetImageView() }
        },
        [this]( glm::vec2 inPos ){}
    ),
    mComputeDescriptorSetLayout(
        vkt::DescriptorSetLayoutBuilder( inContext.mDevice )
            .AddLayoutBinding( 0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute )
            .AddLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute )
            .AddLayoutBinding( 2, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute )
            .Build( vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR )
    )
{
    // Compute shader
    auto computeShader = vkt::Shader::CreateVkShaderModule( inContext.mDevice, "resources/shaders/hexagons.comp" );

    std::vector< vk::PushConstantRange > pushConstants =
    {
        vk::PushConstantRange
        (
            vk::ShaderStageFlagBits::eCompute,
            0,
            sizeof( PushConstants )
        )
    };

    mComputePipeline = vkt::ComputePipelineBuilder( inContext.mDevice )
        .SetComputeShader( computeShader )
        .SetDescriptorSetLayouts( mComputeDescriptorSetLayout )
        .SetPushConstants( pushConstants )
        .Build();

    inContext.mDevice.GetVkDevice().destroy( computeShader );

    // Buffer
    mBuffer = vkt::BufferFactory( mDevice )
        .CreateBuffer(
            sizeof( float ) * mElementCount,
            vk::BufferUsageFlagBits::eStorageBuffer,
            vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
            "HexagonView"
        );

    // Color
    std::size_t colorSamples = 8;
    mColorBuffer = vkt::BufferFactory( mDevice )
        .CreateBuffer(
            sizeof( std::uint8_t ) * 4 * colorSamples,
            vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc,
            vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
            "Colors"
        );

    std::uint8_t * data = ( std::uint8_t * ) mColorBuffer->MapMemory();
    FillBuffer( std::span< std::uint8_t >( data, colorSamples * 4 ),
        {
            "d9ed92",
            "b5e48c",
            "99d98c",
            "76c893",
            "52b69a",
            "34a0a4",
            "168aad",
            "1a759f",
        }
    );
    mColorBuffer->UnMapMemory();

    mColorImage = burst::ImageFactory().CreateColorLookup( inContext.mDevice, colorSamples, vk::ImageLayout::eGeneral );

    auto c = mDevice.BeginSingleTimeCommands();
    {
        c.copyBufferToImage
        (
            mColorBuffer->GetVkBuffer(),
            mColorImage->GetImage()->GetVkImage(),
            mColorImage->GetImage()->GetVkImageLayout(),
            vk::BufferImageCopy
            (
                0,
                colorSamples * sizeof( std::uint8_t ) * 4,
                1,
                vk::ImageSubresourceLayers( vk::ImageAspectFlagBits::eColor, 0, 0, 1 ),
                vk::Offset3D( 0, 0, 0 ),
                vk::Extent3D( colorSamples, 1, 1 )
            )
        );
    }
    mDevice.EndSingleTimeCommands( c );
}

hex::HexagonView::~HexagonView()
{
    mDevice.WaitIdle();
}

void hex::HexagonView::Compute( vk::CommandBuffer inCommandBuffer, float inHexSize, std::span< float > inData, float inColorScale )
{
    // Copy data to buffer
    std::uint8_t * data = ( std::uint8_t * ) mBuffer->MapMemory();
    std::memcpy( data, inData.data(), sizeof( float ) * mElementCount );
    mBuffer->UnMapMemory();

    // Begin pipeline
    mComputePipeline->Bind( inCommandBuffer );

    // Push buffer descriptor set
    auto bufferInfo = mBuffer->GetVkDescriptorBufferInfo();
    auto writeDescriptorSet = vk::WriteDescriptorSet();
    writeDescriptorSet.setDstBinding( 0 );
    writeDescriptorSet.setDstArrayElement( 0 );
    writeDescriptorSet.setDescriptorType( vk::DescriptorType::eStorageBuffer );
    writeDescriptorSet.setDescriptorCount( 1 );
    writeDescriptorSet.setPBufferInfo( & bufferInfo );
    mComputePipeline->BindPushDescriptorSet( inCommandBuffer, writeDescriptorSet );

    // Push color descriptor set
    auto imageInfo = mColorImage->CreateDescriptorImageInfo();
    writeDescriptorSet = vk::WriteDescriptorSet();
    writeDescriptorSet.setDstBinding( 1 );
    writeDescriptorSet.setDstArrayElement( 0 );
    writeDescriptorSet.setDescriptorType( vk::DescriptorType::eCombinedImageSampler );
    writeDescriptorSet.setDescriptorCount( 1 );
    writeDescriptorSet.setPImageInfo( & imageInfo );
    mComputePipeline->BindPushDescriptorSet( inCommandBuffer, writeDescriptorSet );

    // Push image descriptor set
    imageInfo = mImage->CreateDescriptorImageInfo();
    writeDescriptorSet = vk::WriteDescriptorSet();
    writeDescriptorSet.setDstBinding( 2 );
    writeDescriptorSet.setDstArrayElement( 0 );
    writeDescriptorSet.setDescriptorType( vk::DescriptorType::eStorageImage );
    writeDescriptorSet.setDescriptorCount( 1 );
    writeDescriptorSet.setPImageInfo( & imageInfo );
    mComputePipeline->BindPushDescriptorSet( inCommandBuffer, writeDescriptorSet );

    // Push constants
    static bool sortEven = false;
    sortEven = !sortEven;
    PushConstants thePushConstants
        {
            int( mImage->GetImage()->GetWidth() ),
            int( mImage->GetImage()->GetHeight() ),
            inHexSize,
            inColorScale
        };
    mComputePipeline->PushConstants
        (
            inCommandBuffer,
            vk::ShaderStageFlagBits::eCompute,
            0,
            sizeof( PushConstants ),
            & thePushConstants
        );

    float groupsize = 16;
    inCommandBuffer.dispatch
        (
            ceil( mImage->GetImage()->GetWidth() / groupsize ),
            ceil( mImage->GetImage()->GetHeight() / groupsize ),
            1
        );
}

void hex::HexagonView::Update( float inDelta )
{
    mImageInspector.Update( { true } );
}

