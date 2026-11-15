#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QString>

namespace KRunner {
class QueryMatch;
}

struct HistoryEntry {
    QString id;
    QString title;
    QString subtext;
    QString category;
    QString iconName;
    QString runnerId;
    QString query;
    int count = 0;
    qint64 lastUsed = 0;
};

/**
 * Stores and ranks launched KRunner matches by frequency or recency,
 * persisting both launch history and user display preferences.
 */
class HistoryModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString mode READ mode WRITE setMode NOTIFY settingsChanged)
    Q_PROPERTY(int maxItems READ maxItems WRITE setMaxItems NOTIFY settingsChanged)
    Q_PROPERTY(int launcherWidth READ launcherWidth WRITE setLauncherWidth NOTIFY settingsChanged)
    Q_PROPERTY(int maxVisibleRows READ maxVisibleRows WRITE setMaxVisibleRows NOTIFY settingsChanged)
    Q_PROPERTY(int posX READ posX WRITE setPosX NOTIFY settingsChanged)
    Q_PROPERTY(int posY READ posY WRITE setPosY NOTIFY settingsChanged)

public:
    enum Roles {
        DecorationRole = Qt::DecorationRole,
        DisplayRole = Qt::DisplayRole,
        SubtextRole = Qt::UserRole + 1,
        CategoryRole = Qt::UserRole + 2,
    };

    explicit HistoryModel(QObject *parent = nullptr);

    QString mode() const { return m_mode; }
    void setMode(const QString &mode);

    int maxItems() const { return m_maxItems; }
    void setMaxItems(int maxItems);

    int launcherWidth() const { return m_launcherWidth; }
    void setLauncherWidth(int width);

    int maxVisibleRows() const { return m_maxVisibleRows; }
    void setMaxVisibleRows(int rows);

    int posX() const { return m_posX; }
    void setPosX(int x);

    int posY() const { return m_posY; }
    void setPosY(int y);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    const HistoryEntry *entryAt(int row) const;
    void recordMatch(const KRunner::QueryMatch &match, const QString &query);
    void incrementEntry(int row);

Q_SIGNALS:
    void settingsChanged();

private:
    void sortEntries();
    void loadSettings();
    void saveSettings() const;
    void loadHistory();
    void sortAndSave();

    QString m_dbPath;
    QString m_cfgPath;
    QString m_mode = QStringLiteral("often");
    int m_maxItems = 5;
    int m_launcherWidth = 760;
    int m_maxVisibleRows = 8;
    int m_posX = -1;
    int m_posY = 220;
    QList<HistoryEntry> m_entries;
};
