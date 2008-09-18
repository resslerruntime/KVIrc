//=============================================================================
//
//   File : libkvifile.cpp
//   Creation date : Fri Nov  9 03:27:59 2001 GMT by Szymon Stefanek
//
//   This file is part of the KVirc irc client distribution
//   Copyright (C) 2001-2008 Szymon Stefanek (pragma at kvirc dot net)
//
//   This program is FREE software. You can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your opinion) any later version.
//
//   This program is distributed in the HOPE that it will be USEFUL,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program. If not, write to the Free Software Foundation,
//   Inc. ,59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//=============================================================================

#include "kvi_module.h"
#include "kvi_fileutils.h"
#include "kvi_locale.h"
#include "kvi_malloc.h"
#include "kvi_app.h"
#include "kvi_options.h"
#include "kvi_qcstring.h"
#include "kvi_kvs_arraycast.h"
#include "kvi_file.h"

#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QTextCodec>

/*
	@doc: file.copy
	@type:
		command
	@title:
		file.copy
	@keyterms:
		copying files
	@short:
		Makes a copy of a file
	@syntax:
		file.copy [-o] <source:string> <destination:string>
	@switches:
		!sw: -o | --overwrite
		Overwrites the file even if it already exists.
	@description:
		Makes a copy of the <source> file as <destination>.[br]
		If the [-o] switch is used , the <destination> file is overwritten , if already exists.[br]
		With no [-o] switch , this command does not overwrite files.[br]
		The destination path must be already existing: if you want to ensure this, use [fnc]$file.mkdir[/fnc] first.[br]
		The paths (<source> and <destination>) are adjusted according to the system that KVIrc
		is running on so you don't have to bother about portability: it *should* be automatically
		guaranteed. Just use UNIX style paths for them.[br]
	@seealso:
		[cmd]file.rename[/cmd], [fnc]$file.exists[/fnc]
*/

static bool file_kvs_cmd_copy(KviKvsModuleCommandCall * c)
{
	QString szSrc,szDst;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("source",KVS_PT_NONEMPTYSTRING,0,szSrc)
		KVSM_PARAMETER("destination",KVS_PT_NONEMPTYSTRING,0,szDst)
	KVSM_PARAMETERS_END(c)
	KviFileUtils::adjustFilePath(szSrc);
	KviFileUtils::adjustFilePath(szDst);
	if(KviFileUtils::fileExists(szDst) && !c->switches()->find('o',"overwrite"))
	{
		c->warning(__tr2qs("Destinaion file exists: no copy made"));
		return true;
	}
	if(!KviFileUtils::copyFile(szSrc,szDst))
	{
		c->warning(__tr2qs("Failed to copy from %Q to %Q"),&szSrc,&szDst);
		c->warning(__tr2qs("Either the source doesn't exist or the destination can not be created"));
	}
	return true;
}


/*
	@doc: file.addimagepath
	@type:
		command
	@title:
		file.addimagepath
	@short:
		Adds an image search path
	@syntax:
		file.addimagepath <path:string>
	@description:
		Adds <path> to the image search path list.[br]
		KVIrc will look for images files (also) in that path (when a relative image file name is used).[br]
		Yes, this function is a bit misplaced...
	@seealso:
		[cmd]file.delimagepath[/cmd]
*/

static bool file_kvs_cmd_addimagepath(KviKvsModuleCommandCall * c)
{
	QString szDst;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("path",KVS_PT_NONEMPTYSTRING,0,szDst)
	KVSM_PARAMETERS_END(c)
	KviFileUtils::adjustFilePath(szDst);

	//QStringList::Iterator it = KVI_OPTION_STRINGLIST(KviOption_stringlistImageSearchPaths).find(szDst);
	int index=KVI_OPTION_STRINGLIST(KviOption_stringlistImageSearchPaths).indexOf(szDst);
	//if(it == KVI_OPTION_STRINGLIST(KviOption_stringlistImageSearchPaths).end())
	if (index==-1)
		KVI_OPTION_STRINGLIST(KviOption_stringlistImageSearchPaths).append(szDst);
	return true;
}



/*
	@doc: file.delimagepath
	@type:
		command
	@title:
		file.delimagepath
	@short:
		Removes an image search path
	@syntax:
		file.delimagepath <path:string>
	@description:
		Removes <path> from the image search path list.[br]
		Yes, this function is a bit misplaced...
	@seealso:
		[cmd]file.addimagepath[/cmd]
*/

static bool file_kvs_cmd_delimagepath(KviKvsModuleCommandCall * c)
{
	QString szDst;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("path",KVS_PT_NONEMPTYSTRING,0,szDst)
	KVSM_PARAMETERS_END(c)
	KviFileUtils::adjustFilePath(szDst);

	//QStringList::Iterator it = KVI_OPTION_STRINGLIST(KviOption_stringlistImageSearchPaths).find(szDst);
	int index=KVI_OPTION_STRINGLIST(KviOption_stringlistImageSearchPaths).indexOf(szDst);
	//if(it == KVI_OPTION_STRINGLIST(KviOption_stringlistImageSearchPaths).end())
	if(index!=-1)
		KVI_OPTION_STRINGLIST(KviOption_stringlistImageSearchPaths).removeAt(index);
	return true;
}


