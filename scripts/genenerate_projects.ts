import { finalPremakeExec, downloadPremake } from './premake.ts';
import * as path from "https://deno.land/std@0.195.0/path/mod.ts";

// Download premake if not already in vendor folder
try {
    Deno.statSync(finalPremakeExec);
} catch (e) {
    await downloadPremake();
}

function generatePremakeProjects() {
    const premakeExec = path.resolve("../vendor/premake/premake5.exe");
    const premakeTarget = prompt("Input premake target (eg. gmake2, vs2022...), default: gmake2") || "gmake2";

    if (!premakeTarget) {
        console.log("Target was empty, exiting...");
        Deno.exit();
    }

    console.log("Generating solution");
    const cmd = new Deno.Command(premakeExec, {
        args: [premakeTarget],
        cwd: path.resolve(".."),
    });
    cmd.spawn();
    cmd.outputSync();
}

generatePremakeProjects();