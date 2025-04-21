#!/usr/bin/env python3

from dataclasses import dataclass, field
from typing import Dict, Optional
from typing import cast, TextIO

# --- Constants ---

# Fields to extract from gamecontrollerdb.txt
FIELDS = [
    "a", "b", "x", "y", "leftshoulder", "rightshoulder", "leftstick", "rightstick",
    "dpup", "dpdown", "dpleft", "dpright", "back", "start", "guide",
    "leftx", "lefty", "rightx", "righty", "lefttrigger", "righttrigger", "platform"
]

# Buttons that define a layout group
LAYOUT_KEYS = [
    "a", "b", "x", "y", "leftshoulder", "rightshoulder", "leftstick", "rightstick",
    "dpup", "dpdown", "dpleft", "dpright",
    "leftx", "lefty", "rightx", "righty", "lefttrigger", "righttrigger"
]

# Keys to be displayed in output (layout + extra 2 keys)
DISPLAY_KEYS = LAYOUT_KEYS + ["back", "start"]

# --- Data Structures ---

@dataclass
class GameControllerMapping:
    GUID: str
    Name: str
    VID: int = 0
    PID: int = 0
    BUS: int = 0
    Mappings: Dict[str, Optional[str]] = field(default_factory=lambda: {btn: None for btn in FIELDS})

# --- Parser Functions ---

def parse_controller_line(line: str) -> Optional[GameControllerMapping]:
    if line.startswith('#') or not line.strip(): return None  # Skip comment lines

    parts = line.strip().split(',')
    if len(parts) < 20: return None  # Must have at least 20 tokens

    guid, name = parts[:2]
    vid = pid = bus = 0

    if len(guid) >= 32:  # Guid must have 32 digits
        bytes_list = bytes.fromhex(guid)
        vid = bytes_list[4] | (bytes_list[5] << 8)
        pid = bytes_list[8] | (bytes_list[9] << 8)
        bus = bytes_list[0]

    mapping_dict = {fld: None for fld in FIELDS}
    for mapping in parts[2:]:
        if ':' not in mapping: continue
        key, value = mapping.split(':', 1)
        if key in mapping_dict:
            mapping_dict[key] = value

    return GameControllerMapping(GUID=guid, Name=name, VID=vid, PID=pid, BUS=bus, Mappings=mapping_dict)

def read_gamecontrollerdb(file_path: str):
    mappings = []
    with open(file_path, 'r', encoding='utf-8') as f:
        for line in f:
            controller = parse_controller_line(line)
            if not controller: continue
            if controller.Mappings.get("platform") != "Linux": continue                   # Linux only
            if controller.BUS not in (3,5,6): continue                                    # BT/USB/Virtual only
            #if controller.BUS != 3: continue                                              # USB only
            #if controller.BUS != 5: continue                                              # Bluetooth only
            if any(controller.Mappings.get(key) is None for key in LAYOUT_KEYS): continue # Skip if missing keys
            mappings.append(controller)
    return mappings

# --- Analysis Function ---

def get_unique_layouts(controllers, top_n=20):
    layout_occurrences = {}

    # Group controllers by their layout tuple
    for controller in controllers:
        layout = tuple(controller.Mappings.get(key) for key in LAYOUT_KEYS)
        layout_occurrences.setdefault(layout, []).append(controller)
        #print(layout)

    # Sort by how many controllers share each layout
    all_layout_counts = sorted(
        ((layout, len(controllers)) for layout, controllers in layout_occurrences.items()),
        key=lambda x: x[1], reverse=True
    )

    # Build top layouts with extra 'back' and 'start' info
    top_layouts = []
    for layout, count in all_layout_counts[:top_n]:
        matching_controllers = layout_occurrences[layout]
        sample_controller = matching_controllers[0]
        back =  sample_controller.Mappings.get('back')
        start = sample_controller.Mappings.get('start')
        #label = sample_controller.Name
        extended_layout = layout + (back, start)

        # eliminate items with the same vid:pid:bus, but keep one name
        vidpid_dict = {}
        for c in matching_controllers:
            key = (c.VID, c.PID, c.BUS)
            if key not in vidpid_dict:
                 vidpid_dict[key] = c.Name
        vidpid_list = sorted((vid, pid, bus, name) for (vid, pid, bus), name in vidpid_dict.items())

        # vidpid_list = sorted((c.VID, c.PID, c.BUS, c.Name) for c in matching_controllers)
        #vidpid_list = sorted({(c.VID, c.PID, c.BUS, c.Name) for c in matching_controllers})  # eliminates duplicates

        cnt = len(vidpid_list)
        label = group_name(layout, vidpid_list)
        top_layouts.append((extended_layout, cnt, label, vidpid_list))

    return top_layouts

