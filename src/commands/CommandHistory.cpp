#include "CommandHistory.h"

bool CommandHistory::executeCommand(std::unique_ptr<Command> command)
{
    if (!command) {
        return false;
    }

    /*
     * Se viene eseguita una nuova operazione dopo uno o più undo,
     * la cronologia redo non è più valida e va eliminata.
     */
    if (!command->execute()) {
        return false;
    }

    m_undoStack.push_back(std::move(command));
    m_redoStack.clear();

    return true;
}

bool CommandHistory::undo()
{
    if (m_undoStack.empty()) {
        return false;
    }

    std::unique_ptr<Command> command = std::move(m_undoStack.back());
    m_undoStack.pop_back();

    if (!command->undo()) {
        /*
         * Se l'undo fallisce, rimetto il comando nello stack originale.
         * Così la history non perde informazioni.
         */
        m_undoStack.push_back(std::move(command));
        return false;
    }

    m_redoStack.push_back(std::move(command));
    return true;
}

bool CommandHistory::redo()
{
    if (m_redoStack.empty()) {
        return false;
    }

    std::unique_ptr<Command> command = std::move(m_redoStack.back());
    m_redoStack.pop_back();

    if (!command->execute()) {
        /*
         * Se il redo fallisce, rimetto il comando nello stack redo.
         * In questo modo la history resta coerente.
         */
        m_redoStack.push_back(std::move(command));
        return false;
    }

    m_undoStack.push_back(std::move(command));
    return true;
}

bool CommandHistory::canUndo() const
{
    return !m_undoStack.empty();
}

bool CommandHistory::canRedo() const
{
    return !m_redoStack.empty();
}

QString CommandHistory::undoDescription() const
{
    if (!canUndo()) {
        return "Undo";
    }

    return QString("Undo: %1").arg(m_undoStack.back()->description());
}

QString CommandHistory::redoDescription() const
{
    if (!canRedo()) {
        return "Redo";
    }

    return QString("Redo: %1").arg(m_redoStack.back()->description());
}

int CommandHistory::undoCount() const
{
    return static_cast<int>(m_undoStack.size());
}

int CommandHistory::redoCount() const
{
    return static_cast<int>(m_redoStack.size());
}

void CommandHistory::clear()
{
    m_undoStack.clear();
    m_redoStack.clear();
}