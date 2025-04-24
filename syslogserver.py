#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Syslog server to receive and display syslog messages from MiniDexed.
"""

import socket
import time
import threading

class SyslogServer:
    def __init__(self, host='0.0.0.0', port=8514):
        self.host = host
        self.port = port
        self.server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.server.bind((self.host, self.port))
        self.start_time = None
        self.running = True

    def start(self):
        ip_address = socket.gethostbyname(socket.gethostname())
        print(f"Syslog server listening on {ip_address}:{self.port}")
        input_thread = threading.Thread(target=self.wait_for_input)
        input_thread.daemon = True
        input_thread.start()
        while self.running:
            try:
                data, address = self.server.recvfrom(1024)
                self.handle_message(data)
            except KeyboardInterrupt:
                self.running = False

    def handle_message(self, data):
        message = data[2:].decode('utf-8').strip()

        if self.start_time is None:
            self.start_time = time.time()
            relative_time = "0:00:00.000"
        else:
            elapsed_time = time.time() - self.start_time
            hours = int(elapsed_time // 3600)
            minutes = int((elapsed_time % 3600) // 60)
            seconds = int(elapsed_time % 60)
            milliseconds = int((elapsed_time % 1) * 1000)
            relative_time = f"{hours:02d}:{minutes:02d}:{seconds:02d}.{milliseconds:03d}"

        print(f"{relative_time} {message}")

    def wait_for_input(self):
        try:
            input("Press any key to exit...")
        except EOFError:
            pass
        self.running = False

if __name__ == "__main__":
    server = SyslogServer()
    server.start()
    print("Syslog server stopped.")