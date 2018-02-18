# Features

## Working

### Graphics
 * Physically Based Rendering
 * HighDynamicRange Environment maps
 * Image Based Lighting
 * Deferred and Forward rendering
 * Lights rendered as Light Volumes
 * Bloom with texture downsampling
 * Stereoscopic Normal map encoding and decoding for optimal use of G-Buffer
 * Mesh Filter -> Vertex buffer building depending on material needs
 * Basic Statet Managment
 * Render Pipeline Object
 * Frustum culling
 * Sprite font rendering
 * Sprite rendering
 * Primitive renderer
 * GLSL shader preprocessing with includes and vertex / geo / fragment specializations
 * FXAA (Fast approximate Antialiasing)
 
### Planets
 * Custom triangle CDLOD
 * Triangle tree with frustum culling
 * Icosphere generation
 * Star data 
 
### General
 * Scenegraph Entity Hirachy 
 * Entity Component System 
 * Singleton Pattern for things like WINDOW, INPUT, TIME, CAMERA etc 
 * Observer Pattern for input 
 * Factory Pattern for Content Managment 
 * Performance measurment
 * Custom Maths Library: Vectors, Matrices, Quaternions, Transform and utility functions, Geometry
 * Specific atomic types ( int32, uint16 ... )
 * Platform agnostic file system
 * JSON parser
 * Read engine settings from file
 * Rigid body Physics with BULLET
 * Audio System with OpenAL
     * AudioListenerComponent
     * AudioSourceComponent
     * Loading .ogg(vorbis) and .wav(pcm) files
 * Logging system for console, file and debug

### Integration
 * Project Generation with GENie
 * Continous Integration with AppVeyor
 * Unit Testing setup with catch.hpp
 * Unit tests for math functions
 * Unit tests for the file system
 
## Work in progress

### Graphics
 * Shadow Mapping
 * SSR
 * Debug renderer

### Planets
 * Atmospheric Scattering
 
### General
 * UI System
 * Debug Overlay
     * Debug Overlay as Editor build mode
 
## Backlog

### Graphics
 * Effects
     * SSAO
     * Motion Blur
     * TAA
 * Api / Pipeline
     * Render state is pushed right before draw call
     * Allow better Graphics debugging support by tagging
     * Abstract OpenGL for Shader and Framebuffer
     * Vulkan??
     * Sorting objects before rendering
     * batching 
 * UI
     * Dynamic font file rasterization (any size)
     * UI Space definition and pipeline stage
 * Utility
     * Generate some common shaders from code
     * Basic deferred fragment shader generation
     * Cone Lights/Culling/Primitive
 * Transparent shadows
 * Particle Systems
 * Animation support
 
### Planets
 * World moves around camera
 * Reference frame system
 * Generate local physics heightfield
 * Clouds / Fog
 * Trees / grass etc
 * Near ground Detail / Detail data
 * Ocean sim
 * Vectorized streets / fields
 * Effects such as Lightning, Aurora Borealis, Atmospheric glow
 * Biome Separation
 * Weather particles
 * God rays
 * occlusion culling
 
### General
 * Serialize Levels
 * Packaging system
 * Binary asset file types
 * Linux support
     * Travis CI Setup
 * Screenshot manager
 * SIMD math
 * Multithreading support
 * Loading screen / splash screen
 
### Integration
 * Code coverage measurment
 * Test more engine code