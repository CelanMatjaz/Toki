import { dependencyProjectsPaths } from "./projects.ts";


function buildDependencyProjects() {
    for (const projectDir of dependencyProjectsPaths) {
        console.log('Building project', projectDir);
        const cmd = new Deno.Command('make', {
            args: [],
            cwd: projectDir,
        });
        cmd.spawn();
        cmd.outputSync();
    }   
}

buildDependencyProjects();