#!/usr/bin/env python3
"""
Loop.py - Wakuseibokan Client Ping Responder

Listens for ping messages from Lobby.py, verifies the target name
matches its own identity, and immediately sends back a pong so that
Lobby.py can confirm network visibility and measure round-trip latency.

Run this on every client machine while Lobby.py is monitoring.

Usage:  python Loop.py <name>
  <name>  must exactly match the name in lobby.conf (e.g. womby, carly)
"""

import socket
import json
import sys
import signal
import time

LISTEN_PORT = 5000   # must match Lobby.py's PING_PORT


def main():
    if len(sys.argv) < 2:
        print("Usage: python Loop.py <name>")
        print("  <name> must match your entry in lobby.conf")
        sys.exit(1)

    my_name = sys.argv[1].strip()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    try:
        sock.bind(('', LISTEN_PORT))
    except OSError as e:
        print(f"Error: cannot bind to UDP port {LISTEN_PORT}: {e}")
        sys.exit(1)

    def shutdown(sig, frame):
        print(f"\nLoop [{my_name}] stopped.")
        sock.close()
        sys.exit(0)

    signal.signal(signal.SIGINT,  shutdown)
    signal.signal(signal.SIGTERM, shutdown)

    print(f"Loop [{my_name}]  listening on UDP port {LISTEN_PORT}")
    print("Waiting for pings from Lobby.py ...  (CTRL+C to stop)\n")

    pong_count = 0

    while True:
        try:
            data, addr = sock.recvfrom(1024)
        except OSError:
            break

        try:
            msg = json.loads(data.decode('utf-8'))
        except (json.JSONDecodeError, UnicodeDecodeError):
            print(f"  [warn] bad message from {addr}")
            continue

        if msg.get('type') != 'ping':
            continue

        target = msg.get('name', '')
        seq    = msg.get('seq', 0)
        ts     = msg.get('ts', 0.0)

        if target != my_name:
            # ping intended for a different client on the same machine — ignore
            print(f"  [skip] ping for '{target}'  (I am '{my_name}')")
            continue

        pong = {
            'type': 'pong',
            'name': my_name,
            'seq':  seq,
            'ts':   ts,       # echo original timestamp so Lobby.py can measure RTT
        }
        sock.sendto(json.dumps(pong).encode('utf-8'), addr)
        pong_count += 1
        ts_str = time.strftime('%H:%M:%S')
        print(f"  [{ts_str}]  pong #{pong_count}  seq={seq}  → {addr[0]}:{addr[1]}")


if __name__ == '__main__':
    main()
