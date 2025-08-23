#ifndef CTCONFIG_H
#define CTCONFIG_H

/*
Compile-Time Configuration definitions for CRSS.
*/

#define PLOT_SIZE_DEFAULT 16 // In multiples of 16

#define PLOT_MAX_TRUSTED 100

// Server ID for Encryption Request Packet
#define PROTOCOL_SERVER_ID "CRSS"

// Dimension Codec Defaults
#define DIM_CODEC_PIGLIN_SAFE 1
#define DIM_CODEC_NATURAL 1
#define DIM_CODEC_AMBIENT_LIGHT 1.0
#define DIM_CODEC_FIXED_TIME 0
#define DIM_CODEC_INFINIBURN "#minecraft:infiniburn-overworld"
#define DIM_CODEC_RESPAWN_ANCHOR_WORKS 0
#define DIM_CODEC_HAS_SKYLIGHT 1
#define DIM_CODEC_BED_WORKS 0
#define DIM_CODEC_EFFECTS "minecraft:overworld"
#define DIM_CODEC_HAS_RAIDS 0
#define DIM_CODEC_MIN_Y -64
#define DIM_CODEC_HEIGHT 1024
#define DIM_CODEC_LOGICAL_HEIGHT 256
#define DIM_CODEC_COORDINATE_SCALE 1.0
#define DIM_CODEC_ULTRAWARM 0
#define DIM_CODEC_HAS_CEILING 0

#endif // CTCONFIG_H
