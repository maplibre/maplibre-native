// This script runs a Node.js server passed to it while running aother program.

const { spawn } = require('child_process')

if (process.argv.length < 4) {
	console.error("Usage: node with-server.js <path-to-server-js> <command-to-run> [arg]...");
	process.exit(1);
}

const serverScript = process.argv[2];

const prog = process.argv[3];
const args = process.argv.slice(4);

const sameStdio = { stdio: [process.stdin, process.stdout, process.stderr] }; 

const server = spawn('node', [serverScript]);

server.stdout.on('data', (data) => {
	const dataAsStr = data.toString();

	if (dataAsStr === "OK") {
		const test = spawn(prog, args, sameStdio);

		test.on('close', (code) => {
			console.error(`Sub process exited with code '${code}', killing server`);
			server.kill();
			process.exit(code);
		});
		return;
	}

	console.error(`[server] ${dataAsStr}`);
});
