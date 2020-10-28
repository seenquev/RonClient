/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __CONST_H_
#define __CONST_H_

#define APP_OS 100

static const char* APP_NAME = "RonClient";
static const char* APP_VERSION = "v0.7.0a";
static const char* APP_COMPANY = "RonIT Company";
static const char* APP_WEBSITE = "http://client.ronit.pl";
static const char* APP_RIGHTS = "Free to use, reconfigure and redistribute";
static const char* APP_INTERACT_ADDRESS = "client.ronit.pl/update";


enum Direction {
	NORTH = 0,
	EAST = 1,
	SOUTH = 2,
	WEST = 3,
	NORTHWEST = 4,
	NORTHEAST = 5,
	SOUTHEAST = 6,
	SOUTHWEST = 7,
	NONE = 0xFF,
};

enum SpeakClasses {
	SPEAK_SAY			= 0x01,	//normal talk
	SPEAK_WHISPER		= 0x02,	//whispering - #w text
	SPEAK_YELL			= 0x03,	//yelling - #y text
	SPEAK_PRIVATE_PN	= 0x04, //Player-to-NPC speaking(NPCs channel)
	SPEAK_PRIVATE_NP	= 0x05, //NPC-to-Player speaking
	SPEAK_PRIVATE		= 0x06, //Players speaking privately to players
	SPEAK_CHANNEL_Y    	= 0x07,	//Yellow message on chat
	SPEAK_CHANNEL_W		= 0x08, //White message in chat
	SPEAK_BROADCAST    	= 0x09,	//Broadcast a message - #b
	SPEAK_CHANNEL_R1   	= 0x0A,	//Talk red on chat - #c
	SPEAK_PRIVATE_RED  	= 0x0B,	//Red private - @name@ text
	SPEAK_CHANNEL_O    	= 0x0C,	//Talk orange on text
	SPEAK_CHANNEL_R2   	= 0x0D,	//Talk red anonymously on chat - #d
	SPEAK_MONSTER_SAY  	= 0x0E,	//Talk orange
	SPEAK_MONSTER_YELL 	= 0x0F,	//Yell orange
};

enum SpeakClasses822 {
	SPEAK822_SAY			= 0x01,	//normal talk
	SPEAK822_WHISPER		= 0x02,	//whispering - #w text
	SPEAK822_YELL			= 0x03,	//yelling - #y text
	SPEAK822_PRIVATE_PN		= 0x04, //Player-to-NPC speaking(NPCs channel)
	SPEAK822_PRIVATE_NP		= 0x05, //NPC-to-Player speaking
	SPEAK822_PRIVATE		= 0x06, //Players speaking privately to players
	SPEAK822_CHANNEL_Y    	= 0x07,	//Yellow message on chat
	SPEAK822_CHANNEL_W		= 0x08, //White message in chat
	SPEAK822_BROADCAST    	= 0x0B,	//Broadcast a message - #b
	SPEAK822_CHANNEL_R1   	= 0x0C,	//Talk red on chat - #c
	SPEAK822_PRIVATE_RED  	= 0x0D,	//Red private - @name@ text
	SPEAK822_CHANNEL_O    	= 0x0E,	//Talk orange on text
	SPEAK822_CHANNEL_R2   	= 0x10,	//Talk red anonymously on chat - #d
	SPEAK822_MONSTER_SAY  	= 0x12,	//Talk orange
	SPEAK822_MONSTER_YELL 	= 0x13,	//Yell orange
};

enum SpeakClasses840 {
	SPEAK840_SAY			= 0x01,
	SPEAK840_WHISPER		= 0x02,
	SPEAK840_YELL			= 0x03,
	SPEAK840_PRIVATE_PN		= 0x04,
	SPEAK840_PRIVATE_NP		= 0x05,
	SPEAK840_PRIVATE		= 0x06,
	SPEAK840_CHANNEL_Y		= 0x07,
	SPEAK840_CHANNEL_W		= 0x08,
	SPEAK840_BROADCAST		= 0x0C,
	SPEAK840_CHANNEL_R1		= 0x0D,
	SPEAK840_PRIVATE_RED	= 0x0E,
	SPEAK840_CHANNEL_O		= 0x0F,
	SPEAK840_CHANNEL_R2		= 0x11,
	SPEAK840_MONSTER_SAY	= 0x13,
	SPEAK840_MONSTER_YELL	= 0x14,
	SPEAK840_RVR_CHANNEL	= 0x09, //Reporting rule violation - Ctrl+R
	SPEAK840_RVR_ANSWER		= 0x0A, //Answering report
	SPEAK840_RVR_CONTINUE	= 0x0B, //Answering the answer of the report
};

