// Rule tests for the chess engine in Chess/. No OpenGL needed.
// Build and run with: make test
//
// Coordinates: row 1 is White's back rank, row 8 is Black's.
// Columns 1..8 are files a..h, so the kings start on column 5.
#include <cstdio>
#include <vector>

#include "../Chess/Game.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond) \
    do { \
        checks++; \
        if(!(cond)) { \
            failures++; \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        } \
    } while(0)

// Remove every piece so a test can set up its own position.
static void clearBoard(Game& game)
{
    for(int row = 1; row <= 8; row++)
        for(int col = 1; col <= 8; col++)
            delete game.getBoard()->getSquare(row, col)->removeOccupyingPiece();
}

// Constructed positions that are not about castling call this, so that the
// engine does not offer castling to a king that was placed by hand.
static void noCastling(Game& game)
{
    game.getGameStatus()->setKingMove(PieceColor::WHITE);
    game.getGameStatus()->setKingMove(PieceColor::BLACK);
}

static void put(Game& game, int row, int col, PieceType type, PieceColor color)
{
    game.getBoard()->getSquare(row, col)->occupySquare(new Piece(type, color));
}

// Total number of legal moves for the side to move.
static int legalMoveCount(Game& game)
{
    int count = 0;
    for(int row = 1; row <= 8; row++)
        for(int col = 1; col <= 8; col++)
            if(game.isSquareOccupied(row, col) && game.getPieceColor(row, col) == game.getTurnColor())
                count += game.getValidMoves(row, col).size();
    return count;
}

static bool hasMove(Game& game, int fromRow, int fromCol, int toRow, int toCol, MoveType type)
{
    std::vector<Move> moves = game.getValidMoves(fromRow, fromCol);
    for(size_t i = 0; i < moves.size(); i++)
        if(moves[i].getDestinationPosition().first == toRow
            && moves[i].getDestinationPosition().second == toCol
            && moves[i].getType() == type)
            return true;
    return false;
}

static int countMovesOfType(Game& game, int fromRow, int fromCol, MoveType type)
{
    int count = 0;
    std::vector<Move> moves = game.getValidMoves(fromRow, fromCol);
    for(size_t i = 0; i < moves.size(); i++)
        if(moves[i].getType() == type) count++;
    return count;
}

// Make a move for the side to move and pass the turn, like main.cpp does.
static bool play(Game& game, int fromRow, int fromCol, int toRow, int toCol)
{
    if(!game.move(fromRow, fromCol, toRow, toCol)) return false;
    game.nextTurn();
    return true;
}

static void testStartPosition()
{
    Game game;
    CHECK(game.getTurnColor() == PieceColor::WHITE);
    CHECK(legalMoveCount(game) == 20);
    CHECK(!game.inCheckState());
    CHECK(!game.inCheckMateState());
    CHECK(game.getPiece(1, 5)->getType() == PieceType::KING);
    CHECK(game.getPiece(1, 4)->getType() == PieceType::QUEEN);
    CHECK(game.getPiece(8, 5)->getColor() == PieceColor::BLACK);
}

static void testSimpleMovesAndTurns()
{
    Game game;
    CHECK(!game.move(1, 1, 3, 1));          // rook cannot jump over its pawn
    CHECK(!game.move(2, 5, 5, 5));          // pawn cannot move three squares
    CHECK(play(game, 2, 5, 4, 5));          // e2-e4
    CHECK(game.isSquareOccupied(4, 5));
    CHECK(!game.isSquareOccupied(2, 5));
    CHECK(game.getTurnColor() == PieceColor::BLACK);
    CHECK(legalMoveCount(game) == 20);
}

static void testFoolsMate()
{
    Game game;
    CHECK(play(game, 2, 6, 3, 6));          // f2-f3
    CHECK(play(game, 7, 5, 5, 5));          // e7-e5
    CHECK(play(game, 2, 7, 4, 7));          // g2-g4
    CHECK(play(game, 8, 4, 4, 8));          // Qd8-h4 mate
    CHECK(game.getTurnColor() == PieceColor::WHITE);
    CHECK(game.inCheckState());
    CHECK(game.inCheckMateState());
}

