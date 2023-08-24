# https://github.com/platformio/platformio-core/issues/4181

import os

if os.path.isfile(".gitignore"):
    os.remove(".gitignore")
