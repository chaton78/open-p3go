#include "PL3/config.h"
#include "PL3/shellcode_egghunt.h"

#define MAGIC_NUMBER		0x50, 0x53, 0x47, 0x72, 0x6f, 0x6f, 0x76, 0x65

#if defined (FIRMWARE_3_41)
#define RTOC_TABLE		0x80, 0x00, 0x00, 0x00, 0x00, 0x33, 0xe7, 0x20
#elif defined (FIRMWARE_3_15)
#define RTOC_TABLE		0x80, 0x00, 0x00, 0x00, 0x00, 0x33, 0xda, 0x10
#elif defined (FIRMWARE_3_10)
#define RTOC_TABLE		0x80, 0x00, 0x00, 0x00, 0x00, 0x33, 0xda, 0x10
#elif defined (FIRMWARE_3_01)
#define RTOC_TABLE		0x80, 0x00, 0x00, 0x00, 0x00, 0x32, 0x06, 0x40
#else
#error You must specify the target firmware. Define a supported FIRMWARE_X_YZ in Makefile
#endif /* FIRMWARE_X_YZ */


#ifdef USE_JIG
#define default_shellcode shellcode_egghunt
#define default_shellcode_macro shellcode_egghunt_macro

#define PAYLOAD_FW_DEPENDENT
//#define PAYLOAD payload_dev
#define PAYLOAD payload_no_unauth_syscall


#if defined (FIRMWARE_3_41)
#define FIRMWARE 3_41
#define SHELLCODE_ADDR_BASE	0x80, 0x00, 0x00, 0x00, 0x00, 0x3d, 0xee, 0x70
#elif defined (FIRMWARE_3_15)
#define FIRMWARE 3_15
#define SHELLCODE_ADDR_BASE	0x80, 0x00, 0x00, 0x00, 0x00, 0x3d, 0xde, 0x30
#elif defined (FIRMWARE_3_10)
#define FIRMWARE 3_10
#define SHELLCODE_ADDR_BASE	0x80, 0x00, 0x00, 0x00, 0x00, 0x3d, 0xde, 0x30
#elif defined (FIRMWARE_3_01)
#define FIRMWARE 3_01
#define SHELLCODE_ADDR_BASE	0x80, 0x00, 0x00, 0x00, 0x00, 0x3B, 0xFB, 0xC8
#endif /* FIRMWARE_X_YZ */


#define SHELLCODE_PAGE		0x80, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00
#define SHELLCODE_DESTINATION	SHELLCODE_ADDR_BASE
#define SHELLCODE_PTR 		SHELLCODE_ADDR_BASE + 0x08
#define SHELLCODE_ADDRESS	SHELLCODE_ADDR_BASE + 0x18

#define PORT1_NUM_CONFIGS	4

#else /* USE_JIG */
#define default_shellcode shellcode_egghunt
#define default_shellcode_macro shellcode_egghunt_macro

#define PAYLOAD dump_lv2

#define SHELLCODE_ADDR_BASE	0x80, 0x00, 0x00, 0x00, 0x00, 0x4E, 0x00, 0x00

#define SHELLCODE_PAGE		SHELLCODE_ADDR_BASE
#define SHELLCODE_DESTINATION	SHELLCODE_ADDR_BASE + 0x20
#define SHELLCODE_PTR 		SHELLCODE_ADDR_BASE + 0x28
#define SHELLCODE_ADDRESS	SHELLCODE_ADDR_BASE + 0x38

#define PORT1_NUM_CONFIGS	100

#endif /* USE_JIG */


#ifdef PAYLOAD_FW_DEPENDENT

# define PAYLOAD_INCLUDE_(payload, firmware) <PL3/payload##_##firmware.h>
# define PAYLOAD_INCLUDE(payload, firmware) PAYLOAD_INCLUDE_(payload, firmware)
# define default_payload_macro__(payload, firmware) payload##_##firmware##_macro
# define default_payload_macro_(payload, firmware) \
  default_payload_macro__(payload, firmware)

# include PAYLOAD_INCLUDE(PAYLOAD, FIRMWARE)
# define default_payload_macro default_payload_macro_(PAYLOAD, FIRMWARE)

#else

# define PAYLOAD_INCLUDE_(payload) <PL3/payload.h>
# define PAYLOAD_INCLUDE(payload) PAYLOAD_INCLUDE_(payload)
# define default_payload_macro__(payload) payload##_macro
# define default_payload_macro_(payload) default_payload_macro__(payload)

# include PAYLOAD_INCLUDE(PAYLOAD)
# define default_payload_macro default_payload_macro_(PAYLOAD)

#endif


#define PORT1_DESC_LEN		0xf00
#define PORT1_DESC_LEN_HI	((PORT1_DESC_LEN >> 8) & 0xFF)
#define PORT1_DESC_LEN_LO	(PORT1_DESC_LEN & 0xFF)

#define PORT3_DESC_LEN		0xa4d
#define PORT3_DESC_LEN_HI	((PORT3_DESC_LEN >> 8) & 0xFF)
#define PORT3_DESC_LEN_LO	(PORT3_DESC_LEN & 0xFF)

static const u8 jig_response[64] = {
	SHELLCODE_PTR,
	SHELLCODE_ADDRESS,
	RTOC_TABLE,
	default_shellcode_macro,
};

