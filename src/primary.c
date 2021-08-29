#include "primary.h"

/**
 * TO-DO NEXT:
 * validation layers
 */

App app;
App* Engine = &app;

const uint32_t WIDTH = 800U;
const uint32_t HEIGHT = 600U;

const char *validationLayers[1] = {
	"VK_LAYER_KHRONOS_validation"
};
const bool enableValidationLayers = false;

void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
}

void initWindow() {
        glfwInit();

	if (!glfwVulkanSupported()) {
		printf("[ERROR] Vulkan is not available\n");
	}

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        Engine->window = glfwCreateWindow(WIDTH, HEIGHT, "Tetris", NULL, NULL);
}

void initVulkan() {
        createInstance();
	selectPhysicalDevice();
	createLogicalDevice();
	//dDisplayEngineProperties();
}


void selectPhysicalDevice() {
	// Instead of checking for suitability of devices, 
	// consider scoring them and picking the best one 
	Engine->physicalDevice = VK_NULL_HANDLE;

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(Engine->instance, &deviceCount, NULL);
	//printf("Device count: %d\n", deviceCount);

	if (deviceCount == 0) {
		printf("[ERROR] Failed to find physical devices with vulkan support\n");
	}

	VkPhysicalDevice devices[deviceCount];
	vkEnumeratePhysicalDevices(Engine->instance, &deviceCount, devices);

	uint32_t cnt = 0;
	for (VkPhysicalDevice *device = &devices[0]; cnt < deviceCount; cnt++) {
		if (isDeviceSuitable(device)) {
			Engine->physicalDevice = *device;
			break;
		}
		device = devices + 1;
	}
}

bool isDeviceSuitable(VkPhysicalDevice *device) {
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(*device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(*device, &deviceFeatures);

	//printf("\t %.256s\n", deviceProperties.deviceName);

	// Almost every operation in vulkan requires commands to be submitted to a queue.

	QueueFamilyIndices indices = findQueueFamilies(device);
	printf("[INFO] indices in isDeviceSuitable() value after completion of findQueueFamilies: %d\n", indices.graphicsFamily);

	// Luckily only my gpu supports vulkan, so I always get the same device
	// Still needs a logical device to interface with the physical one

	printf("[INFO] Device %.256s is type ", deviceProperties.deviceName);

	switch (deviceProperties.deviceType) {
		case 0:
			printf("VK_PHYSICAL_DEVICE_TYPE_OTHER ");
		break;
		case 1:
			printf("VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ");
		break;
		case 2:
			printf("VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ");
		break;
		case 3:
			printf("VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU ");
		break;
		case 4:
			printf("VK_PHYSICAL_DEVICE_TYPE_CPU ");
		break;
		default:
			printf("undefined ");
	}

	if (deviceFeatures.geometryShader)
		printf("and supports geometry shaders\n");
	else printf("and does not support geometry shaders\n");

	// Another type of return statement to consider
	// return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
	//	deviceFeatures.geometryShader;

	return deviceFeatures.geometryShader;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice *device) {
	// TO-DO: logic to find queue family indices to populate struct with
	// 	Looks very complicated
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, NULL);

	VkQueueFamilyProperties queueFamilies[queueFamilyCount];
	vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, queueFamilies);

	int i = 0;
	size_t n = sizeof(queueFamilies)/sizeof(queueFamilies[0]);
	// if only 1 queue familiy, instantly pass that
	if (n == 1) {
		indices.graphicsFamily = 0;
		return indices;
	}

	VkBool32 presentSupport = false;

	for (VkQueueFamilyProperties queueFamily; i < n; i++) {
		vkGetPhysicalDeviceSurfaceSupportKHR(*device, i, Engine->surface, &presentSupport);
		printf("[INFO] Surface present support: %d", presentSupport);
		if (presentSupport)
			indices.presentFamily = i;

		queueFamily = queueFamilies[i];
		/**
		 * VkQueueFlagBits struct looks like this:
		 * VK_QUEUE_GRAPHICS_BIT = 0x00000001
		 * VK_QUEUE_COMPUTE_BIT = 0x00000002
		 * VK_QUEUE_TRANSFER_BIT = 0x00000004
		 * VK_QUEUE_SPARSE_BINDING_BIT = 0x00000008
		 * 
		 * Performs a bitwise AND operation
		 * e.g. in my case, queueFlags = 7, in binary that is 0111
		 * first three bits present the GRAPHICS, COMPUTE and TRANSFER bit
		 * thus the calculation works like this:
		 *     111 (my side)
		 * AND 001 (VK_QUEUE_GRAPHICS_BIT)
		 *   = 001
		 * 
		 * TO-DO: clean and expand
		 */
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}
	}
	return indices;
}

