#pragma once
#include <cstdint>
#include <fstream>
#include <chrono>
#include <random>
#include <string>
#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>


const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_START_ADDRESS = 0x50;
const unsigned int FONTSET_SIZE = 80;
const unsigned int VIDEO_WIDTH = 64;
const unsigned int VIDEO_HEIGHT = 32;

class Chip8
{
public:
	uint8_t registers[16]{};
	uint8_t memory[4096]{};
	uint16_t index{};
	uint16_t pc{};
	uint16_t stack[16]{};
	uint8_t sp{};
	uint8_t delay_timer{};
	uint8_t sound_timer{};
	uint8_t keypad[16]{};
	uint32_t video[64 * 32]{};
	uint16_t opcode{};
	std::default_random_engine rand_gen;
	std::uniform_int_distribution<int> rand_byte;

	typedef void (Chip8::* Chip8Func)();
	Chip8Func table[0xF + 1];
	Chip8Func table0[0xE + 1];
	Chip8Func table8[0xE + 1];
	Chip8Func tableE[0xE + 1];
	Chip8Func tableF[0x65 + 1];

	void load_ROM(char const* fname);
	Chip8();

	void op_00E0(); // clear # CLS
	void op_00EE(); // return from subroutine # RET
	void op_1nnn(); // jump to loaction nnn # JP addr
	void op_2nnn(); // call subroutine at nnn # CALL addr
	void op_3xkk(); // skip next if Vx==kk # SE Vx,byte
	void op_4xkk(); // skip next if Vx!=kk # SNE Vx,byte
	void op_5xy0(); // skip next if Vx==Vy # SE Vx,Vy
	void op_6xkk(); // set Vx to kk # LD Vx,byte
	void op_7xkk(); // set Vx to Vx+kk # ADD Vx,byte
	void op_8xy0(); // set Vx to Vy # LD Vx,Vy
	void op_8xy1(); // set Vx to Vx or Vy # OR Vx,Vy
	void op_8xy2(); // set Vx to Vx and Vy # AND Vx,Vy
	void op_8xy3(); // set Vx to Vx xor Vy # XOR Vx,Vy
	void op_8xy4(); // set Vx to Vx+Vy, set VF = carry # ADD Vx,Vy
	void op_8xy5(); // set Vx to Vx-Vy, set VF = NOT borrow # SUB Vx,Vy
	void op_8xy6(); // set Vx to Vx shr 1 # SHR Vx
	void op_8xy7(); // set Vx to Vy-Vx, set VF = NOT borrow # SUBN Vx,Vy
	void op_8xyE(); // set Vx to Vx shl 1 # SHL Vx {, Vy}
	void op_9xy0(); // skip next if Vx!=Vy # SNE Vx,Vy 
	void op_Annn(); // set I = nnn # LD I,addr 
	void op_Bnnn(); // jump to nnn+V0 # JP V0,addr 
	void op_Cxkk(); // set Vx to random byte AND kk # RND Vx,byte
	void op_Dxyn(); // display sprite # DRW Vx, Vy, nibble
	void op_Ex9E(); // skip next if key Vx is pressed # SKP Vx 
	void op_ExA1(); // skip next if key Vx is not pressed # SKNP Vx 
	void op_Fx07(); // set Vx = delay timer value # LD Vx,DT 
	void op_Fx0A(); // wait for key, store value in Vx # LD Vx,k
	void op_Fx15(); // # LD DT, Vx 
	void op_Fx18(); // # LD ST, Vx 
	void op_Fx1E(); // # ADD I, Vx 
	void op_Fx29(); // # LD F, Vx
	void op_Fx33(); // # LD B, Vx
	void op_Fx55(); // # LD [I], Vx
	void op_Fx65(); // # LD Vx, [I]
	void Table0()
	{
		((*this).*(table0[opcode & 0x000Fu]))();
	}
	void Table8()
	{
		((*this).*(table8[opcode & 0x000Fu]))();
	}
	void TableE()
	{
		((*this).*(tableE[opcode & 0x000Fu]))();
	}
	void TableF()
	{
		((*this).*(tableF[opcode & 0x00FFu]))();
	}
	void op_NULL()
	{
	}
	
	void Cycle();
};


class Platform
{
private:
	SDL_Window* window{};
	SDL_Renderer* renderer{};
	SDL_Texture* texture{};
public:
	Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight);
	~Platform();
	void update(void const* buffer, int pitch);
	bool process_input(uint8_t* keys);

};