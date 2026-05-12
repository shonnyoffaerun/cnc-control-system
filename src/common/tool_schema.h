#pragma once

#include <cstdint>
#include <cstddef>

/*
    enum class ToolType - enumeration of tool types
    Attributes: EndMill - end mill tool.
                BallNose - ball nose tool.
                Drill - drill tool.
                VBit - V bit tool.
                Laser - laser tool.
                Probe - probe tool.
                Unknown - unknown tool.
*/
enum class ToolType : uint8_t 
{
    EndMill  = 0x01,
    BallNose = 0x02,
    Drill    = 0x03,
    VBit     = 0x04,
    Laser    = 0x05,
    Probe    = 0x06,
    Unknown  = 0xFF
};

/*
    enum class ToolMaterial - enumeration of tool materials, used in tool schema.
    Attributes: HSS - high speed steel,
                Carbide - carbide,
                Ceramic - ceramic,
                Diamond - diamond,
                Unknown - unknown.
*/
enum class ToolMaterial : uint8_t 
{
    HSS     = 0x01,
    Carbide = 0x02,
    Ceramic = 0x03,
    Diamond = 0x04,
    Unknown = 0xFF
};

/*
    enum class CoolantType - enumeration of coolant types, used in tool schema.
    Attributes: None - no coolant,
                Air - air coolant,
                Mist - mist coolant,
                Flood - flood coolant.
*/
enum class CoolantType : uint8_t 
{
    None   = 0x00,
    Air    = 0x01,
    Mist   = 0x02,
    Flood  = 0xFF
};

#pragma pack(push, 1)

/*
    struct ToolLimits - limits of tool, as defined in tool schema
    Attributes: uint16_t max_rpm - maximum rpm, supported by tool.
                uint16_t max_feed_mm_min - maximum feed, supported by tool.
                float    max_depth_per_pass - maximum depth per pass, supported by tool.
                float    max_load_percent - maximum load, supported by tool.
*/
struct ToolLimits
{
    uint16_t max_rpm;
    uint16_t max_feed_mm_min;
    float    max_depth_per_pass;
    float    max_load_percent;
};

/*
    struct ToolGeometry - geometry of tool, as defined in tool schema
    Attributes: float diameter_mm - tool diameter in mm.
                float flute_length_mm - tool flute length in mm.
                float total_length_mm - tool total length in mm.
                float corner_radius_mm - tool corner radius in mm.
*/
struct ToolGeometry
{
    float diameter_mm;
    float flute_length_mm;
    float total_length_mm;
    float corner_radius_mm;
};

/*
    struct ToolSchema - tool schema, which contains all information about tool.
    Attributes: uint8_t index - tool index.
                ToolType type - tool type.
                ToolMaterial material - tool material.
                CoolantType coolant - coolant type.
                ToolGeometry geometry - tool geometry.
                ToolLimits limits - tool limits.
*/
struct ToolSchema
{
    uint8_t      index;
    ToolType     type;
    ToolMaterial material;
    CoolantType  coolant;

    ToolGeometry geometry;
    ToolLimits   limits;

    uint8_t      checksum;
};

#pragma pack(pop)

/*
    namespace ToolConstants - namespace of constants, used in tool
    Attributes: constexpr float DEFAULT_DIAMETER - default tool diameter.
                constexpr uint16_t SAFE_RPM_HSS - safe rpm for HSS.
                constexpr uint16_t SAFE_RPM_CARBIDE - safe rpm for carbide.
                constexpr uint8_t CHECKSUM_SEED - seed for checksum.
*/
namespace ToolConstants
{
    constexpr float    DEFAULT_DIAMETER = 3.175f;
    constexpr uint16_t SAFE_RPM_HSS     = 12000;
    constexpr uint16_t SAFE_RPM_CARBIDE = 18000;
    constexpr uint8_t  CHECKSUM_SEED    = 0xAA;
};

// inline uint8_t tool_calc_checksum(const ToolSchema& tool) - function to calculate checksum of ToolSchema tool
inline uint8_t tool_calc_checksum(const ToolSchema& tool)
{
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&tool);
    size_t size = sizeof(tool) - sizeof(tool.checksum);

    uint8_t cs = ToolConstants::CHECKSUM_SEED;
    for (size_t i = 0; i < size; ++i)
    {
        cs ^= ptr[i];
    }
    return cs;
}

// inline bool tool_is_safe_rpm(const ToolSchema& tool, uint16_t requested_rpm) - function to check if rpm is safe
inline bool tool_is_safe_rpm(const ToolSchema& tool,
                             uint16_t requested_rpm)
{
    uint16_t safe_limit = 30000;

    if (tool.material == ToolMaterial::HSS)
    {
        safe_limit = 12000;
    }
    else if (tool.material == ToolMaterial::Carbide)
    {
        safe_limit = 24000;
    }

    return requested_rpm <= safe_limit;
}

