# Library System — Design & C++ Notes

These are my own notes and comments from building the Library System project,
pulled out of the code so the code stays clean. They are kept in full, not
shortened. Two layers here: **design thinking** (ID generation, deletion,
how big systems handle this) and **C++ mechanics** (encapsulation, static,
heap allocation, the Rule of Three).

---

## PART 1 — ID GENERATION DESIGN

### The decision for this project
For this project the correct solution is an **auto-increment 64-bit integer
with no reuse**. Each new book increments `max_id` and gets the value. One
variable, O(1), and no two IDs ever come out equal.

### Why ID reuse is wrong (not just unnecessary)
At first I designed an efficient free-id system (track freed ids, push them on
delete, pop on create — both O(1)). It works, but it is fundamentally the wrong
idea, for two reasons:

- **Not necessary:** a system like this will never pass ~10,000 books (even if
  you assume a million). A 64-bit integer holds ~1.8 × 10^19 values. "Limited"
  is not a real concern.
- **Wrong by design:** an ID is an *identity*, not a count and not an array
  index. A book with id 5 was bought, borrowed, has bills and records in the
  system. It does not just exist — history points at it. If I delete book 5 and
  later hand id 5 to a brand-new book, every old record that said "book 5" now
  points at the wrong book. So reuse creates **dangling references**.

The right way to "delete" is **soft delete**: keep the record, set a flag
(`is_deleted` / `removed_date`) instead of erasing it. The system can say
"there was a book with id 5 that was deleted on date ---". The ID stays alive
forever as a stable anchor that all the history can keep pointing to.

**Cost of soft delete (named so it's a decision, not a surprise):** once books
can be soft-deleted, every read path has to ask "is this book active?" —
borrowing checks it, listing filters it, search skips it. Cheap and worth it
here, but it is a real cost threaded through the reads.

### Security — what I actually meant
The security concern is NOT the integer itself. It's that a sequential,
incrementing id is **predictable and enumerable**: if book 41 exists, an
attacker guesses 40 and 42 exist too, and can count the whole collection by
watching the max. That's a real bug class in a web API (insecure direct object
reference). In a local console library app it's **zero** concern — right to drop
it, but good to know exactly *what* was dropped.

### How huge systems do it (general thinking)
- Make the id a **string**, randomly generated, so there's effectively no limit.
- "Can a random generator produce only unique values?" — I guessed impossible.
  It's actually possible (Feistel networks / format-preserving encryption,
  full-period generators, shuffled permutations emit every value once). Out of
  scope, but "impossible" was a false belief worth correcting.
- The practical real-world answer is the **UUID**: 122 random bits. Collision
  probability is so astronomically small that production systems generate them
  and **never check for duplicates**.

### The hash mistake (important correction)
I said "generate a random id, hash it, and uniqueness is solved." That's
backwards. I meant **hash table**, not **hash function** — two different things
that share the word "hash":

- A **hash function** is deterministic: same input → same output. It creates no
  new uniqueness. If the input repeats, the hash repeats. It can even *destroy*
  uniqueness (two inputs can collide to the same output). Uniqueness must come
  from the input, not from hashing.
- A **hash set / hash table** is the data structure. The reason "does this id
  already exist?" can be ~O(1) is that you store existing ids in a hash set and
  test membership. The O(1) lives in the *set*, not in hashing the id itself.

Lesson shape: don't confuse *where* a guarantee comes from.

### "Isn't a duplicate id a huge problem?" — yes, and the paradox resolved
A duplicate id IS catastrophic (two products sharing an id → review on the wrong
item, order ships the wrong thing). My alarm was correct. The wrong turn was the
conclusion "therefore they must actively check for duplicates." That does not
follow. Two true-feeling statements:

1. A duplicate would be catastrophic. **(true)**
2. Therefore we must check for duplicates. **(does NOT follow)**

The escape: make collision not *unlikely* but **effectively impossible**, so
checking guards against nothing real. For a UUID, reaching a one-in-a-billion
chance of ANY collision needs ~2.7 × 10^15 ids generated. The datacenter is
likelier hit by a meteor mid-write.

### The deep idea — prevention by design vs prevention by enforcement
- **Enforcement:** generate something, then *check* it against everything that
  exists. Needs a global view of all prior ids. Across many servers and
  datacenters all minting ids at once, that shared check *is* the bottleneck;
  coordination is the expensive thing distributed systems avoid.
