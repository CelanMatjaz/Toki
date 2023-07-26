import * as path from 'https://deno.land/std@0.195.0/path/mod.ts';
import { ensureDir } from 'https://deno.land/std@0.195.0/fs/ensure_dir.ts';
import { unZipFromURL } from 'https://deno.land/x/zip@v1.1.0/mod.ts';

const premakeDownloadUrl =
    'https://github.com/premake/premake-core/releases/download/v5.0.0-beta2/premake-5.0.0-beta2-windows.zip';
const tempDir = path.resolve('./temp');
const tempPremakeExec = path.join(tempDir, 'premake5.exe');
const premakeDir = path.resolve('../vendor/premake/');
export const finalPremakeExec = path.join(premakeDir, 'premake5.exe');

export async function downloadPremake() {
    try {
        Deno.statSync(finalPremakeExec);
        Deno.exit();
    } catch (e) {}

    {
        const downloadZipConfirmation = confirm(`Would you like to download premake from ${premakeDownloadUrl}?`);

        if (!downloadZipConfirmation) {
            console.log('Cancelling premake download...');
            Deno.exit();
        } else {
            console.log('Proceeding with download...');
        }

        console.log(`Downloading and extracting file ${premakeDownloadUrl}...`);
        await unZipFromURL(premakeDownloadUrl, "./temp");
    }

    {
        try {
            await Deno.lstat(finalPremakeExec);
        } catch (err) {
            const premakeDirCreationConfirmation = confirm(
                `Would you like to create directory ${premakeDir} and move premake inside it?`
            );

            if (!premakeDirCreationConfirmation) {
                console.log('Cancelling premake directory creation...');
                Deno.exit();
            }

            await ensureDir(premakeDir);
            await Deno.rename(tempPremakeExec, finalPremakeExec);
            await Deno.remove(tempDir, { recursive: true });
        }
    }
}
