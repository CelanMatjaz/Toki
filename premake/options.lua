newoption {
    trigger = "max-windows",
    value = "COUNT",
    description = "Set max windows for the application",
    default = "1"
}

if os.host() == "linux" then
    newoption {
        trigger = "display-server",
        value = "SERVER",
        description = "Display server to compile for",
        default="<Empty>",
        allowed = {
            { "<Empty>", "Let premake decide" },
            { "wayland", "Wayland" },
            { "x11", "X11" },
        }
    }
end
