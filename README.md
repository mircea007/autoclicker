# Currently Unnamed Autoclicker

[![Linux](https://svgshare.com/i/Zhy.svg)](https://svgshare.com/i/Zhy.svg) [![GitHub version](https://badge.fury.io/gh/mircea007%2Fautoclicker.svg)](https://github.com/mircea007/autoclicker) [![GitHub stars](https://img.shields.io/github/stars/mircea007/autoclicker.svg?style=social&label=Star&maxAge=2592000)](https://github.com/mircea007/autoclicker/) [![GitHub watchers](https://img.shields.io/github/watchers/mircea007/autoclicker.svg?style=social&label=Watch&maxAge=2592000)](https://github.com/mircea007/autoclicker/watchers/)

## Constraints

Is linux + X.org **only**.
you must use sudo or add user to `input` group (for `/dev/input/mice`)

## Why?

* sick [cps](https://cpstest.org/)
* minecraft (also btw if you get banned it's not my fault)
* it's cool (duh)

## How to run

Compile:
```bash
make
```

Usage:
```bash
bin/autoclicker [options] [--help] [--cps CPS]
```

## How it works

there are 5 threads running:

* main thread (initializes stuff, then waits to be interupted by `SIGINT` (`Ctrl`+`C`), and then cleans up)
* autoclicker for the left click   (implemented by `AsyncAutoClicker`)
* autoclicker for the right click  (implemented by `AsyncAutoClicker`)
* listens for real mouse clicks (needs access to `/dev/input/mice`) (worker thread in `MimicMouseButFaster`)
* listens for CapsLock to toggle autoclicking (listen thread in `MimicMouseButFaster`)

## TO DO

* windows compatibility
* get rid of sudo requirement (would also merge `listen` and `worker` threads in `MimicMouseButFaster`)
* user friendly (maybe system tray icon or gui application)
