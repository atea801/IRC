#!/usr/bin/env python3
# ============================================================
#  TESTEUR IRC COMPLET
#  Couvre : enregistrement, NICK, USER, PASS, PRIVMSG, JOIN, PART,
#           MODE, KICK, INVITE, PING, QUIT + erreurs + cas vicieux
#           + resistance crash et donnees partielles
#  Usage : python3 tester.py   (depuis le dossier du binaire ./irc)
# ============================================================

import socket
import subprocess
import time

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


def titre(txt):
    print("\n" + BLEU + GRAS + "=== " + txt + " ===" + RESET)


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


def envoyer(s, texte):
    s.send((texte + "\r\n").encode())


def nouveau_client(nick, user=None):
    if user is None:
        user = nick
    s = brut()
    envoyer(s, "PASS " + PASSWORD)
    envoyer(s, "NICK " + nick)
    envoyer(s, "USER " + user + " 0 * :" + user)
    return s


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
        raise SystemExit(1)
    print(VERT + "Serveur lance sur le port " + str(PORT) + RESET)
    return serveur


def login_and_flush(nick, user=None):
    client = nouveau_client(nick, user)
    lire(client)
    return client


def join_and_flush(client, channel, key=None):
    if key is None:
        envoyer(client, "JOIN " + channel)
    else:
        envoyer(client, "JOIN " + channel + " " + key)
    return lire(client)


