/*
AppleWin : An Apple //e emulator for Windows

Copyright (C) 1994-1996, Michael O'Brien
Copyright (C) 1999-2001, Oliver Schmidt
Copyright (C) 2002-2005, Tom Charlesworth
Copyright (C) 2006-2007, Tom Charlesworth, Michael Pohoreski

AppleWin is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

AppleWin is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with AppleWin; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* Description: Keyboard emulation
 *
 * Author: Various
 */

#include "StdAfx.h"

#include "Applewin.h"
#include "Frame.h"
#include "Keyboard.h"
#include "Pravets.h"
#include "Tape.h"
#include "YamlHelper.h"
#include "Video.h" // Needed by TK3000 //e, to refresh the frame at each |Mode| change

static BYTE asciicode[2][10] = {
	{0x08,0x0D,0x15,0x2F,0x00,0x00,0x00,0x00,0x00,0x00},
	{0x08,0x0B,0x15,0x0A,0x00,0x00,0x00,0x00,0x00,0x7F}
};	// Convert PC arrow keys to Apple keycodes

bool  g_bShiftKey = false;
bool  g_bCtrlKey  = false;
bool  g_bAltKey   = false;

static bool  g_bTK3KModeKey   = false; //TK3000 //e |Mode| key

static bool  g_bCapsLock = true; //Caps lock key for Apple2 and Lat/Cyr lock for Pravets8
static bool  g_bP8CapsLock = true; //Caps lock key of Pravets 8A/C
static int   lastvirtkey     = 0;	// Current PC keycode
static BYTE  keycode         = 0;	// Current Apple keycode

static BOOL  keywaiting      = 0;

static BYTE g_nLastKey = 0x00;

//
// ----- ALL GLOBALLY ACCESSIBLE FUNCTIONS ARE BELOW THIS LINE -----
//

//===========================================================================

void KeybReset()
{
	keywaiting = 0;
}

//===========================================================================
bool KeybGetAltStatus ()
{
	return g_bAltKey;
}

//===========================================================================
bool KeybGetCapsStatus ()
{
	return g_bCapsLock;
}
//===========================================================================
bool KeybGetP8CapsStatus()
{
	return g_bP8CapsLock;
}
//===========================================================================
/*
bool KeybGetCapsAllowed() //For Pravets 8A/C only
{
	return g_CapsLockAllowed;
}
*/
//===========================================================================
bool KeybGetCtrlStatus ()
{
	return g_bCtrlKey;
}

//===========================================================================
bool KeybGetShiftStatus ()
{
	return g_bShiftKey;
}

//===========================================================================
void KeybUpdateCtrlShiftStatus()
{
	g_bShiftKey = (GetKeyState( VK_SHIFT  ) & KF_UP) ? true : false; // 0x8000 KF_UP
	g_bCtrlKey  = (GetKeyState( VK_CONTROL) & KF_UP) ? true : false;
	g_bAltKey   = (GetKeyState( VK_MENU   ) & KF_UP) ? true : false;
}

//===========================================================================
BYTE KeybGetKeycode ()		// Used by MemCheckPaging() & VideoCheckMode()
{
	return keycode;
}

