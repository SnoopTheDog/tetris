#ifndef PRIMARY_H
#define PRIMARY_H

/*
Triangle making process(?):
  instance and physical device selection
  logical device and queue families
  window surface and swap chain
  image views and framebuffers
  render passes
  graphics pipeline
  command pools and command buffers
  main loop
*/

//#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdbool.h>

typedef struct App {
  GLFWwindow* window;
  VkInstance instance;
  VkPhysicalDevice physicalDevice;
  VkDevice device;
  VkQueue graphicsQueue;
  VkSurfaceKHR surface;
} App;

typedef struct QueueFamilyIndices{
	uint32_t graphicsFamily;
	uint32_t presentFamily;
} QueueFamilyIndices;

extern const uint32_t WIDTH;
extern const uint32_t HEIGHT;

void run();
void initWindow();
void initVulkan();
void mainLoop();
void cleanup();

// The instance is the connection  between the app and vulkan library
void createInstance();

// Selected gpu will be stored in VkPhysicalDevice handle and is destroyed upon VkInstance is destroyed
void selectPhysicalDevice();
// Base device suitability checks
bool isDeviceSuitable(VkPhysicalDevice*);

// Checks if the requested layers are available
bool checkValidationLayerSupport(void);

// Checks which queue families are supported by device
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice*);

// d for debug, helper function to see current "status" of the engine structs
void dDisplayEngineProperties(void);

void createLogicalDevice(void);

#endif