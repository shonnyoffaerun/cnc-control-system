#pragma once

#include <cstdint>
#include <cstddef>

/*
    enum class MotionMode - enumeration of gcode commands to select the type of motion.
    Attributes: Rapid - rapid motion,
                Linear - linear motion,
                CW_Arc - clockwise arc motion,
                CCW_Arc - counterclockwise arc motion,
                Probe - probe motion,
                Cancel - cancel motion
*/
enum class MotionMode : uint8_t 
{
    Rapid    = 0x00,
    Linear   = 0x01,
    CW_Arc   = 0x02,
    CCW_Arc  = 0x03,
    Probe    = 0x04,
    Cancel   = 0x50,
    Unknown  = 0xFF
};

/*
    enum class CoordSystem - enumeration of gcode commands to select the coordinate system.
    Attributes: G54 - coordinate system 54,
                G55 - coordinate system 55,
                G56 - coordinate system 56,
                G57 - coordinate system 57,
                G58 - coordinate system 58,
                G59 - coordinate system 59,
                G90 - absolute coordinate system,
                G91 - incremental coordinate system
                Unknown - unknown coordinate system 
*/
enum class CoordSystem : uint8_t
{
    G54     = 0x01,
    G55     = 0x02,
    G56     = 0x03,
    G57     = 0x04,
    G58     = 0x05,
    G59     = 0x06,
    G90     = 0x10,
    G91     = 0x11,
    Unknown = 0xFF
};

/*
    enum class UnitMode - enumeration of gcode commands to select the unit mode.
    Attributes: MM - millimeters,
                Inch - inches
*/
enum class UnitMode : uint8_t
{
    MM    = 0x00,
    Inch  = 0x01
};

/*
    enum class PlaneMode - enumeration of gcode commands to select the plane mode.
    Attributes: XY - XY plane,
                XZ - XZ plane,
                YZ - YZ plane
*/
enum class PlaneMode : uint8_t
{
    XY = 0x00,
    XZ = 0x01,
    YZ = 0x02
};

/*
    enum class FeedMode - enumeration of gcode commands to select the feed mode.
    Attributes: UnitsPerMin - units per minute,
                InverseTime - inverse time,
                PerRev - per revolution
*/
enum class FeedMode : uint8_t
{
    UnitsPerMin = 0x00,
    InverseTime = 0x01,
    PerRev      = 0x02
};

/*
    enum class MachineState - enumeration of machine states.
    Attributes: Idle - idle,
                Alarm - alarm,
                Run - currently running,
                Hold - holding,
                Jog - jog,
                Home - home,
                Check - checking,
                Door - door opened,
                Unknown - unknown
*/
enum class MachineState : uint8_t
{
    Idle    = 0x00,
    Alarm   = 0x01,
    Run     = 0x02,
    Hold    = 0x03,
    Jog     = 0x04,
    Home    = 0x05,
    Check   = 0x06,
    Door    = 0x07,
    Unknown = 0xFF
};

/*
    enum class GCodeError - class of gcode error codes, used for error handling.
    Attributes: None - no error,
                BadNumberFormat - bad number format,
                UnsupportedCmd - unsupported command,
                MissingParam - missing parameter,
                InvalidAxis - invalid axis,
                ArcRadiusError - arc radius error,
                ModalViolation - modal violation,
                QueueOverflow - queue overflow,
                HardLimit - hard limit,
                SoftLimit - soft limit
*/
enum class GCodeError : uint8_t
{
    None            = 0x00,
    BadNumberFormat = 0x01,
    UnsupportedCmd  = 0x02,
    MissingParam    = 0x03,
    InvalidAxis     = 0x04,
    ArcRadiusError  = 0x05,
    ModalViolation  = 0x06,
    QueueOverflow   = 0x07,
    HardLimit       = 0x08,
    SoftLimit       = 0x09
};

/*
    namespace StatusFlags - namespace of constant status flags.
    Attributes: constexpr uint8_t IS_CYCLE_ACTIVE - cycle active,
                constexpr uint8_t IS_HOLD_ACTIVE - hold active,
                constexpr uint8_t IS_HOMED - homed,
                constexpr uint8_t IS_ALARM - alarm,
                constexpr uint8_t SPINDLE_CW - spindle clockwise,
                constexpr uint8_t SPINDLE_CCW - spindle counterclockwise,
                constexpr uint8_t COOLANT_MIST - coolant mist,
                constexpr uint8_t COOLANT_FLOOD - coolant flood
*/
namespace StatusFlags
{
    constexpr uint8_t IS_CYCLE_ACTIVE = (1 << 0);
    constexpr uint8_t IS_HOLD_ACTIVE  = (1 << 1);
    constexpr uint8_t IS_HOMED        = (1 << 2);
    constexpr uint8_t IS_ALARM        = (1 << 3);
    constexpr uint8_t SPINDLE_CW      = (1 << 4);
    constexpr uint8_t SPINDLE_CCW     = (1 << 5);
    constexpr uint8_t COOLANT_MIST    = (1 << 6);
    constexpr uint8_t COOLANT_FLOOD   = (1 << 7);
}

#pragma pack(push, 1)

