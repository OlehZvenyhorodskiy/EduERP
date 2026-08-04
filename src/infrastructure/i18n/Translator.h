#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <spdlog/spdlog.h>

namespace eduerp::infra {

/**
 * @brief Runtime i18n translator — loads JSON translation files and exposes
 *        a `t(key)` function to QML. Supports dot-notation keys like
 *        "auth.login_google" and simple {placeholder} substitution.
 *
 * Supported locales: nl-BE (default), en-GB, fr-BE.
 * The active locale is set by the school's default_language from the server
 * or overridden by the user in Settings.
 *
 * Usage in QML:
 *   Text { text: translator.t("auth.login_google") }
 *   Text { text: translator.t("dashboard.welcome", {"name": userName}) }
 */
class Translator : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString locale READ locale WRITE setLocale NOTIFY localeChanged)
    Q_PROPERTY(QString currentLocale READ locale NOTIFY localeChanged)

public:
    explicit Translator(QObject* parent = nullptr)
        : QObject(parent)
    {
        // Load default locale on construction
        loadLocale(m_locale);
    }

    // --- QML singleton factory ---
    static Translator* create(QQmlEngine*, QJSEngine*) {
        return new Translator();
    }

    // -----------------------------------------------------------------------
    // Locale management
    // -----------------------------------------------------------------------

    QString locale() const { return m_locale; }

    /**
     * @brief Switch the active locale and reload translations.
     *        Triggers a localeChanged signal so all QML bindings refresh.
     */
    void setLocale(const QString& locale) {
        if (m_locale == locale) return;

        if (!m_supportedLocales.contains(locale)) {
            spdlog::warn("Translator: unsupported locale '{}', ignoring", locale.toStdString());
            return;
        }

        if (loadLocale(locale)) {
            m_locale = locale;
            spdlog::info("Translator: switched to locale '{}'", locale.toStdString());
            emit localeChanged();
        }
    }

    /** @brief Returns the list of supported locale codes. */
    Q_INVOKABLE QStringList supportedLocales() const { return m_supportedLocales; }

    /** @brief Returns the human-readable name for a locale code. */
    Q_INVOKABLE QString localeName(const QString& locale) const {
        if (locale == "nl-BE") return "Nederlands (België)";
        if (locale == "en-GB") return "English (UK)";
        if (locale == "fr-BE") return "Français (Belgique)";
        return locale;
    }

    // -----------------------------------------------------------------------
    // Translation lookup
    // -----------------------------------------------------------------------

    /**
     * @brief Translate a dot-notation key, e.g. "auth.login_google".
     *        Falls back to the key itself if not found.
     *
     * @param key  Dot-notation translation key.
     * @return     Translated string, or key if missing.
     */
    Q_INVOKABLE QString t(const QString& key) const {
        QString result = resolveKey(key);
        if (result.isEmpty()) {
            spdlog::debug("Translator: missing key '{}' in locale '{}'",
                          key.toStdString(), m_locale.toStdString());
            return key; // Transparent fallback — shows the raw key
        }
        return result;
    }

    /**
     * @brief Translate a key with named placeholder substitution.
     *        Replaces {name} patterns with values from placeholders map.
     *
     * @param key           Dot-notation translation key.
     * @param placeholders  QVariantMap of { "name": value } pairs.
     * @return              Translated string with placeholders substituted.
     *
     * QML example:
     *   translator.t("dashboard.welcome", {"name": "Alice"})
     *   // → "Welkom, Alice!" in nl-BE
     */
    Q_INVOKABLE QString t(const QString& key, const QVariantMap& placeholders) const {
        QString result = t(key);

        // Substitute each {placeholder}
        for (auto it = placeholders.cbegin(); it != placeholders.cend(); ++it) {
            result.replace(QStringLiteral("{") + it.key() + QStringLiteral("}"),
                           it.value().toString());
        }

        return result;
    }

    /**
     * @brief Translate a key with positional placeholder substitution.
     *        Replaces {0}, {1}, {2}... with values from the args list.
     *
     * QML usage: translator.tp("dashboard.welcome", ["Jan"])
     */
    Q_INVOKABLE QString tp(const QString& key, const QVariantList& args) const {
        QString result = t(key);

        for (int i = 0; i < args.size(); ++i) {
            result.replace(QStringLiteral("{") + QString::number(i) + QStringLiteral("}"),
                           args.at(i).toString());
        }

        return result;
    }

signals:
    /** Emitted when the active locale changes — causes all t() bindings to refresh. */
    void localeChanged();

private:
    // -----------------------------------------------------------------------
    // Internal helpers
    // -----------------------------------------------------------------------

    /**
     * @brief Load a JSON translation file from the translations directory.
     *        Path: :/i18n/translations/<locale>.json (Qt resource) or
     *              <appDir>/i18n/translations/<locale>.json (file system fallback).
     */
    bool loadLocale(const QString& locale) {
        // Try Qt resource system first (for bundled translations)
        QString resourcePath = QStringLiteral(":/i18n/translations/") + locale + ".json";
        QFile file(resourcePath);

        // Fall back to filesystem (useful during development)
        if (!file.exists()) {
            QString fsPath = QStringLiteral("./src/infrastructure/i18n/translations/") + locale + ".json";
            file.setFileName(fsPath);
        }

        if (!file.open(QIODevice::ReadOnly)) {
            spdlog::error("Translator: cannot open translation file for locale '{}': {}",
                          locale.toStdString(),
                          file.errorString().toStdString());
            return false;
        }

        QJsonParseError parseError;
        auto doc = QJsonDocument::fromJson(file.readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            spdlog::error("Translator: JSON parse error in '{}': {}",
                          locale.toStdString(),
                          parseError.errorString().toStdString());
            return false;
        }

        m_translations = doc.object();
        spdlog::info("Translator: loaded {} top-level keys from locale '{}'",
                     m_translations.size(), locale.toStdString());
        return true;
    }

    /**
     * @brief Resolve a dot-notation key against the loaded translations object.
     *        Traverses nested objects: "simulation.modules.finance" →
     *        translations["simulation"]["modules"]["finance"].
     */
    QString resolveKey(const QString& key) const {
        QStringList parts = key.split('.');
        QJsonValue current = m_translations;

        for (const QString& part : parts) {
            if (!current.isObject()) return {};
            current = current.toObject().value(part);
            if (current.isUndefined()) return {};
        }

        if (current.isString()) return current.toString();
        return {};
    }

    // -----------------------------------------------------------------------
    // State
    // -----------------------------------------------------------------------

    QString m_locale = "nl-BE";
    QJsonObject m_translations;
    const QStringList m_supportedLocales = {"nl-BE", "en-GB", "fr-BE"};
};

} // namespace eduerp::infra