#include "Chip8.hpp"


uint8_t fontsett[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void Chip8::load_ROM(char const* fname)
{
	std::ifstream file(fname, std::ios::binary | std::ios::ate);

	if (file.is_open())
	{
		std::streampos size = file.tellg();
		char* buffer = new char[size];

		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();

		for (long i = 0; i < size; i++)
		{
			memory[START_ADDRESS + i] = buffer[i];
		}
		delete[] buffer;
	}
}

Chip8::Chip8() : rand_gen((unsigned int)std::chrono::system_clock::now().time_since_epoch().count())
{
	pc = START_ADDRESS;

	for (int i = 0; i < FONTSET_SIZE; i++)
	{
		memory[FONTSET_START_ADDRESS + i] = fontsett[i];
	}
	rand_byte = std::uniform_int_distribution<int>(0, 255U);

	table[0x0] = &Chip8::Table0;
	table[0x1] = &Chip8::op_1nnn;
	table[0x2] = &Chip8::op_2nnn;
	table[0x3] = &Chip8::op_3xkk;
	table[0x4] = &Chip8::op_4xkk;
	table[0x5] = &Chip8::op_5xy0;
	table[0x6] = &Chip8::op_6xkk;
	table[0x7] = &Chip8::op_7xkk;
	table[0x8] = &Chip8::Table8;
	table[0x9] = &Chip8::op_9xy0;
	table[0xA] = &Chip8::op_Annn;
	table[0xB] = &Chip8::op_Bnnn;
	table[0xC] = &Chip8::op_Cxkk;
	table[0xD] = &Chip8::op_Dxyn;
	table[0xE] = &Chip8::TableE;
	table[0xF] = &Chip8::TableF;

	for (size_t i = 0; i <= 0xE; i++)
	{
		table0[i] = &Chip8::op_NULL;
		table8[i] = &Chip8::op_NULL;
		tableE[i] = &Chip8::op_NULL;
	}

	table0[0x0] = &Chip8::op_00E0;
	table0[0xE] = &Chip8::op_00EE;

	table8[0x0] = &Chip8::op_8xy0;
	table8[0x1] = &Chip8::op_8xy1;
	table8[0x2] = &Chip8::op_8xy2;
	table8[0x3] = &Chip8::op_8xy3;
	table8[0x4] = &Chip8::op_8xy4;
	table8[0x5] = &Chip8::op_8xy5;
	table8[0x6] = &Chip8::op_8xy6;
	table8[0x7] = &Chip8::op_8xy7;
	table8[0xE] = &Chip8::op_8xyE;

	tableE[0x1] = &Chip8::op_ExA1;
	tableE[0xE] = &Chip8::op_Ex9E;

	for (size_t i = 0; i <= 0x65; i++)
	{
		tableF[i] = &Chip8::op_NULL;
	}

	tableF[0x07] = &Chip8::op_Fx07;
	tableF[0x0A] = &Chip8::op_Fx0A;
	tableF[0x15] = &Chip8::op_Fx15;
	tableF[0x18] = &Chip8::op_Fx18;
	tableF[0x1E] = &Chip8::op_Fx1E;
	tableF[0x29] = &Chip8::op_Fx29;
	tableF[0x33] = &Chip8::op_Fx33;
	tableF[0x55] = &Chip8::op_Fx55;
	tableF[0x65] = &Chip8::op_Fx65;
}



void Chip8::op_00E0()
{
	memset(video, 0, sizeof(video));
}

void Chip8::op_00EE()
{
	sp--;
	pc = stack[sp];
}

void Chip8::op_1nnn()
{
	uint16_t address = opcode & 0x0FFFu;
	pc = address;
}

void Chip8::op_2nnn()
{
	uint16_t address = opcode & 0x0FFFu;
	stack[sp] = pc;
	pc = address;
}

void Chip8::op_3xkk()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] == byte)
	{
		pc += 2;
	}
}

void Chip8::op_4xkk()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] != byte)
	{
		pc += 2;
	}
}

void Chip8::op_5xy0()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] != registers[Vy])
	{
		pc += 2;
	}
}

void Chip8::op_6xkk()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] = byte;
}

void Chip8::op_7xkk()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] += byte;
}

void Chip8::op_8xy0()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] = registers[Vy];
}

