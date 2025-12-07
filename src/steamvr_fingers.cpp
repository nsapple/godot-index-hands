#include "steamvr_fingers.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/file_access.hpp>

using namespace godot;

SteamVRFingers::SteamVRFingers() {
    vr_system = nullptr;
    vr_input = nullptr;
    is_initialized = false;
    steamvr_available = false;

    action_set_handle = vr::k_ulInvalidActionSetHandle;
    left_hand_skeleton_action = vr::k_ulInvalidActionHandle;
    right_hand_skeleton_action = vr::k_ulInvalidActionHandle;

    reset_finger_values();
}

SteamVRFingers::~SteamVRFingers() {
    shutdown_steamvr();
}

void SteamVRFingers::_bind_methods() {
    // Initialization methods
    ClassDB::bind_method(D_METHOD("initialize_steamvr"), &SteamVRFingers::initialize_steamvr);
    ClassDB::bind_method(D_METHOD("shutdown_steamvr"), &SteamVRFingers::shutdown_steamvr);
    ClassDB::bind_method(D_METHOD("is_steamvr_initialized"), &SteamVRFingers::is_steamvr_initialized);

    // Left hand getters
    ClassDB::bind_method(D_METHOD("get_left_thumb_curl"), &SteamVRFingers::get_left_thumb_curl);
    ClassDB::bind_method(D_METHOD("get_left_index_curl"), &SteamVRFingers::get_left_index_curl);
    ClassDB::bind_method(D_METHOD("get_left_middle_curl"), &SteamVRFingers::get_left_middle_curl);
    ClassDB::bind_method(D_METHOD("get_left_ring_curl"), &SteamVRFingers::get_left_ring_curl);
    ClassDB::bind_method(D_METHOD("get_left_pinky_curl"), &SteamVRFingers::get_left_pinky_curl);

    // Right hand getters
    ClassDB::bind_method(D_METHOD("get_right_thumb_curl"), &SteamVRFingers::get_right_thumb_curl);
    ClassDB::bind_method(D_METHOD("get_right_index_curl"), &SteamVRFingers::get_right_index_curl);
    ClassDB::bind_method(D_METHOD("get_right_middle_curl"), &SteamVRFingers::get_right_middle_curl);
    ClassDB::bind_method(D_METHOD("get_right_ring_curl"), &SteamVRFingers::get_right_ring_curl);
    ClassDB::bind_method(D_METHOD("get_right_pinky_curl"), &SteamVRFingers::get_right_pinky_curl);
}

void SteamVRFingers::_ready() {
    UtilityFunctions::print("[SteamVR Fingers] Initializing...");
    initialize_steamvr();
}

void SteamVRFingers::_process(double delta) {
    if (is_initialized && steamvr_available) {
        update_finger_curls();
    }
}

void SteamVRFingers::_exit_tree() {
    shutdown_steamvr();
}

