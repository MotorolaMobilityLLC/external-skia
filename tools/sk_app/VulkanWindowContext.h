
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef VulkanWindowContext_DEFINED
#define VulkanWindowContext_DEFINED

#include "SkTypes.h" // required to pull in any SkUserConfig defines

#ifdef SK_VULKAN

#include "vk/GrVkBackendContext.h"
#include "vk/GrVkInterface.h"
#include "vk/VkTestUtils.h"
#include "WindowContext.h"

class GrRenderTarget;

namespace sk_app {

class VulkanWindowContext : public WindowContext {
public:
    ~VulkanWindowContext() override;

    sk_sp<SkSurface> getBackbufferSurface() override;
    void swapBuffers() override;

    bool isValid() override { return fDevice != VK_NULL_HANDLE; }

    void resize(int w, int h) override {
        this->createSwapchain(w, h, fDisplayParams);
    }

    void setDisplayParams(const DisplayParams& params) override {
        this->destroyContext();
        fDisplayParams = params;
        this->initializeContext();
    }

    /** Platform specific function that creates a VkSurfaceKHR for a window */
    using CreateVkSurfaceFn = std::function<VkSurfaceKHR(VkInstance)>;
    /** Platform specific function that determines whether presentation will succeed. */
    using CanPresentFn = sk_gpu_test::CanPresentFn;

    VulkanWindowContext(const DisplayParams&, CreateVkSurfaceFn, CanPresentFn,
                        PFN_vkGetInstanceProcAddr, PFN_vkGetDeviceProcAddr);

private:
    void initializeContext();
    void destroyContext();

    struct BackbufferInfo {
        uint32_t        fImageIndex;          // image this is associated with
        VkSemaphore     fAcquireSemaphore;    // we signal on this for acquisition of image
        VkSemaphore     fRenderSemaphore;     // we wait on this for rendering to be done
        VkCommandBuffer fTransitionCmdBuffers[2]; // to transition layout between present and render
        VkFence         fUsageFences[2];      // used to ensure this data is no longer used on GPU
    };

    BackbufferInfo* getAvailableBackbuffer();
    bool createSwapchain(int width, int height, const DisplayParams& params);
    void createBuffers(VkFormat format, SkColorType colorType);
    void destroyBuffers();

    VkInstance fInstance = VK_NULL_HANDLE;
    VkPhysicalDevice fPhysicalDevice = VK_NULL_HANDLE;
    VkDevice fDevice = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT fDebugCallback = VK_NULL_HANDLE;

    // simple wrapper class that exists only to initialize a pointer to NULL
    template <typename FNPTR_TYPE> class VkPtr {
    public:
        VkPtr() : fPtr(NULL) {}
        VkPtr operator=(FNPTR_TYPE ptr) { fPtr = ptr; return *this; }
        operator FNPTR_TYPE() const { return fPtr; }
    private:
        FNPTR_TYPE fPtr;
    };

    // Create functions
    CreateVkSurfaceFn fCreateVkSurfaceFn;
    CanPresentFn      fCanPresentFn;

    // Vulkan GetProcAddr functions
    VkPtr<PFN_vkGetInstanceProcAddr> fGetInstanceProcAddr;
    VkPtr<PFN_vkGetDeviceProcAddr> fGetDeviceProcAddr;

    // WSI interface functions
    VkPtr<PFN_vkDestroySurfaceKHR> fDestroySurfaceKHR;
    VkPtr<PFN_vkGetPhysicalDeviceSurfaceSupportKHR> fGetPhysicalDeviceSurfaceSupportKHR;
    VkPtr<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR> fGetPhysicalDeviceSurfaceCapabilitiesKHR;
    VkPtr<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR> fGetPhysicalDeviceSurfaceFormatsKHR;
    VkPtr<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR> fGetPhysicalDeviceSurfacePresentModesKHR;

    VkPtr<PFN_vkCreateSwapchainKHR> fCreateSwapchainKHR;
    VkPtr<PFN_vkDestroySwapchainKHR> fDestroySwapchainKHR;
    VkPtr<PFN_vkGetSwapchainImagesKHR> fGetSwapchainImagesKHR;
    VkPtr<PFN_vkAcquireNextImageKHR> fAcquireNextImageKHR;
    VkPtr<PFN_vkQueuePresentKHR> fQueuePresentKHR;

    VkPtr<PFN_vkDestroyInstance> fDestroyInstance;
    VkPtr<PFN_vkDeviceWaitIdle> fDeviceWaitIdle;
    VkPtr<PFN_vkQueueWaitIdle> fQueueWaitIdle;
    VkPtr<PFN_vkDestroyDevice> fDestroyDevice;
    VkPtr<PFN_vkGetDeviceQueue> fGetDeviceQueue;

    sk_sp<const GrVkInterface> fInterface;

    VkSurfaceKHR      fSurface;
    VkSwapchainKHR    fSwapchain;
    uint32_t          fGraphicsQueueIndex;
    VkQueue           fGraphicsQueue;
    uint32_t          fPresentQueueIndex;
    VkQueue           fPresentQueue;

    uint32_t               fImageCount;
    VkImage*               fImages;         // images in the swapchain
    VkImageLayout*         fImageLayouts;   // layouts of these images when not color attachment
    sk_sp<SkSurface>*      fSurfaces;       // surfaces client renders to (may not be based on rts)
    VkCommandPool          fCommandPool;
    BackbufferInfo*        fBackbuffers;
    uint32_t               fCurrentBackbufferIndex;
};

}   // namespace sk_app

#endif // SK_VULKAN

#endif
