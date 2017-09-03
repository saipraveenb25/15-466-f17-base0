#include <glm/glm.hpp>
#include "Draw.hpp"
#include <SDL2/SDL.h>
#include <iostream>

#ifndef GAME_H
#define GAME_H
class TennisForOne
{
	/*
	* Dynamic state variables here.
	*/
	bool game_pause; // No processing when false.
	glm::fvec2 ball_pos; // Current ball position.
	glm::fvec2 ball_velocity; // Current ball velocity
	glm::fvec2 board_pos; // Current position of the board.

# define GAME_PAUSED 0
# define GAME_RUNNING 1
# define GAME_WON 2
# define GAME_LOST 3

	unsigned int game_state; // Current game state.

	/*
	* Init-time variables.
	*/
	glm::fvec2 target_start; // Start position of the target.
	glm::fvec2 target_end; // End position of the target.
	glm::u8vec4 board_color; // Color of the player board.
	glm::u8vec4 ball_color; // Color of the ball.
	glm::u8vec4 target_color; // Color of the target box.
	glm::vec2 board_size; // Size of the user board.

	glm::uvec2 window_size;

	std::string round_text; // Round text

public:
	TennisForOne(unsigned int x, unsigned int y, float target_size, std::string round_text) {
		window_size.x = x;
		window_size.y = y;

		target_start = glm::fvec2(-1.0, -1.0);
		target_end = glm::fvec2(-1.0, -(1.0 - target_size));

		board_color = glm::u8vec4(0xFF, 0xFF, 0xFF, 0xFF);
		ball_color = glm::u8vec4(0xFF, 0xFF, 0x00, 0xFF);
		target_color = glm::u8vec4(0x99, 0x99, 0xFF, 0xFF);

		ball_pos = glm::fvec2(0.0, 0.0);
		board_size = glm::fvec2(0.05, 0.25);

		game_pause = true;
		ball_velocity = glm::fvec2(0, 0);

		this->round_text = round_text;
		game_state = GAME_PAUSED;
	}

	/*
	* Processing functions
	*/
	// processInput() function.
	void processInput(SDL_Event evt)
	{
		glm::vec2 mouse;
		if (evt.type == SDL_MOUSEMOTION) {
			mouse.x = (evt.motion.x + 0.5f) / float(window_size.x) * 2.0f - 1.0f;
			mouse.y = (evt.motion.y + 0.5f) / float(window_size.y) *-2.0f + 1.0f;

			// Respond to input actions.
			board_pos.y = mouse.y;
			board_pos.x = 1.0f;

		}
		else if (evt.type == SDL_MOUSEBUTTONDOWN) {

			mouse.x = (evt.button.x + 0.5f) / float(window_size.x) * 2.0f - 1.0f;
			mouse.y = (evt.button.y + 0.5f) / float(window_size.y) *-2.0f + 1.0f;

			// If the game is paused, start it
			if (game_pause) { game_pause = false; game_state = GAME_RUNNING; }
			else return;

			// The following statements are only executed if the game is resumed.

			// If the game was paused, start the ball in the right direction (randomly)
			

			float y_vel = ((float) rand()) / RAND_MAX;
			ball_velocity = glm::vec2(0.5f, y_vel * 2 - 1);

			ball_velocity = glm::normalize(ball_velocity);
			ball_velocity = ball_velocity * 1.6f;
		}


	}

