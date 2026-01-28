// SPDX-FileCopyrightText: 2002-2026 PCSX2 Dev Team
// SPDX-License-Identifier: GPL-3.0+

#include "Switch/SwitchPlatform.h"
#include "Switch/SwitchHost.h"

namespace SwitchPlatform
{
	bool Initialize()
	{
		return SwitchHost::InitializeConfig();
	}

	void Shutdown()
	{
		SwitchHost::Shutdown();
	}
}
