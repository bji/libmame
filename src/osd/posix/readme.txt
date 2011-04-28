This is a version of the mame osd that implements as much of the OSD
as possible using pure POSIX API calls.  POSIX is the portable
operating system interface, and should be supported on all systems.

For those systems that do not support POSIX, compatibility functions
are defined in system-specific files.  Generally this applies to
Microsoft Windows only as it's the only modern operating system that
doesn't support POSIX well.

The parts of the OSD that are not implemented in a meaningful way by
this implementation are meant to be implemented elsewhere.