enum SpeakClasses870 {
	SPEAK870_SAY			= 0x01,	//normal talk
	SPEAK870_WHISPER		= 0x02,	//whispering - #w text
	SPEAK870_YELL			= 0x03,	//yelling - #y text
	SPEAK870_PRIVATE_PN		= 0x04, //Player-to-NPC speaking(NPCs channel)
	SPEAK870_PRIVATE_NP		= 0x05, //NPC-to-Player speaking
	SPEAK870_PRIVATE		= 0x06, //Players speaking privately to players
	SPEAK870_CHANNEL_Y		= 0x07,	//Yellow message in chat
	SPEAK870_CHANNEL_W		= 0x08, //White message in chat
	SPEAK870_BROADCAST		= 0x09,	//Broadcast a message - #b
	SPEAK870_CHANNEL_R1		= 0x0A, //Talk red on chat - #c
	SPEAK870_PRIVATE_RED	= 0x0B,	//Red private - @name@ text
	SPEAK870_CHANNEL_O		= 0x0C,	//Talk orange on text
	SPEAK870_MONSTER_SAY	= 0x0D,	//Talk orange
	SPEAK870_MONSTER_YELL	= 0x0E,	//Yell orange

	SPEAK870_CHANNEL_R2		= 0xFF, //Not used in 870
};

enum SpeakClasses910 {
	SPEAK910_SAY				= 0x01,
	SPEAK910_WHISPER			= 0x02,
	SPEAK910_YELL				= 0x03,
	SPEAK910_PRIVATE_FROM		= 0x04,
	SPEAK910_PRIVATE_TO			= 0x05,
	SPEAK910_CHANNEL_Y			= 0x07,
	SPEAK910_CHANNEL_O			= 0x08,
	SPEAK910_PRIVATE_NP			= 0x0A,
	SPEAK910_PRIVATE_PN			= 0x0B,
	SPEAK910_BROADCAST			= 0x0C,
	SPEAK910_CHANNEL_R1			= 0x0D, //red - #c text
	SPEAK910_PRIVATE_RED_FROM	= 0x0E, //@name@text
	SPEAK910_PRIVATE_RED_TO		= 0x0F, //@name@text
	SPEAK910_MONSTER_SAY		= 0x22,
	SPEAK910_MONSTER_YELL		= 0x23,

	SPEAK910_CHANNEL_R2		= 0xFE,
	SPEAK910_CHANNEL_W		= 0xFF,
};

enum MessageClasses {
	MSG_ORANGE	                = 0x01, //Orange message in the console
	MSG_ORANGE2					= 0x02, //Not used
	MSG_RED_TEXT	    		= 0x03, //Red message in game window and in the console
	MSG_ADVANCE			        = 0x04, //White message in game window and in the console
	MSG_EVENT			        = 0x05, //White message at the bottom of the game window and in the console
	MSG_STATUS_DEFAULT			= 0x06, //White message at the bottom of the game window and in the console
	MSG_INFO   	    		    = 0x07, //Green message in game window and in the console
	MSG_SMALLINFO			    = 0x08, //White message at the bottom of the game window"
	MSG_BLUE_TEXT          		= 0x09, //Blue smessage in the console
	MSG_RED_INFO         		= 0x0A, //Red message in the console
};

enum MessageClasses822 {
	MSG822_ORANGE	                = 0x13, //Orange message in the console
	MSG822_RED_TEXT	    			= 0x14, //Red message in game window and in the console
	MSG822_ADVANCE			        = 0x15, //White message in game window and in the console
	MSG822_EVENT			        = 0x16, //White message at the bottom of the game window and in the console
	MSG822_STATUS_DEFAULT			= 0x17, //White message at the bottom of the game window and in the console
	MSG822_INFO   	    		    = 0x18, //Green message in game window and in the console
	MSG822_SMALLINFO			    = 0x19, //White message at the bottom of the game window"
	MSG822_BLUE_TEXT          		= 0x1A, //Blue message in the console
	MSG822_RED_INFO         		= 0x11, //Red message in the console

