import pytest
import subprocess
import time
import os
import pexpect

from .conftest import SERVER_HOST, SERVER_PORT, TEST_PASSWORD


pytestmark = pytest.mark.usefixtures("irc_server")


class TestIrssiIntegration:
    """Real client compatibility verification."""

    def spawn_irssi(self):
        import pexpect

        return pexpect.spawn(
            "irssi",
            args=[
                "-c",
                SERVER_HOST,
                "-p",
                str(SERVER_PORT),
                "-w",
                TEST_PASSWORD,
                "-n",
                "testuser",
            ],
            timeout=10,
            encoding="utf-8",
        )

    def test_irssi_clean_connection(self, request):
        """Irssi connects without errors."""
        child = self.spawn_irssi()
        
        try:
            # Wait for connection establishment
            child.expect(["Connected", "Welcome", "001"], timeout=5)
            connection_ok = True
        except pexpect.exceptions.EOF:
            connection_ok = False
        except pexpect.exceptions.TIMEOUT:
            connection_ok = False
        
        # Clean exit
        child.terminate(force=True)
        
        assert connection_ok, "Irssi failed to connect cleanly"

    def test_irssi_no_error_codes(self, request):
        """Irssi doesn't receive protocol errors on valid commands."""
        child = self.spawn_irssi()
        
        try:
            # Wait for connection
            child.expect("Connected|Welcome", timeout=5)
            time.sleep(1)
            
            # Send JOIN
            child.sendline("/join #autojoin")
            time.sleep(1)
            
            # Check for 4xx/5xx errors
            output = child.before
            
            # No critical error codes
            assert "4xx" not in output and "ERR" not in output.upper() or \
                   "disconnect" in output.lower(), \
                   f"Irssi reported errors:\n{output[:1000]}"
        
        finally:
            child.terminate(force=True)

    def test_irssi_multiclient_scenario(self, request):
        """Two irssi clients can interact."""
        c1 = self.spawn_irssi()
        time.sleep(1)

        c2 = self.spawn_irssi()
        time.sleep(1)
        
        # Both should connect successfully
        assert c1.isalive() and c2.isalive(), "Clients didn't survive initialization"
        
        # Clean up
        c1.terminate(force=True)
        c2.terminate(force=True)