//===========================================================================
void KeybQueueKeypress (int key, BOOL bASCII)
{
	if (bASCII == ASCII)
	{
		if (g_bFreshReset && key == VK_CANCEL) // OLD HACK: 0x03
		{
			g_bFreshReset = false;
			return; // Swallow spurious CTRL-C caused by CTRL-BREAK
		}

		g_bFreshReset = false;
		if ((key > 0x7F) && !g_bTK3KModeKey) // When in TK3000 mode, we have special keys which need remapping
			return;

		if (!IS_APPLE2) 
		{
			P8Shift = false;
			if (g_bCapsLock && (key >= 'a') && (key <='z'))
			{
				P8Shift = true;
				keycode = key - 32;
			}
			else
			{
				keycode = key;
			}			

			//The latter line should be applied for Pravtes 8A/C only, but not for Pravets 82/M !!!			
			if ((g_bCapsLock == false) && (key >= 'A') && (key <='Z'))
			{
				P8Shift = true;
				if (g_Apple2Type == A2TYPE_PRAVETS8A) 
					keycode = key + 32;
			}

			//Remap some keys for Pravets82/M
			if (g_Apple2Type == A2TYPE_PRAVETS82)
			{
				if (key == 64) 
					keycode = 96;
				if (key == '^') 
					keycode = '~';

				if (g_bCapsLock == false) //cyrillic letters
				{
					if (key == '`') keycode = '^';
					if (key == 92) keycode = '@'; // \ to @	
					if (key == 124) keycode = 92;
				}
				else //(g_bCapsLock == true) //latin letters
				{
					if (key == 91) keycode = 123;
					if (key == 93) keycode = 125;
					if (key == 124) keycode = 92;
				}
			}
			if (g_Apple2Type == A2TYPE_PRAVETS8M)  //Pravets 8M charset is still uncertain
			{
				if (g_bCapsLock == false) //cyrillic letters
				{
					if (key == '[') keycode = '{';
					if (key == ']') keycode = '}';
					if (key == '`') keycode = '~'; //96= key `~
					if (key == 92) keycode = 96;
				}
				else //latin letters
				{
					if (key == '`') 
						keycode = '^'; //96= key `~
				}
			}
			//Remap some keys for Pravets8A/C, which has a different charset for Pravtes82/M, whose keys MUST NOT be remapped.
			if (g_Apple2Type == A2TYPE_PRAVETS8A) //&& (g_bCapsLock == false))
			{
				if (g_bCapsLock == false) //i.e. cyrillic letters
			    {
					if (key == '[') keycode = '{';
					if (key == ']') keycode = '}';
					if (key == '`') keycode = '~';
					if (key == 92) keycode = 96;
					if (GetCapsLockAllowed() == true)
					{
						if ((key == 92) || (key == 124)) keycode = 96; //� to �
						//This shall be rewriten, so that enabling CAPS_LOCK (i.e. F10) will not invert these keys values)
						//The same for latin letters.
						if ((key == '{') || (key == '}') || (key == '~') || (key == 124) || (key == '^') ||  (key == 95))
							P8Shift = true;					
					}
				}
				else //i.e. latin letters
				{
					if (GetCapsLockAllowed()  == false)
					{
						if (key == '{') keycode = '[';
						if (key == '}') keycode = ']';
						if (key == 124) 
							keycode = 92;
						/*if (key == 92) 
							keycode = 124;*/
					//Characters ` and ~ cannot be generated in 7bit character mode, so they are replaced with
					}
					else
					{
						if (key == '{') keycode = 91;
						if (key == '}')	keycode = 93;
						if (key == 124)	keycode = 92;					
						if ((key == '[') || (key == ']') || (key == 92) || (key == '^') || (key == 95))
							P8Shift= true; 
						if (key == 96)	 //This line shall generate sth. else i.e. ` In fact. this character is not generateable by the pravets keyboard.
						{
							keycode = '^';
							P8Shift= true;
						}
						if (key == 126)	keycode = '^';
					}
				}
			}
			// Remap for the TK3000 //e, which had a special |Mode| key for displaying accented chars on screen
			// Borrowed from F�bio Belavenuto's TK3000e emulator (Copyright (C) 2004) - http://code.google.com/p/tk3000e/
			if (g_bTK3KModeKey)	// We already switch this on only if the the TK3000 is currently being emulated
			{
				if ((key >= 0xC0) && (key <= 0xDA)) key += 0x20; // Convert uppercase to lowercase
				switch (key)
				{
					case 0xE0: key = '_';  break; // �
					case 0xE1: key = '@';  break; // �
					case 0xE2: key = '\\'; break; // �
					case 0xE3: key = '[';  break; // �
					case 0xE7: key = ']';  break; // �
					case 0xE9: key = '`';  break; // �
					case 0xEA: key = '&';  break; // �
					case 0xED: key = '{';  break; // �
					case 0xF3: key = '~';  break; // �
					case 0xF4: key = '}';  break; // �
					case 0xF5: key = '#';  break; // �
					case 0xFA: key = '|';  break; // �
				}
				if (key > 0x7F) return;	// Get out
				if ((key >= 'a') && (key <= 'z') && (g_bCapsLock))
					keycode = key - ('a'-'A');
				else
					keycode = key;
			}
		}
		else
		{
			if (g_Apple2Type == A2TYPE_PRAVETS8A)
			{
			}
			else
			{
				if (key >= '`')
					keycode = key - 32;
				else
					keycode = key;
			}
		}
		lastvirtkey = LOBYTE(VkKeyScan(key));
	} 
	else //(bASCII != ASCII)
	{
		// Note: VK_CANCEL is Control-Break
		if ((key == VK_CANCEL) && (GetKeyState(VK_CONTROL) < 0))
		{
			g_bFreshReset = true;
			CtrlReset();
			return;
		}

		if ((key == VK_INSERT) && (GetKeyState(VK_SHIFT) < 0))
		{
			// Shift+Insert
			ClipboardInitiatePaste();
			return;
		}

		if (key == VK_SCROLL)
		{	// For the TK3000 //e we use Scroll Lock to switch between Apple ][ and accented chars modes
			if (g_Apple2Type == A2TYPE_TK30002E)
			{
				g_bTK3KModeKey = (GetKeyState(VK_SCROLL) & 1) ? true : false;	// Sync with the Scroll Lock status
				FrameRefreshStatus(DRAW_LEDS);	// TODO: Implement |Mode| LED in the UI; make it appear only when in TK3000 mode
				VideoRedrawScreen();	// TODO: Still need to implement page mode switching and 'whatnot'
			}
		}

		if (!((key >= VK_LEFT) && (key <= VK_DELETE) && asciicode[IS_APPLE2 ? 0 : 1][key - VK_LEFT]))
			return;

		keycode = asciicode[IS_APPLE2 ? 0 : 1][key - VK_LEFT];		// Convert to Apple arrow keycode
		lastvirtkey = key;
	}

	keywaiting = 1;
}

