#!/usr/bin/env python3
"""
Lobby.py - Wakuseibokan Network Lobby Monitor

Reads lobby.conf, continuously pings each client via ICMP and UDP, and displays
a live ASCII dashboard with four connectivity states:

  ◌  WAIT    (white)  – not yet tested
  ●  OFFLINE (red)    – ICMP ping failing, host unreachable
  ◑  NO LOOP (yellow) – host reachable on the network, but Loop.py not running
  ●  ONLINE  (green)  – Loop.py responding; RTT shown in ms

Dependencies (add to ps venv):
  pip install ping3      # optional but preferred; subprocess fallback used if absent

Usage: python Lobby.py [lobby.conf]
       Press CTRL+C or 'q' to exit.
"""

import socket
import json
import time
import threading
import curses
import sys
import os
import platform
import subprocess
from concurrent.futures import ThreadPoolExecutor, as_completed
from datetime import datetime

# ── ports ──────────────────────────────────────────────────────────────────────
PING_PORT     = 5000   # Loop.py clients listen here
SERVER_PORT   = 5001   # Lobby.py binds here (send pings / receive pongs)

# ── timing ─────────────────────────────────────────────────────────────────────
PING_INTERVAL   = 1.0  # seconds between full ping rounds
ICMP_TIMEOUT    = 1.0  # seconds for each ICMP probe
UDP_TIMEOUT     = 3.0  # seconds without a UDP pong before downgrading

# ── states ─────────────────────────────────────────────────────────────────────
S_WAIT    = "WAIT"
S_OFFLINE = "OFFLINE"
S_REACH   = "NO LOOP"   # ICMP ok, UDP loop not responding
S_ONLINE  = "ONLINE"

# ── optional ping3 import with subprocess fallback ─────────────────────────────
try:
    import ping3 as _ping3
    _ping3.EXCEPTIONS = False          # return None/False instead of raising
    _HAVE_PING3 = True
except ImportError:
    _HAVE_PING3 = False


def icmp_ping(ip: str, timeout: float = ICMP_TIMEOUT) -> bool:
    """Return True if the host responds to ICMP echo, False otherwise."""
    if _HAVE_PING3:
        try:
            result = _ping3.ping(ip, timeout=timeout)
            return result is not None and result is not False
        except Exception:
            pass  # fall through to subprocess

    # subprocess fallback — works everywhere, no extra dependency needed
    system = platform.system()
    if system == 'Windows':
        cmd = ['ping', '-n', '1', '-w', str(int(timeout * 1000)), ip]
    elif system == 'Darwin':
        cmd = ['ping', '-c', '1', '-W', str(int(timeout * 1000)), ip]
    else:                                        # Linux
        cmd = ['ping', '-c', '1', '-W', str(max(1, int(timeout))), ip]
    try:
        r = subprocess.run(cmd, capture_output=True, timeout=timeout + 2)
        return r.returncode == 0
    except Exception:
        return False


# ── lobby.conf parser ──────────────────────────────────────────────────────────

def read_lobby_conf(path: str) -> list:
    clients = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            parts = line.split()
            if len(parts) >= 2 and parts[0].lower() != 'tankname':
                clients.append({
                    'name':      parts[0],
                    'ip':        parts[1],
                    'status':    S_WAIT,
                    'latency':   None,
                    'last_pong': 0.0,   # last UDP pong timestamp
                    'icmp_ok':   None,  # None=untested, True/False
                })
    return clients


# ── monitor core ───────────────────────────────────────────────────────────────

