#pragma once 

#include <cstdint>
#include <cstddef>

/*
    Constants: constexpr uint8_t PROTOCOL_MAGIC - The magic byte that identifies the start of a valid packet.
               constexpr uint8_t PROTOCOL_MAGIC_ACK - The magic byte used for acknowledgment packets.
               constexpr size_t  PROTOCOL_MAX_PAYLOAD_SIZE - The maximum size of the payload in bytes.
               constexpr size_t  PROTOCOL_HEADER_SIZE - The size of the packet header in bytes.
               constexpr size_t  PROTOCOL_CRC_SIZE - The size of the CRC checksum in bytes.
               constexpr size_t  PROTOCOL_MAX_PACKET_SIZE - The maximum total size of a packet (header + payload + CRC).
*/
constexpr uint8_t PROTOCOL_MAGIC            = 0xAA;
constexpr uint8_t PROTOCOL_MAGIC_ACK        = 0x55;
constexpr size_t  PROTOCOL_MAX_PAYLOAD_SIZE = 128;
constexpr size_t  PROTOCOL_HEADER_SIZE      = 4; 
constexpr size_t  PROTOCOL_CRC_SIZE         = 2;
constexpr size_t  PROTOCOL_MAX_PACKET_SIZE  = PROTOCOL_HEADER_SIZE + PROTOCOL_MAX_PAYLOAD_SIZE + PROTOCOL_CRC_SIZE;

/*
    enum class CmdType - Defines the various command types that can be sent to the CNC controller, including G-code commands, tool configuration, status queries, and control commands.
    Attributes: GCODE - Represents a G-code command.
                TOOL_CONFIG - Represents a tool configuration command.
                QUERY_POS - Represents a position query command.
                QUERY_STATUS - Represents a status query command.
                EMERGENCY_STOP - Represents an emergency stop command.
                HOME_AXIS - Represents a command to home the machine axes.
                RESET_CTRL - Represents a command to reset the controller.
                ACK - Represents an acknowledgment response.
                NACK - Represents a negative acknowledgment response.
                TELEMETRY - Represents a telemetry data packet.
                ERROR_REPORT - Represents an error report packet.
                LOG_MESSAGE - Represents a log message packet.
*/
enum class CmdType : uint8_t 
{
    GCODE          = 0x01,
    TOOL_CONFIG    = 0x02,
    QUERY_POS      = 0x03,
    QUERY_STATUS   = 0x04,
    EMERGENCY_STOP = 0x05,
    HOME_AXIS      = 0x06,
    RESET_CTRL     = 0x07,
    ACK            = 0x80,
    NACK           = 0x81,
    TELEMETRY      = 0x90,
    ERROR_REPORT   = 0x91,
    LOG_MESSAGE    = 0x92
};

/*
    enum class StatusCode - Defines the various status codes that can be returned by the CNC controller, indicating the result of a command or the current state of the machine.
    Attributes: OK - Indicates that the command was executed successfully.
                ERR_CHECKSUM - Indicates a checksum error in the received packet.
                ERR_INVALID_CMD - Indicates that an invalid command was received.
                ERR_QUEUE_FULL - Indicates that the command queue is full and cannot accept new commands.
                ERR_BUSY - Indicates that the machine is currently busy and cannot process the command.
                ERR_HARDWARE_FAILURE - Indicates a hardware failure occurred.
                ERR_TIMEOUT - Indicates that a timeout occurred while waiting for a response.
                ERR_PARAM - Indicates that there was an error with the parameters of the command.
*/
enum class StatusCode : uint8_t 
{
    OK                   = 0x00,
    ERR_CHECKSUM         = 0x01,
    ERR_INVALID_CMD      = 0x02,
    ERR_QUEUE_FULL       = 0x03,
    ERR_BUSY             = 0x04,
    ERR_HARDWARE_FAILURE = 0x05,
    ERR_TIMEOUT          = 0x06,
    ERR_PARAM            = 0x07,
};

/*
    namespace StatusFlags - Defines bit flags for various status indicators of the CNC Machine.
    Attributes: constexpr uint8_t IS_RUNNING - Indicates that the machine is currently running.
                constexpr uint8_t IS_HOMED - Indicates that the machine has been homed.
                constexpr uint8_t IS_ALARM - Indicates that the machine is in an alarm state.
                constexpr uint8_t SPINDLE_ON - Indicates that the spindle is currently on.
                constexpr uint8_t COOLANT_ON - Indicates that the coolant is currently on.
                constexpr uint8_t PROBE_TRIG - Indicates that the probe has been triggered.
*/
namespace StatusFlags 
{
    constexpr uint8_t IS_RUNNING   = (1 << 0);
    constexpr uint8_t IS_HOMED     = (1 << 1);
    constexpr uint8_t IS_ALARM     = (1 << 2);
    constexpr uint8_t SPINDLE_ON   = (1 << 3);
    constexpr uint8_t COOLANT_ON   = (1 << 4);
    constexpr uint8_t PROBE_TRIG   = (1 << 5);
};

