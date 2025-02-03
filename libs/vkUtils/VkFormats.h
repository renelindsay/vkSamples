// Use FormatInfo(...) to get info on a given VkFormat.

#ifndef VKFORMATS_H
#define VKFORMATS_H

#include <map>
#include <string>
#include "vulkan/vulkan.h"

//#include "Logging.h"
#include "CImage.h"

//#define EXTENDED_FORMATS
#define UINT _UINT

enum format_types{UNDEFINED=0, UNORM=1, SNORM=2, SRGB=3, UINT=4, SINT=5, USCALED=6, SSCALED=7, UFLOAT=8, SFLOAT=9};

enum format_Flag_bits {
    SWIZZLED_BIT   = 0x01,
    COMPRESSED_BIT = 0x02,
    DEPTH_BIT      = 0x04,
    STENCIL_BIT    = 0x08,
    YUV_BIT        = 0x10
};
typedef char format_type;
typedef char format_flags;

struct format_info {  // (4 bytes)
    char     size;
    char     channels;
    format_type  type;
    format_flags flags;

    bool isSwizzled()  {return flags & SWIZZLED_BIT;  }
    bool isCompressed(){return flags & COMPRESSED_BIT;}
    bool hasDepth()    {return flags & DEPTH_BIT;     }
    bool hasStencil()  {return flags & STENCIL_BIT;   }
    bool isYUV()       {return flags & YUV_BIT;       }
};

