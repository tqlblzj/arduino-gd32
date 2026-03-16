#!/usr/bin/env python3
"""
UDP Client for testing GD32VW55x WiFiUDPServer example

This script connects to the UDP server running on GD32VW55x board,
sends test messages, and displays the echoed responses.

Usage:
    python udp_client.py

Configuration:
    - Server IP: 192.168.237.1 (default AP IP of GD32_UDPServer)
    - Server Port: 3333
    - Make sure to connect your computer to "GD32_UDPServer" WiFi network first
"""

import socket
import time
import sys

# UDP server configuration
UDP_IP = "192.168.237.1"  # Default IP of GD32_UDPServer SoftAP
UDP_PORT = 3333

# Test configuration
PACKET_COUNT = 5        # Number of test packets to send
PACKET_INTERVAL = 1      # Seconds between packets
SOCKET_TIMEOUT = 5       # Seconds to wait for response

def print_header():
    """Print the header banner"""
    print("\n" + "=" * 50)
    print("  GD32VW55x UDP Client Test")
    print("=" * 50)
    print()

def print_config():
    """Print the configuration"""
    print("Configuration:")
    print(f"  Server IP:    {UDP_IP}")
    print(f"  Server Port:  {UDP_PORT}")
    print(f"  Packet Count: {PACKET_COUNT}")
    print(f"  Interval:     {PACKET_INTERVAL} seconds")
    print(f"  Timeout:      {SOCKET_TIMEOUT} seconds")
    print()

def test_udp_connection():
    """Test UDP connection with a single packet"""
    print("Testing UDP connection...")

    try:
        # Create UDP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.settimeout(SOCKET_TIMEOUT)

        # Send test packet
        test_message = b"Connection Test"
        print(f"  Sending: '{test_message.decode()}'")
        sock.sendto(test_message, (UDP_IP, UDP_PORT))

        # Wait for response
        try:
            data, addr = sock.recvfrom(1024)
            print(f"  Received: '{data.decode()}'")
            print(f"  From: {addr[0]}:{addr[1]}")
            print("  Connection test: PASSED")
            sock.close()
            return True
        except socket.timeout:
            print("  Connection test: FAILED (timeout)")
            sock.close()
            return False

    except Exception as e:
        print(f"  Connection test: FAILED ({e})")
        return False

def run_udp_test():
    """Run the UDP test sequence"""
    print("\n" + "-" * 50)
    print("Starting UDP Test Sequence")
    print("-" * 50)
    print()

    try:
        # Create UDP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.settimeout(SOCKET_TIMEOUT)

        # Bind to a local port (optional, allows receiving responses)
        sock.bind(('', 0))
        local_port = sock.getsockname()[1]
        print(f"Local bound to port: {local_port}")
        print()

        success_count = 0
        fail_count = 0

        # Send test packets
        for i in range(PACKET_COUNT):
            # Create test message
            message = f"Test Packet {i+1}/{PACKET_COUNT} - Timestamp: {time.time():.2f}"
            message_bytes = message.encode()

            print(f"[Packet {i+1}/{PACKET_COUNT}]")
            print(f"  Sending: '{message}'")

            try:
                # Send packet
                sock.sendto(message_bytes, (UDP_IP, UDP_PORT))

                # Receive response
                data, addr = sock.recvfrom(1024)

                print(f"  Received: '{data.decode()}'")
                print(f"  From: {addr[0]}:{addr[1]}")
                print(f"  Status: SUCCESS")
                success_count += 1

            except socket.timeout:
                print(f"  Status: FAILED (timeout)")
                fail_count += 1
            except Exception as e:
                print(f"  Status: FAILED ({e})")
                fail_count += 1

            print()

            # Wait before sending next packet
            if i < PACKET_COUNT - 1:
                time.sleep(PACKET_INTERVAL)

        # Close socket
        sock.close()

        # Print summary
        print("-" * 50)
        print("Test Summary")
        print("-" * 50)
        print(f"  Total Packets:  {PACKET_COUNT}")
        print(f"  Successful:     {success_count}")
        print(f"  Failed:         {fail_count}")
        print(f"  Success Rate:   {success_count/PACKET_COUNT*100:.1f}%")
        print()

        return success_count > 0

    except Exception as e:
        print(f"Error: {e}")
        return False

def interactive_mode():
    """Interactive mode for manual testing"""
    print("\n" + "-" * 50)
    print("Interactive Mode")
    print("-" * 50)
    print("Type your message and press Enter to send.")
    print("Type 'quit' or 'exit' to exit.")
    print()

    try:
        # Create UDP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.settimeout(SOCKET_TIMEOUT)
        sock.bind(('', 0))
        local_port = sock.getsockname()[1]

        print(f"Local bound to port: {local_port}")
        print(f"Connected to {UDP_IP}:{UDP_PORT}")
        print()

        while True:
            try:
                # Get user input
                message = input("udp_client> ").strip()

                # Check for exit command
                if message.lower() in ['quit', 'exit', 'q']:
                    print("Exiting...")
                    break

                # Skip empty messages
                if not message:
                    continue

                # Send message
                message_bytes = message.encode()
                sock.sendto(message_bytes, (UDP_IP, UDP_PORT))
                print(f"Sent: '{message}'")

                # Receive response
                data, addr = sock.recvfrom(1024)
                print(f"Received: '{data.decode()}'")
                print()

            except socket.timeout:
                print("Timeout waiting for response")
                print()
            except KeyboardInterrupt:
                print("\nExiting...")
                break
            except Exception as e:
                print(f"Error: {e}")
                print()

        sock.close()

    except Exception as e:
        print(f"Error: {e}")

def main():
    """Main function"""
    print_header()
    print_config()

    # Check command line arguments
    if len(sys.argv) > 1:
        if sys.argv[1] in ['-i', '--interactive']:
            # Run interactive mode
            interactive_mode()
            return
        elif sys.argv[1] in ['-t', '--test']:
            # Run test mode
            if test_udp_connection():
                run_udp_test()
            return
        elif sys.argv[1] in ['-h', '--help']:
            print("Usage:")
            print("  python udp_client.py              - Run automatic test")
            print("  python udp_client.py --test       - Run connection test + automatic test")
            print("  python udp_client.py --interactive - Run interactive mode")
            print("  python udp_client.py --help       - Show this help")
            return

    # Default: run connection test first, then automatic test
    if test_udp_connection():
        input("\nPress Enter to start automatic test, or Ctrl+C to cancel...")
        run_udp_test()

        # Ask if user wants to try interactive mode
        try:
            answer = input("\nDo you want to try interactive mode? (y/n): ").strip().lower()
            if answer in ['y', 'yes']:
                interactive_mode()
        except KeyboardInterrupt:
            print("\nExiting...")
    else:
        print("\nConnection test failed. Please check:")
        print("  1. Your computer is connected to 'GD32_UDPServer' WiFi")
        print("  2. The UDP server is running on the GD32VW55x board")
        print("  3. The server IP is correct (default: 192.168.4.1)")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nInterrupted by user")
        sys.exit(0)
