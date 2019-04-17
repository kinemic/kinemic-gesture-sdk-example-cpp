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

#include "blemanagerblepp.h"

BleManagerBlePP::~BleManagerBlePP() {
    for (auto adapter : mBands) disconnect(adapter);
    stopScan();
}

void BleManagerBlePP::clear_characteristics() {
    for (auto adapter : mBands) characteristics.at(adapter).clear();
}

bool BleManagerBlePP::connect(const std::string &address) {
    stopScan();
    auto it = std::find(mBands.begin(), mBands.end(), address);
    if (it != mBands.end()) {
        if (mConnectedMap[address]) return true;
        mBands.erase(it);
    }

    mBands.push_back(address);
    mGatts[address] = std::make_shared<BLEPP::BLEGATTStateMachine>();
    m_local_dc_map[address] = false;
    characteristics[address] = std::vector<BLEPP::Characteristic*>();
    services[address] = std::vector<BLEPP::UUID>();
    m_gatt_mutexes[address] = std::make_shared<std::mutex>();
    m_gatt_cvs[address] = std::make_shared<std::condition_variable>();
    m_gatt_action_finished[address] = false;
    mConnectedMap[address] = false;
    m_local_dc_map[address] = false;

    mGatts[address]->cb_connected = [this, address]() {
        mGatts[address]->read_primary_services();
    };

    mGatts[address]->cb_find_characteristics = [this, address]() {
        mGatts[address]->get_client_characteristic_configuration();
    };

    mGatts[address]->cb_services_read = [this, address]() {
        mGatts[address]->find_all_characteristics();
    };

    std::shared_ptr<std::promise<bool>> connection_promise = std::make_shared<std::promise<bool>>();
    auto connect_future = connection_promise->get_future();

    auto blepp_state_machine = std::make_shared<std::thread> ([this, address, connection_promise]() {
        m_local_dc_map[address] = false;
        mConnectedMap[address] = false;

        bool terminate = false;
        bool never_connected = false;

        mGatts[address] -> cb_disconnected = [&](BLEPP::BLEGATTStateMachine::Disconnect d) {
            never_connected = true;
            terminate = true;
        };

        mGatts[address]->cb_get_client_characteristic_configuration = [&]() {
            services[address].clear();
            characteristics[address].clear();

            for(auto& service: mGatts[address]->primary_services) {
                services[address].push_back(service.uuid);
                for(auto& characteristic: service.characteristics) {
                    characteristics[address].push_back(&characteristic);
                }
            }

            mGatts[address]->cb_disconnected = [&](BLEPP::BLEGATTStateMachine::Disconnect d) {
                terminate = true;
            };
            connection_promise->set_value(true);
        };

        try {
            mGatts[address]->connect(address, false, false, mAdapter);
        } catch (BLEPP::SocketConnectFailed& e) {
            std::cerr << "Blepp Socket Connect Failure: " << e.what() << std::endl;
            mGatts[address]->cb_disconnected = [&](BLEPP::BLEGATTStateMachine::Disconnect d) {};
            mGatts[address]->close();
            mConnectedMap[address] = false;
            m_local_dc_map[address] = true;
            terminate = true;
            never_connected = true;
        }

        auto check_equal = [&](int sd) -> bool {
            if (sd != mGatts[address]->socket()) {
                std::cout << "Socket changed during thread! (was: " << std::to_string(sd) << " is: " << std::to_string(mGatts[address]->socket()) << ")" << std::endl;
                return false;
            }
            return true;
        };

        int sd;
        fd_set write_set{}, read_set{};
        while (!m_local_dc_map[address] && !terminate) {
            FD_ZERO(&read_set);
            FD_ZERO(&write_set);

            sd = mGatts[address]->socket();
            if (sd >= 0) {
                FD_SET(sd, &read_set);
                if(mGatts[address]->wait_on_write()) {
                    FD_SET(sd, &write_set);
                }
            } else {
                std::cout << "Socket invalid before setting FD_SET" << std::endl;
            }

            struct timeval tv{};
            tv.tv_sec = 0;
            tv.tv_usec = 10000;
            select(sd + 1, &read_set, &write_set, nullptr, & tv);

            sd = mGatts[address]->socket();
            if (sd >= 0) {
                if(FD_ISSET(sd, &write_set)) {
                    mGatts[address]->write_and_process_next();
                }
            } else {
                std::cout << "Socket invalid before FD_ISSET for write" << std::endl;
            }

            sd = mGatts[address]->socket();
            if (sd >= 0) {
                if(FD_ISSET(sd, &read_set)) {
                    mGatts[address]->read_and_process_next();
                }
            } else {
                std::cout << "Socket invalid before FD_ISSET for read" << std::endl;
            }
        }

        mGatts[address]->cb_disconnected = [&](BLEPP::BLEGATTStateMachine::Disconnect d) {};
        mGatts[address]->close();
        mConnectedMap[address] = false;
        if (mDisconnectCallbacks[address] != nullptr) {
            mDisconnectCallbacks[address]();
        }
    if (never_connected) {
        connection_promise->set_value(false);
    }
    });
    blepp_state_machines[address] = blepp_state_machine;

    mConnectedMap[address] = connect_future.get();
    if (!mConnectedMap[address]) {
        blepp_state_machines[address]->join();
    }
    return mConnectedMap[address];
}

bool BleManagerBlePP::disconnect(const std::string &band) {
    m_local_dc_map[band] = true;

    if (blepp_state_machines[band]->joinable()) {
        blepp_state_machines[band]->join();
    }

    //shutdown(mGatt.socket(), SHUT_RDWR);
    mConnectedMap[band] = false;

    return !mConnectedMap[band];
}

