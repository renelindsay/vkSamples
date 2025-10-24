#!/usr/bin/env python3

# VEXEL: Vulkan_Extension_Loader Generator.
# This script downloads the latest Vulkan headers and generates vexel.cpp/.h from it.
# Link those files to your C++ project, to initialize Vulkan and Extension function pointers.

License = '''
        // Copyright © 2024 Rene Lindsay (rjklindsay@hotmail.com)
        // Report bugs and download new versions at https://github.com/renelindsay/Vexel
        //
        // Permission is hereby granted, free of charge, to any person obtaining a copy
        // of this software and associated documentation files (the "Software"), to deal
        // in the Software without restriction, including without limitation the rights
        // to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
        // copies of the Software, and to permit persons to whom the Software is
        // furnished to do so, subject to the following conditions:
        //
        // The above copyright notice and this permission notice shall be included in all
        // copies or substantial portions of the Software.
        //
        // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
        // IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
        // FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
        // AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
        // LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
        // OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
        // SOFTWARE.
'''

import os
import shutil
import textwrap
import re
import zipfile
import io
import sys
import argparse
import requests
from requests.exceptions import HTTPError, Timeout, ConnectionError

# Add Optional functions:
Add_vexLoadDevice       = True
Add_vexLoadDeviceTable  = True
Add_vexVerifyExtensions = False
Add_VkResultToString    = True

VkResult = {}

def progress_bar(iteration, total, length=50):
    iteration = min(iteration, total)
    percent = (iteration / total) * 100
    filled_length = int(length * iteration / total)
    bar = '■' * filled_length + '-' * (length - filled_length)
    sys.stdout.write(f'[{bar}] {int(percent)}%\r')
    sys.stdout.flush()

def show_progress(caption, itteration, total, length=25):
    print('\r\033[K', end='')  # clear line
    print(caption, end='')
    if itteration<0: print(' failed')
    elif itteration<total: progress_bar(itteration, total, length)
    else: print(' done', end='')

def try_download_vulkan_headers():
    try: download_vulkan_headers()
    except (HTTPError, Timeout, ConnectionError) as e:  print(f"\nError during download:\n{e}")
    except zipfile.BadZipFile:                          print(f"\nThe downloaded file is not a valid ZIP archive.")
    except Exception as e:                              print(f"\nAn unexpected error occurred:\n{e}")

def download_vulkan_headers():
    url = "https://github.com/KhronosGroup/Vulkan-Headers/archive/refs/heads/main.zip"
    chunk_size = 65536  # 64 KB
    print("Download Vulkan headers:", end="")
    sys.stdout.flush()

    response = requests.get(url, stream = True)
    response.raise_for_status()                                  # Raise an error if the request failed
    total_size = int(response.headers.get('Content-Length', 0))  # Get the total file size (fails)
    total_size = max(total_size, chunk_size*37)                  # Fallback size estimate
    downloaded_size = 0
    zip_data = io.BytesIO()

    # Download the file in chunks and update the progress bar
    for chunk in response.iter_content(chunk_size=chunk_size):
        if not chunk: break
        zip_data.write(chunk)
        downloaded_size += len(chunk)
        #progress_bar(downloaded_size, total_size)
        show_progress('Download Vulkan headers:', downloaded_size, total_size)
    zip_data.seek(0)

    # Extract header files from zip data
    with zipfile.ZipFile(zip_data) as zip_file:
        for member in zip_file.namelist():
            src_path = "Vulkan-Headers-main/include/"
            if member.startswith(src_path) and len(member) > len(src_path):
                dst_path = member[len(src_path):]
                if dst_path.endswith("/"):
                    os.makedirs(dst_path[:-1], exist_ok=True)  # create directory
                else:
                    with open(dst_path, 'wb') as f:
                        f.write(zip_file.read(member))  # write file
    print()

class Proc(object):
    LOADER   = 0
    INSTANCE = 1
    DEVICE   = 2

    def __init__(self, name, dispatch):
        self.name = name
        self.dispatch = dispatch
        self.type = self.get_type()

    def get_type(self):
        if not self.dispatch:                                                                           return self.LOADER
        elif   self.dispatch in ["VkInstance", "VkPhysicalDevice"]:                                     return self.INSTANCE
        elif   self.dispatch in ["VkDevice", "VkQueue", "VkCommandBuffer", "VkExternalComputeQueueNV"]: return self.DEVICE
        else:  print(f"Warning: Unknown dispatch type: {self.dispatch}")

    def __repr__(self):
        return f"Proc(name={repr(self.name)}, dispatch={repr(self.dispatch)})"

