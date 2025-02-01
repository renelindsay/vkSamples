#!/bin/bash
sudo apt install build-essential ninja-build
sudo apt install libx11-xcb-dev libxi-dev libxcb-image0-dev libxkbcommon-dev
#sudo apt install libvulkan1 vulkan-tools libvulkan-dev

cd ./libs/vkUtils/Vexel
python3 vexel_gen.py -dg