void BleManagerBlePP::writeChar(const std::string &band, const std::string &characteristic, const std::vector<uint8_t> &value, bool response) {
    if (mConnectedMap[band]) {
        for (auto &chara: characteristics[band]) {
            if (chara->uuid == BLEPP::UUID(characteristic)) {
                if (response) {
                    chara->write_request(&value[0], value.size());
                } else {
                    chara->write_command(&value[0], value.size());
                }
                break;
            }
        }
    }
}

std::vector<uint8_t> BleManagerBlePP::readChar(const std::string &band, const std::string &characteristic) {
    std::vector<uint8_t> return_vector;
    if (mConnectedMap[band]) {
            for(auto& chara: characteristics[band]) {
                if (chara->uuid == BLEPP::UUID(characteristic)) {
                    chara->cb_read = [&](const BLEPP::PDUReadResponse &r) {
                        std::unique_lock<std::mutex> lk(*m_gatt_mutexes[band]);
                        m_gatt_action_finished[band] = true;
                        auto val = r.value();
                        return_vector = std::vector<uint8_t>(val.first, val.second);
                        m_gatt_cvs[band]->notify_all();
                    };

                    std::unique_lock<std::mutex> m_lock_for_gatt_connection(*m_gatt_mutexes[band]);
                    m_gatt_action_finished[band] = false;
                    chara->read_request();
                    m_gatt_cvs[band]->wait_for(m_lock_for_gatt_connection, std::chrono::milliseconds(3000),
                                        [=] { return m_gatt_action_finished[band]; });
                    goto name_found;
                }
            }
            std::cerr << "Characteristic " << characteristic << " not found!" << std::endl;
            return return_vector;
            name_found:
           ;
    }
    return return_vector;
}

BleManagerBlePP::BleManagerBlePP(const std::string &adapter) {
    mAsynchActive = false;
    mScanning = false;
    mAdapter = adapter;
}

void BleManagerBlePP::changeAdapter(const std::string &adapter) {
    mAdapter = adapter;
}

bool BleManagerBlePP::enableNotifications(const std::string &band,
                                          const std::string &characteristic,
                                          std::function<void(const std::vector<uint8_t> &)> callback) {
    if (mConnectedMap[band]) {
        for (auto &chara: characteristics[band]) {
            if (chara->uuid == BLEPP::UUID(characteristic)) {
                std::function<void(BLEPP::Characteristic &chara,
                                    const BLEPP::PDUNotificationOrIndication&)> notify_cb = [callback](
                        BLEPP::Characteristic &chara,
                        const BLEPP::PDUNotificationOrIndication& n)
                {
                    auto v = n.value();
                    std::vector<uint8_t> nvec(v.first, v.second);
                    callback(nvec);
                };
                mNotificationCallbacks[band].push_back(callback);

                mGatts[band]->cb_notify_or_indicate = notify_cb;
                uint8_t valarray[2] = {1, 0};
                mGatts[band]->send_write_command(chara->value_handle + 1, valarray, static_cast<int>(sizeof(valarray)));
                return true;
            }
        }
    }
    return false;
}

bool BleManagerBlePP::setDisconnectCallback(const std::string &band,
                                            std::function<void()> callback) {
    mDisconnectCallbacks[band] = callback;
    return true;
}

void BleManagerBlePP::scan(int16_t duration_ms, std::function<void(const std::string, const int16_t)> callback,
                           std::function<void()> onStart, std::function<void()> onStop) {
    stopScan();

    std::unique_lock<std::mutex> lk(scanMutex);
    mScanning = true;
    BleppScanThread = std::thread([onStart, onStop, callback, duration_ms, this]{
        auto scan_start = std::chrono::steady_clock::now();
        try {
            BLEPP::HCIScanner scanner(false, BLEPP::HCIScanner::FilterDuplicates::Off,
                                      BLEPP::HCIScanner::ScanType::Passive, mAdapter);
            try {
                scanner.start();
            } catch (BLEPP::HCIScanner::IOError &error) {
                std::cerr << error.what() << std::endl;
                std::cerr << "Was not able to start scanner. Scanning requires 'cap_net_raw,cap_net_admin+eip' permissions" << std::endl;
                mScanning = false;
            }
            onStart();
            auto curtime = std::chrono::steady_clock::now();
            while (mScanning) {
                if (duration_ms > 0) {
                    if (std::chrono::duration_cast<std::chrono::milliseconds>(curtime - scan_start).count() > duration_ms) {
                        mScanning = false;
                        break;
                    }
                }
                std::vector<BLEPP::AdvertisingResponse> ads = scanner.get_advertisements();
                for(const auto& ad: ads)
                {
                    if(std::find(ad.UUIDs.begin(), ad.UUIDs.end(),
                                 BLEPP::UUID(METASERVICEUUID)) != ad.UUIDs.end()) {
                        callback(ad.address, (int16_t) ad.rssi);
                    }
                }
                curtime = std::chrono::steady_clock::now();
            }
            try {
                scanner.stop();
            } catch (BLEPP::HCIScanner::IOError &error) {
                std::cerr << error.what() << std::endl;
            }
            onStop();

        } catch (BLEPP::HCIScanner::HCIError &error) {
            std::cerr << error.what() << std::endl;
            std::cerr << "Was not able to create scanner. Did you request an invalid BLE adapter?" << std::endl;
            mScanning = false;
            onStart();
            onStop();
        }
    });
}

void BleManagerBlePP::stopScan() {
    std::unique_lock<std::mutex> lk(scanMutex);
    mScanning = false;
    if (BleppScanThread.joinable()) BleppScanThread.join();
}
