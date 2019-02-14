//
// Created by prostoichelovek on 06.02.19.
//

#ifndef PAPERKEYBOARD_SYSTEMINTERACTION_HPP
#define PAPERKEYBOARD_SYSTEMINTERACTION_HPP

#include <unistd.h>

#if defined(unix) || defined(__unix__) || defined(__unix)
#define OS_UNIX
#endif

namespace SysInter {

#ifdef OS_UNIX
    // https://bharathisubramanian.wordpress.com/2010/03/14/x11-fake-key-event-generation-using-xtest-ext/

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/extensions/XTest.h>

    Display *disp;

    void init() {
        disp = XOpenDisplay(nullptr);
    }

    void sendKey(KeySym keysym, KeySym modsym) {
        KeyCode keycode = 0, modcode = 0;
        keycode = XKeysymToKeycode(disp, keysym);
        if (keycode == 0) return;
        XTestGrabControl(disp, True);
        /* Generate modkey press */
        if (modsym != 0) {
            modcode = XKeysymToKeycode(disp, modsym);
            XTestFakeKeyEvent(disp, modcode, True, 0);
        }
        /* Generate regular key press and release */
        XTestFakeKeyEvent(disp, keycode, True, 0);
        XTestFakeKeyEvent(disp, keycode, False, 0);

        /* Generate modkey release */
        if (modsym != 0)
            XTestFakeKeyEvent(disp, modcode, False, 0);

        XSync(disp, False);
        XTestGrabControl(disp, False);
    }

    void sendLetter(const char *c) {
        sendKey(XStringToKeysym(c), 0);
    }

    void sendString(string str) {
        for (int i = 0; i < str.length(); i++) {
            string sym(1, str[i]);
            sendLetter(sym.c_str());
        }
    }

#endif // is unix

#ifdef _WIN32
#pragma message ( "Windows interaction  is not implemented!" )
#endif // _WIN32

}

#endif //PAPERKEYBOARD_SYSTEMINTERACTION_HPP
