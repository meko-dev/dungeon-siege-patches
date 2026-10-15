# Dungeon Siege DLL mods

Three independent mods for the Steam x86 Dungeon Siege 1 executable: expanded resolution choices, native floating damage numbers, and 4x MSAA through dgVoodoo. A shared `version.dll` loader starts the two custom plugins.

![More available resolutions](https://github.com/meko-dev/dungeon-siege-patches/blob/main/screenshots/full-hd.png?raw=true "More available resolutions")
![Anti-alias](https://github.com/meko-dev/dungeon-siege-patches/blob/main/screenshots/aa-compare.png?raw=true "Anti-alias")
![Floating Damage Numbers](https://github.com/meko-dev/dungeon-siege-patches/blob/main/screenshots/damage.png?raw=true "Floating Damage Numbers")

## How to Install

Download the latest ZIP archive from the **Releases** page. Close the game and unzip it directly into the game folder beside `DungeonSiege.exe`, replacing existing mod files if prompted. Start the game normally; no scripts or mod folders are required.

The downloaded ZIP contains exactly these six files at its root:

```text
D3DImm.dll
DDraw.dll
dgVoodoo.conf
ds_damage_numbers.dll
ds_resolution.dll
version.dll
```

## Requirements

Windows, Visual Studio or Build Tools with **Desktop development with C++**, x86 compiler and Windows SDK, and PowerShell 5.1 or newer. Compilation requires no game files. Installing the native patches requires Steam EXE SHA256 `C408EF77B39484D8AD82BA17859CF1E60B24D3BAF6D429283A52B886D67F33AB`.

## Build

Run `build.cmd` at the repository root for the loader and both native plugins. Run `antialias_mod/build.ps1` in PowerShell to fetch and verify the official dgVoodoo dependency. AA uses third-party closed-source DLLs; this repository contains its configuration and setup scripts only.

Outputs: `mod_loader/build/version.dll`, `resolution_mod/build/ds_resolution.dll`, `damage_numbers_mod/build/ds_damage_numbers.dll`, and `antialias_mod/build/{DDraw,D3DImm}.dll`.

## Disclaimer

These mods are provided **as is**, without any warranty. Use them at your own risk. I accept no responsibility or liability for any damage, data loss, corrupted saves, crashes, or other problems caused by downloading, installing, or using these mods. Back up your game files and saves before installing.