- **Design:** construct the id so it *can't* collide, no lookup needed. Two
  flavors: make the random space so vast a collision is a non-event (UUID), or
  **partition** the space so no two generators can produce the same value
  (server A owns one range, server B another; or fold machine id + timestamp
  into the id — Twitter's Snowflake does this).

My auto-increment-no-reuse is the **same family** as Snowflake: uniqueness by
construction, zero collision check. It partitions trivially because there's only
one generator — me. Amazon can't assume one generator, so they go
enormous-random or partition explicitly. Same goal (uniqueness without a global
lookup), different tools because their constraint (many machines, no shared
view) is one I don't have.

Resolution: real systems care about duplicates *enormously* — so much that they
engineer them out at the **construction** step, which is stronger than catching
them at the **check** step.

---

## PART 2 — CODE DESIGN DECISIONS

### Globals vs encapsulation
*(my comment, kept in full)*

Using global variables is generally considered bad practice because they offer
no control or boundaries; any class or function across your codebase can access
and modify them without restrictions.

For example, if you encapsulate an `id_generator` variable and its logic inside
a struct or class, any component in your codebase that needs an ID must
explicitly call the `generate_id` function. It can no longer directly access or
alter the underlying `max_id` variable. By forcing access through a function,
you gain the ability to implement necessary guards and controls — such as
checking if you have reached the maximum possible ID limit.

On a deeper level, this encapsulation makes it much easier to implement and
reason about synchronization and race conditions. Ultimately, it provides the
logical control required to safely manage and understand how your variable
changes state.

> Principle name: **encapsulation** — "one door in, with the ability to add
> guards." When a value can be touched from everywhere, a bug involving it can
> come from everywhere, and you can't reason about it locally.

### Static vs non-static
*(my comment, kept in full)*

Of course, this approach is much better than using a global variable for the
reasons we discussed. The question now is: should I make this static or not?

Making it static means that the `max_id` variable and the `generate_id`
function will be shared across the entire codebase. This would be perfectly fine
if we only ever have one library, meaning we would only need a single ID
generator for the books in that library.

However, what if we want to scale? Even though scaling won't actually happen in
this specific project, I have another reason to make it non-static: logically,
an ID generator is not a universal tool. It is specifically tied to generating
IDs for a single, distinct library. Therefore, it makes much more sense to
encapsulate it directly within the library instance itself.

> The word I was forgetting: **static** (a member you call without an object,
> e.g. `Id_Generator::generate_id()`). The real lesson: `static` is not a syntax
> choice, it's a statement about *how many of this thing exist in your domain.*
> One global counter, or one per library? I chose non-static (one per library),
> which keeps options open and composes better.

### Heap vs stack allocation in the constructor
*(my comment, kept in full)*

I believe my previous approach was incorrect because it allocated memory on the
stack instead of the heap. If I initialize a temporary array like
`Book temp_book[MAX_Books_Number]{};`, it is placed on the stack frame of the
constructor.

Since stack variables are destroyed when their enclosing function exits, and a
constructor is fundamentally a function that finishes executing after
initialization, any temporary stack variable created inside it will be
destroyed. Therefore, I cannot safely assign a local stack array to
`book_array`. Should I use dynamic memory allocation to allocate this array on
the heap instead?

> Answer: yes. Use `new Book[MAX_Books_Number]` (the C++ way — `new` runs the
> `Book` constructors; `malloc` would not). And `new[]` obligates a matching
> `delete[]` — which is what opens the Rule of Three below.
>
> Also fixed here: **variable shadowing.** Writing `int MAX_Books_Number = ...`
> inside the constructor declares a NEW local that shadows the member, so the
> member stays uninitialized. Dropping the `int` assigns to the member. (Member
> initializer lists avoid the trap entirely.)

---

## PART 3 — THE RULE OF THREE

*(my comment, kept in full)*

When we declare `Book *book_array;` and allocate memory using `new` in the
constructor, we create an interesting dynamic. The pointer itself belongs to the
`Library_Books` object and lives wherever that object lives (such as on the
stack). When the object's scope ends, the pointer variable is automatically
destroyed.

However, the actual array allocated on the heap does not live within the
object's stack scope. It exists independently in heap memory. While the pointer
is tied to the object's lifetime, the memory it points to is not. This
introduces the first rule: **If a constructor uses `new` to allocate heap
memory, a destructor must be implemented to explicitly free it.**

**Rule Two: The Copy Constructor**
Normally, when we write `int x = y;`, the value of `y` is copied into `x`.
Similarly, if we pass an object by value or initialize a new object from an
existing one — like `Library_Books obj2(obj1);` — the default behavior is a
**shallow copy**. This copies the literal address held by the pointer in `obj1`
into the pointer in `obj2`.

As a result, both objects now point to the exact same array on the heap. This
causes severe bugs, such as a **double-free** error when both objects try to
delete the same memory. To fix this, we must implement a Copy Constructor to
perform a **deep copy**, creating a separate array on the heap for the new
object.

**Rule Three: The Copy Assignment Operator**
Similarly, when assigning an already existing object to another, such as
`obj2 = obj1;`, we do not want `obj2` to simply copy `obj1`'s pointer address.
Instead, we want the array that `obj2` points to to contain the same elements as
the array belonging to `obj1`. To achieve this, we must overload the assignment
operator (`=`) to properly clean up `obj2`'s existing heap memory and safely
copy the elements from `obj1`.

### The ordering lesson (copy-constructor bug I had and fixed)
My first copy constructor allocated `new Book[...]` *unconditionally first*,
then checked for null and overwrote the pointer with `nullptr` — leaking the
array I just allocated, and crashing on the null path because
`new Book[-1]` runs before the check. The fix: **decide first, allocate only
inside the `else`.** With manual memory, the order of operations is the whole
game — allocate-then-reconsider leaks; reconsider-then-allocate doesn't.

### The three hazards of the assignment operator
1. **Free before allocating.** Assignment runs on an object that *already owns*
   an array. `delete[]` it first, or you leak it.
2. **Self-assignment guard.** `lib = lib;` — if you free your own array first and
   `other` *is* you, you've freed what you're about to copy from.
   `if (this == &other) return *this;` at the top.
3. **Return type.** Return `Library_Books&` and `return *this;`.

### Why operator= returns a reference (a = b = c)
*(my comment, kept in full)*

I think this will make case `obj1 = obj2 = obj3` work correctly but I need more
explanation. Let's first understand what `a = b` is all about: this is just a
function call `a.operator=(b)` — a function that takes a reference of object `b`
as an argument and modifies `a` and returns a reference to object `a`. This
aligns with `Library_Books &operator=(const Library_Books &other)` and
`return *this`.

`a = b = c` will be `a = (b = c)`, so the function `b.operator=(c)` will be
invoked, and after this function is executed and `b` is modified correctly, what
is returned from this function will be used for `a.operator=( ... )`. Notice
that for everything to be correct, `b.operator=(c)` needs to return a reference
to `b`, so we have `a.operator=(b)`. This is why we make the return type of the
operator function `Library_Books &` and `return *this`.

> Built from three facts:
> 1. Assignment is **right-associative**: `a = b = c` parses as `a = (b = c)`.
> 2. `*this` is the object the method runs on; returning it as a **reference**
>    (`Library_Books&`) hands back the object itself, not a copy.
> 3. That result becomes the right side of the outer assignment.
>
> The wrong return types prove why reference is right:
> - **By value** (`Library_Books`, no `&`): `b = c` makes a copy of `b` — invokes
>   the copy constructor for nothing, assigns `a` from a throwaway temporary.
>   Correct by luck, wasteful, drags any copy-constructor bug into every chain.
> - **`void`**: `a = (b = c)` becomes `a = (nothing)` — no value to assign,
>   **won't compile.** Chaining impossible.
>
> So a reference to `*this` is the one return type that makes `operator=` behave
> like the built-in `=`: chainable AND copy-free.

### Rule of Five (note for later)
The "/Five" adds a **move constructor** and **move assignment operator** — the
C++11 optimization where, instead of deep-copying, you *steal* the dying
object's pointer and null its original so only one owner remains. Pure
performance, not correctness. Ignore until the Three compile and pass the crash
test. **Correctness first, then speed.**

### The brutal test
Make two libraries, add a few books to each, do `lib2 = lib1;`, let both fall
out of scope, run it. No crash + no leak → all three hazards handled. Double-free
→ missed self-assignment or free-first.

---

## Small robustness items (not bugs in current usage, fix later)
- Parameterized constructor doesn't validate `_MAX_Books_Number` (a zero/negative
  size makes `new Book[...]` throw or misbehave). Same guard style as `add_book`.
- Assignment operator doesn't guard a null source (`new Book[-1]` if assigning
  *from* a default-constructed library). Nobody does that in the current flow.
- `#include <stdio.h>` is unused now — harmless noise.

---

## Meta-lesson I want to keep
The recurring move that mattered most: solve the abstract, scaled-up version of
the problem in my head, then **consciously decide not to build it** when the
requirements don't justify it. Right-sizing — knowing when *not* to add
complexity — is itself the senior skill. The destination of all that
distributed-systems ID reasoning was "the boring auto-increment integer is
correct," and reaching that on purpose is the point.