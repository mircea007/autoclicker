# Clickr

![Clickr logo](https://github.com/mircea007/clickr/blob/main/img/logo.png?raw=true)

[![Linux](https://svgshare.com/i/Zhy.svg)](https://svgshare.com/i/Zhy.svg) [![GitHub version](https://badge.fury.io/gh/mircea007%2Fclickr.svg)](https://github.com/mircea007/clickr) [![GitHub stars](https://img.shields.io/github/stars/mircea007/clickr.svg?style=social&label=Star&maxAge=2592000)](https://github.com/mircea007/clickr/stargazers) [![GitHub watchers](https://img.shields.io/github/watchers/mircea007/clickr.svg?style=social&label=Watch&maxAge=2592000)](https://github.com/mircea007/clickr/watchers/)

A linux x11 (for now) autoclicker.

**Comming soon**: [eficient cps test](http://mircea.rebengiuc.com/cpstest)

## Features

* Toggle key is Caps Lock (because who really uses it?). Caps Lock is also closer to WASD keys so it's easy to reach
* Anti-bot detection (by randomizing intervals between clicks, in the future i will add buterfly/dracgclick mode for even better stealth)
* Very small overhead
* Works for both right and left click

## How to run

Compile:
```bash
make
```

Usage:
```bash
sudo bin/clickr [options] [--help] [--cps CPS]

```
options:

* `-h`, `--help`: print help page & exit
* `-w`: show warnings
* `-i`: show info mesages
* `-d`: show debug mesages

## How it works

there are 5 threads running:

* main thread (initializes stuff, then waits to be interupted by `SIGINT` (`Ctrl`+`C`), and then cleans up)
* autoclicker for the left click   (implemented by `AsyncAutoClicker`)
* autoclicker for the right click  (implemented by `AsyncAutoClicker`)
* listens for real mouse clicks (needs access to `/dev/input/mice`) (worker thread in `MimicMouseButFaster`)
* listens for CapsLock to toggle autoclicking (listen thread in `MimicMouseButFaster`)

## TO DO

* **add event loop in windows version**
* add user configuration
* get rid of sudo requirement in linux version (would also merge `listen` and `worker` threads in `MimicMouseButFaster`)
* user friendly (maybe system tray icon or gui application)
