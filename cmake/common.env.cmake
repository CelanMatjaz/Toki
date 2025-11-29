string(TOUPPER "${CMAKE_SYSTEM_NAME}" TARGET)
set(ENV{TARGET} ${TARGET})

if (NOT DEFINED $ENV{VULKAN_SDK})
	message(STATUS "VULKAN_SDK env variable not found - downloading/setting up the SDK")

	execute_process(
		COMMAND python ${CMAKE_SOURCE_DIR}/scripts/setup_vulkan.py
		RESULT_VARIABLE SETUP_VULKAN_SDK_RESULT
		ERROR_VARIABLE SETUP_VULKAN_SDK_ERROR
		OUTPUT_VARIABLE SETUP_VULKAN_SDK_OUTPUT
	)

	if (${SETUP_VULKAN_SDK_RESULT} EQUAL 0)
		set(VULKAN_SDK ${SETUP_VULKAN_SDK_OUTPUT})
		message(STATUS "Setting VULKAN_SDK env variable to ${VULKAN_SDK}")
	else()
		message(FATAL ${SETUP_VULKAN_SDK_ERROR})
	endif()
endif()
