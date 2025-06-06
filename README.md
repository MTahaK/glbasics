
# Model-View-Projection Notes:
| Matrix                | Conceptual Role                                                                                                                             | Practical Level You Need                                                                                                                                                                                |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Model Matrix**      | Places the object in the world. Applies translation, rotation, scaling, etc.                                                                | Understand that it's constructed in C++ using GLM. You build this up by applying transforms line-by-line. Know how different orders of transform calls affect the result.                               |
| **View Matrix**       | Moves the *camera* (or more accurately, moves the world relative to the camera). Think of it as the inverse of the camera’s transformation. | Understand that it's a transformation matrix representing the camera’s position/orientation. Even in 2D, you may eventually want a view matrix to implement camera movement (scrolling, zooming, etc.). |
| **Projection Matrix** | Defines what the camera *can* see and how it sees it. Maps world/view space into clip space.                                                | Know the difference between orthographic (2D, no depth scaling) and perspective (3D, with depth). Know what `glm::ortho()` does. Understand the resulting normalized device coordinates (NDC).          |

“Transforming into world space by applying the model matrix places the object correctly in the scene. This sets up the objective structure of the world. 
Then, the view matrix transforms this world as if we are standing at a particular location, looking in a specific direction — essentially setting up the 
camera's point of view. Finally, the projection matrix warps the camera view to simulate lens effects (like perspective or orthographic projection), 
bringing us into clip space.”

| Stage                  | Matrix           | From → To                          | Purpose                                                           |
| ---------------------- | ---------------- | ---------------------------------- | ----------------------------------------------------------------- |
| 🧱 Model               | `model`          | Object (local) space → World space | Place objects where they belong in the world                      |
| 🎥 View                | `view`           | World space → View (camera) space  | Reposition scene as if viewed from the camera                     |
| 📐 Projection          | `projection`     | View space → Clip space            | Apply lens-like transformation (e.g. orthographic or perspective) |
| 🔪 Clipping & Division | *(not a matrix)* | Clip space → NDC → screen          | Remove out-of-view fragments and normalize                        |

## Coordinate Transformation in OpenGL (2D Focused)

This section explains how coordinate transformations work in OpenGL, specifically in the context of 2D game development. It provides clear summaries and theory for the **Model**, **View**, and **Projection** matrices.

---

## View Matrix

The **view matrix** transforms coordinates from **world space** into **camera space**, representing the position and orientation of the camera observing the scene.

While we often imagine the camera moving through the world, what actually happens is that the **entire world is transformed in the opposite direction**. This is equivalent to applying the inverse of the camera's transformation to all objects.

### Purpose

* Defines **where the camera is**, **what it's looking at**, and **which direction is up**.
* Simulates a camera by transforming the world instead of moving the viewer.
* Comes after model transformations and before projection.

### Construction

For 3D:

```cpp
glm::mat4 view = glm::lookAt(cameraPos, targetPos, upVector);
```

For 2D (simplified translation only):

```cpp
glm::mat4 view = glm::translate(glm::mat4(1.0f), -cameraPosition);
```

> The minus sign moves the entire world in the opposite direction of the camera.

### Conceptual Flow

1. Model matrix places object in the world.
2. View matrix transforms world coordinates relative to the camera.
3. The object now exists in **camera space** (how the camera "sees" it).

---

## Projection Matrix

The **projection matrix** transforms coordinates from **camera space** into **clip space**. It defines how a 3D scene is projected onto a 2D plane.

It is required because OpenGL expects all vertices to ultimately lie within a standardized region: **clip space**, where `x`, `y`, and `z` ∈ `[-w, w]`.

### Purpose

* Defines the **visible area** of the scene.
* Ensures that geometry is **clipped** properly.
* Accounts for **window size and aspect ratio**.
* In 2D, it ensures square objects remain square regardless of resolution.

### Types of Projection

| Type         | Use Case     | Behavior                                                  |
| ------------ | ------------ | --------------------------------------------------------- |
| Orthographic | 2D games     | No perspective distortion; parallel lines remain parallel |
| Perspective  | 3D rendering | Objects shrink with distance (depth perception)           |

### Orthographic Projection (Common for 2D)

Creates a cuboid-shaped view volume that maps directly into clip space.

```cpp
glm::mat4 proj = glm::ortho(left, right, bottom, top);
```

With dynamic aspect ratio:

```cpp
float aspect = windowWidth / static_cast<float>(windowHeight);
glm::mat4 proj = glm::ortho(-aspect, aspect, -1.0f, 1.0f);
```

This keeps units consistent even if the window isn't square.

### How It Works

1. The projection matrix defines a **viewing volume** (orthographic box or perspective frustum).
2. Vertices are transformed into **clip space**: range `[-w, w]` in all axes.
3. The GPU performs **perspective division**: each component is divided by `w`.
4. The result is in **Normalized Device Coordinates (NDC)**: `x`, `y`, `z` ∈ `[-1, 1]`.
5. Anything outside `[-1, 1]` in any axis is **clipped**.

> In orthographic projection, `w` remains 1, so `x/w = x`, etc. This makes orthographic projection linear and distortion-free.

