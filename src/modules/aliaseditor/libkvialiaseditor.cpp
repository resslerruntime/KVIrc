//=============================================================================
//
//   File : libkvialiaseditor.cpp
//   Creation date : Mon 23 Dec 2002 14:30:59 2002 GMT by Szymon Stefanek
//
//   This toolbar is part of the KVirc irc client distribution
//   Copyright (C) 2002-2008 Szymon Stefanek (pragma at kvirc dot net)
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

#include "aliaseditor.h"

#include "kvi_module.h"
#include "kvi_locale.h"
#include "kvi_frame.h"


KviAliasEditorWindow * g_pAliasEditorWindow = 0;
KviModule * g_pAliasEditorModule = 0;


/*
	@doc: aliaseditor.open
	@type:
		command
	@title:
		aliaseditor.open
	@short:
		Shows the alias editor
	@syntax:
		aliaseditor.open
	@description:
		Opens the script alias editor dialog.
*/

static bool aliaseditor_kvs_cmd_open(KviKvsModuleCommandCall * c)
{
	if(!g_pAliasEditorWindow)
	{
		g_pAliasEditorWindow = new KviAliasEditorWindow(c->window()->frame());
		c->window()->frame()->addWindow(g_pAliasEditorWindow);
	}

	g_pAliasEditorWindow->setFocus();

	return true;
}

static bool aliaseditor_module_init(KviModule * m)
{
	KVSM_REGISTER_SIMPLE_COMMAND(m,"open",aliaseditor_kvs_cmd_open);
	g_pAliasEditorWindow = 0;
	g_pAliasEditorModule = m;
	return true;
}

static bool aliaseditor_module_can_unload(KviModule * m)
{
	return (g_pAliasEditorWindow == 0);
}

static bool aliaseditor_module_cleanup(KviModule *m)
{
	if(g_pAliasEditorWindow)delete g_pAliasEditorWindow;
	g_pAliasEditorWindow = 0;
	return true;
}

KVIRC_MODULE(
	"AliasEditor",                                                       // module name
	"4.0.0",                                                             // module version
	"Copyright (C) 2002-2005 Szymon Stefanek (pragma at kvirc dot net)", // author & (C)
	"Editor for the script aliases",
	aliaseditor_module_init,
	aliaseditor_module_can_unload,
	0,
	aliaseditor_module_cleanup
)
