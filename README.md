# MuServer

My attempt to recreate MuOnline server for as eldest version as possible. Most of code is based on so-called 0.65 version src, which were more likely to be from 0.7x period. Not for production use, more for someone wanting to tinker and modernize a bit own .97 server or just play with long-dead MMO game. ;)

## State of code

### What works

Nothing. What were you expecting? ;)

But seriously, Joinserver should be a drop-in replacement for even .97 versions and stable enough for server use, **but test it thoroughly**. There are some tweaks configurable in .ini file, e.g. automatically creating accounts after logging two-times into non-existent one with same credentials. Something that I really liked in UO private server scene. Also, whole thing works under MySQL db, so that's one step towards dropping MSSQL2000 dependency.

### What doesn't work

Dataserver is quite complete, but probably not fitted to talk to your gameserver. Also, if you fit it to work with your GS, you probably have most of server running under MySQL, so that's something to fight for.

### Stuff worth mentioning

At the time of code writing I didn't really understood critical section and mutex stuff, so feel free to inspect code and add them where necessary.

I've tried to work without changing a single protocol packet, but I'm not sure if I haven't changed one or two, so that could be cause of some bugs you may experience.

This code was developed using code::blocks. Converting it to makefile shouldn't be much of hassle. Also, with a bit of work this code could become OS-independent. Having RPi running 100ppl server would be a nice stunt.

## License & contributing

This project is heavily based on reverse engineering, therefore claiming copyright for such work is plain wrong. Feel free to use this code in any way. I don't plan to actively develop it, but if you feel like adding some stuff or have some questions feel free to add issues, since there is lots of stuff I haven't posted here, which is buried deep in my hdd and head. Pull requests are welcome!