### Why This Matters in 2D

Without a proper projection matrix:

* Rectangles may appear squashed or stretched on non-square windows.
* There's no consistent mapping from your coordinate system to the actual window.

With orthographic projection:

* Your in-game units map cleanly to screen space.
* You can design logic in "world coordinates" without worrying about screen resolution distortion.

---

## MVP Order Clarification

GLM performs **right-multiplication** of matrices. So:

```cpp
glm::mat4 MVP = projection * view * model;
```

This means transformations are applied to your geometry in the following order:

1. **Model**: places the object.
2. **View**: simulates the camera.
3. **Projection**: defines the visible volume and maps it to clip space.

However, when composing a single matrix like `model`, GLM uses right multiplication:

```cpp
model = glm::mat4(1.0f);
model = glm::translate(model, ...);
model = glm::rotate(model, ...);
model = glm::scale(model, ...);
```

This results in a final matrix equivalent to:

```cpp
model = Translate * Rotate * Scale;
```

So the **first function call is the last transformation applied**, and the **last function call is the first transformation applied**.

**Key Rule**: The **order of appearance in code is reversed in terms of application**.

This behavior is consistent and predictable:

* You write transformations in logical order (translate, rotate, scale).
* GLM multiplies each new transformation on the **right**, so the last-written is the first-applied.

Always remember:

> **Right-multiplication in GLM means transformations are applied in reverse order from how they're written.**
---
## Camera Zoom in 2D Orthographic View

Zooming in a 2D orthographic projection is a natural extension of camera control and is especially useful in games like Terraria or when building a level editor.

### Concept

In orthographic projection, the visible area is defined by a box:

```cpp
glm::ortho(left, right, bottom, top);
```

To zoom, you simply adjust the size of this box. Enlarging the bounds zooms **out**, reducing them zooms **in**.

This avoids any distortion since orthographic projections preserve sizes and angles regardless of depth.

---

### Implementation Outline

#### Zoom Factor

```cpp
float zoom = 1.0f; // Default zoom level
```

#### Adjust Projection Matrix

```cpp
float aspect = static_cast<float>(width) / height;
float viewHeight = 1.0f * zoom;
float viewWidth = aspect * viewHeight;

projection = glm::ortho(-viewWidth, viewWidth, -viewHeight, viewHeight);
```

#### Input Control Example (Optional)

```cpp
if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    zoom -= 0.5f * deltaTime;
    if (zoom < 0.1f) zoom = 0.1f;
}
if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
    zoom += 0.5f * deltaTime;
    if (zoom > 10.0f) zoom = 10.0f;
}
```
---
## Framebuffer Size and DPI Scaling

### What Is the Framebuffer?

The **framebuffer** is a memory region where OpenGL renders each frame. It holds data such as:

- Color buffer
- Depth buffer
- Stencil buffer

When you create a window with GLFW, it automatically sets up a **default framebuffer** for you:

```cpp
glfwCreateWindow(1000, 1000, "Title", nullptr, nullptr);
```

You don’t manually create this framebuffer—it’s handled by GLFW, the OS, and the GPU driver.

---

### How Is the Framebuffer Size Determined?

The actual size of the framebuffer is determined by:

1. **Window size** — as specified in your `glfwCreateWindow(...)` call.
2. **Pixel density (DPI scaling)** — determined by your OS and GPU.

If the window is 800×600 and the DPI scaling is 2.0 (e.g. Retina display), the framebuffer will be **1600×1200**.

So:

```cpp
// May give different values!
glfwGetWindowSize(...);
glfwGetFramebufferSize(...);
```

Use `glfwGetFramebufferSize(...)` when computing aspect ratios for projection matrices.

---

### How Can Framebuffer Size Change?

- User resizes the window
- Window is dragged between monitors (with different DPI)
- DPI scaling setting is changed
- Fullscreen is toggled
- OS enforces zoom or scaling

You can respond to changes either by:

- **Polling** `glfwGetFramebufferSize` every frame (which you are doing), or
- **Reacting** via a registered callback:

```cpp
glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
});
```

This callback resizes the OpenGL viewport to match the new framebuffer size.

---

### Do You Need the Callback?

**No**, not if you’re already calling:

```cpp
int width, height;
glfwGetFramebufferSize(window, &width, &height);
```

...**every frame** inside your game/render loop. This gives you the current dimensions on demand.

The callback is only helpful if you:

- Want to avoid redundant recalculations
- Only care about size changes (not polling every frame)
- Are managing off-screen framebuffers or complex layouts

---

### Why It Matters

Framebuffer size affects:

- **Aspect ratio** of your scene
- **Projection matrix** calculation
- **Screen-space accuracy**

Without querying the true framebuffer size, your scene might stretch or squish due to incorrect aspect ratio handling.

---

### Summary Table