/*
	@doc: file.write
	@type:
		command
	@title:
		file.write
	@keyterms:
		writing data to files
	@short:
		Writes an ascii data string to a file
	@syntax:
		file.write [-a] [-l] <filename:string> <data:string>
	@switches:
		!sw: -a | --append
		If the file already exists, append the data instead of overwriting the original contents.
		!sw: -l | --local-8-bit
		Causes the file to be written in the local 8 bit character set instead of the
		default utf8.
	@description:
		Writes <data> (which is an ASCII string) to the file <filename>.[br]
		It does NOT append a traling LF character: if you want it you must explicitly specify it in the <data> parameter.[br]
		-a causes the command to append the <data> to the file instead of overwriting the entire file.[br]
		The path is adjusted according to the system that KVIrc
		is running on so you don't have to bother about portability: it *should* be automatically
		guaranteed. Just use UNIX style paths for them.[br]
		The file is saved in utf8 unless the -l switch is specified (in that case the local 8 bit encoding is used).
		Please note that uf8 is the only character set that preserves ALL the possible characters.
	@seealso:
		[cmd]file.rename[/cmd], [fnc]$file.exists[/fnc]
*/

static bool file_kvs_cmd_write(KviKvsModuleCommandCall * c)
{
	QString szFileName,szData;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("filename",KVS_PT_NONEMPTYSTRING,0,szFileName)
		KVSM_PARAMETER("data",KVS_PT_NONEMPTYSTRING,0,szData)
	KVSM_PARAMETERS_END(c)
	KviFileUtils::adjustFilePath(szFileName);

	bool bRet;
	if(c->switches()->find('l',"local-8-bit"))
		bRet = KviFileUtils::writeFileLocal8Bit(szFileName,szData,c->switches()->find('a',"append"));
	else
		bRet = KviFileUtils::writeFile(szFileName,szData,c->switches()->find('a',"append"));

	if(!bRet)
		c->warning(__tr2qs("Failed to write to file %Q: the destination couldn't be opened"),&szFileName);

	return true;
}


/*
	@doc: file.rename
	@type:
		command
	@title:
		file.rename
	@keyterms:
		copying files
	@short:
		Makes a copy of a file
	@syntax:
		file.rename <oldname:string> <newname:string>
	@description:
		Renames a file from <oldname> to <newname>.[br]
		This command can also rename directories.[br]
		If the <newname> file already exists , this command fails.[br]
		The paths (<oldname> and <newname>) are adjusted according to the system that KVIrc
		is running on so you don't have to bother about portability: it *should* be automatically
		guaranteed.Just use UNIX style paths for them.[br]
	@seealso:
		[cmd]file.copy[/cmd], [fnc]$file.exists[/fnc]
*/

static bool file_kvs_cmd_rename(KviKvsModuleCommandCall * c)
{
	QString szOld,szNew;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("oldname",KVS_PT_NONEMPTYSTRING,0,szOld)
		KVSM_PARAMETER("newname",KVS_PT_NONEMPTYSTRING,0,szNew)
	KVSM_PARAMETERS_END(c)

	KviFileUtils::adjustFilePath(szOld);
	KviFileUtils::adjustFilePath(szNew);

	if(KviFileUtils::fileExists(szNew))
		c->warning(__tr2qs("Destination file exists: file not renamed"));

	if(!KviFileUtils::renameFile(szOld,szNew))
		c->warning(__tr2qs("Failed to rename %Q to %Q"),&szOld,&szNew);
	return true;
}


/*
	@doc: file.mkdir
	@type:
		command
	@title:
		file.mkdir
	@keyterms:
		creating directories
	@short:
		Creates a directory
	@syntax:
		file.mkdir <directory:string>
	@description:
		Creates the <directory>.[br]
		The path is adjusted according to the system that KVIrc
		is running on so you don't have to bother about portability: it *should* be automatically
		guaranteed. Just use an UNIX style path.[br]
	@seealso:
		[fnc]$file.exists[/fnc]
*/

static bool file_kvs_cmd_mkdir(KviKvsModuleCommandCall * c)
{
	QString szDir;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("directory",KVS_PT_NONEMPTYSTRING,0,szDir)
	KVSM_PARAMETERS_END(c)
	KviFileUtils::adjustFilePath(szDir);
	if(!KviFileUtils::makeDir(szDir))
		c->warning(__tr2qs("Failed to make the directory %Q"),&szDir);
	return true;
}



/*
	@doc: file.remove
	@type:
		command
	@title:
		file.remove
	@keyterms:
		removing files
	@short:
		Removes a file
	@syntax:
		file.remove [-q] <name:string>
	@switches:
		!sw: -q | --quiet
		Suppresses any warning message
	@description:
		Removes the file <name>.[br]
		-q suppresses any warning message (about non existing file , for example).[br]
		The path is adjusted according to the system that KVIrc
		is running on so you don't have to bother about portability: it *should* be automatically
		guaranteed. Just use an UNIX style path.[br]
	@seealso:
		[fnc]$file.exists[/fnc]
*/

static bool file_kvs_cmd_remove(KviKvsModuleCommandCall * c)
{
	QString szName;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("name",KVS_PT_NONEMPTYSTRING,0,szName)
	KVSM_PARAMETERS_END(c)
	KviFileUtils::adjustFilePath(szName);
	if(!KviFileUtils::removeFile(szName))
	{
		if(!c->switches()->find('q',"quiet"))
			c->warning(__tr2qs("Failed to remove the file %Q"),&szName);
	}
	return true;
}


