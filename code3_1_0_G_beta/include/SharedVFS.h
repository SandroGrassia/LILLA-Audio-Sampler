/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#pragma once
#include <Arduino.h>



// VFS/Virtual File System

static constexpr int PACKETS = 1024;
static constexpr int PACKET_DIM = 65536;  // number of bytes in a packet = 256 blocks of 128 samples (256 bytes)
static constexpr int PACKET_BLOCKS = 256; // number of AUDIO_BLOCKs in a packet
static constexpr int PACKETS_LAG = 3;     // Live Sampler: number of packets always EMPTY between last packet recorderd and first packet recorded
static constexpr int FLASH_FREE_SPACE = 131072;  // 512 AUDIO_BLOCKs - minimum space free in Flash chip
static constexpr int VFS_PACKETS_MIN = 20;       // 20x256x2.9ms = 14.8s
static constexpr int VFS_PACKETS_MAX = 512;      // maximum number of Packet in Flash chip <= PACKETS
static constexpr int VFS_PACKETS_DS = 512;       // <= VFS_PACKETS_MAX / 2 ; maximum number of Packets for Direct Sampler
static constexpr int RECORDINGS = 30; // number of recordings allowed; a single recording is made up with a group of 64KB "packets" stored in Flash chip
extern int VFS_FAT_table[VFS_PACKETS_DS]; // contiene il Recording_id cui appartiene il packet
extern int VFS_packets; // number of Packets on FLASH (maximum is VFS_PACKETS_MAX)
extern int VFS_packets_max;
extern const char PROGMEM name_packet[PACKETS][10]; // array dei nomi dei file .rec (packet usati da Direct Sampling)
struct EEPROM_VFS_Recording // 4 byte
{
    uint16_t first_packet; // if stereo is Left channel's first packet (Right channel's first packet is .first_packet + 1)
    uint8_t packets;       // packets per channel
    uint8_t info;
};
extern EEPROM_VFS_Recording EEPROM_Recording[RECORDINGS];
static constexpr uint8_t SIZE_OF_EEPROM_RECORDING = sizeof(EEPROM_Recording[0]);
struct VFS_Recording // runtime
{
    int first_packet; // if stereo is Left channel's first packet (Right channel's first packet is .first_packet + 1)
    int packets;      // packets per channel
    int bytes;        // per channel
    float seconds;
    bool stereo;
    bool consistent;
};
extern VFS_Recording Recording[RECORDINGS];