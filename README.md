## About LockStatus

![Screenshot](/docs/screenshot.png?raw=true)

This is a tiny program whose sole purpose is to display the status of Caps Lock, Num Lock, and/or Scroll Lock in the notification area, a.k.a. the system tray.

The program can be launched without arguments, in which case it will only show an icon for Caps Lock and Scroll Lock, and only when they are in the "on" state.

The program has no settings and no installer. To make it start at system startup, just [drop it into the Startup folder](http://windows.microsoft.com/en-US/windows-vista/Run-a-program-automatically-when-Windows-starts). There is no way to exit the program either; the tray icons it shows have no right-click menu.

LockStatus uses roughly 1 MB of private bytes RAM.

## Command line options

The program optionally takes a single three-letter argument on the command line. Each letter configures how you want a certain key's status indicated. The first letter corresponds to the Caps Lock key, second to Num Lock, and third to Scroll Lock.

The following characters can be used:

* "0" (zero): show the icon only if this key status is OFF.
* "1": show the icon only if this key status is ON.
* "N": never show the icon for this key.
* "A" or anything else: always show the icon for this key.

So, for example, "AAA" will show the status of all three keys at all times, while "101" will show Caps/Scroll lock icon only if they're ON, and a Num Lock icon only if it's OFF.

If you don't provide exactly one three-character-long argument, default settings will be used (equivalent to "1N1")
