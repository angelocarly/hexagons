#include "feedbackloop/TestPresenter.h"

#include "vkt/Buffer.h"
#include "vkt/Device.h"
#include "vkt/ForwardDecl.h"
#include "vkt/GraphicsPipeline.h"
#include "vkt/RenderPass.h"
#include "vkt/Shader.h"
#include "vkt/ComputePipeline.h"
#include "imgui_impl_vulkan.h"

#include <imgui.h>
#include <ImGuiFileDialog.h>

#include <glm/glm.hpp>

#include <spdlog/spdlog.h>
#include <iostream>

feedbackloop::TestPresenter::TestPresenter( burst::PresentContext const & inContext, burst::ImageAsset inImage )
:
    mContext( inContext ),
    mComputeDescriptorSetLayout(
        vkt::DescriptorSetLayoutBuilder( mContext.mDevice )
        .AddLayoutBinding( 0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute )
        .AddLayoutBinding( 1, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute )
        .Build( vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR )
    ),
    mGraphicsDescriptorSetLayout(
        vkt::DescriptorSetLayoutBuilder( mContext.mDevice )
        .AddLayoutBinding( 0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment )
        .Build( vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR )
    ),
    mInitialImage( inImage ),
    mDrawImage( CreateImageData( vk::Extent2D( inImage.mWidth, inImage.mHeight ) ) ),
    mDisplayImage( CreateImageData( vk::Extent2D( inImage.mWidth, inImage.mHeight ) ) ),
    mDisplayInspector(
        "Display",
        vk::Extent2D( mDisplayImage.mImage->GetWidth(), mDisplayImage.mImage->GetHeight() ),
        {
            { mDisplayImage.mSampler, mDisplayImage.mImageView },
            { mDrawImage.mSampler, mDrawImage.mImageView }
        },
        [this]( glm::vec2 inPos ){ PaintDrawImage( inPos ); }
    ),
    mShaderEditor( std::make_shared< burst::ShaderEditor >( inContext.mDevice, "zep.cfg" ) )
{
    WriteImage( inImage );

    // Compute shader
    auto computeShader = vkt::Shader::CreateVkShaderModule( mContext.mDevice, "resources/shaders/Compute.comp" );

    std::vector< vk::PushConstantRange > pushConstants =
    {
        vk::PushConstantRange
        (
            vk::ShaderStageFlagBits::eCompute,
            0,
            sizeof( PushConstants )
        )
    };

    mComputePipeline = vkt::ComputePipelineBuilder( mContext.mDevice )
        .SetComputeShader( computeShader )
        .SetDescriptorSetLayouts( mComputeDescriptorSetLayout )
        .SetPushConstants( pushConstants )
        .Build();

    mContext.mDevice.GetVkDevice().destroy( computeShader );

    // Present shader
    auto vertexShader = vkt::Shader::CreateVkShaderModule( mContext.mDevice, "resources/shaders/ScreenRect.vert" );
    auto fragmentShader = vkt::Shader::CreateVkShaderModule( mContext.mDevice, "resources/shaders/Sampler.frag" );

    mPipeline = vkt::GraphicsPipelineBuilder( mContext.mDevice )
        .SetDescriptorSetLayouts( mGraphicsDescriptorSetLayout )
        .SetVertexShader( vertexShader )
        .SetFragmentShader( fragmentShader )
        .SetRenderPass( mContext.mRenderPass->GetVkRenderPass() )
        .Build();

    mContext.mDevice.GetVkDevice().destroy( vertexShader );
    mContext.mDevice.GetVkDevice().destroy( fragmentShader );

    ClearDrawImage();
}

feedbackloop::TestPresenter::~TestPresenter()
{
    mContext.mDevice.GetVkDevice().waitIdle();

    DestroyImageData( mDrawImage );
    DestroyImageData( mDisplayImage );
}

