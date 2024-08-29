
// ========================================================
// Structs & Enums

typedef enum {
	M_NONE            = 0x0000,				// None machine supported
	M_OCM_ID          = 0x0001,				// Device 0 (1<<0)
	M_ZEMNEO_SX1_ID   = 0x0002,				// Device 1 (1<<1)
	M_SMX_MCP2_ID     = 0x0004,				// Device 2 (1<<2)
	M_SX2             = 0x0008,				// Device 3 (1<<3)
	M_SMXMIN_SMXHB_ID = 0x0010,				// Device 4 (1<<4)
	M_DE0CV_ID        = 0x0020,				// Device 5 (1<<5)
	M_ALL             = 0xffff				// All devices supported
} MachineMask_t;

typedef enum {
	IOREV_ALL = 0,							// Supported by all revisions
	IOREV_1,								// OCM-PLD Pack v2.4
	IOREV_2,								// OCM-PLD Pack v3.0
	IOREV_3,								// OCM-PLD Pack v3.1
	IOREV_4,								// OCM-PLD Pack v3.2
	IOREV_5,								// OCM-PLD Pack v3.4
	IOREV_6,								// OCM-PLD Pack v3.5
	IOREV_7,								// OCM-PLD Pack v3.6
	IOREV_8,								// OCM-PLD Pack v3.6.2
	IOREV_9,								// OCM-PLD Pack v3.7
	IOREV_10,								// OCM-PLD Pack v3.9
	IOREV_11,								// OCM-PLD Pack v3.9.1
	IOREV_NA = 0xff							// Not/Available
} IOrev_t;

typedef enum {
	LABEL,									// No widget, just label
	VALUE,									// Show numeric value
	SLIDER,									// Slider with ranges of 0 to 2/4/8 values
	END,									// **End of Elements array
	// Custom widgets
	CUSTOM_CPUSPEED_VALUE,
	CUSTOM_VOLUME_SLIDER,
} Widget_t;

typedef enum {
	CMDTYPE_NONE,
	CMDTYPE_STANDARD,
	CMDTYPE_CUSTOM_SLOTS12,
} CmdType_t;

enum {										// Masks for Element_t.attribs.raw
	ATR_FORCEPANELRELOAD = 1,
	ATR_NEEDRESETTOAPPLY = 2,
	ATR_UNUSED6          = 8,
	ATR_UNUSED5          = 8,
	ATR_UNUSED4          = 16,
	ATR_UNUSED3          = 32,
	ATR_UNUSED2          = 64,
	ATR_UNUSED1          = 128,
};

typedef struct {
	Widget_t type;							// Widget type
	uint8_t posX, posY;						// Element position
	char *label;							// Element text label
	int8_t goUp, goDown, goLeft, goRight;	// Navigation to indexes (relative offset)
	uint8_t *value;							// Cached full byte for value
	uint8_t valueMask;						// Mask for value
	uint8_t minValue;						// Min value
	uint8_t maxValue;						// Max value
	uint8_t **valueStr;						// Array with labels for each value
	uint8_t valueOffsetX;					// Offset X for value widget
	CmdType_t cmdType;						// OCM Smart Cmd type mode
	uint8_t cmd[8];							// OCM Smart Cmd to set each value
	union {
		uint8_t attribs_raw;
		struct {
			unsigned forcePanelReload: 1;	// Force panel reload when value changes
			unsigned needResetToApply: 1;	// Need a Cold/Warm reset to apply
			unsigned reserved: 6;			// Not used flags [reserved]
		};
	};
	char *description[4];					// Description lines
	IOrev_t ioRevNeeded;					// I/O Revision needed [0x00:all 0xff:n/a]
	MachineMask_t supportedBy;				// Supported machines
} Element_t;

typedef struct {
	char *title;							// Panel name at top header
	uint8_t titlex, titley, titlelen;		// Position & llength at top header
	Element_t *elements;					// Array of elements
} Panel_t;

