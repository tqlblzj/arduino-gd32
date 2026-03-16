#!/usr/bin/env python3
"""
Simple NTP Server
A basic NTP server that provides time synchronization for local network devices.
Run this on a computer in your local network to serve NTP requests.
"""

import socket
import struct
import time
from datetime import datetime, timezone

# NTP server configuration
NTP_HOST = '0.0.0.0'  # Listen on all interfaces
NTP_PORT = 123

# NTP timestamp epoch (1900-01-01 00:00:00)
NTP_DELTA = 2208988800  # Seconds between 1900-01-01 and 1970-01-01

def get_ntp_timestamp():
    """
    Get current time as NTP timestamp
    Returns: NTP timestamp (seconds since 1900-01-01)
    """
    return time.time() + NTP_DELTA

def create_ntp_response(request_data, client_address):
    """
    Create NTP response packet

    Args:
        request_data: NTP request packet
        client_address: Client (address, port)

    Returns: NTP response packet
    """
    # Parse request
    try:
        # First byte: LI (2 bits), VN (3 bits), Mode (3 bits)
        first_byte = request_data[0] if len(request_data) > 0 else 0
        version = (first_byte & 0x38) >> 3
        mode = first_byte & 0x07

        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] "
              f"NTP request from {client_address[0]}:{client_address[1]} "
              f"(Version: {version}, Mode: {mode}, Length: {len(request_data)})")

        # Log request packet for debugging
        if len(request_data) >= 48:
            transmit_ts = struct.unpack('!I', request_data[40:44])[0]
            print(f"  Request Transmit Timestamp: {transmit_ts}")

        # Get current time
        current_time = time.time()
        ntp_seconds = int(current_time + NTP_DELTA)
        ntp_fraction = int((current_time % 1) * 2**32)

        # Build NTP response packet (48 bytes)
        # Byte 0: LI=0, VN=4, Mode=4 (Server)
        response = bytearray(48)
        response[0] = 0x24  # LI=0, VN=4, Mode=4

        # Bytes 1-3: Stratum, Poll, Precision
        response[1] = 1     # Stratum (1 = primary reference)
        response[2] = 6     # Poll (6 = 64 seconds)
        response[3] = 0xEC  # Precision (2^-20 seconds)

        # Bytes 4-7: Root Delay
        response[4:8] = struct.pack('!I', 0)

        # Bytes 8-11: Root Dispersion
        response[8:12] = struct.pack('!I', 0)

        # Bytes 12-15: Reference Identifier (ASCII "LOCL")
        response[12:16] = b'LOCL'

        # Bytes 16-23: Reference Timestamp (time when clock was last set)
        response[16:24] = struct.pack('!II', ntp_seconds, ntp_fraction)

        # Bytes 24-31: Originate Timestamp (client request time)
        if len(request_data) >= 48:
            response[24:32] = request_data[40:48]  # Transmit timestamp from request
        else:
            response[24:32] = struct.pack('!II', 0, 0)

        # Bytes 32-39: Receive Timestamp (time when request received)
        response[32:40] = struct.pack('!II', ntp_seconds, ntp_fraction)

        # Bytes 40-47: Transmit Timestamp (time of response transmission)
        response[40:48] = struct.pack('!II', ntp_seconds, ntp_fraction)

        return bytes(response)

    except Exception as e:
        print(f"Error creating NTP response: {e}")
        return None

def start_ntp_server():
    """
    Start NTP server
    """
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    try:
        sock.bind((NTP_HOST, NTP_PORT))
        print(f"NTP Server listening on {NTP_HOST}:{NTP_PORT}")
        print(f"Current time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print("Press Ctrl+C to stop...")
        print()

        while True:
            try:
                # Receive NTP request
                data, client_address = sock.recvfrom(512)

                # Create and send response
                response = create_ntp_response(data, client_address)
                if response:
                    # Log response packet for debugging
                    resp_transmit_ts = struct.unpack('!I', response[40:44])[0]
                    print(f"  Response Transmit Timestamp: {resp_transmit_ts}")

                    bytes_sent = sock.sendto(response, client_address)
                    print(f"  -> Sent {bytes_sent} bytes response to {client_address[0]}:{client_address[1]}")
                else:
                    print(f"  -> Failed to create response")

            except KeyboardInterrupt:
                print("\nShutting down NTP server...")
                break
            except Exception as e:
                print(f"Error handling request: {e}")

    finally:
        sock.close()
        print("NTP server stopped.")

if __name__ == '__main__':
    print("=" * 60)
    print("Simple NTP Server")
    print("=" * 60)
    print()
    print("This server provides NTP time synchronization for local devices.")
    print()
    print("Usage:")
    print("  1. Run this script on a computer in your local network")
    print("  2. Configure your GD32 device to use this computer's IP as NTP server")
    print("  3. Example: #define NTP_SERVER \"192.168.1.100\"")
    print()
    print(f"Server will listen on: {NTP_HOST}:{NTP_PORT}")
    print()
    print("=" * 60)
    print()

    start_ntp_server()
