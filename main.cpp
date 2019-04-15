#include <iostream>
#include <KinemicEngine.h>
#include <EventHandlers.h>
#include "blemanagerblepp.h"

using namespace std;

int main()
{
    cout << "Creating Kinemic Engine on adapter hci0!" << endl;
    kinemic::KinemicEngine engine(std::make_shared<BleManagerBlePP>("hci0"));

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
