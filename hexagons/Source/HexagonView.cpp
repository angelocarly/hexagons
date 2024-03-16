#include "hexagons/HexagonView.h"

#include "vkt/Shader.h"
#include "vkt/ComputePipeline.h"

#include <glm/glm.hpp>

hex::HexagonView::HexagonView( const burst::PresentContext & inContext, glm::ivec2 inResolution )
:
    mDevice( inContext.mDevice ),
    mImage( burst::ImageFactory().Create( inContext.mDevice, inResolution, vk::ImageLayout::eGeneral ) ),
    mImageInspector(
        "Hexagon View",
        vk::Extent2D( inResolution.x, inResolution.y ),
        {
            { mImage->GetSampler(), mImage->GetImageView() },
        },
        [this]( glm::vec2 inPos ){}
    ),
    mComputeDescriptorSetLayout(
        vkt::DescriptorSetLayoutBuilder( inContext.mDevice )
            .AddLayoutBinding( 0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute )
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
}

hex::HexagonView::~HexagonView()
{
    mDevice.WaitIdle();
}

void
hex::HexagonView::Update( float inDelta )
{
    mImageInspector.Update( { true } );
}

void
hex::HexagonView::Compute( vk::CommandBuffer inCommandBuffer, float inHexSize ) const
{
    // Begin pipeline
    mComputePipeline->Bind( inCommandBuffer );

    // Push image descriptor set
    auto imageInfo = mImage->CreateDescriptorImageInfo();
    auto writeDescriptorSet = vk::WriteDescriptorSet();
    writeDescriptorSet.setDstBinding( 0 );
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
        inHexSize
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