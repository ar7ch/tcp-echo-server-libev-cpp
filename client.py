#!/usr/bin/env python3

import argparse
import asyncio
import sys
from asyncio import StreamReader, StreamWriter


def parse_args() -> argparse.Namespace:
    """
    Parse command-line arguments.
    Returns:
        argparse.Namespace: Parsed arguments with ip, port, and oneshot message (optional).
    """
    parser = argparse.ArgumentParser(description="Async TCP Client")
    parser.add_argument("ip", help="Server IP")
    parser.add_argument("port", type=int, help="Server Port")
    parser.add_argument(
        "-o", "--oneshot", metavar="MESSAGE", help="Send one message and exit"
    )
    return parser.parse_args()


async def oneshot_mode(ip: str, port: int, message: str) -> None:
    """
    One-shot mode: Connect to server, send a single message, wait for response, and exit.

    Args:
        ip (str): Server IP address.
        port (int): Server port number.
        message (str): Message to send.
    """
    reader, writer = await asyncio.open_connection(ip, port)
    print(f"Connected to {ip}:{port}")
    writer.write(message.encode())
    await writer.drain()

    expected_len = len(message)
    data = b""
    while len(data) < expected_len:
        chunk = await reader.read(4096)
        if not chunk:
            break
        data += chunk
    print(data.decode(), end="")
    writer.close()
    await writer.wait_closed()


async def handle_input(writer: StreamWriter) -> None:
    """
    Handle user input from stdin and send to server.

    Args:
        writer (StreamWriter): StreamWriter connected to server.
    """
    loop = asyncio.get_event_loop()
    while True:
        try:
            line = await loop.run_in_executor(None, sys.stdin.readline)
            if not line:  # Ctrl+D
                break
            writer.write(line.encode())
            await writer.drain()
        except (EOFError, KeyboardInterrupt):
            break


async def handle_response(reader: StreamReader) -> None:
    """
    Handle incoming messages from the server and print them.

    Args:
        reader (StreamReader): StreamReader connected to server.
    """
    try:
        while True:
            data = await reader.read(4096)
            if not data:
                break
            print("Server: " + data.decode(), end="")
    except asyncio.CancelledError:
        pass


async def interactive_mode(ip: str, port: int) -> None:
    """
    Interactive mode: Connect to server and allow bidirectional communication.

    Args:
        ip (str): Server IP address.
        port (int): Server port number.
    """
    reader, writer = await asyncio.open_connection(ip, port)
    print(f"Connected to {ip}:{port}")
    print("Input your text and press <enter> to send.")
    print("Press CTRL+D to exit.")
    try:
        input_task = asyncio.create_task(handle_input(writer))
        response_task = asyncio.create_task(handle_response(reader))
        _ = await asyncio.wait(
            [input_task, response_task], return_when=asyncio.FIRST_COMPLETED
        )
        _ = response_task.cancel()
    except KeyboardInterrupt:
        pass
    finally:
        writer.close()
        await writer.wait_closed()


async def main() -> None:
    """
    Entry point: Parse arguments and run appropriate mode.
    """
    try:
        args = parse_args()
        if args.oneshot:
            await oneshot_mode(args.ip, args.port, args.oneshot)
        else:
            await interactive_mode(args.ip, args.port)
    except KeyboardInterrupt:
        print("\nClient exited.")


if __name__ == "__main__":
    asyncio.run(main())
