#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHash>
#include <spdlog/spdlog.h>

namespace eduerp::services {

/**
 * @brief Manages translations for the EduERP UI.
 *        Loads JSON translation bundles and provides O(1) lookup via QHash.
 *        Supports nl-BE, en-GB, fr-BE as per spec.
 *
 *        Usage from QML:  translator.t("nav.dashboard")
 *        Fallback chain:  requested locale → nl-BE → key itself
 */
class TranslationManager : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString currentLocale READ currentLocale WRITE setLocale NOTIFY localeChanged)
    Q_PROPERTY(QStringList availableLocales READ availableLocales CONSTANT)

public:
    explicit TranslationManager(QObject* parent = nullptr) : QObject(parent) {
        m_availableLocales = {"nl-BE", "en-GB", "fr-BE"};
    }

    /**
     * @brief Load translation files from a directory.
     *        Expects files named: nl-BE.json, en-GB.json, fr-BE.json
     */
    void loadTranslations(const QString& directory) {
        for (const auto& locale : m_availableLocales) {
            QString path = directory + "/" + locale + ".json";
            QFile file(path);

            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                spdlog::warn("TranslationManager: Could not open '{}'", path.toStdString());
                continue;
            }

            auto doc = QJsonDocument::fromJson(file.readAll());
            if (doc.isNull()) {
                spdlog::error("TranslationManager: Invalid JSON in '{}'", path.toStdString());
                continue;
            }

            QHash<QString, QString> translations;
            flattenJson(doc.object(), "", translations);
            m_bundles[locale] = translations;

            spdlog::info("TranslationManager: Loaded {} keys for locale '{}'",
                         translations.size(), locale.toStdString());
        }

        // If no explicit locale set yet, default to nl-BE
        if (m_currentLocale.isEmpty()) {
            m_currentLocale = "nl-BE";
        }
    }

    /**
     * @brief Load translations from embedded JSON strings (no file I/O).
     */
    void loadFromJson(const QString& locale, const QJsonObject& json) {
        QHash<QString, QString> translations;
        flattenJson(json, "", translations);
        m_bundles[locale] = translations;
        spdlog::info("TranslationManager: Loaded {} keys for locale '{}' (inline)",
                     translations.size(), locale.toStdString());
    }

    QString currentLocale() const { return m_currentLocale; }
    QStringList availableLocales() const { return m_availableLocales; }

    void setLocale(const QString& locale) {
        if (m_currentLocale != locale && m_availableLocales.contains(locale)) {
            m_currentLocale = locale;
            spdlog::info("TranslationManager: Locale changed to '{}'", locale.toStdString());
            emit localeChanged();
        }
    }

    /**
     * @brief Translate a key. This is the primary API, callable from QML.
     * @param key Dot-separated key, e.g. "nav.dashboard" or "finance.ledger_title"
     * @return Translated string, or key itself if not found.
     */
    Q_INVOKABLE QString t(const QString& key) const {
        // Try current locale
        if (m_bundles.contains(m_currentLocale)) {
            const auto& bundle = m_bundles[m_currentLocale];
            auto it = bundle.find(key);
            if (it != bundle.end()) return it.value();
        }

        // Fallback to nl-BE (primary language)
        if (m_currentLocale != "nl-BE" && m_bundles.contains("nl-BE")) {
            const auto& fallback = m_bundles["nl-BE"];
            auto it = fallback.find(key);
            if (it != fallback.end()) return it.value();
        }

        // Last resort: return the key itself
        spdlog::debug("TranslationManager: Missing key '{}' for locale '{}'",
                       key.toStdString(), m_currentLocale.toStdString());
        return key;
    }

    /**
     * @brief Translate with parameter substitution.
     *        Replaces {0}, {1}, ... with provided arguments.
     *        QML usage:  translator.tp("welcome.greeting", [userName])
     */
    Q_INVOKABLE QString tp(const QString& key, const QVariantList& params) const {
        QString result = t(key);
        for (int i = 0; i < params.size(); ++i) {
            result.replace("{" + QString::number(i) + "}", params[i].toString());
        }
        return result;
    }

    /**
     * @brief Get the display name for a locale (e.g. "Nederlands (België)").
     */
    Q_INVOKABLE QString localeName(const QString& locale) const {
        static const QHash<QString, QString> names = {
            {"nl-BE", "Nederlands (België)"},
            {"en-GB", "English (UK)"},
            {"fr-BE", "Français (Belgique)"},
        };
        return names.value(locale, locale);
    }

    /**
     * @brief Get the flag emoji for a locale.
     */
    Q_INVOKABLE QString localeFlag(const QString& locale) const {
        static const QHash<QString, QString> flags = {
            {"nl-BE", "🇧🇪"},
            {"en-GB", "🇬🇧"},
            {"fr-BE", "🇫🇷"},
        };
        return flags.value(locale, "🏳️");
    }

signals:
    void localeChanged();

private:
    /**
     * @brief Flatten nested JSON into dot-separated keys.
     *        { "nav": { "dashboard": "Dashboard" } }  →  "nav.dashboard" = "Dashboard"
     */
    void flattenJson(const QJsonObject& obj, const QString& prefix,
                     QHash<QString, QString>& out) const
    {
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            QString fullKey = prefix.isEmpty() ? it.key() : prefix + "." + it.key();
            if (it.value().isObject()) {
                flattenJson(it.value().toObject(), fullKey, out);
            } else if (it.value().isString()) {
                out[fullKey] = it.value().toString();
            }
        }
    }

    QHash<QString, QHash<QString, QString>> m_bundles;
    QString m_currentLocale = "nl-BE";
    QStringList m_availableLocales;
};

} // namespace eduerp::services