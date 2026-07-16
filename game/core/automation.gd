class_name Automation
extends RefCounted
## Shared flags for smoke / presentation E2E / screenshot capture.

static var enabled: bool = false
static var smoke_test: bool = false
static var e2e_test: bool = false
static var capture_screenshots: bool = false
static var screenshot_dir: String = "user://screenshots"
static var instant_animations: bool = false
static var deterministic_seed: int = 424242
static var last_error: String = ""


static func parse_cmdline() -> void:
	var args := OS.get_cmdline_user_args()
	args.append_array(OS.get_cmdline_args())
	for arg in args:
		match str(arg):
			"--smoke-test":
				enabled = true
				smoke_test = true
				instant_animations = true
				capture_screenshots = true
			"--e2e-test":
				enabled = true
				e2e_test = true
				instant_animations = true
			"--capture-screenshots":
				capture_screenshots = true
			"--instant-anims":
				instant_animations = true
	if OS.get_environment("BRASSDEEP_SMOKE") == "1":
		enabled = true
		smoke_test = true
		instant_animations = true
		capture_screenshots = true


static func fail(message: String) -> void:
	last_error = message
	push_error("AUTOMATION FAIL: " + message)


static func ensure_screenshot_dir() -> void:
	DirAccess.make_dir_recursive_absolute(screenshot_dir)
