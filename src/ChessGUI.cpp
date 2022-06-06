#include "ChessGUI.h"

#include <iostream>

enum SpriteType
{
	WHITE_KING,
	WHITE_QUEEN,
	WHITE_BISHOP,
	WHITE_KNIGHT,
	WHITE_ROOK,
	WHITE_PAWN,

	BLACK_KING,
	BLACK_QUEEN,
	BLACK_BISHOP,
	BLACK_KNIGHT,
	BLACK_ROOK,
	BLACK_PAWN,
};

ChessGUI::ChessGUI()
{
	mDarkColour = sf::Color(242, 206, 162);
	mLightColour = sf::Color(245, 245, 245);

	static sf::Texture spriteSheet;
	if (!spriteSheet.loadFromFile("pieceSpriteSheet.png"))
		std::cout << "Sprite Sheet could not load\n";

	spriteSheet.setSmooth(true);
	for (int spriteType = 0; spriteType < 12; spriteType++)
	{
		mPieceSprites[spriteType].setTexture(spriteSheet);
		//mPieceSprites[spriteType].setTextureRect(sf::IntRect(spriteType * (800 / 6 - 5), , 800 / 6 - 5, 267 / 2 - 5))
	}
}

// 138

void ChessGUI::init(int wWidth, int wHeight)
{
	mWindowWidth  = wWidth;
	mWindowHeight = wHeight;
	mWindow.create(sf::VideoMode(mWindowWidth, mWindowHeight), "Chess Engine");

	mSquareSize = mWindowHeight / 8;
	bool darkColour = true;
	for (int square = 63; square >= 0; square--)
	{
		mBoardSquares[square].setSize(sf::Vector2f(mSquareSize, mSquareSize)); 

		mBoardSquares[square].setPosition(sf::Vector2f((7 - (square % 8)) * wHeight / 8, square / 8 * wHeight / 8));
		
		if (darkColour)
			mBoardSquares[square].setFillColor(mDarkColour);
		else
			mBoardSquares[square].setFillColor(mLightColour);

		darkColour = !darkColour;

		if (square % 8 == 0)
			darkColour = !darkColour;
	}
}

UserInput ChessGUI::getUserInput()
{
	UserInput input;

	while (mWindow.isOpen())
	{
		sf::Event event;
		while (mWindow.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				input.inputType = UserInput::InputType::GameClose;
				mWindow.close();
				return input;
			}
			else if (event.type == sf::Event::MouseButtonPressed)
			{
				int mx = sf::Mouse::getPosition(mWindow).x;
				int my = mWindowHeight - sf::Mouse::getPosition(mWindow).y;

				if (event.mouseButton.button == sf::Mouse::Left)
				{
					input.inputType = UserInput::InputType::SquareSelect;
					input.squareLoc = my / mSquareSize * 8 + mx / mSquareSize;
					return input;
				}
			}
		}

		// negate y when finding what tile the user is selected
	}
}

void ChessGUI::updateBoard(Board* board)
{
	mWindow.clear();
	for (int square = 0; square < 64; square++)
		mWindow.draw(mBoardSquares[square]);

	// get a picture of all the pieces and assign them textures
	// fuckkkkk

	mWindow.display();
}