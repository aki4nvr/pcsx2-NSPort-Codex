// SPDX-FileCopyrightText: 2002-2026 PCSX2 Dev Team
// SPDX-License-Identifier: GPL-3.0+

#include "Switch/SwitchHost.h"

#include "common/Console.h"
#include "common/CrashHandler.h"
#include "common/Error.h"
#include "common/FileSystem.h"
#include "common/Path.h"
#include "common/ProgressCallback.h"
#include "common/StringUtil.h"

#include "pcsx2/Achievements.h"
#include "pcsx2/Config.h"
#include "pcsx2/GS.h"
#include "pcsx2/Host.h"
#include "pcsx2/INISettingsInterface.h"
#include "pcsx2/ImGui/FullscreenUI.h"
#include "pcsx2/ImGui/ImGuiFullscreen.h"
#include "pcsx2/ImGui/ImGuiManager.h"
#include "pcsx2/Input/InputManager.h"
#include "pcsx2/VMManager.h"

#include <cstring>
#include <memory>

namespace
{
	std::unique_ptr<INISettingsInterface> s_base_settings_interface;
	std::unique_ptr<INISettingsInterface> s_secrets_settings_interface;
}

bool SwitchHost::InitializeConfig()
{
	Error error;

	EmuFolders::SetAppRoot();
	if (!EmuFolders::SetResourcesDirectory())
	{
		Console.Error("Resources directory is missing.");
		return false;
	}

	if (!EmuFolders::SetDataDirectory(&error))
	{
		Console.ErrorFmt("Failed to create data directory: {}", error.GetDescription());
		return false;
	}

	CrashHandler::SetWriteDirectory(EmuFolders::DataRoot);

	const std::string path = Path::Combine(EmuFolders::Settings, "PCSX2.ini");
	const bool settings_exists = FileSystem::FileExists(path.c_str());
	Console.WriteLnFmt("Loading config from {}.", path);

	s_base_settings_interface = std::make_unique<INISettingsInterface>(path);
	Host::Internal::SetBaseSettingsLayer(s_base_settings_interface.get());
	if (!settings_exists || !s_base_settings_interface->Load() || !VMManager::Internal::CheckSettingsVersion())
	{
		VMManager::SetDefaultSettings(*s_base_settings_interface, true, true, true, true, true);
		s_base_settings_interface->SetBoolValue("UI", "SetupWizardIncomplete", true);

		if (!s_base_settings_interface->Save(&error))
		{
			Console.ErrorFmt("Failed to save configuration: {}", error.GetDescription());
			return false;
		}
	}

	const std::string secrets_path = Path::Combine(EmuFolders::Settings, "secrets.ini");
	const bool secrets_exists = FileSystem::FileExists(secrets_path.c_str());
	Console.WriteLnFmt("Loading secrets from {}.", secrets_path);

	s_secrets_settings_interface = std::make_unique<INISettingsInterface>(secrets_path);
	Host::Internal::SetSecretsSettingsLayer(s_secrets_settings_interface.get());
	if (!secrets_exists || !s_secrets_settings_interface->Load())
	{
		if (!s_secrets_settings_interface->Save(&error))
		{
			Console.ErrorFmt("Failed to save secrets: {}", error.GetDescription());
			return false;
		}
	}

	VMManager::Internal::LoadStartupSettings();
	return true;
}

void SwitchHost::Shutdown()
{
	if (s_base_settings_interface && s_base_settings_interface->IsDirty())
		s_base_settings_interface->Save();
	if (s_secrets_settings_interface && s_secrets_settings_interface->IsDirty())
		s_secrets_settings_interface->Save();
}

void Host::CommitBaseSettingChanges()
{
	auto lock = Host::GetSettingsLock();
	SettingsInterface* base_layer = Host::Internal::GetBaseSettingsLayer();
	if (!base_layer)
		return;

	if (auto* ini = static_cast<INISettingsInterface*>(base_layer))
	{
		Error error;
		if (!ini->Save(&error))
			Console.ErrorFmt("Failed to save settings: {}", error.GetDescription());
	}
}

void Host::LoadSettings(SettingsInterface& si, std::unique_lock<std::mutex>& lock)
{
}

void Host::CheckForSettingsChanges(const Pcsx2Config& old_config)
{
}

bool Host::RequestResetSettings(bool folders, bool core, bool controllers, bool hotkeys, bool ui)
{
	{
		auto lock = Host::GetSettingsLock();
		VMManager::SetDefaultSettings(*Host::Internal::GetBaseSettingsLayer(), folders, core, controllers, hotkeys, ui);
	}
	Host::CommitBaseSettingChanges();
	return true;
}

void Host::SetDefaultUISettings(SettingsInterface& si)
{
	si.SetBoolValue("UI", "InhibitScreensaver", true);
	si.SetBoolValue("UI", "ConfirmShutdown", true);
	si.SetBoolValue("UI", "StartPaused", false);
	si.SetBoolValue("UI", "PauseOnFocusLoss", false);
	si.SetBoolValue("UI", "StartFullscreen", true);
	si.SetBoolValue("UI", "DoubleClickTogglesFullscreen", false);
	si.SetBoolValue("UI", "HideMouseCursor", true);
	si.SetBoolValue("UI", "RenderToSeparateWindow", false);
	si.SetBoolValue("UI", "HideMainWindowWhenRunning", true);
	si.SetBoolValue("UI", "DisableWindowResize", true);
	si.SetBoolValue("UI", "PreferEnglishGameList", false);
}

