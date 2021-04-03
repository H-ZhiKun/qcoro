// SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: MIT

#include "testobject.h"
#include "qcoro/coro.h"

class QCoroProcessTest: public QCoro::TestObject<QCoroProcessTest>
{
    Q_OBJECT

private:
    QCoro::Task<> testStartTriggers_coro(QCoro::TestContext context) {
        QProcess process;

        co_await QCoro::coro(process).start(QStringLiteral("true"), {});

        QCORO_COMPARE(process.state(), QProcess::Running);

        process.waitForFinished();
    }

    QCoro::Task<> testStartNoArgsTriggers_coro(QCoro::TestContext context) {
        QProcess process;
        process.setProgram(QStringLiteral("true"));

        co_await QCoro::coro(process).start();

        QCORO_COMPARE(process.state(), QProcess::Running);

        process.waitForFinished();
    }

    QCoro::Task<> testStartDoesntBlock_coro(QCoro::TestContext context) {
        QCoro::EventLoopChecker eventLoopResponsive{1, 0ms};

        QProcess process;
        co_await QCoro::coro(process).start(QStringLiteral("true"), {});

        QCORO_VERIFY(eventLoopResponsive);

        process.waitForFinished();
    }

    QCoro::Task<> testStartDoesntCoAwaitRunningProcess_coro(QCoro::TestContext ctx) {
        QProcess process;
        co_await QCoro::coro(process).start(QStringLiteral("sleep"), {QStringLiteral("1")});

        QCORO_COMPARE(process.state(), QProcess::Running);

        ctx.setShouldNotSuspend();

        QTest::ignoreMessage(QtWarningMsg, "QProcess::start: Process is already running");
        co_await QCoro::coro(process).start();

        process.waitForFinished();
    }

    QCoro::Task<> testFinishTriggers_coro(QCoro::TestContext ctx) {
        QProcess process;
        process.start(QStringLiteral("sleep"), {QStringLiteral("1")});
        process.waitForStarted();

        QCORO_COMPARE(process.state(), QProcess::Running);

        const auto ok = co_await QCoro::coro(process).finished();

        QCORO_VERIFY(ok);
        QCORO_COMPARE(process.state(), QProcess::NotRunning);
    }

    QCoro::Task<> testFinishDoesntCoAwaitFinishedProcess_coro(QCoro::TestContext ctx) {
        QProcess process;
        process.start(QStringLiteral("true"), QStringList{});
        process.waitForFinished();

        ctx.setShouldNotSuspend();

        const auto ok = co_await QCoro::coro(process).finished();
        QCORO_VERIFY(ok);
    }

    QCoro::Task<> testFinishCoAwaitTimeout_coro(QCoro::TestContext ctx) {
        QProcess process;
        process.start(QStringLiteral("sleep"), {QStringLiteral("2")});
        process.waitForStarted();

        QCORO_COMPARE(process.state(), QProcess::Running);

        const auto ok = co_await QCoro::coro(process).finished(1s);

        QCORO_VERIFY(!ok);
        QCORO_COMPARE(process.state(), QProcess::Running);

        process.waitForFinished();
    }

private Q_SLOTS:
    addTest(StartTriggers)
    addTest(StartNoArgsTriggers)
    addTest(StartDoesntBlock)
    addTest(StartDoesntCoAwaitRunningProcess)
    addTest(FinishTriggers)
    addTest(FinishDoesntCoAwaitFinishedProcess)
    addTest(FinishCoAwaitTimeout)
};

QTEST_GUILESS_MAIN(QCoroProcessTest)

#include "qprocess.moc"