#!/usr/bin/env python3
import os
import sys

def merge_bin_files():
    """合并 bootloader.bin 和 Template.bin，生成 Template_add_bootloader.bin。

    工程目录结构：
    Project/
      merge_bin.py          <-- 本脚本（Keil 当前工作目录也是 Project）
      bin_file/
        bootloader.bin
        Template.bin       <-- fromelf 生成
        Template_add_bootloader.bin  <-- 本脚本生成
    """
    # 工程根目录（Keil 调用 python 时的当前目录也是这里）
    project_dir = os.path.dirname(os.path.abspath(__file__))
    bin_dir = os.path.join(project_dir, "bin_file")
	
    # 文件路径
    bootloader_path = os.path.join(bin_dir, "bootloader.bin")
    template_path = os.path.join(bin_dir, "Template.bin")
    output_path = os.path.join(bin_dir, "Template_add_bootloader.bin")
    
    # 检查文件是否存在
    if not os.path.exists(bootloader_path):
        print(f"Error: {bootloader_path} not found!")
        return 1
    if not os.path.exists(template_path):
        print(f"Error: {template_path} not found!")
        return 1
    
    # 读取bootloader.bin
    with open(bootloader_path, 'rb') as f:
        bootloader_data = f.read()
    
    # 读取template.bin
    with open(template_path, 'rb') as f:
        template_data = f.read()
    
    # 计算需要填充的大小（0x4800 - bootloader大小）
    bootloader_size = len(bootloader_data)
    target_offset = 0x4800
    
    if bootloader_size > target_offset:
        print("Error: Bootloader too large!")
        return 1
    
    # 创建填充数据（0xFF或0x00）
    padding_size = target_offset - bootloader_size
    padding_data = b'\xFF' * padding_size  # 或者使用 b'\x00'
    
    # 合并文件
    merged_data = bootloader_data + padding_data + template_data
    
    # 写入新文件
    with open(output_path, 'wb') as f:
        f.write(merged_data)
    
    print(f"Merged binary created: {output_path}")
    print(f"Bootloader size: 0x{bootloader_size:X} bytes")
    print(f"Padding size: 0x{padding_size:X} bytes")
    print(f"Template size: 0x{len(template_data):X} bytes")
    print(f"Total size: 0x{len(merged_data):X} bytes")
    
    return 0

if __name__ == "__main__":
    sys.exit(merge_bin_files())