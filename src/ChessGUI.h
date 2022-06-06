#pragma once

#include <SFML/Graphics.hpp>

#include "Board.h"

struct UserInput
{
	enum class InputType
	{
		SquareSelect,
		GameClose,
	};

	InputType inputType;
	int squareLoc;
};

class ChessGUI
{
private:
	sf::RenderWindow mWindow;
	sf::RectangleShape mBoardSquares[64];

	sf::Sprite mPieceSprites[12];

	int mWindowWidth, mWindowHeight, mSquareSize;
	sf::Color mDarkColour, mLightColour;

public:
	ChessGUI();

	void init(int wWidth, int wHeight);

	UserInput getUserInput(); // have this return a special struct or an enum
	void updateBoard(Board* board);
};