bool SteamVRFingers::initialize_steamvr() {
    if (is_initialized) {
        UtilityFunctions::print("[SteamVR Fingers] Already initialized");
        return true;
    }

    UtilityFunctions::print("[SteamVR Fingers] ========================================");
    UtilityFunctions::print("[SteamVR Fingers] Starting SteamVR initialization...");
    UtilityFunctions::print("[SteamVR Fingers] ========================================");

    // Check if VR runtime is installed
    UtilityFunctions::print("[SteamVR Fingers] Checking if VR runtime is installed...");
    if (!vr::VR_IsRuntimeInstalled()) {
        UtilityFunctions::push_error("[SteamVR Fingers] ERROR: SteamVR runtime not installed!");
        reset_finger_values();
        return false;
    }
    UtilityFunctions::print("[SteamVR Fingers] ✓ VR runtime is installed");

    // Check if HMD is present
    UtilityFunctions::print("[SteamVR Fingers] Checking if HMD is present...");
    if (!vr::VR_IsHmdPresent()) {
        UtilityFunctions::push_warning("[SteamVR Fingers] WARNING: No HMD detected. Finger tracking will return 0.0 values.");
        reset_finger_values();
        return false;
    }
    UtilityFunctions::print("[SteamVR Fingers] ✓ HMD is present");

    // Initialize OpenVR
    // Use VRApplication_Scene instead of Background to get full skeletal input access
    UtilityFunctions::print("[SteamVR Fingers] Initializing OpenVR (VRApplication_Scene)...");
    vr::EVRInitError init_error = vr::VRInitError_None;
    vr_system = vr::VR_Init(&init_error, vr::VRApplication_Scene);

    if (init_error != vr::VRInitError_None) {
        UtilityFunctions::push_error(String("[SteamVR Fingers] ERROR: VR_Init failed with error code: ") + String::num_int64(init_error));
        UtilityFunctions::push_error(String("[SteamVR Fingers] ERROR: ") +
                                     String(vr::VR_GetVRInitErrorAsEnglishDescription(init_error)));
        vr_system = nullptr;
        reset_finger_values();
        return false;
    }
    UtilityFunctions::print("[SteamVR Fingers] ✓ OpenVR initialized successfully");

    // Get input interface
    UtilityFunctions::print("[SteamVR Fingers] Getting VRInput interface...");
    vr_input = vr::VRInput();
    if (!vr_input) {
        UtilityFunctions::push_error("[SteamVR Fingers] ERROR: Failed to get VRInput interface (returned nullptr)");
        shutdown_steamvr();
        return false;
    }
    UtilityFunctions::print("[SteamVR Fingers] ✓ VRInput interface obtained");

    // Get action manifest path
    String manifest_path = get_action_manifest_path();
    UtilityFunctions::print(String("[SteamVR Fingers] Action manifest path: ") + manifest_path);
    UtilityFunctions::print(String("[SteamVR Fingers] Calling SetActionManifestPath..."));

    // Set action manifest
    vr::EVRInputError input_error = vr_input->SetActionManifestPath(manifest_path.utf8().get_data());
    UtilityFunctions::print(String("[SteamVR Fingers] SetActionManifestPath returned error code: ") + String::num_int64(input_error));

    if (input_error != vr::VRInputError_None) {
        UtilityFunctions::push_error(String("[SteamVR Fingers] ERROR: Failed to set action manifest (error ") +
                                     String::num_int64(input_error) + String(")"));
        UtilityFunctions::push_error(String("[SteamVR Fingers] Make sure the action manifest exists at: ") + manifest_path);
        shutdown_steamvr();
        return false;
    }
    UtilityFunctions::print("[SteamVR Fingers] ✓ Action manifest set successfully");

    // Get action set handle
    UtilityFunctions::print("[SteamVR Fingers] Getting action set handle for '/actions/finger_tracking'...");
    input_error = vr_input->GetActionSetHandle("/actions/finger_tracking", &action_set_handle);
    UtilityFunctions::print(String("[SteamVR Fingers] GetActionSetHandle returned error code: ") + String::num_int64(input_error));
    UtilityFunctions::print(String("[SteamVR Fingers] Action set handle value: ") + String::num_uint64(action_set_handle));

    if (input_error != vr::VRInputError_None) {
        UtilityFunctions::push_error("[SteamVR Fingers] ERROR: Failed to get action set handle");
        shutdown_steamvr();
        return false;
    }
    UtilityFunctions::print("[SteamVR Fingers] ✓ Action set handle obtained");

    // Get skeleton action handles
    UtilityFunctions::print("[SteamVR Fingers] Getting left hand skeleton action handle...");
    input_error = vr_input->GetActionHandle("/actions/finger_tracking/in/left_hand_skeleton", &left_hand_skeleton_action);
    UtilityFunctions::print(String("[SteamVR Fingers] GetActionHandle (left) returned error code: ") + String::num_int64(input_error));
    UtilityFunctions::print(String("[SteamVR Fingers] Left hand action handle value: ") + String::num_uint64(left_hand_skeleton_action));

    if (input_error != vr::VRInputError_None) {
        UtilityFunctions::push_error("[SteamVR Fingers] ERROR: Failed to get left hand skeleton action handle");
        shutdown_steamvr();
        return false;
    }
    UtilityFunctions::print("[SteamVR Fingers] ✓ Left hand action handle obtained");

    UtilityFunctions::print("[SteamVR Fingers] Getting right hand skeleton action handle...");
    input_error = vr_input->GetActionHandle("/actions/finger_tracking/in/right_hand_skeleton", &right_hand_skeleton_action);
    UtilityFunctions::print(String("[SteamVR Fingers] GetActionHandle (right) returned error code: ") + String::num_int64(input_error));
    UtilityFunctions::print(String("[SteamVR Fingers] Right hand action handle value: ") + String::num_uint64(right_hand_skeleton_action));

    if (input_error != vr::VRInputError_None) {
        UtilityFunctions::push_error("[SteamVR Fingers] ERROR: Failed to get right hand skeleton action handle");
        shutdown_steamvr();
        return false;
    }
    UtilityFunctions::print("[SteamVR Fingers] ✓ Right hand action handle obtained");

    is_initialized = true;
    steamvr_available = true;

    UtilityFunctions::print("[SteamVR Fingers] ========================================");
    UtilityFunctions::print("[SteamVR Fingers] ✓ Initialization complete! Finger tracking is active.");
    UtilityFunctions::print("[SteamVR Fingers] ========================================");

    return true;
}

