# Library System — Notes Part 4 (Templates, DTOs, Queries)

Notes for the query layer. This part is where the project started looking like a
real backend: a generic container, purpose-built response types, and a clear rule
for what the backend computes vs what the frontend formats.

---

## Templates — the payoff of the copy-paste pain
I wrote the same container three times (`SearchResult` for `Book*`,
`SearchResult_User` for `user*`, one for DTOs) — character-for-character identical
except the element type. That duplication is the signal a template exists to kill.

My own words: instead of copy-pasting a class that has the exact same logic except
the data type, make the class **type-agnostic** — every use of the type becomes a
placeholder `T`. You pass the type as a parameter (`Dynamic_arr<user>`), and the
**compiler does the copy-paste at compile time**, replacing each `T` with the real
type.

```cpp
template <class T>
struct Dynamic_arr { T* data_pointer; int count; int max_size; ... };
```

So `Dynamic_arr<Book*>`, `Dynamic_arr<user*>`, `Dynamic_arr<DTO...>` are one
implementation, written once. This **is** what `std::vector` is — I now understand
`vector<int>`, `queue<double>` from the inside, not as magic syntax.

Bonus: the template makes a whole class of bug **impossible**. The old
copy-pasted containers let me get the copy-constructor's parameter *type* wrong
(`SearchResult` instead of `SearchResult_User`). In the template there's no second
type name to mistype — the copy constructor takes `const Dynamic_arr&` and `T` is
whatever it is. One correct implementation, zero copy-paste surface.

Note fixed while templatizing: the resize loop copies `count` elements, not
`max_size/2`. Same result today (I only resize when full), but `count` is what I
*mean* — "copy the elements that exist" — so it's robust, not coincidental.

---

## DTO — a purpose-built response type
The four queries don't return `Book`s. Query 1 wants book name + borrow date +
supposed return + is_late + fees + remaining. None of those (except name) live on
`Book`; they live on `Borrowed_Book` or are **computed** (is_late, remaining). So
the thing I return is a **row that joins a loan and a book/user**, which is no
single entity I have.

That's a **DTO — Data Transfer Object**: a type whose only job is to carry exactly
the fields a particular response needs, assembled from one or more entities plus
computed values. Not `Book`, not `Borrowed_Book` — `DTO_User_Borrowed_Book`,
built for the query.

So the division a real backend uses, which I arrived at:
- **Reusable container** (`Dynamic_arr<T>` / `vector<T>`) — written once.
- **Purpose-built element** (a DTO per query shape) — the `T`.
Not a new *container* per query — a new *element type* per query, in the *same*
container.

Public fields on a DTO are correct (not a bug) — it's a dumb data-carrier, so a
public field IS the getter. (I was right to push back on this.)

---

## The big architecture question: what does the backend compute vs the frontend?
My worry: if the backend does everything, every new frontend display needs a new
backend function (tight coupling). If it hands over raw data, the frontend
re-implements business rules (duplication, security holes). Both extremes are
wrong.

**The rule I landed on:** the backend owns anything that is a **business rule, a
computation over data it holds, or a security/consistency concern**. The frontend
owns **presentation** — formatting, layout, wording, color.

**The test for any piece of logic:** "If a second frontend existed (mobile, CLI,
another service), would they need this same answer?" Yes → backend, computed once,
so all clients agree. Purely how one screen looks → frontend.

Applied:
- **is_late** = `!is_finished && today > supposed_return_date` → **backend**. It's
  a business rule; clients must agree; the rule can change (grace period); "today"
  on the server is trustworthy, a client clock isn't. Computed and returned as a
  field.
- **remaining to pay** = `fees - what_is_paid` → **backend**, stronger, because
  it's money. Never let a client compute what someone owes.
- **"12 March 2025" / show late in red** → **frontend**. Presentation, varies per
  client. If I ever wrote `get_loans_formatted_in_red_for_mobile`, that's the
  over-coupling to avoid.

So the resolution isn't binary: **compute the decisions and facts** (is_late,
remaining, allowed?), **hand over the raw values for display** (the actual date,
name, id). The DTO is the vehicle — it carries computed facts AND raw values side
by side, so the frontend gets decisions pre-made where correctness/security demand
it, and raw material where only formatting is left.

Note: is_late is computed once at DTO construction — it's a **snapshot**, not
live. That's correct for a DTO (a point-in-time transfer object).

---

## The four queries (+ return + finders)
- **Query 1 — a user's loans.** Split into two functions (option 2, not a
  `bool include_finished` flag): `get_user_active_loans` (checks user exists & not
  deleted) and `get_user_history_loans` (only checks exists — history shows
  everyone). This is the neutral-finder / caller-decides-liveness split applied
  *per query*. Two functions beat a bool because the call site reads its own intent
  (`get_user_active_loans(7)` vs the "boolean trap" of `get_user_loans(7, true)`).
- **Query 2 — users who borrowed a book.** `Dynamic_arr<user*>` — simple, just
  id + name via pointer.
- **Query 3 — borrowers of a certain book.** Same DTO as query 1 but carrying
  **user name** instead of book name (I widened the DTO to hold both). Active and
  history variants.
- **Query 4 — all actively-borrowed books.** Iterate books, for each non-deleted
  book check if it has an active loan, add once (break on first match).
- **return_book** — find the active (user, book) loan, call the loan's own
  `user_return_book()` (one source of truth for "finishing a loan"), then
  `quantity++`. Same validate-then-mutate + atomic pair as borrow, reversed.
- Added **get_user_idx_by_id** to mirror `get_book_index_by_id` — both now return
  on first match (`break`). Routing lookups through these finders is what collapses
  the long hand-written search loops where copy-paste bugs hide.

---

## Bugs this part taught (the recurring failure mode)
The queries came out structurally right but with a cluster of **copy-paste slips**:
dropped `add_element` (query built a DTO and threw it away → empty result), a
truthiness test `if (book_id)` instead of `== book_id` (matched every book), a
wrong flag name (`book_does_not_exist` set where `does_transaction_not_exist` was
checked → return always failed), and a positional-constructor arg-order mismatch
after adding a DTO field.

Root cause every time: **long hand-written loops with near-identical bodies, where
one dropped or wrong line hides silently.** The cure is the same one I keep
re-deriving — route lookups through the small finders so the loops (and their
bug surface) disappear. Also: a positional constructor with many same-typed args
(several time_t, several float) lets a wrong arg order compile silently. Fewer
positional args, or fill a struct field-by-field, kills that class of bug.

---

## Still open (for the review pass)
- `set_real_return_data` is declared `bool` but returns nothing (UB) and is now
  dead code — delete it; `user_return_book()` already sets the date.
- User-index guard is inconsistent: query 2 checks `if (user_idx == -1) continue;`,
  query 3 doesn't. Same risk, guard it the same way everywhere (or document the
  invariant once and drop it everywhere). Inconsistency reads as an oversight.
- A missing filter that yields empty (book not found → no matches) is benign; a
  missing guard before an array index (`library_users_array[-1]`) is not. Know the
  difference.

---

## Meta
Same principle, new layer: reusable mechanism + purpose-built shape (container +
DTO), and one clear ownership line (backend owns rules/computations/security,
frontend owns presentation). The DTO is where those meet — computed facts and raw
values travel together so each side does only its job.