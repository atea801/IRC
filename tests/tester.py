import subprocess
import time
import socket

VERT = "\033[92m"
ROUGE = "\033[91m"
JAUNE = "\033[93m"
RESET = "\033[0m"

def check(description, reponse, attendu):
    if attendu in reponse:
        print("  " + VERT + "PASS" + RESET + ":", description)
    else:
        print("  " + ROUGE + "FAIL" + RESET + ":", description)
        print("    attendu :", repr(attendu))
        print("    recu    :", repr(reponse))

def check_absent(description, reponse, interdit):
    if interdit not in reponse:
        print("  " + VERT + "PASS" + RESET + ":", description)
    else:
        print("  " + ROUGE + "FAIL" + RESET + ":", description)
        print("    ne devrait pas contenir :", repr(interdit))
        print("    recu                    :", repr(reponse))

def lire(client):
    time.sleep(0.3)
    try:
        return client.recv(4096).decode(errors="ignore")
    except (socket.timeout, TimeoutError):
        return ""

def nouveau_client(nick, user=None):
    if user is None:
        user = nick
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(("127.0.0.1", 6667))
    s.settimeout(0.3)
    s.send(("PASS toto\r\nNICK " + nick + "\r\nUSER " + user + " 0 * :" + user + "\r\n").encode())
    return s

subprocess.run(["pkill", "irc"])
time.sleep(0.5)

#lancement du server
server = subprocess.Popen(["./irc", "6667", "toto"])
print("Server lancer")
time.sleep(2)

#test pour savoir si le server est vraiment lancer
code = server.poll()
print("poll() renvoie :", code) 
if code is None:
    print("OK : le serveur tourne")
else:
    print("ERREUR : le serveur s'est arrete tout seul (crash ?)")

print(JAUNE + "=== NICK ===" + RESET)

c = nouveau_client("tom_[42]")
check("nick avec caracteres autorises -> 001", lire(c), "001")
c.close()

c = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
c.connect(("127.0.0.1", 6667)); c.settimeout(0.3)
c.send(b"PASS toto\r\nNICK 1tom\r\n")
check("nick commencant par un chiffre -> 432", lire(c), "432")
c.close()

c = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
c.connect(("127.0.0.1", 6667)); c.settimeout(0.3)
c.send(b"PASS toto\r\nNICK abcdefghijklmnop\r\n")
rep = lire(c)
check_absent("nick trop long -> pas de welcome", rep, "Welcome")
c.close()

c = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
c.connect(("127.0.0.1", 6667)); c.settimeout(0.3)
c.send(b"PASS toto\r\nNICK tom@home\r\n")
check("nick avec @ -> 432", lire(c), "432")
c.close()

c = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
c.connect(("127.0.0.1", 6667)); c.settimeout(0.3)
c.send(b"PASS toto\r\nNICK\r\n")
check("NICK sans argument -> 431", lire(c), "431")
c.close()

print(JAUNE + "=== DOUBLON ===" + RESET)

c1 = nouveau_client("doublon")
lire(c1)
c2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
c2.connect(("127.0.0.1", 6667)); c2.settimeout(0.3)
c2.send(b"PASS toto\r\nNICK doublon\r\nUSER autre 0 * :autre\r\n")
check("nick deja pris -> 433", lire(c2), "433")
c1.close(); c2.close()

print(JAUNE + "=== ERREURS ENREGISTREMENT ===" + RESET)

c = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
c.connect(("127.0.0.1", 6667)); c.settimeout(0.3)
c.send(b"PASS mauvais\r\nNICK x\r\nUSER x 0 * :x\r\n")
rep = lire(c)
check("mauvais password -> 464", rep, "464")
check_absent("mauvais password -> pas de welcome", rep, "001")
c.close()

c = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
c.connect(("127.0.0.1", 6667)); c.settimeout(0.3)
c.send(b"JOIN #test\r\n")
check("commande avant enregistrement -> 451", lire(c), "451")
c.close()

c = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
c.connect(("127.0.0.1", 6667)); c.settimeout(0.3)
c.send(b"PASS toto\r\nNICK bob\r\nUSER bob\r\n")
check("USER incomplet -> 461", lire(c), "461")
c.close()

print(JAUNE + "=== PRIVMSG ===" + RESET)

c1 = nouveau_client("emetteur")
c2 = nouveau_client("recepteur")
lire(c1); lire(c2)
c1.send(b"PRIVMSG recepteur :coucou\r\n")
rep = lire(c2)
check("PRIVMSG recu par destinataire", rep, "coucou")
check("PRIVMSG contient l'emetteur", rep, "emetteur")
c1.close(); c2.close()

c = nouveau_client("solo")
lire(c)
c.send(b"PRIVMSG fantome :hello\r\n")
check("PRIVMSG vers nick inexistant -> 401", lire(c), "401")
c.close()

c = nouveau_client("solo2")
lire(c)
c.send(b"PRIVMSG solo2\r\n")   # sans message -> args[1] piege
check("le serveur survit a PRIVMSG sans message", lire(nouveau_client("verif1")), "001")
c.close()

print(JAUNE + "=== JOIN ===" + RESET)

c1 = nouveau_client("chanA")
lire(c1)
c1.send(b"JOIN #salon\r\n")
check("JOIN renvoie un message JOIN", lire(c1), "JOIN")

c1.send(b"JOIN #salon\r\n")   # deuxieme fois
check_absent("second JOIN ignore (pas de double)", lire(c1), "JOIN")
c1.close()

c1 = nouveau_client("chanB")
c2 = nouveau_client("chanC")
lire(c1); lire(c2)
c1.send(b"JOIN #salon2\r\n"); lire(c1)
c2.send(b"JOIN #salon2\r\n"); lire(c2)
lire(c1)
c1.send(b"PRIVMSG #salon2 :bonjour\r\n")
check("message channel recu par l'autre", lire(c2), "bonjour")
rep_emetteur = lire(c1)
check_absent("emetteur ne recoit pas son propre message channel", rep_emetteur, "bonjour")
c1.close(); c2.close()

#on tue le server proprement
server.terminate()
server.wait()
print("Server s arrete proprement")
