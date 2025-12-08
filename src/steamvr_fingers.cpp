#include "steamvr_fingers.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/xr_interface.hpp>

using namespace godot;

SteamVRFingers::SteamVRFingers() {
    xr_server = nullptr;
    is_initialized = false;
    xr_available = false;

    reset_finger_values();
}

SteamVRFingers::~SteamVRFingers() {
    shutdown_xr();
}

void SteamVRFingers::_bind_methods() {
    // Initialization methods
    ClassDB::bind_method(D_METHOD("initialize_xr"), &SteamVRFingers::initialize_xr);
    ClassDB::bind_method(D_METHOD("shutdown_xr"), &SteamVRFingers::shutdown_xr);
    ClassDB::bind_method(D_METHOD("is_xr_initialized"), &SteamVRFingers::is_xr_initialized);

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
    UtilityFunctions::print("[SteamVR Fingers] Initializing using Godot's native XR system...");
    initialize_xr();
}

void SteamVRFingers::_process(double delta) {
    if (is_initialized && xr_available) {
        update_finger_curls();
    }
}

void SteamVRFingers::_exit_tree() {
    shutdown_xr();
}

bool SteamVRFingers::initialize_xr() {
    if (is_initialized) {
        UtilityFunctions::print("[SteamVR Fingers] Already initialized");
        return true;
    }

    UtilityFunctions::print("[SteamVR Fingers] ========================================");
    UtilityFunctions::print("[SteamVR Fingers] Starting XR hand tracking initialization...");
    UtilityFunctions::print("[SteamVR Fingers] ========================================");

    // Get XRServer singleton
    xr_server = XRServer::get_singleton();
    if (!xr_server) {
        UtilityFunctions::push_error("[SteamVR Fingers] ERROR: XRServer singleton not available");
        return false;
    }
    UtilityFunctions::print("[SteamVR Fingers] ✓ XRServer obtained");

    // Get the primary XR interface (should be OpenXR from your logs)
    Ref<XRInterface> xr_interface = xr_server->get_primary_interface();
    if (!xr_interface.is_valid()) {
        UtilityFunctions::push_warning("[SteamVR Fingers] WARNING: No active XR interface. Make sure VR is enabled in your project.");
        return false;
    }

    String interface_name = xr_interface->get_name();
    UtilityFunctions::print(String("[SteamVR Fingers] ✓ Active XR interface: ") + interface_name);

    // Get hand trackers
    left_hand_tracker = xr_server->get_tracker("/user/hand_tracker/left");
    right_hand_tracker = xr_server->get_tracker("/user/hand_tracker/right");

    if (!left_hand_tracker.is_valid() && !right_hand_tracker.is_valid()) {
        UtilityFunctions::push_warning("[SteamVR Fingers] WARNING: No hand trackers found. Hand tracking may not be enabled.");
        UtilityFunctions::push_warning("[SteamVR Fingers] Make sure hand tracking is enabled in SteamVR settings.");
        return false;
    }

    if (left_hand_tracker.is_valid()) {
        UtilityFunctions::print("[SteamVR Fingers] ✓ Left hand tracker obtained");
    } else {
        UtilityFunctions::push_warning("[SteamVR Fingers] WARNING: Left hand tracker not available");
    }

    if (right_hand_tracker.is_valid()) {
        UtilityFunctions::print("[SteamVR Fingers] ✓ Right hand tracker obtained");
    } else {
        UtilityFunctions::push_warning("[SteamVR Fingers] WARNING: Right hand tracker not available");
    }

    is_initialized = true;
    xr_available = true;

    UtilityFunctions::print("[SteamVR Fingers] ========================================");
    UtilityFunctions::print("[SteamVR Fingers] ✓ Initialization complete! Hand tracking is active.");
    UtilityFunctions::print("[SteamVR Fingers] ========================================");

    return true;
}