class Extension(object):
    def __init__(self, name, guard=None, procs=[]):
        self.name = name
        self.guard = guard
        self.procs = procs[:]

    def __repr__(self):
        lines = [f"Extension(name={repr(self.name)}, guard={repr(self.guard)}, procs=["]
        for proc in self.procs:
            lines.append(f"    {repr(proc)}")
        lines.append("])")
        return "\n".join(lines)

def parse_results(f):
    global VkResult
    while True:
        line = next(f).strip()
        if line.startswith("}"): break
        if "=" in line:
            name, value = line.split("=", 1)
            name = name.strip()
            value = value.strip().rstrip(",")  # remove trailing comma
            try:
                VkResult[name] = int(value)
            except ValueError: continue
    return VkResult

def filter_exts(ext_types):
    filtered = []
    for ext in extensions:
        if any(type in ext.name for type in ext_types):
            #print("%s" % ext.name)
            filtered.append(ext)
        #else: print(FAINT+ "%s" % ext.name +RESET)
    return filtered

# Search for regex pattern.  If found, return the part in ().
def search(regex, line):
    match = re.search(regex, line)
    return match[1] if match else None

def parse_subheader(filename, ext_guard):
    global header_version
    sub_extensions = []
    if not os.path.exists(filename): return sub_extensions  # skip missing files

    with open(filename, "r") as f:
        current_ext = None
        for line in f:
            line = line.strip()
            if line.startswith("#define VK_HEADER_VERSION "):  # Header version number
                header_version = search(r' (\d+)', line)

            elif api_version := search(r'^#define VK_API_VERSION.+VK_MAKE_API_VERSION\(0, \d, (\d), 0\)', line):
                current_ext = Extension(f"VK_core_{api_version}")
                sub_extensions.append(current_ext)  # Add core extension

            elif name := search(r'^#define VK_\w+_EXTENSION_NAME\s+"(\w+)"$', line):  # Get extension name
                current_ext = Extension(name, ext_guard)
                sub_extensions.append(current_ext)

            elif name := search(r'^typedef .+?\*PFN_(vk(?!VoidFunction)\w+).*?\);', line):  # Get proc name
                disp = search(r'\)\((Vk\w+)', line)         # get dispatch type, if any
                current_ext.procs.append(Proc(name, disp))  # add proc to list for current extension

            elif "enum VkResult" in line:
                parse_results(f)

    return sub_extensions

def parse_vulkan_h(filename):
    extensions = []
    if not os.path.isfile(filename): return()

    path = os.path.dirname(filename)
    with open(filename, "r") as f:
        ext_guard = None
        for line in f:
            line = line.strip()
            if line.startswith("#include \"vulkan_"):
                fname = line[9:].replace('"', '') # extract filename
                extensions.extend(parse_subheader(os.path.join(path, fname), ext_guard))  # parse the file
            elif line.startswith("#ifdef VK_USE_PLATFORM") or line.startswith('#ifdef VK_ENABLE_BETA_EXTENSIONS'):
                ext_guard = line[7:]  # set guard
            elif ext_guard and line.startswith("#endif") and ext_guard in line:
                ext_guard = None # cancel guard
    return extensions

def generate_header(filename):
    guard = filename.upper() + "_H"
    cpp = textwrap.dedent(f"""\
        // Vulkan Extension Loader (Vexel)
        {License}
        // This file was generated by vexel_gen.py
        // For Vulkan header version: {header_version}
        
        #ifndef {guard}
        #define {guard}
        
        #ifdef __cplusplus
        extern "C" {{
        #endif
        
        #define VK_NO_PROTOTYPES 1
        #include <vulkan/vulkan.h>
        
        // Device specific dispatch table, used by vexLoadDeviceTable()
        struct vexDeviceTable;
        
        // Load Vulkan function pointers for runtime dispatch by the loader. (slow)
        // Returns true if Vulkan is available, false if not.
        bool vexInitialize();
        \n""")

    if Add_vexLoadDevice:
        cpp += textwrap.dedent(f"""\
        // Load Vulkan functions with device specific pointers, to skip runtime dispatch. (fast)
        // This makes Vulkan calls slightly faster, but limits Vulkan to a single GPU.
        void vexLoadDevice(VkDevice device);
        \n""")

    if Add_vexLoadDeviceTable:
        cpp += textwrap.dedent(f"""\
        // Load a 'vexDeviceTable' struct with device specific pointers. (fast)
        // Allows each GPU to have its own function table, to skip runtime dispatch.
        void vexLoadDeviceTable(VkDevice device, vexDeviceTable* table);
        \n""")

    if Add_vexVerifyExtensions:
        cpp += textwrap.dedent(f"""\
        // Verify all function pointers are loaded.
        // Warn if any null function pointers are found.
        void vexVerifyExtensions();
        \n""")

    if Add_VkResultToString:
        cpp += textwrap.dedent(f"""\
        // Many Vulkan functions return a VkResult enum code to indicate failure modes.
        // This converts VkResult to its string representation. (For printing debug messages.)
        const char* VkResultToString(VkResult result);
        \n""")

    prev_guard = None
    for ext in extensions:
        if ext.procs:
            if ext.guard != prev_guard:
                if prev_guard: cpp += "#endif\n"
                if ext.guard:  cpp += f"\n#ifdef {ext.guard}"
                prev_guard = ext.guard
            cpp += f"\n// {ext.name}\n"
            for proc in ext.procs:
                cpp += f"extern PFN_{proc.name} {proc.name};\n"
    if ext.guard: cpp += "#endif\n\n"

    if Add_vexLoadDeviceTable:
        cpp += "\nstruct vexDeviceTable {\n"
        for ext in extensions:
            if ext.procs and not ext.guard:
                for proc in ext.procs:
                    if proc.type == Proc.DEVICE:
                        cpp += f"    PFN_{proc.name} {proc.name};\n"
        cpp +="};"

    cpp += textwrap.dedent(f"""\n
        #ifdef __cplusplus
        }}
        #endif
        
        #endif  // {guard}    
        """)
    return cpp


