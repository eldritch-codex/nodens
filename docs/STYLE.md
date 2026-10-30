# Writing style

Every page under `docs/` follows this standard, so a reader meets the same shape and the same words whether they arrive from a tutorial link or from the repository's front page. Read it before writing a page or rewriting one.

## Two audiences, two voices

| Folder | Reader | Voice |
|---|---|---|
| `docs/guide/` | someone building an application on Nodens | teaching: the problem, the mechanism, the remedy, a worked case |
| `docs/dev/` | someone changing the framework | design note: the mechanism or the decision, then its rationale and the trade it makes |
| `docs/` root | someone making or releasing a change | process: the rule, then the steps that follow it |

The voice differs because the reader's question differs. The words do not: both folders use the glossary below.

## The glossary

Two tables. The first says what the shared words mean, which gives the second something to point at when it answers "what do I type instead of this word". Any other term is defined at its first use on the page that needs it.

### The words in use

| Word | Means |
|---|---|
| iteration | one pass of `Application::Run` |
| step | one simulation advance: one `OnUpdate` call per layer |
| frame | the image the display shows, and the rate it is shown at |
| overlay | the ImGui layer that draws last and sees input first |
| headless | no window and no overlay, with the loop still running |

### Write this, not that

| Do not write | Write | Because |
|---|---|---|
| frame, for one pass of the loop | iteration | A frame is the image the display shows; an iteration is the pass that may produce one |
| frame, for one simulation advance | step | An iteration can run several steps, or none at all |
| tick | step | Two words for one thing, and the code says step |
| update, as a noun | step | `OnUpdate` is the call that runs per step |
| GUI layer, where the type is meant | overlay, or `ImGuiLayer` | The type is `ImGuiLayer`; overlay is the role it plays in the stack |
| server mode, offscreen | headless | Headless means no window and no overlay, not a machine elsewhere |

A page that needs a metaphor states what maps to what and where the mapping stops, and keeps it to the page that explains it.

### Defining terms on a page

Every term is defined in the sentence that first uses it, whichever table it came from. A universal word still gets one line of explanation where it first appears, because a reader arrives mid-page.

A page may also carry a table of the terms it introduces, above the section that first needs them, under two conditions: it introduces three or more terms that are not in the glossary above, and each of them is used more than once. Those terms stay on that page and never enter the universal table, which is reserved for words every page shares.

The table may keep the page's own name for it. The loop page is the model: its four pacing settings sit in a table called "The four settings", because that is what they are, and its other local words, `ceiling`, `clamp` and `accumulator`, are defined in the sentence that first uses each and never enter the universal table.

## Concept pages

The pages that explain how a part of Nodens works share one skeleton, in this order:

1. **Problem.** The naive approach and what it costs. A page with no problem is a reference page, and this skeleton does not apply to it.
2. **How it works.** The mechanism in one pass, with each term defined as it appears.
3. **What still goes wrong.** The failure modes, in the order a reader will meet them.
4. **Remedies.** One per failure mode, in the same order, so the reader can pair them.
5. **Choosing.** When to use this instead of the alternative, and why.
6. **Worked case.** Small numbers, a trace or a table. Concrete before general.
7. **References.** Markdown footnotes, full entry at the first use, short entry after. A page with no outside source has no references section.

Above them, a mental model callout: three or four sentences a reader can hold while reading the page. Beside them, one table, because narrative explains and a table locates.

## Overview pages

A page that answers "what are the parts, and how do they relate" is a map, not a mechanism. Forcing a problem and its remedies onto it produces filler, so an overview page has its own shape: the parts and what owns what, one iteration through them, how messages travel, which thread does what, what the platform hides, and where to read next.

`Architecture Overview` is the only page of this shape. A page that explains one part in depth is a concept page.

## Task pages

The pages that answer "how do I run this" have a different shape: prerequisites, numbered steps, troubleshooting. Each command is one line and can be copied as written. Each step says what success looks like, so a reader knows whether to continue.

## Process pages

The pages that state a rule the repository follows have a fourth shape: the rule, what enforcing it looks like, the steps for whoever makes that change, and the slips that break it. `Branching and releases` and `Committing` are the two process pages.

## Voice

- Present tense, and declarative sentences for rules.
- Third person for the mechanism; "you" only inside the steps of a task page.
- One idea per sentence, one point per paragraph.
- No adjective that cannot be checked. A renderer is not powerful, and a loop is not smooth.
- Define a term at its first use, then use it unchanged.
- Headings name the content: "The accumulator", not "Now let's look at what happens next".
- No rhetorical questions, and no "let's".

## Which shape each page takes

| Page | Shape |
|---|---|
| [Guide index](./guide/README.md) | index: mental model and reading order |
| [Getting Started](./guide/getting-started.md) | task |
| [Architecture Overview](./guide/architecture-overview.md) | overview |
| [Application Loop](./guide/application-loop.md) | concept |
| [Layer System](./guide/layer-system.md) | concept |
| [Event System](./guide/event-system.md) | concept |
| [Concurrency and TaskSystem](./guide/concurrency-and-tasksystem.md) | concept |
| [Creating a Custom Layer](./guide/creating-a-custom-layer.md) | concept, with the walkthrough as its worked case |
| [Using ImGui and ImPlot](./guide/using-imgui-and-implot.md) | concept |
| [Building and Toolchain](./guide/building-and-toolchain.md) | task |
| [Profiling with Tracy](./guide/profiling-with-tracy.md) | task |
| [Examples Guide](./guide/examples-guide.md) | concept, one section per example |
| [Docs index](./README.md) | index |
| [Branching and releases](./BRANCHING.md) | process |
| [Committing](./COMMITTING.md) | process |
| [`dev/` pages](./dev/naming.md) | design note: the mechanism or the decision first, then its rationale and the trade it makes |

## Before opening a pull request

- The page takes the shape its row above gives it.
- Every glossary word is used in the stated sense.
- Every term is defined at its first use.
- Code samples compile as written, and commands name options and targets that exist.
- A concept page has its lookup table and its references.
- `scripts/check-docs-links.sh` passes.