void createLogicalDevice() {
	/*
	 * After selecting a physical device we need a logical device to interface with it.
	 * Describes the features we want to use, similar to instance creation, also need to specify which queues to create
	 * Can create multiple logical devices from the same physical device.
	 * Can create all of the command buffers on mulitple threads and then submit them all at once on the main
	 * thread with a single low-overhead call.
	 */ 
	QueueFamilyIndices indices = findQueueFamilies(&Engine->physicalDevice);

	VkDeviceQueueCreateInfo queueInfo;
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueFamilyIndex = indices.graphicsFamily;
	queueInfo.queueCount = 1;

	// Vulkan lets you assign priorities to queues between 0.0 and 1.0. Required even with 1 queue
	float queuePriority = 1.0f;
	queueInfo.pQueuePriorities = &queuePriority;

	// Specify the set of device features to use
	// Imma leave it all to VK_FALSE (default) for now
	VkPhysicalDeviceFeatures deviceFeatures;

	VkDeviceCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pEnabledFeatures = &deviceFeatures;

	// Specify device specific extensions and validation layers
	// enabledLayerCount and ppEnabledLayerNames fields of VkDeviceCreateInfo are ignored by up-to-date implementations
	createInfo.enabledExtensionCount = 0;
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = 1;
		createInfo.ppEnabledLayerNames = validationLayers;
	} else {
		createInfo.enabledLayerCount = 0;
	}

	// Logical devices don't interact directly with instances	
	if (vkCreateDevice(Engine->physicalDevice, &createInfo, NULL, &Engine->device) != VK_SUCCESS) {
		printf("[ERROR] Failed to create logical device");
	}
	// Retrieves handles for each queue family
	// Segfault
	printf("[DEBUG] indices.graphicsFamily: %d", indices.graphicsFamily);
	vkGetDeviceQueue(Engine->device, indices.graphicsFamily, 0, &Engine->graphicsQueue);
}

void dDisplayEngineProperties(void) {
	printf("[INFO] Engine properties: \n");

	// Display physical device properties
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(Engine->physicalDevice, &deviceProperties);
	vkGetPhysicalDeviceFeatures(Engine->physicalDevice, &deviceFeatures);

	printf("\tDevice name:  %.256s\n", deviceProperties.deviceName);

}

void createInstance() {
        VkApplicationInfo appInfo;
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Tetris";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Sheeesh";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        VkInstanceCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // Gets the number of extensions 
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
        //printf("extensionCount = %d\n", extensionCount);

        VkExtensionProperties extensions [extensionCount];
        
        // Fills in the extension details
        vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);
        
        // Prints available extensions
        printf("[INFO] Available extensions (%d):\n", extensionCount);
        for (int i = 0; i < extensionCount; i++) {
                printf("\t %.256s\n", extensions[i].extensionName);
        }
        
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

        createInfo.enabledExtensionCount = extensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;

	if (enableValidationLayers) {
		// TO-DO: use actual validationLayers array size instead of hardcorded uint32_t
		createInfo.enabledLayerCount = 1U;
		createInfo.ppEnabledLayerNames = &validationLayers[0];
	} else {
		createInfo.enabledLayerCount = 0;
	}

	printf("[INFO] Enabled layer count: %d\n", createInfo.enabledLayerCount);

        // Stores the result in the Engine struct variable instance
	// After init of vulkan lib thru VkInstance need to select gpu
        if (vkCreateInstance(&createInfo, NULL, &(Engine->instance)) != VK_SUCCESS) {
                printf("[ERROR] Failed to create instance\n");
        }
}

bool checkValidationLayerSupport(void) {
	uint32_t layerCount;
	// Lists all of the available layers
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);

	VkLayerProperties availableLayers[layerCount];
	//printf("layerCount: %zu\n", (unsigned long)layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

	// Todo: check if all of the layers in validationLayers exist in the availableLayers list.

	return false;
}

void mainLoop() {
        while (!glfwWindowShouldClose(Engine->window)) {
                glfwPollEvents();
        }
}

void cleanup() {
        vkDestroyDevice(Engine->device, NULL);
	vkDestroyInstance(Engine->instance, NULL);

        glfwDestroyWindow(Engine->window);
        
        glfwTerminate();
}