#include <QApplication>
#include <QFile>
#include <QMainWindow>
#include <QTextEdit>
#include <QDateTime>

#include <memory>

#include "model/ActivityManager.h"
#include "model/SearchEngine.h"
#include "model/EventActivity.h"
#include "model/DeadlineActivity.h"
#include "model/ReminderActivity.h"
#include "model/ChecklistActivity.h"

static QString activityStatusToItalian(const Activity* activity, const QDateTime& now)
{
    if (!activity) {
        return "Non valida";
    }

    if (activity->isCompleted()) {
        return "Completata";
    }

    if (activity->isOverdue(now)) {
        return "Scaduta";
    }

    return "Attiva";
}

static QString matchedFieldToItalian(const QString& matchedField)
{
    if (matchedField == "title") {
        return "titolo";
    }

    if (matchedField == "category") {
        return "categoria";
    }

    if (matchedField == "description") {
        return "descrizione";
    }

    if (matchedField == "summary") {
        return "riepilogo";
    }

    if (matchedField == "all") {
        return "tutti i campi";
    }

    return "campo sconosciuto";
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }

    const QDateTime now = QDateTime::currentDateTime();

    ActivityManager manager;

    manager.addActivity(std::make_unique<EventActivity>(
        "Lezione di Programmazione a Oggetti",
        now.addDays(1),
        now.addDays(1).addSecs(7200),
        "Aula A",
        QStringList{"Docente", "Studenti"},
        "Discussione sul progetto Qt",
        "Università",
        Priority::High
    ));

    manager.addActivity(std::make_unique<DeadlineActivity>(
        "Consegnare il progetto Qt",
        now.addDays(14),
        "Esame di Programmazione",
        true,
        "Consegna finale su Moodle",
        "Università",
        Priority::Critical
    ));

    manager.addActivity(std::make_unique<ReminderActivity>(
        "Chiamare il medico",
        now.addSecs(3600),
        15,
        "Chiedere conferma dell'appuntamento",
        "Promemoria personale",
        "Salute",
        Priority::Medium
    ));

    auto checklist = std::make_unique<ChecklistActivity>(
        "Preparare sessione di studio",
        now.addDays(3),
        QVector<ChecklistItem>{},
        "Preparare il materiale prima di studiare",
        "Università",
        Priority::Medium
    );

    checklist->addItem("Ripassare la teoria");
    checklist->addItem("Svolgere esercizi");
    checklist->addItem("Scrivere appunti riassuntivi");

    manager.addActivity(std::move(checklist));

    QString output;
    output += "Agenda Qt - Ricerca avanzata inizializzata\n\n";
    output += QString("Attività salvate: %1\n\n").arg(manager.size());

    output += "Tutte le attività:\n\n";

    for (const Activity* activity : manager.activities()) {
        output += QString("- %1 | Categoria: %2 | Stato: %3 | Data principale: %4\n")
                .arg(activity->title())
                .arg(activity->category())
                .arg(activityStatusToItalian(activity, now))
                .arg(activity->primaryDate().toString("dd/MM/yyyy HH:mm"));
    }

    output += "\n----------------------------------------\n\n";

    const QString normalQuery = "qt";
    const SearchEngine::SearchResponse normalSearch =
        SearchEngine::search(manager.activities(), normalQuery);

    output += QString("Ricerca: \"%1\"\n").arg(normalQuery);
    output += QString("Risultati diretti: %1\n\n").arg(static_cast<int>(normalSearch.results.size()));

    for (const SearchEngine::SearchResult& result : normalSearch.results) {
        output += QString("- %1 [campo trovato: %2, punteggio: %3]\n")
                .arg(result.activity->title())
                .arg(matchedFieldToItalian(result.matchedField))
                .arg(result.score);
    }

    output += "\n----------------------------------------\n\n";

    const QString typoQuery = "progeto";
    const SearchEngine::SearchResponse typoSearch =
        SearchEngine::search(manager.activities(), typoQuery);

    output += QString("Ricerca con errore di battitura: \"%1\"\n").arg(typoQuery);
    output += QString("Risultati diretti: %1\n").arg(static_cast<int>(typoSearch.results.size()));

    if (!typoSearch.hasResults() && typoSearch.suggestion.isValid()) {
        output += QString("Nessun risultato diretto. Forse cercavi: %1? ")
                .arg(typoSearch.suggestion.suggestedText);
        output += QString("(testo confrontato: %1, distanza: %2)\n")
                .arg(typoSearch.suggestion.matchedText)
                .arg(typoSearch.suggestion.distance);
    } else if (!typoSearch.hasResults()) {
        output += "Nessun risultato diretto e nessun suggerimento affidabile trovato.\n";
    }

    QMainWindow window;
    window.setWindowTitle("Agenda Qt");
    window.resize(900, 600);

    QTextEdit *textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setText(output);

    window.setCentralWidget(textEdit);
    window.show();

    return app.exec();
}