static void testCheckIsNotMateWhenKingCanEscape()
{
    Game game;
    clearBoard(game);
    noCastling(game);
    put(game, 1, 5, PieceType::KING, PieceColor::WHITE);
    put(game, 8, 5, PieceType::ROOK, PieceColor::BLACK);
    put(game, 8, 1, PieceType::KING, PieceColor::BLACK);
    CHECK(game.inCheckState());
    CHECK(!game.inCheckMateState());
    CHECK(!hasMove(game, 1, 5, 2, 5, MoveType::NORMAL));   // still on the attacked file
    CHECK(hasMove(game, 1, 5, 1, 4, MoveType::NORMAL));
}

static void testBackRankMate()
{
    Game game;
    clearBoard(game);
    noCastling(game);
    put(game, 1, 7, PieceType::KING, PieceColor::WHITE);
    put(game, 2, 6, PieceType::PAWN, PieceColor::WHITE);
    put(game, 2, 7, PieceType::PAWN, PieceColor::WHITE);
    put(game, 2, 8, PieceType::PAWN, PieceColor::WHITE);
    put(game, 1, 1, PieceType::ROOK, PieceColor::BLACK);
    put(game, 8, 5, PieceType::KING, PieceColor::BLACK);
    CHECK(game.inCheckState());
    CHECK(game.inCheckMateState());
}

static void testPinnedPieceStaysOnTheLine()
{
    Game game;
    clearBoard(game);
    noCastling(game);
    put(game, 1, 5, PieceType::KING, PieceColor::WHITE);
    put(game, 2, 5, PieceType::ROOK, PieceColor::WHITE);
    put(game, 8, 5, PieceType::ROOK, PieceColor::BLACK);
    put(game, 8, 1, PieceType::KING, PieceColor::BLACK);
    std::vector<Move> moves = game.getValidMoves(2, 5);
    CHECK(moves.size() == 6);               // e3..e7 and the capture on e8
    for(size_t i = 0; i < moves.size(); i++)
        CHECK(moves[i].getDestinationPosition().second == 5);
    CHECK(hasMove(game, 2, 5, 8, 5, MoveType::CAPTURE));
}

static void testEnPassant()
{
    Game game;
    CHECK(play(game, 2, 5, 4, 5));          // e2-e4
    CHECK(play(game, 7, 1, 6, 1));          // a7-a6
    CHECK(play(game, 4, 5, 5, 5));          // e4-e5
    CHECK(play(game, 7, 4, 5, 4));          // d7-d5, passing the e5 pawn
    CHECK(hasMove(game, 5, 5, 6, 4, MoveType::EN_PASSANT));
    CHECK(play(game, 5, 5, 6, 4));
    CHECK(!game.isSquareOccupied(5, 4));    // the passed pawn is gone
    CHECK(game.getPiece(6, 4)->getColor() == PieceColor::WHITE);
}

static void testEnPassantExpiresAfterOneTurn()
{
    Game game;
    CHECK(play(game, 2, 5, 4, 5));          // e2-e4
    CHECK(play(game, 7, 1, 6, 1));          // a7-a6
    CHECK(play(game, 4, 5, 5, 5));          // e4-e5
    CHECK(play(game, 7, 4, 5, 4));          // d7-d5
    CHECK(play(game, 2, 1, 3, 1));          // a2-a3, declining the capture
    CHECK(play(game, 6, 1, 5, 1));          // a6-a5
    CHECK(!hasMove(game, 5, 5, 6, 4, MoveType::EN_PASSANT));
}

static void testKingsideCastling()
{
    Game game;
    clearBoard(game);
    put(game, 1, 5, PieceType::KING, PieceColor::WHITE);
    put(game, 1, 8, PieceType::ROOK, PieceColor::WHITE);
    put(game, 8, 5, PieceType::KING, PieceColor::BLACK);
    CHECK(hasMove(game, 1, 5, 1, 7, MoveType::CASTLING));
    CHECK(play(game, 1, 5, 1, 7));
    CHECK(game.getPiece(1, 7)->getType() == PieceType::KING);
    CHECK(game.getPiece(1, 6)->getType() == PieceType::ROOK);
    CHECK(!game.isSquareOccupied(1, 8));
    CHECK(!game.isSquareOccupied(1, 5));
}

static void testNoCastlingAfterTheKingHasMoved()
{
    Game game;
    clearBoard(game);
    put(game, 1, 5, PieceType::KING, PieceColor::WHITE);
    put(game, 1, 8, PieceType::ROOK, PieceColor::WHITE);
    put(game, 8, 5, PieceType::KING, PieceColor::BLACK);
    CHECK(play(game, 1, 5, 2, 5));          // Ke1-e2
    CHECK(play(game, 8, 5, 8, 4));          // Ke8-d8
    CHECK(play(game, 2, 5, 1, 5));          // Ke2-e1
    CHECK(play(game, 8, 4, 8, 5));          // Kd8-e8
    CHECK(countMovesOfType(game, 1, 5, MoveType::CASTLING) == 0);
}