/*
	@doc: file.rmdir
	@type:
		command
	@title:
		file.rmdir
	@keyterms:
		removing directories
	@short:
		Removes a directory
	@syntax:
		file.rmdir [-q] <name:string>
	@switches:
		!sw: -q | --quiet
		Suppresses any warning message
	@description:
		Removes the directory <name>.[br]
		The directory must be empty for this command to success.[br]
		-q suppresses any warning message (about non existing directory , for example).[br]
		The path is adjusted according to the system that KVIrc
		is running on so you don't have to bother about portability: it *should* be automatically
		guaranteed. Just use an UNIX style path.[br]
	@seealso:
		[fnc]$file.exists[/fnc]
		[cmd]file.remove[/cmd]
*/

static bool file_kvs_cmd_rmdir(KviKvsModuleCommandCall * c)
{
	QString szName;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("name",KVS_PT_NONEMPTYSTRING,0,szName)
	KVSM_PARAMETERS_END(c)
	KviFileUtils::adjustFilePath(szName);
	if(!KviFileUtils::removeDir(szName))
	{
		if(!c->switches()->find('q',"quiet"))
			c->warning(__tr2qs("Failed to remove the directory %Q"),&szName);
	}
	return true;
}


/*
	@doc: file.exists
	@type:
		function
	@title:
		$file.exists
	@short:
		Check if a file exists
	@syntax:
		<boolean> $file.exists(<filename:string>)
	@description:
		Returns true if the file <filename> exists (this is also valid for directories!).[br]
		The <filename> should be an unix-style file path and is adjusted according to the system that KVIrc is running on.[br]
*/

static bool file_kvs_fnc_exists(KviKvsModuleFunctionCall * c)
{
	QString szName;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("filename",KVS_PT_STRING,0,szName)
	KVSM_PARAMETERS_END(c)
	KviFileUtils::adjustFilePath(szName);
	QFileInfo f(szName);
	c->returnValue()->setBoolean(f.exists());
	return true;
}


/*
	@doc: file.type
	@type:
		function
	@title:
		$file.type
	@short:
		Checks the type of a path
	@syntax:
		<string> $file.type(<filename:string>)
	@description:
		Returns "f" if the <filename> points to a real file , "d" if <filename>
		is the name of a directory or "l" if it is a symbolic link.[br]
		The <filename> should be an unix-style file path and is adjusted according to the system that KVIrc is running on.[br]
*/

static bool file_kvs_fnc_type(KviKvsModuleFunctionCall * c)
{
	QString szName;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("filename",KVS_PT_NONEMPTYSTRING,0,szName)
	KVSM_PARAMETERS_END(c)
	KviFileUtils::adjustFilePath(szName);
	QFileInfo f(szName);
	if(f.isFile())c->returnValue()->setString("f");
	else if(f.isDir())c->returnValue()->setString("d");
	else if(f.isSymLink())c->returnValue()->setString("l");
	return true;
}


/*
	@doc: file.size
	@type:
		function
	@title:
		$file.size
	@short:
		Returns the size of a file
	@syntax:
		<integer> $file.size(<filename:string>)
	@description:
		Returns the size of the file pointed by <filename>.[br]
		If the file does not exist , this function returns 0.[br]
		The <filename> should be an unix-style file path and is adjusted according to the system that KVIrc is running on.[br]
*/

static bool file_kvs_fnc_size(KviKvsModuleFunctionCall * c)
{
	QString szName;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("filename",KVS_PT_NONEMPTYSTRING,0,szName)
	KVSM_PARAMETERS_END(c)
	KviFileUtils::adjustFilePath(szName);
	QFileInfo f(szName);
	c->returnValue()->setInteger(f.size());
	return true;
}
/*
	@doc: file.allSizese
	@type:
		function
	@title:
		$file.allSizes
	@short:
		Returns all sizes of a specified directory.
	@syntax:
	<array> $file.allSize(<dirname:string>)
	@description:
	Returns the size of every files of the specified directory as an array.[br]
*/

static bool file_kvs_fnc_allSizes(KviKvsModuleFunctionCall * c)
{
	QString szDir;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("directory",KVS_PT_NONEMPTYSTRING,0,szDir)
	KVSM_PARAMETERS_END(c)
	KviFileUtils::adjustFilePath(szDir);
	
	QDir d(szDir);
	if(!d.exists())
	{
		c->warning(__tr2qs("The specified directory does not exist '%Q'"),&szDir);
		return true;
	}

	QStringList sl;
	sl = d.entryList(QDir::Files);

	KviKvsArray * a = new KviKvsArray();
	QString szFile;
	if(!sl.isEmpty())
	{
		int idx = 0;
		for(QStringList::Iterator it = sl.begin();it != sl.end();++it)
		{
			szFile=szDir+(*it);
			QFileInfo f(szFile);
			a->set(idx,new KviKvsVariant((kvs_int_t)f.size()));
			idx++;
		}
	}
	c->returnValue()->setArray(a);

	return true;
}

/*
	@doc: file.fixpath
	@type:
		function
	@title:
		$file.fixpath
	@short:
		Converts file paths
	@syntax:
		<string> $file.fixpath(<filename:string>)
	@description:
		Returns the <filename> adjusted to match the current operating
		system file path conventions.[br] This means that on UNIX , a path like "C:\folder\file.mp3"
		will be returned as "/folder/file.mp3" and vice-versa.[br]
		There is a minor problem with unix paths converted to the windows system: unix
		has no "drive" concept thus the unix paths do not contain a drive. KVIrc will
		always map the paths to the "default" C: drive.[br]
		This is a good reason to avoid using absolute hard-coded paths :).[br]
		Please note that you DON'T NEED to call this function on paths that you
		pass to the other file.* functions: they are adjusted automatically.[br]
	@seealso:
		[fnc]$file.ps[/fnc]
*/


