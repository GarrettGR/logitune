#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include "devices/DescriptorWriter.h"

TEST(DescriptorWriter, WritesValidJsonAtomically) {
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QJsonObject obj;
    obj[QStringLiteral("name")] = QStringLiteral("Test");
    obj[QStringLiteral("status")] = QStringLiteral("community-local");

    logitune::DescriptorWriter w;
    QString err;
    EXPECT_EQ(w.write(tmp.path(), obj, &err), logitune::DescriptorWriter::Ok);
    EXPECT_TRUE(err.isEmpty());

    QFile f(tmp.path() + QStringLiteral("/descriptor.json"));
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    auto doc = QJsonDocument::fromJson(f.readAll());
    ASSERT_FALSE(doc.isNull());
    EXPECT_EQ(doc.object()[QStringLiteral("name")].toString(), QStringLiteral("Test"));
}

TEST(DescriptorWriter, ReturnsIoErrorOnUnwritableDir) {
    logitune::DescriptorWriter w;
    QString err;
    QJsonObject obj;
    obj[QStringLiteral("name")] = QStringLiteral("x");
    EXPECT_EQ(w.write(QStringLiteral("/proc/nonexistent-do-not-create"), obj, &err),
              logitune::DescriptorWriter::IoError);
    EXPECT_FALSE(err.isEmpty());
}
