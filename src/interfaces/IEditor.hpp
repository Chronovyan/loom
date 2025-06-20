#pragma once

#include <string>
#include <vector>
#include <memory>
#include "ITextBuffer.hpp"
#include "../TextBuffer.h"
#include "../SyntaxHighlighter.h"
#include "ICommandManager.hpp"
#include "IDiffEngine.hpp"
#include "IMergeEngine.hpp"
#include "IMultiCursor.hpp"

// Forward declarations
class SyntaxHighlighter;
struct SyntaxStyle;

// Selection unit for selection operations
enum class SelectionUnit {
    Character,
    Word,
    Expression,
    Line, 
    Paragraph,
    Block,
    Document
};

/**
 * @interface IEditor
 * @brief Interface for the editor component
 * 
 * This interface defines the contract for the main editor component,
 * providing methods for file operations, cursor management, text editing,
 * selection, clipboard operations, and other core editor functionality.
 */
class IEditor {
public:
    /**
     * @brief Virtual destructor for proper cleanup in derived classes
     */
    virtual ~IEditor() = default;
    
    // File Operations
    /**
     * @brief Open a file for editing
     * 
     * @param filename Path to the file to open
     * @return True if the file was opened successfully
     */
    virtual bool openFile(const std::string& filename) = 0;
    
    /**
     * @brief Save the current file
     * 
     * @return True if the file was saved successfully
     */
    virtual bool saveFile() = 0;
    
    /**
     * @brief Save the current file with a different name
     * 
     * @param filename Path to save the file to
     * @return True if the file was saved successfully
     */
    virtual bool saveFileAs(const std::string& filename) = 0;
    
    /**
     * @brief Check if the current file has unsaved changes
     * 
     * @return True if the file has been modified since the last save
     */
    virtual bool isModified() const = 0;
    
    /**
     * @brief Set the modification state of the current file
     * 
     * @param modified True to mark the file as modified, false otherwise
     */
    virtual void setModified(bool modified) = 0;
    
    // Cursor Management
    /**
     * @brief Set the cursor position
     * 
     * @param line Line number (0-based)
     * @param col Column number (0-based)
     */
    virtual void setCursor(size_t line, size_t col) = 0;
    
    /**
     * @brief Get the current cursor line
     * 
     * @return The line number (0-based)
     */
    virtual size_t getCursorLine() const = 0;
    
    /**
     * @brief Get the current cursor column
     * 
     * @return The column number (0-based)
     */
    virtual size_t getCursorCol() const = 0;
    
    // Cursor Movement
    /**
     * @brief Move the cursor up one line
     */
    virtual void moveCursorUp() = 0;
    
    /**
     * @brief Move the cursor down one line
     */
    virtual void moveCursorDown() = 0;
    
    /**
     * @brief Move the cursor left one character
     */
    virtual void moveCursorLeft() = 0;
    
    /**
     * @brief Move the cursor right one character
     */
    virtual void moveCursorRight() = 0;
    
    /**
     * @brief Move the cursor to the start of the current line
     */
    virtual void moveCursorToLineStart() = 0;
    
    /**
     * @brief Move the cursor to the end of the current line
     */
    virtual void moveCursorToLineEnd() = 0;
    
    /**
     * @brief Move the cursor to the start of the buffer
     */
    virtual void moveCursorToBufferStart() = 0;
    
    /**
     * @brief Move the cursor to the end of the buffer
     */
    virtual void moveCursorToBufferEnd() = 0;
    
    // Buffer Access
    /**
     * @brief Get the text buffer
     * 
     * @return Reference to the text buffer
     */
    virtual ITextBuffer& getBuffer() = 0;
    
    /**
     * @brief Get the text buffer (const version)
     * 
     * @return Const reference to the text buffer
     */
    virtual const ITextBuffer& getBuffer() const = 0;
    
    // Text Editing
    /**
     * @brief Add a line to the end of the buffer
     * 
     * @param text The text to add
     */
    virtual void addLine(const std::string& text) = 0;
    
