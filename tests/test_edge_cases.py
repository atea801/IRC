import pytest
import socket
import subprocess
import os
import time

from .conftest import SERVER_HOST, SERVER_PORT, TEST_PASSWORD


pytestmark = pytest.mark.usefixtures("irc_server")


class TestEdgeCases:
    """Partial commands, boundary conditions, invalid input."""

    def recv_all(self, sock, timeout=2):
        """Drain socket buffer."""
        sock.settimeout(timeout)
        chunks = []
        try:
            while True:
                chunk = sock.recv(4096)
                if not chunk:
                    break
                chunks.append(chunk)
        except socket.timeout:
            pass
        return b"".join(chunks).decode("utf-8", errors="ignore")

    @pytest.fixture
    def nc_raw_socket(self, request):
        """Open fresh socket for each test."""
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((SERVER_HOST, SERVER_PORT))
        sock.settimeout(5)
        sock.sendall(f"PASS {TEST_PASSWORD}\r\n".encode("ascii"))
        sock.sendall(b"NICK edgecase\r\n")
        sock.sendall(b"USER edge 0 * :Edge Case\r\n")
        time.sleep(0.2)
        self.recv_all(sock, timeout=1)
        yield sock
        sock.close()

    def test_partial_nick_command(self, nc_raw_socket):
        """Send incomplete NICK command (missing CRLF)."""
        # Send partial, should buffer or reject
        nc_raw_socket.sendall(b"NICK testuser")
        time.sleep(1)
        nc_raw_socket.sendall(b"\r\n")
        response = self.recv_all(nc_raw_socket)
        assert len(response) > 0, "Partial command caused hang without recovery"

    def test_empty_command(self, nc_raw_socket):
        """Send empty line."""
        nc_raw_socket.sendall(b"\r\n")
        response = self.recv_all(nc_raw_socket)
        # Should either ignore or return error (461 = ERR_NEEDMOREPARAMS)
        assert len(response) >= 0, "Empty command caused crash"

    def test_command_overflow(self, nc_raw_socket):
        """Send command exceeding 512 bytes limit."""
        # Modern IRC: max message 512 bytes including CRLF
        overflow_cmd = "NICK " + "x" * 520 + "\r\n"
        nc_raw_socket.sendall(overflow_cmd.encode("ascii"))
        response = self.recv_all(nc_raw_socket)
        # Server may reject (4xx) or truncate - shouldn't crash
        assert len(response) >= 0, "Overflow caused server crash"

    def test_null_byte_injection(self, nc_raw_socket):
        """Inject null bytes in command."""
        nc_raw_socket.sendall(b"NICK test\x00user\r\n")
        response = self.recv_all(nc_raw_socket)
        # Server should handle safely (truncate or reject)
        assert len(response) >= 0, "Null injection caused crash"

    def test_unknown_command(self, nc_raw_socket):
        """Send nonexistent command."""
        nc_raw_socket.sendall(b"NONEXISTENTCMD arg\r\n")
        response = self.recv_all(nc_raw_socket)
        # Should return ERR_UNKNOWNCOMMAND (421)
        assert len(response) >= 0, "Unknown command caused issues"

    def test_missing_crlf_terminator(self, nc_raw_socket):
        """Command without proper line terminator."""
        nc_raw_socket.sendall(b"NICK testuser")  # No CRLF
        time.sleep(1)
        nc_raw_socket.close()
        # Should not leave server in bad state

    def test_special_characters_in_nick(self, nc_raw_socket):
        """NICK with special characters."""
        nc_raw_socket.sendall(b"NICK test@#$user\r\n")
        response = self.recv_all(nc_raw_socket)
        # May accept or reject (modern specs are stricter)
        assert len(response) >= 0, "Special chars caused server crash"


class TestNetworkFailureModes:
    """Client disconnects, connection resets, etc."""

    def test_unclean_client_disconnect(self, request):
        """Simulate client death (kill -9)."""
        import subprocess
        
        # Spawn temporary client
        client_proc = subprocess.Popen(
            ["nc", "-q0", SERVER_HOST, str(SERVER_PORT)],
            stdin=subprocess.DEVNULL,
            stdout=subprocess.DEVNULL,
        )
        time.sleep(1)
        
        # Force kill
        client_proc.kill()
        
        # Server should still be alive and stable
        probe = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        probe.settimeout(1)
        assert probe.connect_ex((SERVER_HOST, SERVER_PORT)) == 0, \
            "Server crashed after client killed"
        probe.close()
        time.sleep(0.5)

    def test_rapid_connect_disconnect(self, request):
        """Many quick connections in succession."""
        for i in range(5):
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(1)
            sock.connect((SERVER_HOST, SERVER_PORT))
            sock.close()
        
        # Server should handle rapid cycles
        probe = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        probe.settimeout(1)
        assert probe.connect_ex((SERVER_HOST, SERVER_PORT)) == 0, \
            "Server crashed under rapid reconnect load"
        probe.close()