static void testPromotion()
{
    Game game;
    clearBoard(game);
    noCastling(game);
    put(game, 7, 1, PieceType::PAWN, PieceColor::WHITE);
    put(game, 1, 5, PieceType::KING, PieceColor::WHITE);
    put(game, 8, 8, PieceType::KING, PieceColor::BLACK);
    CHECK(game.move(7, 1, 8, 1));
    CHECK(game.promote(8, 1, PieceType::QUEEN));
    CHECK(game.getPiece(8, 1)->getType() == PieceType::QUEEN);
    CHECK(game.getPiece(8, 1)->getColor() == PieceColor::WHITE);
    CHECK(!game.promote(1, 5, PieceType::QUEEN));   // only pawns promote
}

static void testQueensideCastling()
{
    Game game;
    clearBoard(game);
    put(game, 1, 5, PieceType::KING, PieceColor::WHITE);
    put(game, 1, 1, PieceType::ROOK, PieceColor::WHITE);
    put(game, 8, 5, PieceType::KING, PieceColor::BLACK);
    CHECK(hasMove(game, 1, 5, 1, 3, MoveType::CASTLING));
    CHECK(play(game, 1, 5, 1, 3));
    CHECK(game.getPiece(1, 3)->getType() == PieceType::KING);
    CHECK(game.getPiece(1, 4)->getType() == PieceType::ROOK);
    CHECK(!game.isSquareOccupied(1, 1));
}

static void testQueensideCastlingNeedsAnEmptyKnightSquare()
{
    Game game;
    clearBoard(game);
    put(game, 1, 5, PieceType::KING, PieceColor::WHITE);
    put(game, 1, 1, PieceType::ROOK, PieceColor::WHITE);
    put(game, 1, 2, PieceType::KNIGHT, PieceColor::WHITE);
    put(game, 8, 5, PieceType::KING, PieceColor::BLACK);
    CHECK(countMovesOfType(game, 1, 5, MoveType::CASTLING) == 0);
}

static void testNoCastlingWithoutARook()
{
    Game game;
    clearBoard(game);
    put(game, 1, 5, PieceType::KING, PieceColor::WHITE);
    put(game, 8, 5, PieceType::KING, PieceColor::BLACK);
    CHECK(countMovesOfType(game, 1, 5, MoveType::CASTLING) == 0);

    put(game, 1, 1, PieceType::BISHOP, PieceColor::BLACK);   // enemy piece on the corner
    CHECK(countMovesOfType(game, 1, 5, MoveType::CASTLING) == 0);
}

static void testNoCastlingOutOfCheck()
{
    Game game;
    clearBoard(game);
    put(game, 1, 5, PieceType::KING, PieceColor::WHITE);
    put(game, 1, 8, PieceType::ROOK, PieceColor::WHITE);
    put(game, 8, 5, PieceType::ROOK, PieceColor::BLACK);     // checks the king on the e-file
    put(game, 8, 1, PieceType::KING, PieceColor::BLACK);
    CHECK(game.inCheckState());
    CHECK(countMovesOfType(game, 1, 5, MoveType::CASTLING) == 0);
}

static void testNoCastlingThroughAnAttackedSquare()
{
    Game game;
    clearBoard(game);
    put(game, 1, 5, PieceType::KING, PieceColor::WHITE);
    put(game, 1, 8, PieceType::ROOK, PieceColor::WHITE);
    put(game, 8, 6, PieceType::ROOK, PieceColor::BLACK);     // attacks f1, the square the king crosses
    put(game, 8, 1, PieceType::KING, PieceColor::BLACK);
    CHECK(!game.inCheckState());
    CHECK(countMovesOfType(game, 1, 5, MoveType::CASTLING) == 0);
}

static void testCastlingIsAllowedWhenOnlyTheRookPathIsAttacked()
{
    Game game;
    clearBoard(game);
    put(game, 1, 5, PieceType::KING, PieceColor::WHITE);
    put(game, 1, 1, PieceType::ROOK, PieceColor::WHITE);
    put(game, 8, 2, PieceType::ROOK, PieceColor::BLACK);     // attacks b1, which only the rook crosses
    put(game, 8, 8, PieceType::KING, PieceColor::BLACK);
    CHECK(hasMove(game, 1, 5, 1, 3, MoveType::CASTLING));
}

