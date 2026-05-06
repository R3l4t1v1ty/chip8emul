#include "Chip8.hpp"

int SDL_main(int argc, char** argv)
{
	argc = 4;
	if (argc != 4)
	{
		//std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
		std::exit(EXIT_FAILURE);
	}

	int videoScale = 10;//std::stoi(argv[1]);
	int cycleDelay = 1;// std::stoi(argv[2]);
	char const* romFilename = "test.ch8";//argv[3];

	Platform platform("CHIP-8 Emulator", VIDEO_WIDTH * videoScale, VIDEO_HEIGHT * videoScale, VIDEO_WIDTH, VIDEO_HEIGHT);

	Chip8 chip8;
	chip8.load_ROM(romFilename);

	int videoPitch = sizeof(chip8.video[0]) * VIDEO_WIDTH;

	auto lastCycleTime = std::chrono::high_resolution_clock::now();
	bool quit = false;

	while (!quit)
	{
		quit = platform.process_input(chip8.keypad);

		auto currentTime = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

		if (dt > cycleDelay)
		{
			lastCycleTime = currentTime;

			chip8.Cycle();

			platform.update(chip8.video, videoPitch);
		}
	}

	return 0;
}