static bool file_kvs_fnc_fixpath(KviKvsModuleFunctionCall * c)
{
	QString szName;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("filename",KVS_PT_STRING,0,szName)
	KVSM_PARAMETERS_END(c)
	KviFileUtils::adjustFilePath(szName);
	c->returnValue()->setString(szName);
	return true;
}


/*
	@doc: file.ps
	@type:
		function
	@title:
		$file.ps
	@short:
		Returns the file path separator
	@syntax:
		<string> $file.ps()
	@description:
		Returns the file path separator for the current operating system.[br]
		On windows , '\' is returned , on UNIX , '/'.[br]
	@seealso:
		[fnc]$file.fixpath[/fnc]
*/

static bool file_kvs_fnc_ps(KviKvsModuleFunctionCall * c)
{
	c->returnValue()->setString(QString(QChar(KVI_PATH_SEPARATOR_CHAR)));
	return true;
}


/*
	@doc: file.ls
	@type:
		function
	@title:
		$file.ls
	@short:
		Returns a directory listing
	@syntax:
		<array> $file.ls(<directory:string>,[<flags:string>[,<namefilter:string>]])
	@description:
		Returns the listing of the specified directory as an array.[br]
		The <directory> should be an unix-style file path and is adjusted according to the system that KVIrc is running on.[br]
		<flags> may be a combination of the following characters:[br]
		d: list directories[br]
		f: list files[br]
		l: list symbolic links[br]
		r: list readable files[br]
		w: list writeable files[br]
		x: list executable files[br]
		h: list hidden files[br]
		s: list system files[br]
		n: sort files by name[br]
		t: sort files by file time[br]
		b: sort files by file size[br]
		z: put the directories first, then the files[br]
		k: invert sort order[br]
		i: case insensitive sort[br]
		If <flags> is empty then a default of 'dfrwxhs'.[br]
		If none of r,w,x is set then KVIrc sets all of them by default.[br]
		If <namefilter> is passed then it is interpreted as a wildcard string
		that must match the entries to be returned.[br]
	@example:
		[example]
			%dir[]=$file.ls(/,"d")
			[cmd]foreach[/cmd](%f,%dir[])echo %f
			%dir[]=$file.ls(/usr/include,"f","t*.h");
			[cmd]foreach[/cmd](%f,%dir[])echo %f
			%dir[]=$file.ls($file.homedir,"dfr");
			[cmd]foreach[/cmd](%f,%dir[])echo %f
		[/example]
*/

static bool file_kvs_fnc_ls(KviKvsModuleFunctionCall * c)
{
	QString szDir,szFlags,szFilter;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("directory",KVS_PT_NONEMPTYSTRING,0,szDir)
		KVSM_PARAMETER("flags",KVS_PT_STRING,KVS_PF_OPTIONAL,szFlags)
		KVSM_PARAMETER("filter",KVS_PT_STRING,KVS_PF_OPTIONAL,szFilter)
	KVSM_PARAMETERS_END(c)

	KviFileUtils::adjustFilePath(szDir);

	QDir d(szDir);
	if(!d.exists())
	{
		c->warning(__tr2qs("The specified directory does not exist '%Q'"),&szDir);
		return true;
	}

	QFlags<QDir::Filter> iFlags = 0;
	if(szFlags.isEmpty())iFlags = QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::Readable | QDir::Writable | QDir::Executable | QDir::Hidden | QDir::System;
	else {
		if(szFlags.indexOf('d',Qt::CaseInsensitive) != -1)iFlags |= QDir::Dirs;
		if(szFlags.indexOf('f',Qt::CaseInsensitive) != -1)iFlags |= QDir::Files;
		if(szFlags.indexOf('l',Qt::CaseInsensitive) == -1)iFlags |= QDir::NoSymLinks;
		if(szFlags.indexOf('r',Qt::CaseInsensitive) != -1)iFlags |= QDir::Readable;
		if(szFlags.indexOf('w',Qt::CaseInsensitive) != -1)iFlags |= QDir::Writable;
		if(szFlags.indexOf('x',Qt::CaseInsensitive) != -1)iFlags |= QDir::Executable;
		if(szFlags.indexOf('h',Qt::CaseInsensitive) != -1)iFlags |= QDir::Hidden;
		if(szFlags.indexOf('s',Qt::CaseInsensitive) != -1)iFlags |= QDir::System;
	}

	QFlags<QDir::SortFlag> iSort = 0;
	if(szFlags.isEmpty())iSort = QDir::Unsorted;
	else {
		if(szFlags.indexOf('n',Qt::CaseInsensitive) != -1)iSort |= QDir::Name;
		if(szFlags.indexOf('t',Qt::CaseInsensitive) != -1)iSort |= QDir::Time;
		if(szFlags.indexOf('b',Qt::CaseInsensitive) != -1)iSort |= QDir::Size;
		if(szFlags.indexOf('z',Qt::CaseInsensitive) != -1)iSort |= QDir::DirsFirst;
		if(szFlags.indexOf('k',Qt::CaseInsensitive) != -1)iSort |= QDir::Reversed;
		if(szFlags.indexOf('i',Qt::CaseInsensitive) != -1)iSort |= QDir::IgnoreCase;
	}

	QStringList sl;
	if(!szFilter.isEmpty())sl = d.entryList(QStringList(szFilter),iFlags,iSort);
	else sl = d.entryList(iFlags,iSort);

	KviKvsArray * a = new KviKvsArray();
	if(!sl.isEmpty())
	{
		int idx = 0;
		for(QStringList::Iterator it = sl.begin();it != sl.end();++it)
		{
			a->set(idx,new KviKvsVariant(*it));
			idx++;
		}
	}
	c->returnValue()->setArray(a);

	return true;
}

