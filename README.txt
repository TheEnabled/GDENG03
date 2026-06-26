========================================================================
    DirectX 11 Rendering System Test Cases || Sol Andrei William Tabboga
========================================================================

HOW TO RUN THE PROGRAM:
1. Open the Visual Studio Solution file (or open the project folder in Visual Studio).
3. Build the solution (Ctrl + Shift + B) and Run the Local Windows Debugger (F5).
4. Use the W, A, S, D keys and Mouse to navigate the 3D scene (unless the active test case has disabled movement to mimic a fixed camera perspective).

HOW TO SWITCH TEST CASES:
Only ONE test case should be active at a time.
1. Open `AppWindow.cpp` and navigate to the `AppWindow::onUpdate()` function.
2. You will see a tracker comment block at the top of the file listing all 7 cases.
3. Scroll down to the Test Case you want to run. Uncomment its code block (by removing `/*` and `*/`), and make sure all other Test Case blocks are wrapped in `/* ... */`.
4. IMPORTANT: Open `PixelShader.hlsl` and activate the correct shader for your chosen case:
    - Rainbow Shader: For Cases 1, 3, 4, and 5.
    - White Shader (with directional lighting): For Cases 2, 6, and 7.

ENTRY CLASS FILE AND MAIN FUNCTION:
- The entry point of the application is located in `main.cpp`.
- `main.cpp` contains the standard `int main()` function.
- The main function initializes an instance of `AppWindow` (which serves as the core application engine class) and runs its main message pump via `app.broadcast()`. All the rendering logic for the objects and test cases is handled inside `AppWindow.cpp`.