void
feedbackloop::TestPresenter::Compute( vk::CommandBuffer inCommandBuffer ) const
{
    if( !mShouldCompute ) return;

    if( !mShaderEditor ) return;

    auto pipeline = mShaderEditor->GetShader();

    if( !pipeline ) return;

    // Begin pipeline
    pipeline->Bind( inCommandBuffer );

    // Push image descriptor set
    auto imageInfo = vk::DescriptorImageInfo
    (
        mDisplayImage.mSampler,
        mDisplayImage.mImageView,
        vk::ImageLayout::eGeneral
    );

    auto theWriteDescriptorSet = vk::WriteDescriptorSet();
    theWriteDescriptorSet.setDstBinding( 0 );
    theWriteDescriptorSet.setDstArrayElement( 0 );
    theWriteDescriptorSet.setDescriptorType( vk::DescriptorType::eStorageImage );
    theWriteDescriptorSet.setDescriptorCount( 1 );
    theWriteDescriptorSet.setPImageInfo( & imageInfo );

    pipeline->BindPushDescriptorSet( inCommandBuffer, theWriteDescriptorSet );

    // Push image mask descriptor set
    auto drawImageInfo = vk::DescriptorImageInfo
    (
        mDrawImage.mSampler,
        mDrawImage.mImageView,
        vk::ImageLayout::eGeneral
    );

    auto writeDescriptorSet = vk::WriteDescriptorSet();
    writeDescriptorSet.setDstBinding( 1 );
    writeDescriptorSet.setDstArrayElement( 0 );
    writeDescriptorSet.setDescriptorType( vk::DescriptorType::eStorageImage );
    writeDescriptorSet.setDescriptorCount( 1 );
    writeDescriptorSet.setPImageInfo( & drawImageInfo );

    pipeline->BindPushDescriptorSet( inCommandBuffer, writeDescriptorSet );

    // Push constants
    static bool sortEven = false;
    sortEven = !sortEven;
    PushConstants thePushConstants
    {
        mTime,
        std::uint32_t( mDrawImage.mImage->GetWidth() ),
        std::uint32_t( mDrawImage.mImage->GetHeight() ),
        mSlider0
    };
    inCommandBuffer.pushConstants
    (
        pipeline->GetVkPipelineLayout(),
        vk::ShaderStageFlagBits::eCompute,
        0,
        sizeof( PushConstants ),
        & thePushConstants
    );

    float groupsize = 16;
    inCommandBuffer.dispatch
    (
        ceil( mDisplayImage.mImage->GetWidth() / groupsize ),
        ceil( mDisplayImage.mImage->GetHeight() / groupsize ),
        1
    );
}

void
feedbackloop::TestPresenter::Present( vk::CommandBuffer inCommandBuffer ) const
{
    return;
}

void
feedbackloop::TestPresenter::Update( float inDelta )
{
//    ImGui::ShowDemoWindow();

    if( mShaderEditor )
    {
        mShaderEditor->Update();
        mShaderEditor->Display();
    }

    bool mSingleCompute = false;
    ImGui::Begin("feedbackloop");
    {
        if( ImGui::Button( "Play" ) )
        {
            mPlaying = true;
        }
        ImGui::SameLine();
        if( ImGui::Button( "Pause" ) )
        {
            mPlaying = false;
        }
        ImGui::SameLine();
        if( ImGui::Button( "Step" ) )
        {
            mSingleCompute = true;
        }
        ImGui::SameLine();
        if( ImGui::Button( "Reset" ) )
        {
            WriteImage( mInitialImage );
            mPlaying = false;
            mFrame = 0;
        }
        ImGui::SameLine();
        if( ImGui::Button( "Restart" ) )
        {
            WriteImage( mInitialImage );
            mFrame = 0;
        }

        ImGui::SliderFloat( "Power", &mSlider0, 0.0f, 1.0f );
        ImGui::SliderInt( "Frame limiter", &mFrameSpeed, 1, 100 );

        ImGui::Text( "Frame: %zu", mFrame );

        ImGui::Separator();

        ImGui::Checkbox( "Show draw", & mShowDraw );
        ImGui::SliderInt( "Pencil size", & mPencilSize, 2, 500);

        ImGui::Separator();

        if( ImGui::Button( "Reset draw" ) )
        {
            ClearDrawImage();
        }
        ImGui::SameLine();
        if( ImGui::Button( "Export image") )
        {
            SaveImage();
        }

        const char* items[] = { "Dither", "Draw", "Mask" };
        static int item_current = 1;
        if( ImGui::ListBox("listbox", &item_current, items, IM_ARRAYSIZE(items), 4) )
        {
            switch ( item_current )
            {
                case 0:
                    mShaderEditor = std::make_shared< burst::ShaderEditor >( mContext.mDevice, "dither.comp.glsl" );
                    break;
                case 1:
                    mShaderEditor = std::make_shared< burst::ShaderEditor >( mContext.mDevice, "compute.comp.glsl" );
                    break;
                case 2:
                    mShaderEditor = std::make_shared< burst::ShaderEditor >( mContext.mDevice, "mask.comp.glsl" );
                    break;
                default:
                    break;
            }

            mShaderEditor->CompileShader();
        }
    }
    ImGui::End();

    mDisplayInspector.Update( { true, mShowDraw } );

    mShouldCompute = false;
    mTotalFrames++;
    if( mPlaying && mTotalFrames % mFrameSpeed == 0 || mSingleCompute )
    {
        mShouldCompute = true;
        mFrame++;
    }
}

