#ifndef BLEMANAGERBLEPP_H
#define BLEMANAGERBLEPP_H

#include <BleManager.h>

#include <blepp/logging.h>
#include <blepp/lescan.h>
#include <blepp/blestatemachine.h>
#include <blepp/float.h>

#include <condition_variable>
#include <atomic>
#include <cmath>
#include <thread>
#include <future>
#include <map>

class BleManagerBlePP : public kinemic::BleManager
{
public:
    explicit BleManagerBlePP(const std::string &adapter = "hci0");

    BleManagerBlePP(BleManagerBlePP&&) = delete;
    BleManagerBlePP(const BleManagerBlePP&) = delete;
    BleManagerBlePP& operator=(BleManagerBlePP&&) noexcept = delete;
    BleManagerBlePP& operator=(const BleManagerBlePP&) = delete;

    ~BleManagerBlePP();

    bool connect(const std::string &address) override;

    bool disconnect(const std::string &band) override;

    void writeChar(const std::string &band, const std::string &characteristic, const std::vector<uint8_t> &value, bool response) override;

    std::vector<uint8_t> readChar(const std::string &band, const std::string &characteristic) override;

    bool enableNotifications(const std::string &band,
                             const std::string &characteristic,
                             std::function<void(const std::vector<uint8_t> &)> callback) override;

    bool setDisconnectCallback(const std::string &band, std::function<void()> callback) override;

    void scan(int16_t duration_ms, std::function<void(const std::string, const int16_t)> callback,
            std::function<void()> onStart, std::function<void()> onStop) override;

    void stopScan() override;

    void changeAdapter(const std::string &adapter);

private:
    //std::unique_ptr<BLEPP::BLEGATTStateMachine> mGatt;
    std::map<std::string, std::shared_ptr<BLEPP::BLEGATTStateMachine>> mGatts;

    std::map<std::string, std::vector<BLEPP::Characteristic*>> characteristics;
    std::map<std::string, std::vector<BLEPP::UUID>> services;

    std::map<std::string, std::shared_ptr<std::mutex>> m_gatt_mutexes;
    std::map<std::string, std::shared_ptr<std::condition_variable>> m_gatt_cvs;
    std::map<std::string, bool> m_gatt_action_finished;

    std::map<std::string, std::atomic_bool> mConnectedMap;
    std::map<std::string, std::atomic_bool> m_local_dc_map;

    std::map<std::string, std::shared_ptr<std::thread>> blepp_state_machines;

    //BLEPP::HCIScanner scanner;
    std::thread BleppScanThread;
    std::string mAdapter;
    std::vector<std::string> mBands;
    std::atomic_bool mAsynchActive{false};
    std::map<std::string, std::vector<std::function<void(const std::vector<uint8_t> &)>>> mNotificationCallbacks;

    std::mutex scanMutex;
    bool mScanning = false;

    const std::string METASERVICEUUID = "326A9000-85CB-9195-D9DD-464CFBBAE75A";
    std::map<std::string, std::function<void()>> mDisconnectCallbacks;

    void enableAsynch();

    std::atomic_bool m_asynch_thread_running{false};
    std::thread m_asynch_thread;

    void closeAndReenable();

    void clear_characteristics();
};

#endif // BLEMANAGERBLEPP_H