def group_name(layout, vidpid_list):
    #for vid, pid, bus, name in vidpid_list:
    #    if "Xbox 360 Controller" in name: return "XBox compatible";
    vid, pid, bus, name = vidpid_list[0]
    return name


def find_layouts_by_vidpid(controllers, vid_hex, pid_hex):
    vid = int(vid_hex, 16)
    pid = int(pid_hex, 16)

    layouts = {}
    for controller in controllers:
        if controller.VID == vid and controller.PID == pid:
            layout = tuple(controller.Mappings.get(key) for key in LAYOUT_KEYS)
            layouts.setdefault(layout, []).append(controller.Name)

    print(f"Found {len(layouts)} unique layouts for {vid_hex}:{pid_hex}")
    for i, (layout, names) in enumerate(layouts.items(), 1):
        print(f"\nLayout #{i} used by {len(names)} controller(s):")
        for name in names:
            print(f"  {name}")

def find_layout_id_by_vidpid(vid_hex, pid_hex):
    vid_val = int(vid_hex, 16)
    pid_val = int(pid_hex, 16)
    for i, (layout, count, label, vidpid_list) in enumerate(top_layouts, 0):
        for j, (vid, pid, bus, name) in enumerate(vidpid_list, 0):
            if vid == vid_val and pid == pid_val :
                #print(f"{name}:{i}")
                return i

def print_detail(top_layouts):
    print(f"\nTop {len(top_layouts)} unique button layouts:")
    for i, (layout, count, label, vidpid_list) in enumerate(top_layouts, 1):
    #for i, (layout, count, vidpid_list) in enumerate(top_layouts, 1):
        print(f"\nLayout #{i} (Used by {count} controllers):")
        #print(f"Example controller: {label}")

        for key in DISPLAY_KEYS:
            print(f"  {key:13} -> {layout[DISPLAY_KEYS.index(key)]}")

        # Summary line
        #for key in DISPLAY_KEYS:
        #    val = layout[DISPLAY_KEYS.index(key)]
        #    print(f"{val.split('.')[0] if val else '??'}", end=" ")
        #print("\n")

        print("VID:PID list:")
        for vid, pid, bus, name in vidpid_list:
            print(f"  {vid:04x}:{pid:04x}:{bus:01x} : {name}")

