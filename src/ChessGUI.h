#pragma once

#include <SFML/Graphics.hpp>

#include "Board.h"
#include "MoveData.h"

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
    struct guiSquareData
    {
        sf::RectangleShape rect;
        sf::Color ogColour;

        void resetColour() { rect.setFillColor(ogColour); }
    };

	sf::RenderWindow mWindow;
	//sf::RectangleShape mBoardSquares[64];
    guiSquareData mBoardSquares[64];

	sf::Sprite mPieceSprites[12];

	int mWindowWidth, mWindowHeight, mSquareSize;;
	sf::Color mDarkColour, mLightColour, mSelectColour;

	int mLastSelectedSquare;

	void drawPiece(sf::Vector2f pos, Board::PieceType spriteType);

public:
	ChessGUI();

	void init(int wWidth, int wHeight);

	UserInput getUserInput(); // have this return a special struct or an enum
	void updateBoard(Board* board);
	void setMoveColours(MoveData* md);
	void setSelectedSquare(Byte square);
	void unselectSelectedSquare();
};