/*
	@doc: file.read
	@type:
		function
	@title:
		$file.read
	@short:
		Reads a text file
	@syntax:
		<string> $file.read(<filename:string>[,<size:integer>[,<flags:string>]])
	@description:
		Reads at most <size> bytes of the file pointed by <filename>.[br]
		<size> is an upper limit but may be not reached if the real file is smaller.[br]
		The data read is returned as a string , so if the file contains binary data,
		expect strange results.[br] If <size> is not specified, then KVIrc tries to read
		the whole file up to the 1 MB limit (so if you want to read a file that is
		bigger thatn 1 MB then you MUST specify the <size>).[br]
		WARNING: always check the file size before attemting to read a whole file...
		reading a CDROM iso image may sit down your system :) (and will prolly crash while
		allocating memory , before attempting to read anything)[br]
		An empty string is returned if a serious error occures.[br]
		The <filename> is adjusted according to the system that KVIrc is running on.[br]
		Flags are actually limited to the single letter 'l'. By default the file
		is decoded from the ut8 characters set. If 'l' is present the the file
		is decoded by using the local 8 bit character set instead.
	@examples:
		[example]
			echo $file.read(/proc/cpuinfo)
		[/example]
	@seealso:
		[fnc]$file.readbinary[/fnc]
*/

static bool file_kvs_fnc_read(KviKvsModuleFunctionCall * c)
{
	QString szNameZ;
	kvs_uint_t uSize;
	QString szFlags;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("filename",KVS_PT_NONEMPTYSTRING,0,szNameZ)
		KVSM_PARAMETER("size",KVS_PT_UINT,KVS_PF_OPTIONAL,uSize)
		KVSM_PARAMETER("flags",KVS_PT_STRING,KVS_PF_OPTIONAL,szFlags)
	KVSM_PARAMETERS_END(c)
	KviFileUtils::adjustFilePath(szNameZ);

	QFile f(szNameZ);
	if(!f.open(QIODevice::ReadOnly))
	{
		c->warning(__tr2qs("Can't open the file \"%Q\" for reading"),&szNameZ);
		return true;
	}

	if(c->params()->count() < 2)uSize = 1024 * 1024; // 1 meg file default

	char * buf = (char *)kvi_malloc(sizeof(char) * (uSize + 1));
	unsigned int uReaded = 0;
	unsigned int uRetries = 0;

	while((uReaded < uSize) && (!f.atEnd()))
	{
		int readedNow = f.read(buf + uReaded,uSize - uReaded);
		if(readedNow < 0)
		{
			kvi_free(buf);
			c->warning(__tr2qs("Read error for file %Q"),&szNameZ);
			return true;
		} else readedNow += uReaded;
		uRetries ++;
		if(uRetries > 1000)
		{
			// ops
			kvi_free(buf);
			c->warning(__tr2qs("Read error for file %Q (have been unable to read the requested size in 1000 retries)"),&szNameZ);
			return true;
		}
		uReaded += readedNow;
	}

	buf[uReaded] = '\0';

	if(szFlags.indexOf('l',Qt::CaseInsensitive) == -1)
		c->returnValue()->setString(QString::fromUtf8(buf));
	else
		c->returnValue()->setString(QString::fromLocal8Bit(buf));

	kvi_free(buf);

	return true;
}


/*
	@doc: file.readLines
	@type:
		function
	@title:
		$file.readLines
	@short:
		Reads lines of a text file
	@syntax:
		<array> $file.readLines(<filename:string>[,<startline:integer>,[<count:integer>[,<flags:string>]]])
	@description:
		Reads lines from the specified file and returns them as an array of strings.
		The lines are assumed to be separated by linefeed characters (which are NOT returned).
		Eventual terminating carriage return characters at the end of the line are stripped.
		If <startline> is specified, then all the lines with indexes lower that <startline> are
		discarded. If <count> is specified then a maximum of <count> lines is returned.
		If <count> is not specified then all the lines until the end are read.
		The <filename> is adjusted according to the system that KVIrc is running on.[br]
		Flags are actually limited to the single letter 'l'. By default the file
		is decoded from the ut8 characters set. If 'l' is present the the file
		is decoded by using the local 8 bit character set instead.
		WARNING: Always check the size of the file you're going to read: it is not
		a good idea attempting to read a 700 MB binary file with this function since
		it will probably sit down your system and exhaust your virtual memory.
	@examples:
		[example]
			echo $file.readLines(/proc/cpuinfo)
		[/example]
	@seealso:
		[fnc]$file.read[/fnc], [cmd]file.writeLines[/cmd], [fnc]$lf[/fnc]
*/

