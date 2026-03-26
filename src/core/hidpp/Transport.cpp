#include "Transport.h"

#include <QThread>
#include <QDebug>

namespace logitune::hidpp {

Transport::Transport(HidrawDevice *device, QObject *parent)
    : QObject(parent)
    , m_device(device)
{}

std::optional<Report> Transport::sendRequest(const Report &request, int timeoutMs)
{
    QMutexLocker lock(&m_fdMutex);
    return trySend(request, timeoutMs, /*retriesLeft=*/3);
}

std::optional<Report> Transport::trySend(const Report &request, int timeoutMs, int retriesLeft)
{
    // Mutex must be held by caller

    auto bytes = request.serialize();
    int written = m_device->writeReport(bytes);
    if (written < 0) {
        qDebug() << "[Transport] write failed";
        return std::nullopt;
    }

    // Read responses, skipping ones that don't match our request
    int readAttempts = 0;
    while (readAttempts < 10) {
        auto responseBytes = m_device->readReport(timeoutMs);
        if (responseBytes.empty()) {
            if (retriesLeft > 0)
                return trySend(request, timeoutMs, retriesLeft - 1);
            qDebug() << "[Transport] timeout waiting for response";
            return std::nullopt;
        }
        readAttempts++;

        auto response = Report::parse(responseBytes);
        if (!response) {
            qDebug() << "[Transport] failed to parse response";
            continue;
        }

        // Check if this response matches our request
        if (response->deviceIndex == request.deviceIndex &&
            response->featureIndex == request.featureIndex) {
            if (response->isError()) {
                ErrorCode ec = response->errorCode();
                qDebug() << "[Transport] error response: code=" << static_cast<int>(ec);
                emit deviceError(ec, response->params[0]);
                switch (ec) {
                case ErrorCode::Busy:
                    if (retriesLeft > 0) {
                        QThread::msleep(100);
                        return trySend(request, timeoutMs, retriesLeft - 1);
                    }
                    return std::nullopt;
                case ErrorCode::OutOfRange:
                    if (retriesLeft > 0)
                        return trySend(request, timeoutMs, 0);
                    return std::nullopt;
                default:
                    return std::nullopt;
                }
            }
            return response;
        }

        // Not our response — it's a notification. Emit it and keep reading.
        if (!response->isError()) {
            emit notificationReceived(*response);
        }
    }

    qDebug() << "[Transport] exhausted read attempts";
    return std::nullopt;
}

void Transport::run()
{
    qDebug() << "[Transport] I/O thread started, listening for notifications";
    m_running = true;
    while (m_running) {
        // Try to acquire the mutex — if sendRequest holds it, just skip this cycle
        if (m_fdMutex.tryLock(10)) {
            auto bytes = m_device->readReport(/*timeoutMs=*/100);
            if (!bytes.empty()) {
                auto report = Report::parse(bytes);
                if (report) {
                    if (report->isError()) {
                        emit deviceError(report->errorCode(), report->params[0]);
                    } else {
                        emit notificationReceived(*report);
                    }
                }
            }
            m_fdMutex.unlock();
        }
    }
    qDebug() << "[Transport] I/O thread stopped";
}

void Transport::stop()
{
    m_running = false;
}

} // namespace logitune::hidpp
