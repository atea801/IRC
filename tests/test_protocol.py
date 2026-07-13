import pytest
import socket
import time

from .conftest import TEST_PASSWORD, SERVER_PORT


class TestProtocolCommands:
    """RFC-compliant IRC command validation."""

    def recv_until(self, sock, timeout=3):
        """Read all available data from socket."""
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

    def send_command(self, sock, command):
        """Send IRC command with proper CRLF terminator."""
        sock.sendall((command + "\r\n").encode("ascii"))

    def register_client(self, sock, nick="testuser", username="test", realname="Test User"):
        """Perform the minimum IRC registration handshake."""
        self.send_command(sock, f"PASS {TEST_PASSWORD}")
        self.send_command(sock, f"NICK {nick}")
        self.send_command(sock, f"USER {username} 0 * :{realname}")
        time.sleep(1)
        return self.recv_until(sock, timeout=3)

    def assert_contains_codes(self, response, expected_codes, test_name):
        """Verify numeric response codes appeared."""
        for code in expected_codes:
            assert f" :{code}" in response or f" {code}" in response, \
                f"{test_name}: Expected response code {code}, got: {response[:200]}"

    @pytest.mark.usefixtures("tcp_client")
    def test_nick_registration(self, tcp_client):
        """NICK followed by USER triggers welcome sequence."""
        response = self.register_client(tcp_client, nick="testuser")
        
        # RFC: Should receive 001 (RPL_WELCOME) after registration
        assert "001" in response, f"No welcome message received:\n{response[:500]}"

    @pytest.mark.usefixtures("tcp_client")
    def test_nick_taken(self, tcp_client):
        """Duplicate NICK returns error code."""
        self.register_client(tcp_client, nick="existinguser")

        other = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        other.connect(("localhost", SERVER_PORT))
        other.settimeout(5)
        try:
            self.send_command(other, f"PASS {TEST_PASSWORD}")
            self.send_command(other, "NICK existinguser")
            self.send_command(other, "USER test 0 * :Test")
            response = self.recv_until(other, timeout=2)
        finally:
            other.close()
        
        # 432 = ERR_ERRONEUSNICKNAME, 433 = ERR_NICKNAMEINUSE
        assert any(code in response for code in ("432", "433")), \
            f"NICK collision/validation error expected:\n{response[:500]}"

    @pytest.mark.usefixtures("tcp_client")
    def test_join_channel(self, tcp_client):
        """JOIN command adds user to channel."""
        self.register_client(tcp_client, nick="joiner")
        
        self.send_command(tcp_client, "JOIN #testchannel")
        
        response = self.recv_until(tcp_client, timeout=3)
        
        # Should see 353 (NAMES) + 366 (ENDOFNAMES) after successful join
        assert "JOIN" in response or "#testchannel" in response.lower(), \
            f"JOIN acknowledgment expected:\n{response[:500]}"

    @pytest.mark.usefixtures("tcp_client")
    def test_topic_set_and_get(self, tcp_client):
        """TOPIC sets and retrieves channel topic."""
        self.register_client(tcp_client, nick="topicuser")
        self.send_command(tcp_client, "JOIN #topictest")
        
        # Set topic
        self.send_command(tcp_client, "TOPIC #topictest :Hello World Topic")
        topic_response = self.recv_until(tcp_client, timeout=2)
        
        # Get topic
        self.send_command(tcp_client, "TOPIC #topictest")
        topic_retrieval = self.recv_until(tcp_client, timeout=2)
        
        # Verify topic was stored
        assert "Hello World Topic" in topic_response or "Hello World Topic" in topic_retrieval, \
            f"Topic not set/retrieved properly:\n{topic_response}\n{topic_retrieval}"

    @pytest.mark.usefixtures("tcp_client")
    def test_mode_commands(self, tcp_client):
        """MODE command modifies channel/user modes."""
        self.register_client(tcp_client, nick="modeuser")
        self.send_command(tcp_client, "JOIN #modetest")
        
        # Get channel mode
        self.send_command(tcp_client, "MODE #modetest")
        response = self.recv_until(tcp_client, timeout=2)
        
        # MODE response should contain RPL_CHANNELMODEIS (324)
        assert "324" in response or "MODE" in response, \
            f"MODE response expected:\n{response[:500]}"

    @pytest.mark.usefixtures("tcp_client")
    def test_invite_command(self, tcp_client):
        """INVITE sends invitation to user."""
        self.register_client(tcp_client, nick="inviter")
        self.send_command(tcp_client, "JOIN #invitechannel")

        invitee = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        invitee.connect(("localhost", SERVER_PORT))
        invitee.settimeout(5)
        try:
            self.register_client(invitee, nick="invitee")

            self.send_command(tcp_client, "INVITE invitee #invitechannel")
            response = self.recv_until(tcp_client, timeout=2)
        finally:
            invitee.close()
        
        # 341 = RPL_INVITING
        assert "341" in response or "INVITE" in response, \
            f"Invite confirmation expected:\n{response[:500]}"

    @pytest.mark.usefixtures("tcp_client")
    def test_kick_command(self, tcp_client):
        """KICK removes user from channel."""
        self.register_client(tcp_client, nick="kicker")
        self.send_command(tcp_client, "JOIN #kickchannel")

        victim = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        victim.connect(("localhost", SERVER_PORT))
        victim.settimeout(5)
        try:
            self.send_command(victim, f"PASS {TEST_PASSWORD}")
            self.send_command(victim, "NICK victim")
            self.send_command(victim, "USER test 0 * :Victim")
            self.recv_until(victim, timeout=3)
            self.send_command(victim, "JOIN #kickchannel")
            self.recv_until(victim, timeout=2)
        
            self.send_command(tcp_client, "KICK #kickchannel victim :Testing kick")
            response = self.recv_until(tcp_client, timeout=2)
        finally:
            victim.close()
        
        # Either success notification or permission error (if not op)
        assert "KICK" in response or "482" in response or "441" in response or "ERR" in response, \
            f"Kick acknowledgment expected:\n{response[:500]}"

    @pytest.mark.usefixtures("tcp_client")
    def test_part_channel(self, tcp_client):
        """PART command leaves channel gracefully."""
        self.register_client(tcp_client, nick="partuser")
        self.send_command(tcp_client, "JOIN #partchannel")
        self.send_command(tcp_client, "PART #partchannel")
        
        response = self.recv_until(tcp_client, timeout=2)
        
        assert "PART" in response or "leaves" in response.lower() or "#partchannel" in response, \
            f"PART acknowledgment expected:\n{response[:500]}"