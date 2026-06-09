---
name: Feature request
about: Suggest a new feature or IRC command to implement
title: "feat(<scope>): <short description>"
labels: enhancement
assignees: ""
---

## Summary
<!-- One sentence describing what you want to add -->


## Motivation
<!-- Why is this needed? Is it required by the subject, the RFC, or the bonus? -->

- [ ] Mandatory (required by the subject)
- [ ] Bonus (file transfer / bot)
- [ ] Quality of life / code improvement

## Related RFC or subject section
<!-- Link or quote the relevant part of RFC 1459 / 2812 or the subject PDF -->

> paste the relevant excerpt here

## Proposed behavior
<!-- Describe exactly how it should work from the client's perspective -->

**Command syntax:**
```
COMMAND <param1> [<param2>]
```

**Expected server responses:**
```
:<server> <numeric> <target> :<message>
```

**Edge cases to handle:**
- [ ] Missing parameters → `ERR_NEEDMOREPARAMS (461)`
- [ ] User not in channel → `ERR_NOTONCHANNEL (442)`
- [ ] Insufficient privileges → `ERR_CHANOPRIVSNEEDED (482)`
- [ ] Other: <!-- describe -->

## Acceptance criteria
<!-- How do we know this feature is done and working? -->

- [ ] Compiles with `-Wall -Wextra -Werror -std=c++98`
- [ ] Works with reference IRC client
- [ ] Tested with `nc` partial data (ctrl+D test from subject)
- [ ] No memory leaks (Valgrind clean)
- [ ] Code reviewed and approved

## Additional context
<!-- Anything else: related issues, implementation ideas, etc. -->