void SteamVRFingers::shutdown_steamvr() {
    if (!is_initialized) {
        return;
    }

    UtilityFunctions::print("[SteamVR Fingers] Shutting down...");

    if (vr_system) {
        vr::VR_Shutdown();
        vr_system = nullptr;
        vr_input = nullptr;
    }

    is_initialized = false;
    steamvr_available = false;
    reset_finger_values();

    UtilityFunctions::print("[SteamVR Fingers] Shutdown complete");
}

void SteamVRFingers::reset_finger_values() {
    left_thumb_curl = 0.0f;
    left_index_curl = 0.0f;
    left_middle_curl = 0.0f;
    left_ring_curl = 0.0f;
    left_pinky_curl = 0.0f;

    right_thumb_curl = 0.0f;
    right_index_curl = 0.0f;
    right_middle_curl = 0.0f;
    right_ring_curl = 0.0f;
    right_pinky_curl = 0.0f;
}

String SteamVRFingers::get_action_manifest_path() {
    // Use project path for the action manifest (works in both editor and exported)
    String res_path = "res://addons/steamvr_fingers/actions/action_manifest.json";
    String manifest_path = ProjectSettings::get_singleton()->globalize_path(res_path);

    // Verify the file exists
    if (!FileAccess::file_exists(manifest_path)) {
        UtilityFunctions::push_error(String("[SteamVR Fingers] Action manifest file not found: ") + manifest_path);
    }

    return manifest_path;
}

