import * as path from 'https://deno.land/std@0.195.0/path/mod.ts';

export const tokiBaseDir = path.resolve('../');

export const dependencyProjectsPaths = [
    path.join(tokiBaseDir, 'toki/vendor/glfw'), 
    path.join(tokiBaseDir, 'toki/vendor/imgui'),
]

export const projectPaths = [
    ...dependencyProjectsPaths,
    path.join(tokiBaseDir, 'toki'),
];