	MSG822_ORANGE2					= 0xFF, //Not used in 822
};

enum MessageClasses842
{
	MSG840_ORANGE			= 0x13, /*Orange message in the console*/
	MSG840_ORANGE2			= 0x14, /*Orange message in the console*/
	MSG840_RED_TEXT			= 0x15, /*Red message in game window and in the console*/
	MSG840_ADVANCE			= 0x16, /*White message in game window and in the console*/
	MSG840_EVENT			= 0x17, /*White message at the bottom of the game window and in the console*/
	MSG840_STATUS_DEFAULT	= 0x18, /*White message at the bottom of the game window and in the console*/
	MSG840_INFO				= 0x19, /*Green message in game window and in the console*/
	MSG840_SMALLINFO		= 0x1A, /*White message at the bottom of the game window"*/
	MSG840_BLUE_TEXT		= 0x1B, /*Blue message in the console*/
	MSG840_RED_INFO			= 0x12, /*Red message in the console*/
};

enum MessageClasses870 {
	MSG870_ORANGE			= 0x0D, //Orange message in the console
	MSG870_ORANGE2			= 0x0E, //Orange message in the console
	MSG870_RED_TEXT			= 0x0F, //Red message in game window and in the console
	MSG870_ADVANCE			= 0x10, //White message in game window and in the console
	MSG870_EVENT			= 0x11, //White message at the bottom of the game window and in the console
	MSG870_STATUS_DEFAULT	= 0x12, //White message at the bottom of the game window and in the console
	MSG870_INFO				= 0x13, //Green message in game window and in the console
	MSG870_SMALLINFO		= 0x14, //White message at the bottom of the game window
	MSG870_BLUE_TEXT		= 0x15, //Blue message in the console
	MSG870_RED_INFO			= 0x16, //Red message in the console
};

enum MessageClasses910 {
	MSG910_STATUS_CONSOLE_BLUE		= 0x04, /*FIXME Blue message in the console*/
	MSG910_STATUS_CONSOLE_RED		= 0x0C, /*Red message in the console*/
	MSG910_STATUS_DEFAULT			= 0x10, /*White message at the bottom of the game window and in the console*/
	MSG910_STATUS_WARNING			= 0x11, /*Red message in game window and in the console*/
	MSG910_EVENT_ADVANCE			= 0x12, /*White message in game window and in the console*/
	MSG910_STATUS_SMALL				= 0x13, /*White message at the bottom of the game window"*/
	MSG910_INFO_DESCR				= 0x14, /*Green message in game window and in the console*/
	MSG910_DAMAGE_DEALT				= 0x15,
	MSG910_DAMAGE_RECEIVED			= 0x16,
	MSG910_HEALED					= 0x17,
	MSG910_EXPERIENCE				= 0x18,
	MSG910_DAMAGE_OTHERS			= 0x19,
	MSG910_HEALED_OTHERS			= 0x1A,
	MSG910_EXPERIENCE_OTHERS		= 0x1B,
	MSG910_EVENT_DEFAULT			= 0x1C, /*White message at the bottom of the game window and in the console*/
	MSG910_LOOT						= 0x1D,
	MSG910_EVENT_ORANGE				= 0x22, /*Orange message in the console*/
	MSG910_STATUS_CONSOLE_ORANGE	= 0x23  /*Orange message in the console*/
};

const unsigned char FluidMap[] = {
	0x00, //EMPTY
	0x01, //BLUE
	0x07, //PURPLE
	0x03, //BROWN
	0x03, //BROWN
	0x02, //RED
	0x04, //GREEN
	0x03, //BROWN
	0x05, //YELLOW
	0x06, //WHITE
	0x07, //PURPLE
	0x02, //RED
	0x05, //YELLOW
	0x03, //BROWN
	0x05, //YELLOW
	0x06, //WHITE
	0x01, //BLUE
};

enum PlayerIcons {
	ICON_NONE		= 0,
	ICON_POISON 	= 1,
	ICON_BURN 		= 2,
	ICON_ENERGY 	= 4,
	ICON_DRUNK 		= 8,
	ICON_MANASHIELD = 16,
	ICON_PARALYZE 	= 32,
	ICON_HASTE 		= 64,
	ICON_SWORDS 	= 128,
	ICON_DROWNING   = 256,
	ICON_FREEZING   = 512,
	ICON_DAZZLED    = 1024,
	ICON_CURSED     = 2048,
	ICON_PARTY_BUFF = 4096,
	ICON_PZBLOCK    = 8192,
	ICON_PZ         = 16384,
};

