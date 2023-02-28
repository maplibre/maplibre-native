param(
	[Parameter(Mandatory=$true)][string]$Triplet,
	[Parameter(Mandatory=$true)][string]$Renderer
)

Set-Location (Split-Path $MyInvocation.MyCommand.Path -Parent)

$vcpkg_temp_dir = '***'

foreach($letter in [byte][char]'Z'..[byte][char]'A')
{
    $vcpkg_temp_dir = '{0}:' -f [char]$letter

    if(-not (Test-Path $vcpkg_temp_dir))
    {
        & subst $vcpkg_temp_dir ([System.IO.Path]::Combine($PWD.Path, 'vendor', 'vcpkg'))
        break
    }
}

switch($Renderer)
{
    'EGL'    { $renderer_packages = @('egl', 'opengl-registry'); break }
    'OSMesa' { $renderer_packages = @('');                       break }
    'OpenGL' { $renderer_packages = @('opengl-registry');        break }
}

if(-not (Test-Path "$vcpkg_temp_dir\vcpkg.exe"))
{
    & "$vcpkg_temp_dir\bootstrap-vcpkg.bat"
}

& "$vcpkg_temp_dir\vcpkg.exe" --disable-metrics --overlay-triplets=$([System.IO.Path]::Combine($PWD.Path, 'vendor', 'vcpkg-custom-triplets')) --triplet=$($Triplet) --clean-after-build install curl dlfcn-win32 glfw3 icu libuv libjpeg-turbo libpng $($renderer_packages)

subst $vcpkg_temp_dir /D

if($Renderer -eq 'OSMesa' -and -not (Test-Path ([System.IO.Path]::Combine($PWD.Path, 'vendor', 'mesa3d'))))
{
    New-Item -Name temp -Type Directory | Out-Null
    Invoke-WebRequest https://www.7-zip.org/a/7zr.exe -OutFile ([System.IO.Path]::Combine($PWD.Path, 'temp', '7zr.exe'))
    (Invoke-WebRequest https://api.github.com/repos/pal1000/mesa-dist-win/releases | ConvertFrom-Json)[0].assets | Where-Object name -match 'mesa3d-.+-(release|development-pack)-msvc\.7z' | foreach { Invoke-WebRequest $_.browser_download_url -OutFile ([System.IO.Path]::Combine($PWD.Path, 'temp', $_.name)) }
    Get-ChildItem 'temp\*.7z' | foreach { .\temp\7zr.exe x -ovendor\mesa3d $_.FullName }
    Remove-Item temp -Recurse
}
