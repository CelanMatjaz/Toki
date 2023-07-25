import * as path from 'https://deno.land/std@0.195.0/path/mod.ts';
import { projectPaths } from './projects.ts';

const premakeExec = path.resolve('../vendor/premake/premake5.exe');

const premakeTarget = confirm('Would you like to clean all premake projects?');

if (!premakeTarget) {
    console.log('Exiting...');
    Deno.exit();
}

for (const projectDir of projectPaths) {
    console.log('Cleaning project', projectDir);
    const cmd = new Deno.Command(premakeExec, {
        args: ['clean'],
        cwd: projectDir,
    });
    cmd.spawn();
    cmd.outputSync();
}