| Aspect                   | Details                                                   |
| ------------------------ | --------------------------------------------------------- |
| Who creates framebuffer? | GLFW + OS + GPU                                           |
| Size depends on?         | Window size × DPI scaling                                 |
| Can it change?           | Yes — resizing, DPI change, fullscreen toggle, etc.       |
| Update strategy?         | Use callback or poll `glfwGetFramebufferSize`             |
| Your setup               | ✅ Polling every frame — callback not needed              |
| Why it matters           | Ensures correct rendering resolution and projection setup |

---

<!-- # Mouse Input

### 1. 🔍 What Happens When You Move the Mouse?

When you move the mouse in a window:

- Your operating system knows where your cursor is in screen coordinates (typically top-left is (0,0)).
- GLFW tracks this and provides coordinates via glfwGetCursorPos(...).

However:

- These coordinates are in screen space, not OpenGL space.
- OpenGL expects inputs in Normalized Device Coordinates (NDC) for rendering (from [-1,1] in X and Y).
- So we must convert mouse position across multiple spaces.


| Space Name   | Range                       | Description                                           |
| ------------ | --------------------------- | ----------------------------------------------------- |
| Screen Space | `(0,0)` to `(width,height)` | Top-left origin. What `glfwGetCursorPos()` gives you. |
| NDC          | `[-1, 1]` in X and Y        | Post-projection space. OpenGL clips here.             |
| Clip Space   | `[-w, w]` before division   | Coordinates before perspective division.              |
| World Space  | Your own game’s coordinates | What your entities and logic use.                     |

We’ll go:
Mouse (screen space) → NDC → Clip Space → (via inverse MVP) → World Space

### 🛠️ Practical Implementation

#### Step 1: Get Mouse Position in Screen Coordinates

```cpp
double mouseX, mouseY;
glfwGetCursorPos(window, &mouseX, &mouseY);
```

- This gives mouseX, mouseY relative to top-left of window.
- But OpenGL expects coordinates with origin at center, and Y increasing up, not down.

#### Step 2: Convert to Normalized Device Coordinates (NDC)


```cpp
int width, height;
glfwGetFramebufferSize(window, &width, &height);

// Convert screen -> NDC
float xNDC = (2.0f * mouseX) / width - 1.0f;
float yNDC = 1.0f - (2.0f * mouseY) / height;
```
✅ Why this formula?

- Mouse X goes from [0, width] → map to [-1, 1].
- Mouse Y goes from [0, height] (top to bottom), but OpenGL Y goes bottom-to-top, so we subtract from 1.

 -->
# 🧠 Coordinate Conversion: From Screen Space to World Space

When you move the mouse, GLFW gives you coordinates like `(400, 300)` — **but these are not meaningful to OpenGL directly.** You need to go through multiple steps:

```
Screen Space → Normalized Device Coordinates (NDC)
NDC → Clip Space
Clip Space → World Space (via inverse MVP)
```

---

## 1️⃣ Screen Space to NDC

### 🔷 What is screen space?

GLFW returns mouse coordinates in **screen space**, relative to the top-left corner of the window:

- `(0, 0)` is top-left
- `(width, height)` is bottom-right

### 🔷 What does OpenGL want?

OpenGL uses **Normalized Device Coordinates (NDC)**, where:

- Center is `(0, 0)`
- Coordinates go from `-1` to `+1`
- Y is **flipped** compared to screen space

| Screen | →    | NDC  |
| ------ | ---- | ---- |
| 0      |      | -1   |
| width  |      | +1   |
| 0      |      | +1   |
| height |      | -1   |

### 🔢 Deriving the Formula

```cpp
xNDC = 2 * (x / width) - 1;
yNDC = 1 - 2 * (y / height);
```

This rescales `[0, width]` to `[-1, 1]` and flips the Y axis.

✅ **Why?** OpenGL uses centered NDC for rendering; we must match that format.

---

## 2️⃣ NDC to Clip Space

In OpenGL, clip space and NDC are the same after **perspective division**. We reconstruct the clip space coordinate:

```cpp
glm::vec4 clipPos = glm::vec4(xNDC, yNDC, 0.0, 1.0);
```

✅ **Why?** This gives us the format needed to reverse-transform using the MVP matrix.

---

## 3️⃣ Clip Space to World Space

Apply the inverse MVP transformation:

```cpp
glm::mat4 inverse = glm::inverse(MVP);
glm::vec4 worldPos = inverse * clipPos;
```

✅ **Why?** This reverses the full transform:  
`world → view → projection → clip`  
becomes  
`clip → inverse(projection * view * model) → world`

---

## 4️⃣ Divide by w (Perspective Division)

```cpp
glm::vec2 finalWorldPos = glm::vec2(worldPos) / worldPos.w;
```

✅ **Why?** To return from **homogeneous** to **Euclidean** space — this is required in all perspective transformations.

---

## 🔚 Summary

| Step         | Operation                    | Reason                     |
| ------------ | ---------------------------- | -------------------------- |
| Screen → NDC | Rescale to `[-1, 1]`, flip Y | Match OpenGL format        |
| NDC → Clip   | Embed into vec4              | Needed for MVP inversion   |
| Clip → World | Multiply by `inverse(MVP)`   | Undo projection/view/model |
| Divide by w  | Homogeneous → Euclidean      | Restore proper coordinate  |