void SteamVRFingers::update_finger_curls() {
    if (!vr_input || !is_initialized) {
        return;
    }

    static int frame_count = 0;
    static bool first_update = true;
    frame_count++;

    bool should_print = first_update || (frame_count % 60 == 0);

    if (first_update) {
        UtilityFunctions::print("[SteamVR Fingers] ======== FIRST UPDATE CYCLE ========");
    }

    // Update action state
    vr::VRActiveActionSet_t active_action_set = {};
    active_action_set.ulActionSet = action_set_handle;

    if (should_print) {
        UtilityFunctions::print(String("[SteamVR Fingers] Calling UpdateActionState with action set handle: ") + String::num_uint64(action_set_handle));
    }

    vr::EVRInputError error = vr_input->UpdateActionState(&active_action_set, sizeof(vr::VRActiveActionSet_t), 1);

    if (should_print) {
        UtilityFunctions::print(String("[SteamVR Fingers] UpdateActionState returned error code: ") + String::num_int64(error));
    }

    if (error != vr::VRInputError_None) {
        static bool update_error_printed = false;
        if (!update_error_printed) {
            UtilityFunctions::push_warning(String("[SteamVR Fingers] UpdateActionState error: ") + String::num_int64(error));
            update_error_printed = true;
        }
        return;
    }

    if (should_print) {
        UtilityFunctions::print("[SteamVR Fingers] UpdateActionState succeeded, updating finger data...");
    }

    // Update left hand
    if (should_print) {
        UtilityFunctions::print("[SteamVR Fingers] --- Processing LEFT HAND ---");
    }
    calculate_finger_curl_from_skeleton(left_hand_skeleton_action,
                                        left_thumb_curl, left_index_curl, left_middle_curl,
                                        left_ring_curl, left_pinky_curl, should_print);

    // Update right hand
    if (should_print) {
        UtilityFunctions::print("[SteamVR Fingers] --- Processing RIGHT HAND ---");
    }
    calculate_finger_curl_from_skeleton(right_hand_skeleton_action,
                                        right_thumb_curl, right_index_curl, right_middle_curl,
                                        right_ring_curl, right_pinky_curl, should_print);

    if (should_print) {
        UtilityFunctions::print(String("[SteamVR Fingers] Frame ") + String::num_int64(frame_count) + String(" - Final values:"));
        UtilityFunctions::print(String("[SteamVR Fingers]   LEFT:  Thumb=") + String::num(left_thumb_curl, 2) +
                               String(" Index=") + String::num(left_index_curl, 2) +
                               String(" Middle=") + String::num(left_middle_curl, 2) +
                               String(" Ring=") + String::num(left_ring_curl, 2) +
                               String(" Pinky=") + String::num(left_pinky_curl, 2));
        UtilityFunctions::print(String("[SteamVR Fingers]   RIGHT: Thumb=") + String::num(right_thumb_curl, 2) +
                               String(" Index=") + String::num(right_index_curl, 2) +
                               String(" Middle=") + String::num(right_middle_curl, 2) +
                               String(" Ring=") + String::num(right_ring_curl, 2) +
                               String(" Pinky=") + String::num(right_pinky_curl, 2));
    }

    first_update = false;
}

