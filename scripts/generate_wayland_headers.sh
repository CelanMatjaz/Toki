if [[! command -v "wayland-scanner" &> /dev/null ]]; then
    echo "wayland-scanner does not exist"
    exit 1
fi

output_dir="vendor/includes/glfw"
glfw_dir="vendor/glfw/deps/wayland"

mkdir -p "$output_dir"

for file in vendor/glfw/deps/wayland/*; do
    full_path="$glfw_dir/$(basename "$file")"

    basename="$(basename "$file")"
    basename_without_extension="${basename%.*}"

    client_path="${output_dir}/${basename_without_extension}-client-protocol.h"
    code_path="${output_dir}/${basename_without_extension}-client-protocol-code.h"

    wayland-scanner "client-header" "$full_path" "$client_path"
    wayland-scanner "private-code" "$full_path" "$code_path"
done