static bool file_kvs_fnc_readLines(KviKvsModuleFunctionCall * c)
{
	QString szName;
	QString szFlags;
	kvs_int_t iStartLine;
	kvs_int_t iCount;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("filename",KVS_PT_NONEMPTYSTRING,0,szName)
		KVSM_PARAMETER("startline",KVS_PT_INT,KVS_PF_OPTIONAL,iStartLine)
		KVSM_PARAMETER("count",KVS_PT_INT,KVS_PF_OPTIONAL,iCount)
		KVSM_PARAMETER("flags",KVS_PT_STRING,KVS_PF_OPTIONAL,szFlags)
	KVSM_PARAMETERS_END(c)
	KviFileUtils::adjustFilePath(szName);

	QFile f(szName);
	if(!f.open(QIODevice::ReadOnly))
	{
		c->warning(__tr2qs("Can't open the file \"%Q\" for reading"),&szName);
		return true;
	}

	if(c->params()->count() < 2)iStartLine = 0;
	if(c->params()->count() < 3)iCount = -1;

	bool bLocal8Bit = szFlags.indexOf('l',0,Qt::CaseInsensitive) != -1;

	KviKvsArray * a = new KviKvsArray();

	int iIndex=0;

	QTextStream stream( &f );

	if (!bLocal8Bit) stream.setCodec(QTextCodec::codecForMib(106));
	for(int i=0;i<iStartLine;i++)
		stream.readLine();

	if(iCount>0)
	{
		for(; (iCount>0 && !stream.atEnd()) ; iCount-- )
			a->set(iIndex,new KviKvsVariant(stream.readLine()));
			iIndex++;
	} else {
		while(!stream.atEnd()) {
			a->set(iIndex,new KviKvsVariant(stream.readLine()));
			iIndex++;
		}
	}

	f.close();
	
	c->returnValue()->setArray(a);

	return true;
}


/*
	@doc: file.writelines
	@type:
		command
	@title:
		file.writeLines
	@short:
		Writes an array of lines to a file
	@syntax:
		file.writeLines [-l] [-a] [-c] <filename:string> <lines:array>
	@switches:
		!sw: -l | --local-8-bit
		Causes the lines to be saved in local 8 bit character set instead
		of the default unicode encoding.
		!sw: -a | --append
		If the file already exists, then the lines are appended to the end
		instead of overwriting the file.
		!sw: -c | --crlf
		The lines are separated by a carriage-return+line-feed character
		combination, compatible with windows text mode files.
		This is the only way to make the file readable in windows notepad, for example.
		Please note that this is broken design: do NOT use it :)
		!sw: -n | --no-separator
		Do not separate the lines at all (either the separators are already
		inside the lines array or no line separation is desired at all).
		-n takes precedence over -c.
		!sw: -q | --quiet
		Don't complain if the file can't be opened: just fail silently
	@description:
		Writes the array of <lines> to the specified file.
		The lines are separated by a single linefeed character (see also [fnc]$lf[/fnc])
		unless the -c or -n switches are used. If the file already exists then it is
		overwritten with the new data unless the -a switch is used.
		The lines array is encoded in the ut8 character set unless the -l switch is used.
	@seealso:
		[fnc]$file.readLines[/fnc]
*/


static bool file_kvs_cmd_writeLines(KviKvsModuleCommandCall * c)
{
	QString szFile,szFlags;
	KviKvsArrayCast a;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("filename",KVS_PT_NONEMPTYSTRING,0,szFile)
		KVSM_PARAMETER("lines",KVS_PT_ARRAYCAST,0,a)
	KVSM_PARAMETERS_END(c)

	KviFileUtils::adjustFilePath(szFile);

	KviFile f(szFile);
	int iFlags = QIODevice::WriteOnly;

	if(!f.openForWriting(c->switches()->find('a',"append")))
	{
		if(!c->switches()->find('q',"quiet"))
			c->warning(__tr2qs("Can't open the file \"%Q\" for writing"),&szFile);
		return true;
	}

	bool bLocal8Bit = c->switches()->find('l',"local-8-bit");
	bool bNoSeparator = c->switches()->find('n',"no-separator");
	bool bAddCR = c->switches()->find('c',"crlf");

	unsigned int u = 0;
	while(u < a.array()->size())
	{
		KviKvsVariant * v = a.array()->at(u);
		KviQCString dat;
		if(v)
		{
			QString szDat;
			v->asString(szDat);
			dat = bLocal8Bit ? szDat.toLocal8Bit() : szDat.toUtf8();

			if(!bNoSeparator)
			{
				if(bAddCR)dat += "\r\n";
				else dat += '\n';
			}
			f.writeBlock(dat.data(),dat.length());
		}
		u++;
	}

	f.close();

	return true;
}


/*
	@doc: file.localdir
	@type:
		function
	@title:
		$file.localdir
	@short:
		Get the KVIrc local directory
	@syntax:
		<string> $file.localdir([relative_path:string])
	@description:
		Returns the path to the KVIrc local data directory.[br]
		The KVIrc local data directory is always writeable and contains
		the various subdirectories that KVIrc uses internally: audio , avatars ,
		config , help , incoming , log , modules , msgcolors and pics.[br]
		If <relative_path> is passed , then it is appended at the end of the directory
		to form a complete filepath.[br]
		The path is adjusted to contain single separators suitable for the platform
		that KVIrc is atually running on (thus you not need to care about path
		separators in the <relative_path> , KVIrc will adjust them).[br]
	@examples:
		[example]
			echo KVIrc looks for pictures in $file.localdir(pics)
			echo panic.png would be translated to $file.localdir(pics/panic.png)
		[/example]
*/