#pragma pack(push, 1)

/*
    struct PacketHeader - Represents the header of a protocol packet, containing the magic byte, command type, payload length, and sequence ID.
    Attributes: uint8_t magic - The magic byte that identifies the start of a valid packet.
                uint8_t type - The command type of the packet, as defined in CmdType.
                uint8_t length - The length of the payload in bytes.
                uint8_t seq_id - The sequence ID of the packet, used for tracking and acknowledgment.
*/
struct PacketHeader 
{
    uint8_t magic;
    uint8_t type;
    uint8_t length;
    uint8_t seq_id;
};

/*
    struct TelemetryPayload - Represents the payload of a telemetry packet.
    Attributes: float pos_x - The X-axis position of the machine.
                float pos_y - The Y-axis position of the machine.
                float pos_z - The Z-axis position of the machine.
                uint16_t spindle_rpm - The current spindle RPM.
                uint16_t feed_rate - The current feed rate.
                uint8_t flags - Status flags indicating various machine states, as defined in StatusFlags.
                uint8_t active_tool - The ID of the currently active tool.
*/
struct TelemetryPayload 
{
    float    pos_x;
    float    pos_y;
    float    pos_z;
    uint16_t spindle_rpm;
    uint16_t feed_rate;
    uint8_t  flags;
    uint8_t  active_tool;
};

/*
    struct ToolConfigPayload - Represents the payload of a tool configuration packet.
    Attributes: uint8_t tool_id - The ID of the tool being configured.
                float length_offset - The length offset of the tool in millimeters.
                float diameter - The diameter of the tool in millimeters.
                uint16_t max_rpm - The maximum RPM for the tool.
                uint16_t max_feed_rate - The maximum feed rate for the tool in mm/min.
                uint8_t material_type - The material type of the tool, used for selecting appropriate cutting parameters.
*/
struct ToolConfigPayload 
{
    uint8_t  tool_id;
    float    length_offset;
    float    diameter;
    uint16_t max_rpm;
    uint16_t max_feed_rate; 
    uint8_t  material_type;
};

/*
    struct ProtocolFrame - Represents a complete protocol packet, including the header, payload, and CRC checksum.
    Attributes: PacketHeader header - The header of the packet, containing metadata about the packet type.
                uint8_t payload[PROTOCOL_MAX_PAYLOAD_SIZE] - The payload of the packet, containing the actual command data or telemetry information.
                uint16_t crc - The CRC checksum of the packet, used for verifying data integrity.
*/
struct ProtocolFrame 
{
    PacketHeader header;
    uint8_t      payload[PROTOCOL_MAX_PAYLOAD_SIZE];
    uint16_t     crc;
};

#pragma pack(pop)

// uint8_t protocol_calc_crc8(const uint8_t* data, size_t len) - Calculates an 8-bit CRC checksum for the given data buffer and length, used for simple error detection in acknowledgment packets.
uint16_t       protocol_calc_crc16(const uint8_t* data, size_t len);

// size_t protocol_serialize(const ProtocolFrame& frame, uint8_t* buffer, size_t max_out) - Serializes a ProtocolFrame into a buffer, returning the number of bytes written.
size_t         protocol_serialize(const ProtocolFrame& frame, uint8_t* buffer, size_t max_out);

// bool protocol_deserialize(const uint8_t* buffer, size_t len, ProtocolFrame& out_frame) - Deserializes a ProtocolFrame from a buffer, returning true if successful.
bool           protocol_deserialize(const uint8_t* buffer, size_t len, ProtocolFrame& out_frame);

// bool protocol_is_valid_header(const uint8_t* buffer, size_t len) - Checks if the given buffer contains a valid protocol header, returning true if valid.
bool           protocol_is_valid_header(const uint8_t* buffer, size_t len);

// inline uint8_t protocol_next_seq(uint8_t current) - Returns the next sequence ID by incrementing the current sequence ID, wrapping around at 255.
inline uint8_t protocol_next_seq(uint8_t current) { return current + 1; }
