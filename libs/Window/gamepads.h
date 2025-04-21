// Gamepad button layout lookup table for Linux.
// Data derived from gamecontrollerdb.txt
// (Controllers with missing buttons were discarded.)


#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <array>
#include <cstdint>

constexpr std::array gamepad_layout_list = {
//   A   B   X   Y   LB  RB  LS  RS  UP  DN  LE  RI  THUMBL  THUMBR  TRIGER  SEL START
    "b0  b1  b2  b3  b4  b5  b9  b10 h   h   h   h   a0  a1  a3  a4  a2  a5  b6  b7  ",  // 0
    "b1  b2  b0  b3  b4  b5  b10 b11 h   h   h   h   a0  a1  a2  a3  b6  b7  b8  b9  ",  // 1
    "b0  b1  b3  b4  b6  b7  b13 b14 h   h   h   h   a0  a1  a2  a3  a5  a4  b10 b11 ",  // 2
    "b1  b2  b0  b3  b4  b5  b10 b11 h   h   h   h   a0  a1  a2  a5  a3  a4  b8  b9  ",  // 3
    "b1  b0  b4  b3  b6  b7  b13 b14 h   h   h   h   a0  a1  a2  a3  b8  b9  b10 b11 ",  // 4
    "b2  b1  b3  b0  b4  b5  b10 b11 h   h   h   h   a0  a1  a2  a3  b6  b7  b8  b9  ",  // 5
    "b0  b1  b3  b2  b4  b5  b11 b12 h   h   h   h   a0  a1  a3  a4  a2  a5  b8  b9  ",  // 6
    "b0  b1  b2  b3  b4  b5  b10 b11 h   h   h   h   a0  a1  a2  a3  b6  b7  b8  b9  ",  // 7
    "b0  b1  b3  b4  b6  b7  b13 b14 h   h   h   h   a0  a1  a2  a3  b8  b9  b10 b11 ",  // 8
    "b1  b0  b4  b3  b6  b7  b13 b14 h   h   h   h   a0  a1  a2  a3  a5  a4  b10 b11 ",  // 9
    "b0  b1  b2  b3  b4  b5  b7  b8  h   h   h   h   a0  a1  a2  a3  a5  a4  b9  b6  ",  // 10
    "b2  b3  b0  b1  b4  b6  b10 b11 h   h   h   h   a0  a1  a3  a2  b5  b7  b8  b9  ",  // 11
    "b1  b2  b0  b3  b4  b5  b10 b11 h   h   h   h   a0  a1  a2  a5  b6  b7  b8  b9  ",  // 12
    "b14 b13 b15 b12 b10 b11 b1  b2  b4  b6  b7  b5  a0  a1  a2  a3  b8  b9  b0  b3  ",  // 13
    "b0  b1  b2  b3  b4  b5  b9  b10 b13 b14 b11 b12 a0  a1  a3  a4  a2  a5  b6  b7  ",  // 14
    "b0  b2  b1  b3  b4  b6  b10 b11 h   h   h   h   a0  a1  a2  a3  b5  b7  b8  b9  ",  // 15
    "b0  b1  b3  b2  b4  b5  b9  b10 h   h   h   h   a0  a1  a3  a4  a2  a5  b6  b7  ",  // 16
    "b0  b1  b2  b3  b4  b5  b9  b10 h   h   h   h   a0  a1  a2  a3  a5  a4  b6  b7  ",  // 17
    "b2  b3  b0  b1  b4  b5  b8  b9  h   h   h   h   a0  a1  a3  a2  b6  b7  b10 b11 ",  // 18
    "b0  b1  b3  b4  b5  b2  b8  b9  h   h   h   h   a0  a1  a3  a4  a2  a5  b6  b7  ",  // 19
    "b0  b1  b3  b2  b4  b5  b11 b12 b13 b14 b15 b16 a0  a1  a3  a4  a2  a5  b8  b9  ",  // 20
    "b11 b12 b13 b14 b17 b18 b21 b22 b5  b6  b3  b4  a0  a1  a3  a4  a5  a2  b15 b16 ",  // 21
    "b1  b0  b4  b3  b6  b7  b13 b14 h   h   h   h   a0  a1  a3  a4  b8  b9  b10 b11 ",  // 22
    "b0  b1  b3  b4  b6  b7  b12 b13 h   h   h   h   a0  a1  a2  a3  a5  a4  b10 b11 ",  // 23
    "b0  b1  b2  b3  b4  b5  b11 b12 h   h   h   h   a0  a1  a2  a3  b6  b7  b8  b9  ",  // 24
    "b0  b1  b3  b4  b6  b7  b13 b14 h   h   h   h   a0  a1  a2  a3  a6  a5  b17 b11 ",  // 25
    "b0  b1  b3  b2  b5  b6  b12 b13 b14 b15 b16 b17 a0  a1  a2  a3  b7  b8  b9  b10 ",  // 26
    "b0  b1  b2  b3  b4  b5  b8  b9  h   h   h   h   a0  a1  a2  a5  a3  a4  b6  b7  ",  // 27
    "b2  b1  b3  b0  b6  b7  b10 b11 h   h   h   h   a0  a1  a3  a2  b4  b5  b8  b9  ",  // 28
    "b1  b0  b4  b3  b6  b7  b13 b14 h   h   h   h   a0  a1  a2  a3  a4  a5  b10 b11 ",  // 29
    "b1  b2  b0  b3  b4  b5  b10 b11 h   h   h   h   a0  a1  a3  a2  b6  b7  b8  b9  ",  // 30
    "b0  b1  b2  b3  b4  b5  b10 b11 h   h   h   h   a0  a1  a2  a3  a5  a4  b8  b9  ",  // 31
    "b2  b1  b3  b0  b6  b7  b10 b11 h   h   h   h   a0  a1  a2  a3  b4  b5  b9  b8  ",  // 32
    "b0  b1  b3  b2  b5  b6  b12 b13 h   h   h   h   a0  a1  a2  a3  b7  b8  b9  b10 ",  // 33
    "b0  b1  b2  b3  b4  b5  b7  b8  h   h   h   h   a0  a1  a2  a5  a3  a4  b14 b6  ",  // 34
    "b0  b3  b1  b2  b4  b5  b6  b7  b8  b9  b10 b11 a0  a1  a3  a4  a2  a5  __  __  ",  // 35
    "b1  b2  b0  b3  b4  b5  b10 b11 h   h   h   h   a0  a1  a2  a4  b6  b7  b8  b9  ",  // 36
    "b3  b4  b5  b6  b7  b8  b14 b15 b16 b17 b18 b19 a0  a1  a2  a3  a9  a8  b11 b12 ",  // 37
    "b1  b0  b4  b3  b6  b7  b13 b14 h   h   h   h   a0  a1  a2  a3  a5  a5  b10 b11 ",  // 38
    "b0  b1  b2  b3  b4  b5  b10 b11 h   h   h   h   a0  a1  a2  a5  a3  a4  b6  b7  ",  // 39
    "b1  b0  b3  b4  b6  b7  b13 b14 h   h   h   h   a0  a1  a2  a4  b8  b9  b10 b11 ",  // 40
    "b0  b1  b2  b3  b4  b5  b6  b7  h   h   h   h   a0  a1  a2  a3  a5  a4  b9  b8  ",  // 41
    "b0  b1  b2  b3  b4  b5  b6  b7  h   h   h   h   a0  a1  a2  a3  a5  -a4 b9  b8  ",  // 42
    "b1  b2  b0  b3  b4  b5  b10 b11 h   h   h   h   a0  a1  a2  a4  a3  -a3 b8  b9  ",  // 43
    "b1  b0  b3  b2  b4  b5  b9  b10 h   h   h   h   a0  a1  a3  a4  a2  a5  b6  b7  ",  // 44
    "b2  b3  b0  b1  b4  b5  b8  b9  h   h   h   h   a0  a1  a2  a3  b6  b7  b10 b11 ",  // 45
    "b2  b1  b3  b0  b6  b7  b10 b11 b12 b14 b15 b13 a0  a1  a5  a2  b4  b5  b8  b9  ",  // 46
    "b0  b1  b3  b4  b6  b7  b13 b16 h   h   h   h   a0  a1  a2  a3  b8  b9  b10 b11 ",  // 47
    "b2  b1  b3  b0  b6  b7  b9  b10 h   h   h   h   a0  a1  a3  a2  b4  b5  b8  b11 ",  // 48
    "b0  b1  b2  b3  b4  b6  b10 b11 h   h   h   h   a0  a1  a3  a2  b5  b7  b8  b9  ",  // 49
    "b0  b1  b2  b3  b4  b5  b9  b10 h   h   h   h   a0  a1  a3  a4  a2  a5  b6  b7  ",  // 50
    "b3  b4  b0  b1  b6  b7  b2  b5  h   h   h   h   a0  a1  a2  a3  b8  b9  b10 b11 ",  // 51
    "b2  b1  b3  b0  b4  b5  b10 b11 h   h   h   h   a0  a1  a3  a2  b6  b7  b8  b9  ",  // 52
    "b0  b1  b2  b3  b4  b5  b10 b11 h   h   h   h   a0  a1  a3  a2  b6  b7  b8  b9  ",  // 53
    "b2  b3  b0  b1  b4  b6  b10 b11 h   h   h   h   a0  a1  a2  a3  b5  b7  b8  b9  ",  // 54
    "b0  b1  b2  b3  b4  b5  b9  b10 -h  -h  -h  -h  a0  a1  a3  a4  a2  a5  b6  b7  ",  // 55
    "b0  b1  b2  b3  b4  b5  b9  b10 b12 b13 b14 b15 a0  a1  a3  a4  a2  a5  b6  b7  ",  // 56
    "b1  b0  b3  b2  b9  b10 b7  b8  b11 b12 b13 b14 a0  a1  a2  a3  a4  a5  b4  b6  ",  // 57
    "b0  b1  b3  b2  b4  b5  b11 b12 b13 b14 b15 b16 a0  a1  a2  a3  b6  b7  b8  b9  ",  // 58
    "b0  b1  b2  b3  b4  b6  b12 b11 h   h   h   h   a0  a1  a3  a2  b5  b7  b8  b10 ",  // 59
    "b0  b1  b2  b3  b4  b5  b8  b9  h   h   h   h   a0  a1  a2  a3  a5  a4  __  b7  ",  // 60
    "b0  b1  b2  b3  b4  b5  b11 b12 h   h   h   h   a0  a1  a3  a2  b6  b7  b8  b9  ",  // 61
    "b0  b3  b1  b2  b4  b5  b6  b7  b8  b9  b10 b11 a0  a1  a3  a4  b12 b13 b14 b16 ",  // 62
    "b0  b1  b2  b3  b4  b5  b8  b9  h   h   h   h   a0  a1  a2  a3  b6  b7  b10 b11 ",  // 63
    "b0  b1  b2  b3  b4  b5  b9  b10 b14 b13 b14 b13 a0  a1  a3  a4  a2  a5  b6  b7  ",  // 64
    "b1  b0  b2  b3  b4  b5  b10 b11 h   h   h   h   a0  a1  a2  a3  b6  b7  b8  b9  ",  // 65
    "b14 b13 b15 b12 b10 b11 b1  b2  b4  b6  b7  b5  a0  a1  a2  a3  a12 a13 b0  b3  ",  // 66
    "b1  b2  b0  b3  b4  b5  b10 b11 h   h   h   h   a0  a1  a2  a5  b6  a4  b8  b9  ",  // 67
    "b2  b3  b1  b0  b4  b5  b10 b11 h   h   h   h   a0  a1  a2  a3  b6  b7  b8  b9  ",  // 68
    "b1  b2  b0  b3  b6  b7  b10 b11 h   h   h   h   a0  a1  a3  a2  b4  b5  b8  b12 ",  // 69
    "b1  b2  b0  b3  b4  b5  b10 b11 h   h   h   h   a0  a1  a3  a4  a2  b7  b8  b9  ",  // 70
    "b2  b1  b3  b0  b6  b7  b9  b10 b12 b14 b15 b13 a0  a1  a2  a3  b4  b5  b8  b11 ",  // 71
    "b2  b3  b4  b5  b6  b7  b13 b14 -a5 a5  -a4 a4  a0  a1  a2  a3  a7  a6  b10 b11 ",  // 72
    "b2  b1  b3  b0  b6  b7  b10 b11 b12 b14 b15 b13 a0  a1  a2  a3  b4  b5  b9  b8  ",  // 73
    "b0  b1  b3  b2  b4  b5  b10 b11 h   h   h   h   a0  a1  a5  a2  b6  b7  b8  b9  ",  // 74
    "b0  b2  b1  b3  b4  b6  b10 b11 h   h   h   h   a0  a1  a2  a3  b8  b9  __  __  ",  // 75
    "b0  b2  b1  b3  b4  b6  b11 b12 h   h   h   h   a0  a1  a2  a3  b5  b7  b9  b10 ",  // 76
    "b2  b1  b3  b0  b4  b5  b10 b11 h   h   h   h   a0  a1  a3  a4  b6  b7  b8  b9  ",  // 77
    "b0  b1  b2  b3  b6  b7  b10 b11 h   h   h   h   a0  a1  a3  a2  b4  b5  b8  b9  ",  // 78
    "b1  b0  b3  b2  b4  b5  b11 b12 h   h   h   h   a0  a1  a2  a3  b6  b7  b8  b9  ",  // 79
    "b0  b1  b2  b3  b4  b5  b8  b9  h   h   h   h   a0  a1  a3  a4  a2  a5  b6  b7  ",  // 80
    "b0  b1  b3  b4  b6  b7  b13 b14 h   h   h   h   a0  a1  a2  a5  a7  a6  b10 b11 ",  // 81
};

