import pytest
import signal
import os
import time
import subprocess
import socket

from .conftest import SERVER_PATH, TEST_PASSWORD, SERVER_PORT, SERVER_HOST


class TestLifecycleSignals:
    """Server response to Ctrl+C, Ctrl+D, Ctrl+Z scenarios."""

    def test_sigterm_graceful_shutdown(self, request):
        """Server handles TERM signal gracefully."""
        server_proc = subprocess.Popen(
            [SERVER_PATH, str(SERVER_PORT + 3), TEST_PASSWORD],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

        time.sleep(1)
        server_proc.send_signal(signal.SIGTERM)

        try:
            server_proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            server_proc.kill()
            pytest.fail("Server did not terminate within 5s of SIGTERM")

        assert server_proc.returncode is not None

    def test_sigint_handler(self, request):
        """Ctrl+C (SIGINT) triggers cleanup."""
        test_server = subprocess.Popen(
            [SERVER_PATH, str(SERVER_PORT + 4), TEST_PASSWORD],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        
        # Wait for start
        time.sleep(1)
        test_server.send_signal(signal.SIGINT)
        
        try:
            stdout, stderr = test_server.communicate(timeout=5)
            # Should exit cleanly
            assert test_server.returncode is not None, "SIGINT handler hung"
        except subprocess.TimeoutExpired:
            test_server.kill()
            pytest.fail("SIGINT did not trigger graceful shutdown")

    @pytest.mark.xfail(reason="Current server does not exit on EOF from stdin")
    def test_eof_ctrl_d_handling(self, request):
        """EOF on stdin closes server gracefully."""
        test_server = subprocess.Popen(
            [SERVER_PATH, str(SERVER_PORT + 5), TEST_PASSWORD],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        
        time.sleep(1)
        
        # Close stdin (simulates Ctrl+D)
        test_server.stdin.close()
        
        try:
            stdout, stderr = test_server.communicate(timeout=5)
            # Exit code doesn't matter too much, but shouldn't hang
            assert test_server.returncode is not None, "EOF handling caused hang"
        except subprocess.TimeoutExpired:
            test_server.kill()
            pytest.fail("Ctrl+D (EOF) caused server to hang")

    @pytest.mark.xfail(reason="SIGSTOP requires terminal in some environments")
    def test_sigstop_resume(self, request):
        """Server survives SIGSTOP/SIGCONT cycle (Ctrl+Z behavior)."""
        test_server = subprocess.Popen(
            [SERVER_PATH, str(SERVER_PORT + 6), TEST_PASSWORD],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

        time.sleep(1)

        # Stop
        os.kill(test_server.pid, signal.SIGSTOP)
        time.sleep(1)
        
        # Resume
        os.kill(test_server.pid, signal.SIGCONT)
        time.sleep(1)
        
        # Server should still be responsive
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(1)
        try:
            result = sock.connect_ex((SERVER_HOST, SERVER_PORT + 6))
            sock.close()
            assert result == 0, "Server unresponsive after SIGSTOP/SIGCONT"
        except Exception:
            pytest.fail("Server lost connectivity after suspend/resume")
        finally:
            test_server.terminate()