def generate_source(filename):
    header = filename + ".h"
    cpp = textwrap.dedent(f"""\
        // Vulkan Extension Loader (Vexel)
        {License}
        // This file was generated by vexel_gen.py
        // For Vulkan header version: {header_version}
        
        #ifdef __cplusplus
        extern "C" {{
        #endif
        
        #ifdef _WIN32
            #define WIN32_LEAN_AND_MEAN
            #include <windows.h>
            FARPROC __stdcall dlsym(HMODULE lib, LPCSTR fn) {{ return GetProcAddress(lib, fn); }}
            HMODULE LoadVulkan(){{return LoadLibrary(\"vulkan-1.dll\");}}
        #else
            #include <dlfcn.h>
            void* LoadVulkan(){{
                void*    lib = dlopen(\"libvulkan.so\", RTLD_NOW | RTLD_LOCAL);
                if(!lib) lib = dlopen(\"libvulkan.so.1\", RTLD_NOW | RTLD_LOCAL);
                return lib;
            }}
        #endif
        
        #include "{header}"
        
        struct AutoRun{{AutoRun(){{ vexInitialize(); }}}}AutoRun;  // Run vexInitialize() at startup.
        
        bool vexInitialize(void) {{
            auto libvulkan = LoadVulkan();
            if (!libvulkan) return false;
        
            // Set Vulkan function pointers to use loader for dispatch
            #define LOAD(PROC) PROC = (PFN_##PROC) dlsym(libvulkan, #PROC);
        """)

    prev_guard = None
    for ext in extensions:
        if ext.procs:
            if ext.guard != prev_guard:
                if prev_guard: cpp += "#endif\n"
                if ext.guard:  cpp += f"\n#ifdef {ext.guard}\n"
                prev_guard = ext.guard

            for proc in ext.procs:
                cpp += f"    LOAD({proc.name})\n"

    if ext.guard: cpp += "#endif\n"

    cpp += textwrap.dedent("""\
        #undef LOAD
        return true;
        }
        \n""")

    if Add_vexLoadDevice:
        cpp += textwrap.dedent("""
            // Set Vulkan function pointers to use a specific device.
            void vexLoadDevice(VkDevice device) {
            #define LOAD_DEVICE(PROC) PROC = (PFN_##PROC) vkGetDeviceProcAddr(device, #PROC);
            """)

        for ext in extensions:
            if ext.procs and not ext.guard:
                for proc in ext.procs:
                    if proc.type == Proc.DEVICE:
                        cpp += f"    LOAD_DEVICE({proc.name})\n"

        cpp += textwrap.dedent("""\
            #undef LOAD_DEVICE
            }
            \n\n""")

    prev_guard = None
    for ext in extensions:
        if ext.procs:
            if ext.guard != prev_guard:
                if prev_guard: cpp += "#endif\n"
                if ext.guard:  cpp += f"\n#ifdef {ext.guard}\n"
                prev_guard = ext.guard

            for proc in ext.procs:
                cpp += f"PFN_{proc.name} {proc.name};\n"

    if ext.guard: cpp += "#endif\n"

    if Add_vexLoadDeviceTable:
        cpp += "\nvoid vexLoadDeviceTable(VkDevice device, vexDeviceTable* table) {\n"
        cpp += "# define LOAD_DEVICE_TABLE(PROC) table->PROC = (PFN_##PROC) vkGetDeviceProcAddr(device, #PROC);\n"
        for ext in extensions:
            if ext.procs and not ext.guard:
                for proc in ext.procs:
                    if proc.type == Proc.DEVICE:
                        cpp += f"    LOAD_DEVICE_TABLE({proc.name})\n"
        cpp += "#undef LOAD_DEVICE_TABLE\n"
        cpp += "}\n"

    if Add_vexVerifyExtensions:
        cpp += "\nvoid vexVerifyExtensions() {\n"
        cpp += "  printf(\"Uninitialized functions:\\n\");\n"
        for ext in extensions:
            if ext.procs and not ext.guard:
                cpp += f'    // {ext.name}\n'
                for proc in ext.procs:
                    cpp += f"    if(!{proc.name}) printf(\"  {proc.name}\\n\");\n"
        cpp += "}\n"

    if Add_VkResultToString:
        cpp += "\nconst char* VkResultToString(VkResult result) {\n"
        cpp += "# define STR(r) case r: return #r\n"
        cpp += "    switch(result) {\n"

        for name, value in VkResult.items():
            s = "" if value < 0 else " "
            cpp += f"        STR({name});".ljust(70) +f"// {s}{value}\n"
        cpp += '        default: return "UNKNOWN_RESULT";\n'
        cpp += "    }\n"
        cpp += "# undef STR\n"
        cpp += "}\n"

    cpp += textwrap.dedent("""\n\
        #ifdef __cplusplus
        }
        #endif
        """)
    return cpp

