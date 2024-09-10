#!/usr/bin/nodejs
const net = require('net');
const fs = require('fs');
const path = require('path');
const readline = require('readline');

const VERSION = `Gopher File Server 1.1 (by NataliaPC'2024)`;
const PORT = 7070;

let rootDirectory = process.cwd(); // Default to current working directory

if (process.argv[2]) {
	rootDirectory = path.resolve(process.argv[2]);
}

const server = net.createServer((socket) => {
	socket.on('data', (data) => {
		const server = getServerIP();
		const request = data.toString().trim();
		const requestedPath = path.join(rootDirectory, request);

		console.log(`${getDate(socket)} -> ${request}`);

		fs.stat(requestedPath, (err, stats) => {
			if (err) {
				console.log(`${getDate()} -> ERROR: File or directory not found!`);
				printError(socket, 'File or directory not found!');
				return;
			}

			if (stats.isDirectory()) {
				fs.readdir(requestedPath, (err, files) => {
					if (err) {
						console.log(`${getDate()} -> ERROR: Unable to read directory!`);
						printError(socket, 'Unable to read directory!');
						return;
					}

					sendHeader(socket);
					println(socket, `Folder: ${request=='/'?request:'/'+request+'/'}`);
					println(socket, '');

					// Sort files, directories go first
					const sortedFiles = files.sort((a, b) => {
						const aPath = path.join(requestedPath, a);
						const bPath = path.join(requestedPath, b);
						const aIsDir = fs.statSync(aPath).isDirectory();
						const bIsDir = fs.statSync(bPath).isDirectory();
						
						if (aIsDir && !bIsDir) return -1;
						if (!aIsDir && bIsDir) return 1;
						return a.localeCompare(b);
					});

					sortedFiles.forEach(file => {
						const filePath = path.join(requestedPath, file);
						const fileStats = fs.statSync(filePath);
						const relativePath = path.relative(rootDirectory, filePath);
						let line;
						if (fileStats.isDirectory()) {
							line = `1<DIR>  ${file}\t${relativePath}\t${server}\t${PORT}\r\n`;
						} else {
							const fileSize = getFileSize(fileStats.size);
							line = `9 ${fileSize}  ${file}\t${relativePath}\t${server}\t${PORT}\r\n`;
						}
						socket.write(line);
					});
					socket.end('.');
				});
			} else {
				fs.readFile(requestedPath, (err, content) => {
					if (err) {
						console.log(`${getDate()} -> ERROR: Unable to read file!`);
						printError(socket, 'Unable to read file!');
						return;
					}
					socket.write(content);
					socket.end();
				});
			}
		});
	});
});

server.listen(PORT, () => {
	console.log(VERSION);
	console.log(`Running on port ${PORT}`);
	console.log(`Root directory: ${rootDirectory}`);
});

function getDate(socket) {
	let date = new Date();
	return  `${date.toISOString().slice(0,10)} `+
			`${date.toTimeString().slice(0,8)}.${date.getMilliseconds().toString().padStart(3, '0')} `+
			`[${socket.remoteAddress.replace(/^.*:/, '')}]`;
}

function sendHeader(socket) {
	const bannerPath = path.join(__dirname, 'banner.txt');
	
	if (fs.existsSync(bannerPath)) {
		printFile(bannerPath, socket);
	}
	println(socket, `${VERSION}`);
}

function println(socket, txt) {
	socket.write(`i${txt}\t-\t-\t-\r\n`);
}

function printFile(filePath, socket, showError = false) {
	try {
		const fileContent = fs.readFileSync(filePath, 'utf8');
		const lines = fileContent.split('\n');
		
		for (const line of lines) {
			println(socket, line.trimEnd());
		}
	} catch (err) {
		if (showError) {
			console.log(`${getDate()} -> ERROR: Unable to read file!`);
			printError(socket, 'Unable to read file!');
		}
	}
}

function printError(socket, txt) {
	socket.write(`3Error: ${txt}\tError\t(NULL)\t0\r\n`);
	socket.end('.');
}

function getServerIP() {
	const os = require('os');
	const interfaces = os.networkInterfaces();
	for (let iface of Object.values(interfaces)) {
		for (let alias of iface) {
			if (alias.family === 'IPv4' && !alias.internal) {
				return alias.address;
			}
		}
	}
	return '127.0.0.1'; // Fallback to localhost if no external IP is found
}

function getFileSize(size) {
	if (size < 1000) return `${size}b`.padStart(4);
	if (size < 1000 * 1000) return `${Math.floor(size / 1000)}K`.padStart(4);
	return `${Math.floor(size / (1000 * 1000))}M`.padStart(4);
}
