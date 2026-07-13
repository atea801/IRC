import os
import signal
import socket
import subprocess
import time
import pytest


SERVER_HOST = "localhost"
SERVER_PORT = int(os.getenv("IRC_TEST_PORT", "6667"))
SERVER_PATH = os.getenv("SERVER_BIN_PATH", "./ircserv")
TEST_PASSWORD = os.getenv("IRC_TEST_PASS", "password")


def wait_for_port(host, port, timeout=5):
    """Poll until TCP port accepts connections."""
    start = time.time()
    while time.time() - start < timeout:
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(1)
            result = sock.connect_ex((host, port))
            sock.close()
            if result == 0:
                return True
        except Exception:
            pass
        time.sleep(0.2)
    return False


@pytest.fixture(scope="session")
def irc_server(request):
    """
    Start IRC server before session, stop after.
    Tracks PID for signal testing.
    """
    server_process = None
    server_pid = None

    # Build if necessary
    if not os.path.exists(SERVER_PATH):
        pytest.skip(f"Server binary not found at {SERVER_PATH}. Run 'make' first.")

    # Start server
    cmd = [SERVER_PATH, str(SERVER_PORT), TEST_PASSWORD]
    server_process = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    server_pid = server_process.pid

    # Wait for readiness
    if not wait_for_port(SERVER_HOST, SERVER_PORT):
        pytest.fail(f"Server failed to start on port {SERVER_PORT}")

    yield

    # Teardown: graceful shutdown
    if server_process.poll() is None:
        server_process.send_signal(signal.SIGTERM)
        try:
            server_process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            server_process.kill()


@pytest.fixture
def tcp_client(irc_server):
    """Raw TCP socket for protocol testing."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((SERVER_HOST, SERVER_PORT))
    sock.settimeout(5)
    yield sock
    sock.close()


@pytest.fixture(scope="class")
def class_teardown(request):
    """Ensure server cleanup even if class tests fail."""
    yield
    cls = getattr(request, "cls", None)
    if cls is not None and hasattr(cls, "server_process"):
        proc = getattr(cls, "server_process")
        if proc and proc.poll() is None:
            proc.terminate()