def print_extensions():
    find_vulkan_header()
    print("Included extensions:")
    for ext in extensions:
        print(ext.name)

def print_extensions_verbose():
    RED    = "\033[31m"
    GREEN  = "\033[32m"
    YELLOW = "\033[33m"
    BLUE   = "\033[34m"
    RESET  = "\033[0m"
    FAINT  = "\033[90m"

    find_vulkan_header()
    print("Included extensions:")
    for ext in extensions:
        line = f"{ext.name}"
        if ext.guard: line += f"  guard={ext.guard}"
        print(line)
        for proc in ext.procs:
            if   proc.type == Proc.LOADER:   print(f"{GREEN }    {proc.name} : LOADER{RESET}")
            elif proc.type == Proc.INSTANCE: print(f"{YELLOW}    {proc.name} : INSTANCE{RESET}")
            elif proc.type == Proc.DEVICE:   print(f"{BLUE  }    {proc.name} : DEVICE{RESET}")

def delete_vulkan_headers():
    "Delete vulkan and vk_video folders, and generated .h/.cpp files."
    shutil.rmtree('./vulkan',   ignore_errors=True)
    shutil.rmtree('./vk_video', ignore_errors=True)
    try: os.remove(name+'.h')
    except: pass
    try: os.remove(name+'.cpp')
    except: pass

def find_vulkan_header():
    if not os.path.isfile('vulkan/vulkan.h'):
        print("Vulkan header not found. (Use -d flag to download it.)")
        exit()

def generate_loader():
    find_vulkan_header()
    print("Generate Vulkan Extension Loader.")
    header = generate_header(name)
    source = generate_source(name)
    with open(name+".h", "w") as f:
        print(header, file=f)
    with open(name+".cpp", "w") as f:
        print(source, file=f)


if __name__ == "__main__":
    name = "vexel"
    heading = textwrap.dedent(f'''\
    Downloads the latest Vulkan headers and generates vexel.cpp/.h from it.
    Link those files to your project, to initialize Vulkan and Extensions.
    Includes all VK_KHR extensions by default. (Use --ext to add more.)
    ''')
    epilog = "Quick start: ./vexel_gen.py -dg"

    parser = argparse.ArgumentParser(description=heading, epilog=epilog, add_help=False, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('-h', '--help',     action='store_true', help='Show this help message and exit.')
    parser.add_argument('-c', '--clean',    action='store_true', help='Delete Vulkan headers and generated files.')
    parser.add_argument('-d', '--download', action='store_true', help='Download latest Vulkan headers.')
    parser.add_argument('-g', '--generate', action='store_true', help='Generate Extension Loader from Vulkan headers.')
    parser.add_argument('-l', '--list',     action='store_true', help='List included extensions.')
    parser.add_argument('-v', '--verbose',  action='store_true', help='List included extensions and their functions.')
    parser.add_argument('--ext', type=str, nargs='*', default=['VK_KHR'], help='Specify extension sets to include. (eg. VK_KHR VK_EXT)')
    args = parser.parse_args()

    if args.help or len(sys.argv) == 1:
        print('VEXEL: Vulkan_Extension_Loader Generator.')
        parser.print_help()
        exit()

    if args.clean:    delete_vulkan_headers()
    if args.download: try_download_vulkan_headers()

    extensions = parse_vulkan_h("vulkan/vulkan.h")
    extensions = filter_exts(["VK_core"] + args.ext)

    if args.generate: generate_loader()
    if args.list:     print_extensions()
    if args.verbose:  print_extensions_verbose()


