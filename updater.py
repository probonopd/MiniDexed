#!/usr/bin/env python3
#  -*- coding: utf-8 -*-

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
            # Only add if TXT record contains 'MiniDexed'
            txt_records = info.properties
            if txt_records:
                for k, v in txt_records.items():
                    # v may be bytes, decode if needed
                    val = v.decode() if isinstance(v, bytes) else v
                    if (b"MiniDexed" in k or b"MiniDexed" in v) or ("MiniDexed" in str(k) or "MiniDexed" in str(val)):
                        ip = socket.inet_ntoa(info.addresses[0])
                        if ip not in self.ip_list:
                            self.ip_list.append(ip)
                            self.name_list.append(info.server.rstrip('.'))
                        break


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

# Function to download the latest release asset using GitHub API
def download_latest_release_github_api(release_type):
    """
    release_type: 'latest' or 'continuous'
    Returns: path to downloaded zip or None
    """
    import json
    headers = {'Accept': 'application/vnd.github.v3+json'}
    repo = 'probonopd/MiniDexed'
    if release_type == 'latest':
        api_url = f'https://api.github.com/repos/{repo}/releases/latest'
        resp = requests.get(api_url, headers=headers)
        if resp.status_code != 200:
            print(f"GitHub API error: {resp.status_code}")
            return None
        release = resp.json()
        assets = release.get('assets', [])
    elif release_type == 'continuous':
        api_url = f'https://api.github.com/repos/{repo}/releases'
        resp = requests.get(api_url, headers=headers)
        if resp.status_code != 200:
            print(f"GitHub API error: {resp.status_code}")
            return None
        releases = resp.json()
        release = next((r for r in releases if 'continuous' in (r.get('tag_name','')+r.get('name','')).lower()), None)
        if not release:
            print("No continuous release found.")
            return None
        assets = release.get('assets', [])
    else:
        print(f"Unknown release type: {release_type}")
        return None
    asset = next((a for a in assets if a['name'].startswith('MiniDexed') and a['name'].endswith('.zip')), None)
    if not asset:
        print("No MiniDexed*.zip asset found in release.")
        return None
    url = asset['browser_download_url']
    print(f"Downloading asset: {asset['name']} from {url}")
    resp = requests.get(url, stream=True)
    if resp.status_code == 200:
        zip_path = os.path.join(TEMP_DIR, asset['name'])
        with open(zip_path, 'wb') as f:
            for chunk in resp.iter_content(chunk_size=8192):
                f.write(chunk)
        return zip_path
    print(f"Failed to download asset: {resp.status_code}")
    return None

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="MiniDexed Updater")
    parser.add_argument("-v", action="store_true", help="Enable verbose FTP debug output")
    parser.add_argument("--ip", type=str, help="IP address of the device to upload to (skip mDNS discovery)")
    parser.add_argument("--version", type=int, choices=[1,2,3], help="Version to upload: 1=Latest official, 2=Continuous, 3=Local build (skip prompt)")
    parser.add_argument("--pr", type=str, help="Pull request number or URL to fetch build artifacts from PR comment")
    parser.add_argument("--github-token", type=str, help="GitHub personal access token for downloading PR artifacts (optional, can also use GITHUB_TOKEN env var)")
    args = parser.parse_args()

    import time
    # Check for local kernel*.img files
    local_kernel_dir = os.path.join(os.path.dirname(__file__), 'src')
    local_kernel_imgs = [f for f in os.listdir(local_kernel_dir) if f.startswith('kernel') and f.endswith('.img')]
    has_local_build = len(local_kernel_imgs) > 0

    # Get GitHub token from argument or environment
    github_token = args.github_token or os.environ.get("GITHUB_TOKEN")

    # Ask user which release to download (numbered choices)
    release_options = [
        ("Latest official release", "https://github.com/probonopd/MiniDexed/releases/expanded_assets/latest"),
        ("Continuous (experimental) build", "https://github.com/probonopd/MiniDexed/releases/expanded_assets/continuous")
    ]
    if has_local_build:
        release_options.append(("Local build (from src/)", None))

    if args.version:
        selected_idx = args.version - 1
        if selected_idx < 0 or selected_idx >= len(release_options):
            print(f"Invalid version selection: {args.version}")
            sys.exit(1)
        github_url = release_options[selected_idx][1]
    else:
        print("Which release do you want to update?")
        for idx, (desc, _) in enumerate(release_options):
            print(f"  [{idx+1}] {desc}")
        print("  [PR] Pull request build (enter PR number or URL)")
        while True:
            choice = input(f"Enter the number of your choice (1-{len(release_options)}) or PR number: ").strip()
            if choice.isdigit() and 1 <= int(choice) <= len(release_options):
                selected_idx = int(choice)-1
                github_url = release_options[selected_idx][1]
                args.pr = None
                break
            # Accept PR number, #NNN, or PR URL
            pr_match = re.match(r'^(#?\d+|https?://github.com/[^/]+/[^/]+/pull/\d+)$', choice)
            if pr_match:
                args.pr = choice
                selected_idx = None
                github_url = None
                break
            print("Invalid selection. Please enter a valid number or PR number/URL.")

    # If local build is selected, skip all GitHub/zip logic and do not register cleanup
    use_local_build = has_local_build and selected_idx == len(release_options)-1
    if args.pr:
        # Extract PR number from input (accepts URL, #899, or 899)
        import re
        pr_input = args.pr.strip()
        m = re.search(r'(\d+)$', pr_input)
        if not m:
            print(f"Could not parse PR number from: {pr_input}")
            sys.exit(1)
        pr_number = m.group(1)
        # Fetch PR page HTML
        pr_url = f"https://github.com/probonopd/MiniDexed/pull/{pr_number}"
        print(f"Fetching PR page: {pr_url}")
        resp = requests.get(pr_url)
        if resp.status_code != 200:
            print(f"Failed to fetch PR page: {resp.status_code}")
            sys.exit(1)
        html = resp.text
        # Find all 'Build for testing' artifact blocks (look for <p dir="auto">Build for testing: ...</p>)
        import html as ihtml
        import re
        pattern = re.compile(r'<p dir="auto">Build for testing:(.*?)Use at your own risk\.', re.DOTALL)
        matches = pattern.findall(html)
        if not matches:
            print("No build artifact links found in PR comment.")
            sys.exit(1)
        last_block = matches[-1]
        # Find all artifact links in the last block
        link_pattern = re.compile(r'<a href="([^"]+)">([^<]+)</a>')
        links = link_pattern.findall(last_block)
        if not links:
            print("No artifact links found in PR comment block.")
            sys.exit(1)
        # Download both 32bit and 64bit artifacts if present
        artifact_paths = []
        for url, name in links:
            print(f"Downloading artifact: {name} from {url}")
            # Try to extract the artifact ID from the URL
            m = re.search(r'/artifacts/(\d+)', url)
            if m and github_token:
                artifact_id = m.group(1)
                api_url = f"https://api.github.com/repos/probonopd/MiniDexed/actions/artifacts/{artifact_id}/zip"
                headers = {
                    "Authorization": f"Bearer {github_token}",
                    "Accept": "application/vnd.github.v3+json",
                    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/123.0.0.0 Safari/537.36"
                }
                resp = requests.get(api_url, stream=True, headers=headers)
                if resp.status_code == 200:
                    local_path = os.path.join(TEMP_DIR, name + ".zip")
                    with open(local_path, 'wb') as f:
                        for chunk in resp.iter_content(chunk_size=8192):
                            f.write(chunk)
                    artifact_paths.append(local_path)
                else:
                    print(f"Failed to download artifact {name} via GitHub API: {resp.status_code}")
                    print(f"Request headers: {resp.request.headers}")
                    print(f"Response headers: {resp.headers}")
                    print(f"Response URL: {resp.url}")
                    print(f"Response text (first 500 chars): {resp.text[:500]}")
            else:
                # Fallback to direct link if no artifact ID or no token
                headers = {}
                if github_token:
                    headers["Authorization"] = f"Bearer {github_token}"
                headers["Accept"] = "application/octet-stream"
                headers["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/123.0.0.0 Safari/537.36"
                resp = requests.get(url, stream=True, headers=headers)
                if resp.status_code == 200:
                    local_path = os.path.join(TEMP_DIR, name + ".zip")
                    with open(local_path, 'wb') as f:
                        for chunk in resp.iter_content(chunk_size=8192):
                            f.write(chunk)
                    artifact_paths.append(local_path)
                else:
                    print(f"Failed to download artifact {name}: {resp.status_code}")
                    print(f"Request headers: {resp.request.headers}")
                    print(f"Response headers: {resp.headers}")
                    print(f"Response URL: {resp.url}")
                    print(f"Response text (first 500 chars): {resp.text[:500]}")
                    if not github_token:
                        print("You may need to provide a GitHub personal access token using --github-token or the GITHUB_TOKEN environment variable.")
        if not artifact_paths:
            print("No artifacts downloaded.")
            sys.exit(1)
        # Extract all downloaded zips
        extract_paths = []
        for path in artifact_paths:
            ep = extract_zip(path)
            print(f"Extracted {path} to {ep}")
            extract_paths.append(ep)
        # Use the first extracted path for further logic (or merge as needed)
        extract_path = extract_paths[0] if extract_paths else None
        use_local_build = False
    elif use_local_build:
        # Remove cleanup function if registered
        atexit.unregister(cleanup_temp_files)
        print("Using local build: src/kernel*.img will be uploaded.")
        extract_path = None
    else:
        # Use GitHub API instead of HTML parsing
        if selected_idx == 0:
            zip_path = download_latest_release_github_api('latest')
        elif selected_idx == 1:
            zip_path = download_latest_release_github_api('continuous')
        else:
            zip_path = None
        if zip_path:
            print(f"Downloaded to: {zip_path}")
            extract_path = extract_zip(zip_path)
            print(f"Extracted to: {extract_path}")
        else:
            print("Failed to download the release.")
            sys.exit(1)

    # Ask user if they want to update Performances (default no)
    if not use_local_build:
        update_perf = input("Do you want to update the Performances? This will OVERWRITE all existing performances. [y/N]: ").strip().lower()
        update_performances = update_perf == 'y'
    else:
        update_performances = False

    # Using mDNS to find the IP address of the device(s) that advertise the FTP service "_ftp._tcp."
    ip_addresses = []
    device_names = []
    selected_ip = None
    selected_name = None
    if args.ip:
        selected_ip = args.ip
        selected_name = args.ip
    else:
        zeroconf = Zeroconf()
        listener = MyListener(ip_addresses, device_names)
        browser = ServiceBrowser(zeroconf, "_ftp._tcp.local.", listener)
        try:
            print("Searching for devices...")
            time.sleep(10)
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
        # --- Performances update logic ---
        if update_performances and not use_local_build:
            print("Updating Performance: recursively deleting and uploading /SD/performance directory...")
            def ftp_rmdirs(ftp, path):
                try:
                    items = ftp.nlst(path)
                except Exception as e:
                    print(f"[WARN] Could not list {path}: {e}")
                    return
                for item in items:
                    if item in ['.', '..', path]:
                        continue
                    full_path = f"{path}/{item}" if not item.startswith(path) else item
                    try:
                        # Try to delete as a file first
                        ftp.delete(full_path)
                        print(f"Deleted file: {full_path}")
                    except Exception:
                        # If not a file, try as a directory
                        try:
                            ftp_rmdirs(ftp, full_path)
                            ftp.rmd(full_path)
                            print(f"Deleted directory: {full_path}")
                        except Exception as e:
                            print(f"[WARN] Could not delete {full_path}: {e}")
            try:
                ftp_rmdirs(ftp, '/SD/performance')
                try:
                    ftp.rmd('/SD/performance')
                    print("Deleted /SD/performance on device.")
                except Exception as e:
                    print(f"[WARN] Could not delete /SD/performance directory itself: {e}")
            except Exception as e:
                print(f"Warning: Could not delete /SD/performance: {e}")
            # Upload extracted performance/ recursively
            local_perf = os.path.join(extract_path, 'performance')
            def ftp_mkdirs(ftp, path):
                try:
                    ftp.mkd(path)
                except Exception:
                    pass
            def ftp_upload_dir(ftp, local_dir, remote_dir):
                ftp_mkdirs(ftp, remote_dir)
                for item in os.listdir(local_dir):
                    lpath = os.path.join(local_dir, item)
                    rpath = f"{remote_dir}/{item}"
                    if os.path.isdir(lpath):
                        ftp_upload_dir(ftp, lpath, rpath)
                    else:
                        with open(lpath, 'rb') as fobj:
                            ftp.storbinary(f'STOR {rpath}', fobj)
                        print(f"Uploaded {rpath}")
            if os.path.isdir(local_perf):
                ftp_upload_dir(ftp, local_perf, '/SD/performance')
                print("Uploaded new /SD/performance directory.")
            else:
                print("No extracted performance/ directory found, skipping upload.")
            # Upload performance.ini if it exists in extract_path
            local_perfini = os.path.join(extract_path, 'performance.ini')
            if os.path.isfile(local_perfini):
                with open(local_perfini, 'rb') as fobj:
                    ftp.storbinary('STOR /SD/performance.ini', fobj)
                print("Uploaded /SD/performance.ini.")
            else:
                print("No extracted performance.ini found, skipping upload.")
        # Upload kernel files
        if use_local_build:
            for file in local_kernel_imgs:
                local_path = os.path.join(local_kernel_dir, file)
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
        else:
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
    except ftplib.all_errors as e:
        print(f"FTP error: {e}")
        ftp = None  # Mark ftp as unusable

    # Only attempt to send BYE if ftp is connected and has a socket
    try:
        if ftp is not None and getattr(ftp, 'sock', None) is not None:
            ftp.sendcmd("BYE")
            print(f"Disconnected from {selected_ip}.")
        else:
            print(f"No active FTP connection to disconnect from.")
    except (*ftplib.all_errors, AttributeError) as e:
        # Handle timeout or already closed connection
        if isinstance(e, TimeoutError) or (str(e).strip().lower() == "timed out"):
            print(f"Disconnected from {selected_ip} (timeout after BYE, device likely restarted).")
        else:
            print(f"FTP error after BYE: {e}")
