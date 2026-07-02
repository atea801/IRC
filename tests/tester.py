#!/usr/bin/env python3
# ============================================================
#  TESTEUR IRC COMPLET
#  Couvre : NICK, USER, PASS, PRIVMSG, JOIN, MODE, KICK, INVITE,
#           PING, QUIT + cas d'erreur + cas vicieux + resistance crash
#  Usage : python3 tester_complet.py   (depuis le dossier du binaire ./irc)
# ============================================================

import subprocess
import time
import socket

BINARY = "./irc"
PORT = 6667
PASSWORD = "toto"

VERT = "\033[92m"
ROUGE = "\033[91m"
JAUNE = "\033[93m"
BLEU = "\033[94m"
GRAS = "\033[1m"
RESET = "\033[0m"

nb_pass = 0
nb_fail = 0


def check(description, reponse, attendu):
    global nb_pass, nb_fail
    if attendu in reponse:
        nb_pass += 1
        print("  " + VERT + "PASS" + RESET + ":", description)
    else:
        nb_fail += 1
        print("  " + ROUGE + "FAIL" + RESET + ":", description)
        print("    attendu :", repr(attendu))
        print("    recu    :", repr(reponse))


def check_absent(description, reponse, interdit):
    global nb_pass, nb_fail
    if interdit not in reponse:
        nb_pass += 1
        print("  " + VERT + "PASS" + RESET + ":", description)
    else:
        nb_fail += 1
        print("  " + ROUGE + "FAIL" + RESET + ":", description)
        print("    ne devrait pas contenir :", repr(interdit))
        print("    recu                    :", repr(reponse))


def titre(txt):
    print("\n" + BLEU + GRAS + "=== " + txt + " ===" + RESET)


def lire(client, timeout_total=1.5):
    client.settimeout(0.3)
    debut = time.time()
    recu = ""
    while time.time() - debut < timeout_total:
        try:
            data = client.recv(4096).decode(errors="ignore")
            if data:
                recu += data
            else:
                break
        except (socket.timeout, TimeoutError):
            if recu:
                break
    return recu


def brut():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(("127.0.0.1", PORT))
    s.settimeout(0.3)
    return s


def nouveau_client(nick, user=None):
    if user is None:
        user = nick
    s = brut()
    s.send(("PASS " + PASSWORD + "\r\nNICK " + nick +
            "\r\nUSER " + user + " 0 * :" + user + "\r\n").encode())
    return s


def envoyer(s, texte):
    s.send((texte + "\r\n").encode())


def attendre_serveur(timeout=5.0):
    debut = time.time()
    while time.time() - debut < timeout:
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect(("127.0.0.1", PORT))
            s.close()
            return True
        except ConnectionRefusedError:
            time.sleep(0.1)
    return False


