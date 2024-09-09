#!/usr/bin/nodejs
const http = require('http');
const fs = require('fs');
const path = require('path');

// Root directory for serving files
const rootDirectory = process.argv[2] ? path.resolve(process.argv[2]) : path.join(__dirname, '../dsk/');

// Port on which the server will run
const port = 3333;

// Create and HTTP server
const server = http.createServer((req, res) => {
	// Obtain the requested path
	const requestedPath = req.url;

	if (requestedPath == "/exit") {
		console.log('#### Server finished');
		res.statusCode = 410;	// Gone
		res.end();
		server.close();
		process.exit(0);
	}

	console.log(`${getDate()} -> [${req.method}] [HTTP/${req.httpVersion}] [${req.socket.remoteAddress.replace(/^.*:/, '')}]: ${requestedPath}`);

	// Build the complete file path
	const filePath = path.join(rootDirectory, requestedPath);
	// Verify that the requested path is within the root directory to avoid security issues
	if (!filePath.startsWith(rootDirectory)) {
		res.statusCode = 403; // Forbidden
		res.end('403 Forbidden');
		console.log(`${getDate()} << #### 403 Forbidden: ${requestedPath}`);
		return;
	}

	// Check if the file exists
	fs.stat(filePath, (err, stats) => {
		if (err || !stats.isFile()) {
			// if error or isn't an archive send 404 error
			res.statusCode = 404; // Not found
			res.end('404 Not Found');
			console.log(`${getDate()} << #### 404 File not found: ${requestedPath}`);
		} else {
			// if file exists, read and send it
			console.log(`${getDate()} << Start: ${requestedPath} [${stats.size} bytes]`);
			let startTime = Date.now();
			res.setHeader('Content-Length', stats.size);
			res.setHeader('Content-Type', 'application/octet-stream');
			res.on('finish', () => {
				let elapsedTime = (Date.now() - startTime); // time in ms
				let speed = (stats.size * 1000 / elapsedTime / 1024).toFixed(2); // speed in Kb/s
				console.log(`${getDate()} << End: ${speed} Kb/s`);
			})
			fs.createReadStream(filePath).pipe(res);
		}
	});
});

// Start the server
server.listen(port, () => {
	console.log(`#### Server listening at http://localhost:${port}/`);
	console.log(`#### Root directory: ${rootDirectory}`);
});

function getDate() {
	let date = new Date();
	return date.toISOString().slice(0,10)+" "+date.toTimeString().slice(0,8)+"."+date.getMilliseconds().toString().padStart(3, '0');
}
