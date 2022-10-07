# CanFestival-3 CANopen support tools and library

###### This repository was forked from (stagnant) Mercurial repo https://hg.beremiz.org/canfestival @ changeset de1fc3261f21

## Introduction

CanFestival-3 provides tools to support the CANopen CAN bus protocol. These include a C implementation
library/headers and (if Python 2 is available) the `objdictedit` and `obdictgen` tools for editing object
dictionaries and generate C source files from object dictionary files.

## Why this fork?
I have been using CanFestival to successfully support another project for some time. After a recent system upgrade to
an operating system which no longer provides Python 2, I discovered that CanFestival's `objdictedit` and `objdictgen`
tools have a very tight dependency on Python 2. I had hoped to find Python 3 updates for CanFestival, but instead found
that no commits at all had been made in the original repository for a long while. 

For my project which uses CanFestival, `objdictgen` and `objdictedit` are no longer needed, since I already have
the needed `.od` object dictionary file and working `.c ` and `.h` files which were generated some time ago
by `objdictgen`. All I really need is CanFestival's C library.

I have modified CanFestival to ignore the Python components if Python 2 is not the default version seen at build
time. The C library will still build and install properly, but `objdictgen` and `objdictedit` get replaced with
versions which just print a notification and exit with an error. Because they require `objdictgen`, the examples 
provided in the repository do not get built.
