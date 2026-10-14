# Floating damage numbers

`ds_damage_numbers.dll` observes health loss and draws gold numbers with native renderer/UIText code. Numbers rise for 1.15 seconds, remain at the hit location, and cap killing blows at remaining health. Healing, party-member damage and multiplayer are filtered. Exceptions disable numbers for the session and write `runtime.log`.

Build with `build.cmd` using Visual Studio C++ x86 tools. Output: `build/ds_damage_numbers.dll`, static CRT. No game files or resolution sources are required to compile.

Supported Steam EXE SHA256: `C408EF77B39484D8AD82BA17859CF1E60B24D3BAF6D429283A52B886D67F33AB`. Source: `damage.cpp`.