    /**
     * @brief Insert a line at the specified index
     * 
     * @param lineIndex The index to insert at
     * @param text The text to insert
     */
    virtual void insertLine(size_t lineIndex, const std::string& text) = 0;
    
    /**
     * @brief Delete a line at the specified index
     * 
     * @param lineIndex The index to delete
     */
    virtual void deleteLine(size_t lineIndex) = 0;
    
    /**
     * @brief Replace a line at the specified index
     * 
     * @param lineIndex The index to replace
     * @param text The new text
     */
    virtual void replaceLine(size_t lineIndex, const std::string& text) = 0;
    
    /**
     * @brief Insert text at the current cursor position
     * 
     * @param textToInsert The text to insert
     */
    virtual void typeText(const std::string& textToInsert) = 0;
    
    /**
     * @brief Insert a character at the current cursor position
     * 
     * @param charToInsert The character to insert
     */
    virtual void typeChar(char charToInsert) = 0;
    
    /**
     * @brief Process a character input (typically from keyboard)
     * 
     * @param ch The character to process
     */
    virtual void processCharacterInput(char ch) = 0;
    
    /**
     * @brief Delete the current selection
     */
    virtual void deleteSelection() = 0;
    
    /**
     * @brief Delete the character before the cursor
     */
    virtual void backspace() = 0;
    
    /**
     * @brief Delete the character after the cursor
     */
    virtual void deleteForward() = 0;
    
    /**
     * @brief Insert a new line at the cursor position
     */
    virtual void newLine() = 0;
    
    /**
     * @brief Join the current line with the next line
     */
    virtual void joinWithNextLine() = 0;
    
    // Indentation
    /**
     * @brief Increase the indentation of the current line or selection
     */
    virtual void increaseIndent() = 0;
    
    /**
     * @brief Decrease the indentation of the current line or selection
     */
    virtual void decreaseIndent() = 0;
    
    // Undo/Redo
    /**
     * @brief Check if undo is available
     * 
     * @return True if there are operations to undo
     */
    virtual bool canUndo() const = 0;
    
    /**
     * @brief Check if redo is available
     * 
     * @return True if there are operations to redo
     */
    virtual bool canRedo() const = 0;
    
    /**
     * @brief Undo the last operation
     * 
     * @return True if an operation was undone
     */
    virtual bool undo() = 0;
    
    /**
     * @brief Redo the last undone operation
     * 
     * @return True if an operation was redone
     */
    virtual bool redo() = 0;
    
    // Selection
    /**
     * @brief Check if there is an active selection
     * 
     * @return True if there is a selection
     */
    virtual bool hasSelection() const = 0;
    
    /**
     * @brief Clear the current selection
     */
    virtual void clearSelection() = 0;
    
    /**
     * @brief Set the selection range
     * 
     * @param startLine Start line
     * @param startCol Start column
     * @param endLine End line
     * @param endCol End column
     */
    virtual void setSelectionRange(size_t startLine, size_t startCol, size_t endLine, size_t endCol) = 0;
    
    /**
     * @brief Get the text of the current selection
     * 
     * @return The selected text
     */
    virtual std::string getSelectedText() const = 0;
    
    /**
     * @brief Start a selection at the current cursor position
     */
    virtual void startSelection() = 0;
    
    /**
     * @brief Update the selection to the current cursor position
     */
    virtual void updateSelection() = 0;
    
    /**
     * @brief Replace the current selection with the specified text
     * 
     * @param text The text to replace with
     */
    virtual void replaceSelection(const std::string& text) = 0;
    
    /**
     * @brief Select the entire current line
     */
    virtual void selectLine() = 0;
    
    /**
     * @brief Select all text in the buffer
     */
    virtual void selectAll() = 0;
    
    /**
     * @brief Shrink the current selection according to the specified unit
     * 
     * @param targetUnit The unit to shrink by (e.g., Word, Line)
     */
    virtual void shrinkSelection(SelectionUnit targetUnit = SelectionUnit::Word) = 0;
    
