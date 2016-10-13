:: Electron's version.
SET npm_config_target=1.4.2
:: The architecture of Electron, can be ia32 or x64.
SET npm_config_arch=x64
SET npm_config_target_arch=x64
:: Download headers for Electron.
SET npm_config_disturl=https://atom.io/download/atom-shell
:: Tell node-pre-gyp that we are building for Electron.
SET npm_config_runtime=electron
:: Tell node-pre-gyp to build module from source code.
SET npm_config_build_from_source=true
:: Install all dependencies, and store cache to ~/.electron-gyp.
SET "HOME=%HOME%/.electron-gyp"
npm run build
