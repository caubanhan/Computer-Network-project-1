all:
	g++ -o client_app Client.cpp ClientLauncher.cpp RtpReceiver.cpp VideoDisplay.cpp -Llib -lmingw32 -lSDL2main -lSDL2 -mwindows -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -lws2_32 -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -static-libgcc -static-libstdc++ -lSDL2_ttf

runHD:
	./client_app.exe 127.0.0.1 8089 25000 sample_1280x720.mjpeg | cat
runFHD:
	./client_app.exe 127.0.0.1 8089 25000 sample_1920x1080.mjpeg | cat
runSD:
	./client_app.exe 127.0.0.1 8089 25000 movie.mjpeg | cat
run2K:
	./client_app.exe 127.0.0.1 8089 25000 sample_2560x1440.mjpeg | cat