static const u8 HUB_Device_Descriptor[] = {
	0x12, 0x01, 0x00, 0x02, 0x09, 0x00, 0x01, 0x08,
	0xAA, 0xAA, 0xCC, 0xCC, 0x00, 0x01, 0x00, 0x00,
	0x00, 0x01,
};

static const u8 HUB_Config_Descriptor[] = {
	// Config
	0x09, 0x02, 0x19, 0x00, 0x01, 0x01, 0x00, 0xe0,
	0x32,
	// Interface
	0x09, 0x04, 0x00, 0x00, 0x01, 0x09, 0x00, 0x00,
	0x00,
	// Endpoint (interrupt in)
	0x07, 0x05, 0x81, 0x03, 0x01, 0x00, 0x0c,
};

static const u8 HUB_Hub_Descriptor[] = {
	0x09, 0x29, 0x06, 0xa9, 0x00, 0x05, 0x64, 0x00,
	0xff,
};

static const u8 port1_device_descriptor[] = {
	0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x08,
	0xAA, 0xAA, 0x55, 0x55, 0x00, 0x00, 0x00, 0x00,
	0x00, PORT1_NUM_CONFIGS,
};

static const u8 port1_short_config_descriptor[] = {
	0x09, 0x02, PORT1_DESC_LEN_LO, PORT1_DESC_LEN_HI, 0x01, 0x00, 0x00, 0x80,
};

static const u8 port1_config_descriptor[] = {
	0x09, 0x02, 0x12, 0x00, 0x01, 0x00, 0x00, 0x80, 0xfa,
	0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	MAGIC_NUMBER,
#ifndef USE_JIG
	SHELLCODE_PTR,
	SHELLCODE_ADDRESS,
	RTOC_TABLE,
#endif
	default_payload_macro,
};

static const u8 port2_device_descriptor[] = {
	0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x08,
	0xAA, 0xAA, 0xBB, 0xBB, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01,
};

static const u8 port2_config_descriptor[] = {
	// config
	0x09, 0x02, 0x16, 0x00, 0x01, 0x01, 0x00, 0x80,
	0x01,
	// interface
	0x09, 0x04, 0x00, 0x00, 0x00, 0xFE, 0x01, 0x02,
	0x00,
	// extra
	0x04, 0x21, 0xb4, 0x2f,
};

static const u8 port3_device_descriptor[] = {
	0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x08,
	0xAA, 0xAA, 0x55, 0x55, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x02,
};

static const u8 port3_config_descriptor[] = {
  0x09, 0x02, 0x4d, 0x0a, 0x01, 0x01, 0x00, 0x80, 0x01,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x00, 0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00, 0x09, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00, 0x09, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x02, 0x00, 0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x00, 0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00, 0x09, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x02, 0x00, 0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x00, 0xfe, 0x01, 0x02, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02,
};

static const u8 port4_device_descriptor[] = {
	0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x08,
	0xAA, 0xAA, 0x55, 0x55, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x03,
};

static const u8 port4_config_descriptor_1[] = {
	// config
	0x09, 0x02, 0x12, 0x00, 0x01, 0x01, 0x00, 0x80,
	0x01,
	// interface
	0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02,
	0x00,
};

static const u8 port4_short_config_descriptor_2[] = {
	// config
	0x09, 0x02, 0x12, 0x00, 0x01, 0x01, 0x00, 0x80,
};

static const u8 port4_config_descriptor_2[] = {
	// config
	0x09, 0x02, 0x00, 0x00, 0x01, 0x01, 0x00, 0x80,
	0x01,
	// interface
	0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02,
	0x00,
};

static const u8 port4_config_descriptor_3[] = {
	0x09, 0x02, 0x30, 0x00, 0x01, 0x01, 0x00, 0x80, 0x01, 0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01,
	0x02, 0x00, 0x3e, 0x21, 0x00, 0x00, 0x00, 0x00,
	MAGIC_NUMBER, /* magic number to look for in the start of the page */
	SHELLCODE_PAGE, /* Initial data search ptr */
	SHELLCODE_DESTINATION, /* destination ptr for heap structure (jig response) */
};

static const u8 port5_device_descriptor[] = {
	0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x08,
	0x4c, 0x05, 0xeb, 0x02, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01,
};

const u8 port5_config_descriptor_pl3[] = {
	// config
	0x09, 0x02, 0x20, 0x00, 0x01, 0x00, 0x00, 0x80,
	0x01,
	// interface
	0x09, 0x04, 0x00, 0x00, 0x02, 0xff, 0x00, 0x00,
	0x00,
	// endpoint
	0x07, 0x05, 0x01, 0x02, 0x08, 0x00, 0x00,	//danny changed out endpoint to 1
	// endpoint
	0x07, 0x05, 0x81, 0x02, 0x08, 0x00, 0x00,
};

static const u8 port6_device_descriptor[] = {
	0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x08,
	0xaa, 0xaa, 0xc0, 0xde, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01,
};

static const u8 port6_config_descriptor[] = {
	// config
	0x09, 0x02, 0x12, 0x00, 0x01, 0x01, 0x00, 0x80,
	0x01,
	// interface
	0x09, 0x04, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x02,
	0x00,
};