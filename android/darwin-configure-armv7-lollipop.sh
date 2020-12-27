BDIR=build-armv7-lollipop
rm -rf $BDIR/*
mkdir -p $BDIR
cd $BDIR

export JAVA_HOME=/Library/Java/JavaVirtualMachines/jdk1.8.0_181.jdk/Contents/Home/
export ANDROID_SDK=/Users/Shared/Android/Sdk
export ANDROID_NDK=/Users/Shared/Android/android-ndk-r17c
export ANDROID_NATIVE_API_LEVEL=16
export ANDROID_SDK_MINVER=18
export ANDROID_SDK_TARGET=19
export QT_DIR=/Users/Shared/Qt/5.11.3/android_armv7

cmake ../.. -DCMAKE_SYSTEM_NAME=Android \
-DCMAKE_PREFIX_PATH=$QT_DIR \
-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
-DCMAKE_MAKE_PROGRAM=$ANDROID_NDK/prebuilt/darwin-x86_64/bin/make \
-DCMAKE_BUILD_TYPE=Release \
-DANDROID_ABI="armeabi-v7a" \
-DANDROID_SDK_MINVER=$ANDROID_SDK_MINVER \
-DANDROID_SDK_TARGET=$ANDROID_SDK_TARGET \
-DANDROID_NATIVE_API_LEVEL=$ANDROID_NATIVE_API_LEVEL \
-DQT_ANDROID_PLATFORM_LEVEL=$ANDROID_NATIVE_API_LEVEL \
-DQT_ANDROID_TOOLCHAIN_TOOL_PREFIX="arm-linux-androideabi" \
-DQT_ANDROID_SDK_ROOT=$ANDROID_SDK \
-DQT_ANDROID_NDK_ROOT=$ANDROID_NDK \
-DQT_ANDROID_QT_ROOT=$QT_DIR \
-DQT_ANDROID_SDK_BUILDTOOLS_REVISION="21.0.0" \
-DQt5Core_DIR=$QT_DIR/lib/cmake/Qt5Core \
-DQt5Gui_DIR=$QT_DIR/lib/cmake/Qt5Gui \
-DQt5Qml_DIR=$QT_DIR/lib/cmake/Qt5Qml \
-DQt5Network_DIR=$QT_DIR/lib/cmake/Qt5Network \
-DQt5Quick_DIR=$QT_DIR/lib/cmake/Qt5Quick \
-DQt5QuickControls2_DIR=$QT_DIR/lib/cmake/Qt5QuickControls2 \
-DQt5Xml_DIR=$QT_DIR/lib/cmake/Qt5Xml \
-DQt5Svg_DIR=$QT_DIR/lib/cmake/Qt5Svg \
-DQt5Widgets_DIR=$QT_DIR/lib/cmake/Qt5Widgets \
$@
