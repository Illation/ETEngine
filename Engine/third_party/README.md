# Third party libraries



## bc7enc

- source: [https://github.com/richgel999/bc7enc](https://github.com/richgel999/bc7enc/tree/ffcc3a085f7345dee94ed5ee8a7427dbd06ac7b7)
- license: [MIT / Public Domain](https://github.com/richgel999/bc7enc/blob/master/LICENSE)


## Bullet
- source: [https://github.com/bulletphysics/bullet3](https://github.com/bulletphysics/bullet3/tree/66fc3a9ce94cbf9793898d9ebb11dec587670ada)
- license: [zlib](https://github.com/bulletphysics/bullet3/blob/master/LICENSE.txt)


## Catch 2
- source: [https://github.com/catchorg/Catch2](https://github.com/catchorg/Catch2/tree/af8b2538a62db34b3cec10ceb74e3d29bd06e37f)
- license: [boost](https://github.com/catchorg/Catch2/blob/master/LICENSE.txt)


## ConvectionKernels
- source: [https://github.com/elasota/ConvectionKernels](https://github.com/elasota/ConvectionKernels/tree/2f06a14b4ceb6c5234231f002154c4e4765c713b)
- license: [MIT](https://github.com/elasota/ConvectionKernels/blob/master/LICENSE.txt)


## glad
- source: [https://github.com/Dav1dde/glad](https://github.com/Dav1dde/glad/tree/2a5ac49ba7b5275e6f0f6d1fa07bfb0e6875ce18)
- license: [MIT](https://github.com/Dav1dde/glad/blob/master/LICENSE)

Glad uses a python interpreter which is obtained by Vcpkg (see below).


## GLFW
- source: [https://github.com/glfw/glfw](https://github.com/glfw/glfw/tree/7f02898264d6077738f0e8032f027eb7e252c90e)
- license: [zlib](https://github.com/glfw/glfw/blob/master/LICENSE.md)


## mikktspace
- source: [https://github.com/tcoppex/ext-mikktspace](https://github.com/tcoppex/ext-mikktspace/tree/f824a879858b7f843f900ac023fc9495b064ae5d)
- license: [zlib](https://opensource.org/licenses/Zlib)

Mirror of blender tangent space calculations: https://wiki.blender.org/index.php/Dev:Shading/Tangent_Space_Normal_Maps


## OpenAL-soft
- source: [https://github.com/kcat/openal-soft](https://github.com/kcat/openal-soft/tree/987fd13796d5761b7adc5f6e137e7a6149d02f7b)
- license: [LGPL](https://github.com/kcat/openal-soft/blob/master/COPYING)

This project dynamically links to openAl soft.


## RmlUi
- source: [https://github.com/mikke89/RmlUi](https://github.com/mikke89/RmlUi/tree/d56f17e49ca2ee88aadeb304228cd1eae14e3f51)
- license: [MIT](https://github.com/mikke89/RmlUi/blob/master/LICENSE.txt)


## Dear ImGui
- source: [https://github.com/ocornut/imgui](https://github.com/ocornut/imgui/tree/467a1cd4a583edd55db310c82e2823692616e108)
- license: [MIT](https://github.com/ocornut/imgui/blob/master/LICENSE.txt)


## RTTR
- source: [https://github.com/rttrorg/rttr](https://github.com/rttrorg/rttr/tree/34c4d0e889fb0fe721638975835e2505c1957539)
- license: [MIT](https://github.com/rttrorg/rttr/blob/master/LICENSE.txt)


## stb
- source: [https://github.com/nothings/stb](https://github.com/nothings/stb/tree/052dce117ed989848a950308bd99eef55525dfb1)
- license: [Public domain / MIT](https://opensource.org/licenses/MIT)

single file libraries being used:

##### stb_image
##### stb_image_resize
##### stb_image_write
##### stb_vorbis


## GTK-for-Windows-Runtime-Environment-Installer
- source: https://github.com/tschoonj/GTK-for-Windows-Runtime-Environment-Installer
- license: None provided

Used to copy some dlls for the Engine editor. No license is provided, so LGPL is assumed as that is the case for all other GTK libraries


## Vcpkg
- source: [https://github.com/microsoft/vcpkg](https://github.com/Microsoft/vcpkg/tree/3f6ab4834e8469bbe1f6569b7941c720f178b688)
- license: [MIT](https://github.com/microsoft/vcpkg/blob/master/LICENSE.txt)

Vcpkg is used to build and obtain sources of other libraries. It also downloads a python interpreter, which is used for gl context binding creation.

Libraries used by the runtime engine are:

#### libpng
LibPng License - http://www.libpng.org/pub/png/src/libpng-LICENSE.txt

#### zlib
ZLib License - https://www.zlib.net/zlib_license.html

#### bzip2
bzip2 license - https://sourceware.org/bzip2/

------------------------------------------------------------

Other libraries, which are used by the editor are:

#### atk
GNU LGPL License - https://developer.gnome.org/atk/

#### atkmm
GNU LGPL License - https://github.com/GNOME/atkmm/blob/master/COPYING

#### cairo
GNU LGPL License - https://www.cairographics.org/

#### cairomm-1.0
GNU LGPL License - https://www.cairographics.org/cairomm/

#### epoxy
MIT - https://github.com/anholt/libepoxy/blob/master/COPYING

#### expat
MIT - https://github.com/libexpat/libexpat/blob/master/expat/COPYING

#### fontconfig
MIT - https://www.freedesktop.org/wiki/Software/fontconfig/

#### freetype
FreeType License - https://www.freetype.org/license.html

#### gdk-pixbuf
GNU LGPL License - https://github.com/GNOME/gdk-pixbuf/blob/master/COPYING

#### glib
GNU LGPL License - https://github.com/GNOME/glib/blob/master/COPYING

#### glibmm
GNU LGPL License - https://github.com/GNOME/glibmm/blob/master/COPYING

#### gobject-2.0
GNU LGPL License - https://developer.gnome.org/gobject/

#### gtk-3.0
GNU LGPL License - https://github.com/GNOME/gtk/blob/master/COPYING

#### gtkmm
GNU LGPL License - https://www.gtkmm.org/en/license.html

#### harfbuzz
Old MIT - https://github.com/harfbuzz/harfbuzz/blob/master/COPYING

#### libffi
MIT - https://github.com/libffi/libffi/blob/master/LICENSE

#### libiconv
GNU LGPL License - https://www.gnu.org/software/libiconv/

#### libintl
GNU LGPL License - https://www.gnu.org/software/gettext/manual/html_node/GNU-LGPL.html#GNU-LGPL

#### pango
GNU LGPL License - https://github.com/GNOME/pango/blob/master/COPYING

#### pangomm
GNU LGPL License - https://github.com/GNOME/pangomm/blob/master/COPYING

#### pcre
BSD License - https://www.pcre.org/licence.txt

#### pixman-1
MIT - http://www.pixman.org/

#### sigc++
LGPL-3.0 - https://github.com/libsigcplusplus/libsigcplusplus/blob/master/COPYING

