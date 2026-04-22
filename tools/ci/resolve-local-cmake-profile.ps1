function Get-UrpgVisualStudio2022InstallPath {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        return $null
    }

    $installPath = & $vswhere -latest -products * `
        -requires Microsoft.Component.MSBuild Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -version "[17.0,18.0)" `
        -property installationPath
    if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($installPath)) {
        return $null
    }

    return $installPath.Trim()
}

function Get-UrpgLocalBuildProfile {
    $ninja = Get-Command ninja -ErrorAction SilentlyContinue
    $mingwMake = Get-Command mingw32-make -ErrorAction SilentlyContinue
    $gxx = Get-Command g++ -ErrorAction SilentlyContinue
    $vsInstall = Get-UrpgVisualStudio2022InstallPath

    if ($ninja) {
        return [pscustomobject]@{
            ConfigurePreset = "dev-ninja-debug"
            BuildPreset = "dev-debug"
            BuildDirectory = "build/dev-ninja-debug"
            Configuration = "Debug"
            Generator = "Ninja"
        }
    }

    if ($vsInstall) {
        return [pscustomobject]@{
            ConfigurePreset = "dev-vs2022"
            BuildPreset = "dev-vs2022-debug"
            BuildDirectory = "build/dev-vs2022"
            Configuration = "Debug"
            Generator = "Visual Studio 17 2022"
        }
    }

    if ($mingwMake -and $gxx) {
        return [pscustomobject]@{
            ConfigurePreset = "dev-mingw-debug"
            BuildPreset = "dev-mingw-debug-build"
            BuildDirectory = "build/dev-mingw-debug"
            Configuration = "Debug"
            Generator = "MinGW Makefiles"
        }
    }

    throw "No supported local CMake profile found. Install one of: Ninja, Visual Studio 2022 Build Tools, or MinGW g++ + mingw32-make."
}
