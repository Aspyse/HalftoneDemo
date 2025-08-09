<h1 align="center">
  Artist-Centered Approaches to Real-time Non-Photorealistic Rendering
</h1>

<h5 align="center">
 By Derek Royce Burias
</h5>

<p align="center">
 <img alt="GitHub last commit" src="https://img.shields.io/github/last-commit/Aspyse/HalftoneDemo">
 <a href="https://opensource.org/license/mit"><img src="https://img.shields.io/github/license/Aspyse/HalftoneDemo"></a>
</p>

<p align="center">User-friendly, no-code methods for developing custom real-time, non-photorealistic graphics. Tweak intuitive parameters and combine a prebuilt set of render passes to create your own stylized rendering effects.</p>


## Video Demo

https://github.com/user-attachments/assets/9de359e2-fe37-44cb-aa91-76d1b41bfaca


## Setup

### 1. Clone the repository

```sh
git clone https://github.com/Aspyse/HalftoneDemo.git
cd HalftoneDemo
```

### 2. Download the test models

The test models are not included in this repository due to their large file sizes. They may instead be found at [The Stanford 3D Scanning Repository](https://graphics.stanford.edu/data/3Dscanrep/) (`bun_zipper.ply`, `dragon_vrip.ply`, `lucy.ply`). The Sponza model may be downloaded from the [Intel GPU Research Sample Library](https://www.intel.com/content/www/us/en/developer/topic-technology/graphics-research/samples.html), then packed into a `.glb` using [gltfpack](https://github.com/zeux/meshoptimizer/releases).

Save the `.ply` and `.glb` files in `HalftoneDemo/Models`.


### 3. Configure and build

The repository includes CMake configuration presets. In a command prompt, run the configuration using:
```sh
cmake --preset x64-release
```
Then build using:
```sh
cmake --build out\build\x64-release
```


### 4. Run executable

In a command prompt, run:
```sh
.\out\build\x64-release\HalftoneDemo.exe
```


## Libraries Used

- [miniPLY](https://github.com/vilya/miniply)
- [fastGLTF](https://github.com/spnda/fastgltf)
- [DirectX Tool Kit](https://github.com/microsoft/DirectXTK)
