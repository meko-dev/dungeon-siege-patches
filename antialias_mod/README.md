# 4x MSAA with dgVoodoo

This mod is a configuration and installer for Dege's closed-source dgVoodoo 2.87.5 DirectDraw/Direct3D wrapper. Its binaries are downloaded separately; they are not built from this repository or bundled in it.

Run `build.ps1` to download the official package, verify its SHA256 and extract only the x86 `DDraw.dll` and `D3DImm.dll` into ignored `build/`. For offline preparation, use `build.ps1 -Archive C:\path\dgVoodoo2_87_5.zip`.

Close the game and run `install.ps1`. It installs those DLLs and `dgVoodoo.conf`, preserving previous files in ignored `backup/`. Run `disable.ps1` to disable AA and restore any backed-up files. The loader and other mods are unaffected.

The configuration uses Direct3D 11, 4x MSAA and native device names. Resolution and filtering remain application-controlled. Change `[DirectX] Antialiasing` to `2x`, `4x`, `8x` or `off` and restart the game.

Vendor/download/license: https://dege.freeweb.hu/dgVoodoo2/
Documentation: https://dege.freeweb.hu/dgVoodoo2/ReadmeDirectX/
