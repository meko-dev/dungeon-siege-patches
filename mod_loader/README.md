# Shared DLL loader

`version.dll` forwards the 17 Windows version APIs and loads `ds_resolution.dll` and `ds_damage_numbers.dll` from its own directory at game startup. Missing plugins are disabled. Load errors go to `mod_loader/runtime.log`. Modules remain loaded because game hooks reference their code. AA uses its own dgVoodoo wrappers.

Source: `loader.cpp` and `version.def`. See the root README for complete setup.
