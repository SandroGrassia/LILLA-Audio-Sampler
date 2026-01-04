/*
 * LILLA Audio Sampler
 * Author: Sandro Grassia, info@lillasampler.it
 *
 */

#include "SharedVFS.h"


// VFS/Virtual File System

int VFS_FAT_table[VFS_PACKETS_DS] = {0};
int VFS_packets = 0; // number of Packets on FLASH (maximum is VFS_PACKETS_MAX)
int VFS_packets_max = 0;

EEPROM_VFS_Recording EEPROM_Recording[RECORDINGS];
VFS_Recording Recording[RECORDINGS];