    // Clipboard
    /**
     * @brief Cut the current selection to the clipboard
     */
    virtual void cutSelection() = 0;
    
    /**
     * @brief Copy the current selection to the clipboard
     */
    virtual void copySelection() = 0;
    
    /**
     * @brief Paste text from the clipboard at the cursor position
     */
    virtual void pasteAtCursor() = 0;
    
    /**
     * @brief Get the current clipboard text
     * 
     * @return The clipboard text
     */
    virtual std::string getClipboardText() const = 0;
    
    /**
     * @brief Set the clipboard text
     * 
     * @param text The text to set
     */
    virtual void setClipboardText(const std::string& text) = 0;
    
    // Search
    /**
     * @brief Search for text in the buffer
     * 
     * @param searchTerm The text to search for
     * @param caseSensitive Whether the search is case sensitive
     * @param forward Whether to search forward (true) or backward (false)
     * @return True if a match was found
     */
    virtual bool search(const std::string& searchTerm, bool caseSensitive = true, bool forward = true) = 0;
    
    /**
     * @brief Search for the next occurrence of the current search term
     * 
     * @return True if a match was found
     */
    virtual bool searchNext() = 0;
    
    /**
     * @brief Search for the previous occurrence of the current search term
     * 
     * @return True if a match was found
     */
    virtual bool searchPrevious() = 0;
    
    /**
     * @brief Replace the current search match
     * 
     * @param searchTerm The text to search for
     * @param replacementText The text to replace with
     * @param caseSensitive Whether the search is case sensitive
     * @return True if a replacement was made
     */
    virtual bool replace(const std::string& searchTerm, const std::string& replacementText, bool caseSensitive = true) = 0;
    
    /**
     * @brief Replace all occurrences of the search term
     * 
     * @param searchTerm The text to search for
     * @param replacementText The text to replace with
     * @param caseSensitive Whether the search is case sensitive
     * @return True if any replacements were made
     */
    virtual bool replaceAll(const std::string& searchTerm, const std::string& replacementText, bool caseSensitive = true) = 0;
    
    // Diff and Merge Operations
    /**
     * @brief Compare two texts and show differences
     * 
     * @param text1 First text to compare
     * @param text2 Second text to compare
     * @return True if the diff was successfully created and displayed
     */
    virtual bool showDiff(const std::vector<std::string>& text1, const std::vector<std::string>& text2) = 0;
    
    /**
     * @brief Compare the current buffer with another text and show differences
     * 
     * @param otherText Text to compare with the current buffer
     * @return True if the diff was successfully created and displayed
     */
    virtual bool diffWithCurrent(const std::vector<std::string>& otherText) = 0;
    
    /**
     * @brief Compare with a file and show differences
     * 
     * @param filename Path to the file to compare with
     * @return True if the diff was successfully created and displayed
     */
    virtual bool diffWithFile(const std::string& filename) = 0;
    
    /**
     * @brief Perform a three-way merge
     * 
     * @param base Base version (common ancestor)
     * @param ours Our version
     * @param theirs Their version
     * @return True if the merge was successful
     */
    virtual bool mergeTexts(
        const std::vector<std::string>& base,
        const std::vector<std::string>& ours,
        const std::vector<std::string>& theirs) = 0;
    
    /**
     * @brief Merge current buffer with another file
     * 
     * @param theirFile Path to their file
     * @param baseFile Path to the base file (common ancestor)
     * @return True if the merge was successful
     */
    virtual bool mergeWithFile(const std::string& theirFile, const std::string& baseFile) = 0;
    
    /**
     * @brief Apply changes from a diff
     * 
     * @param changes The diff changes to apply
     * @param sourceText The source text where the changes come from
     * @return True if the changes were successfully applied
     */
    virtual bool applyDiffChanges(
        const std::vector<DiffChange>& changes,
        const std::vector<std::string>& sourceText) = 0;
    
