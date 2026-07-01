# Library System — Notes Part 2 (Deletion, SearchResult, RAII)

Continuation of my design notes, pulled out of the code so the code stays clean.
Kept in full, not summarized. This part covers: soft-delete + lookup, building
**SearchResult** (a dynamic array by hand), the Rule of Three on a *second* type,
reference vs pointer, and the big one — **RAII**, which is what this whole
exercise was secretly teaching me.

---

## PART 4 — DELETION AND LOOKUP

### The code
```cpp
int get_book_index_by_id(int Book_id) {
    if (curr_book <= 0) return -1;
    for (int i = 0; i < curr_book; i++)
        if (book_array[i].id == Book_id) return i;
    return -1;
}

bool delete_book(int Book_id) {
    int idx = get_book_index_by_id(Book_id);
    if (idx == -1 || book_array[idx].is_deleted == true)
        return false;
    book_array[idx].is_deleted = true;
    return true;
}
```

### Short-circuit evaluation (my syntax question — answered)
`idx == -1 || book_array[idx].is_deleted` is **correct and safe**. `||`
short-circuits: it evaluates left to right and STOPS the instant something is
true. So if `idx == -1` is true, the right side never runs, and I never touch
`book_array[-1]`. The ordering is doing real defensive work — the guard that
proves the index is valid sits BEFORE the operation that uses it. If I flipped
them (`book_array[idx].is_deleted || idx == -1`), I'd read out of bounds before
checking. Same idea for `&&` (stops on the first false). Tool to remember: lean
on short-circuit to guard a dangerous operation behind a safety check in one
condition.

### The finder/caller split (design decision made on purpose)
`get_book_index_by_id` finds a book **whether or not it's deleted.**
`delete_book` then rejects the already-deleted ones itself. That division of
labor is the RIGHT one, and it commits me to a principle:

> **Let the finder answer "where is it?" and let each caller decide "do I accept
> it in this state?"**

- delete says "must be alive."
- a future borrow will say "must be alive."
- a future "restore deleted book" admin feature will say "must be *deleted*."
- a raw lookup might not care.

Because the finder stays neutral, all four are possible. If I'd baked
`is_deleted == false` into the finder, restore would be impossible without a
second finder. Same shape as the search-vs-display split: keep the lookup dumb
and reusable, push the policy to the edge.

### Micro-note
`if (curr_book <= 0) return -1;` at the top of the finder is redundant — if
`curr_book` is 0 the loop runs zero times and returns -1 anyway. Harmless, some
keep it for readability.

### Domain thread left dangling (for the Loan entity later)
Soft-delete is on the book *record*, but `quantity` is copies. Is "delete this
book" the same as "we no longer stock any copies"? And can I delete a title
while copies are currently borrowed? Pulls straight into the Loan entity.

---

## PART 5 — SearchResult: BUILDING A DYNAMIC ARRAY BY HAND

The search function should RETURN matches, not print them — separation of
concerns. Printing is one *consumer* of the results; deleting is another; a GUI
is another; JSON-over-HTTP is another. Returning data instead of printing is
what makes the function frontend-agnostic. So I built a proper return type.

### The three options I weighed (my comment, kept in full)
1. you can return a whole object copy aka define an object called
   book_that_match_prefix that have array and it is own variable and you can just
   return a copy of it — this of course is a waste of memory, I do not think it is
   the optimal solution.
2. you make a pointer to an array of books allocated on heap; it will only have
   books that match the prefix. It is a safe solution but has the same problem as
   point 1, and it has also a big problem: when will you de-allocate this array?
   The caller has the pointer, he can deallocate whenever he wants, but the
   complexity of your program rises.
3. an array of pointers where each pointer points to a book object — this is a
   good solution for the copy problem, but it may have a security problem: you
   give the caller direct access to the books. But is it really? Also the caller
   needs to de-allocate the pointers array.

> An easy task like this seems complex in C++ compared to Python and JS. I see
> now why memory management is a pain and why it's for real engineers. I leaned
> toward the third solution.

### Why I made it grow (my comment, kept in full)
I know the goal is not to do resizing stuff, but thinking about the size of this
object and making it fixed will always lead to a problem — waste of memory or not
enough (a query can return 1 book or 1000 books).

> This is *exactly* why `std::vector` grows instead of being fixed. I felt the
> motivation from the inside.

### The fields
```cpp
Book** Pointer_to_pointer_array;  // heap array of pointers to real Books
int count;                        // how many are actually filled
int max_size;                     // capacity of the array
```
`count` and `max_size` are separate on purpose: capacity vs how much of it is
used. This split IS what a vector is (`size()` vs `capacity()`).

### Geometric growth — resize_to_double
```cpp
bool resize_to_double() {
    if (max_size == INT_MAX) return false;   // guard overflow (won't happen here)
    max_size *= 2;
    Book** new_arr = new Book*[max_size];
    for (int i = 0; i < max_size / 2; i++)   // copy the old half
        new_arr[i] = Pointer_to_pointer_array[i];
    delete[] Pointer_to_pointer_array;       // free old
    Pointer_to_pointer_array = new_arr;      // repoint
    return true;
}
```
Order is correct: copy old → delete[] old → repoint. No leak. Doubling (instead
of +1 each time) is what real vectors do — it makes append **amortized O(1)**.
Notes:
- `INT_MAX` comes from `<climits>` (I first wrote `__INT_MAX__`, a compiler
  builtin — the portable name is `INT_MAX`).
