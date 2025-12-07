extends Control

var finger_tracker: SteamVRFingers

@onready var status_label = $VBoxContainer/StatusLabel
@onready var left_hand_container = $VBoxContainer/HBoxContainer/LeftHandPanel/LeftHandVBox
@onready var right_hand_container = $VBoxContainer/HBoxContainer/RightHandPanel/RightHandVBox

# Left hand labels
@onready var left_thumb_label = $VBoxContainer/HBoxContainer/LeftHandPanel/LeftHandVBox/LeftThumb
@onready var left_index_label = $VBoxContainer/HBoxContainer/LeftHandPanel/LeftHandVBox/LeftIndex
@onready var left_middle_label = $VBoxContainer/HBoxContainer/LeftHandPanel/LeftHandVBox/LeftMiddle
@onready var left_ring_label = $VBoxContainer/HBoxContainer/LeftHandPanel/LeftHandVBox/LeftRing
@onready var left_pinky_label = $VBoxContainer/HBoxContainer/LeftHandPanel/LeftHandVBox/LeftPinky

# Right hand labels
@onready var right_thumb_label = $VBoxContainer/HBoxContainer/RightHandPanel/RightHandVBox/RightThumb
@onready var right_index_label = $VBoxContainer/HBoxContainer/RightHandPanel/RightHandVBox/RightIndex
@onready var right_middle_label = $VBoxContainer/HBoxContainer/RightHandPanel/RightHandVBox/RightMiddle
@onready var right_ring_label = $VBoxContainer/HBoxContainer/RightHandPanel/RightHandVBox/RightRing
@onready var right_pinky_label = $VBoxContainer/HBoxContainer/RightHandPanel/RightHandVBox/RightPinky

func _ready():
	print("=== SteamVR Fingers Demo Starting ===")

	# Create and add the finger tracker
	finger_tracker = SteamVRFingers.new()
	add_child(finger_tracker)

	# Wait a frame for initialization
	await get_tree().process_frame

	# Update status
	if finger_tracker.is_steamvr_initialized():
		status_label.text = "✓ SteamVR Initialized - Tracking Active"
		status_label.add_theme_color_override("font_color", Color.GREEN)
	else:
		status_label.text = "✗ SteamVR Not Available - Showing Zero Values"
		status_label.add_theme_color_override("font_color", Color.ORANGE)

func _process(delta):
	if finger_tracker:
		# Update left hand
		update_finger_label(left_thumb_label, "Left Thumb:", finger_tracker.get_left_thumb_curl())
		update_finger_label(left_index_label, "Left Index:", finger_tracker.get_left_index_curl())
		update_finger_label(left_middle_label, "Left Middle:", finger_tracker.get_left_middle_curl())
		update_finger_label(left_ring_label, "Left Ring:", finger_tracker.get_left_ring_curl())
		update_finger_label(left_pinky_label, "Left Pinky:", finger_tracker.get_left_pinky_curl())

		# Update right hand
		update_finger_label(right_thumb_label, "Right Thumb:", finger_tracker.get_right_thumb_curl())
		update_finger_label(right_index_label, "Right Index:", finger_tracker.get_right_index_curl())
		update_finger_label(right_middle_label, "Right Middle:", finger_tracker.get_right_middle_curl())
		update_finger_label(right_ring_label, "Right Ring:", finger_tracker.get_right_ring_curl())
		update_finger_label(right_pinky_label, "Right Pinky:", finger_tracker.get_right_pinky_curl())

func update_finger_label(label: Label, name: String, value: float):
	label.text = "%s %.2f" % [name, value]

	# Color code based on curl value
	if value < 0.3:
		label.add_theme_color_override("font_color", Color.GREEN)
	elif value < 0.7:
		label.add_theme_color_override("font_color", Color.YELLOW)
	else:
		label.add_theme_color_override("font_color", Color.RED)