    /**
     * @brief Resolve a merge conflict
     * 
     * @param conflictIndex Index of the conflict to resolve
     * @param resolution Resolution strategy to apply
     * @param customResolution Custom resolution lines (used if resolution is CUSTOM)
     * @return True if the conflict was resolved successfully
     */
    virtual bool resolveConflict(
        size_t conflictIndex,
        MergeConflictResolution resolution,
        const std::vector<std::string>& customResolution = {}) = 0;
    
    // Syntax Highlighting
    /**
     * @brief Enable or disable syntax highlighting
     * 
     * @param enable True to enable, false to disable
     */
    virtual void enableSyntaxHighlighting(bool enable = true) = 0;
    
    /**
     * @brief Check if syntax highlighting is enabled
     * 
     * @return True if syntax highlighting is enabled
     */
    virtual bool isSyntaxHighlightingEnabled() const = 0;
    
    /**
     * @brief Set the filename for syntax highlighting purposes
     * 
     * @param filename The filename
     */
    virtual void setFilename(const std::string& filename) = 0;
    
    /**
     * @brief Get the current filename
     * 
     * @return The filename
     */
    virtual std::string getFilename() const = 0;
    
    /**
     * @brief Get the current syntax highlighter
     * 
     * @return Shared pointer to the current syntax highlighter
     */
    virtual std::shared_ptr<SyntaxHighlighter> getCurrentHighlighter() const = 0;
    
    /**
     * @brief Get the syntax highlighting styles for the visible buffer
     * 
     * @return Vector of syntax highlighting styles
     */
    virtual std::vector<std::vector<SyntaxStyle>> getHighlightingStyles() const = 0;

    // Multiple cursor operations
    /**
     * @brief Check if multiple cursor mode is enabled
     * 
     * @return True if multiple cursor mode is enabled
     */
    virtual bool isMultiCursorEnabled() const = 0;
    
    /**
     * @brief Enable or disable multiple cursor mode
     * 
     * @param enable True to enable, false to disable
     */
    virtual void setMultiCursorEnabled(bool enable) = 0;
    
    /**
     * @brief Get the number of active cursors
     * 
     * @return The number of cursors
     */
    virtual size_t getCursorCount() const = 0;
    
    /**
     * @brief Add a new cursor at the specified position
     * 
     * @param line Line number (0-based)
     * @param col Column number (0-based)
     * @return True if the cursor was added successfully
     */
    virtual bool addCursor(size_t line, size_t col) = 0;
    
    /**
     * @brief Remove a cursor at the specified position
     * 
     * @param line Line number (0-based)
     * @param col Column number (0-based)
     * @return True if a cursor was removed
     */
    virtual bool removeCursor(size_t line, size_t col) = 0;
    
    /**
     * @brief Remove all secondary cursors
     */
    virtual void removeAllSecondaryCursors() = 0;
    
    /**
     * @brief Add cursors at all occurrences of the specified text
     * 
     * @param text Text to search for
     * @param caseSensitive Whether the search should be case-sensitive
     * @return Number of new cursors added
     */
    virtual size_t addCursorsAtAllOccurrences(const std::string& text, bool caseSensitive = true) = 0;
    
    /**
     * @brief Add cursors at the same column position on multiple lines
     * 
     * @param startLine Start line for column selection
     * @param endLine End line for column selection
     * @param column Column position for all cursors
     * @return Number of new cursors added
     */
    virtual size_t addCursorsAtColumn(size_t startLine, size_t endLine, size_t column) = 0;
    
    /**
     * @brief Get the multi-cursor manager
     * 
     * @return Reference to the multi-cursor manager
     */
    virtual IMultiCursor& getMultiCursor() = 0;
    
    /**
     * @brief Get the multi-cursor manager (const version)
     * 
     * @return Const reference to the multi-cursor manager
     */
    virtual const IMultiCursor& getMultiCursor() const = 0;
}; 