# Description
<!-- What does this PR do? One or two sentences max -->


## Type of change
- [ ] `feat` — new feature or IRC command
- [ ] `fix` — bug fix
- [ ] `refactor` — code restructure, no behavior change
- [ ] `chore` — Makefile, CI, templates, config
- [ ] `docs` — README, comments, documentation
- [ ] `test` — test scripts

## Related issue
<!-- Link the issue this PR closes -->
Closes #

---

## Changes made
<!-- List the files/classes modified and what changed -->

- `src/` —
- `inc/` —
- Other —

## IRC commands affected
<!-- Check all commands impacted by this PR -->
- [ ] PASS / NICK / USER (registration)
- [ ] JOIN
- [ ] PRIVMSG / NOTICE
- [ ] KICK
- [ ] INVITE
- [ ] TOPIC
- [ ] MODE (`+i` `+t` `+k` `+o` `+l`)
- [ ] QUIT
- [ ] None

---

## Checklist before requesting review

**Compilation**
- [ ] Compiles with `make` — no errors, no warnings
- [ ] Flags respected : `-Wall -Wextra -Werror -std=c++98`
- [ ] No external library used

**Functionality**
- [ ] Tested with reference IRC client
- [ ] Tested with `nc` partial data (ctrl+D test from subject)
- [ ] Multiple simultaneous clients tested
- [ ] No blocking I/O — `poll()` used correctly

**Code quality**
- [ ] No memory leaks (`valgrind --leak-check=full ./ircserv`)
- [ ] No unnecessary `fork()`
- [ ] `fcntl()` used only as `fcntl(fd, F_SETFL, O_NONBLOCK)` (macOS)
- [ ] Clean code — no debug `std::cout`, no dead code

**Git**
- [ ] Branch is up to date with `dev`
- [ ] Commit messages follow Conventional Commits
- [ ] No unrelated changes in this PR

---

## How to test this PR
<!-- Give the exact commands a reviewer needs to run to verify your changes -->

```bash
make
./ircserv 6667 password
```

```
# Then in another terminal:
nc -C 127.0.0.1 6667
PASS password
NICK testnick
USER testuser 0 * :Test User
```
