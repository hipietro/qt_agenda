#ifndef COMMANDHISTORY_H
#define COMMANDHISTORY_H

#include "Command.h"

#include <QString>

#include <memory>
#include <vector>

/*
 * Gestisce gli stack undo/redo dell'applicazione.
 *
 * Ho scelto di separare questa classe dalla MainWindow perché la logica
 * undo/redo non deve dipendere dalla GUI. La finestra dovrà solo chiedere
 * alla history di eseguire, annullare o ripetere comandi.
 */
class CommandHistory
{
public:
    CommandHistory() = default;

    bool executeCommand(std::unique_ptr<Command> command);

    bool undo();
    bool redo();

    bool canUndo() const;
    bool canRedo() const;

    QString undoDescription() const;
    QString redoDescription() const;

    int undoCount() const;
    int redoCount() const;

    void clear();

private:
    std::vector<std::unique_ptr<Command>> m_undoStack;
    std::vector<std::unique_ptr<Command>> m_redoStack;
};

#endif