	void intersect(glm::vec2 line1_start, glm::vec2 line1_end,
		glm::vec2 line2_start, glm::vec2 line2_end,
		bool * intersected,
		glm::vec2* intersection,
		float* o_alpha,
		float* o_beta
	)
	{

		// Build matrix 
		float m00 = line1_start.x - line1_end.x;
		float m10 = line1_start.y - line1_end.y;
		float m01 = line2_end.x - line2_start.x;
		float m11 = line2_end.y - line2_start.y;

		// Invert matrix.
		float det = m11 * m00 - m10 * m01;
		if (det == 0)
		{
			// Parallel lines.
			*intersected = false;
			return;
		}

		float mi00 = m11 / det;
		float mi01 = -m01 / det;
		float mi11 = m00 / det;
		float mi10 = -m10 / det;

		// Build C vector.
		float c0 = line2_end.x - line1_end.x;
		float c1 = line2_end.y - line1_end.y;

		// Compute intersection coefficients.
		float alpha = c0 * mi00 + c1 * mi01;
		float beta = c0 * mi10 + c1 * mi11;

		if (alpha >= 0 && alpha <= 1 && beta >= 0 && beta <= 1)
		{
			// Found an intersection.
			*intersected = true;
			*intersection = glm::vec2(alpha * line1_start + (1 - alpha) * line1_end);
			*o_alpha = alpha;
			*o_beta = beta;
			return;
		}

		*intersected = false;
		*intersection = glm::vec2(0, 0);
		return;
	}
	// timeElapsed() function.
	bool timeElapsed(float elapsed)
	{
		elapsed = (elapsed < 0.08) ? elapsed : 0.08;
		glm::vec2 new_ball_pos = ball_pos + elapsed * ball_velocity;

		bool intersected = false;
		glm::vec2 intersection(0.0, 0.0);
		float alpha = 0;
		float beta = 0;

		// intersect with the target.
		glm::vec2 line_start = target_start + glm::vec2(0.03, 0);
		glm::vec2 line_end = target_end + glm::vec2(0.03, 0);
		intersect(line_start, line_end, ball_pos, new_ball_pos,
			&intersected, &intersection, &alpha, &beta);
		// If intersected.
		// game over. Signal back to start the next one.
		if (intersected) {
			game_state = GAME_WON;
			return false;
		}


		// intersect with the board.
		line_start = (board_pos + board_size * 0.7f) - glm::vec2(0.03, 0);
		line_end = (board_pos - board_size * 0.7f) - glm::vec2(0.03, 0);
		intersect(line_start, line_end, ball_pos, new_ball_pos,
			&intersected, &intersection, &alpha, &beta);
		// If intersected with the board.
		// Figure out the new velocity for the ball.
		if (intersected) {
			float d_alpha = 2 * alpha - 1;
			glm::vec2 new_dir = glm::vec2(-1, 0) * (1 - abs(d_alpha)) + glm::vec2(0, 1) * d_alpha;
			new_dir = glm::normalize(new_dir);
			ball_velocity = new_dir * (float)ball_velocity.length();
			ball_pos = intersection + beta * elapsed * ball_velocity;
			std::cout << "INTERSECTION BOARD " << alpha << std::endl;

			return true;
		}

		// If intersected with any of the sides
		line_start = glm::vec2(1.0, 1.0);
		line_end = glm::vec2(-1.0, 1.0);
		intersect(line_start, line_end, ball_pos, new_ball_pos,
			&intersected, &intersection, &alpha, &beta);
		//If intersected, mirror.
		if (intersected)
		{
			ball_velocity.y = -ball_velocity.y;
			ball_pos = intersection + beta * elapsed * ball_velocity;
			std::cout << "INTERSECTION TOP WALL" << std::endl;
			return true;
		}

		// If intersected with any of the sides
		line_start = glm::vec2(1.0, -1.0);
		line_end = glm::vec2(-1.0, -1.0);
		intersect(line_start, line_end, ball_pos, new_ball_pos,
			&intersected, &intersection, &alpha, &beta);
		//If intersected, mirror.
		if (intersected)
		{
			ball_velocity.y = -ball_velocity.y;
			ball_pos = intersection + beta * elapsed * ball_velocity;
			std::cout << "INTERSECTION BOTTOM WALL" << std::endl;
			return true;
		}

		// If intersected with any of the sides
		line_start = glm::vec2(-1.0, 1.0);
		line_end = glm::vec2(-1.0, -1.0);
		intersect(line_start, line_end, ball_pos, new_ball_pos,
			&intersected, &intersection, &alpha, &beta);
		//If intersected, mirror.
		if (intersected)
		{
			ball_velocity.x = -ball_velocity.x;
			ball_pos = intersection + beta * elapsed * ball_velocity;
			std::cout << "INTERSECTION LEFT WALL" << std::endl;
			return true;
		}

		// If intersected with any of the sides
		line_start = glm::vec2(1.0, 1.0);
		line_end = glm::vec2(1.0, -1.0);
		intersect(line_start, line_end, ball_pos, new_ball_pos,
			&intersected, &intersection, &alpha, &beta);
		//If intersected, mirror.
		if (intersected)
		{
			//ball_velocity.x = -ball_velocity.x;
			//ball_pos = intersection + beta * elapsed * ball_velocity;
			std::cout << "INTERSECTION RIGHT WALL" << std::endl;
			game_state = GAME_LOST;
			return false;
		}

		ball_pos = new_ball_pos;

		return true;
	}

	/*
	* Draw functions.
	*/

	// draw() function.
	void draw()
	{



		{ //draw game state:
			Draw draw;
			// board
			draw.add_rectangle(board_pos - board_size * 0.5f,
				board_pos + board_size * 0.5f, board_color);
			// target
			draw.add_rectangle(target_start, target_end + glm::vec2(0.03, 0), target_color);

			// ball
			draw.add_rectangle(ball_pos - glm::vec2(0.02, 0.02), ball_pos + glm::vec2(0.02, 0.02), ball_color);

			if (game_pause) {
				//glRasterPos2f(-0.5, -0.5f);
				//glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_10, (unsigned char*)round_text.c_str());
			}

			draw.draw();
		}
	}

	/*
	* Accessor functions
	*/

	bool isRunning() { return game_state == GAME_RUNNING; }
	bool isPaused() { return game_state == GAME_PAUSED; }
	bool isWon() { return game_state == GAME_WON; }
	bool isLost() { return game_state == GAME_LOST; }
};

#endif