const std::map<VkFormat, format_info> vk_formats = {
    {VK_FORMAT_UNDEFINED,                          { 0, 0, UNDEFINED, 0}},
    {VK_FORMAT_R4G4_UNORM_PACK8,                   { 1, 2, UNORM    , 0}},
    {VK_FORMAT_R4G4B4A4_UNORM_PACK16,              { 2, 4, UNORM    , 0}},
    {VK_FORMAT_B4G4R4A4_UNORM_PACK16,              { 2, 4, UNORM    , SWIZZLED_BIT}},
    {VK_FORMAT_R5G6B5_UNORM_PACK16,                { 2, 3, UNORM    , 0}},
    {VK_FORMAT_B5G6R5_UNORM_PACK16,                { 2, 3, UNORM    , SWIZZLED_BIT}},
    {VK_FORMAT_R5G5B5A1_UNORM_PACK16,              { 2, 4, UNORM    , 0}},
    {VK_FORMAT_B5G5R5A1_UNORM_PACK16,              { 2, 4, UNORM    , SWIZZLED_BIT}},
    {VK_FORMAT_A1R5G5B5_UNORM_PACK16,              { 2, 4, UNORM    , 0}},
    {VK_FORMAT_R8_UNORM,                           { 1, 1, UNORM    , 0}},
    {VK_FORMAT_R8_SNORM,                           { 1, 1, SNORM    , 0}},
    {VK_FORMAT_R8_USCALED,                         { 1, 1, USCALED  , 0}},
    {VK_FORMAT_R8_SSCALED,                         { 1, 1, SSCALED  , 0}},
    {VK_FORMAT_R8_UINT,                            { 1, 1, UINT     , 0}},
    {VK_FORMAT_R8_SINT,                            { 1, 1, SINT     , 0}},
    {VK_FORMAT_R8_SRGB,                            { 1, 1, SRGB     , 0}},
    {VK_FORMAT_R8G8_UNORM,                         { 2, 2, UNORM    , 0}},
    {VK_FORMAT_R8G8_SNORM,                         { 2, 2, SNORM    , 0}},
    {VK_FORMAT_R8G8_USCALED,                       { 2, 2, USCALED  , 0}},
    {VK_FORMAT_R8G8_SSCALED,                       { 2, 2, SSCALED  , 0}},
    {VK_FORMAT_R8G8_UINT,                          { 2, 2, UINT     , 0}},
    {VK_FORMAT_R8G8_SINT,                          { 2, 2, SINT     , 0}},
    {VK_FORMAT_R8G8_SRGB,                          { 2, 2, SRGB     , 0}},
    {VK_FORMAT_R8G8B8_UNORM,                       { 3, 3, UNORM    , 0}},
    {VK_FORMAT_R8G8B8_SNORM,                       { 3, 3, SNORM    , 0}},
    {VK_FORMAT_R8G8B8_USCALED,                     { 3, 3, USCALED  , 0}},
    {VK_FORMAT_R8G8B8_SSCALED,                     { 3, 3, USCALED  , 0}},
    {VK_FORMAT_R8G8B8_UINT,                        { 3, 3, UINT     , 0}},
    {VK_FORMAT_R8G8B8_SINT,                        { 3, 3, SINT     , 0}},
    {VK_FORMAT_R8G8B8_SRGB,                        { 3, 3, SRGB     , 0}},
    {VK_FORMAT_B8G8R8_UNORM,                       { 3, 3, UNORM    , SWIZZLED_BIT}},
    {VK_FORMAT_B8G8R8_SNORM,                       { 3, 3, SNORM    , SWIZZLED_BIT}},
    {VK_FORMAT_B8G8R8_USCALED,                     { 3, 3, USCALED  , SWIZZLED_BIT}},
    {VK_FORMAT_B8G8R8_SSCALED,                     { 3, 3, SSCALED  , SWIZZLED_BIT}},
    {VK_FORMAT_B8G8R8_UINT,                        { 3, 3, UINT     , SWIZZLED_BIT}},
    {VK_FORMAT_B8G8R8_SINT,                        { 3, 3, SINT     , SWIZZLED_BIT}},
    {VK_FORMAT_B8G8R8_SRGB,                        { 3, 3, SRGB     , SWIZZLED_BIT}},
    {VK_FORMAT_R8G8B8A8_UNORM,                     { 4, 4, UNORM    , 0}},
    {VK_FORMAT_R8G8B8A8_SNORM,                     { 4, 4, SNORM    , 0}},
    {VK_FORMAT_R8G8B8A8_USCALED,                   { 4, 4, USCALED  , 0}},
    {VK_FORMAT_R8G8B8A8_SSCALED,                   { 4, 4, SSCALED  , 0}},
    {VK_FORMAT_R8G8B8A8_UINT,                      { 4, 4, UINT     , 0}},
    {VK_FORMAT_R8G8B8A8_SINT,                      { 4, 4, SINT     , 0}},
    {VK_FORMAT_R8G8B8A8_SRGB,                      { 4, 4, SRGB     , 0}},
    {VK_FORMAT_B8G8R8A8_UNORM,                     { 4, 4, UNORM    , SWIZZLED_BIT}},
    {VK_FORMAT_B8G8R8A8_SNORM,                     { 4, 4, SNORM    , SWIZZLED_BIT}},
    {VK_FORMAT_B8G8R8A8_USCALED,                   { 4, 4, USCALED  , SWIZZLED_BIT}},
    {VK_FORMAT_B8G8R8A8_SSCALED,                   { 4, 4, SSCALED  , SWIZZLED_BIT}},
    {VK_FORMAT_B8G8R8A8_UINT,                      { 4, 4, UINT     , SWIZZLED_BIT}},
    {VK_FORMAT_B8G8R8A8_SINT,                      { 4, 4, SINT     , SWIZZLED_BIT}},
    {VK_FORMAT_B8G8R8A8_SRGB,                      { 4, 4, SRGB     , SWIZZLED_BIT}},
    {VK_FORMAT_A8B8G8R8_UNORM_PACK32,              { 4, 4, UNORM    , 0}},
    {VK_FORMAT_A8B8G8R8_SNORM_PACK32,              { 4, 4, SNORM    , 0}},
    {VK_FORMAT_A8B8G8R8_USCALED_PACK32,            { 4, 4, USCALED  , 0}},
    {VK_FORMAT_A8B8G8R8_SSCALED_PACK32,            { 4, 4, SSCALED  , 0}},
    {VK_FORMAT_A8B8G8R8_UINT_PACK32,               { 4, 4, UINT     , 0}},
    {VK_FORMAT_A8B8G8R8_SINT_PACK32,               { 4, 4, SINT     , 0}},
    {VK_FORMAT_A8B8G8R8_SRGB_PACK32,               { 4, 4, SRGB     , 0}},
    {VK_FORMAT_A2R10G10B10_UNORM_PACK32,           { 4, 4, UNORM    , 0}},
    {VK_FORMAT_A2R10G10B10_SNORM_PACK32,           { 4, 4, SNORM    , 0}},
    {VK_FORMAT_A2R10G10B10_USCALED_PACK32,         { 4, 4, USCALED  , 0}},
    {VK_FORMAT_A2R10G10B10_SSCALED_PACK32,         { 4, 4, SSCALED  , 0}},
    {VK_FORMAT_A2R10G10B10_UINT_PACK32,            { 4, 4, UINT     , 0}},
    {VK_FORMAT_A2R10G10B10_SINT_PACK32,            { 4, 4, SINT     , 0}},
    {VK_FORMAT_A2B10G10R10_UNORM_PACK32,           { 4, 4, UNORM    , SWIZZLED_BIT}},
    {VK_FORMAT_A2B10G10R10_SNORM_PACK32,           { 4, 4, SNORM    , SWIZZLED_BIT}},
    {VK_FORMAT_A2B10G10R10_USCALED_PACK32,         { 4, 4, USCALED  , SWIZZLED_BIT}},
    {VK_FORMAT_A2B10G10R10_SSCALED_PACK32,         { 4, 4, SSCALED  , SWIZZLED_BIT}},
    {VK_FORMAT_A2B10G10R10_UINT_PACK32,            { 4, 4, UINT     , SWIZZLED_BIT}},
    {VK_FORMAT_A2B10G10R10_SINT_PACK32,            { 4, 4, SINT     , SWIZZLED_BIT}},
    {VK_FORMAT_R16_UNORM,                          { 2, 1, UNORM    , 0}},
    {VK_FORMAT_R16_SNORM,                          { 2, 1, SNORM    , 0}},
    {VK_FORMAT_R16_USCALED,                        { 2, 1, USCALED  , 0}},
    {VK_FORMAT_R16_SSCALED,                        { 2, 1, SSCALED  , 0}},
    {VK_FORMAT_R16_UINT,                           { 2, 1, UINT     , 0}},
    {VK_FORMAT_R16_SINT,                           { 2, 1, SINT     , 0}},
    {VK_FORMAT_R16_SFLOAT,                         { 2, 1, SFLOAT   , 0}},
    {VK_FORMAT_R16G16_UNORM,                       { 4, 2, UNORM    , 0}},
    {VK_FORMAT_R16G16_SNORM,                       { 4, 2, SNORM    , 0}},
    {VK_FORMAT_R16G16_USCALED,                     { 4, 2, USCALED  , 0}},
    {VK_FORMAT_R16G16_SSCALED,                     { 4, 2, SSCALED  , 0}},
    {VK_FORMAT_R16G16_UINT,                        { 4, 2, UINT     , 0}},
    {VK_FORMAT_R16G16_SINT,                        { 4, 2, SINT     , 0}},
    {VK_FORMAT_R16G16_SFLOAT,                      { 4, 2, SFLOAT   , 0}},
    {VK_FORMAT_R16G16B16_UNORM,                    { 6, 3, UNORM    , 0}},
    {VK_FORMAT_R16G16B16_SNORM,                    { 6, 3, SNORM    , 0}},
    {VK_FORMAT_R16G16B16_USCALED,                  { 6, 3, USCALED  , 0}},
    {VK_FORMAT_R16G16B16_SSCALED,                  { 6, 3, SSCALED  , 0}},
    {VK_FORMAT_R16G16B16_UINT,                     { 6, 3, UINT     , 0}},
    {VK_FORMAT_R16G16B16_SINT,                     { 6, 3, SINT     , 0}},
    {VK_FORMAT_R16G16B16_SFLOAT,                   { 6, 3, SFLOAT   , 0}},
    {VK_FORMAT_R16G16B16A16_UNORM,                 { 8, 4, UNORM    , 0}},
    {VK_FORMAT_R16G16B16A16_SNORM,                 { 8, 4, SNORM    , 0}},
    {VK_FORMAT_R16G16B16A16_USCALED,               { 8, 4, USCALED  , 0}},
    {VK_FORMAT_R16G16B16A16_SSCALED,               { 8, 4, SSCALED  , 0}},
    {VK_FORMAT_R16G16B16A16_UINT,                  { 8, 4, UINT     , 0}},
    {VK_FORMAT_R16G16B16A16_SINT,                  { 8, 4, SINT     , 0}},
    {VK_FORMAT_R16G16B16A16_SFLOAT,                { 8, 4, SFLOAT   , 0}},
    {VK_FORMAT_R32_UINT,                           { 4, 1, UINT     , 0}},
    {VK_FORMAT_R32_SINT,                           { 4, 1, SINT     , 0}},
    {VK_FORMAT_R32_SFLOAT,                         { 4, 1, SFLOAT   , 0}},
    {VK_FORMAT_R32G32_UINT,                        { 8, 2, UINT     , 0}},
    {VK_FORMAT_R32G32_SINT,                        { 8, 2, SINT     , 0}},
    {VK_FORMAT_R32G32_SFLOAT,                      { 8, 2, SFLOAT   , 0}},
    {VK_FORMAT_R32G32B32_UINT,                     {12, 3, UINT     , 0}},
    {VK_FORMAT_R32G32B32_SINT,                     {12, 3, SINT     , 0}},
    {VK_FORMAT_R32G32B32_SFLOAT,                   {12, 3, SFLOAT   , 0}},
    {VK_FORMAT_R32G32B32A32_UINT,                  {16, 4, UINT     , 0}},
    {VK_FORMAT_R32G32B32A32_SINT,                  {16, 4, SINT     , 0}},
    {VK_FORMAT_R32G32B32A32_SFLOAT,                {16, 4, SFLOAT   , 0}},
    {VK_FORMAT_R64_UINT,                           { 8, 1, UINT     , 0}},
    {VK_FORMAT_R64_SINT,                           { 8, 1, SINT     , 0}},
    {VK_FORMAT_R64_SFLOAT,                         { 8, 1, SFLOAT   , 0}},
    {VK_FORMAT_R64G64_UINT,                        {16, 2, UINT     , 0}},
    {VK_FORMAT_R64G64_SINT,                        {16, 2, SINT     , 0}},
    {VK_FORMAT_R64G64_SFLOAT,                      {16, 2, SFLOAT   , 0}},
    {VK_FORMAT_R64G64B64_UINT,                     {24, 3, UINT     , 0}},
    {VK_FORMAT_R64G64B64_SINT,                     {24, 3, SINT     , 0}},
    {VK_FORMAT_R64G64B64_SFLOAT,                   {24, 3, SFLOAT   , 0}},
    {VK_FORMAT_R64G64B64A64_UINT,                  {32, 4, UINT     , 0}},
    {VK_FORMAT_R64G64B64A64_SINT,                  {32, 4, SINT     , 0}},
    {VK_FORMAT_R64G64B64A64_SFLOAT,                {32, 4, SFLOAT   , 0}},
    {VK_FORMAT_B10G11R11_UFLOAT_PACK32,            { 4, 3, UFLOAT   , SWIZZLED_BIT}},
    {VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,             { 4, 3, UFLOAT   , 0}},
    {VK_FORMAT_D16_UNORM,                          { 2, 1, UNORM    , DEPTH_BIT}},
    {VK_FORMAT_X8_D24_UNORM_PACK32,                { 4, 1, UNORM    , DEPTH_BIT}},
    {VK_FORMAT_D32_SFLOAT,                         { 4, 1, SFLOAT   , DEPTH_BIT}},
    {VK_FORMAT_S8_UINT,                            { 1, 1, UINT     , STENCIL_BIT}},
    {VK_FORMAT_D16_UNORM_S8_UINT,                  { 3, 2, UNORM    , DEPTH_BIT | STENCIL_BIT}},
    {VK_FORMAT_D24_UNORM_S8_UINT,                  { 4, 2, UNORM    , DEPTH_BIT | STENCIL_BIT}},
    {VK_FORMAT_D32_SFLOAT_S8_UINT,                 { 8, 2, UINT     , DEPTH_BIT | STENCIL_BIT}},
    {VK_FORMAT_BC1_RGB_UNORM_BLOCK,                { 8, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_BC1_RGB_SRGB_BLOCK,                 { 8, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_BC1_RGBA_UNORM_BLOCK,               { 8, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_BC1_RGBA_SRGB_BLOCK,                { 8, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_BC2_UNORM_BLOCK,                    {16, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_BC2_SRGB_BLOCK,                     {16, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_BC3_UNORM_BLOCK,                    {16, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_BC3_SRGB_BLOCK,                     {16, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_BC4_UNORM_BLOCK,                    { 8, 1, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_BC4_SNORM_BLOCK,                    { 8, 1, SNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_BC5_UNORM_BLOCK,                    {16, 2, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_BC5_SNORM_BLOCK,                    {16, 2, SNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_BC6H_UFLOAT_BLOCK,                  {16, 4, UFLOAT   , COMPRESSED_BIT}},
    {VK_FORMAT_BC6H_SFLOAT_BLOCK,                  {16, 4, SFLOAT   , COMPRESSED_BIT}},
    {VK_FORMAT_BC7_UNORM_BLOCK,                    {16, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_BC7_SRGB_BLOCK,                     {16, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,            { 8, 3, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,             { 8, 3, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,          { 8, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,           { 8, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,          {16, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,           {16, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_EAC_R11_UNORM_BLOCK,                { 8, 1, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_EAC_R11_SNORM_BLOCK,                { 8, 1, SNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_EAC_R11G11_UNORM_BLOCK,             {16, 2, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_EAC_R11G11_SNORM_BLOCK,             {16, 2, SNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_4x4_UNORM_BLOCK,               {16, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_4x4_SRGB_BLOCK,                {16, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_5x4_UNORM_BLOCK,               {16, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_5x4_SRGB_BLOCK,                {16, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_5x5_UNORM_BLOCK,               {16, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_5x5_SRGB_BLOCK,                {16, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_6x5_UNORM_BLOCK,               {16, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_6x5_SRGB_BLOCK,                {16, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_6x6_UNORM_BLOCK,               {16, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_6x6_SRGB_BLOCK,                {16, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_8x5_UNORM_BLOCK,               {16, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_8x5_SRGB_BLOCK,                {16, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_8x6_UNORM_BLOCK,               {16, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_8x6_SRGB_BLOCK,                {16, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_8x8_UNORM_BLOCK,               {16, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_8x8_SRGB_BLOCK,                {16, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_10x5_UNORM_BLOCK,              {16, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_10x5_SRGB_BLOCK,               {16, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_10x6_UNORM_BLOCK,              {16, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_10x6_SRGB_BLOCK,               {16, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_10x8_UNORM_BLOCK,              {16, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_10x8_SRGB_BLOCK,               {16, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_10x10_UNORM_BLOCK,             {16, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_10x10_SRGB_BLOCK,              {16, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_12x10_UNORM_BLOCK,             {16, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_12x10_SRGB_BLOCK,              {16, 4, SRGB     , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_12x12_UNORM_BLOCK,             {16, 4, UNORM    , COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_12x12_SRGB_BLOCK,              {16, 4, SRGB     , COMPRESSED_BIT}},  //184

    //YUV
    {VK_FORMAT_G8B8G8R8_422_UNORM,                         { 2, 4, UNORM, YUV_BIT}},
    {VK_FORMAT_G16B16G16R16_422_UNORM,                     { 4, 4, UNORM, YUV_BIT}},

#ifdef EXTENDED_FORMATS
    // non-standard / YUV
    {VK_FORMAT_G8B8G8R8_422_UNORM,                         { 4, 4, UNORM, 0}},
    {VK_FORMAT_B8G8R8G8_422_UNORM,                         { 4, 4, UNORM, 0}},
    {VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,                  { 6, 3, UNORM, 0}},
    {VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,                   { 6, 3, UNORM, 0}},
    {VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,                  { 4, 3, UNORM, 0}},
    {VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,                   { 4, 3, UNORM, 0}},
    {VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,                  { 3, 3, UNORM, 0}},
    {VK_FORMAT_R10X6_UNORM_PACK16,                         { 2, 1, UNORM, 0}},
    {VK_FORMAT_R10X6G10X6_UNORM_2PACK16,                   { 4, 2, UNORM, 0}},
    {VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,         { 8, 4, UNORM, 0}},
    {VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,     { 8, 4, UNORM, 0}},
    {VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,     { 8, 4, UNORM, SWIZZLED_BIT}},
    {VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, {12, 3, UNORM, 0}},
    {VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,  {12, 3, UNORM, 0}},
    {VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16, { 8, 3, UNORM, 0}},
    {VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,  { 8, 3, UNORM, 0}},
    {VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16, { 6, 3, UNORM, 0}},
    {VK_FORMAT_R12X4_UNORM_PACK16,                         { 2, 1, UNORM, 0}},
    {VK_FORMAT_R12X4G12X4_UNORM_2PACK16,                   { 4, 2, UNORM, 0}},
    {VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,         { 8, 4, UNORM, 0}},
    {VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,     { 8, 4, UNORM, 0}},
    {VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,     { 8, 4, UNORM, SWIZZLED_BIT}},
    {VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16, {12, 3, UNORM, 0}},
    {VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,  {12, 3, UNORM, 0}},
    {VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16, { 8, 3, UNORM, 0}},
    {VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,  { 8, 3, UNORM, 0}},
    {VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16, { 6, 3, UNORM, 0}},
    {VK_FORMAT_G16B16G16R16_422_UNORM,                     { 8, 4, UNORM, 0}},
    {VK_FORMAT_B16G16R16G16_422_UNORM,                     { 8, 4, UNORM, 0}},
    {VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,               {12, 3, UNORM, 0}},
    {VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,                {12, 3, UNORM, 0}},
    {VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,               { 8, 3, UNORM, 0}},
    {VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,                { 8, 3, UNORM, 0}},
    {VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,               { 6, 3, UNORM, 0}},

    // iPhone
    {VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG, { 8, 4, UNORM , COMPRESSED_BIT}},
    {VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG, { 8, 4, UNORM , COMPRESSED_BIT}},
    {VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG, { 8, 4, UNORM , COMPRESSED_BIT}},
    {VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG, { 8, 4, UNORM , COMPRESSED_BIT}},
    {VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG,  { 8, 4, SRGB  , COMPRESSED_BIT}},
    {VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG,  { 8, 4, SRGB  , COMPRESSED_BIT}},
    {VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG,  { 8, 4, SRGB  , COMPRESSED_BIT}},
    {VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG,  { 8, 4, SRGB  , COMPRESSED_BIT}},

    // HDR
    {VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT,   {64, 4, SFLOAT, COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT,   {64, 4, SFLOAT, COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT,   {64, 4, SFLOAT, COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT,   {64, 4, SFLOAT, COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT,   {64, 4, SFLOAT, COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT,   {64, 4, SFLOAT, COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT,   {64, 4, SFLOAT, COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT,   {64, 4, SFLOAT, COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT,  {64, 4, SFLOAT, COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT,  {64, 4, SFLOAT, COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT,  {64, 4, SFLOAT, COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT, {64, 4, SFLOAT, COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT, {64, 4, SFLOAT, COMPRESSED_BIT}},
    {VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT, {64, 4, SFLOAT, COMPRESSED_BIT}}
#endif // EXTENDED_FORMATS
};

static format_info FormatInfo(VkFormat format) {
    auto item = vk_formats.find(format);
    if (item == vk_formats.end()) return {};
    return item->second;
}

static bool isCompatible(VkFormat formatA, VkFormat formatB) {
    ASSERT((formatA<185) && (formatB<185), "Unknown format.\n");
    if(formatA==formatB) return true;
    format_info fa = FormatInfo(formatA);
    format_info fb = FormatInfo(formatB);
    if(fa.size!=fb.size) return false;
    if((fa.flags | fb.flags) & (DEPTH_BIT | STENCIL_BIT)) return false;
    if(fa.isCompressed() && fb.isCompressed()) {
        if((formatB - formatA)==1 && formatB&1) return false;
        if((formatA - formatB)==1 && formatA&1) return false;
    }
    return true;
}

static std::string FormatString(VkFormat format) {
    auto fmt = FormatInfo(format);
    const char* chans[]{"","R","RG","RGB", "RGBA", "S", "D", "DS", "BGR", "BGRA"};
    const char* types[]{"UNDEFINED", "UNORM", "SNORM", "SRGB", "UINT", "SINT", "USCALED", "SSCALED", "UFLOAT", "SFLOAT"};
    int c=fmt.channels;
    if(fmt.flags) c+=5;
    std::string str = chans[c];
    if(fmt.flags == STENCIL_BIT) str="S";
    if(fmt.flags & COMPRESSED_BIT) str="BLOCK";
    if(fmt.flags & YUV_BIT) str="YUV";
    str+=std::to_string(fmt.size*8)+"_";
    str+=types[fmt.type];
    return str;
}

#undef UINT
#endif