static bool file_kvs_fnc_localdir(KviKvsModuleFunctionCall * c)
{
	QString szName;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("relative_path",KVS_PT_STRING,KVS_PF_OPTIONAL,szName)
	KVSM_PARAMETERS_END(c)
	if(szName.isEmpty())szName.append(KVI_PATH_SEPARATOR_CHAR);
	QString szPath;
	g_pApp->getLocalKvircDirectory(szPath,KviApp::None,szName);
	KviFileUtils::adjustFilePath(szPath);
	c->returnValue()->setString(szPath);
	return true;
}



/*
	@doc: file.homedir
	@type:
		function
	@title:
		$file.homedir
	@short:
		Get the user's HOME directory
	@syntax:
		<string> $file.homedir([relative_path:string])
	@description:
		Returns the user's HOME directory path.[br]
		If <relative_path> is passed , then it is appended at the end of the directory
		to form a complete filepath.[br]
		The path is adjusted to contain single separators suitable for the platform
		that KVIrc is atually running on (thus you not need to care about path
		separators in the <relative_path> , KVIrc will adjust them).[br]
*/

static bool file_kvs_fnc_homedir(KviKvsModuleFunctionCall * c)
{
	QString szName;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("relative_path",KVS_PT_STRING,KVS_PF_OPTIONAL,szName)
	KVSM_PARAMETERS_END(c)
	if(szName.isEmpty())szName.append(KVI_PATH_SEPARATOR_CHAR);
	QString szPath = QDir::homePath();
	KviQString::ensureLastCharIs(szPath,KVI_PATH_SEPARATOR_CHAR);
	szPath.append(szName);
	KviFileUtils::adjustFilePath(szPath);
	c->returnValue()->setString(szPath);
	return true;
}



/*
	@doc: file.rootdir
	@type:
		function
	@title:
		$file.rootdir
	@short:
		Get the system root directory
	@syntax:
		$file.rootdir(<relative_path>)
		$file.rootdir
	@description:
		Returns the system's root directory (/ on UNIX and C:/ on Windows).[br]
		If <relative_path> is passed , then it is appended at the end of the directory
		to form a complete filepath.[br]
		The path is adjusted to contain single separators suitable for the platform
		that KVIrc is atually running on (thus you not need to care about path
		separators in the <relative_path> , KVIrc will adjust them).[br]
*/

static bool file_kvs_fnc_rootdir(KviKvsModuleFunctionCall * c)
{
	QString szName;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("relative_path",KVS_PT_STRING,KVS_PF_OPTIONAL,szName)
	KVSM_PARAMETERS_END(c)
	if(szName.isEmpty())szName.append(KVI_PATH_SEPARATOR_CHAR);
	QString szPath = QDir::rootPath();
	KviQString::ensureLastCharIs(szPath,KVI_PATH_SEPARATOR_CHAR);
	szPath.append(szName);
	KviFileUtils::adjustFilePath(szPath);
	c->returnValue()->setString(szPath);
	return true;
}


/*
	@doc: file.cwd
	@type:
		function
	@title:
		$file.currentdir
	@short:
		Get the current directory
	@syntax:
		<string> $file.cwd([relative_path:string])
	@description:
		Returns the current working directory.[br]
		If <relative_path> is passed , then it is appended at the end of the directory
		to form a complete filepath.[br]
		The path is adjusted to contain single separators suitable for the platform
		that KVIrc is atually running on (thus you not need to care about path
		separators in the <relative_path> , KVIrc will adjust them).[br]
*/

static bool file_kvs_fnc_cwd(KviKvsModuleFunctionCall * c)
{
	QString szName;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("relative_path",KVS_PT_STRING,KVS_PF_OPTIONAL,szName)
	KVSM_PARAMETERS_END(c)
	if(szName.isEmpty())szName.append(KVI_PATH_SEPARATOR_CHAR);
	QString szPath = QDir::currentPath();
	KviQString::ensureLastCharIs(szPath,KVI_PATH_SEPARATOR_CHAR);
	szPath.append(szName);
	KviFileUtils::adjustFilePath(szPath);
	c->returnValue()->setString(szPath);
	return true;
}


/*
	@doc: file.globaldir
	@type:
		function
	@title:
		$file.globaldir
	@short:
		Get the KVIrc global directory
	@syntax:
		$file.globaldir(<relative_path>)
		$file.globaldir
	@description:
		Returns the path to the KVIrc global data directory.[br]
		The KVIrc local data directory is always readable but usually not writeable and contains
		the various subdirectories that KVIrc uses internally: audio , avatars ,
		config , help , incoming , log , modules , msgcolors and pics.[br]
		If <relative_path> is passed , then it is appended at the end of the directory
		to form a complete filepath.[br]
		The path is adjusted to contain single separators suitable for the platform
		that KVIrc is atually running on (thus you not need to care about path
		separators in the <relative_path> , KVIrc will adjust them).[br]
	@examples:
		[example]
			echo KVIrc looks for pictures in $file.globaldir(pics)
			echo panic.png would be translated to $file.globaldir(pics/panic.png)
		[/example]
*/

static bool file_kvs_fnc_globaldir(KviKvsModuleFunctionCall * c)
{
	QString szName;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("relative_path",KVS_PT_STRING,KVS_PF_OPTIONAL,szName)
	KVSM_PARAMETERS_END(c)
	if(szName.isEmpty())szName.append(KVI_PATH_SEPARATOR_CHAR);
	QString szPath;
	g_pApp->getGlobalKvircDirectory(szPath,KviApp::None,szName);
	KviFileUtils::adjustFilePath(szPath);
	c->returnValue()->setString(szPath);
	return true;
}