def tests():
    titre("ENREGISTREMENT")

    c = nouveau_client("reg1")
    check("enregistrement complet -> 001", lire(c), "001")
    c.close()

    c = brut()
    envoyer(c, "USER u 0 * :u")
    envoyer(c, "NICK regordre")
    envoyer(c, "PASS " + PASSWORD)
    check("enregistrement ordre inverse -> 001", lire(c), "001")
    c.close()

    c = brut()
    envoyer(c, "PASS mauvais")
    envoyer(c, "NICK m")
    envoyer(c, "USER m 0 * :m")
    rep = lire(c)
    check("mauvais password -> 464", rep, "464")
    check_absent("mauvais password -> pas de welcome", rep, "001")
    c.close()

    c = brut()
    envoyer(c, "JOIN #x")
    check("commande avant enregistrement -> 451", lire(c), "451")
    c.close()

    c = brut()
    envoyer(c, "PASS " + PASSWORD)
    envoyer(c, "NICK reg2")
    envoyer(c, "USER reg2")
    check("USER incomplet -> 461", lire(c), "461")
    c.close()

    c = login_and_flush("reg3")
    envoyer(c, "PASS " + PASSWORD)
    check("re-PASS apres enregistrement -> 462", lire(c), "462")
    c.close()

    c = brut()
    envoyer(c, "BLABLA")
    check("commande inconnue avant enregistrement -> 451", lire(c), "451")
    c.close()

    titre("NICK")

    c = nouveau_client("tom_[42]")
    check("nick caracteres autorises -> 001", lire(c), "001")
    c.close()

    c = brut()
    envoyer(c, "PASS " + PASSWORD)
    envoyer(c, "NICK 1tom")
    check("nick commence par chiffre -> 432", lire(c), "432")
    c.close()

    c = brut()
    envoyer(c, "PASS " + PASSWORD)
    envoyer(c, "NICK abcdefghi")
    envoyer(c, "USER abcdefghi 0 * :abcdefghi")
    check("nick 9 caracteres (limite) -> accepte", lire(c), "001")
    c.close()

    c = brut()
    envoyer(c, "PASS " + PASSWORD)
    envoyer(c, "NICK abcdefghij")
    rep = lire(c)
    check_absent("nick 10 caracteres -> refuse", rep, "001")
    c.close()

    c = brut()
    envoyer(c, "PASS " + PASSWORD)
    envoyer(c, "NICK tom@home")
    check("nick avec @ -> 432", lire(c), "432")
    c.close()

    c = brut()
    envoyer(c, "PASS " + PASSWORD)
    envoyer(c, "NICK")
    check("NICK sans argument -> 431", lire(c), "431")
    c.close()

    c1 = login_and_flush("doublon")
    c2 = brut()
    envoyer(c2, "PASS " + PASSWORD)
    envoyer(c2, "NICK doublon")
    envoyer(c2, "USER a 0 * :a")
    check("nick deja pris -> 433", lire(c2), "433")
    c1.close()
    c2.close()

    c = login_and_flush("nickchange")
    envoyer(c, "NICK nickchange2")
    check_absent("changement de nick ne casse pas la session", lire(c), "431")
    envoyer(c, "PRIVMSG personne :test")
    check("apres changement de nick, la session reste vivante", lire(c), "401")
    c.close()

    titre("JOIN")

    c1 = login_and_flush("ja")
    envoyer(c1, "JOIN #s1")
    check("JOIN renvoie un message JOIN", lire(c1), "JOIN")
    envoyer(c1, "JOIN #s1")
    check_absent("second JOIN ignore", lire(c1), "JOIN")
    c1.close()

    c1 = login_and_flush("jb")
    c2 = login_and_flush("jc")
    envoyer(c1, "JOIN #s2")
    lire(c1)
    envoyer(c2, "JOIN #s2")
    lire(c2)
    lire(c1)
    envoyer(c1, "PRIVMSG #s2 :bonjour")
    check("message channel recu par l'autre", lire(c2), "bonjour")
    check_absent("emetteur ne recoit pas son propre message", lire(c1), "bonjour")
    c1.close()
    c2.close()

    c = login_and_flush("jd")
    envoyer(c, "JOIN")
    check("JOIN sans arg -> 461", lire(c), "461")
    verifier = brut()
    envoyer(verifier, "PASS " + PASSWORD)
    envoyer(verifier, "NICK jverif")
    envoyer(verifier, "USER jverif 0 * :jverif")
    check("le serveur survit a JOIN sans arg", lire(verifier), "001")
    verifier.close()
    c.close()

    c = login_and_flush("jbad")
    envoyer(c, "JOIN chan")
    check("JOIN sans prefix channel -> 476", lire(c), "476")
    c.close()

    c1 = login_and_flush("jm1")
    c2 = login_and_flush("jm2")
    envoyer(c1, "JOIN #jm1,#jm2")
    rep = lire(c1)
    check("JOIN multi channel envoie JOIN", rep, "JOIN")
    c1.close()
    c2.close()

    titre("PART")

    p1 = login_and_flush("pa")
    p2 = login_and_flush("pb")
    p3 = login_and_flush("pc")
    envoyer(p1, "JOIN #part")
    lire(p1)
    envoyer(p2, "JOIN #part")
    lire(p2)
    envoyer(p3, "JOIN #part")
    lire(p3)
    lire(p1)
    envoyer(p1, "PART #part :bye")
    rep_p2 = lire(p2)
    rep_p3 = lire(p3)
    check("PART diffuse la sortie aux membres", rep_p2, "PART")
    check("PART transmet le message optionnel", rep_p2, "bye")
    envoyer(p2, "PRIVMSG #part :afterpart")
    check("PART laisse les autres membres dans le channel", lire(p3), "afterpart")
    check_absent("PART retire l'emetteur du channel", lire(p1), "afterpart")
    p1.close()
    p2.close()
    p3.close()

    p = login_and_flush("pnone")
    envoyer(p, "PART")
    check("PART sans arg -> 461", lire(p), "461")
    p.close()

    p = login_and_flush("pbad")
    envoyer(p, "PART #inexistant")
    check("PART channel inexistant -> 403", lire(p), "403")
    p.close()

    p = login_and_flush("pout")
    envoyer(p, "JOIN #pout")
    lire(p)
    other = login_and_flush("pout2")
    envoyer(other, "PART #pout")
    check("PART non membre -> 442", lire(other), "442")
    p.close()
    other.close()

    p = login_and_flush("pmulti")
    envoyer(p, "JOIN #pm1")
    lire(p)
    envoyer(p, "JOIN #pm2")
    lire(p)
    envoyer(p, "PART #pm1,#pm2 :multi")
    rep = lire(p)
    check("PART multi channel envoie des PART", rep, "PART")
    p.close()

    titre("PRIVMSG")

    a = login_and_flush("pa1")
    b = login_and_flush("pb1")
    envoyer(a, "PRIVMSG pb1 :coucou")
    rep = lire(b)
    check("PRIVMSG recu par destinataire", rep, "coucou")
    check("PRIVMSG contient l'emetteur", rep, "pa1")
    a.close()
    b.close()

    c = login_and_flush("psolo")
    envoyer(c, "PRIVMSG fantome :hi")
    check("PRIVMSG nick inexistant -> 401", lire(c), "401")
    c.close()

    c = login_and_flush("psolo2")
    envoyer(c, "PRIVMSG")
    check("PRIVMSG sans recipient -> 411", lire(c), "411")
    envoyer(c, "PRIVMSG psolo2")
    check("PRIVMSG sans texte -> 412", lire(c), "412")
    c.close()

    a = login_and_flush("pma")
    b = login_and_flush("pmb")
    d = login_and_flush("pmd")
    envoyer(a, "PRIVMSG pmb,pmd :groupe")
    check("PRIVMSG multi : pmb recoit", lire(b), "groupe")
    check("PRIVMSG multi : pmd recoit", lire(d), "groupe")
    a.close()
    b.close()
    d.close()

    ch1 = login_and_flush("pmch1")
    ch2 = login_and_flush("pmch2")
    envoyer(ch1, "JOIN #pmch")
    lire(ch1)
    envoyer(ch2, "JOIN #pmch")
    lire(ch2)
    lire(ch1)
    envoyer(ch1, "PRIVMSG #pmch :hello channel")
    check("PRIVMSG channel recu par l'autre membre", lire(ch2), "hello channel")
    check_absent("PRIVMSG channel n'est pas renvoye a l'emetteur", lire(ch1), "hello channel")
    ch1.close()
    ch2.close()

    # titre("MODE +k / +i / +l / +o")

    # op = login_and_flush("mk")
    # join_and_flush(op, "#mk")
    # other = login_and_flush("mkintr")
    # envoyer(op, "MODE #mk +k secret")
    # lire(op)
    # envoyer(other, "JOIN #mk")
    # check("JOIN +k sans cle -> 475", lire(other), "475")
    # envoyer(other, "JOIN #mk secret")
    # check("JOIN +k avec bonne cle -> JOIN", lire(other), "JOIN")
    # op.close()
    # other.close()

    # op = login_and_flush("mk2")
    # join_and_flush(op, "#mk2")
    # envoyer(op, "MODE #mk2 +k secret")
    # lire(op)
    # envoyer(op, "MODE #mk2 -k")
    # lire(op)
    # intr = login_and_flush("mk2i")
    # envoyer(intr, "JOIN #mk2")
    # check("apres -k : JOIN libre", lire(intr), "JOIN")
    # op.close()
    # intr.close()

    # op = login_and_flush("mi")
    # join_and_flush(op, "#mi")
    # envoyer(op, "MODE #mi +i")
    # intr = login_and_flush("miintr")
    # envoyer(intr, "JOIN #mi")
    # check("JOIN +i sans invitation -> 473", lire(intr), "473")
    # op.close()
    # intr.close()

    # op = login_and_flush("ml")
    # join_and_flush(op, "#ml")
    # envoyer(op, "MODE #ml +l 1")
    # intr = login_and_flush("mlintr")
    # envoyer(intr, "JOIN #ml")
    # check("JOIN +l channel plein -> 471", lire(intr), "471")
    # op.close()
    # intr.close()

    # op = login_and_flush("md")
    # join_and_flush(op, "#md")
    # mem = login_and_flush("mdmem")
    # join_and_flush(mem, "#md")
    # envoyer(mem, "MODE #md +i")
    # check("non-op change un mode -> 482", lire(mem), "482")
    # op.close()
    # mem.close()

    # c = login_and_flush("mns")
    # envoyer(c, "MODE #nexistepas +i")
    # check("MODE channel inexistant -> 403", lire(c), "403")
    # c.close()

    # op = login_and_flush("mq")
    # join_and_flush(op, "#mq")
    # envoyer(op, "MODE #mq")
    # check("MODE sans flag -> 324 (liste modes)", lire(op), "324")
    # op.close()

    # op = login_and_flush("mo")
    # join_and_flush(op, "#mo")
    # mem = login_and_flush("momem")
    # join_and_flush(mem, "#mo")
    # envoyer(op, "MODE #mo +o momem")
    # lire(op)
    # lire(mem)
    # envoyer(mem, "MODE #mo +i")
    # check_absent("apres +o : le membre promu peut agir (pas de 482)", lire(mem), "482")
    # op.close()
    # mem.close()

    # op = login_and_flush("moerr")
    # join_and_flush(op, "#moerr")
    # envoyer(op, "MODE #moerr +o fantome")
    # check("MODE +o sur membre absent -> 441", lire(op), "441")
    # op.close()

    titre("INVITE")

    op = login_and_flush("inv")
    join_and_flush(op, "#inv")
    envoyer(op, "MODE #inv +i")
    cible = login_and_flush("invc")
    envoyer(op, "INVITE invc #inv")
    rep_op = lire(op)
    rep_cible = lire(cible)
    check("INVITE : l'invitant recoit 341", rep_op, "341")
    check("INVITE : la cible recoit l'invitation", rep_cible, "INVITE")
    envoyer(cible, "JOIN #inv")
    check("INVITE : la cible invitee peut JOIN le +i", lire(cible), "JOIN")
    op.close()
    cible.close()

    op = login_and_flush("inv2")
    envoyer(op, "INVITE quelquun #nexistepas")
    check("INVITE channel inexistant -> 403", lire(op), "403")
    op.close()

    op = login_and_flush("inv3")
    join_and_flush(op, "#inv3")
    envoyer(op, "INVITE fantome #inv3")
    check("INVITE nick inexistant -> 401", lire(op), "401")
    op.close()

    op = login_and_flush("inv4")
    join_and_flush(op, "#inv4")
    cible = login_and_flush("inv4c")
    envoyer(op, "INVITE inv4c #inv4")
    check("INVITE utilisateur deja sur channel -> 443", lire(op), "443")
    op.close()
    cible.close()

    op = login_and_flush("inv5")
    join_and_flush(op, "#inv5")
    mem = login_and_flush("inv5m")
    join_and_flush(mem, "#inv5")
    envoyer(op, "MODE #inv5 +i")
    envoyer(mem, "INVITE inv5m #inv5")
    check("INVITE non-op sur +i -> 482", lire(mem), "482")
    op.close()
    mem.close()

    titre("PING")

    c = login_and_flush("ping1")
    envoyer(c, "PING :token123")
    check("PING -> PONG avec le token", lire(c), "token123")
    c.close()

    titre("QUIT")

    c1 = login_and_flush("q1")
    join_and_flush(c1, "#q")
    c2 = login_and_flush("q2")
    join_and_flush(c2, "#q")
    lire(c1)
    envoyer(c1, "QUIT :au revoir")
    check("QUIT diffuse aux membres du channel", lire(c2), "QUIT")
    c1.close()
    c2.close()

    titre("KICK : succes")

    op = login_and_flush("k1op")
    join_and_flush(op, "#k1")
    cib = login_and_flush("k1cib")
    join_and_flush(cib, "#k1")
    lire(op)
    envoyer(op, "KICK #k1 k1cib :degage")
    rep = lire(cib)
    check("kick simple : la cible recoit KICK", rep, "KICK")
    check("kick simple : commentaire present", rep, "degage")
    op.close()
    cib.close()

    op = login_and_flush("k2op")
    join_and_flush(op, "#k2")
    x = login_and_flush("k2x")
    join_and_flush(x, "#k2")
    lire(op)
    y = login_and_flush("k2y")
    join_and_flush(y, "#k2")
    lire(op)
    envoyer(op, "KICK #k2 k2x,k2y :bye")
    check("kick multiple : x kicke", lire(x), "KICK")
    check("kick multiple : y kicke", lire(y), "KICK")
    op.close()
    x.close()
    y.close()

    titre("KICK : erreurs")

    op = login_and_flush("k3")
    envoyer(op, "KICK #nexistepas k3 :x")
    check("kick channel inexistant -> 403", lire(op), "403")
    op.close()

    op = login_and_flush("k4op")
    join_and_flush(op, "#k4")
    out = login_and_flush("k4out")
    envoyer(out, "KICK #k4 k4op :x")
    check("kicker pas membre -> 442", lire(out), "442")
    op.close()
    out.close()

    op = login_and_flush("k5op")
    join_and_flush(op, "#k5")
    mem = login_and_flush("k5mem")
    join_and_flush(mem, "#k5")
    lire(op)
    envoyer(mem, "KICK #k5 k5op :x")
    check("kicker non-op -> 482", lire(mem), "482")
    op.close()
    mem.close()

    op = login_and_flush("k6op")
    join_and_flush(op, "#k6")
    envoyer(op, "KICK #k6 fantome :x")
    check("cible inexistante -> 441", lire(op), "441")
    op.close()

    op = login_and_flush("k7op")
    join_and_flush(op, "#k7")
    envoyer(op, "KICK #k7")
    check("KICK sans cible -> 461", lire(op), "461")
    op.close()

    titre("KICK : cas vicieux")

    op = login_and_flush("kv1op")
    join_and_flush(op, "#kv1")
    bb = login_and_flush("kv1b")
    join_and_flush(bb, "#kv1")
    lire(op)
    aa = login_and_flush("kv1a")
    join_and_flush(aa, "#kv1")
    lire(op)
    envoyer(op, "KICK #kv1 kv1b,fantome,kv1a :mix")
    check("kick partiel : b kicke", lire(bb), "KICK")
    check("kick partiel : a kickee", lire(aa), "KICK")
    check("kick partiel : 441 pour fantome", lire(op), "441")
    op.close()
    bb.close()
    aa.close()

    titre("AUTRES COMMANDES ET SURVIE")

    c = login_and_flush("cap1")
    envoyer(c, "CAP")
    survivor = login_and_flush("cap2")
    check("CAP ne coupe pas la session", lire(survivor), "001")
    c.close()
    survivor.close()

    c = login_and_flush("unk1")
    envoyer(c, "BLABLA")
    check("commande inconnue -> 421", lire(c), "421")
    c.close()

    c = login_and_flush("longmsg")
    envoyer(c, "PRIVMSG longmsg :" + ("A" * 2000))
    lire(c)  # Flush le message recu par longmsg lui-meme
    survivor = login_and_flush("longverif")
    check("PRIVMSG long ne casse pas le serveur", lire(survivor), "001")
    c.close()
    survivor.close()

    titre("DCC / CTCP")

    # Le vrai octet SOH (0x01) qui délimite un message CTCP/DCC.
    # En Python, "\x01" EST l'octet réel, contrairement à la chaîne
    # littérale \x01 qu'on obtiendrait en tapant au clavier dans un terminal.
    CTCP = "\x01"

    a = login_and_flush("dcc_a")
    b = login_and_flush("dcc_b")

    payload = CTCP + "DCC SEND rapport.pdf 3232235521 5000 245678" + CTCP
    envoyer(a, "PRIVMSG dcc_b :" + payload)

    rep = lire(b)
    check("DCC : le destinataire recoit bien PRIVMSG", rep, "PRIVMSG")
    check("DCC : le contenu CTCP contient DCC SEND", rep, "DCC SEND")
    check("DCC : le nom de fichier est preserve", rep, "rapport.pdf")
    check("DCC : l'octet SOH de debut est preserve", rep, CTCP + "DCC")
    check("DCC : l'octet SOH de fin est preserve", rep, "245678" + CTCP)

    a.close()
    b.close()

    # Variante : DCC envoyé vers un channel (moins standard mais bon test de robustesse)
    c1 = login_and_flush("dcc_c1")
    c2 = login_and_flush("dcc_c2")
    join_and_flush(c1, "#dcc")
    lire(c1)
    join_and_flush(c2, "#dcc")
    lire(c1)  # flush le JOIN de c2 vu par c1

    payload2 = CTCP + "DCC SEND autre.txt 3232235521 5001 100" + CTCP
    envoyer(c1, "PRIVMSG #dcc :" + payload2)
    rep2 = lire(c2)
    check("DCC sur channel : octet SOH preserve", rep2, CTCP + "DCC")

    c1.close()
    c2.close()

    titre("RESISTANCE AU CRASH")

    s = login_and_flush("stress")
    join_and_flush(s, "#stress")
    for cmd in [
        b"\r\n",
        b"KICK\r\n",
        b"KICK #stress\r\n",
        b"KICK ,,,\r\n",
        # b"MODE\r\n",
        b"MODE #stress\r\n",
        b"JOIN\r\n",
        b"PRIVMSG\r\n",
        b"PRIVMSG stress\r\n",
        b"INVITE\r\n",
        b"PING\r\n",
        b"MODE #stress +k\r\n",
        b"MODE #stress +l abc\r\n",
        b"BLABLA nimportequoi\r\n",
    ]:
        s.send(cmd)
    s.send(b"PRIVMSG #stress :" + b"A" * 6000 + b"\r\n")
    lire(s)
    s.close()

    check("le serveur survit a la rafale de commandes tordues",
          lire(login_and_flush("survivant")), "001")

    s = login_and_flush("partiel")
    s.send(b"PRI")
    time.sleep(0.2)
    s.send(b"VMSG partiel :coupe")
    time.sleep(0.2)
    s.send(b" en morceaux\r\n")
    check("donnees partielles reconstituees", lire(s), "coupe en morceaux")
    s.close()


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

