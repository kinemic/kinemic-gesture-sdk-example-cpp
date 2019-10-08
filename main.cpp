/*
 *
 *  Kinemic Gesture SDK C++ Example
 *
 *  Copyright (C) 2019 Kinemic GmbH
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <map>
#include <mutex>
#include <iostream>
#ifdef WIN32
#include <kinemic/KinemicEngine.h>
#include <kinemic/EventHandlers.h>
#include <objbase.h>
#else
#include "blemanagerblepp.h"
#include <KinemicEngine.h>
#include <EventHandlers.h>
#endif

using namespace std;

int main()
{
    cout << "Creating Kinemic Engine on adapter hci0!" << endl;
#ifdef WIN32
	CoInitialize(NULL);
    kinemic::KinemicEngine engine;
#else
    kinemic::KinemicEngine engine(std::make_shared<BleManagerBlePP>("hci0"));
#endif

    cout << "Registering connection state handler" << endl;

    std::map<kinemic::EngineHandler::connection_state, std::string> state_names {
      std::make_pair(kinemic::EngineHandler::connection_state::DISCONNECTED, "DISCONNECTED"),
      std::make_pair(kinemic::EngineHandler::connection_state::DISCONNECTING, "DISCONNECTING"),
      std::make_pair(kinemic::EngineHandler::connection_state::CONNECTED, "CONNECTED"),
      std::make_pair(kinemic::EngineHandler::connection_state::CONNECTING, "CONNECTING"),
      std::make_pair(kinemic::EngineHandler::connection_state::RECONNECTING, "RECONNECTING")
    };

    kinemic::EngineHandler ehandler;
    ehandler.onConnectionStateChanged = [&](const std::string &band, kinemic::EngineHandler::connection_state state, kinemic::EngineHandler::connection_reason reason) {
        cout << "Band " << band << " is now: " << state_names[state] << endl;
    };
    engine.registerEventHandler(ehandler);

    std::map<kinemic::GestureHandler::gesture, std::string> gesture_names {
      std::make_pair(kinemic::GestureHandler::gesture::ROTATE_RL, "Rotate RL"),
              std::make_pair(kinemic::GestureHandler::gesture::ROTATE_RL, "Rotate RL"),
              std::make_pair(kinemic::GestureHandler::gesture::ROTATE_LR, "Rotate LR"),
              std::make_pair(kinemic::GestureHandler::gesture::CIRCLE_R, "Circle R"),
              std::make_pair(kinemic::GestureHandler::gesture::CIRCLE_L, "Circle L"),
              std::make_pair(kinemic::GestureHandler::gesture::SWIPE_R, "Swipe R"),
              std::make_pair(kinemic::GestureHandler::gesture::SWIPE_L, "Swipe L"),
              std::make_pair(kinemic::GestureHandler::gesture::SWIPE_UP, "Swipe Up"),
              std::make_pair(kinemic::GestureHandler::gesture::SWIPE_DOWN, "Swipe Down"),
              std::make_pair(kinemic::GestureHandler::gesture::CHECK_MARK, "Check Mark"),
              std::make_pair(kinemic::GestureHandler::gesture::CROSS_MARK, "X Mark"),
              std::make_pair(kinemic::GestureHandler::gesture::EARTOUCH_L, "Eartouch L"),
              std::make_pair(kinemic::GestureHandler::gesture::EARTOUCH_R, "Eartouch R")
    };

    kinemic::GestureHandler ghandler;
    ghandler.onGesture = [&](const std::string &band, kinemic::GestureHandler::gesture gesture) {
        cout << "Band " << band << " issued a gesture event: " << gesture_names[gesture] << endl;
    };
    engine.registerEventHandler(ghandler);

    std::mutex cmutex;
    std::condition_variable cv;
    bool search_stopped = false;
    std::unique_lock<std::mutex> lk(cmutex);

    cout << "Searching for Kinemic Bands and connecting to strongest one!" << endl;
    kinemic::SearchHandler shandler;
    shandler.onSensorFound = [&](const std::string &addr, const int16_t rssi) {
        cout << endl << "Current candidate: " << addr << " (" << rssi << "%)";
    };
    shandler.onSearchStopped = [&] {
        cout << "Stopped searching for Bands" << endl;
        std::unique_lock<std::mutex> lk(cmutex);
        search_stopped = true;
        cv.notify_all();
    };
    engine.connectStrongest(shandler);
    cv.wait(lk, [&]{return search_stopped;});

    auto bands = engine.getBands();
    if (bands.size() > 0) {
        cout << "Connecting now to connection candidate: " << bands[0] << endl;
        cout << "Press Enter to exit" << endl;
        cin.get();
    } else {
        cout << "Did not find a Kinemic Band in range..." << endl;
        cout << "Make sure your Band is charged and in range!" << endl;
    }

    return 0;
}
