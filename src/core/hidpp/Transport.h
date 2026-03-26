#pragma once
#include "HidppTypes.h"
#include "HidrawDevice.h"

#include <QObject>
#include <QMutex>
#include <optional>

namespace logitune::hidpp {

class Transport : public QObject {
    Q_OBJECT
public:
    explicit Transport(HidrawDevice *device, QObject *parent = nullptr);

    // Thread-safe: can be called from any thread.
    // If called from a different thread than the one owning the fd,
    // it uses a mutex to serialize access.
    std::optional<Report> sendRequest(const Report &request, int timeoutMs = 2000);

    void run();   // Notification listener loop — call from I/O thread
    void stop();

signals:
    void notificationReceived(const Report &report);
    void deviceError(ErrorCode code, uint8_t featureIndex);
    void deviceDisconnected();

private:
    std::optional<Report> trySend(const Report &request, int timeoutMs, int retriesLeft);

    HidrawDevice *m_device;
    QMutex m_fdMutex;  // Serializes all fd access (read + write)
    bool m_running = false;
};

} // namespace logitune::hidpp
