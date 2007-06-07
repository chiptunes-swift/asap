/*
 * settings.c - settings dialog box for Winamp and GSPlayer plugins
 *
 * Copyright (C) 2007  Piotr Fusik
 *
 * This file is part of ASAP (Another Slight Atari Player),
 * see http://asap.sourceforge.net
 *
 * ASAP is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * ASAP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ASAP; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <windows.h>
#include <tchar.h>

#ifdef _WIN32_WCE
#define CheckDlgButton(hwnd, id, val)  SendDlgItemMessage (hwnd, id, BM_SETCHECK, val, 0)
#define IsDlgButtonChecked(hwnd, id)   (BOOL) SendDlgItemMessage (hwnd, id, BM_GETCHECK, 0, 0)
#endif

#include "asap.h"
#include "gui.h"

#ifndef WASAP

int song_length = -1;
int silence_seconds = -1;
BOOL play_loops = FALSE;
int mute_mask = 0;
static int saved_mute_mask;
#ifdef WINAMP
BOOL playing_info = FALSE;
#endif

static void enableTimeInput(HWND hDlg, BOOL enable)
{
	EnableWindow(GetDlgItem(hDlg, IDC_MINUTES), enable);
	EnableWindow(GetDlgItem(hDlg, IDC_SECONDS), enable);
}

static void setFocusAndSelect(HWND hDlg, int nID)
{
	HWND hWnd = GetDlgItem(hDlg, nID);
	SetFocus(hWnd);
	SendMessage(hWnd, EM_SETSEL, 0, -1);
}

static BOOL getDlgInt(HWND hDlg, int nID, int *result)
{
	BOOL translated;
	UINT r = GetDlgItemInt(hDlg, nID, &translated, FALSE);
	if (!translated) {
		MessageBox(hDlg, _T("Invalid number"), _T("Error"), MB_OK | MB_ICONERROR);
		return FALSE;
	}
	*result = (int) r;
	return TRUE;
}

static INT_PTR CALLBACK settingsDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int i;
	switch (uMsg) {
	case WM_INITDIALOG:
		if (song_length <= 0) {
			CheckRadioButton(hDlg, IDC_UNLIMITED, IDC_LIMITED, IDC_UNLIMITED);
			SetDlgItemInt(hDlg, IDC_MINUTES, DEFAULT_SONG_LENGTH / 60, FALSE);
			SetDlgItemInt(hDlg, IDC_SECONDS, DEFAULT_SONG_LENGTH % 60, FALSE);
			enableTimeInput(hDlg, FALSE);
		}
		else {
			CheckRadioButton(hDlg, IDC_UNLIMITED, IDC_LIMITED, IDC_LIMITED);
			SetDlgItemInt(hDlg, IDC_MINUTES, (UINT) song_length / 60, FALSE);
			SetDlgItemInt(hDlg, IDC_SECONDS, (UINT) song_length % 60, FALSE);
			enableTimeInput(hDlg, TRUE);
		}
		if (silence_seconds <= 0) {
			CheckDlgButton(hDlg, IDC_SILENCE, BST_UNCHECKED);
			SetDlgItemInt(hDlg, IDC_SILSECONDS, DEFAULT_SILENCE_SECONDS, FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_SILSECONDS), FALSE);
		}
		else {
			CheckDlgButton(hDlg, IDC_SILENCE, BST_CHECKED);
			SetDlgItemInt(hDlg, IDC_SILSECONDS, (UINT) silence_seconds, FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_SILSECONDS), TRUE);
		}
		CheckRadioButton(hDlg, IDC_LOOPS, IDC_NOLOOPS, play_loops ? IDC_LOOPS : IDC_NOLOOPS);
		saved_mute_mask = mute_mask;
		for (i = 0; i < 8; i++)
			CheckDlgButton(hDlg, IDC_MUTE1 + i, ((mute_mask >> i) & 1) != 0 ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;
	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			WORD wCtrl = LOWORD(wParam);
			BOOL enabled;
			switch (wCtrl) {
			case IDC_UNLIMITED:
			case IDC_LIMITED:
				enabled = (wCtrl == IDC_LIMITED);
				enableTimeInput(hDlg, enabled);
				if (enabled)
					setFocusAndSelect(hDlg, IDC_MINUTES);
				return TRUE;
			case IDC_SILENCE:
				enabled = (IsDlgButtonChecked(hDlg, IDC_SILENCE) == BST_CHECKED);
				EnableWindow(GetDlgItem(hDlg, IDC_SILSECONDS), enabled);
				if (enabled)
					setFocusAndSelect(hDlg, IDC_SILSECONDS);
				return TRUE;
			case IDC_LOOPS:
			case IDC_NOLOOPS:
				return TRUE;
			case IDC_MUTE1:
			case IDC_MUTE1 + 1:
			case IDC_MUTE1 + 2:
			case IDC_MUTE1 + 3:
			case IDC_MUTE1 + 4:
			case IDC_MUTE1 + 5:
			case IDC_MUTE1 + 6:
			case IDC_MUTE1 + 7:
				i = 1 << (wCtrl - IDC_MUTE1);
				if (IsDlgButtonChecked(hDlg, wCtrl) == BST_CHECKED)
					mute_mask |= i;
				else
					mute_mask &= ~i;
				ASAP_MutePokeyChannels(&asap, mute_mask);
				return TRUE;
			case IDOK:
			{
				int new_song_length;
				if (IsDlgButtonChecked(hDlg, IDC_UNLIMITED) == BST_CHECKED)
					new_song_length = -1;
				else {
					int minutes;
					int seconds;
					if (!getDlgInt(hDlg, IDC_MINUTES, &minutes)
					 || !getDlgInt(hDlg, IDC_SECONDS, &seconds))
						return TRUE;
					new_song_length = 60 * minutes + seconds;
				}
				if (IsDlgButtonChecked(hDlg, IDC_SILENCE) != BST_CHECKED)
					silence_seconds = -1;
				else if (!getDlgInt(hDlg, IDC_SILSECONDS, &silence_seconds))
					return TRUE;
				song_length = new_song_length;
				play_loops = (IsDlgButtonChecked(hDlg, IDC_LOOPS) == BST_CHECKED);
			}
				EndDialog(hDlg, IDOK);
				return TRUE;
			case IDCANCEL:
				mute_mask = saved_mute_mask;
				ASAP_MutePokeyChannels(&asap, mute_mask);
				EndDialog(hDlg, IDCANCEL);
				return TRUE;
			}
		}
		break;
	default:
		break;
	}
	return FALSE;
}

BOOL settingsDialog(HINSTANCE hInstance, HWND hwndParent)
{
	return DialogBox(hInstance, MAKEINTRESOURCE(IDD_SETTINGS), hwndParent, settingsDialogProc) == IDOK;
}

int getSongDuration(const ASAP_ModuleInfo *module_info, int song)
{
	int duration = module_info->durations[song];
	if (duration < 0)
		return 1000 * song_length;
	if (play_loops && module_info->loops[song])
		return 1000 * song_length;
	return duration;
}

int playSong(int song)
{
	int duration = asap.module_info.durations[song];
	if (duration < 0) {
		if (silence_seconds > 0)
			ASAP_DetectSilence(&asap, silence_seconds);
		duration = 1000 * song_length;
	}
	if (play_loops && asap.module_info.loops[song])
		duration = 1000 * song_length;
	ASAP_PlaySong(&asap, song, duration);
	ASAP_MutePokeyChannels(&asap, mute_mask);
	return duration;
}

#endif /* WASAP */

