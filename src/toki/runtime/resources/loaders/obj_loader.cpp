#include <toki/runtime/resources/loaders/obj_loader.h>

namespace toki {

ObjData load_obj(const Path& path) {
	File file(path, FileMode::READ);
	u32 vertex_count{};
	u32 normal_count{};
	u32 texture_coord_count{};
	u32 face_count{};

	u32 read_count{};
	char buf[256]{};
	do {
		read_count = file.read_line(buf, 256);
		buf[read_count] = 0;

		if (toki::starts_with(buf, "#")) {
			continue;
		}

		if (toki::starts_with(buf, "v ")) {
			vertex_count++;
		} else if (toki::starts_with(buf, "vn ")) {
			normal_count++;
		} else if (toki::starts_with(buf, "vt ")) {
			texture_coord_count++;
		} else if (toki::starts_with(buf, "f ")) {
			face_count++;
		}
	} while (read_count > 0);

	DynamicArray<Vector3> vertices(vertex_count);
	DynamicArray<Vector3> normals(normal_count);
	DynamicArray<Vector2> texture_coords(texture_coord_count);
	DynamicArray<Vertex> vertex_data;
	vertex_data.reserve(face_count * 3);
	DynamicArray<u32> index_data;
	index_data.reserve(face_count * 3);
	vertex_count = normal_count = texture_coord_count = face_count = 0;

	file.seek(0, FileCursorStart::BEGIN);

	auto handle_face = [&](const char* face_string) {
		const char* temp = face_string;
		enum {
			VERTEX_INDEX,
			TEXTURE_COORD_INDEX,
			NORMAL_INDEX,
		};
		i32 indices[3]{};

		for (; is_space(*temp); ++temp) {}

		u32 slash_count = 0;
		for (u32 n = 0; !is_space(*temp); ++temp) {
			if (n == 3) {
				break;
			}

			u32 read_byte_count = atoi<i32>(temp, indices[slash_count]);
			++n;
			temp += read_byte_count;
			if (*temp == '/') {
				++slash_count;
			}
		}

		Vertex temp_vertex{};

		temp_vertex.position = vertices[indices[VERTEX_INDEX] - 1];
		if (indices[NORMAL_INDEX] > 0) {
			temp_vertex.normals = normals[indices[NORMAL_INDEX] - 1];
		} else {
			temp_vertex.normals = {};
		}
		if (indices[TEXTURE_COORD_INDEX] > 0) {
			temp_vertex.uv = texture_coords[indices[TEXTURE_COORD_INDEX] - 1];
		} else {
			temp_vertex.uv = {};
		}

		auto bootleg_hash = [&vertex_data](const Vertex& v) {
			i32 hash = v.position.length() * 100000 + v.normals.length() * 10000 + v.uv.length() * 100000;
			hash *= 97;
			hash = hash % (vertex_data.capacity() * 103);
			return hash;
		};

		auto hash = bootleg_hash(temp_vertex);

		// This is temporary until i make a map
		for (u32 i = 0; i < vertex_data.size(); i++) {
			Vertex& current = vertex_data[i];
			auto current_hash = bootleg_hash(current);
			if (current_hash == hash && temp_vertex == current) {
				index_data.push_back(i);
				goto ret;
			}
		}

		index_data.push_back(vertex_data.size());
		vertex_data.push_back(temp_vertex);
ret:
		return temp - face_string + 1;
	};

	char* temp{};
	f64 value{};
	do {
		read_count = file.read_line(buf, 256);
		buf[read_count] = 0;

		if (toki::starts_with(buf, "#")) {
			continue;
		}

		if (toki::starts_with(buf, "vn ")) {
			temp = buf + 2;
			Vector3& normal = normals[normal_count++];
			for (u32 i = 0; i < 3; i++) {
				u32 converted_count = atof(temp, value);
				temp += converted_count - 1;
				reinterpret_cast<f32*>(&normal)[i] = value;
			}
		} else if (toki::starts_with(buf, "v ")) {
			temp = buf + 1;
			Vector3& vertex = vertices[vertex_count++];
			for (u32 i = 0; i < 3; i++) {
				u32 converted_count = atof(temp, value);
				temp += converted_count - 1;
				reinterpret_cast<f32*>(&vertex)[i] = value;
			}
		} else if (toki::starts_with(buf, "vt ")) {
			temp = buf + 2;
			Vector2& texture_coord = texture_coords[texture_coord_count++];
			for (u32 i = 0; i < 2; i++) {
				u32 converted_count = atof(temp, value);
				temp += converted_count - 1;
				reinterpret_cast<f32*>(&texture_coord)[i] = value;
			}
		} else if (toki::starts_with(buf, "f ")) {
			temp = buf + 1;
			for (u32 i = 0; i < 3; i++) {
				temp += handle_face(temp) - 1;
			}
		}
	} while (read_count > 0);

	return { toki::move(vertex_data), toki::move(index_data) };
}

}  // namespace toki
