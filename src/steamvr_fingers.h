#ifndef STEAMVR_FINGERS_H
#define STEAMVR_FINGERS_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <openvr.h>

namespace godot {

class SteamVRFingers : public Node {
    GDCLASS(SteamVRFingers, Node)

private:
    // OpenVR system pointers
    vr::IVRSystem* vr_system;
    vr::IVRInput* vr_input;
    vr::IVRCompositor* vr_compositor;

    // Initialization state
    bool is_initialized;
    bool steamvr_available;

    // Action handles
    vr::VRActionSetHandle_t action_set_handle;
    vr::VRActionHandle_t left_hand_skeleton_action;
    vr::VRActionHandle_t right_hand_skeleton_action;

    // Finger curl values (0.0 = open, 1.0 = closed)
    float left_thumb_curl;
    float left_index_curl;
    float left_middle_curl;
    float left_ring_curl;
    float left_pinky_curl;

    float right_thumb_curl;
    float right_index_curl;
    float right_middle_curl;
    float right_ring_curl;
    float right_pinky_curl;

    // Helper methods
    void update_finger_curls();
    void calculate_finger_curl_from_skeleton(vr::VRActionHandle_t action_handle,
                                              float& thumb, float& index, float& middle,
                                              float& ring, float& pinky, bool print_debug = false);
    void reset_finger_values();
    String get_action_manifest_path();

protected:
    static void _bind_methods();

public:
    SteamVRFingers();
    ~SteamVRFingers();

    // Godot lifecycle methods
    void _ready() override;
    void _process(double delta) override;
    void _exit_tree() override;

    // Initialization methods
    bool initialize_steamvr();
    void shutdown_steamvr();
    bool is_steamvr_initialized() const { return is_initialized; }

    // Left hand getters
    float get_left_thumb_curl() const { return left_thumb_curl; }
    float get_left_index_curl() const { return left_index_curl; }
    float get_left_middle_curl() const { return left_middle_curl; }
    float get_left_ring_curl() const { return left_ring_curl; }
    float get_left_pinky_curl() const { return left_pinky_curl; }

    // Right hand getters
    float get_right_thumb_curl() const { return right_thumb_curl; }
    float get_right_index_curl() const { return right_index_curl; }
    float get_right_middle_curl() const { return right_middle_curl; }
    float get_right_ring_curl() const { return right_ring_curl; }
    float get_right_pinky_curl() const { return right_pinky_curl; }
};

}

#endif // STEAMVR_FINGERS_H
