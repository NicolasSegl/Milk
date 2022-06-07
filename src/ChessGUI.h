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
	enum SpriteType
	{
		WHITE_PAWN,
		WHITE_ROOK,
		WHITE_BISHOP,
		WHITE_QUEEN,
		WHITE_KNIGHT,
		WHITE_KING,

		BLACK_PAWN,
		BLACK_ROOK,
		BLACK_BISHOP,
		BLACK_QUEEN,
		BLACK_KNIGHT,
		BLACK_KING,
	};


	sf::RenderWindow mWindow;
	sf::RectangleShape mBoardSquares[64];

	sf::Sprite mPieceSprites[12];

	int mWindowWidth, mWindowHeight, mSquareSize;
	sf::Color mDarkColour, mLightColour;

	void drawPiece(sf::Vector2f pos, SpriteType spriteType);

public:
	ChessGUI();

	void init(int wWidth, int wHeight);

	UserInput getUserInput(); // have this return a special struct or an enum
	void updateBoard(Board* board);
};

