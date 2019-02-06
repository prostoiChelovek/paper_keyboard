//
// Created by prostoichelovek on 06.02.19.
//

#ifndef PAPERKEYBOARD_SYSTEMINTERACTION_HPP
#define PAPERKEYBOARD_SYSTEMINTERACTION_HPP

#if defined(unix) || defined(__unix__) || defined(__unix)
#define OS_UNIX

#include <SerialStream.h>

#endif

namespace SysInter {

#ifdef OS_UNIX
    // https://bharathisubramanian.wordpress.com/2010/03/14/x11-fake-key-event-generation-using-xtest-ext/

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>

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


    LibSerial::SerialStream serial;

    bool openSerial(string port) {
        serial.Open(move(port));
        serial.SetCharSize(LibSerial::SerialStreamBuf::CHAR_SIZE_8);
        serial.SetBaudRate(LibSerial::SerialStreamBuf::BaudRateEnum::BAUD_9600);
        serial.SetNumOfStopBits(1);
        serial.SetFlowControl(LibSerial::SerialStreamBuf::FLOW_CONTROL_NONE);
        return serial.good();
    }

    bool writeSerial(const string &str) {
        if (serial.good()) {
            usleep(5000);
        } else {
            return false;
        }
        serial << str;
        usleep(100000);
        return true;
    }

#endif // is unix

#ifdef _WIN32
#pragma message ( "Windows interaction  is not implemented!" )
#endif // _WIN32

}

#endif //PAPERKEYBOARD_SYSTEMINTERACTION_HPP
