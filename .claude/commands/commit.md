---
description: Create a git commit from current changes without Co-Authored-By
---

Create a new git commit as follows:

1. Run `git status` and `git diff` in parallel to check current changes.
2. Infer commit message conventions from recent commits via `git log`.
3. Analyze changes and draft a commit message as a summary to human readers:
    - Mostly a single line. Use body only when a single line gives too little context.
    - No prefix such as `feat:` or `fix:`, but may sparingly use a topic prefix with colon if required.
    - Imperative mood or declarative present tense, ending with a period.
    - Reference specific identifiers (function names, opcode names, macro names, type names) when relevant.
    - Keep it concise, typically under 80 characters.
    - Feel free to use parenthetical remarks for informal commentary if relevant.
    - Remember: readers are NOT expected to parse detailed changes which can be inferred from the commit itself.
4. Stage all changes with `git add .` or selectively with `git add <file>`.
5. Write the commit message in the HEREDOC format, **without `Co-Authored-By` and Claude Code advertisements.**
6. Run `git commit` to create the commit.
7. Run `git status` to confirm the creation of a new commit.

Ideally each commit has to be self-contained. Multiple changes have to be split into multiple commits unless doing so temporarily regress the build or tests.

AGAIN: NEVER, EVER ADD `Co-Authored-By` OR ANY MENTION OF CLAUDE CODE IN THE COMMIT MESSAGE!