static void testCapturedRookLosesItsCastlingRight()
{
    Game game;
    clearBoard(game);
    put(game, 1, 5, PieceType::KING, PieceColor::WHITE);
    put(game, 1, 8, PieceType::ROOK, PieceColor::WHITE);
    put(game, 2, 1, PieceType::PAWN, PieceColor::WHITE);
    put(game, 2, 7, PieceType::BISHOP, PieceColor::BLACK);
    put(game, 8, 5, PieceType::KING, PieceColor::BLACK);
    CHECK(play(game, 2, 1, 3, 1));          // a2-a3
    CHECK(play(game, 2, 7, 1, 8));          // bishop takes the rook on h1
    CHECK(game.getGameStatus()->isLastColRookMove(PieceColor::WHITE));
    CHECK(countMovesOfType(game, 1, 5, MoveType::CASTLING) == 0);
}

static void testRookOnAnotherRankDoesNotSpendTheOtherCastlingRight()
{
    Game game;
    clearBoard(game);
    put(game, 1, 5, PieceType::KING, PieceColor::WHITE);
    put(game, 1, 1, PieceType::ROOK, PieceColor::WHITE);
    put(game, 1, 8, PieceType::ROOK, PieceColor::WHITE);
    put(game, 8, 5, PieceType::KING, PieceColor::BLACK);
    CHECK(play(game, 1, 8, 5, 8));          // Rh1-h5
    CHECK(play(game, 8, 5, 8, 4));          // Ke8-d8
    CHECK(play(game, 5, 8, 5, 1));          // Rh5-a5, now on the a-file but not on a1
    CHECK(play(game, 8, 4, 8, 5));          // Kd8-e8
    CHECK(play(game, 5, 1, 5, 2));          // Ra5-b5
    CHECK(play(game, 8, 5, 8, 4));          // Ke8-d8
    CHECK(hasMove(game, 1, 5, 1, 3, MoveType::CASTLING));    // the a1 rook never moved
    CHECK(!hasMove(game, 1, 5, 1, 7, MoveType::CASTLING));   // the h1 rook did
}

static void testStalemateIsNotCheckmate()
{
    Game game;
    clearBoard(game);
    noCastling(game);
    put(game, 8, 1, PieceType::KING, PieceColor::BLACK);     // a8
    put(game, 6, 2, PieceType::QUEEN, PieceColor::WHITE);    // b6
    put(game, 7, 3, PieceType::KING, PieceColor::WHITE);     // c7
    game.nextTurn();                                         // Black to move
    CHECK(game.getTurnColor() == PieceColor::BLACK);
    CHECK(legalMoveCount(game) == 0);
    CHECK(!game.inCheckState());
    CHECK(game.inStaleMateState());
    CHECK(!game.inCheckMateState());
}

static void testCheckmateIsNotStalemate()
{
    Game game;
    CHECK(!game.inStaleMateState());
    CHECK(play(game, 2, 6, 3, 6));          // f2-f3
    CHECK(play(game, 7, 5, 5, 5));          // e7-e5
    CHECK(play(game, 2, 7, 4, 7));          // g2-g4
    CHECK(play(game, 8, 4, 4, 8));          // Qd8-h4 mate
    CHECK(game.inCheckMateState());
    CHECK(!game.inStaleMateState());
}

int main()
{
    testStartPosition();
    testSimpleMovesAndTurns();
    testFoolsMate();
    testCheckIsNotMateWhenKingCanEscape();
    testBackRankMate();
    testPinnedPieceStaysOnTheLine();
    testEnPassant();
    testEnPassantExpiresAfterOneTurn();
    testKingsideCastling();
    testNoCastlingAfterTheKingHasMoved();
    testPromotion();
    testQueensideCastling();
    testQueensideCastlingNeedsAnEmptyKnightSquare();
    testNoCastlingWithoutARook();
    testNoCastlingOutOfCheck();
    testNoCastlingThroughAnAttackedSquare();
    testCastlingIsAllowedWhenOnlyTheRookPathIsAttacked();
    testCapturedRookLosesItsCastlingRight();
    testRookOnAnotherRankDoesNotSpendTheOtherCastlingRight();
    testStalemateIsNotCheckmate();
    testCheckmateIsNotStalemate();

    std::printf("%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
