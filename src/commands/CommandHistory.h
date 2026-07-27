// Undo/redo stack used by the main window.

#ifndef COMMANDHISTORY_H
#define COMMANDHISTORY_H

#include "Command.h"

#include <QString>

#include <memory>
#include <vector>

/*
 * Owns every executed command through the abstract Command interface.
 *
 * The history is intentionally independent from the GUI. It transfers each
 * unique command between the undo and redo stacks without copying it, and a
 * successful new command truncates the obsolete redo branch.
 */
class CommandHistory
{
public:
    CommandHistory() = default;
    ~CommandHistory() = default;

    CommandHistory(const CommandHistory&) = delete;
    CommandHistory& operator=(const CommandHistory&) = delete;
    CommandHistory(CommandHistory&&) = default;
    CommandHistory& operator=(CommandHistory&&) = default;

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
