## Project 3 — Beta: **pldraw**

Design and implement **pldraw**, a Qt-based graphical interpreter that extends your **postlisp** interpreter from Project 2. It parses and evaluates Lisp-like expressions, renders graphical output (points, lines, arcs, rectangles, ellipses), and responds to user commands via a REPL widget. A reference implementation is available in `ref-binary`.

### Schedule & Links

* **Released:** Oct 16, 12:00 AM
* **Beta due:** Nov 01, 11:59 PM
* **Final due:** Nov 15, 11:59 PM
* **GitHub Classroom invite:** (updated Oct 16, 5:26 PM) — see assignment link.

---

## Honor Code & Generative AI

* You may reuse **only your own Project 2 (Fall 2025)** code. Reusing others’ work or your own from prior semesters violates the Honor Code.
* If you use Generative AI (e.g., GPT-4, Copilot), follow the syllabus policy and **include the AI Reflection Form** in your submission. Failure to disclose use is a violation.
* **Hard-coding tests is prohibited**; code will be checked by an automated comparison system.

---

## Application Architecture (GUI)

The main window stacks three widgets vertically:

1. **MessageWidget** — read-only info/error output
2. **CanvasWidget** — drawing surface
3. **REPLWidget** — line-based REPL input

### GUI Modules and APIs

* **MainWindow (`main_window.hpp/cpp`)**

    * `MainWindow(QWidget* parent=nullptr);`
    * `MainWindow(std::string filename, QWidget* parent=nullptr);` (optional preload script)

* **MessageWidget (`message_widget.hpp/cpp`)**

    * `MessageWidget(QWidget* parent=nullptr);`
    * `void info(QString message);`
    * `void error(QString message);` (red background, selected text)

* **CanvasWidget (`canvas_widget.hpp/cpp`)**

    * `CanvasWidget(QWidget* parent=nullptr);`
    * `void addGraphic(QGraphicsItem* item);` (receives `QGraphicsItem*`)

* **REPLWidget (`repl_widget.hpp/cpp`)**

    * `REPLWidget(QWidget* parent=nullptr);`
    * `signal: void lineEntered(QString);`
    * History with ↑/↓ (MRU order)

* **QtInterpreter (`qt_interpreter.hpp/cpp`)** — extend interpreter for GUI; **do not duplicate** interpreter code.

    * `QtInterpreter(QObject* parent=nullptr);`
    * `signals:` `drawGraphic(QGraphicsItem*)`, `info(QString)`, `error(QString)`
    * `public slot:` `parseAndEvaluate(QString)`

* **QGraphicsArcItem (`qgraphics_arc_item.hpp/cpp`)** — subclass `QGraphicsEllipseItem` and override `paint()` to draw an **arc curve only** (no radial lines).

---

## Graphical Program (pldraw)

* Implement `pldraw.cpp` (Qt app).
* **Requirements:**

    1. Create and show `MainWindow`, **min size 800×600**.
    2. Enter event loop `app.exec()`.
    3. Optionally accept **one script filename** to preload environment.
    4. Show parse/semantic errors in `MessageWidget`.
* **Behavior:** Type expressions in REPL; info/results go to MessageWidget; errors appear with red background; graphical expressions render immediately on CanvasWidget.
* Example:

  ```
  ./pldraw /mnt/tests/test_car.slp
  ./pldraw /mnt/tests/test_airplane.slp
  ```

---

## Language Specification (Extended postlisp)

### Graphical Types

* **Point** — two Numbers; render as filled black circle radius 2.
* **Line** — two Points (segment).
* **Arc** — `center` (Point), `start` (Point), `angle` (radians).
* **Rect** — `lower-left` and `upper-right` Points.
* **FillRect** — `Rect` + three Numbers (R,G,B). **No border** (use `Qt::transparent`).
* **Ellipse** — bounding **Rect**.

### Special Form

* **`draw`** — m-ary; accepts graphical types; returns `None`. Draws only in graphical context.

### Built-ins

`point`, `line`, `arc`, `rect`, `fill_rect`, `ellipse` with the argument shapes described above.

### Math

`sin`, `cos` (unary); `arctan` (binary, `y`, `x`). Compare floats with `abs(diff) ≤ std::numeric_limits<double>::epsilon()`.

---

## Text-based Interpreter (postlisp)

* Builds to `postlisp` (macOS/Linux) or `postlisp.exe` (Windows).

* **Usage modes:**

    * `-e "<program>"` — print result or `Error: ...` (exit success/failure)
    * `<filename.slp>` — run file
    * **REPL** — prompt `postlisp>`, evaluate lines; on semantic error, **rollback environment** to pre-eval state; ignore empty lines. EOF: Ctrl-K (Windows), Ctrl-D (Unix).

* **Output format examples:**
  `( <atom> )` for Boolean/Number/Symbol; `(x,y)` for Point; `((x1,y1),(x2,y2))` for Line; `((cx,cy),(sx,sy) span)` for Arc; etc.

* **Example sessions** (see spec for full transcript).

---

## Interpreter Module Specifications (C++11; no Qt in interpreter)

* **Modules:** `expression`, `tokenize`, `environment`, `interpreter` (+ helpers allowed). Interpreter must throw `InterpreterSemanticError` with clear messages.

### `Expression` API (constructors & equality)

Provide at least:

```cpp
Expression();                           // None
Expression(bool value);                 // Boolean
Expression(double value);               // Number
Expression(const std::string &value);   // Symbol
Expression(std::tuple<double,double> value); // Point
Expression(std::tuple<double,double> start,
           std::tuple<double,double> end);   // Line
Expression(std::tuple<double,double> center,
           std::tuple<double,double> start,
           double angle);                    // Arc
bool operator==(const Expression &exp) const noexcept;
```

