---
name: Bug report
about: Report a reproducible bug in ircserv
title: "fix(<scope>): <short description>"
labels: bug
assignees: ""
---

## Description
<!-- A clear and concise description of the bug -->


## Steps to reproduce
<!-- Exact commands or sequence of actions that trigger the bug -->

1. Start the server: `./ircserv <port> <password>`
2. Connect with: `nc -C 127.0.0.1 <port>` or your IRC client
3. Send the following commands:
```
PASS <password>
NICK ...
USER ...
```
4. Observe the error

## Expected behavior
<!-- What should have happened -->


## Actual behavior
<!-- What actually happened — paste the server output or client error -->

```
paste output here
```

## Environment
- OS: <!-- e.g. Ubuntu 22.04 / macOS 14 -->
- Compiler: <!-- e.g. g++ 11.4 / clang++ 14 -->
- IRC client used: <!-- e.g. irssi 1.4 / WeeChat 3.8 / nc -->
- Commit hash: <!-- git log --oneline -1 -->

## Reproducibility
- [ ] Always
- [ ] Sometimes
- [ ] Only once

## Additional context
<!-- Valgrind output, partial data test, screenshots, etc. -->