/*
    struct GCodeCommand - main structure to realize the GCode responce to the host.
    Attributes: MotionMode motion - motion mode,
                CoordSystem coord - coordinate system,
                UnitMode units - unit mode,
                PlaneMode plane - plane mode,
                FeedMode feed_mode - feed mode,
                float x - x coordinate,
                float y - y coordinate,
                float z - z coordinate,
                float i - i coordinate,
                float j - j coordinate,
                float k - k coordinate,
                float feed_rate - feed rate,
                uint16_t spindle_rpm - spindle rpm,
                uint8_t tool_index - tool index,
                bool is_relative - is relative,
                bool is_probe - is probe
*/
struct GCodeCommand
{
    MotionMode   motion;
    CoordSystem  coord;
    UnitMode     units;
    PlaneMode    plane;
    FeedMode     feed_mode;
    
    float        x{0.0f}, y{0.0f}, z{0.0f};
    float        i{0.0f}, j{0.0f}, k{0.0f};
    float        feed_rate{0.0f};
    uint16_t     spindle_rpm{0};
    
    uint8_t      tool_index{0};
    bool         is_relative{false};
    bool         is_probe{false};
};

/*
    struct MachineStatus - structure to realize current state of machine;
    Attributes: MachineState state - machine state,
                uint8_t status_flags - status flags,
                uint8_t active_tool - active tool,
                float pos_x - x position,
                float pos_y - y position,
                float pos_z - z position,
                float offset_x - x offset,
                float offset_y - y offset,
                float offset_z - z offset,
                uint16_t feed_rate - feed rate,
                uint16_t spindle_rpm - spindle rpm,
                uint8_t queued_blocks - queued blocks
*/
struct MachineStatus
{
    MachineState state;
    uint8_t      status_flags;
    uint8_t      active_tool;

    float        pos_x{0.0f}, pos_y{0.0f}, pos_z{0.0f};
    float        offset_x{0.0f}, offset_y{0.0f}, offset_z{0.0f};

    uint16_t     feed_rate{0};
    uint16_t     spindle_rpm{0};
    uint8_t      queued_blocks{0};
};

/*
    struct ParserLimits - struct to implement parser limits;
    Attributes: float max_feed_rate - maximum feed rate,
                float max_rapid_rate - maximum rapid rate,
                float max_arc_radius - maximum arc radius,
                float arc_tolerance - arc tolerance,
                uint8_t max_buffer_size - maximum buffer size,
                bool laser_mode - laser mode
*/
struct ParserLimits
{
    float   max_feed_rate{5000.0f};
    float   max_rapid_rate{3000.0f};
    float   max_arc_radius{1000.0f};
    float   arc_tolerance{0.002f};
    uint8_t max_buffer_size{32};
    bool    laser_mode{false};
};

#pragma pack(pop)

/*
    namespace GCodeConstants - namespace of constant values of GCode.
    Attributes: constexpr size_t MAX_LINE_LENGTH - maximum line length,
                constexpr float SAFE_Z_HEIGHT - safe z height,
                constexpr float ARC_TOLERANCE - arc tolerance,
                constexpr uint8_t QUEUE_WATERMARK - queue watermark
*/
namespace GCodeConstants
{
    constexpr size_t  MAX_LINE_LENGTH = 128;
    constexpr float   SAFE_Z_HEIGHT  = 10.0f;
    constexpr float   ARC_TOLERANCE  = 0.002f;
    constexpr uint8_t QUEUE_WATERMARK = 24;
}

// inline bool gcode_is_motion_valid(MotionMode mode, UnitMode units) - checks if MotionMode valid 
inline bool gcode_is_motion_valid(MotionMode mode, UnitMode units) 
{
    if (mode == MotionMode::CW_Arc || mode == MotionMode::CCW_Arc) 
    {
        return true;
    }
    return true;
}

// inline float gcode_convert_units(float value, UnitMode from, UnitMode to) - converts units
inline float gcode_convert_units(float value, UnitMode from, UnitMode to) 
{
    if (from == to) return value;
    return (from == UnitMode::Inch) ? value * 25.4f : value / 25.4f;
}

// inline bool gcode_queue_is_full(uint8_t current_queued, uint8_t max_size) - checks if queue is full 
inline bool gcode_queue_is_full(uint8_t current_queued, uint8_t max_size) 
{
    return current_queued >= max_size;
}

// inline uint8_t gcode_state_to_flags(MachineState state) - converts state to flags
inline uint8_t gcode_state_to_flags(MachineState state) 
{
    uint8_t flags = 0;
    if (state == MachineState::Run)    flags |= StatusFlags::IS_CYCLE_ACTIVE;
    if (state == MachineState::Hold)   flags |= StatusFlags::IS_HOLD_ACTIVE;
    if (state == MachineState::Alarm)  flags |= StatusFlags::IS_ALARM;
    return flags;
}

// inline MachineState gcode_flags_to_state(uint8_t flags) - converts flags to state
inline MachineState gcode_flags_to_state(uint8_t flags) 
{
    if (flags & StatusFlags::IS_ALARM)        return MachineState::Alarm;
    if (flags & StatusFlags::IS_HOLD_ACTIVE)  return MachineState::Hold;
    if (flags & StatusFlags::IS_CYCLE_ACTIVE) return MachineState::Run;
    return MachineState::Idle;
}
