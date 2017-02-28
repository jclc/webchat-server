# Cool Webchat

This is a WebSockets based webchat server with a simple static HTML browser client. You can join chatrooms or create new ones. Everything is connected via a JSON API, so external clients are also possible.

## Requirements

* Linux 2.6 or newer
* C++11 compliant compiler
* SQLite3 (plans to change to Postgres)
* CMake 3.1.0 or newer
* GNU Make or equivalent

## Dependencies

* WebSocket++ 0.7.0
* JSON for Modern C++
* Asio
These dependencies are located in "external/" and are included in the repository or fetched with git.

## Building

First, clone the repository `git --recursive`. The '--recursive' switch is required for external dependencies.

Create a build directory, for example with `mkdir build`. Run `cmake ..` in the build directory to create makefiles. Build with `make`.

## Running

Run `webchat-server`. You must run in the build directory so that the application can create databases and find index.html.

Change the port with the `-p` or `--port` switch. The default port is 80.

Display help message or version string with `-h`|`--help` or `-V`|`--version` respectively.

Shut down the application with either ctrl+c or by sending a SIGINT signal, for example using `htop` or `killall -s 2 webchat-server`. The program will handle the signal terminate cleanly.

## License

This program is licensed under the GNU General Public License 2. External software in the "external/" directory is licensed under their respective licenses.

## Disclaimer

Although this program should be safe from SQL injections or cross-site scripting attacks, it comes with no guarantee. The creator(s) of this software are not liable for any harm the user may experience.