Numeric comparisons must use machine epsilon. Equality requires same type, atom value, and arg count.

### `Interpreter` API

* `Interpreter();`
* `bool parse(std::istream &expression) noexcept;`
* `Expression eval();` (throws `InterpreterSemanticError` on semantic errors with descriptive message)

---

## Unit & GUI Testing

* **Interpreter (Catch2 style from Project 2):** `test_interpreter.cpp` provided; add your own cases in `unittests.cpp`.
* **GUI (QTest):** `test_gui.cpp` provided; add your own cases in `gui_unittests.cpp`.
* **Do not modify** the two provided test files; extra cases there are ignored for coverage/score. Include everything in CMake.

---

## Building & Running (CMake + Docker reference environment)

### Standard build & ctest

```bash
# Inside the container
mkdir -p /mnt/build && cd /mnt/build
cmake ..
make
make test
```

This treats `/mnt` as the shared source directory; build output goes to `/mnt/build`. Executables produced: `postlisp`, `pldraw`, and test binaries `test_library`, `test_gui`.

### Integration test (CLI)

```bash
cd /mnt
mkdir -p build && cd build
cmake ..
make
python3 ../scripts/integration_test.py
```

Start pldraw after building:

```bash
cd /mnt/build
./pldraw
```

Stop with Ctrl-C (or OS window shortcuts). Reference binaries are in `/mnt/ref-binary` if you need to compare behavior.

---

## Beta Grading Runbook (what graders will run)

```bash
cd /mnt
mkdir build && cd build
cmake ..
make
./test_library    # postlisp library unit tests
./test_gui        # Qt GUI unit tests
```

Ensure **all unit and integration tests pass**.

---

## Final Grading Readiness (you’ll need these soon)

1. **Strict build (treat warnings as errors):**

```bash
cd /mnt
mkdir -p build && cd build
cmake -WERROR=TRUE ..
cmake --build . --config Debug
```

2. **Coverage ≥ 95% (line coverage in `coverage.xml`):**

```bash
cd /mnt
apt install -y dos2unix
dos2unix ./coverage.sh
chmod a+x coverage.sh
cmake -DCOVERAGE=TRUE ..
cmake --build . --config Debug
make coverage-grading
```

3. **Memory check (no leaks/errors):**

```bash
cd /mnt
mkdir -p build && cd build
cmake ..
make
valgrind --child-silent-after-fork=yes ./unittests
```

4. **Code quality (clang-tidy, fix all bad/warn):**

```bash
cd /mnt
apt install -y dos2unix
dos2unix code_quality.sh
chmod a+x code_quality.sh
./code_quality.sh
```



---

## Submission Requirements

Your **Git repository** must contain only source, tests, and configuration (Dockerfile, `CMakeLists.txt`, `tests/`, etc.)—the provided `.gitignore` already excludes IDE and build artifacts; **do not modify it**.

**Tag and push** the exact commit you want graded, then submit on Gradescope:

```bash
# Tag (use latest tag if multiple: beta, beta2, ... / final, final2, ...)
git tag beta        # or final
git push origin beta

# Then submit your GitHub Classroom repo to Gradescope
```

Late policy: −10% per day (up to 2 days), then no credit.

---

## Text-mode Program Notes (postlisp)

* Must **not** hard-code test cases.
* Three usage modes and exact REPL behavior/EOF keys are specified above; adhere to **output format** exactly.

---

## Implementation Constraints & Tips

* **C++11** compatible; **no Qt** in interpreter core.
* Compare floating-point values using `std::numeric_limits<double>::epsilon()`.
* The `QtInterpreter` must reuse (extend/compose) the interpreter logic, not duplicate it.
* The `draw` special form should be a **no-op outside GUI context** (returns `None`).

---

## Quick Docker Cheatsheet (build, run, test)

```bash
# Build everything
mkdir -p /mnt/build && cd /mnt/build
cmake ..
make

# Run CLI interpreter examples
./postlisp -e "(-2 4 point)"
./postlisp /mnt/tests/sample.slp

# Run GUI app
./pldraw
./pldraw /mnt/tests/test_car.slp

# Unit tests
ctest --output-on-failure
./test_library
./test_gui

# Integration test
python3 /mnt/scripts/integration_test.py
```

(These mirrors the reference environment commands in the spec.)

---

## Release Notes / Reference

* You can launch the provided reference binaries from `/mnt/ref-binary` to compare behavior (`chmod a+x *; ./postlisp; ./pldraw`).
* Minimum window size and event loop requirements are enforced for GUI grading.

---

## Beta Submission Checklist

* [ ] Implemented **Expression**, **tokenize**, **environment**, **interpreter** (no Qt)
* [ ] Implemented **GUI modules**: MainWindow, MessageWidget, CanvasWidget, REPLWidget, QtInterpreter, **QGraphicsArcItem**
* [ ] Implemented **draw**, graphical types & built-ins exactly as specified
* [ ] REPL behavior & **output format** matches spec; no hard-coding tests
* [ ] CMake builds: `postlisp`, `pldraw`, `test_library`, `test_gui`
* [ ] Unit tests added in `unittests.cpp`; GUI tests in `gui_unittests.cpp`; both included in CMake
* [ ] **Docker build + ctest** pass (`cmake .. && make && make test`)
* [ ] **Integration test** runs (`python3 ../scripts/integration_test.py`)
* [ ] Tagged commit as **`beta`** (or `beta2`, …), **pushed tag**, and **submitted to Gradescope**
* [ ] Included **AI Reflection Form** if applicable

---