bool Host::LocaleCircleConfirm()
{
	return false;
}

std::unique_ptr<ProgressCallback> Host::CreateHostProgressCallback()
{
	return ProgressCallback::CreateNullProgressCallback();
}

void Host::ReportInfoAsync(const std::string_view title, const std::string_view message)
{
	if (!title.empty() && !message.empty())
		INFO_LOG("ReportInfoAsync: {}: {}", title, message);
	else if (!message.empty())
		INFO_LOG("ReportInfoAsync: {}", message);
}

void Host::ReportErrorAsync(const std::string_view title, const std::string_view message)
{
	if (!title.empty() && !message.empty())
		ERROR_LOG("ReportErrorAsync: {}: {}", title, message);
	else if (!message.empty())
		ERROR_LOG("ReportErrorAsync: {}", message);
}

void Host::OpenURL(const std::string_view url)
{
}

bool Host::CopyTextToClipboard(const std::string_view text)
{
	return false;
}

void Host::BeginTextInput()
{
}

void Host::EndTextInput()
{
}

std::optional<WindowInfo> Host::GetTopLevelWindowInfo()
{
	WindowInfo wi;
	wi.type = WindowInfo::Type::Surfaceless;
	wi.surface_width = 1280;
	wi.surface_height = 720;
	wi.surface_scale = 1.0f;
	wi.surface_refresh_rate = 60.0f;
	return wi;
}

void Host::OnInputDeviceConnected(const std::string_view identifier, const std::string_view device_name)
{
}

void Host::OnInputDeviceDisconnected(const InputBindingKey key, const std::string_view identifier)
{
}

void Host::SetMouseMode(bool relative_mode, bool hide_cursor)
{
}

std::optional<WindowInfo> Host::AcquireRenderWindow(bool recreate_window)
{
	return GetTopLevelWindowInfo();
}

void Host::ReleaseRenderWindow()
{
}

void Host::BeginPresentFrame()
{
}

void Host::RequestResizeHostDisplay(s32 width, s32 height)
{
}

void Host::OnVMStarting()
{
}

void Host::OnVMStarted()
{
}

void Host::OnVMDestroyed()
{
}

void Host::OnVMPaused()
{
}

void Host::OnVMResumed()
{
}

void Host::OnGameChanged(const std::string& title, const std::string& elf_override, const std::string& disc_path,
	const std::string& disc_serial, u32 disc_crc, u32 current_crc)
{
}

void Host::OnPerformanceMetricsUpdated()
{
}

void Host::OnSaveStateLoading(const std::string_view filename)
{
}

void Host::OnSaveStateLoaded(const std::string_view filename, bool was_successful)
{
}

void Host::OnSaveStateSaved(const std::string_view filename)
{
}

void Host::RunOnCPUThread(std::function<void()> function, bool block /* = false */)
{
	function();
}

void Host::RefreshGameListAsync(bool invalidate_cache)
{
}

void Host::CancelGameListRefresh()
{
}

bool Host::IsFullscreen()
{
	return true;
}

void Host::SetFullscreen(bool enabled)
{
}

void Host::OnCaptureStarted(const std::string& filename)
{
}

void Host::OnCaptureStopped()
{
}

void Host::RequestExitApplication(bool allow_confirm)
{
}

void Host::RequestExitBigPicture()
{
}

void Host::RequestVMShutdown(bool allow_confirm, bool allow_save_state, bool default_save_state)
{
	if (!VMManager::HasValidVM())
		return;

	VMManager::Shutdown(allow_save_state && default_save_state);
}

void Host::PumpMessagesOnCPUThread()
{
}

bool Host::InBatchMode()
{
	return true;
}

bool Host::InNoGUIMode()
{
	return true;
}

bool Host::ShouldPreferHostFileSelector()
{
	return false;
}

void Host::OpenHostFileSelectorAsync(std::string_view title, bool select_directory, FileSelectorCallback callback,
	FileSelectorFilters filters, std::string_view initial_directory)
{
}

void Host::OnAchievementsLoginRequested(Achievements::LoginRequestReason reason)
{
}

void Host::OnAchievementsLoginSuccess(const char* username, u32 points, u32 sc_points, u32 unread_messages)
{
}

void Host::OnAchievementsRefreshed()
{
}

void Host::OnAchievementsHardcoreModeChanged(bool enabled)
{
}

void Host::OnCoverDownloaderOpenRequested()
{
}

void Host::OnCreateMemoryCardOpenRequested()
{
}

s32 Host::Internal::GetTranslatedStringImpl(
	const std::string_view context, const std::string_view msg, char* tbuf, size_t tbuf_space)
{
	if (msg.size() > tbuf_space)
		return -1;
	else if (msg.empty())
		return 0;

	std::memcpy(tbuf, msg.data(), msg.size());
	return static_cast<s32>(msg.size());
}

std::string Host::TranslatePluralToString(const char* context, const char* msg, const char* disambiguation, int count)
{
	TinyString count_str = TinyString::from_format("{}", count);

	std::string ret(msg);
	for (;;)
	{
		std::string::size_type pos = ret.find("%n");
		if (pos == std::string::npos)
			break;

		ret.replace(pos, pos + 2, count_str.view());
	}

	return ret;
}