class LobbyMonitor:
    def __init__(self, clients: list):
        self.clients   = clients
        self.by_name   = {c['name']: c for c in clients}
        self.lock      = threading.Lock()
        self.pings_sent  = 0
        self.pongs_rcvd  = 0
        self.running     = True
        self._executor   = ThreadPoolExecutor(max_workers=max(4, len(clients)))

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(('', SERVER_PORT))
        self.sock.settimeout(0.5)

    def start(self):
        threading.Thread(target=self._recv_loop,    daemon=True).start()
        threading.Thread(target=self._ping_loop,    daemon=True).start()
        threading.Thread(target=self._timeout_loop, daemon=True).start()

    def stop(self):
        self.running = False
        self._executor.shutdown(wait=False)
        try:
            self.sock.close()
        except OSError:
            pass

    def snapshot(self):
        with self.lock:
            return [dict(c) for c in self.clients], self.pings_sent, self.pongs_rcvd

    # ── threads ────────────────────────────────────────────────────────────────

    def _recv_loop(self):
        """Receive UDP pongs and update online status + latency."""
        while self.running:
            try:
                data, _ = self.sock.recvfrom(1024)
                recv_time = time.time()
                msg = json.loads(data.decode('utf-8'))
                if msg.get('type') == 'pong':
                    name    = msg.get('name', '')
                    send_ts = float(msg.get('ts', 0))
                    latency = (recv_time - send_ts) * 1000.0
                    with self.lock:
                        if name in self.by_name:
                            c = self.by_name[name]
                            c['status']    = S_ONLINE
                            c['latency']   = latency
                            c['last_pong'] = recv_time
                            self.pongs_rcvd += 1
            except socket.timeout:
                pass
            except Exception:
                pass

    def _ping_loop(self):
        """ICMP-probe all clients in parallel, then send UDP pings to reachable ones."""
        seq = 0
        while self.running:
            now = time.time()
            with self.lock:
                targets = [(c['name'], c['ip']) for c in self.clients]

            # run ICMP probes concurrently
            futures = {
                self._executor.submit(icmp_ping, ip, ICMP_TIMEOUT): (name, ip)
                for name, ip in targets
            }
            icmp_results = {}
            for fut in as_completed(futures):
                name, ip = futures[fut]
                try:
                    icmp_results[name] = (ip, fut.result())
                except Exception:
                    icmp_results[name] = (ip, False)

            # apply results and send UDP pings where ICMP is OK
            msg_ts = time.time()
            for name, (ip, reachable) in icmp_results.items():
                with self.lock:
                    c = self.by_name[name]
                    c['icmp_ok'] = reachable

                    if not reachable:
                        c['status']  = S_OFFLINE
                        c['latency'] = None
                    else:
                        # only downgrade; _recv_loop promotes to S_ONLINE
                        if c['status'] in (S_OFFLINE, S_WAIT):
                            c['status'] = S_REACH
                        # send UDP ping regardless (quickly detect Loop.py startup)
                        udp_msg = json.dumps({
                            'type': 'ping',
                            'name': name,
                            'seq':  seq,
                            'ts':   msg_ts,
                        })
                        try:
                            self.sock.sendto(udp_msg.encode('utf-8'), (ip, PING_PORT))
                            self.pings_sent += 1
                        except Exception:
                            pass
            seq += 1
            time.sleep(PING_INTERVAL)

    def _timeout_loop(self):
        """Downgrade ONLINE → NO LOOP when UDP pongs stop arriving."""
        while self.running:
            now = time.time()
            with self.lock:
                for c in self.clients:
                    if (c['status'] == S_ONLINE
                            and c['last_pong'] > 0
                            and now - c['last_pong'] > UDP_TIMEOUT):
                        c['status']  = S_REACH if c['icmp_ok'] else S_OFFLINE
                        c['latency'] = None
            time.sleep(0.25)


# ── curses dashboard ───────────────────────────────────────────────────────────

CW_NAME   = 18
CW_IP     = 18
CW_STATUS = 12
CW_RTT    = 12
TABLE_W   = CW_NAME + CW_IP + CW_STATUS + CW_RTT + 5  # cols + 5 border chars


def _hline(stdscr, row: int, left: str, mid: str, right: str, attr: int):
    line = (left
            + '─' * CW_NAME
            + mid + '─' * CW_IP
            + mid + '─' * CW_STATUS
            + mid + '─' * CW_RTT
            + right)
    try:
        stdscr.addstr(row, 2, line, attr)
    except curses.error:
        pass


def _put(stdscr, row: int, col: int, text: str, attr: int):
    try:
        stdscr.addstr(row, col, text, attr)
    except curses.error:
        pass


