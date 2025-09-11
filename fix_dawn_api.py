#!/usr/bin/env python3
import os
import re

def fix_string_view_assignments(content):
    """Fix WGPUStringView assignments"""
    # Pattern to find label assignments
    pattern = r'(\w+Desc(?:riptor)?\.label)\s*=\s*"([^"]+)"'

    def replace_label(match):
        var_name = match.group(1)
        label_text = match.group(2)
        return f'WGPUStringView {var_name}_str = {{"{label_text}", strlen("{label_text}")}};\n    {var_name} = {var_name}_str'

    content = re.sub(pattern, replace_label, content)

    # Fix entryPoint assignments
    pattern2 = r'(\w+State\.entryPoint)\s*=\s*"([^"]+)"'

    def replace_entry(match):
        var_name = match.group(1)
        entry_text = match.group(2)
        return f'WGPUStringView {var_name.replace(".", "_")}_str = {{"{entry_text}", strlen("{entry_text}")}};\n    {var_name} = {var_name.replace(".", "_")}_str'

    content = re.sub(pattern2, replace_entry, content)

    # Fix code assignments for WGSL
    pattern3 = r'(\w+Desc\.code)\s*=\s*(\w+)\.c_str\(\)'

    def replace_code(match):
        var_name = match.group(1)
        src_var = match.group(2)
        return f'WGPUStringView {var_name.replace(".", "_")}_str = {{{src_var}.c_str(), {src_var}.length()}};\n    {var_name} = {var_name.replace(".", "_")}_str'

    content = re.sub(pattern3, replace_code, content)

    return content

def fix_bool_values(content):
    """Fix WGPUBool values"""
    content = re.sub(r'\bWGPUBool_True\b', '1', content)
    content = re.sub(r'\bWGPUBool_False\b', '0', content)
    content = re.sub(r'(\w+\.alphaToCoverageEnabled)\s*=\s*false', r'\1 = 0', content)
    content = re.sub(r'(\w+\.alphaToCoverageEnabled)\s*=\s*true', r'\1 = 1', content)
    content = re.sub(r'(\w+\.depthWriteEnabled)\s*=\s*false', r'\1 = 0', content)
    content = re.sub(r'(\w+\.depthWriteEnabled)\s*=\s*true', r'\1 = 1', content)
    return content

def fix_stype_enum(content):
    """Fix WGPUSType enum values"""
    content = re.sub(r'WGPUSType_ShaderModuleWGSLDescriptor', '(WGPUSType)0x00040006', content)
    return content

def fix_chained_struct(content):
    """Fix chained struct assignments"""
    content = re.sub(r'(\w+Desc\.nextInChain)\s*=\s*&(\w+)\.chain;',
                     r'\1 = (WGPUChainedStruct*)&\2.chain;', content)
    content = re.sub(r'(\w+Desc\.nextInChain)\s*=\s*reinterpret_cast<const WGPUChainedStruct\*>\(&(\w+)\)',
                     r'\1 = (WGPUChainedStruct*)&\2', content)
    return content

def add_includes(content):
    """Add necessary includes"""
    if '#include <cstring>' not in content and 'strlen' in content:
        # Find the last include
        last_include = max([m.end() for m in re.finditer(r'#include\s+[<"][^>"]+[>"]', content)] + [0])
        if last_include > 0:
            content = content[:last_include] + '\n#include <cstring>' + content[last_include:]
    return content

def process_file(filepath):
    """Process a single file"""
    with open(filepath, 'r') as f:
        content = f.read()

    original = content

    # Apply fixes
    content = add_includes(content)
    content = fix_string_view_assignments(content)
    content = fix_bool_values(content)
    content = fix_stype_enum(content)
    content = fix_chained_struct(content)

    if content != original:
        with open(filepath, 'w') as f:
            f.write(content)
        print(f"Fixed: {filepath}")
        return True
    return False

def main():
    """Main function"""
    webgpu_dirs = [
        '/Users/admin/repos/maplibre-native/src/mbgl/webgpu',
        '/Users/admin/repos/maplibre-native/src/mbgl/shaders/webgpu'
    ]

    fixed_count = 0

    for dir_path in webgpu_dirs:
        for root, dirs, files in os.walk(dir_path):
            for file in files:
                if file.endswith('.cpp'):
                    filepath = os.path.join(root, file)
                    if process_file(filepath):
                        fixed_count += 1

    print(f"\nTotal files fixed: {fixed_count}")

if __name__ == "__main__":
    main()
