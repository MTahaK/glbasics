
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
