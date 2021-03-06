# HyperTiler (Version Alpha)
Tiled dataset conversion tool w/ web interface

<img src="https://raw.githubusercontent.com/bennywwg/HyperTiler/main/media.png">(Alpha User Interface)

### Description:
HyperTiler is a tool used for manipulating large tiled datasets. The principle operation is conversion of a tiled dataset, composed of a large set of images associated with 2D coordinates, and generating resized and downscaled images that form a new tiled dataset.

The default configuration will convert NASA's SRTM dataset (hosted from spkit.org) into an equivalent dataset with smaller tiles, in 16 bit PNG format.

### Prerequisites:
- Visual Studio 2019 (Or compatible build system)
- node / npm

### How to use:
- Pull down the repo and build the solution
- Run the built executable from the desired output directory
- Open localhost:5000 and configure the program settings
- Click "Process Dataset" to start the conversion

### Todo list (feel free to submit a PR!):
- Remove any unnecessary files from the git repo
- Make the npm build step more comprehensive (currently it is just the shell command npm install; npm run build)
- Implement unimplemented features (custom endianness, image formats, gamma, multi-channel support, etc)
- Show conversion progress in the web interface
- Add a button or mode to export a single tile, not an entire dataset
- Make the output pattern more cache-friendly
- Fix all bugs (of which there are many)
