set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/vendor)

if (NOT DEFINED $ENV{TARGET_OS})
	string(TOLOWER "${CMAKE_SYSTEM_NAME}" TARGET)
	set(ENV{TARGET} ${TARGET})
else()
	string(TOLOWER "${TARGET_OS}" TARGET)
	set(ENV{TARGET} ${TARGET_OS})
endif()

if (NOT DEFINED $ENV{VULKAN_SDK})
	message(STATUS "VULKAN_SDK env variable not found - downloading/setting up the SDK")

	execute_process(
		COMMAND python ${CMAKE_SOURCE_DIR}/scripts/setup_vulkan.py
		RESULT_VARIABLE SETUP_VULKAN_SDK_RESULT
		ERROR_VARIABLE SETUP_VULKAN_SDK_ERROR
		# OUTPUT_VARIABLE SETUP_VULKAN_SDK_OUTPUT
	)

	if (${SETUP_VULKAN_SDK_RESULT} EQUAL 0)
		set(VULKAN_SDK ${VENDOR_DIR}/${CMAKE_SYSTEM_NAME}/vulkan_sdk/$ENV{VULKAN_SDK_VERSION}/x86_64)
		string(TOLOWER "${VULKAN_SDK}" VULKAN_SDK)
		message(STATUS "Setting VULKAN_SDK env variable to ${VULKAN_SDK}")
	else()
		message(FATAL ${SETUP_VULKAN_SDK_ERROR})
	endif()
endif()
