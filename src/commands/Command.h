// Base interface for undoable commands.

#ifndef COMMAND_H
#define COMMAND_H

#include <QString>

/*
 * Interfaccia base per il sistema undo/redo.
 *
 * Ho scelto il Command pattern perché ogni modifica dell'agenda può essere
 * rappresentata come un'azione eseguibile e annullabile.
 * In questo modo MainWindow non dovrà conoscere i dettagli di undo/redo
 * per ogni singola operazione.
 */
class Command
{
public:
    virtual ~Command() = default;

    /*
     * Esegue l'azione.
     * Ritorna true se l'operazione è riuscita, false altrimenti.
     */
    virtual bool execute() = 0;

    /*
     * Annulla l'azione precedentemente eseguita.
     * Ritorna true se l'annullamento è riuscito, false altrimenti.
     */
    virtual bool undo() = 0;

    /*
     * Testo breve utile per debug, log o messaggi futuri nella GUI.
     */
    virtual QString description() const = 0;
};

#endif