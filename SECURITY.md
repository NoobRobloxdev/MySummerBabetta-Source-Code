# Security Policy

Thanks for helping keep **My Summer Babetta** and its players safe. The game is in ALPHA, so things change quickly, but we take security reports seriously.

## Supported Versions

Only the latest release receives security fixes. Please update to the newest version before reporting an issue.

| Version        | Supported          |
| -------------- | ------------------ |
| 0.4.8ADEV2     | :white_check_mark: |
| < 0.4.8ADEV2   | :x:                |

## Scope

Examples of issues we want to hear about:

- Vulnerabilities in the networking code (`net.cpp`), such as crashes, buffer overflows, or ways to disrupt or take over other players' sessions.
- Modloader issues that let a mod in `/build/mods` read or write files outside the mods folder or run unintended code.
- Planned features such as accounts and password login, once released (credential handling, profile tampering).
- Exploits that affect other players on a server (for example, duplicating money or forcing payouts).

Not in scope: single-player glitches that only affect your own game (for example, editing your own money), and gameplay bugs. Please report those as normal bug reports.

## Reporting a Vulnerability

**Please do not open a public GitHub issue for security problems.**

Report privately by email: **vondranoob233@gmail.com**

Use the subject line `[SECURITY] My Summer Babetta` and include, if you can:

- A description of the vulnerability and its impact
- The game version and your operating system
- Steps to reproduce, or a proof of concept
- Any suggested fix

### What to expect

- **Acknowledgement:** within **72 hours** of your report.
- **Updates:** at least once every **7 days** until the report is resolved.
- **If accepted:** we will confirm the issue, work on a fix, and tell you when a patched version is released. With your permission, we will credit you in the release notes.
- **If declined:** we will explain why (for example, out of scope or not reproducible). You are welcome to reply with more detail.

Please give us a reasonable amount of time to fix an issue before disclosing it publicly. We aim to release fixes within **90 days** of a confirmed report.

## Safe Harbor

If you act in good faith, avoid harming other players, and do not access or change data that is not yours, we will not take action against you for your research.