#if defined(WASAP) || defined(WINAMP)

static HWND infoDialog = NULL;

static BOOL saveInfo(void)
{
	char filename[MAX_PATH];
	HANDLE fh;
	byte module[ASAP_MODULE_MAX];
	DWORD module_len;
	ASAP_ModuleInfo module_info;
	byte out_module[ASAP_MODULE_MAX];
	int out_len;
	SendDlgItemMessage(infoDialog, IDC_FILENAME, WM_GETTEXT, MAX_PATH, (LPARAM) filename);
	fh = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING,
	                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (fh == INVALID_HANDLE_VALUE)
		return FALSE;
	if (!ReadFile(fh, module, sizeof(module), &module_len, NULL)) {
		CloseHandle(fh);
		return FALSE;
	}
	CloseHandle(fh);
	SendDlgItemMessage(infoDialog, IDC_AUTHOR, WM_GETTEXT, sizeof(module_info.author), (LPARAM) module_info.author);
	SendDlgItemMessage(infoDialog, IDC_NAME, WM_GETTEXT, sizeof(module_info.name), (LPARAM) module_info.name);
	SendDlgItemMessage(infoDialog, IDC_DATE, WM_GETTEXT, sizeof(module_info.date), (LPARAM) module_info.date);
	out_len = ASAP_SetModuleInfo(&module_info, module, module_len, out_module);
	if (out_len <= 0)
		return FALSE;
	fh = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fh == INVALID_HANDLE_VALUE)
		return FALSE;
	if (!WriteFile(fh, out_module, out_len, &module_len, NULL)) {
		CloseHandle(fh);
		return FALSE;
	}
	CloseHandle(fh);
	return TRUE;
}

