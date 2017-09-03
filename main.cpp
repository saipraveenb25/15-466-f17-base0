#include "Draw.hpp"
#include "game.h"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <sstream>
#include <chrono>
#include <iostream>
//#include <windows.h>
//#include <GL/GL.h>
//#include <GL/GLU.h>
#include "GL.hpp"

int main(int argc, char **argv) {
	//Configuration:
	struct {
		std::string title = "Game0: Tennis For One";
		glm::uvec2 size = glm::uvec2(640, 640);
	} config;
	
	//------------  initialization ------------

	//Initialize SDL library:
	SDL_Init(SDL_INIT_VIDEO);

	//Ask for an OpenGL context version 3.3, core profile, enable debug:
	SDL_GL_ResetAttributes();
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	//create window:
	SDL_Window *window = SDL_CreateWindow(
		config.title.c_str(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		config.size.x, config.size.y,
		SDL_WINDOW_OPENGL /*| SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI*/
	);

	if (!window) {
		std::cerr << "Error creating SDL window: " << SDL_GetError() << std::endl;
		return 1;
	}

	//Create OpenGL context:
	SDL_GLContext context = SDL_GL_CreateContext(window);

	if (!context) {
		SDL_DestroyWindow(window);
		std::cerr << "Error creating OpenGL context: " << SDL_GetError() << std::endl;
		return 1;
	}

	#ifdef _WIN32
	//On windows, load OpenGL extensions:
	if (!init_gl_shims()) {
		std::cerr << "ERROR: failed to initialize shims." << std::endl;
		return 1;
	}
	#endif

	//Set VSYNC + Late Swap (prevents crazy FPS):
	if (SDL_GL_SetSwapInterval(-1) != 0) {
		std::cerr << "NOTE: couldn't set vsync + late swap tearing (" << SDL_GetError() << ")." << std::endl;
		if (SDL_GL_SetSwapInterval(1) != 0) {
			std::cerr << "NOTE: couldn't set vsync (" << SDL_GetError() << ")." << std::endl;
		}
	}

	//Hide mouse cursor (note: showing can be useful for debugging):
	SDL_ShowCursor(SDL_DISABLE);

	//------------  game state ------------
	int strikes = 0; // Count losses.
	int round = 0; // Round number;
	float curr_target_size = 1.5;
	std::stringstream round_text;
	round_text << "ROUND " << round;
	std::stringstream strike_text;
	round_text << "Strike " << round << "/3";

	
	TennisForOne game(config.size.x, config.size.y, curr_target_size, round_text.str());

	//------------  game loop ------------

	auto previous_time = std::chrono::high_resolution_clock::now();
	bool should_quit = false;
	
	while (true) {
		static SDL_Event evt;
		while (SDL_PollEvent(&evt) == 1) {
			//handle input:
			if (evt.type == SDL_QUIT)
			{
				should_quit = true;
				break;
			}
			else if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE) {
				should_quit = true;
				break;
			}
			game.processInput(evt);
			
		}
		if (should_quit) break;

		auto current_time = std::chrono::high_resolution_clock::now();
		float elapsed = std::chrono::duration< float >(current_time - previous_time).count();
		previous_time = current_time;

		{ //update game state:
			
			if (!game.timeElapsed(elapsed))
			{
				// Game is either Lost or Won.

				// If it's Lost.
				if( game.isLost() )
				{	
					// Reset game to the same round.
					curr_target_size = curr_target_size;
					strikes++;
					
					if (strikes == 3)
					{

						// clear the screen
						//glClearColor(0.0, 0.0, 0.0, 0.0);
						//glClear(GL_COLOR_BUFFER_BIT);

						//glRasterPos2f(-0.5, -0.5f);
						//glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_10, (unsigned char*)"FAILED");

						//SDL_GL_SwapWindow(window);
						// Wait a little while.
						//SDL_Delay(2000);

						// Then exit
						should_quit = true;
						break;
					}

					// clear the screen
					//glClearColor(0.0, 0.0, 0.0, 0.0);
					//glClear(GL_COLOR_BUFFER_BIT);
					
					//glRasterPos2f(-0.5, -0.5f);
					//glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_10, (unsigned char*) strike_text.str().c_str());

					//SDL_GL_SwapWindow(window);
					// Wait a little while.
					SDL_Delay(2000);

					
				}
				else 
				{
					curr_target_size = curr_target_size / 2;
				}


				// Reset the game to the next round.
				game = TennisForOne(config.size.x, config.size.y, curr_target_size, "Round X");
				
				if (curr_target_size < 0.25)
				{
					should_quit = true;
					break;
				}

			}
		}

		//draw output:
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);

		
		{ //draw game state:
			/*Draw draw;
			draw.add_rectangle(mouse + glm::vec2(-0.1f,-0.1f), mouse + glm::vec2(0.1f, 0.1f), glm::u8vec4(0xff, 0xff, 0x00, 0xff));
			draw.add_rectangle(ball + glm::vec2(-0.05f,-0.05f), ball + glm::vec2(0.05f, 0.05f), glm::u8vec4(0xff, 0x00, 0xff, 0xff));
			draw.draw();*/
			game.draw();
		}


		SDL_GL_SwapWindow(window);
	}


	//------------  teardown ------------

	SDL_GL_DeleteContext(context);
	context = 0;

	SDL_DestroyWindow(window);
	window = NULL;

	return 0;
}