- `new` can throw `std::bad_alloc` on allocation failure. Ignoring that is the
  right call *for this project* — named and dismissed on purpose, not by accident.

### Amortized append
```cpp
bool add_pointer_to_book(Book* Book_ptr) {
    if (count == max_size) {
        if (!resize_to_double()) return false;   // propagate failure
    }
    Pointer_to_pointer_array[count] = Book_ptr;
    count++;
    return true;
}
```
Check-full → resize-if-needed → append → bump count. Clean.

### The count invariant (load-bearing rule)
When I allocate `max_size` but only fill `count`, the tail slots are
uninitialized garbage pointers. That is FINE — but only because `count` says
"only the first `count` are real" and nothing ever reads past `count`. The
garbage is harmless *only because `count` guards it.* This invariant —
**never read past `count`** — must hold in every single method. Drop it in one
place and garbage becomes a crash. That's the discipline tax of hand-rolling:
the correctness lives in a rule I must never break, not in the types.

---

## PART 5b — RULE OF THREE ON A SECOND TYPE

Doing the Rule of Three again on a different type is what turns it from "a thing
I did once" into "a thing I know."

### Copy constructor
```cpp
SearchResult(SearchResult const &other) {
    max_size = other.max_size;
    count = other.count;
    Pointer_to_pointer_array = new Book*[max_size];   // our OWN outer array
    for (int i = 0; i < other.count; i++)
        Pointer_to_pointer_array[i] = other.Pointer_to_pointer_array[i];
}
```
The asymmetry that matters: I deep-copy the **outer array** (new `Book*[]`, copy
the addresses in), but the pointers inside still point at the **same real
Books**. That is CORRECT, not a bug. Two SearchResults pointing at the same
library Books is fine; two SearchResults owning the same *outer array* would be
the double-free. So I copy one level and share the other.

### Copy assignment
```cpp
SearchResult &operator=(SearchResult const &other) {
    if (this == &other) return *this;          // self-assignment guard
    delete[] Pointer_to_pointer_array;         // FREE BEFORE ALLOCATE
    max_size = other.max_size;
    count = other.count;
    Pointer_to_pointer_array = new Book*[max_size];
    for (int i = 0; i < other.count; i++)
        Pointer_to_pointer_array[i] = other.Pointer_to_pointer_array[i];
    return *this;
}
```
Same three hazards as before, handled: free-before-allocate, self-assignment
guard, return `*this` by reference.

### Destructor
```cpp
~SearchResult() { delete[] Pointer_to_pointer_array; }
```
Frees ONLY the outer array — never the Books. The ownership contract that used to
live in a comment nobody reads is now **executable code the compiler runs.**

### The const bug I fixed
My copy ctor and copy assignment first took `SearchResult &other` (non-const).
That means you cannot copy from a `const` object OR from a **temporary** — and
`get_books_by_prefix` returns a temporary. Fix: `const SearchResult& other`,
matching what I already did on `Library_Books`. A copy has no reason to modify
its source, so it should promise not to (`const`), which also lets it bind
temporaries and consts.

---

## PART 5c — REFERENCE vs POINTER (my confusion, cleared)

My comment (kept): *pointers are the real address of the object; you can change
where they point. A reference can also be considered an address but it cannot be
changed to point to another object. A reference feels like a real object, a trick
to avoid copying — something in between a pointer and a real value.*

That intuition is good. Exact version — a reference is an **alias**: another name
for an existing object. Three concrete differences:
- A pointer can be **reseated** (point at A, then B). A reference binds once at
  creation and IS that object forever. (I had this right.)
- A pointer can be **null**. A reference must always refer to something valid —
  no "reference to nothing."
- A pointer needs `*` / `->` and allows arithmetic. A reference you just use like
  the object itself.

So `const SearchResult& other` = "another name for the caller's object, which I
promise not to modify, and which I don't copy to receive." Right type for a copy:
no copy to pass the thing I'm about to copy, and `const` so temporaries/consts
bind.

### "Does `return *this` return a copy?" (my flagged question — answered)
It depends **entirely on the return type** — same lesson as `a = b = c`.
- Return type `SearchResult&` (a reference) → `return *this` hands back the object
  ITSELF, by alias, **no copy.**
- Return type `SearchResult` (no `&`) → `return *this` invokes the **copy
  constructor** and hands back a copy.

Same expression `*this`, opposite behavior — the *return type* decides, not the
expression. I used `&`, so I get chainable assignment with zero copies, exactly
like the built-in `=`.

---

## PART 6 — RAII (THE BIG INSIGHT — the real "wow")

### What I actually discovered
I didn't reinvent "an array that grows." I discovered **RAII: Resource
Acquisition Is Initialization.** Clumsy name, profound content:

> **Tie a resource's lifetime to an object's lifetime, so acquiring and releasing
> it become automatic instead of manual.**

The destructor runs when the object goes out of scope — guaranteed, no matter how
the scope exits, even on an exception. So "who frees it and when" stops being a
contract the caller must honor and becomes a thing that CANNOT be forgotten,
because the compiler runs it.

### What RAII eliminated from the trap I was stuck in
- "caller must remember to `delete[]`" → gone, the destructor does it.
- "caller might `delete[]` twice" → gone, they don't call delete at all.
- "caller might free the real Books by mistake" → gone, the destructor frees only
  the outer array.
- "the rules only live in a comment nobody reads" → gone, the rules are now
  **executable code in the destructor.** The type ENFORCES the contract instead of
  DESCRIBING it.

That last one is the deepest: a comment is understood only by me or a reader. A
destructor is understood by the **compiler.** I moved the ownership rule from
documentation into mechanism. That's the whole art of safe C++ in one step.

### The precise conclusion (not "use a vector" — bigger)
I end up building the **principle** that `vector` is one instance of. RAII is
everywhere once you see it:
- `std::vector` — RAII for a growable array (exactly my SearchResult, done right).
- `std::string` — RAII for a character buffer (why you never `free` a string).
- `std::unique_ptr` / `shared_ptr` — RAII for a single heap object. (The smart
  pointers I already used to avoid memory headaches — THIS is why they work.
  They're destructors wrapping a pointer. I was using RAII without the name.)
- `std::lock_guard` — RAII for a mutex: lock on construction, unlock on
  destruction, so you can't forget to release.
- My own `Library_Books` — `new[]` in the constructor, `delete[]` in the
  destructor. **I already wrote an RAII class.** The Rule of Three I fought
  through was me hand-implementing the copy semantics RAII types need. I built the
  machine before I knew its name.

So "just use vector" isn't laziness — it's using a correct, battle-tested
implementation of the exact thing I proved I need. I reached for it because I
re-derived that it's RIGHT, not because I was told it's easy. Different reasons;
the second is the mark of understanding the tool instead of obeying it.

### The fractal problem (why the standard types exist)
If `SearchResult` owns a resource, it needs its own Rule of Three. If I gave it a
member that owned a resource, THAT needs a Rule of Three too. Every hand-owned
resource re-raises the same three questions (destructor? copy? assignment?). The
standard types answered those three questions correctly, once, so I can stop
re-answering and re-bugging them every project.

### The honest tally (the point of the whole rep)
To make `SearchResult` correct: default ctor, copy ctor, copy assignment,
destructor, a resize, a `count`/`max_size` invariant I must never violate,
`const`-correctness I must remember, and (if I go further) move ctor + move
assignment. ~8 pieces of machinery, each a place to introduce a bug, to safely
hold "a growable list of pointers." That tally IS the argument for
`std::vector<Book*>`. I don't take it on faith — I built it and can list what it
costs.

---

## PART 7 — OWNERSHIP AND THE DOUBLE-FREE (my answer, reasoned)

Question: the copy constructor deep-copies the outer array, but the inner
`Book*`s still point at the same real Books — two SearchResults hold pointers to
the same Books. Double-free risk when both destruct?

My answer: **No double-free.** You are not touching the Book objects' array. You
have two arrays that point to the same objects; when you free one, the other
doesn't even know it exists — you are freeing **pointers**, not the objects
themselves. The Books are owned by `Library_Books`, not by `SearchResult`.
`SearchResult` only borrows addresses. Ownership boundary drawn in the right
place, and I can say why.

---

## CORRECTNESS WATCH (fix before trusting the code)

- **`Library_Books::operator=` is missing its `delete[]`.** It does
  `this->book_array = new Book[MAX_Books_Number];` WITHOUT freeing the old array
  first → leak. This is hazard #1 (free-before-allocate) that I already handled
  correctly in `SearchResult::operator=`. Fix: add
  `delete[] this->book_array;` right before the `new`, matching my own correct
  pattern.
- `get_books_by_prefix_consolo` has a stray `cout << "No ";` with no `return` on
  the empty path — it prints half a message then falls into the loop. Throwaway
  console version, but wrong as written.
- Optional stretch: give `SearchResult` a **move constructor** and **move
  assignment** (the Rule of Five). Returning a `SearchResult` by value is the
  textbook case: the temporary is about to die, so instead of deep-copying its
  array, STEAL its pointer and null the original, leaving one owner. Copy = "make
  my own array"; move = "take yours and blank you out so we don't both free it."
  Correctness (the Three) first, then this.

---

## META-LESSON TO KEEP

Manual memory management, done *safely*, converges on RAII. The standard library
is a box of pre-built, correct, tested RAII types so I don't hand-roll and
hand-debug them every time. When I rewrite this project "for real" with `vector`,
I'm not taking a shortcut — I'm replacing my hand-built RAII with the library's
hand-built RAII, understanding every guarantee it makes *because I built those
guarantees myself first.* That's the difference between using `vector` because I
was told to and being able to write one if I had to.