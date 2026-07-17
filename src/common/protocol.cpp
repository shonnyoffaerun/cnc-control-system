/*
    protocol.cpp - implementation of protocol.h
    Binary framing for host <-> MKS DLC32 (grbl_esp32) communication.
*/

#include "protocol.h"

#include <cstring>
#include <algorithm>

namespace
{
    constexpr uint16_t CRC16_POLY = 0x8005;
    constexpr uint16_t CRC16_INIT = 0xFFFF;

    uint16_t crc16_update(uint16_t crc, uint8_t byte)
    {
        crc ^= static_cast<uint16_t>(byte) << 8;
        for (int i = 0; i < 8; ++i)
        {
            if (crc & 0x8000)
            {
                crc = static_cast<uint16_t>((crc << 1) ^ CRC16_POLY);
            }
            else
            {
                crc = static_cast<uint16_t>(crc << 1);
            }
        }
        return crc;
    }
}

uint16_t protocol_calc_crc16(const uint8_t* data, size_t len)
{
    uint16_t crc = CRC16_INIT;
    if (data == nullptr)
    {
        return crc;
    }
    for (size_t i = 0; i < len; ++i)
    {
        crc = crc16_update(crc, data[i]);
    }
    return crc;
}

size_t protocol_serialize(const ProtocolFrame& frame, uint8_t* buffer, size_t max_out)
{
    const size_t total = PROTOCOL_HEADER_SIZE + frame.header.length + PROTOCOL_CRC_SIZE;
    if (buffer == nullptr || max_out < total)
    {
        return 0;
    }

    size_t offset = 0;
    buffer[offset++] = frame.header.magic;
    buffer[offset++] = frame.header.type;
    buffer[offset++] = frame.header.length;
    buffer[offset++] = frame.header.seq_id;

    std::memcpy(buffer + offset, frame.payload, frame.header.length);
    offset += frame.header.length;

    const uint16_t crc = protocol_calc_crc16(buffer, offset);
    buffer[offset++] = static_cast<uint8_t>(crc & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((crc >> 8) & 0xFF);

    return offset;
}

bool protocol_deserialize(const uint8_t* buffer, size_t len, ProtocolFrame& out_frame)
{
    if (buffer == nullptr)
    {
        return false;
    }
    if (len < PROTOCOL_HEADER_SIZE + PROTOCOL_CRC_SIZE)
    {
        return false;
    }
    if (buffer[0] != PROTOCOL_MAGIC && buffer[0] != PROTOCOL_MAGIC_ACK)
    {
        return false;
    }

    const uint8_t magic  = buffer[0];
    const uint8_t type   = buffer[1];
    const uint8_t length = buffer[2];
    const uint8_t seq_id = buffer[3];

    const size_t total = PROTOCOL_HEADER_SIZE + length + PROTOCOL_CRC_SIZE;
    if (len < total)
    {
        return false;
    }

    const uint16_t received_crc =
        static_cast<uint16_t>(buffer[total - 2]) |
        static_cast<uint16_t>(buffer[total - 1]) << 8;

    const uint16_t calc_crc = protocol_calc_crc16(buffer, PROTOCOL_HEADER_SIZE + length);
    if (calc_crc != received_crc)
    {
        return false;
    }

    out_frame.header.magic  = magic;
    out_frame.header.type   = type;
    out_frame.header.length = length;
    out_frame.header.seq_id = seq_id;

    const size_t copy_len = std::min<size_t>(length, PROTOCOL_MAX_PAYLOAD_SIZE);
    std::memcpy(out_frame.payload, buffer + PROTOCOL_HEADER_SIZE, copy_len);
    out_frame.crc = received_crc;

    return true;
}

bool protocol_is_valid_header(const uint8_t* buffer, size_t len)
{
    if (buffer == nullptr || len < PROTOCOL_HEADER_SIZE)
    {
        return false;
    }
    if (buffer[0] != PROTOCOL_MAGIC && buffer[0] != PROTOCOL_MAGIC_ACK)
    {
        return false;
    }
    return (PROTOCOL_HEADER_SIZE + buffer[2] + PROTOCOL_CRC_SIZE) <= PROTOCOL_MAX_PACKET_SIZE;
}