void Chip8::op_8xy1()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] |= registers[Vy];
}

void Chip8::op_8xy2()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] &= registers[Vy];
}

void Chip8::op_8xy3()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] ^= registers[Vy];
}

void Chip8::op_8xy4()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint16_t sum = registers[Vx] + registers[Vy];
	
	registers[0xF] = 0;

	if (sum > 255U)
	{
		registers[0xF] = 1;
	}

	registers[Vx] = sum & 0xFFu;
}

void Chip8::op_8xy5()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[0xF] = 0;

	if (registers[Vx] > registers[Vy])
	{
		registers[0xF] = 1;
	}

	registers[Vx] -= registers[Vy];
}

void Chip8::op_8xy6()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[0xF] = (registers[Vx] & 0x1u);

	registers[Vx] >>= 1;
}

void Chip8::op_8xy7()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[0xF] = 0;

	if (registers[Vy] > registers[Vx])
	{
		registers[0xF] = 1;
	}

	registers[Vx] = registers[Vy] - registers[Vx];
}

void Chip8::op_8xyE()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

	registers[Vx] <<= 1;
}

void Chip8::op_9xy0()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] != registers[Vy])
	{
		pc += 2;
	}
}

void Chip8::op_Annn()
{
	uint16_t address = opcode & 0x0FFFu;

	index = address;
}

void Chip8::op_Bnnn()
{
	uint16_t address = opcode & 0x0FFFu;
	pc = registers[0] + address;
}

void Chip8::op_Cxkk()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] = rand_byte(rand_gen) & byte;
}

void Chip8::op_Dxyn()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	uint8_t xpos = registers[Vx] % VIDEO_WIDTH;
	uint8_t ypos = registers[Vy] % VIDEO_HEIGHT;

	registers[0xF] = 0;

	for (unsigned int row = 0; row < height; row++)
	{
		uint8_t sprite_byte = memory[index + row];
		for (unsigned int col = 0; col < 8; col++)
		{
			uint8_t sprite_pixel = sprite_byte & (0x80u >> col);
			uint32_t* screen_pixel = &video[(ypos + row) * VIDEO_WIDTH + (xpos + col)];

			if (sprite_pixel)
			{
				if (*screen_pixel == 0xFFFFFFFF)
				{
					registers[0xF] = 1;
				}

				*screen_pixel ^= 0xFFFFFFFF;
			}
		}
	}
}

void Chip8::op_Ex9E()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t key = registers[Vx];

	if (keypad[key])
	{
		pc += 2;
	}
}

void Chip8::op_ExA1()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t key = registers[Vx];

	if (!keypad[key])
	{
		pc += 2;
	}
}

void Chip8::op_Fx07()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[Vx] = delay_timer;
}

void Chip8::op_Fx0A()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	if (keypad[0])
	{
		registers[Vx] = 0;
	}
	else if (keypad[1])
	{
		registers[Vx] = 1;
	}
	else if (keypad[2])
	{
		registers[Vx] = 2;
	}
	else if (keypad[3])
	{
		registers[Vx] = 3;
	}
	else if (keypad[4])
	{
		registers[Vx] = 4;
	}
	else if (keypad[5])
	{
		registers[Vx] = 5;
	}
	else if (keypad[6])
	{
		registers[Vx] = 6;
	}
	else if (keypad[7])
	{
		registers[Vx] = 7;
	}
	else if (keypad[8])
	{
		registers[Vx] = 8;
	}
	else if (keypad[9])
	{
		registers[Vx] = 9;
	}
	else if (keypad[10])
	{
		registers[Vx] = 10;
	}
	else if (keypad[11])
	{
		registers[Vx] = 11;
	}
	else if (keypad[12])
	{
		registers[Vx] = 12;
	}
	else if (keypad[13])
	{
		registers[Vx] = 13;
	}
	else if (keypad[14])
	{
		registers[Vx] = 14;
	}
	else if (keypad[15])
	{
		registers[Vx] = 15;
	}
	else
	{
		pc -= 2;
	}
}

void Chip8::op_Fx15()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	delay_timer = registers[Vx];
}

void Chip8::op_Fx18()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	sound_timer = registers[Vx];
}