# noinspection PyTypeChecker
def write_header(top_layouts):
    with open("gamepads.h", "w") as f:
        print("// Gamepad button layout lookup table for Linux.",file=f)
        print("// Data derived from gamecontrollerdb.txt", file=f)
        print("// (Controllers with missing buttons were discarded.)", file=f)
        print("", file=f)
        print("", file=f)
        print("#ifndef GAMEPAD_H",file=f)
        print("#define GAMEPAD_H",file=f)
        print(file=f)
        print("#include <array>",file=f)
        print("#include <cstdint>",file=f)
        print(file=f)
        print("constexpr std::array gamepad_layout_list = {",file=f)
        print("//   A   B   X   Y   LB  RB  LS  RS  UP  DN  LE  RI  THUMBL  THUMBR  TRIGER  SEL START",file=f)

        for i, (layout, count, label, vidpid_list) in enumerate(top_layouts, 0):
            print(f'    "', end="",file=f)
            for key in DISPLAY_KEYS:
                val = layout[DISPLAY_KEYS.index(key)]
                if key == 'dpup'    and val == 'h0.4': val = '-h'
                if key == 'dpdown'  and val == 'h0.1': val = '-h'
                if key == 'dpleft'  and val == 'h0.2': val = '-h'
                if key == 'dpright' and val == 'h0.8': val = '-h'
                val = val.lstrip('+') if val else " "
                val = val.split('.')[0] if val else " "
                if val == ' ': val = '__'
                if val == 'h0': val = 'h'
                print(f"{val:<3}", end=" ",file=f)
            print(f'",  // {i}',file=f)
            #print(f'",  // {i}    {label}',file=f)
        print("};",file=f)

        # List gamepad vid:pid:inx
        print(file=f)
        print("static struct gamepad_index {",file=f)
        print("    uint16_t VID;     // Vendor ID",file=f)
        print("    uint16_t PID;     // Product ID",file=f)
        print("    uint8_t  BUS;     // USB(3) or Bluetooth(5)", file=f)
        print("    uint8_t  inx;     // Layout index",file=f)
        print("} gamepad_index[] = {",file=f)

        for i, (layout, count, label, vidpid_list) in enumerate(top_layouts, 0):
            for j,(vid, pid, bus, name) in enumerate(vidpid_list, 0):
                bus_str = {3: "USB", 5: "BT ", 6: "VRT"}.get(bus, "   ")
                print(f"    {{0x{vid:04x},0x{pid:04x},{bus},{i:2d}}},  // {bus_str}: {name}", file=f)
        print("};",file=f)
        print(file=f)

        SwitchPro = find_layout_id_by_vidpid("057e", "2009")  # Nintendo Switch Pro Controller

        print("static struct gamepad_by_name {",file=f)
        print("    char name[32];       // Name string",file=f)
        print("    uint8_t  inx;        // Layout index",file=f)
        print("} gamepad_by_name[] = {",file=f)
        print(f'    {{"Lic Pro Controller", {SwitchPro}}},',file=f)
        print("};",file=f)
        print(file=f)
        print("//------ Encode gamepad layout to binary at compile-time ------",file=f)
        print("constexpr std::array<uint8_t, 20> encodeLine(const char* line) {",file=f)
        print("    std::array<uint8_t, 20> data = {};",file=f)
        print("for(auto& d:data) d=0xff;",file=f)
        print(file=f)
        print("int i=0;",file=f)
        print("char c = *line;",file=f)
        print("while(*line && i<data.size()) {",file=f)
        print("    uint8_t flags = 0;",file=f)
        print("    while(c==' ') {          c=*++line;}  // white-space",file=f)
        print("    if(c=='-') {flags|=0x80; c=*++line;}  // flip axis",file=f)
        print("    if(c=='b') {flags&=0x1F; c=*++line;}  // is button",file=f)
        print("    if(c=='a') {flags|=0x20; c=*++line;}  // is axis",file=f)
        print("    if(c=='h') {flags|=0x40; c=*++line;}  // is HAT",file=f)
        print("    uint8_t num=0;",file=f)
        print("    while(c>='0' && c<='9') {",file=f)
        print("        num=num*10+(c-'0');",file=f)
        print("        c=*++line;",file=f)
        print("    }",file=f)
        print("    data[i++]=num|flags;",file=f)
        print("}",file=f)
        print("return data;",file=f)
        print("}",file=f)
        print(file=f)
        print("template<std::size_t N>",file=f)
        print("constexpr auto encodeLines(const std::array<const char*, N>& lines) {",file=f)
        print("std::array<std::array<uint8_t, 20>, N> result = {};",file=f)
        print("    for (std::size_t i = 0; i < N; ++i) {",file=f)
        print("        result[i] = encodeLine(lines[i]);",file=f)
        print("    }",file=f)
        print("    return result;",file=f)
        print("}",file=f)
        print(file=f)
        print("constexpr auto gamepad_layouts = encodeLines(gamepad_layout_list);",file=f)
        print("//-------------------------------------------------------------",file=f)
        print(file=f)
        print("static const std::array<uint8_t, 20>* get_gamepad_layout(uint16_t VID, uint16_t PID, uint8_t BUS, const char* name) {",file=f)
        print("    const std::array<uint8_t, 20>* layout = nullptr;",file=f)
        print("    for (const auto& entry : gamepad_index)",file=f)
        print("        if (entry.VID == VID && entry.PID == PID && entry.BUS == BUS)",file=f)
        print("            layout = &gamepad_layouts[entry.inx];",file=f)
        print("        if(layout) return layout;",file=f)
        print("    for (const auto& entry : gamepad_by_name)",file=f)
        print("        if (strstr(name, entry.name))",file=f)
        print("            layout = &gamepad_layouts[entry.inx];",file=f)
        print("        if(layout) return layout;",file=f)
        print("    return nullptr;",file=f)
        print("}",file=f)
        print(file=f)
        print("#endif",file=f)


# --- Main Program ---

if __name__ == "__main__":
    db_path = "gamecontrollerdb.txt"
    controllers = read_gamecontrollerdb(db_path)
    print(f"Total number of controllers: {len(controllers)}")

    top_layouts = get_unique_layouts(controllers, top_n=256)
    print_detail(top_layouts)
    write_header(top_layouts)