static INT_PTR CALLBACK infoDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef WINAMP
		CheckDlgButton(hDlg, IDC_PLAYING, playing_info ? BST_CHECKED : BST_UNCHECKED);
#endif
		SendDlgItemMessage(hDlg, IDC_AUTHOR, EM_LIMITTEXT, 127, 0);
		SendDlgItemMessage(hDlg, IDC_NAME, EM_LIMITTEXT, 127, 0);
		SendDlgItemMessage(hDlg, IDC_DATE, EM_LIMITTEXT, 127, 0);
		return TRUE;
	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			switch (LOWORD(wParam)) {
#ifdef WINAMP
			case IDC_PLAYING:
				playing_info = (IsDlgButtonChecked(hDlg, IDC_PLAYING) == BST_CHECKED);
				if (playing_info)
					updateInfoDialog(current_filename, &asap.module_info);
				return TRUE;
#endif
			case IDC_SAVE:
				if (!saveInfo())
					MessageBox(hDlg, "Cannot save information to file", "Error", MB_OK | MB_ICONERROR);
				return TRUE;
			case IDCANCEL:
				DestroyWindow(hDlg);
				infoDialog = NULL;
				return TRUE;
			}
		}
		break;
	default:
		break;
	}
	return FALSE;
}

void showInfoDialog(HINSTANCE hInstance, HWND hwndParent,
                    const char *filename, const ASAP_ModuleInfo *module_info)
{
	if (infoDialog == NULL)
		infoDialog = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_INFO), hwndParent, infoDialogProc);
	updateInfoDialog(filename, module_info);
}

void updateInfoDialog(const char *filename, const ASAP_ModuleInfo *module_info)
{
	abool can_edit;
	const char *author;
	const char *name;
	const char *date;
	if (infoDialog == NULL)
		return;
	can_edit = filename != NULL && ASAP_CanSetModuleInfo(filename);
	SendDlgItemMessage(infoDialog, IDC_AUTHOR, EM_SETREADONLY, !can_edit, 0);
	SendDlgItemMessage(infoDialog, IDC_NAME, EM_SETREADONLY, !can_edit, 0);
	SendDlgItemMessage(infoDialog, IDC_DATE, EM_SETREADONLY, !can_edit, 0);
	EnableWindow(GetDlgItem(infoDialog, IDC_SAVE), can_edit);
	if (module_info == NULL) {
		author = "";
		name = "";
		date = "";
	}
	else {
		author = module_info->author;
		name = module_info->name;
		date = module_info->date;
	}
	SendDlgItemMessage(infoDialog, IDC_FILENAME, WM_SETTEXT, 0, (LPARAM) filename);
	SendDlgItemMessage(infoDialog, IDC_AUTHOR, WM_SETTEXT, 0, (LPARAM) author);
	SendDlgItemMessage(infoDialog, IDC_NAME, WM_SETTEXT, 0, (LPARAM) name);
	SendDlgItemMessage(infoDialog, IDC_DATE, WM_SETTEXT, 0, (LPARAM) date);
}

#endif