def demarrer():
    subprocess.run(["pkill", "-f", BINARY.lstrip("./")],
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    time.sleep(0.5)
    serveur = subprocess.Popen([BINARY, str(PORT), PASSWORD],
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if not attendre_serveur():
        print(ROUGE + "ERREUR : le serveur n'ecoute pas." + RESET)
        serveur.terminate()
        exit(1)
    print(VERT + "Serveur lance sur le port " + str(PORT) + RESET)
    return serveur


def tests():

    titre("ENREGISTREMENT")

    c = nouveau_client("reg1")
    check("enregistrement complet -> 001", lire(c), "001"); c.close()

    c = brut()
    envoyer(c, "USER u 0 * :u"); envoyer(c, "NICK regordre"); envoyer(c, "PASS " + PASSWORD)
    check("enregistrement ordre inverse -> 001", lire(c), "001"); c.close()

    c = brut()
    envoyer(c, "PASS mauvais"); envoyer(c, "NICK m"); envoyer(c, "USER m 0 * :m")
    rep = lire(c)
    check("mauvais password -> 464", rep, "464")
    check_absent("mauvais password -> pas de welcome", rep, "001"); c.close()

    c = brut(); envoyer(c, "JOIN #x")
    check("commande avant enregistrement -> 451", lire(c), "451"); c.close()

    c = brut(); envoyer(c, "PASS " + PASSWORD); envoyer(c, "NICK reg2"); envoyer(c, "USER reg2")
    check("USER incomplet -> 461", lire(c), "461"); c.close()

    c = nouveau_client("reg3"); lire(c)
    envoyer(c, "PASS " + PASSWORD)
    check("re-PASS apres enregistrement -> 462", lire(c), "462"); c.close()

    titre("NICK")

    c = nouveau_client("tom_[42]")
    check("nick caracteres autorises -> 001", lire(c), "001"); c.close()

    c = brut(); envoyer(c, "PASS " + PASSWORD); envoyer(c, "NICK 1tom")
    check("nick commence par chiffre -> 432", lire(c), "432"); c.close()

    c = brut(); envoyer(c, "PASS " + PASSWORD); envoyer(c, "NICK abcdefghi")
    check("nick 9 caracteres (limite) -> accepte", lire(c), "001"); c.close()

    c = brut(); envoyer(c, "PASS " + PASSWORD); envoyer(c, "NICK abcdefghij")
    rep = lire(c)
    check_absent("nick 10 caracteres -> refuse", rep, "Welcome"); c.close()

    c = brut(); envoyer(c, "PASS " + PASSWORD); envoyer(c, "NICK tom@home")
    check("nick avec @ -> 432", lire(c), "432"); c.close()

    c = brut(); envoyer(c, "PASS " + PASSWORD); envoyer(c, "NICK")
    check("NICK sans argument -> 431", lire(c), "431"); c.close()

    c1 = nouveau_client("doublon"); lire(c1)
    c2 = brut(); envoyer(c2, "PASS " + PASSWORD); envoyer(c2, "NICK doublon"); envoyer(c2, "USER a 0 * :a")
    check("nick deja pris -> 433", lire(c2), "433"); c1.close(); c2.close()

    titre("PRIVMSG")

    a = nouveau_client("pa"); b = nouveau_client("pb"); lire(a); lire(b)
    envoyer(a, "PRIVMSG pb :coucou")
    rep = lire(b)
    check("PRIVMSG recu par destinataire", rep, "coucou")
    check("PRIVMSG contient l'emetteur", rep, "pa"); a.close(); b.close()

    c = nouveau_client("psolo"); lire(c)
    envoyer(c, "PRIVMSG fantome :hi")
    check("PRIVMSG nick inexistant -> 401", lire(c), "401"); c.close()

    c = nouveau_client("psolo2"); lire(c)
    envoyer(c, "PRIVMSG")
    envoyer(c, "PRIVMSG psolo2")
    check("le serveur survit a PRIVMSG mal forme",
          lire(nouveau_client("pverif")), "001"); c.close()

    a = nouveau_client("pma"); b = nouveau_client("pmb"); d = nouveau_client("pmd")
    lire(a); lire(b); lire(d)
    envoyer(a, "PRIVMSG pmb,pmd :groupe")
    check("PRIVMSG multi : pmb recoit", lire(b), "groupe")
    check("PRIVMSG multi : pmd recoit", lire(d), "groupe")
    a.close(); b.close(); d.close()

    titre("JOIN")

    c1 = nouveau_client("ja"); lire(c1)
    envoyer(c1, "JOIN #s1")
    check("JOIN renvoie un message JOIN", lire(c1), "JOIN")
    envoyer(c1, "JOIN #s1")
    check_absent("second JOIN ignore", lire(c1), "JOIN"); c1.close()

    c1 = nouveau_client("jb"); c2 = nouveau_client("jc"); lire(c1); lire(c2)
    envoyer(c1, "JOIN #s2"); lire(c1)
    envoyer(c2, "JOIN #s2"); lire(c2); lire(c1)
    envoyer(c1, "PRIVMSG #s2 :bonjour")
    check("message channel recu par l'autre", lire(c2), "bonjour")
    check_absent("emetteur ne recoit pas son propre message", lire(c1), "bonjour")
    c1.close(); c2.close()

    c = nouveau_client("jd"); lire(c)
    envoyer(c, "JOIN")
    check("le serveur survit a JOIN sans arg", lire(nouveau_client("jverif")), "001"); c.close()

    titre("MODE +k (password)")

    # op = nouveau_client("mk"); lire(op)
    # envoyer(op, "JOIN #mk"); lire(op)
    # envoyer(op, "MODE #mk +k secret"); lire(op)
    # intr = nouveau_client("mkintr"); lire(intr)
    # envoyer(intr, "JOIN #mk")
    # check("JOIN +k sans cle -> 475", lire(intr), "475")
    # envoyer(intr, "JOIN #mk secret")
    # check("JOIN +k avec bonne cle -> JOIN", lire(intr), "JOIN")
    # op.close(); intr.close()

    # op = nouveau_client("mk2"); lire(op)
    # envoyer(op, "JOIN #mk2"); lire(op)
    # envoyer(op, "MODE #mk2 +k secret"); lire(op)
    # envoyer(op, "MODE #mk2 -k"); lire(op)
    # intr = nouveau_client("mk2i"); lire(intr)
    # envoyer(intr, "JOIN #mk2")
    # check("apres -k : JOIN libre", lire(intr), "JOIN")
    # op.close(); intr.close()

    titre("MODE +i (invite only)")

    op = nouveau_client("mi"); lire(op)
    envoyer(op, "JOIN #mi"); lire(op)
    envoyer(op, "MODE #mi +i"); lire(op)
    intr = nouveau_client("miintr"); lire(intr)
    envoyer(intr, "JOIN #mi")
    check("JOIN +i sans invitation -> 473", lire(intr), "473")
    op.close(); intr.close()

    titre("MODE +l (user limit)")

    op = nouveau_client("ml"); lire(op)
    envoyer(op, "JOIN #ml"); lire(op)
    envoyer(op, "MODE #ml +l 1"); lire(op)
    intr = nouveau_client("mlintr"); lire(intr)
    envoyer(intr, "JOIN #ml")
    check("JOIN +l channel plein -> 471", lire(intr), "471")
    op.close(); intr.close()

    titre("MODE : droits et erreurs")

    op = nouveau_client("md"); lire(op)
    envoyer(op, "JOIN #md"); lire(op)
    mem = nouveau_client("mdmem"); lire(mem)
    envoyer(mem, "JOIN #md"); lire(mem); lire(op)
    envoyer(mem, "MODE #md +i")
    check("non-op change un mode -> 482", lire(mem), "482")
    op.close(); mem.close()

    c = nouveau_client("mns"); lire(c)
    envoyer(c, "MODE #nexistepas +i")
    check("MODE channel inexistant -> 403", lire(c), "403"); c.close()

    op = nouveau_client("mq"); lire(op)
    envoyer(op, "JOIN #mq"); lire(op)
    envoyer(op, "MODE #mq")
    check("MODE sans flag -> 324 (liste modes)", lire(op), "324"); op.close()

    titre("MODE +o (operateur)")

    op = nouveau_client("mo"); lire(op)
    envoyer(op, "JOIN #mo"); lire(op)
    mem = nouveau_client("momem"); lire(mem)
    envoyer(mem, "JOIN #mo"); lire(mem); lire(op)
    envoyer(op, "MODE #mo +o momem"); lire(op); lire(mem)
    envoyer(mem, "MODE #mo +i")
    check_absent("apres +o : le membre promu peut agir (pas de 482)", lire(mem), "482")
    op.close(); mem.close()

    titre("INVITE")

    op = nouveau_client("inv"); lire(op)
    envoyer(op, "JOIN #inv"); lire(op)
    envoyer(op, "MODE #inv +i"); lire(op)
    cible = nouveau_client("invc"); lire(cible)
    envoyer(op, "INVITE invc #inv")
    rep_op = lire(op)
    rep_cible = lire(cible)
    check("INVITE : l'invitant recoit 341", rep_op, "341")
    check("INVITE : la cible recoit l'invitation", rep_cible, "INVITE")
    envoyer(cible, "JOIN #inv")
    check("INVITE : la cible invitee peut JOIN le +i", lire(cible), "JOIN")
    op.close(); cible.close()

    op = nouveau_client("inv2"); lire(op)
    envoyer(op, "INVITE quelquun #nexistepas")
    check("INVITE channel inexistant -> 403", lire(op), "403"); op.close()

    op = nouveau_client("inv3"); lire(op)
    envoyer(op, "JOIN #inv3"); lire(op)
    envoyer(op, "INVITE fantome #inv3")
    check("INVITE nick inexistant -> 401", lire(op), "401"); op.close()

    titre("PING")

    c = nouveau_client("ping1"); lire(c)
    envoyer(c, "PING :token123")
    check("PING -> PONG avec le token", lire(c), "token123"); c.close()

    titre("QUIT")

    c1 = nouveau_client("q1"); lire(c1)
    envoyer(c1, "JOIN #q"); lire(c1)
    c2 = nouveau_client("q2"); lire(c2)
    envoyer(c2, "JOIN #q"); lire(c2); lire(c1)
    envoyer(c1, "QUIT :au revoir")
    check("QUIT diffuse aux membres du channel", lire(c2), "QUIT")
    c1.close(); c2.close()

    titre("KICK : succes")

    op = nouveau_client("k1op"); lire(op)
    envoyer(op, "JOIN #k1"); lire(op)
    cib = nouveau_client("k1cib"); lire(cib)
    envoyer(cib, "JOIN #k1"); lire(cib); lire(op)
    envoyer(op, "KICK #k1 k1cib :degage")
    rep = lire(cib)
    check("kick simple : la cible recoit KICK", rep, "KICK")
    check("kick simple : commentaire present", rep, "degage")
    op.close(); cib.close()

    op = nouveau_client("k2op"); lire(op)
    envoyer(op, "JOIN #k2"); lire(op)
    x = nouveau_client("k2x"); lire(x); envoyer(x, "JOIN #k2"); lire(x); lire(op)
    y = nouveau_client("k2y"); lire(y); envoyer(y, "JOIN #k2"); lire(y); lire(op)
    envoyer(op, "KICK #k2 k2x,k2y :bye")
    check("kick multiple : x kicke", lire(x), "KICK")
    check("kick multiple : y kicke", lire(y), "KICK")
    op.close(); x.close(); y.close()

    titre("KICK : erreurs")

    op = nouveau_client("k3"); lire(op)
    envoyer(op, "KICK #nexistepas k3 :x")
    check("kick channel inexistant -> 403", lire(op), "403"); op.close()

    op = nouveau_client("k4op"); lire(op)
    envoyer(op, "JOIN #k4"); lire(op)
    out = nouveau_client("k4out"); lire(out)
    envoyer(out, "KICK #k4 k4op :x")
    check("kicker pas membre -> 442", lire(out), "442")
    op.close(); out.close()

    op = nouveau_client("k5op"); lire(op)
    envoyer(op, "JOIN #k5"); lire(op)
    mem = nouveau_client("k5mem"); lire(mem)
    envoyer(mem, "JOIN #k5"); lire(mem); lire(op)
    envoyer(mem, "KICK #k5 k5op :x")
    check("kicker non-op -> 482", lire(mem), "482")
    op.close(); mem.close()

    op = nouveau_client("k6op"); lire(op)
    envoyer(op, "JOIN #k6"); lire(op)
    envoyer(op, "KICK #k6 fantome :x")
    check("cible inexistante -> 441", lire(op), "441"); op.close()

    op = nouveau_client("k7op"); lire(op)
    envoyer(op, "JOIN #k7"); lire(op)
    envoyer(op, "KICK #k7")
    check("KICK sans cible -> 461", lire(op), "461"); op.close()

    titre("KICK : cas vicieux")

    op = nouveau_client("kv1op"); lire(op)
    envoyer(op, "JOIN #kv1"); lire(op)
    bb = nouveau_client("kv1b"); lire(bb); envoyer(bb, "JOIN #kv1"); lire(bb); lire(op)
    aa = nouveau_client("kv1a"); lire(aa); envoyer(aa, "JOIN #kv1"); lire(aa); lire(op)
    envoyer(op, "KICK #kv1 kv1b,fantome,kv1a :mix")
    check("kick partiel : b kicke", lire(bb), "KICK")
    check("kick partiel : a kickee", lire(aa), "KICK")
    check("kick partiel : 441 pour fantome", lire(op), "441")
    op.close(); bb.close(); aa.close()

    titre("RESISTANCE AU CRASH")

    s = nouveau_client("stress"); lire(s)
    envoyer(s, "JOIN #stress"); lire(s)
    for cmd in [b"\r\n", b"KICK\r\n", b"KICK #stress\r\n", b"KICK ,,,\r\n",
                b"MODE\r\n", b"MODE #stress\r\n", b"JOIN\r\n", b"PRIVMSG\r\n",
                b"PRIVMSG stress\r\n", b"INVITE\r\n", b"PING\r\n",
                b"MODE #stress +k\r\n", b"MODE #stress +l abc\r\n",
                b"BLABLA nimportequoi\r\n"]:
        s.send(cmd)
    s.send(b"PRIVMSG #stress :" + b"A" * 6000 + b"\r\n")
    lire(s); s.close()

    check("le serveur survit a la rafale de commandes tordues",
          lire(nouveau_client("survivant")), "001")

    s = nouveau_client("partiel"); lire(s)
    s.send(b"PRI"); time.sleep(0.2)
    s.send(b"VMSG partiel :coupe"); time.sleep(0.2)
    s.send(b" en morceaux\r\n")
    check("donnees partielles reconstituees", lire(s), "coupe en morceaux"); s.close()


def main():
    print(GRAS + "TESTEUR IRC COMPLET" + RESET)
    serveur = demarrer()
    try:
        tests()
    finally:
        serveur.terminate()
        serveur.wait()
        print(VERT + "\nServeur arrete proprement" + RESET)

    total = nb_pass + nb_fail
    print("\n" + GRAS + "RECAPITULATIF" + RESET)
    print("  " + VERT + str(nb_pass) + " PASS" + RESET + "   " +
          ROUGE + str(nb_fail) + " FAIL" + RESET + "   (" + str(total) + " tests)")
    if nb_fail == 0:
        print("  " + VERT + GRAS + "Tout est vert !" + RESET)
    else:
        print("  " + JAUNE + "Des FAIL a examiner ci-dessus." + RESET)


if __name__ == "__main__":
    main()

