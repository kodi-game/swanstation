#pragma once
#include "common/string.h"
#include "common/timer.h"
#include "settings.h"
#include "types.h"
#include <chrono>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

enum LOGLEVEL;

class AudioStream;
class ByteStream;
class CDImage;
class HostDisplay;
class GameList;

class System;
struct SystemBootParameters;

class HostInterface
{
  friend System;

public:
  enum : u32
  {
    AUDIO_SAMPLE_RATE = 44100,
    AUDIO_CHANNELS = 2,
    DEFAULT_AUDIO_BUFFER_SIZE = 2048
  };

  HostInterface();
  virtual ~HostInterface();

  /// Access to host display.
  ALWAYS_INLINE HostDisplay* GetDisplay() const { return m_display.get(); }

  /// Access to host audio stream.
  ALWAYS_INLINE AudioStream* GetAudioStream() const { return m_audio_stream.get(); }

  /// Returns a settings object which can be modified.
  ALWAYS_INLINE Settings& GetSettings() { return m_settings; }

  /// Access to emulated system.
  ALWAYS_INLINE System* GetSystem() const { return m_system.get(); }

  /// Initializes the emulator frontend.
  virtual bool Initialize();

  /// Shuts down the emulator frontend.
  virtual void Shutdown();

  virtual bool BootSystem(const SystemBootParameters& parameters);
  virtual void PowerOffSystem();

  virtual void ResetSystem();
  virtual void DestroySystem();

  /// Loads state from the specified filename.
  bool LoadState(const char* filename);

  virtual void ReportError(const char* message);
  virtual void ReportMessage(const char* message);
  virtual bool ConfirmMessage(const char* message);

  void ReportFormattedError(const char* format, ...);
  void ReportFormattedMessage(const char* format, ...);
  bool ConfirmFormattedMessage(const char* format, ...);

  /// Adds OSD messages, duration is in seconds.
  virtual void AddOSDMessage(std::string message, float duration = 2.0f);
  void AddFormattedOSDMessage(float duration, const char* format, ...);

  /// Returns the base user directory path.
  ALWAYS_INLINE const std::string& GetUserDirectory() const { return m_user_directory; }

  /// Returns a path relative to the user directory.
  std::string GetUserDirectoryRelativePath(const char* format, ...) const;

  /// Returns a path relative to the application directory (for system files).
  std::string GetProgramDirectoryRelativePath(const char* format, ...) const;

  /// Returns a string which can be used as part of a filename, based on the current date/time.
  static TinyString GetTimestampStringForFileName();

  /// Displays a loading screen with the logo, rendered with ImGui. Use when executing possibly-time-consuming tasks
  /// such as compiling shaders when starting up.
  virtual void DisplayLoadingScreen(const char* message, int progress_min = -1, int progress_max = -1,
                                    int progress_value = -1);

  /// Retrieves information about specified game from game list.
  virtual void GetGameInfo(const char* path, CDImage* image, std::string* code, std::string* title);

  /// Returns the default path to a memory card.
  virtual std::string GetSharedMemoryCardPath(u32 slot) const;

  /// Returns the default path to a memory card for a specific game.
  virtual std::string GetGameMemoryCardPath(const char* game_code, u32 slot) const;

  /// Returns the path to the shader cache directory.
  virtual std::string GetShaderCacheBasePath() const;

  /// Returns a setting value from the configuration.
  virtual std::string GetSettingValue(const char* section, const char* key, const char* default_value = "") = 0;

  /// Returns a boolean setting from the configuration.
  bool GetBooleanSettingValue(const char* section, const char* key, bool default_value = false);

  /// Returns an integer setting from the configuration.
  s32 GetIntegerSettingValue(const char* section, const char* key, s32 default_value = 0);

  /// Returns a float setting from the configuration.
  float GetFloatSettingValue(const char* section, const char* key, float default_value = 0.0f);

protected:
  virtual bool AcquireHostDisplay() = 0;
  virtual void ReleaseHostDisplay() = 0;
  virtual std::unique_ptr<AudioStream> CreateAudioStream(AudioBackend backend) = 0;

  virtual void OnSystemCreated();
  virtual void OnSystemDestroyed();
  virtual void OnSystemPerformanceCountersUpdated();
  virtual void OnSystemStateSaved(bool global, s32 slot);
  virtual void OnRunningGameChanged();
  virtual void OnControllerTypeChanged(u32 slot);

  /// Restores all settings to defaults.
  virtual void SetDefaultSettings(SettingsInterface& si);

  /// Loads settings to m_settings and any frontend-specific parameters.
  virtual void LoadSettings(SettingsInterface& si);

  /// Saves current settings variables to ini.
  virtual void SaveSettings(SettingsInterface& si);

  /// Checks for settings changes, std::move() the old settings away for comparing beforehand.
  virtual void CheckForSettingsChanges(const Settings& old_settings);

  /// Switches the GPU renderer by saving state, recreating the display window, and restoring state (if needed).
  virtual void RecreateSystem();

  /// Sets the user directory to the program directory, i.e. "portable mode".
  void SetUserDirectoryToProgramDirectory();

  /// Loads the BIOS image for the specified region.
  std::optional<std::vector<u8>> GetBIOSImage(ConsoleRegion region);

  /// Quick switch between software and hardware rendering.
  void ToggleSoftwareRendering();

  /// Adjusts the internal (render) resolution of the hardware backends.
  void ModifyResolutionScale(s32 increment);

  /// Updates software cursor state, based on controllers.
  void UpdateSoftwareCursor();

  bool SaveState(const char* filename);
  void CreateAudioStream();

  std::unique_ptr<HostDisplay> m_display;
  std::unique_ptr<AudioStream> m_audio_stream;
  std::unique_ptr<System> m_system;
  Settings m_settings;
  std::string m_program_directory;
  std::string m_user_directory;
};