static struct gamepad_index {
    uint16_t VID;     // Vendor ID
    uint16_t PID;     // Product ID
    uint8_t  BUS;     // USB(3) or Bluetooth(5)
    uint8_t  inx;     // Layout index
} gamepad_index[] = {
    {0x0079,0x18d4,3, 0},  // USB: GPD Win 2 Controller
    {0x044f,0xb326,3, 0},  // USB: Thrustmaster GP XID
    {0x045e,0x028e,3, 0},  // USB: Be1 GC101 Xbox 360
    {0x045e,0x028e,5, 0},  // BT : Microsoft Xbox One Elite 2
    {0x045e,0x02a1,3, 0},  // USB: Xbox 360 Controller
    {0x045e,0x02d1,3, 0},  // USB: Microsoft Xbox One
    {0x045e,0x02dd,3, 0},  // USB: Microsoft Xbox One
    {0x045e,0x02e3,3, 0},  // USB: Microsoft Xbox One Elite
    {0x045e,0x02ea,3, 0},  // USB: Microsoft Xbox One
    {0x045e,0x02ea,6, 0},  // VRT: Xbox One S Controller
    {0x045e,0x02fd,5, 0},  // BT : Xbox One Controller
    {0x045e,0x0b00,3, 0},  // USB: Microsoft Xbox One Elite 2
    {0x045e,0x0b12,3, 0},  // USB: Microsoft Xbox Series Controller
    {0x045e,0x0b12,6, 0},  // VRT: Microsoft Xbox One
    {0x046d,0xc21d,3, 0},  // USB: Logitech F310
    {0x046d,0xc21e,3, 0},  // USB: Logitech F510
    {0x046d,0xc21f,3, 0},  // USB: Logitech F710
    {0x0738,0x4716,3, 0},  // USB: Mad Catz Xbox 360 Controller
    {0x0e6f,0x011f,3, 0},  // USB: Rock Candy
    {0x0e6f,0x0131,3, 0},  // USB: PDP EA Sports Controller
    {0x0e6f,0x0139,3, 0},  // USB: Afterglow Prismatic Controller
    {0x0e6f,0x0164,3, 0},  // USB: PDP Battlefield One
    {0x0e6f,0x0213,3, 0},  // USB: Afterglow Xbox 360 Controller
    {0x0e6f,0x02a7,3, 0},  // USB: PDP Xbox One Raven Black
    {0x0e6f,0x02b8,3, 0},  // USB: PDP Afterglow Xbox One Controller
    {0x0e6f,0x02c8,3, 0},  // USB: PDP Kingdom Hearts Controller
    {0x0e6f,0x02d8,3, 0},  // USB: PDP Xbox Series Controller
    {0x0e6f,0x02ef,3, 0},  // USB: PDP Xbox Series Kinetic Wired Controller
    {0x0e6f,0x02f1,3, 0},  // USB: PDP Xbox Atomic
    {0x0e6f,0x0301,3, 0},  // USB: Logic3 Controller
    {0x0e6f,0x0315,3, 0},  // USB: Xbox 360 Controller
    {0x0e6f,0x0401,3, 0},  // USB: Gamestop Logic3 Controller
    {0x0e6f,0x0413,3, 0},  // USB: Xbox Controller
    {0x0f0d,0x0067,3, 0},  // USB: Horipad One
    {0x0f0d,0x0150,3, 0},  // USB: Hori Fighting Commander Octa Xbox One
    {0x0f0d,0x0185,3, 0},  // USB: Hori Split Pad Fit
    {0x1038,0x1430,3, 0},  // USB: SteelSeries Stratus Duo
    {0x1038,0x1431,3, 0},  // USB: SteelSeries Stratus Duo
    {0x10f5,0x7008,6, 0},  // VRT: Turtle Beach Recon
    {0x146b,0x0609,3, 0},  // USB: Nacon Asymmetric Wireless PS4 Controller
    {0x1532,0x0a03,3, 0},  // USB: Razer Wildcat
    {0x1532,0x0a14,3, 0},  // USB: Razer Wolverine Ultimate Xbox
    {0x1689,0xfe00,3, 0},  // USB: Razer Sabertooth
    {0x1bad,0xf016,3, 0},  // USB: Mad Catz Xbox 360 Controller
    {0x1bad,0xf501,3, 0},  // USB: Hori Pad EX Turbo 2
    {0x20d6,0x2002,3, 0},  // USB: PowerA Xbox One Controller
    {0x20d6,0x2005,3, 0},  // USB: PowerA Xbox Series Controller
    {0x20d6,0x200b,3, 0},  // USB: PowerA Xbox Series Controller
    {0x20d6,0x200f,3, 0},  // USB: PowerA Xbox Series Controller
    {0x20d6,0x2802,3, 0},  // USB: PowerA Xbox One Controller
    {0x20d6,0x4001,3, 0},  // USB: PowerA Fusion Pro 2 Controller
    {0x20d6,0x4002,3, 0},  // USB: PowerA Xbox One Spectra Infinity
    {0x20d6,0x4005,3, 0},  // USB: PowerA Advantage Xbox Series Controller
    {0x24c6,0x5300,3, 0},  // USB: PowerA
    {0x24c6,0x531a,3, 0},  // USB: PowerA Mini Pro Ex
    {0x24c6,0x541a,3, 0},  // USB: PowerA Xbox One Mini Controller
    {0x24c6,0x543a,3, 0},  // USB: PowerA 1428124-01
    {0x24c6,0x581a,3, 0},  // USB: PowerA Xbox One
    {0x24c6,0x5b02,3, 0},  // USB: Thrustmaster GPX
    {0x24c6,0x5d04,3, 0},  // USB: Razer Sabertooth
    {0x24c6,0xfafe,3, 0},  // USB: Rock Candy Xbox 360 Controller
    {0x28de,0x11ff,3, 0},  // USB: Steam Virtual Gamepad
    {0x294b,0x3004,3, 0},  // USB: Snakebyte Xbox Series Controller
    {0x2dc8,0x2000,3, 0},  // USB: 8BitDo Pro 2 for Xbox
    {0x2dc8,0x2000,6, 0},  // VRT: 8BitDo Pro 2 for Xbox
    {0x2dc8,0x3106,3, 0},  // USB: 8BitDo Adapter 2
    {0x2dc8,0x310a,3, 0},  // USB: 8BitDo Ultimate 2C
    {0x2e24,0x1688,3, 0},  // USB: Hyperkin X91
    {0x2f24,0x0091,3, 0},  // USB: EasySMX ESM-9101
    {0x2f24,0x00f7,3, 0},  // USB: Mayflash Magic S Pro
    {0x3285,0x0607,3, 0},  // USB: Nacon GC-100
    {0xdead,0xbeef,6, 0},  // VRT: Hidromancer Controller
    {0x0079,0x1800,3, 1},  // USB: Mayflash Wii U Pro Adapter
    {0x0079,0x181a,3, 1},  // USB: Venom PS4 Arcade Joystick
    {0x0079,0x18d2,3, 1},  // USB: Mayflash Magic NS
    {0x044f,0xd007,3, 1},  // USB: Thrustmaster T Mini
    {0x044f,0xd009,3, 1},  // USB: Thrustmaster Run N Drive PlayStation Controller
    {0x046d,0xc216,3, 1},  // USB: Logitech Dual Action
    {0x046d,0xc218,3, 1},  // USB: Logitech RumblePad 2
    {0x046d,0xc219,3, 1},  // USB: Logitech Cordless RumblePad 2
    {0x046d,0xcad1,3, 1},  // USB: Logitech Chillstream
    {0x046d,0xcad2,3, 1},  // USB: Precision Controller
    {0x0738,0x3180,3, 1},  // USB: Mad Catz FightStick Alpha PS3
    {0x0738,0x3250,3, 1},  // USB: Mad Catz Fightpad Pro PS3
    {0x0738,0x3384,3, 1},  // USB: Mad Catz Fightstick TE S PS3
    {0x0738,0x5266,5, 1},  // BT : Mad Catz CTRLR
    {0x0738,0x8180,3, 1},  // USB: Mad Catz FightStick Alpha PS4
    {0x0c12,0x0e21,3, 1},  // USB: Brook Mars PS4 Controller
    {0x0c12,0x0e30,3, 1},  // USB: Brook Audio Fighting Board PS3
    {0x0e6f,0x011e,3, 1},  // USB: Rock Candy PS3 Controller
    {0x0e6f,0x0128,3, 1},  // USB: PDP PS3 Rock Candy Controller
    {0x0e6f,0x012f,3, 1},  // USB: PDP Wired PS3 Controller
    {0x0e6f,0x0130,3, 1},  // USB: EA Sports PS3 Controller
    {0x0e6f,0x0180,3, 1},  // USB: Faceoff Pro Nintendo Switch Controller
    {0x0e6f,0x0181,3, 1},  // USB: Faceoff Deluxe Pro Nintendo Switch Controller
    {0x0e6f,0x0184,3, 1},  // USB: Faceoff Deluxe Nintendo Switch Controller
    {0x0e6f,0x0187,3, 1},  // USB: Rock Candy Nintendo Switch Controller
    {0x0e6f,0x0188,3, 1},  // USB: Afterglow Deluxe Nintendo Switch Controller
    {0x0e6f,0x0214,3, 1},  // USB: PS3 Controller
    {0x0e8f,0x310d,3, 1},  // USB: SZMY Power 3 Turbo
    {0x0e8f,0x3114,3, 1},  // USB: SZMY Power PS3
    {0x0f0d,0x0009,3, 1},  // USB: Natec Genesis P44
    {0x0f0d,0x0011,3, 1},  // USB: Hori Real Arcade Pro 3
    {0x0f0d,0x004d,3, 1},  // USB: Hori Gem Pad 3
    {0x0f0d,0x006b,3, 1},  // USB: Hori Real Arcade Pro 4
    {0x0f0d,0x006e,3, 1},  // USB: Horipad 4 PS3
    {0x0f0d,0x0085,3, 1},  // USB: Hori Fighting Commander PS3
    {0x0f0d,0x00c1,3, 1},  // USB: Horipad Nintendo Switch Controller
    {0x11c0,0x5503,3, 1},  // USB: Acrux Gamepad
    {0x12bd,0xc003,3, 1},  // USB: Joypad Alpha Shock
    {0x146b,0x0902,3, 1},  // USB: Bigben
    {0x1532,0x0402,3, 1},  // USB: Razer Panthera PS3
    {0x1a34,0x0836,3, 1},  // USB: PS3 Controller
    {0x20bc,0x5656,3, 1},  // USB: GameSir T4w
    {0x20d6,0xa710,3, 1},  // USB: Mayflash Magic NS
    {0x20d6,0xa711,3, 1},  // USB: PowerA Core Controller
    {0x20d6,0xa712,3, 1},  // USB: PowerA Fusion Nintendo Switch Fight Pad
    {0x20d6,0xa713,3, 1},  // USB: PowerA Nintendo Switch Controller
    {0x20d6,0xa714,3, 1},  // USB: PowerA Spectra Nintendo Switch Controller
    {0x20d6,0xca6d,3, 1},  // USB: PowerA Pro Ex
    {0x2185,0x0102,3, 1},  // USB: Final Fantasy XIV Online Controller
    {0x25f0,0x83c1,3, 1},  // USB: Goodbetterbest Controller
    {0x2c22,0x2302,3, 1},  // USB: Qanba Obsidian Arcade Joystick PS3
    {0x2c22,0x2502,3, 1},  // USB: Qanba Dragon Arcade Joystick PS3
    {0x62dd,0xa715,3, 1},  // USB: PowerA Fusion Nintendo Switch Arcade Stick
    {0x62dd,0xa716,3, 1},  // USB: PowerA Fusion Pro Nintendo Switch Controller
    {0x6469,0x6469,5, 1},  // BT : idroidcon Controller
    {0x0111,0x1419,5, 2},  // BT : SteelSeries Stratus XL
    {0x0111,0x1431,5, 2},  // BT : SteelSeries Stratus Duo
    {0x03f0,0x038d,3, 2},  // USB: HyperX Clutch
    {0x045e,0x02fd,5, 2},  // BT : Xbox One Controller
    {0x045e,0x0b13,3, 2},  // USB: Xbox Series Controller
    {0x045e,0x0b13,5, 2},  // BT : Xbox Series Controller
    {0x045e,0x0b20,5, 2},  // BT : Xbox Wireless Controller
    {0x045e,0x0b22,5, 2},  // BT : Xbox One Elite 2 Controller
    {0x0502,0x1309,3, 2},  // USB: Anbernic RG P01
    {0x05ac,0x022d,5, 2},  // BT : GameSir G4s
    {0x05ac,0x061a,3, 2},  // USB: GameSir-T3 2.02
    {0x0b05,0x7905,3, 2},  // USB: ASUS ROG Kunai 3
    {0x0b05,0x7906,5, 2},  // BT : ASUS ROG Kunai 3
    {0x0f0d,0x0196,5, 2},  // BT : Horipad Steam
    {0x0f0d,0x01ab,3, 2},  // USB: Horipad Steam
    {0x11c3,0x9107,3, 2},  // USB: Be1 GC101 Controller 1.03
    {0x1532,0x0705,3, 2},  // USB: Razer Raiju Mobile
    {0x1915,0x7856,3, 2},  // USB: Uniplay U6
    {0x1949,0x0402,5, 2},  // BT : Amazon Fire Controller
    {0x20bc,0x5500,3, 2},  // USB: GameSir G3w
    {0x24c6,0x891a,5, 2},  // BT : MOGA XP5X Plus
    {0x24c6,0x891b,3, 2},  // USB: BDA MOGA XP5X Plus
    {0x24c6,0x892a,5, 2},  // BT : MOGA XP5A Plus
    {0x24c6,0x892b,3, 2},  // USB: MOGA XP5A Plus
    {0x2563,0x0526,3, 2},  // USB: Shanwan Gamepad
    {0x27f8,0x0bbf,3, 2},  // USB: Razer Kishi
    {0x2dc8,0x2101,3, 2},  // USB: 8BitDo Xbox One SN30 Pro
    {0x2dc8,0x2101,5, 2},  // BT : 8BitDo Xbox One SN30 Pro
    {0x2dc8,0x3011,3, 2},  // USB: 8BitDo Ultimate Wired
    {0x2dc8,0x3012,3, 2},  // USB: 8BitDo Ultimate Wireless
    {0x2dc8,0x3012,5, 2},  // BT : 8BitDo Ultimate
    {0x2dc8,0x3013,3, 2},  // USB: 8BitDo Ultimate Wireless
    {0x2dc8,0x3015,3, 2},  // USB: 8BitDo Ultimate C
    {0x2dc8,0x3016,3, 2},  // USB: 8BitDo Ultimate C
    {0x2dc8,0x3017,3, 2},  // USB: 8BitDo Ultimate C
    {0x2dc8,0x301b,5, 2},  // BT : 8BitDo Ultimate 2C
    {0x2dc8,0x301d,3, 2},  // USB: 8BitDo Ultimate 2C
    {0x3285,0x0305,5, 2},  // BT : Nacon MG-X Pro
    {0x3537,0x1007,3, 2},  // USB: Anbernic RG P01
    {0x3537,0x1046,5, 2},  // BT : Anbernic RG P01
    {0x358a,0x0102,3, 2},  // USB: Backbone One
    {0x358a,0x0202,3, 2},  // USB: Backbone One
    {0x358a,0x0203,3, 2},  // USB: Backbone One
    {0x358a,0x0204,3, 2},  // USB: Backbone One
    {0x4f4d,0x4554,5, 2},  // BT : Mocute 054X
    {0x0079,0x181b,3, 3},  // USB: Venom PS4 Arcade Joystick
    {0x044f,0xd00e,3, 3},  // USB: Thrustmaster eSwap Pro Controller
    {0x054c,0x05c4,3, 3},  // USB: PS4 Controller
    {0x054c,0x05c4,5, 3},  // BT : PS4 Controller
    {0x054c,0x09cc,3, 3},  // USB: PS4 Controller
    {0x054c,0x09cc,5, 3},  // BT : PS4 Controller
    {0x054c,0x0ba0,3, 3},  // USB: PS4 Controller
    {0x054c,0x0ce6,3, 3},  // USB: PS5 Controller
    {0x054c,0x0ce6,5, 3},  // BT : PS5 Controller
    {0x054c,0x0df2,3, 3},  // USB: PS5 Controller
    {0x054c,0x0df2,5, 3},  // BT : PS5 Controller
    {0x054c,0x0e5f,3, 3},  // USB: PS5 Access Controller
    {0x0738,0x8250,3, 3},  // USB: Mad Catz Fightpad Pro PS4
    {0x0738,0x8384,3, 3},  // USB: Mad Catz Fightstick TE S PS4
    {0x0c12,0x0e10,3, 3},  // USB: Zeroplus P4
    {0x0c12,0x0e20,3, 3},  // USB: Brook Mars PS4 Controller
    {0x0c12,0x0e31,3, 3},  // USB: Brook Audio Fighting Board PS4
    {0x0c12,0x1e10,3, 3},  // USB: Zeroplus P4
    {0x0f0d,0x0066,3, 3},  // USB: Horipad 4 PS4
    {0x0f0d,0x006a,3, 3},  // USB: Hori Real Arcade Pro 4
    {0x0f0d,0x0084,3, 3},  // USB: Hori Fighting Commander
    {0x0f0d,0x00ee,3, 3},  // USB: Horipad Mini 4
    {0x146b,0x0d01,3, 3},  // USB: Revolution Pro Controller
    {0x146b,0x0d13,3, 3},  // USB: Revolution Pro Controller 3
    {0x1532,0x0401,3, 3},  // USB: Razer Panthera PS4
    {0x1532,0x1000,3, 3},  // USB: Razer Raiju
    {0x1532,0x1008,3, 3},  // USB: Razer Panthera PS4 Evo Arcade Stick
    {0x1532,0x100a,5, 3},  // BT : Razer Raiju Tournament Edition
    {0x1532,0x100b,3, 3},  // USB: Razer Wolverine PS5 Controller
    {0x1532,0x1100,3, 3},  // USB: Razer Raion PS4 Fightpad
    {0x20d6,0x792a,3, 3},  // USB: BDA PS4 Fightpad
    {0x2c22,0x2300,3, 3},  // USB: Qanba Obsidian Arcade Joystick PS4
    {0x2c22,0x2500,3, 3},  // USB: Qanba Dragon Arcade Joystick PS4
    {0x3285,0x0d17,3, 3},  // USB: Nacon Revolution 5 Pro
    {0x3285,0x0d19,3, 3},  // USB: Nacon Revolution 5 Pro
    {0x7545,0x0104,3, 3},  // USB: SZMY Power DS4 Wired Controller
    {0x1002,0x9000,3, 4},  // USB: 8BitDo FC30 Pro
    {0x2002,0x9000,3, 4},  // USB: 8BitDo NES30 Pro
    {0x2dc8,0x2862,5, 4},  // BT : 8BitDo SN30 Pro
    {0x2dc8,0x2865,5, 4},  // BT : 8BitDo N30 Pro 2
    {0x2dc8,0x3101,3, 4},  // USB: 8BitDo Receiver
    {0x2dc8,0x3102,3, 4},  // USB: 8BitDo Receiver
    {0x2dc8,0x3103,3, 4},  // USB: 8BitDo Receiver
    {0x2dc8,0x3104,3, 4},  // USB: 8BitDo Receiver
    {0x2dc8,0x3810,5, 4},  // BT : 8BitDo FC30 Pro
    {0x2dc8,0x5111,3, 4},  // USB: 8BitDo Lite SE
    {0x2dc8,0x5111,5, 4},  // BT : 8BitDo Lite SE
    {0x2dc8,0x6000,3, 4},  // USB: 8BitDo SF30 Pro
    {0x2dc8,0x6000,5, 4},  // BT : 8BitDo SF30 Pro
    {0x2dc8,0x6001,3, 4},  // USB: 8BitDo SN30 Pro
    {0x2dc8,0x6002,3, 4},  // USB: 8BitDo SN30 Pro Plus
    {0x2dc8,0x6101,5, 4},  // BT : 8BitDo SN30 Pro
    {0x2dc8,0x6102,5, 4},  // BT : 8BitDo SN30 Pro Plus
    {0x2dc8,0x9015,3, 4},  // USB: 8BitDo N30 Pro 2
    {0x3820,0x0009,5, 4},  // BT : 8BitDo NES30 Pro
    {0x05ac,0x055b,3, 5},  // USB: GameSir G3w
    {0x0e8f,0x0003,3, 5},  // USB: PS3 Controller
    {0x0e8f,0x0008,3, 5},  // USB: Gasia PlayStation Gamepad
    {0x0f0d,0x00aa,3, 5},  // USB: Hori Real Arcade Pro for Nintendo Switch
    {0x11c0,0x9105,3, 5},  // USB: Torid
    {0x11c1,0x9101,3, 5},  // USB: EasySMX
    {0x11c2,0x9107,3, 5},  // USB: Be1 GC101 Controller 1.03
    {0x11ff,0x3331,3, 5},  // USB: PC Controller
    {0x145f,0x01c5,3, 5},  // USB: Trust Gamepad
    {0x145f,0x0231,3, 5},  // USB: PS3 Controller
    {0x20bc,0x1264,3, 5},  // USB: Betop Controller
    {0x20e8,0x5860,3, 5},  // USB: Cideko AK08b
    {0x2563,0x0523,3, 5},  // USB: ShanWan Gamepad
    {0x2563,0x0575,3, 5},  // USB: Ipega PG 9099
    {0x25f0,0x83c3,3, 5},  // USB: GT VX2
    {0x25f0,0xc121,3, 5},  // USB: Shanwan Gioteck PS3 Controller
    {0x2f24,0x002d,3, 5},  // USB: JYS Adapter
    {0x7545,0x1122,3, 5},  // USB: SZMY Power Gamepad
    {0x054c,0x05c4,3, 6},  // USB: PS4 Controller
    {0x054c,0x05c4,5, 6},  // BT : PS4 Controller
    {0x054c,0x09cc,3, 6},  // USB: PS4 Controller
    {0x054c,0x09cc,5, 6},  // BT : PS4 Controller
    {0x054c,0x0ba0,3, 6},  // USB: PS4 Controller
    {0x054c,0x0ce6,3, 6},  // USB: PS5 Controller
    {0x054c,0x0ce6,5, 6},  // BT : PS5 Controller
    {0x054c,0x0df2,3, 6},  // USB: PS5 Controller
    {0x054c,0x0df2,5, 6},  // BT : PS5 Controller
    {0x057e,0x2009,5, 7},  // BT : Nintendo Switch Pro Controller
    {0x0c45,0x4320,3, 7},  // USB: XEOX SL6556 BK
    {0x0f0d,0x00f6,5, 7},  // BT : Horipad Switch Pro Controller
    {0x11c9,0x55f0,3, 7},  // USB: HJC Gamepad
    {0x1345,0x1000,3, 7},  // USB: Genius Maxfire Grandias 12
    {0x146b,0x0c01,3, 7},  // USB: Nacon GC 400ES
    {0x1a34,0x0809,3, 7},  // USB: SL6566
    {0x4f4d,0x4554,5, 7},  // BT : Mocute 053X
    {0x5347,0x6d61,5, 7},  // BT : GameStop Gamepad
    {0x694c,0x7250,5, 7},  // BT : Nintendo Switch Controller
    {0x0079,0x181c,3, 8},  // USB: Mobapad Chitu HD
    {0x04e8,0x046e,5, 8},  // BT : Mocute 053X M59
    {0x1949,0x0403,5, 8},  // BT : Ipega PG9099
    {0x20bc,0x504d,3, 8},  // USB: Beitong A1T2 BFM
    {0x20bc,0x5500,5, 8},  // BT : Betop AX1 BFM
    {0x2dc8,0x3100,3, 8},  // USB: 8BitDo Adapter
    {0x8555,0x061b,3, 8},  // USB: GameSir G4 Pro
    {0x2dc8,0x3010,3, 9},  // USB: 8BitDo Pro 2
    {0x2dc8,0x3820,5, 9},  // BT : 8BitDo NES30 Pro
    {0x2dc8,0x5112,5, 9},  // BT : 8BitDo Lite 2
    {0x2dc8,0x6006,3, 9},  // USB: 8BitDo Pro 2
    {0x2dc8,0x6006,5, 9},  // BT : 8BitDo Pro 2
    {0x2dc8,0x6007,3, 9},  // USB: 8BitDo Ultimate Wireless
    {0x2dc8,0x6100,5, 9},  // BT : 8BitDo SF30 Pro
    {0x0171,0x0419,5,10},  // BT : Amazon Luna Controller
    {0x0b05,0x4500,5,10},  // BT : ASUS Gamepad
    {0x20d6,0x0dad,5,10},  // BT : Moga Pro
    {0x20d6,0x6271,5,10},  // BT : Moga Pro 2
    {0x20d6,0x89e5,5,10},  // BT : Moga 2
    {0x07b5,0x0312,3,11},  // USB: Mega World Logic 3 Controller
    {0x07b5,0x0315,3,11},  // USB: Impact
    {0x0f30,0x0110,3,11},  // USB: Jess Tech Dual Analog Rumble
    {0x0f30,0x0111,3,11},  // USB: Jess Tech Colour Rumble Pad
    {0x0f30,0x0112,3,11},  // USB: Saitek P380
    {0x044f,0xb323,3,12},  // USB: Thrustmaster Dual Trigger PlayStation Controller
    {0x044f,0xd008,3,12},  // USB: Thrustmaster Run N Drive PlayStation Controller
    {0x11c0,0x4001,3,12},  // USB: PS4 Controller
    {0x1345,0x3008,3,12},  // USB: NYKO CORE
    {0x2c22,0x2010,3,12},  // USB: Qanba Drone 2 Arcade Joystick PS5
    {0x054c,0x0268,3,13},  // USB: PS3 Controller
    {0x054c,0x0268,5,13},  // BT : PS3 Controller
    {0x054c,0x0268,6,13},  // VRT: PS3 Controller
    {0x4c50,0x5453,5,13},  // BT : PS3 Controller
    {0x045e,0x0291,3,14},  // USB: Xbox 360 Controller
    {0x045e,0x02a1,3,14},  // USB: Xbox 360 Controller
    {0x045e,0x0719,3,14},  // USB: Xbox 360 Controller
    {0x1689,0xfd01,3,14},  // USB: Razer Onza Classic Edition
    {0x044f,0xb304,3,15},  // USB: Thrustmaster Firestorm Dual Power
    {0x044f,0xb312,3,15},  // USB: Thrustmaster Vibrating Gamepad
    {0x044f,0xb315,3,15},  // USB: Thrustmaster Dual Analog 3.2
    {0x044f,0xb320,3,15},  // USB: Thrustmaster Dual Trigger
    {0x3250,0x1002,3,16},  // USB: Atari VCS Modern Controller
    {0x3250,0x1002,5,16},  // BT : Atari VCS Modern Controller
    {0x1532,0x0900,3,17},  // USB: Razer Serval
    {0x1532,0x0900,5,17},  // BT : Razer Serval
    {0x18d1,0x9400,3,17},  // USB: Google Stadia Controller
    {0x18d1,0x9400,5,17},  // BT : Google Stadia Controller
    {0x05ef,0x0003,3,18},  // USB: InterAct AxisPad
    {0x06a3,0x0109,3,18},  // USB: Saitek P880
    {0x06a3,0xff0c,3,18},  // USB: Saitek P2500 Force Rumble
    {0x0c12,0x0005,3,18},  // USB: InterAct AxisPad
    {0x045e,0x0202,3,19},  // USB: Xbox Controller
    {0x045e,0x0285,3,19},  // USB: Microsoft Xbox
    {0x045e,0x0289,3,19},  // USB: Microsoft Xbox
    {0xffff,0xffff,3,19},  // USB: Xbox Controller
    {0x054c,0x0268,3,20},  // USB: PS3 Controller
    {0x054c,0x0268,5,20},  // BT : PS3 Controller
    {0x0378,0x0003,3,21},  // USB: TRBot Virtual Joypad
    {0x06f0,0x0003,3,21},  // USB: TRBot Virtual Joypad
    {0x0a68,0x0003,3,21},  // USB: TRBot Virtual Joypad
    {0x0de0,0x0003,3,21},  // USB: TRBot Virtual Joypad
    {0x2dc8,0x6001,3,22},  // USB: 8BitDo SN30 Pro
    {0x2dc8,0x6101,3,22},  // USB: 8BitDo SN30 Pro
    {0x2dc8,0x9012,3,22},  // USB: 8BitDo SN30 Pro
    {0x04b4,0x2411,3,23},  // USB: Flydigi Vader 2
    {0x04b4,0x2412,3,23},  // USB: Flydigi Vader 2
    {0x1915,0x0040,5,23},  // BT : Flydigi Vader 2
    {0x0001,0x0001,5,24},  // BT : Nintendo Wii Remote
    {0x0e6f,0x02a8,3,24},  // USB: PDP Xbox One Controller
    {0x0f0d,0x0086,3,24},  // USB: Hori Fighting Commander Xbox 360
    {0x045e,0x02e3,5,25},  // BT : Xbox One Elite
    {0x045e,0x0b05,5,25},  // BT : Microsoft Xbox One Elite 2
    {0x057e,0x2006,6,26},  // VRT: Nintendo Switch Combined Joy-Cons
    {0x057e,0x2008,6,26},  // VRT: Nintendo Switch Combined Joy-Cons
    {0x694e,0x6e65,6,26},  // VRT: Nintendo Switch Combined Joy-Cons
    {0x2e95,0x434b,3,27},  // USB: Scuf Envision
    {0x2e95,0x434d,3,27},  // USB: Scuf Envision
    {0x2e95,0x434e,3,27},  // USB: Scuf Envision
    {0x04d9,0x0f16,3,28},  // USB: Sony PlayStation Controller Adapter
    {0x0810,0x0001,3,28},  // USB: Twin PS2 Adapter
    {0x0810,0x0003,3,28},  // USB: USB Gamepad
    {0x2dc8,0x9000,3,29},  // USB: 8BitDo FC30 Pro
    {0x2dc8,0x9001,3,29},  // USB: 8BitDo NES30 Pro
    {0x06a3,0x040b,3,30},  // USB: Saitek P990 Dual Analog
    {0x187c,0x0600,3,30},  // USB: Alienware Dual Compatible Game PlayStation Controller
    {0x04b4,0x2412,5,31},  // BT : Flydigi APEX 4
    {0x4f43,0x4e41,5,31},  // BT : VX Gaming Command Series
    {0x0925,0x0005,3,32},  // USB: Sony PS2 pad with SmartJoy Adapter
    {0x0925,0x8866,3,32},  // USB: MP8866 Super Dual Box
    {0x057e,0x2009,3,33},  // USB: Nintendo Switch Pro Controller
    {0x057e,0x2009,5,33},  // BT : Nintendo Switch Pro Controller
    {0x0955,0x7214,3,34},  // USB: NVIDIA Controller
    {0x0955,0x7214,5,34},  // BT : NVIDIA Controller
    {0x2836,0x0001,5,35},  // BT : OUYA Controller
    {0x06a3,0xf620,3,36},  // USB: Saitek PS2700 Rumble
    {0x06a3,0xf623,3,36},  // USB: Saitek Cyborg PlayStation Controller
    {0x28de,0x1205,3,37},  // USB: Steam Deck
    {0x2dc8,0x5112,3,38},  // USB: 8BitDo Lite 2
    {0x1949,0x0419,3,39},  // USB: Amazon Luna Controller
    {0x1008,0x01e5,3,40},  // USB: Anbernic Handheld
    {0x3250,0x1002,3,41},  // USB: Atari VCS Modern Controller
    {0x3250,0x1002,5,42},  // BT : Atari VCS Modern Controller
    {0x06a3,0xf622,3,43},  // USB: Cyborg V3 Rumble
    {0x045e,0x028e,3,44},  // USB: Data Frog S80
    {0x056e,0x2003,3,45},  // USB: Elecom U3613M
    {0x0b43,0x0003,3,46},  // USB: EMS Production PS2 Adapter
    {0x05ac,0x057a,3,47},  // USB: GameSir G5
    {0x0e8f,0x1006,3,48},  // USB: GreenAsia Electronics Controller
    {0x0e8f,0x0012,3,49},  // USB: GreenAsia Joystick
    {0x0f0d,0x006d,3,50},  // USB: Hori EDGE 301
    {0x05fd,0x262a,3,51},  // USB: InterAct HammerHead FX
    {0x0f30,0x010b,3,52},  // USB: Jess Tech GGE909 PC Recoil
    {0x0c12,0x0005,3,53},  // USB: Manta DualShock 2
    {0x07b5,0x004f,3,54},  // USB: Mega World Logic 3 Controller
    {0x045e,0x028e,3,55},  // USB: Microsoft Xbox 360
    {0x045e,0x028e,3,56},  // USB: MSI GC20 V2
    {0x057e,0x2009,3,57},  // USB: Nintendo Switch Pro Controller
    {0x057e,0x0330,5,58},  // BT : Nintendo Wii U Pro Controller
    {0x050d,0x0803,3,59},  // USB: Nostromo n45 Dual Analog
    {0x0955,0x7210,3,60},  // USB: NVIDIA Controller
    {0x124b,0x4d01,3,61},  // USB: NYKO Airflo EX
    {0x2836,0x0001,5,62},  // BT : OUYA Controller
    {0x0583,0x2050,3,63},  // USB: Padix Rockfire PlayStation Bridge
    {0x0e6f,0x02d7,3,64},  // USB: PDP Black Camo Wired Xbox Series Controller
    {0x0e6f,0x0185,3,65},  // USB: PDP Fightpad Pro Gamecube Controller
    {0x054c,0x0268,5,66},  // BT : PS3 Controller
    {0x2c22,0x2012,3,67},  // USB: Qanba Drone 2 Arcade Joystick PS4
    {0x0e6f,0x1113,3,68},  // USB: Saffun Controller
    {0x06a3,0x040c,3,69},  // USB: Saitek P2900
    {0x06a3,0xf518,3,70},  // USB: Saitek P3200 Rumble
    {0x6666,0x0667,3,71},  // USB: Sony PlayStation Adapter
    {0x28de,0x1201,3,72},  // USB: Steam Controller
    {0x6666,0x8804,3,73},  // USB: Super Joy Box 5 Pro
    {0x22ba,0x0107,3,74},  // USB: Technology Innovation PS2 Adapter
    {0x044f,0xb303,3,75},  // USB: Thrustmaster Firestorm Dual Analog 2
    {0x044f,0xb300,3,76},  // USB: Thrustmaster Firestorm Dual Power
    {0x0079,0x0006,3,77},  // USB: USB gamepad
    {0x05ac,0x3232,5,78},  // BT : VR Box Controller
    {0x045e,0x0b0a,3,79},  // USB: Xbox One Controller
    {0x045e,0x02e0,5,80},  // BT : Xbox One Controller
    {0x2717,0x3144,5,81},  // BT : XiaoMi Controller
};