enum Shields {
	SHIELD_NONE = 0,
	SHIELD_WHITEYELLOW = 1,
	SHIELD_WHITEBLUE = 2,
	SHIELD_BLUE = 3,
	SHIELD_YELLOW = 4,
	SHIELD_BLUE_SHAREDEXP = 5,
	SHIELD_YELLOW_SHAREDEXP = 6,
	SHIELD_BLUE_NOSHAREDEXP_BLINK = 7,
	SHIELD_YELLOW_NOSHAREDEXP_BLINK = 8,
	SHIELD_BLUE_NOSHAREDEXP = 9,
	SHIELD_YELLOW_NOSHAREDEXP = 10
};

enum Actions {
	ACTION_NONE = 0x00,
	ACTION_LOOK = 0x01,
	ACTION_USE = 0x02,
	ACTION_USEWITH = 0x03,
	ACTION_OPEN = 0x04,
	ACTION_OPENNEW = 0x05,
	ACTION_ROTATE = 0x06,
	ACTION_TRADEWITH = 0x07,
	ACTION_MOVETO = 0x08,

	ACTION_ATTACK = 0x11,
	ACTION_FOLLOW = 0x12,
	ACTION_COPYNAME = 0x13,

	ACTION_SENDMESSAGE = 0x21,
	ACTION_IGNORE = 0x22,
	ACTION_UNIGNORE = 0x23,
	ACTION_ADDTOVIP = 0x24,
	ACTION_REMOVEFROMVIP = 0x25,
	ACTION_PARTYINVITE = 0x26,
	ACTION_PARTYJOIN = 0x27,
	ACTION_PARTYREVOKE = 0x28,
	ACTION_PARTYPASSLEADER = 0x29,
	ACRION_RULEVIOLATION = 0x2A,

	ACTION_SETOUTFIT = 0x31,
	ACTION_MOUNT = 0x32,
	ACTION_DISMOUNT = 0x33,
	ACTION_PARTYENABLESHARED = 0x34,
	ACTION_PARTYDISABLESHARED = 0x35,
	ACTION_PARTYLEAVE = 0x36,

	ACTION_CLOSECHANNEL = 0x41,
	ACTION_SAVECHANNEL = 0x42,

	ACTION_PASTE = 0x51,
	ACTION_COPY = 0x52,
	ACTION_CUT = 0x53,
	ACTION_COPYMESSAGE = 0x54,
	ACTION_CLEARCHANNEL = 0x55,

	ACTION_SETHOTKEY = 0x61,
	ACTION_SETHOTKEYTEXT = 0x62,
	ACTION_SETHOTKEYMODE1 = 0x63,
	ACTION_SETHOTKEYMODE2 = 0x64,
	ACTION_SETHOTKEYMODE3 = 0x65,
	ACTION_SETHOTKEYMODE4 = 0x66,
	ACTION_SETHOTKEYMODE5 = 0x67,
	ACTION_CLEARHOTKEY = 0x68,

	ACTION_CLEARFREE = 0x6A,

	ACTION_ADDMARKER = 0x70,
	ACTION_EDITMARKER = 0x71,
	ACTION_REMOVEMARKER = 0x72,
	ACTION_ADDWAYPOINT = 0x73,
	ACTION_EDITWAYPOINT = 0x74,
	ACTION_REMOVEWAYPOINT = 0x75,
	ACTION_CLEARWAYPOINTS = 0x76,
	ACTION_SAVEWAYPOINTS = 0x77,
	ACTION_LOADWAYPOINTS = 0x78,

	ACTION_CHANGEFIGHTMODE = 0x80,

	ACTION_OPENWINDOW = 0xA0,
	ACTION_CHANNELS = 0xA1,
	ACTION_QUESTLOG = 0xA2,
	ACTION_SPELLS = 0xA3,

	ACTION_EXECUTE = 0xFC,
	ACTION_GAME = 0xFD,
	ACTION_LOGOUT = 0xFE,
	ACTION_QUIT = 0xFF,
};

#endif //__CONST_H_
