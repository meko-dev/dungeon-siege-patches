# Dungeon Siege resolution plugin

`ds_resolution.dll` adds common resolutions to Options > Video > Screen Resolution, retains all original entries, and uses the game's apply/save code. All added modes use 32-bit color. The dropdown has four visible rows; scroll to find more choices. Original DirectDraw compatibility checks still apply: only modes reported as compatible by the display/graphics wrapper appear. Saved settings take precedence over the retained Full HD startup fallback.

Supported additions:

- 1280x720, 1280x800, 1280x1024
- 1360x768, 1366x768, 1440x900, 1600x900, 1680x1050
- 1920x1080 (Full HD), 1920x1200, 2048x1080 (DCI 2K)
- 2560x1080, 2560x1440 (QHD/1440p, commonly called 2K), 2560x1600
- 3440x1440, 3840x1600, 3840x2160 (4K UHD), 5120x1440

The installed plugin contains only resolution code. `version.dll` is now a separate shared loader; see `../mod_loader/README.md`. Damage numbers and dgVoodoo anti-aliasing use their own DLLs.

Close the game and run `install.ps1` to install/update only this plugin; the shared loader must already be installed. Run `disable.ps1` to disable only resolution. Re-enable with `install.ps1`. Do not restore historical version.dll backups to disable this plugin; that would replace the shared loader.

Run `build.cmd` to produce `build/ds_resolution.dll` using Visual Studio C++ x86 tools. No game files are needed to compile. Static CRT; no external Visual C++ runtime dependency.

Supported Steam EXE SHA256: `C408EF77B39484D8AD82BA17859CF1E60B24D3BAF6D429283A52B886D67F33AB`. Other builds reject the patches. Source: `version.cpp` (historical filename, now resolution plugin code).