void
feedbackloop::TestPresenter::WriteImage( burst::ImageAsset inImage )
{
    // Buffer
    auto stagingBuffer = vkt::BufferFactory( mContext.mDevice ).CreateBuffer
    (
        sizeof( glm::vec4 ) * inImage.mWidth * inImage.mHeight,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc,
        vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
        "PointBuffer"
    );

    // Store image data
    auto bufferData = ( std::uint32_t * ) stagingBuffer->MapMemory();
    {
        std::memcpy( bufferData, inImage.mPixels.data(), sizeof( std::uint8_t ) * 4  * inImage.mWidth * inImage.mHeight );
    }
    stagingBuffer->UnMapMemory();

    auto commandBuffer = mContext.mDevice.BeginSingleTimeCommands();
    {
        // Copy the staging buffer into the image
        commandBuffer.copyBufferToImage
        (
            stagingBuffer->GetVkBuffer(),
            mDisplayImage.mImage->GetVkImage(),
            vk::ImageLayout::eGeneral,
            vk::BufferImageCopy
                (
                    0,
                    0,
                    0,
                    vk::ImageSubresourceLayers
                        (
                            vk::ImageAspectFlagBits::eColor,
                            0,
                            0,
                            1
                        ),
                    vk::Offset3D( 0, 0, 0 ),
                    vk::Extent3D( inImage.mWidth, inImage.mHeight, 1 )
                )
        );
    }
    mContext.mDevice.EndSingleTimeCommands( commandBuffer );
}

void
feedbackloop::TestPresenter::PaintDrawImage( const glm::vec2 inPos )
{
    // Only draw strokes
    if( mLastMouse == inPos ) return;

    auto dir = glm::normalize( mLastMouse - inPos );
    auto mappedDir = ( dir + 1.0f ) /2.0f;
    mLastMouse = inPos;

    glm::vec2 pencilSize = glm::vec2( mPencilSize, mPencilSize );

    if( inPos.x < 0 || inPos.y < 0 || inPos.x + pencilSize.x > mDrawImage.mImage->GetWidth() || inPos.y + pencilSize.y > mDrawImage.mImage->GetHeight() )
    {
        return;
    }

    // Update the mask image at the position
    // Buffer
    auto stagingBuffer = vkt::BufferFactory( mContext.mDevice ).CreateBuffer
    (
        sizeof( glm::vec4 ) * pencilSize.x * pencilSize.y,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc,
        vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
        "MaskStagingBuffer"
    );

    // Store pencil data
    auto bufferData = ( std::uint8_t * ) stagingBuffer->MapMemory();
    {
        std::uint8_t value = 255;
        if( ImGui::IsKeyDown(ImGuiKey::ImGuiKey_LeftShift)) value = 0;

        std::size_t i = 0;
        for( std::size_t y = 0; y < pencilSize.y; y++ )
        {
            for( std::size_t x = 0; x < pencilSize.x; x++ )
            {
                bufferData[ i + 0 ] = mappedDir.x * 255;
                bufferData[ i + 1 ] = mappedDir.y * 255;
                bufferData[ i + 2 ] = 0;
                bufferData[ i + 3 ] = 255;
                i += 4;
            }
        }
    }
    stagingBuffer->UnMapMemory();

    auto commandBuffer = mContext.mDevice.BeginSingleTimeCommands();
    {
        // Clear the mask image
        commandBuffer.copyBufferToImage
        (
            stagingBuffer->GetVkBuffer(),
            mDrawImage.mImage->GetVkImage(),
            vk::ImageLayout::eGeneral,
            vk::BufferImageCopy
            (
                0,
                0,
                0,
                vk::ImageSubresourceLayers
                (
                    vk::ImageAspectFlagBits::eColor,
                    0,
                    0,
                    1
                ),
                vk::Offset3D( inPos.x, inPos.y, 0 ),
                vk::Extent3D( pencilSize.x, pencilSize.y, 1 )
            )
        );
    }
    mContext.mDevice.EndSingleTimeCommands( commandBuffer );
}

