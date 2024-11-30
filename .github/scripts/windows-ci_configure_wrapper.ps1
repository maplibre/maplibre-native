$compile_flags = @(
	'-DCMAKE_POLICY_DEFAULT_CMP0141=NEW',
	'-DCMAKE_MSVC_DEBUG_INFORMATION_FORMAT=Embedded',
	'-DCMAKE_BUILD_TYPE=RelWithDebInfo'
)

switch ($env:RENDERER)
{
	'opengl' { $compile_flags += '-DMLN_WITH_OPENGL=ON'; break; }
	'egl'    { $compile_flags += '-DMLN_WITH_EGL=ON'   ; break; }
	'vulkan' { $compile_flags += @('-DMLN_WITH_VULKAN=ON', '-DMLN_WITH_OPENGL=OFF'); break; }
	'osmesa' { $compile_flags += '-DMLN_WITH_OSMESA=ON'; break; }
}

switch ($env:RENDERING_MODE)
{
	'legacy'   { $compile_flags += '-DMLN_LEGACY_RENDERER=ON'  ; break; }
	'drawable' { $compile_flags += '-DMLN_DRAWABLE_RENDERER=ON'; break; }
}

Write-Host 'Compile flags: ' $compile_flags

& cmake -B build -G Ninja $($compile_flags)