//===========================================================================

static HGLOBAL hglb = NULL;
static LPTSTR lptstr = NULL;
static bool g_bPasteFromClipboard = false;
static bool g_bClipboardActive = false;

void ClipboardInitiatePaste()
{
	if (g_bClipboardActive)
		return;

	g_bPasteFromClipboard = true;
}

static void ClipboardDone()
{
	if (g_bClipboardActive)
	{
		g_bClipboardActive = false;
		GlobalUnlock(hglb);
		CloseClipboard();
	}
}

static void ClipboardInit()
{
	ClipboardDone();

	if (!IsClipboardFormatAvailable(CF_TEXT))
		return;
	
	if (!OpenClipboard(g_hFrameWindow))
		return;
	
	hglb = GetClipboardData(CF_TEXT);
	if (hglb == NULL)
	{	
		CloseClipboard();
		return;
	}

	lptstr = (char*) GlobalLock(hglb);
	if (lptstr == NULL)
	{	
		CloseClipboard();
		return;
	}

	g_bPasteFromClipboard = false;
	g_bClipboardActive = true;
}

static char ClipboardCurrChar(bool bIncPtr)
{
	char nKey;
	int nInc = 1;

	if((lptstr[0] == 0x0D) && (lptstr[1] == 0x0A))
	{
		nKey = 0x0D;
		nInc = 2;
	}
	else
	{
		nKey = lptstr[0];
	}

	if(bIncPtr)
		lptstr += nInc;

	return nKey;
}

//===========================================================================

// For AKD (Any Key Down), need special handling for the hooked key combos(*), as GetKeyState() doesn't detect the keys as being down.
// . And equally GetKeyState() doesn't detect the keys as being up: eg. Whilst pressing TAB, press LEFT ALT, then release TAB.
// (*) ALT+TAB, ALT+ESCAPE, ALT+SPACE

