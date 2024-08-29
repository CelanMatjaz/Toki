function generateWaylandProtocolFiles()
    local WAYLAND_SCANNER_EXECUTABLE = "wayland-scanner"

    function generateFile(input, output, command)
        os.execute(WAYLAND_SCANNER_EXECUTABLE .. " " .. command .. " " .. input .. " " .. output)
    end

    local newDirectory = path.join(os.getcwd(), "includes", "glfw")
    os.mkdir(newDirectory)
    
    for file in io.popen([[find $(pwd)/glfw/deps/wayland -name *.xml]]):lines() do
        local directory = path.getdirectory(file)
        local name = path.getbasename(file)

        local filePath = path.join(directory, name)

        local clientFileName = name .. "-client-protocol.h"
        local codeFileName = name .. "-client-protocol-code.h"

        generateFile(file, path.join(newDirectory, clientFileName), "client-header")
        generateFile(file, path.join(newDirectory, codeFileName), "private-code")
    end
end


