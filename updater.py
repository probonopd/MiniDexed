#!/usr/bin/env python3  
# -*- coding: utf-8 -*-

# Updater for MiniDexed

import os
import sys
import tempfile
import zipfile
import requests
import ftplib
import socket
import atexit
import re
import argparse

try:
    from zeroconf import ServiceBrowser, ServiceListener, Zeroconf
except ImportError:
    print("Please install the zeroconf library to use mDNS functionality.")
    print("You can install it using: pip install zeroconf")
    sys.exit(1)

class MyListener(ServiceListener):
    def __init__(self, ip_list, name_list):
        self.ip_list = ip_list
        self.name_list = name_list

    def update_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        print(f"Service {name} updated")

    def remove_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        print(f"Service {name} removed")

    def add_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        info = zc.get_service_info(type_, name)
        print(f"Service {name} added, service info: {info}")
        if info and info.addresses:
            ip = socket.inet_ntoa(info.addresses[0])
            if ip not in self.ip_list:
                self.ip_list.append(ip)
                self.name_list.append(info.server.rstrip('.'))


# Constants
TEMP_DIR = tempfile.gettempdir()

# Register cleanup function for temp files
zip_path = None
extract_path = None
def cleanup_temp_files():
    if zip_path and os.path.exists(zip_path):
        os.remove(zip_path)
    if extract_path and os.path.exists(extract_path):
        for root, dirs, files in os.walk(extract_path, topdown=False):
            for name in files:
                os.remove(os.path.join(root, name))
            for name in dirs:
                os.rmdir(os.path.join(root, name))
        os.rmdir(extract_path)
    print("Cleaned up temporary files.")
atexit.register(cleanup_temp_files)

# Function to download the latest release
def download_latest_release(url):
    response = requests.get(url, stream=True)
    if response.status_code == 200:
        zip_path = os.path.join(TEMP_DIR, "MiniDexed_latest.zip")
        with open(zip_path, 'wb') as f:
            for chunk in response.iter_content(chunk_size=8192):
                f.write(chunk)
        return zip_path
    return None

