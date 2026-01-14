# Build
If you want to build the project yourself, you will need to install [SFML](https://github.com/SFML/SFML.git) version **3.0.2**:

If a `sudo apt install libsfml-dev` doesn't install the correct version, remove it with a `sudo apt remove libsfml-dev`, then run the following:
```bash
cd ~
# Installs needed dependencies:
sudo apt update
sudo apt install -y build-essential cmake git libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev libopenal-dev libvorbis-dev libflac-dev libfreetype6-dev libjpeg-dev

# Gets the 3.0.2 version of SFML.
git clone --branch 3.0.2 https://github.com/SFML/SFML.git
cd SFML

# Builds the project
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_SHARED_LIBS=ON
make -j$(nproc)
sudo make install
sudo ldconfig
```