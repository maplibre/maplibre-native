// This script runs a Node.js server passed to it while running aother program.

const { spawn } = require('child_process')

if (process.argv.length < 4) {
	console.error("Usage: node with-server.js <path-to-server-js> <command-to-run> [arg]...");
	process.exit(1);
}

const serverScript = process.argv[2];

const prog = process.argv[3];
const args = process.argv.slice(4);

const server = spawn('node', [serverScript])

server.stdout.on('data', (data) => {
	if (data.toString() === "OK") {
		const test = spawn(prog, args, {
			stdio: [process.stdin, process.stdout, process.stderr]
		});


		test.on('close', (code) => {
			server.kill();
			process.exit(code);
		})
	}
});