void SteamVRFingers::shutdown_xr() {
    if (!is_initialized) {
        return;
    }

    UtilityFunctions::print("[SteamVR Fingers] Shutting down...");

    left_hand_tracker.unref();
    right_hand_tracker.unref();
    xr_server = nullptr;

    is_initialized = false;
    xr_available = false;
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

float SteamVRFingers::get_finger_curl_from_tracker(Ref<XRHandTracker> tracker, XRHandTracker::HandJoint finger_tip, XRHandTracker::HandJoint finger_base) {
    if (!tracker.is_valid()) {
        return 0.0f;
    }

    // Get joint flags to check if tracking is active
    BitField<XRHandTracker::HandJointFlags> tip_flags = tracker->get_hand_joint_flags(finger_tip);
    BitField<XRHandTracker::HandJointFlags> base_flags = tracker->get_hand_joint_flags(finger_base);

    // Check if joints are being tracked
    if (!tip_flags.has_flag(XRHandTracker::HAND_JOINT_FLAG_POSITION_TRACKED) ||
        !base_flags.has_flag(XRHandTracker::HAND_JOINT_FLAG_POSITION_TRACKED)) {
        return 0.0f;
    }

    // Get joint transforms
    Transform3D tip_transform = tracker->get_hand_joint_transform(finger_tip);
    Transform3D base_transform = tracker->get_hand_joint_transform(finger_base);

    // Calculate distance between tip and base
    float distance = tip_transform.origin.distance_to(base_transform.origin);

    // Normalize to 0-1 range (approximate - tune these values for Index controllers)
    // Extended finger: ~0.08m, Curled finger: ~0.02m (approximate values for Index)
    float min_distance = 0.02f;  // Fully curled
    float max_distance = 0.08f;  // Fully extended

    float curl = 1.0f - ((distance - min_distance) / (max_distance - min_distance));

    // Clamp to 0-1 range
    return Math::clamp(curl, 0.0f, 1.0f);
}

void SteamVRFingers::update_finger_curls() {
    // Update left hand
    if (left_hand_tracker.is_valid()) {
        left_thumb_curl = get_finger_curl_from_tracker(left_hand_tracker,
            XRHandTracker::HAND_JOINT_THUMB_TIP, XRHandTracker::HAND_JOINT_THUMB_METACARPAL);
        left_index_curl = get_finger_curl_from_tracker(left_hand_tracker,
            XRHandTracker::HAND_JOINT_INDEX_FINGER_TIP, XRHandTracker::HAND_JOINT_INDEX_FINGER_METACARPAL);
        left_middle_curl = get_finger_curl_from_tracker(left_hand_tracker,
            XRHandTracker::HAND_JOINT_MIDDLE_FINGER_TIP, XRHandTracker::HAND_JOINT_MIDDLE_FINGER_METACARPAL);
        left_ring_curl = get_finger_curl_from_tracker(left_hand_tracker,
            XRHandTracker::HAND_JOINT_RING_FINGER_TIP, XRHandTracker::HAND_JOINT_RING_FINGER_METACARPAL);
        left_pinky_curl = get_finger_curl_from_tracker(left_hand_tracker,
            XRHandTracker::HAND_JOINT_PINKY_FINGER_TIP, XRHandTracker::HAND_JOINT_PINKY_FINGER_METACARPAL);
    }

    // Update right hand
    if (right_hand_tracker.is_valid()) {
        right_thumb_curl = get_finger_curl_from_tracker(right_hand_tracker,
            XRHandTracker::HAND_JOINT_THUMB_TIP, XRHandTracker::HAND_JOINT_THUMB_METACARPAL);
        right_index_curl = get_finger_curl_from_tracker(right_hand_tracker,
            XRHandTracker::HAND_JOINT_INDEX_FINGER_TIP, XRHandTracker::HAND_JOINT_INDEX_FINGER_METACARPAL);
        right_middle_curl = get_finger_curl_from_tracker(right_hand_tracker,
            XRHandTracker::HAND_JOINT_MIDDLE_FINGER_TIP, XRHandTracker::HAND_JOINT_MIDDLE_FINGER_METACARPAL);
        right_ring_curl = get_finger_curl_from_tracker(right_hand_tracker,
            XRHandTracker::HAND_JOINT_RING_FINGER_TIP, XRHandTracker::HAND_JOINT_RING_FINGER_METACARPAL);
        right_pinky_curl = get_finger_curl_from_tracker(right_hand_tracker,
            XRHandTracker::HAND_JOINT_PINKY_FINGER_TIP, XRHandTracker::HAND_JOINT_PINKY_FINGER_METACARPAL);
    }
}
