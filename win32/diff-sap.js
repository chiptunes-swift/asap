// wscript.exe "PATH\TO\diff-sap.js" %base %mine %bname %yname //E:javascript

// Get command-line arguments

var argc = WScript.Arguments.length;
if (argc != 2 && argc != 4) {
	WScript.Echo("Specify two filenames and optionally two titles");
	WScript.Quit(1);
}
var sap1 = WScript.Arguments(0);
var sap2 = WScript.Arguments(1);
var title1 = WScript.Arguments(argc - 2);
var title2 = WScript.Arguments(argc - 1);

// Find TortoiseMerge

var wsh = new ActiveXObject("WScript.Shell");
var tmerge;
try {
	tmerge = wsh.RegRead("HKLM\\SOFTWARE\\TortoiseSVN\\TMergePath");
}
catch (e) {
	try {
		tmerge = wsh.RegRead("HKLM\\SOFTWARE\\TortoiseGit\\TMergePath");
	}
	catch (e) {
		WScript.Echo("TortoiseMerge not found!");
		WScript.Quit(1);
	}
}

// Convert SAPs to plain text

var fso = new ActiveXObject("Scripting.FileSystemObject");
var exe = fso.GetParentFolderName(WScript.ScriptFullName);
exe = fso.BuildPath(exe, "sap2txt.exe");

function sap2txt(sap)
{
	var txt = fso.BuildPath(fso.GetSpecialFolder(2), fso.GetTempName());
	wsh.Run("cmd /c \"\"" + exe + "\" \"" + sap + "\" >\"" + txt + "\"\"", 0, true);
	return txt;
}

var txt1 = sap2txt(sap1);
var txt2 = sap2txt(sap2);

// Display results

wsh.Run("\"" + tmerge + "\" /base:\"" + txt1 + "\" /mine:\"" + txt2 + "\" /basename:\"" + title1 + "\" /minename:\"" + title2 + "\"", 4, true);

// Write back changes

wsh.Run("\"" + exe +"\" \"" + txt2 + "\" \"" + sap2 + "\"", 0, true);

// Delete temporary files when done

fso.DeleteFile(txt2);
fso.DeleteFile(txt1);
