# Shared DLL loader

`version.dll` forwards the 17 Windows version APIs and loads `ds_resolution.dll` and `ds_damage_numbers.dll` from its own directory at game startup. Missing plugins are disabled. Load errors go to `mod_loader/runtime.log`. Modules remain loaded because game hooks reference their code. AA uses its own dgVoodoo wrappers.

Run `build.cmd` with Visual Studio C++ x86 tools to produce `build/version.dll`. Close the game and run `install.ps1`, then install each plugin. The installer preserves the previous version.dll under ignored `backup/`. Plugin updates do not replace the loader. No game files are needed to compile.

Source: `loader.cpp` and `version.def`. See the root README for complete setup.
