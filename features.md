# Features

## Working

### Graphics
 * Basic Physically Based Rendering
 * HighDynamicRange Environment maps
 * Image Based Lighting
 * Deferred and Forward rendering
 * Lights rendered as Light Volumes
 * Shadow Mapping
 * Bloom with texture downsampling
 * Stereoscopic Normal map encoding and decoding for optimal use of G-Buffer
 * Mesh Filter -> Vertex buffer building depending on material needs
 * Basic Statet Managment
 * Render Pipeline Object
 * Frustum culling
 * Sprite font rendering
 * Primitive renderer
 
### Planets
 * Custom triangle CDLOD
 * Triangle tree with frustum culling
 * Icosphere generation
 
### General
 * Scenegraph Entity Hirachy 
 * Entity Component System 
 * Singleton Pattern for things like WINDOW, INPUT, TIME, CAMERA etc 
 * Observer Pattern for input 
 * Factory Pattern for Content Managment 
 * Performance measurment
 * GLSL shader preprocessing with includes and vertex / geo / fragment specializations

 
## Work in progress

### Planets
 * Atmospheric Scattering

 
## Backlog

### Graphics
 * SSAO
 * SSR
 * Render state is pushed right before draw call
 
### Planets
 * World moves around camera
 * Reference frame system
 * Send back vertices for physicalization
 
### General
 * Physics Library (PhysX?)
 * Sounds (FMOD?)
