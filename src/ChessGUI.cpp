#include "ChessGUI.h"

#include <iostream>

const int SPRITE_SIZE = 80; // pixels

ChessGUI::ChessGUI()
{
	mDarkColour = sf::Color(242, 206, 162);
	mLightColour = sf::Color(245, 245, 245);

	static sf::Image spriteSheetImage;
	if (!spriteSheetImage.loadFromFile("pieceSpriteSheet.png"))
		std::cout << "Sprite Sheet could not load\n";

	spriteSheetImage.createMaskFromColor(sf::Color(127, 127, 127));

	static sf::Texture spriteSheetTexture;
	spriteSheetTexture.loadFromImage(spriteSheetImage);
	spriteSheetTexture.setSmooth(true);

	for (int spriteIndex = 0; spriteIndex < 12; spriteIndex++)
	{
		mPieceSprites[spriteIndex].setTexture(spriteSheetTexture);

		if (spriteIndex < 6)
			mPieceSprites[spriteIndex].setTextureRect(sf::IntRect(spriteIndex * SPRITE_SIZE + spriteIndex, 0, SPRITE_SIZE, SPRITE_SIZE));
		else
			mPieceSprites[spriteIndex].setTextureRect(sf::IntRect(spriteIndex * SPRITE_SIZE + spriteIndex - (SPRITE_SIZE * 6 + 6), 
																  SPRITE_SIZE + 1, SPRITE_SIZE, SPRITE_SIZE));
	}
}

void ChessGUI::init(int wWidth, int wHeight)
{
	mWindowWidth  = wWidth;
	mWindowHeight = wHeight;
	mWindow.create(sf::VideoMode(mWindowWidth, mWindowHeight), "Chess Engine");

	mSquareSize = mWindowHeight / 8;

	// so that the sprites are never larger than the squares (only if special window dimensions are inputted)
	while (SPRITE_SIZE > mSquareSize)
		for (int i = 0; i < 12; i++)
			mPieceSprites[i].setScale(sf::Vector2f(0.9, 0.9));

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

					if (input.squareLoc >= 0 && input.squareLoc < 64)
					{
						//mBoardSquares[64 - input.squareLoc].set
					}

					return input;
				}
			}
		}
	}
}

void ChessGUI::drawPiece(sf::Vector2f pos, SpriteType spriteType)
{
	mPieceSprites[spriteType].setPosition(pos);
	mWindow.draw(mPieceSprites[spriteType]);
}

void ChessGUI::updateBoard(Board* board)
{
	mWindow.clear();
	for (int square = 0; square < 64; square++)
		mWindow.draw(mBoardSquares[square]);

	// draw all of the pieces on the board
	for (int bit = 63; bit >= 0; bit--)
	{
		// currently only works for SPRITE_SIZE = 80
		sf::Vector2f translatedPos(bit % 8 * mSquareSize + (mSquareSize - SPRITE_SIZE) / 2, 
								   (7 - bit / 8) * mSquareSize + (mSquareSize - SPRITE_SIZE) / 2); // they all start top left 
		// try for bit = 0

		if		(board->whitePawnsBB & BB::boardSquares[bit])	drawPiece(translatedPos, WHITE_PAWN);
		else if (board->whiteKingBB & BB::boardSquares[bit])	drawPiece(translatedPos, WHITE_KING);
		else if (board->whiteRooksBB & BB::boardSquares[bit])   drawPiece(translatedPos, WHITE_ROOK);
		else if (board->whiteBishopsBB & BB::boardSquares[bit]) drawPiece(translatedPos, WHITE_BISHOP);
		else if (board->whiteQueensBB & BB::boardSquares[bit])  drawPiece(translatedPos, WHITE_QUEEN);
		else if (board->whiteKnightsBB & BB::boardSquares[bit]) drawPiece(translatedPos, WHITE_KNIGHT);

		if (board->blackPawnsBB & BB::boardSquares[bit])		drawPiece(translatedPos, BLACK_PAWN);
		else if (board->blackKingBB & BB::boardSquares[bit])	drawPiece(translatedPos, BLACK_KING);
		else if (board->blackRooksBB & BB::boardSquares[bit])   drawPiece(translatedPos, BLACK_ROOK);
		else if (board->blackBishopsBB & BB::boardSquares[bit]) drawPiece(translatedPos, BLACK_BISHOP);
		else if (board->blackQueensBB & BB::boardSquares[bit])  drawPiece(translatedPos, BLACK_QUEEN);
		else if (board->blackKnightsBB & BB::boardSquares[bit]) drawPiece(translatedPos, BLACK_KNIGHT);
	}

	mWindow.display();
}