def draw(stdscr, monitor: LobbyMonitor):
    curses.curs_set(0)
    curses.start_color()
    curses.use_default_colors()
    curses.init_pair(1, curses.COLOR_GREEN,  -1)   # ONLINE
    curses.init_pair(2, curses.COLOR_RED,    -1)   # OFFLINE
    curses.init_pair(3, curses.COLOR_YELLOW, -1)   # NO LOOP / hints
    curses.init_pair(4, curses.COLOR_CYAN,   -1)   # chrome / borders
    curses.init_pair(5, curses.COLOR_WHITE,  -1)   # normal text

    C_ONLINE  = curses.color_pair(1) | curses.A_BOLD
    C_OFFLINE = curses.color_pair(2) | curses.A_BOLD
    C_REACH   = curses.color_pair(3) | curses.A_BOLD
    C_WAIT    = curses.color_pair(5)
    C_BORDER  = curses.color_pair(4)
    C_HDR     = curses.color_pair(4) | curses.A_BOLD
    C_NORM    = curses.color_pair(5)
    C_HINT    = curses.color_pair(3)

    # Legend characters and colours per state
    STATE_STYLE = {
        S_WAIT:    ('◌', 'WAIT',    C_WAIT),
        S_OFFLINE: ('●', 'OFFLINE', C_OFFLINE),
        S_REACH:   ('◑', 'NO LOOP', C_REACH),
        S_ONLINE:  ('●', 'ONLINE',  C_ONLINE),
    }

    stdscr.nodelay(True)

    icmp_label = "ping3" if _HAVE_PING3 else "subprocess"

    while True:
        key = stdscr.getch()
        if key in (ord('q'), ord('Q'), 3):
            raise KeyboardInterrupt

        stdscr.erase()
        clients, pings_sent, pongs_rcvd = monitor.snapshot()
        now_str = datetime.now().strftime("%Y-%m-%d  %H:%M:%S")

        online_cnt  = sum(1 for c in clients if c['status'] == S_ONLINE)
        reach_cnt   = sum(1 for c in clients if c['status'] == S_REACH)
        offline_cnt = sum(1 for c in clients if c['status'] == S_OFFLINE)
        wait_cnt    = sum(1 for c in clients if c['status'] == S_WAIT)

        # ── title bar ──────────────────────────────────────────────────────────
        title = f"  WAKUSEIBOKAN LOBBY MONITOR    {now_str}  "
        _put(stdscr, 0, 2, '┌' + '─' * (TABLE_W - 2) + '┐', C_BORDER)
        _put(stdscr, 1, 2, '│', C_BORDER)
        _put(stdscr, 1, 3, title.center(TABLE_W - 2)[:TABLE_W - 2], C_HDR)
        _put(stdscr, 1, 2 + TABLE_W - 1, '│', C_BORDER)

        # ── column headers ─────────────────────────────────────────────────────
        _hline(stdscr, 2, '├', '┬', '┤', C_BORDER)
        row = 3
        x = 2
        _put(stdscr, row, x, '│', C_BORDER);                    x += 1
        _put(stdscr, row, x, f"{'  NAME':<{CW_NAME}}", C_HDR);  x += CW_NAME
        _put(stdscr, row, x, '│', C_BORDER);                    x += 1
        _put(stdscr, row, x, f"{'  IP':<{CW_IP}}", C_HDR);      x += CW_IP
        _put(stdscr, row, x, '│', C_BORDER);                    x += 1
        _put(stdscr, row, x, f"{'  STATUS':<{CW_STATUS}}", C_HDR); x += CW_STATUS
        _put(stdscr, row, x, '│', C_BORDER);                    x += 1
        _put(stdscr, row, x, f"{'  RTT':<{CW_RTT}}", C_HDR);   x += CW_RTT
        _put(stdscr, row, x, '│', C_BORDER)

        _hline(stdscr, 4, '├', '┼', '┤', C_BORDER)
        row = 5

        # ── client rows ────────────────────────────────────────────────────────
        for c in clients:
            dot, label, sc = STATE_STYLE.get(c['status'], ('?', c['status'], C_NORM))
            lat_str = (f"{c['latency']:.1f} ms"
                       if c['latency'] is not None else '-- ms')

            x = 2
            _put(stdscr, row, x, '│', C_BORDER);                              x += 1
            _put(stdscr, row, x, f"  {c['name']:<{CW_NAME - 2}}", C_NORM);   x += CW_NAME
            _put(stdscr, row, x, '│', C_BORDER);                              x += 1
            _put(stdscr, row, x, f"  {c['ip']:<{CW_IP - 2}}", C_NORM);       x += CW_IP
            _put(stdscr, row, x, '│', C_BORDER);                              x += 1
            # coloured indicator dot + label
            _put(stdscr, row, x,     f"  {dot} ", sc)
            _put(stdscr, row, x + 4, f"{label:<{CW_STATUS - 4}}", sc)
            x += CW_STATUS
            _put(stdscr, row, x, '│', C_BORDER);                              x += 1
            _put(stdscr, row, x, f"  {lat_str:<{CW_RTT - 2}}", C_NORM);      x += CW_RTT
            _put(stdscr, row, x, '│', C_BORDER)
            row += 1

        # ── stats footer ───────────────────────────────────────────────────────
        _hline(stdscr, row, '├', '┴', '┤', C_BORDER)
        row += 1
        stats = (f"  {len(clients)} clients  │  "
                 f"{online_cnt} online  │  "
                 f"{reach_cnt} no-loop  │  "
                 f"{offline_cnt} offline  │  "
                 f"{wait_cnt} wait  │  "
                 f"tx:{pings_sent}  rx:{pongs_rcvd}")

        _put(stdscr, row, 2, '│', C_BORDER)
        _put(stdscr, row, 3, stats[:TABLE_W - 2], C_NORM)
        _put(stdscr, row, 2 + TABLE_W - 1, '│', C_BORDER)
        row += 1

        # ── legend ─────────────────────────────────────────────────────────────
        _put(stdscr, row, 2, '│', C_BORDER)
        x = 3
        _put(stdscr, row, x, '  ● ONLINE', C_ONLINE);   x += 11
        _put(stdscr, row, x, ' ◑ NO LOOP', C_REACH);    x += 11
        _put(stdscr, row, x, ' ● OFFLINE', C_OFFLINE);  x += 11
        _put(stdscr, row, x, ' ◌ WAIT', C_WAIT);        x += 8
        _put(stdscr, row, x, f'  [icmp:{icmp_label}]', C_HINT)
        _put(stdscr, row, 2 + TABLE_W - 1, '│', C_BORDER)
        row += 1

        _put(stdscr, row, 2, '└' + '─' * (TABLE_W - 2) + '┘', C_BORDER)
        row += 1
        _put(stdscr, row, 2, "  Press CTRL+C or 'q' to exit", C_HINT)

        stdscr.refresh()
        time.sleep(0.2)


# ── entry point ────────────────────────────────────────────────────────────────

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    default_conf = os.path.join(script_dir, 'lobby.conf')
    conf_path = sys.argv[1] if len(sys.argv) > 1 else default_conf

    try:
        clients = read_lobby_conf(conf_path)
    except FileNotFoundError:
        print(f"Error: cannot find {conf_path}")
        sys.exit(1)

    if not clients:
        print("No clients found in lobby.conf (remember to skip the header line).")
        sys.exit(1)

    icmp_method = "ping3" if _HAVE_PING3 else "subprocess ping"
    print(f"Loaded {len(clients)} client(s) from {conf_path}")
    print(f"ICMP method : {icmp_method}")
    print(f"Ping port   : {PING_PORT}   (Loop.py listens here)")
    print(f"Server port : {SERVER_PORT}  (Lobby.py listens here)")
    time.sleep(0.8)

    monitor = LobbyMonitor(clients)
    monitor.start()

    try:
        curses.wrapper(lambda stdscr: draw(stdscr, monitor))
    except KeyboardInterrupt:
        pass
    finally:
        monitor.stop()
        print("\nLobby monitor stopped.")


if __name__ == '__main__':
    main()