# Function to extract the downloaded zip file
def extract_zip(zip_path):
    extract_path = os.path.join(TEMP_DIR, "MiniDexed")
    with zipfile.ZipFile(zip_path, 'r') as zip_ref:
        zip_ref.extractall(extract_path)
    return extract_path

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="MiniDexed Updater")
    parser.add_argument("-v", action="store_true", help="Enable verbose FTP debug output")
    args = parser.parse_args()

    import time
    # Ask user which release to download (numbered choices)
    release_options = [
        ("Latest official release", "https://github.com/probonopd/MiniDexed/releases/expanded_assets/latest"),
        ("Continuous (experimental) build", "https://github.com/probonopd/MiniDexed/releases/expanded_assets/continuous")
    ]
    print("Which release do you want to download?")
    for idx, (desc, _) in enumerate(release_options):
        print(f"  [{idx+1}] {desc}")
    while True:
        choice = input(f"Enter the number of your choice (1-{len(release_options)}): ").strip()
        if choice.isdigit() and 1 <= int(choice) <= len(release_options):
            github_url = release_options[int(choice)-1][1]
            break
        print("Invalid selection. Please enter a valid number.")

    # Using mDNS to find the IP address of the device(s) that advertise the FTP service "_ftp._tcp."
    ip_addresses = []
    device_names = []
    zeroconf = Zeroconf()
    listener = MyListener(ip_addresses, device_names)
    browser = ServiceBrowser(zeroconf, "_ftp._tcp.local.", listener)
    try:
        print("Searching for devices...")
        time.sleep(5)
        if ip_addresses:
            print("Devices found:")
            for idx, (name, ip) in enumerate(zip(device_names, ip_addresses)):
                print(f"  [{idx+1}] {name} ({ip})")
            while True:
                selection = input(f"Enter the number of the device to upload to (1-{len(ip_addresses)}): ").strip()
                if selection.isdigit() and 1 <= int(selection) <= len(ip_addresses):
                    selected_ip = ip_addresses[int(selection)-1]
                    selected_name = device_names[int(selection)-1]
                    break
                print("Invalid selection. Please enter a valid number.")
        else:
            print("No devices found.")
            sys.exit(1)
    finally:
        zeroconf.close()
        print("Devices found:", list(zip(device_names, ip_addresses)))

    # Use the selected GitHub URL for release
    def get_release_url(github_url):
        print(f"Fetching release page: {github_url}")
        response = requests.get(github_url)
        print(f"HTTP status code: {response.status_code}")
        if response.status_code == 200:
            print("Successfully fetched release page. Scanning for MiniDexed*.zip links...")
            # Find all <a ... href="..."> tags with a <span class="Truncate-text text-bold">MiniDexed*.zip</span>
            pattern = re.compile(r'<a[^>]+href=["\']([^"\']+\.zip)["\'][^>]*>\s*<span[^>]*class=["\']Truncate-text text-bold["\'][^>]*>(MiniDexed[^<]*?\.zip)</span>', re.IGNORECASE)
            matches = pattern.findall(response.text)
            print(f"Found {len(matches)} candidate .zip links.")
            for href, filename in matches:
                print(f"Examining link: href={href}, filename={filename}")
                if filename.startswith("MiniDexed") and filename.endswith(".zip"):
                    if href.startswith('http'):
                        print(f"Selected direct link: {href}")
                        return href
                    else:
                        full_url = f"https://github.com{href}"
                        print(f"Selected relative link, full URL: {full_url}")
                        return full_url
            print("No valid MiniDexed*.zip link found.")
        else:
            print(f"Failed to fetch release page. Status code: {response.status_code}")
        return None

    latest_release_url = get_release_url(github_url)
    if latest_release_url:
        print(f"Release URL: {latest_release_url}")
        zip_path = download_latest_release(latest_release_url)
        if zip_path:
            print(f"Downloaded to: {zip_path}")
            extract_path = extract_zip(zip_path)
            print(f"Extracted to: {extract_path}")
        else:
            print("Failed to download the release.")
            sys.exit(1)
    else:
        print("Failed to get the release URL.")
        sys.exit(1)

    # Log into the selected device and upload the new version of MiniDexed
    print(f"Connecting to {selected_name} ({selected_ip})...")
    try:
        ftp = ftplib.FTP()
        if args.v:
            ftp.set_debuglevel(2)
        ftp.connect(selected_ip, 21, timeout=10)
        ftp.login("admin", "admin")
        ftp.set_pasv(True)
        print(f"Connected to {selected_ip} (passive mode).")
        for root, dirs, files in os.walk(extract_path):
            for file in files:
                if file.startswith("kernel") and file.endswith(".img"):
                    local_path = os.path.join(root, file)
                    remote_path = f"/SD/{file}"
                    # Check if file exists on FTP server
                    file_exists = False
                    try:
                        ftp.cwd("/SD")
                        if file in ftp.nlst():
                            file_exists = True
                    except Exception as e:
                        print(f"Error checking for {file} on FTP server: {e}")
                        file_exists = False
                    if not file_exists:
                        print(f"Skipping {file}: does not exist on device.")
                        continue
                    filesize = os.path.getsize(local_path)
                    uploaded = [0]
                    def progress_callback(data):
                        uploaded[0] += len(data)
                        percent = uploaded[0] * 100 // filesize
                        print(f"\rUploading {file}: {percent}%", end="", flush=True)
                    with open(local_path, 'rb') as f:
                        ftp.storbinary(f'STOR {remote_path}', f, 8192, callback=progress_callback)
                    print(f"\nUploaded {file} to {selected_ip}.")
        ftp.sendcmd("BYE")
        print(f"Disconnected from {selected_ip}.")
    except ftplib.all_errors as e:
        print(f"FTP error: {e}")
