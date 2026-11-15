#pragma once

#include <KRunner/ResultsModel>
#include <QObject>

#include "HistoryModel.h"

class QQmlComponent;
class QQuickWindow;

/**
 * Bridges KRunner::ResultsModel and HistoryModel to QML, and manages
 * creating/destroying the LayerShellQt overlay window on demand.
 */
class RunnerBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(KRunner::ResultsModel *model READ model CONSTANT)
    Q_PROPERTY(HistoryModel *historyModel READ historyModel CONSTANT)

public:
    explicit RunnerBridge(KRunner::ResultsModel *model,
                          HistoryModel *history,
                          QQmlComponent *windowComponent,
                          QObject *parent = nullptr);

    KRunner::ResultsModel *model() const { return m_model; }
    HistoryModel *historyModel() const { return m_history; }

    Q_INVOKABLE bool runRow(int row);
    Q_INVOKABLE bool runHistoryRow(int row);
    Q_INVOKABLE void hide();
    Q_INVOKABLE void show();
    Q_INVOKABLE void moveBy(int dx, int dy);
    Q_INVOKABLE void resetPosition();
    Q_INVOKABLE void setDesignMode(bool enabled);
    Q_INVOKABLE int activeScreenWidth() const;
    Q_INVOKABLE int activeScreenHeight() const;
    void toggle();

Q_SIGNALS:
    void shown();

private:
    void applyLayerGeometry();
    KRunner::ResultsModel *m_model = nullptr;
    HistoryModel *m_history = nullptr;
    QQmlComponent *m_windowComponent = nullptr;
    QQuickWindow *m_window = nullptr;
    bool m_closing = false;
};