void SteamVRFingers::calculate_finger_curl_from_skeleton(vr::VRActionHandle_t action_handle,
                                                          float& thumb, float& index, float& middle,
                                                          float& ring, float& pinky, bool print_debug) {
    if (!vr_input) {
        if (print_debug) {
            UtilityFunctions::push_warning("[SteamVR Fingers] vr_input is nullptr!");
        }
        return;
    }

    if (print_debug) {
        UtilityFunctions::print(String("[SteamVR Fingers] Processing action handle: ") + String::num_uint64(action_handle));
    }

    // Get skeletal data
    vr::InputSkeletalActionData_t skeletal_data = {};
    vr::EVRInputError error = vr_input->GetSkeletalActionData(
        action_handle,
        &skeletal_data,
        sizeof(skeletal_data)
    );

    if (print_debug) {
        UtilityFunctions::print(String("[SteamVR Fingers] GetSkeletalActionData returned error code: ") + String::num_int64(error));
        UtilityFunctions::print(String("[SteamVR Fingers] Skeletal data bActive: ") + (skeletal_data.bActive ? "TRUE" : "FALSE"));
    }

    if (error != vr::VRInputError_None) {
        static bool error_printed = false;
        if (!error_printed || print_debug) {
            UtilityFunctions::push_warning(String("[SteamVR Fingers] GetSkeletalActionData error: ") + String::num_int64(error));
            error_printed = true;
        }
        return;
    }

    if (!skeletal_data.bActive) {
        static bool inactive_printed = false;
        if (!inactive_printed || print_debug) {
            UtilityFunctions::push_warning("[SteamVR Fingers] Skeletal action is not active - controllers may not be detected");
            UtilityFunctions::push_warning("[SteamVR Fingers] Make sure Index controllers are turned on and tracked by SteamVR");
            inactive_printed = true;
        }
        return;
    }

    if (print_debug) {
        UtilityFunctions::print("[SteamVR Fingers] Skeletal action is ACTIVE! Getting summary data...");
    }

    // Get finger curl data (using curl summary data for simplicity)
    vr::VRSkeletalSummaryData_t summary_data = {};
    error = vr_input->GetSkeletalSummaryData(
        action_handle,
        vr::VRSummaryType_FromAnimation,
        &summary_data
    );

    if (print_debug) {
        UtilityFunctions::print(String("[SteamVR Fingers] GetSkeletalSummaryData returned error code: ") + String::num_int64(error));
    }

    if (error != vr::VRInputError_None) {
        static bool summary_error_printed = false;
        if (!summary_error_printed || print_debug) {
            UtilityFunctions::push_warning(String("[SteamVR Fingers] GetSkeletalSummaryData error: ") + String::num_int64(error));
            summary_error_printed = true;
        }
        return;
    }

    if (print_debug) {
        UtilityFunctions::print("[SteamVR Fingers] Raw finger curl values from OpenVR:");
        UtilityFunctions::print(String("[SteamVR Fingers]   Thumb:  ") + String::num(summary_data.flFingerCurl[vr::VRFinger_Thumb], 4));
        UtilityFunctions::print(String("[SteamVR Fingers]   Index:  ") + String::num(summary_data.flFingerCurl[vr::VRFinger_Index], 4));
        UtilityFunctions::print(String("[SteamVR Fingers]   Middle: ") + String::num(summary_data.flFingerCurl[vr::VRFinger_Middle], 4));
        UtilityFunctions::print(String("[SteamVR Fingers]   Ring:   ") + String::num(summary_data.flFingerCurl[vr::VRFinger_Ring], 4));
        UtilityFunctions::print(String("[SteamVR Fingers]   Pinky:  ") + String::num(summary_data.flFingerCurl[vr::VRFinger_Pinky], 4));
    }

    // Map finger curl values (already normalized 0.0 to 1.0)
    thumb = summary_data.flFingerCurl[vr::VRFinger_Thumb];
    index = summary_data.flFingerCurl[vr::VRFinger_Index];
    middle = summary_data.flFingerCurl[vr::VRFinger_Middle];
    ring = summary_data.flFingerCurl[vr::VRFinger_Ring];
    pinky = summary_data.flFingerCurl[vr::VRFinger_Pinky];

    // Clamp values to ensure they're in valid range
    thumb = CLAMP(thumb, 0.0f, 1.0f);
    index = CLAMP(index, 0.0f, 1.0f);
    middle = CLAMP(middle, 0.0f, 1.0f);
    ring = CLAMP(ring, 0.0f, 1.0f);
    pinky = CLAMP(pinky, 0.0f, 1.0f);

    if (print_debug) {
        UtilityFunctions::print("[SteamVR Fingers] Clamped finger curl values:");
        UtilityFunctions::print(String("[SteamVR Fingers]   Thumb:  ") + String::num(thumb, 4));
        UtilityFunctions::print(String("[SteamVR Fingers]   Index:  ") + String::num(index, 4));
        UtilityFunctions::print(String("[SteamVR Fingers]   Middle: ") + String::num(middle, 4));
        UtilityFunctions::print(String("[SteamVR Fingers]   Ring:   ") + String::num(ring, 4));
        UtilityFunctions::print(String("[SteamVR Fingers]   Pinky:  ") + String::num(pinky, 4));
    }
}