/*
	@doc: file.extractpath
	@type:
		function
	@title:
		$file.extractpath
	@short:
		Extract the path from a filename
	@syntax:
		<string> $file.extractpath(<filepath:string>)
	@description:
		Returns the path part of the <filepath> translated to match the current
		platform filesystem conventions.[br]
		The path will NOT contain a trailing path separator.[br]
		For example, if <filepath> is /usr/arch/mp3/Carisma_SuonoDelSilenzio.mp3 then
		this function will return /usr/arch/mp3 on UNIX and C:\usr\arch\mp3 on Windows.
	@seealso:
		[fnc]$file.extractFileName[/fnc]
*/

static bool file_kvs_fnc_extractpath(KviKvsModuleFunctionCall * c)
{
	QString szName;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("filepath",KVS_PT_NONEMPTYSTRING,0,szName)
	KVSM_PARAMETERS_END(c)
	c->returnValue()->setString(QFileInfo(szName).absolutePath());
	return true;
}


/*
	@doc: file.extractfilename
	@type:
		function
	@title:
		$file.extractfilename
	@short:
		Extract the filename from a file path
	@syntax:
		<string> $file.extractpath(<filepath:string>)
	@description:
		Returns the filename part of the filepath translated to match the current
		platform filesystem conventions.[br]
		For example, if <filepath> is /usr/arch/mp3/Carisma_SuonoDelSilenzio.mp3 then
		this function will return Carisma_SuonoDelSilenzio.mp3
	@seealso:
		[fnc]$file.extractPath[/fnc]
*/

static bool file_kvs_fnc_extractfilename(KviKvsModuleFunctionCall * c)
{
	QString szName;
	KVSM_PARAMETERS_BEGIN(c)
		KVSM_PARAMETER("filepath",KVS_PT_NONEMPTYSTRING,0,szName)
	KVSM_PARAMETERS_END(c)
	KviFileUtils::extractFileName(szName);
	KviQString::cutToLast(szName,KVI_PATH_SEPARATOR_CHAR);
	c->returnValue()->setString(szName);
	return true;
}


static bool file_module_init(KviModule * m)
{

	KVSM_REGISTER_SIMPLE_COMMAND(m,"copy",file_kvs_cmd_copy);
	KVSM_REGISTER_SIMPLE_COMMAND(m,"rename",file_kvs_cmd_rename);
	KVSM_REGISTER_SIMPLE_COMMAND(m,"mkdir",file_kvs_cmd_mkdir);
	KVSM_REGISTER_SIMPLE_COMMAND(m,"write",file_kvs_cmd_write);
	KVSM_REGISTER_SIMPLE_COMMAND(m,"remove",file_kvs_cmd_remove);
	KVSM_REGISTER_SIMPLE_COMMAND(m,"rmdir",file_kvs_cmd_rmdir);
	KVSM_REGISTER_SIMPLE_COMMAND(m,"addimagepath",file_kvs_cmd_addimagepath);
	KVSM_REGISTER_SIMPLE_COMMAND(m,"delimagepath",file_kvs_cmd_delimagepath);
	KVSM_REGISTER_SIMPLE_COMMAND(m,"writeLines",file_kvs_cmd_writeLines);

	KVSM_REGISTER_FUNCTION(m,"exists",file_kvs_fnc_exists);
	KVSM_REGISTER_FUNCTION(m,"type",file_kvs_fnc_type);
	KVSM_REGISTER_FUNCTION(m,"size",file_kvs_fnc_size);
	KVSM_REGISTER_FUNCTION(m,"allsizes",file_kvs_fnc_allSizes);
	KVSM_REGISTER_FUNCTION(m,"fixpath",file_kvs_fnc_fixpath);
	KVSM_REGISTER_FUNCTION(m,"ps",file_kvs_fnc_ps);
	KVSM_REGISTER_FUNCTION(m,"read",file_kvs_fnc_read);
	KVSM_REGISTER_FUNCTION(m,"localdir",file_kvs_fnc_localdir);
	KVSM_REGISTER_FUNCTION(m,"homedir",file_kvs_fnc_homedir);
	KVSM_REGISTER_FUNCTION(m,"rootdir",file_kvs_fnc_rootdir);
	KVSM_REGISTER_FUNCTION(m,"cwd",file_kvs_fnc_cwd);
	KVSM_REGISTER_FUNCTION(m,"globaldir",file_kvs_fnc_globaldir);
	KVSM_REGISTER_FUNCTION(m,"extractpath",file_kvs_fnc_extractpath);
	KVSM_REGISTER_FUNCTION(m,"extractfilename",file_kvs_fnc_extractfilename);
	KVSM_REGISTER_FUNCTION(m,"ls",file_kvs_fnc_ls);
	KVSM_REGISTER_FUNCTION(m,"readLines",file_kvs_fnc_readLines);

	return true;
}

static bool file_module_cleanup(KviModule *m)
{
	return true;
}

KVIRC_MODULE(
	"File",                                                 // module name
	"4.0.0",                                                // module version
	"Copyright (C) 2001 Szymon Stefanek (pragma at kvirc dot net)", // author & (C)
	"Interface to the file system",
	file_module_init,
	0,
	0,
	file_module_cleanup
)