void Chip8::op_Fx1E()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	index += registers[Vx];
}

void Chip8::op_Fx29()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = registers[Vx];

	index = FONTSET_START_ADDRESS + (5 * digit);
}

void Chip8::op_Fx33()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = registers[Vx];

	for (int i = 0; i < 3; i++)
	{
		memory[index + 2 - i] = value % 10;
		value /= 10;
	}
}

void Chip8::op_Fx55()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		memory[index + i] = registers[i];
	}
}

void Chip8::op_Fx65()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		registers[i] = memory[index + i];
	}
}

void Chip8::Cycle()
{
	opcode = (memory[pc] << 8u) | memory[pc + 1];

	pc += 2;

	((*this).*(table[(opcode & 0xF000u) >> 12u]))();

	if (delay_timer > 0)
	{
		delay_timer--;
	}

	if (sound_timer > 0)
	{
		sound_timer--;
	}
}

Platform::Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight)
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_CreateWindowAndRenderer("Hello World", 800, 600, SDL_WINDOW_FULLSCREEN, &window, &renderer);

	texture = SDL_CreateTexture(
		renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight);
}

Platform::~Platform()
{
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Platform::update(void const* buffer, int pitch)
{
	SDL_UpdateTexture(texture, nullptr, buffer, pitch);
	SDL_RenderClear(renderer);
	SDL_RenderTexture(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
}

bool Platform::process_input(uint8_t* keys)
{
	bool quit = false;

	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_EVENT_QUIT:
		{
			quit = true;
		} break;

		case SDL_EVENT_KEY_DOWN:
		{
			switch (event.key.key)
			{
			case SDLK_ESCAPE:
			{
				quit = true;
			} break;

			case SDLK_X:
			{
				keys[0] = 1;
			} break;

			case SDLK_1:
			{
				keys[1] = 1;
			} break;

			case SDLK_2:
			{
				keys[2] = 1;
			} break;

			case SDLK_3:
			{
				keys[3] = 1;
			} break;

			case SDLK_Q:
			{
				keys[4] = 1;
			} break;

			case SDLK_W:
			{
				keys[5] = 1;
			} break;

			case SDLK_E:
			{
				keys[6] = 1;
			} break;

			case SDLK_A:
			{
				keys[7] = 1;
			} break;

			case SDLK_S:
			{
				keys[8] = 1;
			} break;

			case SDLK_D:
			{
				keys[9] = 1;
			} break;

			case SDLK_Z:
			{
				keys[0xA] = 1;
			} break;

			case SDLK_C:
			{
				keys[0xB] = 1;
			} break;

			case SDLK_4:
			{
				keys[0xC] = 1;
			} break;

			case SDLK_R:
			{
				keys[0xD] = 1;
			} break;

			case SDLK_F:
			{
				keys[0xE] = 1;
			} break;

			case SDLK_V:
			{
				keys[0xF] = 1;
			} break;
			}
		} break;

		case SDL_EVENT_KEY_UP:
		{
			switch (event.key.key)
			{
			case SDLK_X:
			{
				keys[0] = 0;
			} break;

			case SDLK_1:
			{
				keys[1] = 0;
			} break;

			case SDLK_2:
			{
				keys[2] = 0;
			} break;

			case SDLK_3:
			{
				keys[3] = 0;
			} break;

			case SDLK_Q:
			{
				keys[4] = 0;
			} break;

			case SDLK_W:
			{
				keys[5] = 0;
			} break;

			case SDLK_E:
			{
				keys[6] = 0;
			} break;

			case SDLK_A:
			{
				keys[7] = 0;
			} break;

			case SDLK_S:
			{
				keys[8] = 0;
			} break;

			case SDLK_D:
			{
				keys[9] = 0;
			} break;

			case SDLK_Z:
			{
				keys[0xA] = 0;
			} break;

			case SDLK_C:
			{
				keys[0xB] = 0;
			} break;

			case SDLK_4:
			{
				keys[0xC] = 0;
			} break;

			case SDLK_R:
			{
				keys[0xD] = 0;
			} break;

			case SDLK_F:
			{
				keys[0xE] = 0;
			} break;

			case SDLK_V:
			{
				keys[0xF] = 0;
			} break;
			}
		} break;
		}
	}

	return quit;
}
