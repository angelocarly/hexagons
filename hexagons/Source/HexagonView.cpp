#include "hexagons/HexagonView.h"

#include "vkt/Shader.h"
#include "vkt/ComputePipeline.h"

#include <glm/glm.hpp>


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
            .AddLayoutBinding( 1, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute )
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
    mBuffer = mBuffer = vkt::BufferFactory( mDevice )
        .CreateBuffer(
            sizeof( float ) * mElementCount,
            vk::BufferUsageFlagBits::eStorageBuffer,
            vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
            "HexagonView"
        );
}

hex::HexagonView::~HexagonView()
{
    mDevice.WaitIdle();
}

void hex::HexagonView::Compute( vk::CommandBuffer inCommandBuffer, float inHexSize, std::span< float > inData, const hex::HexagonView::ColorFunction & inColorFunction )
{
    // Copy data to buffer
    std::uint8_t * data = ( std::uint8_t * ) mBuffer->MapMemory();
    std::memcpy( data, inData.data(), sizeof( float ) * mElementCount );
    mBuffer->UnMapMemory();

    // Begin pipeline
    mComputePipeline->Bind( inCommandBuffer );

    // Push buffer descriptor set
    auto bufferInfo = vk::DescriptorBufferInfo( mBuffer->GetVkBuffer(), 0, sizeof( float ) * mElementCount );
    auto writeDescriptorSet = vk::WriteDescriptorSet();
    writeDescriptorSet.setDstBinding( 0 );
    writeDescriptorSet.setDstArrayElement( 0 );
    writeDescriptorSet.setDescriptorType( vk::DescriptorType::eStorageBuffer );
    writeDescriptorSet.setDescriptorCount( 1 );
    writeDescriptorSet.setPBufferInfo( & bufferInfo );

    mComputePipeline->BindPushDescriptorSet( inCommandBuffer, writeDescriptorSet );

    // Push image descriptor set
    auto imageInfo = mImage->CreateDescriptorImageInfo();
    writeDescriptorSet = vk::WriteDescriptorSet();
    writeDescriptorSet.setDstBinding( 1 );
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
            inColorFunction.tScale,
            inColorFunction.mColorA,
            inColorFunction.mColorB,
            inColorFunction.mColorC,
            inColorFunction.mColorD
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