static struct gamepad_by_name {
    char name[32];       // Name string
    uint8_t  inx;        // Layout index
} gamepad_by_name[] = {
    {"Lic Pro Controller", 7},
};

//------ Encode gamepad layout to binary at compile-time ------
constexpr std::array<uint8_t, 20> encodeLine(const char* line) {
    std::array<uint8_t, 20> data = {};
for(auto& d:data) d=0xff;

int i=0;
char c = *line;
while(*line && i<data.size()) {
    uint8_t flags = 0;
    while(c==' ') {          c=*++line;}  // white-space
    if(c=='-') {flags|=0x80; c=*++line;}  // flip axis
    if(c=='b') {flags&=0x1F; c=*++line;}  // is button
    if(c=='a') {flags|=0x20; c=*++line;}  // is axis
    if(c=='h') {flags|=0x40; c=*++line;}  // is HAT
    uint8_t num=0;
    while(c>='0' && c<='9') {
        num=num*10+(c-'0');
        c=*++line;
    }
    data[i++]=num|flags;
}
return data;
}

template<std::size_t N>
constexpr auto encodeLines(const std::array<const char*, N>& lines) {
std::array<std::array<uint8_t, 20>, N> result = {};
    for (std::size_t i = 0; i < N; ++i) {
        result[i] = encodeLine(lines[i]);
    }
    return result;
}

constexpr auto gamepad_layouts = encodeLines(gamepad_layout_list);
//-------------------------------------------------------------

static const std::array<uint8_t, 20>* get_gamepad_layout(uint16_t VID, uint16_t PID, uint8_t BUS, const char* name) {
    const std::array<uint8_t, 20>* layout = nullptr;
    for (const auto& entry : gamepad_index)
        if (entry.VID == VID && entry.PID == PID && entry.BUS == BUS)
            layout = &gamepad_layouts[entry.inx];
        if(layout) return layout;
    for (const auto& entry : gamepad_by_name)
        if (strstr(name, entry.name))
            layout = &gamepad_layouts[entry.inx];
        if(layout) return layout;
    return nullptr;
}

#endif