static enum {AKD_TAB=0, AKD_ESCAPE, AKD_SPACE};
static bool g_specialAKD[3] = {false,false,false};

void KeybSpecialKeyTransition(UINT message, WPARAM wparam)
{
	_ASSERT(message == WM_KEYUP || message == WM_KEYDOWN);
	bool bState = message == WM_KEYDOWN;

	switch (wparam)
	{
	case VK_TAB:
		g_specialAKD[AKD_TAB] = bState;
		break;
	case VK_ESCAPE:
		g_specialAKD[AKD_ESCAPE] = bState;
		break;
	case VK_SPACE:
		g_specialAKD[AKD_SPACE] = bState;
		break;
	};
}

static void GetKeyStateOfSpecialAKD(int lastvirtkey, bool& bState)
{
	if (VK_TAB == lastvirtkey)
		bState = g_specialAKD[AKD_TAB];
	else if (VK_ESCAPE == lastvirtkey)
		bState = g_specialAKD[AKD_ESCAPE];
	else if (VK_SPACE == lastvirtkey)
		bState = g_specialAKD[AKD_SPACE];
}

//===========================================================================

BYTE __stdcall KeybReadData (WORD, WORD, BYTE, BYTE, ULONG)
{
	LogFileTimeUntilFirstKeyRead();

	if(g_bPasteFromClipboard)
		ClipboardInit();

	if(g_bClipboardActive)
	{
		if(*lptstr == 0)
			ClipboardDone();
		else
			return 0x80 | ClipboardCurrChar(false);
	}

	//

	return keycode | (keywaiting ? 0x80 : 0);
}

//===========================================================================

BYTE __stdcall KeybReadFlag (WORD, WORD, BYTE, BYTE, ULONG)
{
	if(g_bPasteFromClipboard)
		ClipboardInit();

	if(g_bClipboardActive)
	{
		if(*lptstr == 0)
			ClipboardDone();
		else
			return 0x80 | ClipboardCurrChar(true);
	}

	//

	keywaiting = 0;

	if (IS_APPLE2)	// Include Pravets machines too?
		return keycode;

	// AKD

	bool bState = GetKeyState(lastvirtkey) < 0;
	GetKeyStateOfSpecialAKD(lastvirtkey, bState);

	return keycode | (bState ? 0x80 : 0);
}

//===========================================================================
void KeybToggleCapsLock ()
{
	if (!IS_APPLE2)
	{
		g_bCapsLock = (GetKeyState(VK_CAPITAL) & 1);
		FrameRefreshStatus(DRAW_LEDS);
	}
}

//===========================================================================
void KeybToggleP8ACapsLock ()
{
	_ASSERT(g_Apple2Type == A2TYPE_PRAVETS8A);
	P8CAPS_ON = !P8CAPS_ON;
	FrameRefreshStatus(DRAW_LEDS);
	// g_bP8CapsLock= g_bP8CapsLock?false:true; //The same as the upper, but slower
}

//===========================================================================

void KeybSetSnapshot_v1(const BYTE LastKey)
{
	g_nLastKey = LastKey;
}

//

#define SS_YAML_KEY_LASTKEY "Last Key"

static std::string KeybGetSnapshotStructName(void)
{
	static const std::string name("Keyboard");
	return name;
}

void KeybSaveSnapshot(YamlSaveHelper& yamlSaveHelper)
{
	YamlSaveHelper::Label state(yamlSaveHelper, "%s:\n", KeybGetSnapshotStructName().c_str());
	yamlSaveHelper.SaveHexUint8(SS_YAML_KEY_LASTKEY, g_nLastKey);
}

void KeybLoadSnapshot(YamlLoadHelper& yamlLoadHelper)
{
	if (!yamlLoadHelper.GetSubMap(KeybGetSnapshotStructName()))
		return;

	g_nLastKey = (BYTE) yamlLoadHelper.LoadUint(SS_YAML_KEY_LASTKEY);

	yamlLoadHelper.PopMap();
}
