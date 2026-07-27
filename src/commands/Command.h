// Base interface for undoable commands.

#ifndef COMMAND_H
#define COMMAND_H

#include <QString>

/*
 * Interfaccia base per il sistema undo/redo.
 *
 * Ogni comando rappresenta una modifica del modello con comportamento
 * dinamico diverso sia in esecuzione sia in annullamento. La history usa
 * esclusivamente questa interfaccia astratta e non conosce i tipi concreti.
 */
class Command
{
public:
    virtual ~Command() = default;

    // Executes or re-executes the concrete operation.
    virtual bool execute() = 0;

    // Restores the exact state that preceded the concrete operation.
    virtual bool undo() = 0;

    // Generic operation label, useful for logs and diagnostics.
    virtual QString description() const = 0;

    /*
     * Contextual labels are intentionally virtual: adding, removing,
     * replacing and toggling an activity require different user-facing
     * explanations when they are undone or redone.
     */
    virtual QString undoDescription() const = 0;
    virtual QString redoDescription() const = 0;
};

#endif
