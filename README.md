
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

