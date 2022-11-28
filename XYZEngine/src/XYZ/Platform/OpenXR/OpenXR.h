#pragma once
#include "XYZ/Core/Logger.h"
#include "XYZ/Core/Assert.h"

#include <openxr/openxr.h>

namespace XYZ {
	namespace Utils {

		inline const char* XrResultToString(XrResult result)
		{
			switch (result)
			{
			case XR_SUCCESS:
				break;
			case XR_TIMEOUT_EXPIRED:
				break;
			case XR_SESSION_LOSS_PENDING:
				break;
			case XR_EVENT_UNAVAILABLE:
				break;
			case XR_SPACE_BOUNDS_UNAVAILABLE:
				break;
			case XR_SESSION_NOT_FOCUSED:
				break;
			case XR_FRAME_DISCARDED:
				break;
			case XR_ERROR_VALIDATION_FAILURE:
				break;
			case XR_ERROR_RUNTIME_FAILURE:
				break;
			case XR_ERROR_OUT_OF_MEMORY:
				break;
			case XR_ERROR_API_VERSION_UNSUPPORTED:
				break;
			case XR_ERROR_INITIALIZATION_FAILED:
				break;
			case XR_ERROR_FUNCTION_UNSUPPORTED:
				break;
			case XR_ERROR_FEATURE_UNSUPPORTED:
				break;
			case XR_ERROR_EXTENSION_NOT_PRESENT:
				break;
			case XR_ERROR_LIMIT_REACHED:
				break;
			case XR_ERROR_SIZE_INSUFFICIENT:
				break;
			case XR_ERROR_HANDLE_INVALID:
				break;
			case XR_ERROR_INSTANCE_LOST:
				break;
			case XR_ERROR_SESSION_RUNNING:
				break;
			case XR_ERROR_SESSION_NOT_RUNNING:
				break;
			case XR_ERROR_SESSION_LOST:
				break;
			case XR_ERROR_SYSTEM_INVALID:
				break;
			case XR_ERROR_PATH_INVALID:
				break;
			case XR_ERROR_PATH_COUNT_EXCEEDED:
				break;
			case XR_ERROR_PATH_FORMAT_INVALID:
				break;
			case XR_ERROR_PATH_UNSUPPORTED:
				break;
			case XR_ERROR_LAYER_INVALID:
				break;
			case XR_ERROR_LAYER_LIMIT_EXCEEDED:
				break;
			case XR_ERROR_SWAPCHAIN_RECT_INVALID:
				break;
			case XR_ERROR_SWAPCHAIN_FORMAT_UNSUPPORTED:
				break;
			case XR_ERROR_ACTION_TYPE_MISMATCH:
				break;
			case XR_ERROR_SESSION_NOT_READY:
				break;
			case XR_ERROR_SESSION_NOT_STOPPING:
				break;
			case XR_ERROR_TIME_INVALID:
				break;
			case XR_ERROR_REFERENCE_SPACE_UNSUPPORTED:
				break;
			case XR_ERROR_FILE_ACCESS_ERROR:
				break;
			case XR_ERROR_FILE_CONTENTS_INVALID:
				break;
			case XR_ERROR_FORM_FACTOR_UNSUPPORTED:
				break;
			case XR_ERROR_FORM_FACTOR_UNAVAILABLE:
				break;
			case XR_ERROR_API_LAYER_NOT_PRESENT:
				break;
			case XR_ERROR_CALL_ORDER_INVALID:
				break;
			case XR_ERROR_GRAPHICS_DEVICE_INVALID:
				break;
			case XR_ERROR_POSE_INVALID:
				break;
			case XR_ERROR_INDEX_OUT_OF_RANGE:
				break;
			case XR_ERROR_VIEW_CONFIGURATION_TYPE_UNSUPPORTED:
				break;
			case XR_ERROR_ENVIRONMENT_BLEND_MODE_UNSUPPORTED:
				break;
			case XR_ERROR_NAME_DUPLICATED:
				break;
			case XR_ERROR_NAME_INVALID:
				break;
			case XR_ERROR_ACTIONSET_NOT_ATTACHED:
				break;
			case XR_ERROR_ACTIONSETS_ALREADY_ATTACHED:
				break;
			case XR_ERROR_LOCALIZED_NAME_DUPLICATED:
				break;
			case XR_ERROR_LOCALIZED_NAME_INVALID:
				break;
			case XR_ERROR_GRAPHICS_REQUIREMENTS_CALL_MISSING:
				break;
			case XR_ERROR_RUNTIME_UNAVAILABLE:
				break;
			case XR_ERROR_ANDROID_THREAD_SETTINGS_ID_INVALID_KHR:
				break;
			case XR_ERROR_ANDROID_THREAD_SETTINGS_FAILURE_KHR:
				break;
			case XR_ERROR_CREATE_SPATIAL_ANCHOR_FAILED_MSFT:
				break;
			case XR_ERROR_SECONDARY_VIEW_CONFIGURATION_TYPE_NOT_ENABLED_MSFT:
				break;
			case XR_ERROR_CONTROLLER_MODEL_KEY_INVALID_MSFT:
				break;
			case XR_ERROR_REPROJECTION_MODE_UNSUPPORTED_MSFT:
				break;
			case XR_ERROR_COMPUTE_NEW_SCENE_NOT_COMPLETED_MSFT:
				break;
			case XR_ERROR_SCENE_COMPONENT_ID_INVALID_MSFT:
				break;
			case XR_ERROR_SCENE_COMPONENT_TYPE_MISMATCH_MSFT:
				break;
			case XR_ERROR_SCENE_MESH_BUFFER_ID_INVALID_MSFT:
				break;
			case XR_ERROR_SCENE_COMPUTE_FEATURE_INCOMPATIBLE_MSFT:
				break;
			case XR_ERROR_SCENE_COMPUTE_CONSISTENCY_MISMATCH_MSFT:
				break;
			case XR_ERROR_DISPLAY_REFRESH_RATE_UNSUPPORTED_FB:
				break;
			case XR_ERROR_COLOR_SPACE_UNSUPPORTED_FB:
				break;
			case XR_ERROR_SPACE_COMPONENT_NOT_SUPPORTED_FB:
				break;
			case XR_ERROR_SPACE_COMPONENT_NOT_ENABLED_FB:
				break;
			case XR_ERROR_SPACE_COMPONENT_STATUS_PENDING_FB:
				break;
			case XR_ERROR_SPACE_COMPONENT_STATUS_ALREADY_SET_FB:
				break;
			case XR_ERROR_UNEXPECTED_STATE_PASSTHROUGH_FB:
				break;
			case XR_ERROR_FEATURE_ALREADY_CREATED_PASSTHROUGH_FB:
				break;
			case XR_ERROR_FEATURE_REQUIRED_PASSTHROUGH_FB:
				break;
			case XR_ERROR_NOT_PERMITTED_PASSTHROUGH_FB:
				break;
			case XR_ERROR_INSUFFICIENT_RESOURCES_PASSTHROUGH_FB:
				break;
			case XR_ERROR_UNKNOWN_PASSTHROUGH_FB:
				break;
			case XR_ERROR_RENDER_MODEL_KEY_INVALID_FB:
				break;
			case XR_RENDER_MODEL_UNAVAILABLE_FB:
				break;
			case XR_ERROR_MARKER_NOT_TRACKED_VARJO:
				break;
			case XR_ERROR_MARKER_ID_INVALID_VARJO:
				break;
			case XR_ERROR_SPATIAL_ANCHOR_NAME_NOT_FOUND_MSFT:
				break;
			case XR_ERROR_SPATIAL_ANCHOR_NAME_INVALID_MSFT:
				break;
			case XR_RESULT_MAX_ENUM:
				break;
			default:
				break;
			}
			return "";
		}

		inline void OpenXRCheckResult(XrResult result)
		{
			if (result != XR_SUCCESS)
			{
				XYZ_CORE_ERROR("VkResult is '{0}' in {1}:{2}", XrResultToString(result), __FILE__, __LINE__);
				XYZ_ASSERT(result == XR_SUCCESS, "");
			}
		}
	}
}

#define XR_CHECK_RESULT(f)\
{\
	XrResult res = (f);\
	::XYZ::Utils::OpenXRCheckResult(res);\
}