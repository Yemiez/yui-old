
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(Freetype REQUIRED)
find_package(glm REQUIRED)
#find_package(glad CONFIG REQUIRED)

add_library(yui STATIC
        Application.h Application.cpp
        Badge.h
        Clipboard.h Clipboard.cpp
        Datetime.h Datetime.cpp
        FrameTimer.h
        includes.h
        Optional.h
        Painter.h Painter.cpp
        ResourceLoader.h ResourceLoader.cpp
        Stream.h Stream.cpp
        Types.h
        Utf8String.h Utf8String.cpp
        Util.h Util.cpp
        Vector.h
        Window.h Window.cpp
        io/Profiler.h
        layout/Box.h layout/Box.cpp
        layout/BoxCompute.h layout/BoxCompute.cpp
        layout/DocumentWidget.h layout/DocumentWidget.cpp
        layout/EditorEngine.h layout/EditorEngine.cpp
        layout/Inline.h layout/Inline.cpp
        layout/InlineBox.h layout/InlineBox.cpp
        layout/InlineCompute.h layout/InlineCompute.cpp
        layout/Input.h layout/Input.cpp
        layout/LayoutNode.h layout/LayoutNode.cpp
        layout/LayoutTreeDumper.h layout/LayoutTreeDumper.cpp
        layout/PainterUtilities.h layout/PainterUtilities.cpp
        layout/Textarea.h layout/Textarea.cpp
        ymd/DocumentLexer.h ymd/DocumentLexer.cpp
        ymd/DocumentNode.h ymd/DocumentNode.cpp
        ymd/DocumentParser.h ymd/DocumentParser.cpp
        ymd/Node.h ymd/Node.cpp
        yss/Computed.h yss/Computed.cpp
        yss/Selector.h yss/Selector.cpp
        yss/StyleHelper.h yss/StyleHelper.cpp
        yss/StylesheetDeclaration.h yss/StylesheetDeclaration.cpp
        yss/StylesheetLexer.h yss/StylesheetLexer.cpp
        yss/StylesheetParser.h yss/StylesheetParser.cpp
        )
target_link_libraries(yui PUBLIC
        fmt::fmt
        spdlog::spdlog
        glfw
        GLEW::GLEW
        Freetype::Freetype
        glm::glm
        )