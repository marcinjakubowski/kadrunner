#include "HistoryModel.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

#include <KRunner/AbstractRunner>
#include <KRunner/QueryMatch>

#include <algorithm>

HistoryModel::HistoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    m_dbPath = dataDir + QStringLiteral("/history.json");

    const QString cfgDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(cfgDir);
    m_cfgPath = cfgDir + QStringLiteral("/settings.json");

    loadSettings();
    loadHistory();
}

void HistoryModel::setMode(const QString &mode)
{
    if (m_mode == mode) {
        return;
    }
    m_mode = mode;
    beginResetModel();
    sortEntries();
    endResetModel();
    saveSettings();
    emit settingsChanged();
}

void HistoryModel::setMaxItems(int maxItems)
{
    const int clamped = std::clamp(maxItems, 1, 10);
    if (m_maxItems == clamped) {
        return;
    }
    m_maxItems = clamped;
    beginResetModel();
    endResetModel();
    saveSettings();
    emit settingsChanged();
}

int HistoryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || m_mode == QLatin1String("off")) {
        return 0;
    }
    return std::min(static_cast<int>(m_entries.size()), m_maxItems);
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount()) {
        return {};
    }
    const auto &e = m_entries[index.row()];
    switch (role) {
    case DisplayRole:
        return e.title;
    case DecorationRole:
        return e.iconName.isEmpty() ? QStringLiteral("application-x-executable") : e.iconName;
    case SubtextRole:
        return e.subtext;
    case CategoryRole:
        return e.category;
    default:
        return {};
    }
}

QHash<int, QByteArray> HistoryModel::roleNames() const
{
    return {
        {DisplayRole, "display"},
        {DecorationRole, "decoration"},
        {SubtextRole, "subtext"},
        {CategoryRole, "category"},
    };
}

const HistoryEntry *HistoryModel::entryAt(int row) const
{
    if (row < 0 || row >= rowCount()) {
        return nullptr;
    }
    return &m_entries[row];
}

void HistoryModel::recordMatch(const KRunner::QueryMatch &match, const QString &query)
{
    const QString id = match.id();
    if (id.isEmpty()) {
        return;
    }

    const qint64 now = QDateTime::currentSecsSinceEpoch();
    auto it = std::find_if(m_entries.begin(), m_entries.end(), [&](const HistoryEntry &e) {
        return e.id == id;
    });

    if (it != m_entries.end()) {
        it->count += 1;
        it->lastUsed = now;
        it->title = match.text();
        it->subtext = match.subtext();
        it->category = match.matchCategory();
        if (!match.iconName().isEmpty()) {
            it->iconName = match.iconName();
        }
        if (match.runner()) {
            it->runnerId = match.runner()->id();
        }
        if (!query.isEmpty()) {
            it->query = query;
        }
    } else {
        HistoryEntry entry;
        entry.id = id;
        entry.title = match.text();
        entry.subtext = match.subtext();
        entry.category = match.matchCategory();
        entry.iconName = match.iconName();
        entry.runnerId = match.runner() ? match.runner()->id() : QString();
        entry.query = query;
        entry.count = 1;
        entry.lastUsed = now;
        m_entries.append(entry);
    }

    sortAndSave();
}

void HistoryModel::incrementEntry(int row)
{
    if (row < 0 || row >= m_entries.size()) {
        return;
    }
    m_entries[row].count += 1;
    m_entries[row].lastUsed = QDateTime::currentSecsSinceEpoch();
    sortAndSave();
}

void HistoryModel::sortEntries()
{
    std::sort(m_entries.begin(), m_entries.end(), [this](const HistoryEntry &a, const HistoryEntry &b) {
        if (m_mode == QLatin1String("recent")) {
            if (a.lastUsed != b.lastUsed) {
                return a.lastUsed > b.lastUsed;
            }
            return a.count > b.count;
        }
        if (a.count != b.count) {
            return a.count > b.count;
        }
        return a.lastUsed > b.lastUsed;
    });
}

void HistoryModel::loadSettings()
{
    QFile f(m_cfgPath);
    if (!f.open(QIODevice::ReadOnly)) {
        return;
    }
    const QJsonObject obj = QJsonDocument::fromJson(f.readAll()).object();
    const QString m = obj.value(QStringLiteral("mode")).toString(QStringLiteral("often"));
    if (m == QLatin1String("often") || m == QLatin1String("recent") || m == QLatin1String("off")) {
        m_mode = m;
    }
    m_maxItems = std::clamp(obj.value(QStringLiteral("maxItems")).toInt(5), 1, 10);
}

void HistoryModel::saveSettings() const
{
    QJsonObject obj;
    obj.insert(QStringLiteral("mode"), m_mode);
    obj.insert(QStringLiteral("maxItems"), m_maxItems);
    QFile f(m_cfgPath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    }
}

void HistoryModel::loadHistory()
{
    QFile f(m_dbPath);
    if (!f.open(QIODevice::ReadOnly)) {
        return;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isArray()) {
        return;
    }
    for (const QJsonValue &v : doc.array()) {
        const QJsonObject o = v.toObject();
        HistoryEntry e;
        e.id = o.value(QStringLiteral("id")).toString();
        e.title = o.value(QStringLiteral("title")).toString();
        e.subtext = o.value(QStringLiteral("subtext")).toString();
        e.category = o.value(QStringLiteral("category")).toString();
        e.iconName = o.value(QStringLiteral("iconName")).toString();
        e.runnerId = o.value(QStringLiteral("runnerId")).toString();
        e.query = o.value(QStringLiteral("query")).toString();
        e.count = o.value(QStringLiteral("count")).toInt(1);
        e.lastUsed = o.value(QStringLiteral("lastUsed")).toInteger();
        if (!e.id.isEmpty() && !e.title.isEmpty()) {
            m_entries.append(e);
        }
    }
    sortEntries();
}

void HistoryModel::sortAndSave()
{
    beginResetModel();
    sortEntries();
    endResetModel();

    QJsonArray arr;
    for (const auto &e : m_entries) {
        QJsonObject o;
        o.insert(QStringLiteral("id"), e.id);
        o.insert(QStringLiteral("title"), e.title);
        o.insert(QStringLiteral("subtext"), e.subtext);
        o.insert(QStringLiteral("category"), e.category);
        o.insert(QStringLiteral("iconName"), e.iconName);
        o.insert(QStringLiteral("runnerId"), e.runnerId);
        o.insert(QStringLiteral("query"), e.query);
        o.insert(QStringLiteral("count"), e.count);
        o.insert(QStringLiteral("lastUsed"), e.lastUsed);
        arr.append(o);
    }

    QFile f(m_dbPath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(QJsonDocument(arr).toJson(QJsonDocument::Compact));
    }
}
