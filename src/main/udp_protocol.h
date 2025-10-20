/*
 * UDP Protocol Definition for Spacecraft Attitude Control
 *
 * This file defines the communication protocol between external controllers
 * (e.g., Simulink, joystick interfaces) and the GUI application.
 *
 * Include this file in both sender and receiver implementations to ensure
 * protocol compatibility.
 */

#ifndef UDP_PROTOCOL_H
#define UDP_PROTOCOL_H

#include <cstdint>

// Protocol version for compatibility checking
#define UDP_PROTOCOL_VERSION 1

// Network configuration
#define UDP_DEFAULT_PORT 8888

// Input value constraints
#define JOYSTICK_INPUT_MIN -100.0f
#define JOYSTICK_INPUT_MAX  100.0f
#define JOYSTICK_INPUT_TOLERANCE 150.0f  // Max acceptable value (with tolerance)

/*
 * JoystickInputPacket
 *
 * Packet structure for sending joystick/controller inputs to the GUI.
 * All input values should be in the range [JOYSTICK_INPUT_MIN, JOYSTICK_INPUT_MAX].
 *
 * Fields:
 *   - rollInput:  Roll axis command (-100 to +100)
 *   - pitchInput: Pitch axis command (-100 to +100)
 *   - yawInput:   Yaw axis command (-100 to +100)
 *   - timestamp:  Packet timestamp or sequence number
 *
 * Total size: 16 bytes (4 floats × 4 bytes each)
 */
struct JoystickInputPacket {
    float rollInput;    // Roll axis command
    float pitchInput;   // Pitch axis command
    float yawInput;     // Yaw axis command
    uint32_t timestamp; // Timestamp or sequence number
} __attribute__((packed));

// Verify packet size at compile time
static_assert(sizeof(JoystickInputPacket) == 16,
              "JoystickInputPacket must be exactly 16 bytes");

#endif // UDP_PROTOCOL_H
