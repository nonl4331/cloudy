alias r := run

configure:
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build --preset release

[default]
run:
	cmake --build build
	-./build/cloudy