feedbackloop::TestPresenter::ImageData
feedbackloop::TestPresenter::CreateImageData( vk::Extent2D inExtent )
{
    ImageData data;

    // Image
    data.mImage = vkt::ImageFactory( mContext.mDevice ).CreateImage
    (
        inExtent.width,
        inExtent.height,
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc,
        vma::AllocationCreateFlagBits::eDedicatedMemory,
        "Test Image",
        1
    );

    auto commandBuffer = mContext.mDevice.BeginSingleTimeCommands();
    {
        // Image layout
        data.mImage->MemoryBarrier
        (
            commandBuffer,
            vk::ImageLayout::eGeneral,
            vk::AccessFlagBits::eNone,
            vk::AccessFlagBits::eShaderRead,
            vk::PipelineStageFlagBits::eAllCommands,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlagBits::eByRegion
        );
    }
    mContext.mDevice.EndSingleTimeCommands( commandBuffer );

    // Sampler
    data.mSampler = mContext.mDevice.GetVkDevice().createSampler
    (
        vk::SamplerCreateInfo
        (
            vk::SamplerCreateFlags(),
            vk::Filter::eNearest,
            vk::Filter::eNearest,
            vk::SamplerMipmapMode::eLinear,
            vk::SamplerAddressMode::eRepeat,
            vk::SamplerAddressMode::eRepeat,
            vk::SamplerAddressMode::eRepeat,
            0.0f,
            VK_FALSE,
            16.0f,
            VK_FALSE,
            vk::CompareOp::eAlways,
            0.0f,
            0.0f,
            vk::BorderColor::eIntOpaqueBlack,
            VK_FALSE
        )
    );

    // Image view
    data.mImageView = mContext.mDevice.GetVkDevice().createImageView
    (
        vk::ImageViewCreateInfo
        (
            vk::ImageViewCreateFlags(),
            data.mImage->GetVkImage(),
            vk::ImageViewType::e2D,
            vk::Format::eR8G8B8A8Unorm,
            vk::ComponentMapping(),
            vk::ImageSubresourceRange
            (
                vk::ImageAspectFlagBits::eColor,
                0,
                1,
                0,
                1
            )
        )
    );

    return data;
}

void
feedbackloop::TestPresenter::DestroyImageData( const feedbackloop::TestPresenter::ImageData & inImageData )
{
    mContext.mDevice.GetVkDevice().destroy( inImageData.mImageView );
    mContext.mDevice.GetVkDevice().destroy( inImageData.mSampler );
}

void
feedbackloop::TestPresenter::ClearDrawImage()
{
    auto commandBuffer = mContext.mDevice.BeginSingleTimeCommands();
    {
        vk::ClearColorValue cv = vk::ClearColorValue( 0.0f, 0.0f, 0.0f, 0.0f );

        auto subResourceRange = vk::ImageSubresourceRange
            (
                vk::ImageAspectFlagBits::eColor,
                0,
                1,
                0,
                1
            );

        commandBuffer.clearColorImage
        (
            mDrawImage.mImage->GetVkImage(),
            mDrawImage.mImage->GetVkImageLayout(),
            & cv,
            1,
            & subResourceRange
        );

    }
    mContext.mDevice.EndSingleTimeCommands( commandBuffer );
}

void
feedbackloop::TestPresenter::SaveImage()
{
    // Create a buffer to store/read the image
    std::size_t byteSize = mDisplayImage.mImage->GetWidth() * mDisplayImage.mImage->GetHeight() * 4;
    auto imageBuffer = vkt::BufferFactory( mContext.mDevice ).CreateBuffer
    (
        byteSize,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vma::AllocationCreateFlagBits::eHostAccessRandom,
        "MaskStagingBuffer"
    );

    // Copy the image
    auto commandBuffer = mContext.mDevice.BeginSingleTimeCommands();
    {
        commandBuffer.copyImageToBuffer
        (
            mDisplayImage.mImage->GetVkImage(),
            vk::ImageLayout::eGeneral,
            imageBuffer->GetVkBuffer(),
            vk::BufferImageCopy
            (
                0,
                mDisplayImage.mImage->GetWidth(),
                mDisplayImage.mImage->GetHeight(),
                vk::ImageSubresourceLayers
                (
                    vk::ImageAspectFlagBits::eColor,
                    0,
                    0,
                    1
                ),
                vk::Offset3D( 0, 0, 0 ),
                vk::Extent3D( mDisplayImage.mImage->GetWidth(), mDisplayImage.mImage->GetHeight(), 1 )
            )
        );
    }
    mContext.mDevice.EndSingleTimeCommands( commandBuffer );

    burst::ImageAsset imageAsset = burst::ImageAsset();
    imageAsset.mWidth = mDisplayImage.mImage->GetWidth();
    imageAsset.mHeight = mDisplayImage.mImage->GetHeight();

    imageAsset.mPixels.resize( byteSize );

    std::uint8_t * bufferData = ( std::uint8_t * ) imageBuffer->MapMemory();
    std::memcpy( imageAsset.mPixels.data(), bufferData, byteSize );
    imageBuffer->UnMapMemory();

    burst::AssetLoader::SaveImage( imageAsset, "output.png" );
}



