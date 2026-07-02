# Library System — Notes Part 3 (Users & Borrowing)

Notes for the borrowing part. Kept lighter this time, as I asked — the ideas, not
every detail.

---

## The requirement
Admin-side app. Admin can add users; a user can borrow a book and return it. A
borrow has a start date, a supposed return date, and a fee that can be fully
paid, partially paid, or unpaid. Can't borrow a book with quantity 0.

---

## Key design decision: the loan is its own entity (a join entity)
A user borrows many books; a book is borrowed by many users — the relationship is
**many-to-many**. So instead of giving each user an array of borrowed books, I
made `Borrowed_Book` a **separate object** holding `user_id` and `book_id`.

In DB terms this is a **junction / join entity**: it owns the facts that belong to
neither side alone (dates, fees, status). Cleaner even without a database.

## The loan references IDs, not pointers
`Borrowed_Book` stores `user_id` and `book_id`, not `User*` / `Book*`. Reason:
my IDs are **stable and never reused**, so an ID is a permanent anchor. Even after
a book is soft-deleted, the loan still correctly says "book 5 was borrowed" and
nothing dangles. Pointers could go stale; IDs can't. (Same principle as
"an ID is an identity, not a count" from Part 1.)

## Return = close the record, don't delete it
`user_return_book` doesn't destroy the loan — it sets `is_finished = true` and
stamps `real_return_date`. Same soft-delete instinct as books: a loan is a
historical fact, and I keep it because the data is valuable (most-borrowed books,
who returns on time, monthly revenue). Never delete a fact; close it.

---

## Single-door creation (answers "a constructor can't return false")
A constructor can't reject bad input by returning false. My fix: the loan is
**only ever created inside `user_borrow_book`** in the library. That function is
the single door — it validates everything (book exists, quantity > 0, fees valid)
*before* constructing. By the time the constructor runs, inputs are known-good.
The constructor's precondition is explicit, and one function guarantees it.

### Why this also fixes coordination
Creating a loan and doing `quantity--` are **one atomic fact** ("a copy left the
shelf"). Both belong in `user_borrow_book`, not split across places, so they
can't drift out of sync. Same for return: flip to finished + `quantity++`
together. The coordinating function owns the pair.

---

## Fees (kept simple in the end)
One accumulator, `increment_what_is_paid(money)`:
- rejects if already paid, if money < 0, or if it would overshoot `fees`
- adds the money, and auto-marks `is_fully_paid` when the total reaches `fees`
- plus `pay_full_fees()` as an admin shortcut

I dropped the separate "set what_is_paid" function — it let the admin *lower* an
already-recorded payment, which is a footgun and doesn't match how paying works.
Accumulate-only models reality.

### Float and money
Money comparisons can't rely on float `==` (`0.1 + 0.2 != 0.3`), so I check
"close enough" with `std::fabs(fees - what_is_paid) < 0.001` instead of `==`. The
real fix in banking is **store money as integer cents** so equality is exact —
right answer if this ever became real, over-engineering for a practice app.

---

## The big insight: stored vs derived state (the ledger idea)
My worry: if the admin enters a wrong payment and the loan auto-completes, undoing
it is painful — un-flip the flag, subtract the number, un-do anything downstream.

Why the pain exists: **I'm storing derived state as if it were source-of-truth.**
`what_is_paid` and `is_fully_paid` are *stored and mutated* in my design.

How real systems remove the pain (event sourcing / immutable ledger):
- Never mutate money — **append payment events** (`+5`, `+3`). The log is
  immutable; you never edit or delete an entry.
- To fix a mistake, append a **reversing entry** (`-3`, "correction"). The error
  stays visible; a compensating event cancels it. (This is my soft-delete
  instinct applied to money.)
- Then `what_is_paid = sum(events)` and `is_fully_paid = (sum >= fees)` are
  **derived, not stored** — computed on demand. Undo becomes "append the
  opposite," and every dependent value recomputes itself automatically. No
  cascading rollback, because nothing was ever manually set.

**Important correction to my own words:** in MY version these are **stored**
attributes, not derived. That's exactly why undo is manual here. The lesson is
knowing that a real system would make them *derived* from an immutable log — and
that doing so is why undo would be trivial. I'm choosing stored + a manual
correction for a practice app; I'd reach for the ledger the moment it was real
money.

(Related name I don't need here: **idempotency** — giving each payment a unique
key so a double-click / retry can't charge twice.)

---

## Access control seam (noticed, not fully built)
In a `struct` everything is public, so the admin can poke any field directly. But
`what_is_paid` should only change through a guarded function (can't overshoot,
auto-completes, and — in a real system — logs an event). That guarantee is
impossible with a public field. This is the real motivation for `private` /
encapsulation: **one door, so every change is recorded and guardable.** Same idea
as the ledger. I didn't switch to private for this project, but this is the first
place all-public actually costs a guarantee I want.

---

## Fixes applied / to keep in mind
- `is_fully_paid = _is_fully_paid;` — the constructor parameter was shadowing the
  member (same shadowing bug as the very first `MAX_Books_Number`). Renamed the
  param with `_` and assigned the member. Fixed.
- `pay_full_fees` guard should be just `if (!is_fully_paid)` — the old
  `fees > what_is_paid` blocks a **zero-fee loan** from ever completing, so a free
  book could never be returned.
- Use `std::fabs`, not bare `abs`, so it can't resolve to integer `abs` and
  truncate the difference.

---

## Meta
Same principle keeps reappearing from new angles: never destroy a fact (soft
delete → close loans → append reversing payments), reference stable IDs not
pointers, and route changes through one guarded door (encapsulation → coordinating
functions → the ledger). Recognizing "my pain comes from storing derived state" is
the senior insight; choosing not to build the full ledger in a toy app is the
right-sizing that goes with it.