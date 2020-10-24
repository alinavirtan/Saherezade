#include <bits/stdc++.h>
#include "chess.h"

using namespace std;

/* Available colors for the pieces on the chessboard */
enum Color {BLACK, WHITE, NONE};

/* Two players in the game: SAHEREZADE vs OPPONENT 
	Note: Saherezade = my super-bot */
enum Player {SAHEREZADE, OPPONENT};

/* Force mode active or not*/

enum Force {ACTIVE, INACTIVE};


class Piece {
 public:
	int type; // the type of the piece: KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN
    Color color; // the color of the piece: BLACK or WHITE
	
	Piece(int type, Color color) {
		this->type = type;
		this->color = color;
	}

	Piece() {
		this->type = EMPTY;
		this->color = NONE;
	}
};

/* Define the characteristics of a move:
	- line = the line where the piece comes from
	- column = the column where the piece comes from
	- next_line = the line where the piece goes to
	- next_column = the column where the piece goes to */
class Move {
 public:
 	int line, column;
 	int next_line, next_column;
 	int score;
 	int pawn_steps;

 	Move(int line, int column, int next_line, int next_column) {
 		this->line = line;
 		this->column = column;
 		this->next_line = next_line;
 		this->next_column = next_column;
 		this->score = 0;
 	}

 	Move() {
 		this->line = 0;
 		this->column = 0;
 		this->next_line = 0;
 		this->next_column = 0;
 		this->score = 0;
 	}
};

class Position {
 public:
 	int line, col;

 	Position(int line, int column) {
 		this->line = line;
 		this->col = column;
 	}

 	Position() {
 		this->line = 0;
 		this->col = 0;
 	}
};

/* Game characteristics: 
 *	- engine's color: black 
 *  - the player whose player is  
 *  - cheesboard = distribution of the pieces on the cheesboard
 *
 */
class ChessGame {
 public:
 	bool ColorSwitch;
 	Force force;
	Color color;
	Player player;
	Piece chessboard[9][9];
	Position pos_black[17];
	Position pos_white[17];
	bool white_rook1_moved, white_rook2_moved, white_king_moved;
	bool black_rook1_moved, black_rook2_moved, black_king_moved;
	Position queens_white[9];
 	Position queens_black[9];

	ChessGame() {
		this->color = BLACK;
		this->player = OPPONENT;
		this->force = INACTIVE;
		this->ColorSwitch = false;
		this->white_rook1_moved = false;
		this->white_rook2_moved = false;
		this->white_king_moved = false;
		this->black_rook1_moved = false;
		this->black_rook2_moved = false;
		this->black_king_moved = false;
	}

	int search_queen_index(int line, int column, Position queens[9]) {
		for (int i = 1; i <= 8; i++) {
			if (queens[i].line == line && queens[i].col == column) {
				return i;
			}
		}

		return 0;
	}

	void initChessBoard() {
		/*** initialise the initial state of the board ***/

		/* initialise the white pieces */
		chessboard[1][1] = Piece(R1, WHITE);
		chessboard[1][8] = Piece(R2, WHITE);
		chessboard[1][2] = Piece(K1, WHITE);
		chessboard[1][7] = Piece(K2, WHITE);
		chessboard[1][3] = Piece(B1, WHITE);
		chessboard[1][6] = Piece(B2, WHITE);
		chessboard[1][4] = Piece(Q, WHITE);
		chessboard[1][5] = Piece(KING, WHITE);

		for (int j = 1; j <= 8; j++) {
			chessboard[2][j] = Piece(j, WHITE);
		}

		/* initialise the empty spaces on the board */
		for (int i = 3; i <= 6; i++) {
			for (int j = 1; j <= 8; j++) {
				chessboard[i][j] = Piece(EMPTY, NONE);
			}
		}

		/* initialise the black pieces */ 
		chessboard[8][1] = Piece(R1, BLACK); 
		chessboard[8][8] = Piece(R2, BLACK);
		chessboard[8][2] = Piece(K1, BLACK);
		chessboard[8][7] = Piece(K2, BLACK);
		chessboard[8][3] = Piece(B1, BLACK);
		chessboard[8][6] = Piece(B2, BLACK);
		chessboard[8][4] = Piece(Q, BLACK);
		chessboard[8][5] = Piece(KING, BLACK);

		for (int j = 1; j <= 8; j++) {
			chessboard[7][j] = Piece(j, BLACK);
		}

		/*** initial positions for every piece on the board ***/

		for (int i = 1; i <= 16; i++) {
			/* Pawns */
			for(int j = 1; j <= 8; j++){	
				pos_black[j] = Position(7, j);
				pos_white[j] = Position(2, j);
			}

			/* Rooks */
			pos_black[R1] = Position(8, 1);
			pos_black[R2] = Position(8, 8);

			pos_white[R1] = Position(1, 1);
			pos_white[R2] = Position(1, 8);

			/* Knights */
			pos_black[K1] = Position(8, 2);
			pos_black[K2] = Position(8, 7);

			pos_white[K1] = Position(1, 2);
			pos_white[K2] = Position(1, 7);

			/* Bishops */
			pos_black[B1] = Position(8, 3);
			pos_black[B2] = Position(8, 6);

			pos_white[B1] = Position(1, 3);
			pos_white[B2] = Position(1, 6);

			/* The Queen and The King */
			pos_black[Q] = Position(8, 4);
			pos_black[KING] = Position(8, 5);

			pos_white[Q] = Position(1, 4);
			pos_white[KING] = Position(1, 5);
		}

		for (int i = 1; i <= 8; i++) {
			queens_black[i] = Position(0, 0);
			queens_white[i] = Position(0, 0);
		}
	}

/* opponent's move received from the xboard as a string (eg. a2b3) */
	Move opponent_move_from_xboard(string received) {
		Color opponentColor = (color == WHITE) ? BLACK : WHITE;
		Move m;

		m.column = received[0] - 'a' + 1;
		m.line = received[1] - '0';
		m.next_column = received[2] - 'a' + 1;
		m.next_line = received[3] - '0';

		/* helpful for castling */
		if (makeCastling(m, opponentColor) == true) {
			return m;
		}

		if (chessboard[m.line][m.column].type == KING) {
			if (opponentColor == WHITE) {
				white_king_moved = true;
			} else {
				black_king_moved = true;
			}
		}

		if (chessboard[m.line][m.column].type == R1) {
			if (opponentColor == WHITE) {
				white_rook1_moved = true;
			} else {
				black_rook1_moved = true;
			}
		}

		if (chessboard[m.line][m.column].type == R2) {
			if (opponentColor == WHITE) {
				white_rook2_moved = true;
			} else {
				black_rook2_moved = true;
			}
		}

		int piece_id;
		/* en passant received from xboard */
		piece_id = chessboard[m.line][m.column].type;
		if (piece_id >= 1 && piece_id <= 8) {
			if (opponentColor == WHITE) {
				if (m.next_line == m.line + 1 && m.next_column == m.column - 1 && chessboard[m.next_line][m.next_column].type == EMPTY) {
					/* update the position of the to-be-moved piece */
					chessboard[m.next_line][m.next_column] = chessboard[m.line][m.column];
					chessboard[m.line][m.column] = Piece();
					pos_white[piece_id].line = m.next_line;
					pos_white[piece_id].col =  m.next_column;

					/* update the position of the pawn that is eaten */
					int pawn_id = chessboard[m.line][m.column - 1].type;
					chessboard[m.line][m.column - 1] = Piece();
					pos_black[pawn_id].line = (-1) * pos_black[pawn_id].line;
					pos_black[pawn_id].col = (-1) * pos_black[pawn_id].col;

					return m;
				}

				if (m.next_line == m.line + 1 && m.next_column == m.column + 1 && chessboard[m.next_line][m.next_column].type == EMPTY) {
					/* update the position of the to-be-moved piece */
					chessboard[m.next_line][m.next_column] = chessboard[m.line][m.column];
					chessboard[m.line][m.column] = Piece();
					pos_white[piece_id].line = m.next_line;
					pos_white[piece_id].col =  m.next_column;

					/* update the position of the pawn that is eaten */
					int pawn_id = chessboard[m.line][m.column + 1].type;
					chessboard[m.line][m.column + 1] = Piece();
					pos_black[pawn_id].line = (-1) * pos_black[pawn_id].line;
					pos_black[pawn_id].col = (-1) * pos_black[pawn_id].col;
					
					return m;
				}
			}

			if (opponentColor == BLACK) {
				if (m.next_line == m.line - 1 && m.next_column == m.column - 1 && chessboard[m.next_line][m.next_column].type == EMPTY) {
					/* update the position of the to-be-moved piece */
					chessboard[m.next_line][m.next_column] = chessboard[m.line][m.column];
					chessboard[m.line][m.column] = Piece();
					pos_black[piece_id].line = m.next_line;
					pos_black[piece_id].col =  m.next_column;

					/* update the position of the pawn that is eated */
					int pawn_id = chessboard[m.line][m.column - 1].type;  
					chessboard[m.line][m.column - 1] = Piece();
					pos_white[pawn_id].line = (-1) * pos_white[pawn_id].line;
					pos_white[pawn_id].col = (-1) * pos_white[pawn_id].col;

					return m;
				}

				if (m.next_line == m.line - 1 && m.next_column == m.column + 1 && chessboard[m.next_line][m.next_column].type == EMPTY) {
					/* update the position of the to-be-moved piece */
					chessboard[m.next_line][m.next_column] = chessboard[m.line][m.column];
					chessboard[m.line][m.column] = Piece();
					pos_black[piece_id].line = m.next_line;
					pos_black[piece_id].col =  m.next_column;

					/* update the position of the pawn that is eated */
					int pawn_id = chessboard[m.line][m.column + 1].type;
					chessboard[m.line][m.column + 1] = Piece();
					pos_white[pawn_id].line = (-1) * pos_white[pawn_id].line;
					pos_white[pawn_id].col = (-1) * pos_white[pawn_id].col;

					return m;
				}
			}
		}

		/* pawn promotion */
		bool is_pawn_promotion = false;
		piece_id = chessboard[m.line][m.column].type;

		if (piece_id >= 1 && piece_id <= 8 && (m.next_line == 1 || m.next_line == 8)) {
			if (opponentColor == BLACK) {
				queens_black[piece_id].line = m.next_line;
				queens_black[piece_id].col = m.next_column;

				pos_black[piece_id].line = -1 * pos_black[piece_id].line;
				pos_black[piece_id].col = -1 * pos_black[piece_id].col;
			} else {
				queens_white[piece_id].line = m.next_line;
				queens_white[piece_id].col = m.next_column;

				pos_white[piece_id].line = -1 * pos_white[piece_id].line;
				pos_white[piece_id].col = -1 * pos_white[piece_id].col;
			}

			is_pawn_promotion = true;
		} else {
			/* update the position of the piece that is moved */
			piece_id = chessboard[m.line][m.column].type;
			if (opponentColor == WHITE) {
				if (chessboard[m.line][m.column].type == Q) {
					piece_id = search_queen_index(m.line, m.column, queens_white);
				}

				if (piece_id == 0) {
					/* nu e o regina rezultata din pawn promotion */
					piece_id = chessboard[m.line][m.column].type;
					pos_white[piece_id].line = m.next_line;
					pos_white[piece_id].col = m.next_column;
				} else {
					queens_white[piece_id].line = m.next_line;
					queens_white[piece_id].col = m.next_column;
				}
			} else {
				if (chessboard[m.line][m.column].type == Q) {
					piece_id = search_queen_index(m.line, m.column, queens_black);
				}

				if (piece_id == 0) {
					/* nu e o regina rezultata din pawn promotion */
					piece_id = chessboard[m.line][m.column].type;
					pos_black[piece_id].line = m.next_line;
					pos_black[piece_id].col = m.next_column;
				} else {
					queens_black[piece_id].line = m.next_line;
					queens_black[piece_id].col = m.next_column;
				}
			}
		}

		/* if another piece is eaten, update that piece's position */
		piece_id = chessboard[m.next_line][m.next_column].type;
		if (piece_id != EMPTY) {
			if (color == WHITE) {
				int q_index = 0;
				if (chessboard[m.next_line][m.next_column].type == Q) {
					q_index = search_queen_index(m.next_line, m.next_column, queens_white);
				}

				if (q_index == 0) {
					pos_white[piece_id].line = -1 * m.next_line;
					pos_white[piece_id].col = -1 * m.next_column;
				} else {
					queens_white[q_index].line = -1 * m.next_line;
					queens_white[q_index].col = -1 * m.next_column;
				}
			} else {
				int q_index = 0;
				if (chessboard[m.next_line][m.next_column].type == Q) {
					q_index = search_queen_index(m.next_line, m.next_column, queens_black);
				}

				if (q_index == 0) {
					pos_black[piece_id].line = -1 * m.next_line;
					pos_black[piece_id].col = -1 * m.next_column;
				} else {
					queens_black[q_index].line = -1 * m.next_line;
					queens_black[q_index].col = -1 * m.next_column;
				}
			}
		}

		/* modify the chessboard */
		if (is_pawn_promotion == true) {
			chessboard[m.next_line][m.next_column] = Piece(Q, opponentColor);
			chessboard[m.line][m.column] = Piece();
		} else {
			chessboard[m.next_line][m.next_column] = chessboard[m.line][m.column];
			chessboard[m.line][m.column] = Piece();
		}

		return m;
	}

	// Construct the move that can be made by the pawn   
	vector<Move> pawn_movement(int line, int col, Color color) {
	
		/* if the pawn can take one of its opponents' pieces, it takes else
		 * if it can't, it simply moves forward (double move at the beginning)
		 * also, check the limits of the board  */
		vector<Move> pawn_moves;

		if (color == BLACK) {
			if (line - 1 >= 1 && col + 1 <= COLUMNS && chessboard[line - 1][col + 1].type != EMPTY) {
				if (chessboard[line - 1][col + 1].color != chessboard[line][col].color)  {
					pawn_moves.push_back(Move(line, col, line - 1, col + 1)); 
				}
			}

			if (line - 1 >= 1 && col - 1 >= 1 && chessboard[line - 1][col - 1].type != EMPTY) {
				if (chessboard[line - 1][col - 1].color != chessboard[line][col].color) {
					pawn_moves.push_back(Move(line, col, line - 1, col - 1)); 
				}
			}

			if (line - 2 >= 1 && chessboard[line - 1][col].type == EMPTY) {
				if (line ==  7 && chessboard[line - 2][col].type == EMPTY) {
					pawn_moves.push_back(Move(line, col, line - 2, col));
				}
				
				pawn_moves.push_back(Move(line, col, line - 1, col));
	    	}
		}
		
		if (color == WHITE) {
			if (line + 1 <= ROWS && col + 1 <= COLUMNS && chessboard[line + 1][col + 1].type != EMPTY) {
				if (chessboard[line + 1][col + 1].color != chessboard[line][col].color)  {
					pawn_moves.push_back(Move(line, col, line + 1, col + 1));
				}
			}

			if (line + 1 <= ROWS && col - 1 >= 1 && chessboard[line + 1][col - 1].type != EMPTY) {
				if (chessboard[line + 1][col - 1].color != chessboard[line][col].color) {
					pawn_moves.push_back(Move(line, col, line + 1, col - 1));
				}
			}

			if (line + 2 <= ROWS && chessboard[line + 1][col].type == EMPTY) {
				if (line ==  2 && chessboard[line + 2][col].type == EMPTY) {
					pawn_moves.push_back(Move(line, col, line + 2, col));
				}
				
				pawn_moves.push_back(Move(line, col, line + 1, col));
	    	}
		}

		return pawn_moves; 
	}

	vector<Move> rook_movement(int line, int col, Color color) {
		vector<Move> rook_moves;
		Color opponent_color;

		if (color == BLACK) {
			opponent_color = WHITE;
		} else {
			opponent_color = BLACK;
		}

		/* search available moves on column */
			/* forward */
		int l = line - 1;
		while (l > 0 && chessboard[l][col].type == EMPTY) {
			rook_moves.push_back(Move(line, col, l, col));
			l--;
		}

		if (l > 0 && chessboard[l][col].color == opponent_color){
			rook_moves.push_back(Move(line, col, l, col));
		}

			/* backwards */
		l = line + 1;
		while (l <= ROWS && chessboard[l][col].type == EMPTY) {
			rook_moves.push_back(Move(line, col, l, col));
			l++;
		}

		if (l <= ROWS && chessboard[l][col].color == opponent_color) {
			rook_moves.push_back(Move(line, col, l, col));
		}

		/* search available moves on line */
			/* left */
		int c = col - 1;
		while (c > 0 && chessboard[line][c].type == EMPTY){
			rook_moves.push_back(Move(line, col, line, c));
			c--;
		}

		if (c > 0 && chessboard[line][c].color == opponent_color){
			rook_moves.push_back(Move(line, col, line, c));
		}

			/* right */
		c = col + 1;
		while (c <= COLUMNS && chessboard[line][c].type == EMPTY){
			rook_moves.push_back(Move(line, col, line, c));
			c++;
		}

		if (c <= COLUMNS && chessboard[line][c].color == opponent_color){
			rook_moves.push_back(Move(line, col, line, c));
		}

		return rook_moves;
	}
	
	vector<Move> bishop_movement(int line, int col, Color color) {
		vector<Move> bishop_moves;
		Color opponent_color;

		if (color == BLACK) {
			opponent_color = WHITE;
		} else {
			opponent_color = BLACK;
		}

		/* right upper corner */
		int l = line + 1;
		int c = col + 1;

		while (l <= ROWS && c <= COLUMNS && chessboard[l][c].type == EMPTY) {
			bishop_moves.push_back(Move(line, col, l, c));
			l++;
			c++;
		}

		if (l <= ROWS && c <= COLUMNS && chessboard[l][c].color == opponent_color) {
			bishop_moves.push_back(Move(line, col, l, c));
		}

		/* left upper corner */
		l = line + 1;
		c = col - 1;

		while (l <= ROWS && c > 0 && chessboard[l][c].type == EMPTY) {
			bishop_moves.push_back(Move(line, col, l, c));
			l++;
			c--;
		}

		if (l <= ROWS && c > 0 && chessboard[l][c].color == opponent_color) {
			bishop_moves.push_back(Move(line, col, l, c));
		}

		/* right lower corner */
		l = line - 1;
		c = col + 1;

		while (l > 0 && c <= COLUMNS && chessboard[l][c].type == EMPTY) {
			bishop_moves.push_back(Move(line, col, l, c));
			l--;
			c++;
		}

		if (l > 0 && c <= COLUMNS && chessboard[l][c].color == opponent_color) {
			bishop_moves.push_back(Move(line, col, l, c));
		}

		/* left lower corner */
		l = line - 1;
		c = col - 1;

		while (l > 0 && c > 0 && chessboard[l][c].type == EMPTY) {
			bishop_moves.push_back(Move(line, col, l, c));
			l--;
			c--;
		}

		if (l > 0 && c > 0 && chessboard[l][c].color == opponent_color) {
			bishop_moves.push_back(Move(line, col, l, c));
		}

		return bishop_moves;
	}

	vector<Move> knight_movement(int line, int col, Color color) {
		vector<Move> knight_moves;
		Color opponent_color;

		if (color == BLACK) {
			opponent_color = WHITE;
		} else {
			opponent_color = BLACK;
		}

		/* standing upper right L's */
		int l = line + 2;
		int c = col + 1;
		if (l <= ROWS && c <= COLUMNS && chessboard[l][c].color != color) {
			knight_moves.push_back(Move(line, col, l, c));
		}

		/* standing upper left L's */
		l = line + 2;
		c = col - 1;
		if (l <= ROWS && c > 0 && chessboard[l][c].color != color) {
			knight_moves.push_back(Move(line, col, l, c));
		}

		/* standing lower right L's */
		l = line - 2;
		c = col + 1;
		if (l > 0 && c <= COLUMNS && chessboard[l][c].color != color) {
			knight_moves.push_back(Move(line, col, l, c));
		}

		/* standing lower left L's */
		l = line - 2;
		c = col - 1;
		if (l > 0 && c > 0 && chessboard[l][c].color != color) {
			knight_moves.push_back(Move(line, col, l, c));
		}

		/* sleeping upper right L's */
		l = line + 1;
		c = col + 2;
		if (l <= ROWS && c <= COLUMNS && chessboard[l][c].color != color) {
			knight_moves.push_back(Move(line, col, l, c));
		}

		/* sleeping upper left L's */
		l = line + 1;
		c = col - 2;
		if (l <= ROWS && c > 0 && chessboard[l][c].color != color) {
			knight_moves.push_back(Move(line, col, l, c));
		}

		/* sleeping lower right */
		l = line - 1;
		c = col + 2;
		if (l > 0 && c <= COLUMNS && chessboard[l][c].color != color) {
			knight_moves.push_back(Move(line, col, l, c));
		}

		/* sleeping lower left */
		l = line - 1; 
		c = col - 2;
		if (l > 0 && c > 0 && chessboard[l][c].color != color) {
			knight_moves.push_back(Move(line, col, l, c));
		}

		return knight_moves;
	}

	vector<Move> queen_movement(int line, int col, Color color) {
		vector<Move> queen_moves;
		Color opponent_color;

		if (color == BLACK) {
			opponent_color = WHITE;
		} else {
			opponent_color = BLACK;
		}

		/*** left-right-backwards-forward moves ***/
		/* search available moves on column */
			/* forward */
		int l = line - 1;
		while (l > 0 && chessboard[l][col].type == EMPTY) {
			queen_moves.push_back(Move(line, col, l, col));
			l--;
		}

		if (l > 0 && chessboard[l][col].color == opponent_color){
			queen_moves.push_back(Move(line, col, l, col));
		}

			/* backwards */
		l = line + 1;
		while (l <= ROWS && chessboard[l][col].type == EMPTY) {
			queen_moves.push_back(Move(line, col, l, col));
			l++;
		}

		if (l <= ROWS && chessboard[l][col].color == opponent_color) {
			queen_moves.push_back(Move(line, col, l, col));
		}

		/* search available moves on line */
			/* left */
		int c = col - 1;
		while (c > 0 && chessboard[line][c].type == EMPTY) {
			queen_moves.push_back(Move(line, col, line, c));
			c--;
		}

		if (c > 0 && chessboard[line][c].color == opponent_color) {
			queen_moves.push_back(Move(line, col, line, c));
		}

			/* right */
		c = col + 1;
		while (c <= COLUMNS && chessboard[line][c].type == EMPTY) {
			queen_moves.push_back(Move(line, col, line, c));
			c++;
		}

		if (c <= COLUMNS && chessboard[line][c].color == opponent_color) {
			queen_moves.push_back(Move(line, col, line, c));
		}

		/*** diagonal movement ***/
		/* upper right corner */
		l = line + 1;
		c = col + 1;
		while (l <= ROWS && c <= COLUMNS && chessboard[l][c].type == EMPTY) {
			queen_moves.push_back(Move(line, col, l, c));
			l++;
			c++;
		}

		if (l <= ROWS && c <= COLUMNS && chessboard[l][c].color == opponent_color) {
			queen_moves.push_back(Move(line, col, l, c));
		}

		/* upper left corner */
		l = line + 1;
		c = col - 1;

		while (l <= ROWS && c > 0 && chessboard[l][c].type == EMPTY) {
			queen_moves.push_back(Move(line, col, l, c));
			l++;
			c--;
		}

		if (l <= ROWS && c > 0 && chessboard[l][c].color == opponent_color) {
			queen_moves.push_back(Move(line, col, l, c));
		}

		/* lower right corner */
		l = line - 1;
		c = col + 1;

		while (l > 0 && c <= COLUMNS && chessboard[l][c].type == EMPTY) {
			queen_moves.push_back(Move(line, col, l, c));
			l--;
			c++;
		}

		if (l > 0 && c <= COLUMNS && chessboard[l][c].color == opponent_color) {
			queen_moves.push_back(Move(line, col, l, c));
		}

		/* lower left corner */
		l = line - 1;
		c = col - 1;

		while (l > 0 && c > 0 && chessboard[l][c].type == EMPTY) {
			queen_moves.push_back(Move(line, col, l, c));
			l--;
			c--;
		}

		if (l > 0 && c > 0 && chessboard[l][c].color == opponent_color) {
			queen_moves.push_back(Move(line, col, l, c));
		}

		return queen_moves;
	}

	vector<Move> king_movement(int line, int col, Color color) {
		vector<Move> king_moves;
		Color opponent_color;

		if (color == BLACK) {
			opponent_color == WHITE;
		} else {
			opponent_color == BLACK;
		}

		/* upper right corner */
		int l = line + 1;
		int c = col + 1;

		if (l <= ROWS && c <= COLUMNS && chessboard[l][c].color != color) {
			king_moves.push_back(Move(line, col, l, c));
		}

		/* upper left corner */
		l = line + 1;
		c = col - 1;

		if (l <= ROWS && c > 0 && chessboard[l][c].color != color) {
			king_moves.push_back(Move(line, col, l, c));
		}

		/* lower right corner */
		l = line - 1;
		c = col + 1;

		if (l > 0 && c <= COLUMNS && chessboard[l][c].color != color) {
			king_moves.push_back(Move(line, col, l, c));
		}

		/* lower left corner */
		l = line - 1;
		c = col - 1;

		if (l > 0 && c > 0 && chessboard[l][c].color != color) {
			king_moves.push_back(Move(line, col, l, c));
		}

		/* right line movement */
		l = line;
		c = col + 1;

		if (c <= COLUMNS && chessboard[l][c].color != color) {
			king_moves.push_back(Move(line, col, l, c));
		}

		/* left line movement */
		l = line;
		c = col - 1;
		
		if (c > 0 && chessboard[l][c].color != color) {
			king_moves.push_back(Move(line, col, l, c));
		}

		/* forward line movement */
		c = col;
		l = line + 1;

		if (l <= ROWS && chessboard[l][c].color != color) {
			king_moves.push_back(Move(line, col, l, c));
		}

		/* backwards line movement */
		c = col;
		l = line - 1;

		if (l > 0 && chessboard[l][c].color != color) {
			king_moves.push_back(Move(line, col, l, c));
		}

		return king_moves;
	}

	/* rocada micuta */
	void makeMoveKingCastling(Move m, Color c) {
		chessboard[m.next_line][m.next_column] = chessboard[m.line][m.column];
		chessboard[m.line][m.column] = Piece();
		chessboard[m.line][m.column + 1] = chessboard[m.line][m.column + 3];
		chessboard[m.line][m.column + 3] = Piece();

		if (c == WHITE) {
			pos_white[KING].col += 2;
			pos_white[R2].col -= 2;
			white_king_moved = true;
			white_rook2_moved = true;
		} else {
			pos_black[KING].col += 2;
			pos_black[R2].col -= 2;
			black_king_moved = true;
			black_rook2_moved = true;
		}
	}

	/* rocada mare */
	void makeMoveQueenCastling(Move m, Color c) {
		chessboard[m.next_line][m.next_column] = chessboard[m.line][m.column]; 
		chessboard[m.line][m.column] = Piece();
		chessboard[m.line][m.column - 1] = chessboard[m.line][m.column - 4];
		chessboard[m.line][m.column - 4] = Piece();
	
		if (c == WHITE) {
			pos_white[KING].col -= 2;
			pos_white[R1].col += 3;
			white_king_moved = true;
			white_rook1_moved = true;
		} else {
			pos_black[KING].col -= 2;
			pos_black[R1].col += 3;
			black_king_moved = true;
			black_rook1_moved = true;
		}
	}

	void unmakeMoveKingCastling(Move m, Color c) {
		chessboard[m.line][m.column] = chessboard[m.next_line][m.next_column];
		chessboard[m.next_line][m.next_column] = Piece();
		chessboard[m.line][m.column + 3] = chessboard[m.line][m.column + 1];
		chessboard[m.line][m.column + 1] = Piece();

		if (c == WHITE) {
			pos_white[KING].col -= 2;
			pos_white[R2].col += 2;
			white_king_moved = false;
			white_rook2_moved = false;
		} else {
			pos_black[KING].col -= 2;
			pos_black[R2].col += 2;
			black_king_moved = false;
			black_rook2_moved = false;
		}
	}

	void unmakeMoveQueenCastling(Move m, Color c) {
		chessboard[m.line][m.column] = chessboard[m.next_line][m.next_column]; 
		chessboard[m.next_line][m.next_column] = Piece();
		chessboard[m.line][m.column - 4] = chessboard[m.line][m.column - 1];
		chessboard[m.line][m.column - 1] = Piece();

		if (c == WHITE) {
			pos_white[KING].col += 2;
			pos_white[R1].col -= 3;
			white_king_moved = false;
			white_rook1_moved = false;
		} else {
			pos_black[KING].col += 2;
			pos_black[R1].col -= 3;
			black_king_moved = false;
			black_rook1_moved = false;
		}
	}

	bool unmakeCastling(Move m, Color c) {
		if (m.line == 1 && m.column == 5 && m.next_line == 1 && m.next_column == 7	|| m.line == 8 && m.column == 5 && m.next_line == 8 && m.next_column == 7) {
			unmakeMoveKingCastling(m, c);
			return true;
		}

        if (m.line == 1 && m.column == 5 && m.next_line == 1 && m.next_column == 3	|| m.line == 8 && m.column == 5 && m.next_line == 8 && m.next_column == 3) {
			unmakeMoveQueenCastling(m, c);
			return true;
		}

		return false;
	}

	bool is_KingCastling_possible(Color color) {
		int line, kingLine, kingColumn;

		if (color == WHITE) {
			if (white_king_moved == true || white_rook2_moved == true) {
				return false;
			}
		} else {
			if (black_king_moved == true || black_rook2_moved == true) {
				return false;
			}
		}

		if (color == WHITE) {
			line = 1;
			kingLine = pos_white[KING].line;
			kingColumn = pos_white[KING].col;
		} else {
			line = 8;
			kingLine = pos_black[KING].line;
			kingColumn = pos_black[KING].col;
		}

		if (kingLine != line || kingColumn != 5) {
			return false;
		}

		if (chessboard[kingLine][kingColumn + 1].type != EMPTY ||
				chessboard[kingLine][kingColumn + 2].type != EMPTY) {
			return false;
		}

		if (check(color)) {
			return false;
		}

		/* check checkmate for king on the next column */
		chessboard[kingLine][kingColumn + 1] = chessboard[kingLine][kingColumn];
		chessboard[kingLine][kingColumn] = Piece();

		if (color == WHITE) { 
			pos_white[KING].col = kingColumn + 1;
		} else { 
			pos_black[KING].col = kingColumn + 1;
		}

		if (check(color)) {
			/* remake the initial configuration (col + 1) */
			chessboard[kingLine][kingColumn] = chessboard[kingLine][kingColumn + 1];
			chessboard[kingLine][kingColumn + 1] = Piece();
			if (color == WHITE) {
				pos_white[KING].col = kingColumn;
			} else {
				pos_black[KING].col = kingColumn;
			}
			return false;
		} else {
			/* remake the initial configuration (col + 1) */
			chessboard[kingLine][kingColumn] = chessboard[kingLine][kingColumn + 1];
			chessboard[kingLine][kingColumn + 1] = Piece();
			if (color == WHITE) {
				pos_white[KING].col = kingColumn;
			} else {
				pos_black[KING].col = kingColumn;
			}
		}

		/* check checkmate for king on the next two column (col + 2) */
		chessboard[kingLine][kingColumn + 2] = chessboard[kingLine][kingColumn];
		chessboard[kingLine][kingColumn] = Piece();

		if (color == WHITE) { 
			pos_white[KING].col = kingColumn + 2;
		} else { 
			pos_black[KING].col = kingColumn + 2;
		}

		if (check(color)) {
			/* remake the initial configuration */
			chessboard[kingLine][kingColumn] = chessboard[kingLine][kingColumn + 2];
			chessboard[kingLine][kingColumn + 2] = Piece();
			if (color == WHITE) {
				pos_white[KING].col = kingColumn;
			} else {
				pos_black[KING].col = kingColumn;
			}
			return false;
		} else {
			/* remake the initial configuration */
			chessboard[kingLine][kingColumn] = chessboard[kingLine][kingColumn + 2];
			chessboard[kingLine][kingColumn + 2] = Piece();
			if (color == WHITE) {
				pos_white[KING].col = kingColumn;
			} else {
				pos_black[KING].col = kingColumn;
			}
		}

		return true;
	}

	bool is_QueenCastling_possible(Color color) {
		int line, kingLine, kingColumn;

		if (color == WHITE) {
			if (white_king_moved == true || white_rook1_moved == true) {
				return false;
			}
		} else {
			if (black_king_moved == true || black_rook1_moved == true) {
				return false;
			}
		}

		if (color == WHITE) {
			line = 1;
			kingLine = pos_white[KING].line;
			kingColumn = pos_white[KING].col;
		} else {
			line = 8;
			kingLine = pos_black[KING].line;
			kingColumn = pos_black[KING].col;
		}

		if (kingLine != line || kingColumn != 5) {
			return false;
		}

		if (chessboard[kingLine][kingColumn - 1].type != 0 ||
				chessboard[kingLine][kingColumn - 2].type != 0 || 
				chessboard[kingLine][kingColumn - 3].type != 0) {
			return false;
		}

		if (check(color)) {
			return false;
		}


		/* check checkmate for king on the next column */
		chessboard[kingLine][kingColumn - 1] = chessboard[kingLine][kingColumn];
		chessboard[kingLine][kingColumn] = Piece();

		if (color == WHITE) { 
			pos_white[KING].col = kingColumn - 1;
		} else { 
			pos_black[KING].col = kingColumn - 1;
		}

		if (check(color)) {
			/* remake the initial configuration (col + 1) */
			chessboard[kingLine][kingColumn] = chessboard[kingLine][kingColumn - 1];
			chessboard[kingLine][kingColumn - 1] = Piece();
			if (color == WHITE) {
				pos_white[KING].col = kingColumn;
			} else {
				pos_black[KING].col = kingColumn;
			}

			return false;
		} else {
			/* remake the initial configuration (col + 1) */
			chessboard[kingLine][kingColumn] = chessboard[kingLine][kingColumn - 1];
			chessboard[kingLine][kingColumn - 1] = Piece();
			if (color == WHITE) {
				pos_white[KING].col = kingColumn;
			} else {
				pos_black[KING].col = kingColumn;
			}
		}

		/* check checkmate for king on the next two column (col + 2) */
		chessboard[kingLine][kingColumn - 2] = chessboard[kingLine][kingColumn];
		chessboard[kingLine][kingColumn] = Piece();

		if (color == WHITE) { 
			pos_white[KING].col = kingColumn - 2;
		} else { 
			pos_black[KING].col = kingColumn - 2;
		}

		if (check(color)) {
			/* remake the initial configuration */
			chessboard[kingLine][kingColumn] = chessboard[kingLine][kingColumn - 2];
			chessboard[kingLine][kingColumn - 2] = Piece();
			if (color == WHITE) {
				pos_white[KING].col = kingColumn;
			} else {
				pos_black[KING].col = kingColumn;
			}

			return false;
		} else {
			/* remake the initial configuration */
			chessboard[kingLine][kingColumn] = chessboard[kingLine][kingColumn - 2];
			chessboard[kingLine][kingColumn - 2] = Piece();
			if (color == WHITE) {
				pos_white[KING].col = kingColumn;
			} else {
				pos_black[KING].col = kingColumn;
			}
		}

		return true;
	}

	bool makeCastling(Move m, Color c) {
		if (m.line == 1 && m.column == 5 && m.next_line == 1 && m.next_column == 7	|| m.line == 8 && m.column == 5 && m.next_line == 8 && m.next_column == 7) {
			if (is_KingCastling_possible(c)) {
				makeMoveKingCastling(m, c);
				return true;
			}
		}

        if (m.line == 1 && m.column == 5 && m.next_line == 1 && m.next_column == 3	|| m.line == 8 && m.column == 5 && m.next_line == 8 && m.next_column == 3) {
			if (is_QueenCastling_possible(c)) {
				makeMoveQueenCastling(m, c);
				return true;
			}
        }

		return false;
	}

	vector<Move> castling_movement(Color color) {
		vector<Move> castling_moves;

		if (color == WHITE) {
			if (white_king_moved == true) {
				if (white_rook1_moved && is_QueenCastling_possible(color)) {
					castling_moves.push_back(Move(1, 5, 1, 3));
				}

				if (white_rook2_moved && is_KingCastling_possible(color)) {
					castling_moves.push_back(Move(1, 5, 1, 7));
				}
			}
		}

		if (color == BLACK) {
			if (black_king_moved == true) {
				if (black_rook1_moved && is_QueenCastling_possible(color)) {
					castling_moves.push_back(Move(8, 5, 8, 3));
				}
			}

			if (black_rook2_moved && is_KingCastling_possible(color)) {
				castling_moves.push_back(Move(8, 5, 8, 7));
			}
		}

		return castling_moves;
	}

	vector<Move> generateAllWhiteMoves(Color color) {
		vector<Move> moves, pawn_moves, rook_moves, bishop_moves, 
			knight_moves, queen_moves, king_moves, castling_moves, bonus_queen_moves;

		for (int i = 1; i <= 8; i++) {
			if (pos_white[i].line > 0 && pos_white[i].col > 0) {
				pawn_moves = pawn_movement(pos_white[i].line, pos_white[i].col, color);
				moves.insert(moves.end(), pawn_moves.begin(), pawn_moves.end());
			}
		}

		for (int i = R1; i <= R2; i++) {
			if (pos_white[i].line > 0 && pos_white[i].col > 0) {
				rook_moves = rook_movement(pos_white[i].line, pos_white[i].col, color);
				moves.insert(moves.end(), rook_moves.begin(), rook_moves.end());
			}
		}
		
		for (int i = B1; i <= B2; i++) {
			if (pos_white[i].line > 0 && pos_white[i].col > 0) {
				bishop_moves = bishop_movement(pos_white[i].line, pos_white[i].col, color);
				moves.insert(moves.end(), bishop_moves.begin(), bishop_moves.end());
			}
		}

		for (int i = K1; i <= K2; i++) {
			if (pos_white[i].line > 0 && pos_white[i].col > 0) {
				knight_moves = knight_movement(pos_white[i].line, pos_white[i].col, color);
				moves.insert(moves.end(), knight_moves.begin(), knight_moves.end());
			}
		}

		if (pos_white[Q].line > 0 && pos_white[Q].col > 0) { 
			queen_moves = queen_movement(pos_white[Q].line, pos_white[Q].col, color);
			moves.insert(moves.end(), queen_moves.begin(), queen_moves.end());
		}

		if (pos_white[KING].line > 0 && pos_white[KING].col > 0) {
			king_moves = king_movement(pos_white[KING].line, pos_white[KING].col, color);
			moves.insert(moves.end(), king_moves.begin(), king_moves.end());
		}

		castling_moves = castling_movement(color);
		moves.insert(moves.end(), castling_moves.begin(), castling_moves.end());

		/* check for promoted pawns */
		for (int i = P1; i <= P8; i++) {
			if(queens_white[i].line > 0 && queens_white[i].col > 0) {
				bonus_queen_moves = queen_movement(queens_white[i].line, queens_white[i].col, color);
				moves.insert(moves.end(), bonus_queen_moves.begin(), bonus_queen_moves.end());
			}
		}

		return moves;
	}

	vector<Move> generateAllBlackMoves(Color color) {
		vector<Move> moves, pawn_moves, rook_moves, bishop_moves, 
			knight_moves, queen_moves, king_moves, castling_moves, bonus_queen_moves;

		for (int i = 1; i <= 8; i++) {
			if (pos_black[i].line > 0 && pos_black[i].col > 0) {
				pawn_moves = pawn_movement(pos_black[i].line, pos_black[i].col, color);
				moves.insert(moves.end(), pawn_moves.begin(), pawn_moves.end());
			}
		}

		for (int i = R1; i <= R2; i++) {
			if (pos_black[i].line > 0 && pos_black[i].col > 0) {
				rook_moves = rook_movement(pos_black[i].line, pos_black[i].col, color);
				moves.insert(moves.end(), rook_moves.begin(), rook_moves.end());
			}
		}
		
		for (int i = B1; i <= B2; i++) {
			if (pos_black[i].line > 0 && pos_black[i].col > 0) {
				bishop_moves = bishop_movement(pos_black[i].line, pos_black[i].col, color);
				moves.insert(moves.end(), bishop_moves.begin(), bishop_moves.end());
			}
		}

		for (int i = K1; i <= K2; i++) {
			if (pos_black[i].line > 0 && pos_black[i].col > 0) {
				knight_moves = knight_movement(pos_black[i].line, pos_black[i].col, color);
				moves.insert(moves.end(), knight_moves.begin(), knight_moves.end());
			}
		}

		if (pos_black[Q].line > 0 && pos_black[Q].col > 0) {
			queen_moves = queen_movement(pos_black[Q].line, pos_black[Q].col, color);
			moves.insert(moves.end(), queen_moves.begin(), queen_moves.end());
		}

		if (pos_black[KING].line > 0 && pos_black[KING].col > 0) {
			king_moves = king_movement(pos_black[KING].line, pos_black[KING].col, color);
			moves.insert(moves.end(), king_moves.begin(), king_moves.end());
		}

		castling_moves = castling_movement(color);
		moves.insert(moves.end(), castling_moves.begin(), castling_moves.end());

		/* check for promoted pawns */
		for (int i = P1; i <= P8; i++) {
			if(queens_black[i].line > 0 && queens_black[i].col > 0) {
				bonus_queen_moves = queen_movement(queens_black[i].line, queens_black[i].col, color);
				moves.insert(moves.end(), bonus_queen_moves.begin(), bonus_queen_moves.end());
			}
		}
		
		return moves;
	}

	vector<Move> generateAllMoves(Player player) {
		vector<Move> moves;
		Color c;

		if (player == SAHEREZADE) {
			c = this->color;
		} else {
			if (this->color == WHITE) {
				c = BLACK;
			} else {
				c = WHITE;
			}
		}

		if (c == WHITE) {
			moves = generateAllWhiteMoves(c);
		} else {
			moves = generateAllBlackMoves(c);
		}

		return moves;
	}

	bool empty_column(int kingLine, int opponentLine, int col) {
		int start, end;
		
		if (kingLine < opponentLine) {
			start = kingLine;
			end = opponentLine;
		} else {
			start = opponentLine;
			end = kingLine;
		}

		for (int i = start + 1; i <= end - 1; i++) {
			if (chessboard[i][col].type != EMPTY) {
				return false;
			}
		}
		return true;
	}

	/* intreaga coloana pe care se afla regele */
	bool check_column(Color color) {
		if (color == WHITE) {
			if (pos_black[Q].col == pos_white[KING].col) {
				if (empty_column(pos_white[KING].line, pos_black[Q].line, pos_white[KING].col) == true) {
					return true;
				}
			}

			if (pos_black[R1].col == pos_white[KING].col) {
				if (empty_column(pos_white[KING].line, pos_black[R1].line, pos_white[KING].col) == true) {
					return true;
				}
			}

			if (pos_black[R2].col == pos_white[KING].col) {
				if (empty_column(pos_white[KING].line, pos_black[R2].line, pos_white[KING].col) == true) {
					return true;
				}
			}

			for (int i = 1; i <= 8; i++) {
				if (queens_black[i].col == pos_white[KING].col) {
					if (empty_column(pos_white[KING].line, queens_black[i].line, pos_white[KING].col) == true) {
						return true;
					}
				}
			} 
		}

		if (color == BLACK) {
			if (pos_white[Q].col == pos_black[KING].col) {
				if (empty_column(pos_black[KING].line, pos_white[Q].line, pos_black[KING].col) == true) {
					return true;
				}
			}

			if (pos_white[R1].col == pos_black[KING].col) {
				if (empty_column(pos_black[KING].line, pos_white[R1].line, pos_black[KING].col) == true) {
					return true;
				}
			}

			if (pos_white[R2].col == pos_black[KING].col) {
				if (empty_column(pos_black[KING].line, pos_white[R2].line, pos_black[KING].col) == true) {
					return true;
				}
			}

			for (int i = 1; i <= 8; i++) {
				if (queens_white[i].col == pos_black[KING].col) {
					if (empty_column(pos_black[KING].line, queens_white[i].line, pos_black[KING].col) == true) {
						return true;
					}
				}
			} 	
		}
		return false;
	}

	bool empty_line(int kingColumn, int opponentColumn, int line) {
		int start, end;

		if (kingColumn < opponentColumn) {
			start = kingColumn;
			end = opponentColumn;
		} else {
			start = opponentColumn;
			end = kingColumn;
		}

		for (int j = start + 1; j <= end - 1; j++) {
			if (chessboard[line][j].type != EMPTY) {
				return false;
			}
		}

		return true;
	}

	/* intreaga linie pe care se afla regele */
	bool check_line(Color color) {
		if (color == WHITE) {
			if (pos_black[Q].line == pos_white[KING].line) {
				if (empty_line(pos_white[KING].col, pos_black[Q].col, pos_white[KING].line) == true) {
					return true;
				}
			}

			if (pos_black[R1].line == pos_white[KING].line) {
				if (empty_line(pos_white[KING].col, pos_black[R1].col, pos_white[KING].line) == true) {
					return true;
				}
			}

			if (pos_black[R2].line == pos_white[KING].line) {
				if (empty_line(pos_white[KING].col, pos_black[R2].col, pos_white[KING].line) == true) {
					return true;
				}
			}

			for (int i = 1; i <= 8; i++) {
				if (queens_black[i].line == pos_white[KING].line) {
					if (empty_line(pos_white[KING].col, queens_black[i].col, pos_white[KING].line) == true) {
						return true;
					}
				}
			} 
		}

		if (color == BLACK) {
			if (pos_white[Q].line == pos_black[KING].line) {
				if (empty_line(pos_black[KING].col, pos_white[Q].col, pos_black[KING].line) == true) {
					return true;
				}
			}

			if (pos_white[R1].line == pos_black[KING].line) {
				if (empty_line(pos_black[KING].col, pos_white[R1].col, pos_black[KING].line) == true) {
					return true;
				}
			}

			if (pos_white[R2].line == pos_black[KING].line) {
				if (empty_line(pos_black[KING].col, pos_white[R2].col, pos_black[KING].line) == true) {
					return true;
				}
			} 

			for (int i = 1; i <= 8; i++) {
				if (queens_white[i].line == pos_black[KING].line) {
					if (empty_line(pos_black[KING].col, queens_white[i].col, pos_black[KING].line) == true) {
						return true;
					}
				}
			}	
		}
		return false;
	}

	/* diagonala din dreapta sus (raportat la rege!) */
	bool check_rightUp(Color color) {
		if (color == WHITE) {
			int kingLine = pos_white[KING].line;
			int kingColumn = pos_white[KING].col;
			
			/* if a pawn checks the king */
			if (kingLine < ROWS && kingColumn < COLUMNS) {
				for (int i = P1; i <= P8; i++) {
					if (chessboard[kingLine + 1][kingColumn + 1].type == i
					&& chessboard[kingLine + 1][kingColumn + 1].color == BLACK) {
						return true;
					}
				}
			}

			/* if bishops or the queen check the king */
			int line = kingLine + 1;
			int col = kingColumn + 1;
			int bec = 1;

			while (line <= ROWS && col <= COLUMNS && bec) {
				if (chessboard[line][col].type != EMPTY) {
					if (chessboard[line][col].color == BLACK) {
						if (chessboard[line][col].type == B1 || chessboard[line][col].type == B2 || chessboard[line][col].type == Q) {
							return true;
						}
					}
					bec = 0;
				}
				line++;
				col++;
			}
		}

		if (color == BLACK) {
			int kingLine = pos_black[KING].line;
			int kingColumn = pos_black[KING].col;

			/* if a pawn checks the king */
			if (kingLine > 1 && kingColumn > 1) {
				for (int i = P1; i <= P8; i++) {
					if (chessboard[kingLine - 1][kingColumn - 1].type == i
						&& chessboard[kingLine - 1][kingColumn - 1].color == WHITE) {
						return true;
					}
				}
			}

			/* if a bishop or a queen checks the king */
			int line = kingLine - 1;
			int col = kingColumn - 1;
			int bec = 1;

			while (line > 0 && col > 0 && bec) {
				if (chessboard[line][col].type != EMPTY) {
					if (chessboard[line][col].color == WHITE) {
						if (chessboard[line][col].type == B1 || chessboard[line][col].type == B2 || chessboard[line][col].type == Q) {
							return true;
						}
					}
					bec = 0;
				}
				line--;
				col--;
			}
		}

		return false;
	}

	/* diagonala din stanga sus (raportat la rege!) */
	bool check_leftUp(Color color) {
		if (color == WHITE) {
			int kingLine = pos_white[KING].line;
			int kingColumn = pos_white[KING].col;

			/* if a pawn checks the king */
			if (kingLine < ROWS && kingColumn > 1) {
				for (int i = P1; i <= P8; i++) {
					if (chessboard[kingLine + 1][kingColumn - 1].type == i
					&& chessboard[kingLine + 1][kingColumn - 1].color == BLACK) {
						return true;
					}
				}
			}

			/* if bishops or the queen check the king */
			int line = kingLine + 1;
			int col = kingColumn - 1;
			int bec = 1;

			while (line <= ROWS && col > 0 && bec) {
				if (chessboard[line][col].type != EMPTY) {
					if (chessboard[line][col].color == BLACK) {
						if (chessboard[line][col].type == B1 || chessboard[line][col].type == B2 || chessboard[line][col].type == Q) {
							return true;
						}
					}
					bec = 0;
				}
				line++;
				col--;
			}
		}

		if (color == BLACK) {
			int kingLine = pos_black[KING].line;
			int kingColumn = pos_black[KING].col;

			/* if a pawn checks the king */
			if (kingLine > 1 && kingColumn < COLUMNS) {
				for (int i = P1; i <= P8; i++) {
					if (chessboard[kingLine - 1][kingColumn + 1].type == i
						&& chessboard[kingLine - 1][kingColumn + 1].color == WHITE) {
						return true;
					}
				}
			}

			/* if a bishop or a queen checks the king */
			int line = kingLine - 1;
			int col = kingColumn + 1;
			int bec = 1;

			while (line > 0 && col <= COLUMNS && bec) {
				if (chessboard[line][col].type != EMPTY) {
					if (chessboard[line][col].color == WHITE) {
						if (chessboard[line][col].type == B1 || chessboard[line][col].type == B2 || chessboard[line][col].type == Q) {
							return true;
						}
					}
					bec = 0;
				}
				line--;
				col++;
			}
		}

		return false;
	}

	/* diagonala din dreapta jos (raportat la rege!) */
	bool check_rightDown(Color color) {
		if (color == WHITE) {

			int kingLine = pos_white[KING].line;
			int kingColumn = pos_white[KING].col;

			/* if bishops or the queen check the king */
			int line = kingLine - 1;
			int col = kingColumn + 1;
			int bec = 1;

			while (line > 0 && col <= COLUMNS && bec) {
				if (chessboard[line][col].type != EMPTY) {
					if (chessboard[line][col].color == BLACK) {
						if (chessboard[line][col].type == B1 || chessboard[line][col].type == B2 || chessboard[line][col].type == Q) {
							return true;
						}
					}
					bec = 0;
				}
				line--;
				col++;
			}
		}

		if (color == BLACK) {

			int kingLine = pos_black[KING].line;
			int kingColumn = pos_black[KING].col;

			/* if a bishop or a queen checks the king */
			int line = kingLine + 1;
			int col = kingColumn - 1;
			int bec = 1;

			while (line <= ROWS && col > 0 && bec) {
				if (chessboard[line][col].type != EMPTY) {
					if (chessboard[line][col].color == WHITE) {
						if (chessboard[line][col].type == B1 || chessboard[line][col].type == B2 || chessboard[line][col].type == Q) {
							return true;
						}
					}
					bec = 0;
				}
				line++;
				col--;
			}
		}

		return false;
	}

	/* diagonala din stanga jos (raportat la rege!) */
	bool check_leftDown(Color color) {

		if (color == WHITE) {

			int kingLine = pos_white[KING].line;
			int kingColumn = pos_white[KING].col;

			/* if bishops or the queen check the king */
			int line = kingLine - 1;
			int col = kingColumn - 1;
			int bec = 1;

			while (line > 0 && col > 0 && bec) {
				if (chessboard[line][col].type != EMPTY) {
					if (chessboard[line][col].color == BLACK) {
						if (chessboard[line][col].type == B1 || chessboard[line][col].type == B2 || chessboard[line][col].type == Q) {
							return true;
						}
					}
					bec = 0;
				}
				line--;
				col--;
			}
		}

		if (color == BLACK) {

			int kingLine = pos_black[KING].line;
			int kingColumn = pos_black[KING].col;

			/* if a bishop or a queen checks the king */
			int line = kingLine + 1;
			int col = kingColumn + 1;
			int bec = 1;

			while (line <= ROWS && col <= COLUMNS && bec) {
				if (chessboard[line][col].type != EMPTY) {
					if (chessboard[line][col].color == WHITE) {
						if (chessboard[line][col].type == B1 || chessboard[line][col].type == B2 || chessboard[line][col].type == Q) {
							return true;
						}
					}
					bec = 0;
				}
				line++;
				col++;
			}
		}

		return false;
	}

	/* functie care verifica daca regele e atacat de calul oponentului */
	bool check_horseAttack(Color color) {
		int kingLine, kingColumn;
		Color opponentColor;
		
		if (color == WHITE) {
			kingLine = pos_white[KING].line;
			kingColumn = pos_white[KING].col;
			opponentColor = BLACK;
		} else {
			kingLine = pos_black[KING].line;
			kingColumn = pos_black[KING].col;
			opponentColor = WHITE;
		}

		if (kingLine + 2 <= ROWS && kingColumn + 1 <= COLUMNS) {
			if ((chessboard[kingLine + 2][kingColumn + 1].type == K1 || 
				chessboard[kingLine + 2][kingColumn + 1].type == K2) && 
				chessboard[kingLine + 2][kingColumn + 1].color == opponentColor) {
				return true;
			} 
		}

		if (kingLine + 2 <= ROWS && kingColumn - 1 >= 1) {
			if ((chessboard[kingLine + 2][kingColumn - 1].type == K1 || 
				chessboard[kingLine + 2][kingColumn - 1].type == K2) && 
				chessboard[kingLine + 2][kingColumn - 1].color == opponentColor) {
				return true;
			} 
		}

		if (kingLine + 1 <= ROWS && kingColumn + 2 <= COLUMNS) {
			if ((chessboard[kingLine + 1][kingColumn + 2].type == K1 || 
				chessboard[kingLine + 1][kingColumn + 2].type == K2) && 
				chessboard[kingLine + 1][kingColumn + 2].color == opponentColor) {
				return true;
			} 
		}

		if (kingLine + 1 <= ROWS && kingColumn - 2 >= 1) {
			if ((chessboard[kingLine + 1][kingColumn - 2].type == K1 || 
				chessboard[kingLine + 1][kingColumn - 2].type == K2) && 
				chessboard[kingLine + 1][kingColumn - 2].color == opponentColor) {
				return true;
			} 
		}

		if (kingLine - 2  >= 1 && kingColumn + 1 <= COLUMNS) {
			if ((chessboard[kingLine - 2][kingColumn + 1].type == K1 || 
				chessboard[kingLine - 2][kingColumn + 1].type == K2) && 
				chessboard[kingLine - 2][kingColumn + 1].color == opponentColor) {
				return true;
			} 
		}

		if (kingLine - 2 >= 1 && kingColumn - 1 >= 1) {
			if ((chessboard[kingLine - 2][kingColumn - 1].type == K1 || 
				chessboard[kingLine - 2][kingColumn - 1].type == K2) && 
				chessboard[kingLine - 2][kingColumn - 1].color == opponentColor) {
				return true;
			} 
		}

		if (kingLine - 1 >= 1 && kingColumn - 2 >= 1) {
			if ((chessboard[kingLine - 1][kingColumn - 2].type == K1 || 
				chessboard[kingLine - 1][kingColumn - 2].type == K2) && 
				chessboard[kingLine - 1][kingColumn - 2].color == opponentColor) {
				return true;
			} 
		}

		if (kingLine - 1 >= 1 && kingColumn + 2 <= COLUMNS) {
			if ((chessboard[kingLine - 1][kingColumn + 2].type == K1 || 
				chessboard[kingLine - 1][kingColumn + 2].type == K2) && 
				chessboard[kingLine - 1][kingColumn + 2].color == opponentColor) {
				return true;
			} 
		}
		return false;
	}

	/* functie care verifica daca regele e atacat de regele oponentului */
	bool check_kingAttack(Color color) {
		int kingLine, kingColumn;
		Color opponentColor;
		
		if (color == WHITE) {
			kingLine = pos_white[KING].line;
			kingColumn = pos_white[KING].col;
			opponentColor = BLACK;
		} else {
			kingLine = pos_black[KING].line;
			kingColumn = pos_black[KING].col;
			opponentColor = WHITE;
		}

		if (kingLine + 1 <= ROWS) {
			if (chessboard[kingLine + 1][kingColumn].type == KING && 
				chessboard[kingLine + 1][kingColumn].color == opponentColor) {
				return true;
			}
		}

		if (kingLine - 1 >= 1) {
			if (chessboard[kingLine - 1][kingColumn].type == KING && 
				chessboard[kingLine - 1][kingColumn].color == opponentColor) {
				return true;
			}
		}

		if (kingColumn + 1 <= COLUMNS) {
			if (chessboard[kingLine][kingColumn + 1].type == KING && 
				chessboard[kingLine][kingColumn + 1].color == opponentColor) {
				return true;
			}
		}

		if (kingColumn - 1 >= 1) {
			if (chessboard[kingLine][kingColumn - 1].type == KING && 
				chessboard[kingLine][kingColumn - 1].color == opponentColor) {
				return true;
			}
		}

		if (kingLine + 1 <= ROWS && kingColumn + 1 <= COLUMNS) {
			if (chessboard[kingLine + 1][kingColumn + 1].type == KING && 
				chessboard[kingLine + 1][kingColumn + 1].color == opponentColor) {
				return true;
			}
		}

		if (kingLine - 1 >= 1 && kingColumn + 1 <= COLUMNS) {
			if (chessboard[kingLine - 1][kingColumn + 1].type == KING && 
				chessboard[kingLine - 1][kingColumn + 1].color == opponentColor) {
				return true;
			}
		}

		if (kingLine + 1 <= ROWS && kingColumn - 1 >= 0) {
			if (chessboard[kingLine + 1][kingColumn - 1].type == KING && 
				chessboard[kingLine + 1][kingColumn - 1].color == opponentColor) {
				return true;
			}
		}

		if (kingLine - 1 >= 0 && kingColumn - 1 >= 0) {
			if (chessboard[kingLine - 1][kingColumn - 1].type == KING && 
				chessboard[kingLine - 1][kingColumn - 1].color == opponentColor) {
				return true;
			}
		}
		return false;
	}
	
	/* functie care verifica daca regele este in sah */
	bool check(Color color) {
		int kingLine, kingColumn;

		if (color == WHITE) {

			kingLine = pos_white[KING].line;
			kingColumn = pos_white[KING].col;
		} else {
			kingLine = pos_black[KING].line;
			kingColumn = pos_black[KING].col;
		}

		if (check_line(color)) {
			return true;
		}

		if (check_column(color)) {
			return true;
		}

		if (check_rightUp(color)) {
			return true;
		}

		if (check_leftUp(color)) {
			return true;
		}

		if (check_rightDown(color)) {
			return true;
		}

		if (check_leftDown(color)) {
			return true;
		}

		if (check_horseAttack(color)) {
			return true;
		}

		if (check_kingAttack(color)) {
			return true;
		}

 		return false;
	}


/* Saherezada's move which is going to be sent to the xboard as a string */
	string send_move_to_xboard(Move m) {
		string send = "move ";
		
		send.push_back(m.column + 'a' - 1);
		send.push_back(m.line + '0');
		send.push_back(m.next_column + 'a' - 1);
		send.push_back(m.next_line + '0');
		
		/* check castling */
		if (makeCastling(m, color) == true) {
			return send;
		} 

		if (chessboard[m.line][m.column].type == KING) {
			if (color == WHITE) {
				white_king_moved = true;
			} else {
				black_king_moved = true;
			}
		}

		if (chessboard[m.line][m.column].type == R1) {
			if (color == WHITE) {
				white_rook1_moved = true;
			} else {
				black_rook1_moved = true;
			}
		}

		if (chessboard[m.line][m.column].type == R2) {
			if (color == WHITE) {
				white_rook2_moved = true;
			} else {
				black_rook2_moved = true;
			}
		}

		/* pawn promotion */
		int piece_id = chessboard[m.line][m.column].type;
		bool is_pawn_promotion = false;

		if (piece_id >= 1 && piece_id <= 8 && (m.next_line == 1 || m.next_line == 8)) {
			if (color == BLACK) {
				queens_black[piece_id].line = m.next_line;
				queens_black[piece_id].col = m.next_column;
				pos_black[piece_id].line = -1 * pos_black[piece_id].line;
				pos_black[piece_id].col = -1 * pos_black[piece_id].col;
			} else {
				queens_white[piece_id].line = m.next_line;
				queens_white[piece_id].col = m.next_column;
				pos_white[piece_id].line = -1 * pos_white[piece_id].line;
				pos_white[piece_id].col = -1 * pos_white[piece_id].col;
			}

			is_pawn_promotion = true;
		} else {
			/* update the position of the piece that is moved */
			piece_id = 0;
			if (color == WHITE) {
				if (chessboard[m.line][m.column].type == Q) {
					piece_id = search_queen_index(m.line, m.column, queens_white);
				}

				if (piece_id == 0) {
					/* nu e o regina rezultata din pawn promotion */
					piece_id = chessboard[m.line][m.column].type;
					pos_white[piece_id].line = m.next_line;
					pos_white[piece_id].col = m.next_column;
				} else {
					queens_white[piece_id].line = m.next_line;
					queens_white[piece_id].col = m.next_column;
				}
			} else {
				if (chessboard[m.line][m.column].type == Q) {
					piece_id = search_queen_index(m.line, m.column, queens_black);
				}

				if (piece_id == 0) {
					/* nu e o regina rezultata din pawn promotion */
					piece_id = chessboard[m.line][m.column].type;
					pos_black[piece_id].line = m.next_line;
					pos_black[piece_id].col = m.next_column;
				} else {
					queens_black[piece_id].line = m.next_line;
					queens_black[piece_id].col = m.next_column;
				}
			}
		}

		/* if another piece is eaten, update that piece's position */
		piece_id = chessboard[m.next_line][m.next_column].type;
		if (piece_id != EMPTY) {
			if (color == WHITE) {
				int q_index = 0;
				if (chessboard[m.next_line][m.next_column].type == Q) {
					q_index = search_queen_index(m.next_line, m.next_column, queens_black);
				}

				if (q_index == 0) {
					pos_black[piece_id].line = -1 * m.next_line;
					pos_black[piece_id].col = -1 * m.next_column;
				} else {
					queens_black[q_index].line = -1 * m.next_line;
					queens_black[q_index].col = -1 * m.next_column;
				}
			} else {
				int q_index = 0;
				if (chessboard[m.next_line][m.next_column].type == Q) {
					q_index = search_queen_index(m.next_line, m.next_column, queens_white);
				}

				if (q_index == 0) {
					pos_white[piece_id].line = -1 * m.next_line;
					pos_white[piece_id].col = -1 * m.next_column;
				} else {
					queens_white[q_index].line = -1 * m.next_line;
					queens_white[q_index].line = -1 * m.next_column;
				}
			}
		}

		/* update the chessboard */
		if (is_pawn_promotion == true) {
			chessboard[m.next_line][m.next_column] = Piece(Q, color);
			chessboard[m.line][m.column] = Piece();
		} else {
			chessboard[m.next_line][m.next_column] = chessboard[m.line][m.column];
			chessboard[m.line][m.column] = Piece();
		}

		return send;
	}



	/* print the pieces on the board - for debugging */
	int nrCifre(int x) {
		int nr = 0;
		while (x != 0) {
			nr++;
			x/=10;
		}
		return nr;
	}

	/* print the name of a piece based on its index - for debugging */
	void pieceName(int i) {
		if (i == 1) cout << "PION 1: " << " ";
		if (i == 2) cout << "PION 2: " << " ";
		if (i == 3) cout << "PION 3: " << " ";
		if (i == 4) cout << "PION 4: " << " ";
		if (i == 5) cout << "PION 5: " << " ";
		if (i == 6) cout << "PION 6: " << " ";
		if (i == 7) cout << "PION 7: " << " ";
		if (i == 8) cout << "PION 8: " << " ";
		if (i == 9)	cout << "TURA 1: " << " "; 
		if (i == 10) cout <<"TURA 2: " << " ";
		if (i == 11) cout <<"CAL 1:  " << " ";
		if (i == 12) cout <<"CAL 2:  " << " ";
		if (i == 13) cout <<"NEBUN1: " << " ";
		if (i == 14) cout <<"NEBUN2: " << " ";
		if (i == 15) cout <<"REGINA: " << " "; 
		if (i == 16) cout <<"REGE:   " << " ";
	}

	/* print the chessboard - for debugging */
	void printChessBoard() {
		cout << "TABLA DE JOC " << endl;
		for (int i = 8; i >= 1; i--) {
			for (int j = 1; j <= 8; j++) {
				if (chessboard[i][j].type == EMPTY) {
					cout << 0 << "   ";
					continue;
				}

				if (chessboard[i][j].color == WHITE) {
					cout << chessboard[i][j].type << "  ";
					if (nrCifre(chessboard[i][j].type) == 1) cout << " ";
				} else {
					cout << -1 * chessboard[i][j].type << " ";
					if (nrCifre(chessboard[i][j].type) == 1) cout << " ";
				}
			}
			cout << endl;
		}

		cout << endl;
		cout << "Pozitiile pieselor negre: " << endl;
		for (int i = 1; i <= 16; i++) {
			pieceName(i);
			cout << "linia " << pos_black[i].line << ", coloana " << pos_black[i].col << endl;
		}
		cout << endl;

		cout << "Pozitiile pieselor albe " << endl;
		for (int i = 1; i <= 16; i++) {
			pieceName(i);
			cout << "linia " << pos_white[i].line << ", coloana " << pos_white[i].col << endl;
		}
	}

	/* new table configuration after color switch*/
	void flipChessBoard(){
		Piece aux = Piece();

		for(int i = 1; i <= 4; i++){
			for(int j = 1; j <= 8; j++){
				aux = chessboard[i][j];
				chessboard[i][j] = chessboard[8-i+1][8-j+1];	
				chessboard[8-i+1][8-j+1] = aux;
			}
		}
	}

	bool gameOver() {
		if (color == BLACK) {
			if (pos_black[16].line < 0) {
				return true;
			}
		} else {
			if (pos_white[16].line < 0) {
				return true;
			}
		}

		return false;
	}

};

Color color_of_the_current_player(ChessGame& game, Player p) {
	if (game.color == WHITE && p == SAHEREZADE) {
		return WHITE;
	}

	if (game.color == WHITE && p == OPPONENT) {
		return BLACK;
	}

	if (game.color == BLACK && p == SAHEREZADE) {
		return BLACK;
	}

	if (game.color == BLACK && p == OPPONENT) {
		return WHITE;
	}
}

Piece apply_move(ChessGame& game, Move& move, Player& player) {
	/* update pos_black or pos_white */
	int piece_id = game.chessboard[move.line][move.column].type;
	Piece yummy = Piece();
	Color current_player_color = color_of_the_current_player(game, player);
	Color opponent_color = (current_player_color == WHITE) ? BLACK : WHITE;

	/* helpful for castling; mark the movement of a king or rook */
	if (game.makeCastling(move, current_player_color) == true) {
		return yummy;
	}

	if (game.chessboard[move.line][move.column].type == KING) {
		if (current_player_color == WHITE) {
			game.white_king_moved = true;
		} else {
			game.black_king_moved = true;
		}
	}

	if (game.chessboard[move.line][move.column].type == R1) {
		if (current_player_color == WHITE) {
			game.white_rook1_moved = true;
		} else {
			game.black_rook1_moved = true;
		}
	}

	if (game.chessboard[move.line][move.column].type == R2) {
		if (current_player_color == WHITE) {
			game.white_rook2_moved = true;
		} else {
			game.black_rook2_moved = true;
		}
	}

	/* check for pawn promotion */
	bool is_pawn_promotion = false;
	if (piece_id >= 1 && piece_id <= 8 && (move.next_line == 1 || move.next_line == 8)) {
		if (current_player_color == BLACK) {
			game.pos_black[piece_id].line = -1 * game.pos_black[piece_id].line;
			game.pos_black[piece_id].col = -1 * game.pos_black[piece_id].line;

			game.queens_black[piece_id].line = move.next_line;
			game.queens_black[piece_id].col = move.next_column;
		} else {
			game.pos_white[piece_id].line = -1 * game.pos_white[piece_id].line;
			game.pos_white[piece_id].col = -1 * game.pos_white[piece_id].line;

			game.queens_white[piece_id].line = move.next_line;
			game.queens_white[piece_id].col = move.next_column;
		}

		is_pawn_promotion = true;
	} else {
		/* update the position of the piece that is moved */
		piece_id = 0;
		if (current_player_color == WHITE) {
			if (game.chessboard[move.line][move.column].type == Q) {
				piece_id = game.search_queen_index(move.line, move.column, game.queens_white);
			}

			if (piece_id == 0) {
				/* nu e o regina rezultata din pawn promotion */
				piece_id = game.chessboard[move.line][move.column].type;
				game.pos_white[piece_id].line = move.next_line;
				game.pos_white[piece_id].col = move.next_column;
			} else {
				game.queens_white[piece_id].line = move.next_line;
				game.queens_white[piece_id].col = move.next_column;
			}
		} else {
			if (game.chessboard[move.line][move.column].type == Q) {
				piece_id = game.search_queen_index(move.line, move.column, game.queens_black);
			}

			if (piece_id == 0) {
				/* nu e o regina rezultata din pawn promotion */
				piece_id = game.chessboard[move.line][move.column].type;
				game.pos_black[piece_id].line = move.next_line;
				game.pos_black[piece_id].col = move.next_column;
			} else {
				game.queens_black[piece_id].line = move.next_line;
				game.queens_black[piece_id].col = move.next_column;
			}
		}
	}

	/* the move results in taking out the opponent's piece */
	if (game.chessboard[move.next_line][move.next_column].type != EMPTY) {
		piece_id = game.chessboard[move.next_line][move.next_column].type;
		yummy = game.chessboard[move.next_line][move.next_column];

		if (opponent_color == BLACK) {
			int q_index = 0;
			if (yummy.type == Q) {
				q_index = game.search_queen_index(move.next_line, move.next_column, game.queens_black);
			}

			if (q_index == 0) {
				game.pos_black[piece_id].line = -1 * game.pos_black[piece_id].line;
				game.pos_black[piece_id].col = -1 * game.pos_black[piece_id].col;
			} else {
				game.queens_black[q_index].line = -1 * game.queens_black[q_index].line;
				game.queens_black[q_index].col = -1 * game.queens_black[q_index].line;
			}
		} else {
			int q_index = 0;
			if (yummy.type == Q) {
				q_index = game.search_queen_index(move.next_line, move.next_column, game.queens_white);
			}

			if (q_index == 0) {
				game.pos_white[piece_id].line = -1 * game.pos_white[piece_id].line;
				game.pos_white[piece_id].col = -1 * game.pos_white[piece_id].col;
			} else {
				game.queens_white[q_index].line = -1 * game.queens_white[q_index].line;
				game.queens_white[q_index].col = -1 * game.queens_white[q_index].line;
			}
		}
	}

	/* update the chessboard */
	if (is_pawn_promotion == true) {
		game.chessboard[move.next_line][move.next_column] = Piece(Q, current_player_color);
		game.chessboard[move.line][move.column] = Piece();
	} else {
		game.chessboard[move.next_line][move.next_column] = game.chessboard[move.line][move.column];
		game.chessboard[move.line][move.column] = Piece();
	}

	return yummy;
}

void undo_move(ChessGame& game, Move& move, Piece &yummy, Player& player) {
	int new_next_line = move.line;
	int new_current_line = move.next_line;
	int new_next_column = move.column;
	int new_current_column = move.next_column;

	//Color c = game.chessboard[new_current_line][new_current_column].color;
	Color c = color_of_the_current_player(game, player);

	/* castling: 4 possible cases */
	if (game.unmakeCastling(move, c) == true) {
		return;
	}


	int piece_id = game.chessboard[new_current_line][new_current_column].type;

	if (game.chessboard[new_current_line][new_current_column].color == BLACK) {
		game.pos_black[piece_id].line = new_next_line;
		game.pos_black[piece_id].col = new_next_column;
	} else {
		game.pos_white[piece_id].line = new_next_line;
		game.pos_white[piece_id].col = new_next_column;
	}

	game.chessboard[new_next_line][new_next_column] = game.chessboard[new_current_line][new_current_column];

	if (yummy.type == EMPTY) {
		game.chessboard[new_current_line][new_current_column] = Piece();
	} else {
		game.chessboard[new_current_line][new_current_column] = yummy;

		if(yummy.color == BLACK) {
			game.pos_black[yummy.type].line = new_current_line;
			game.pos_black[yummy.type].col = new_current_column;
		} else {
			game.pos_white[yummy.type].line = new_current_line;
			game.pos_white[yummy.type].col = new_current_column;
		}
	}
}

Player other_player(Player player) {
	if (player == SAHEREZADE) {
		return OPPONENT;
	} 
	
	return SAHEREZADE;
}

int count_pieces(ChessGame& game, Color player_color) {
	int n_pieces = 0;
	for (int i = 1; i <= 8; i++) {
		for (int j = 1; j <= 8; j++) {
			if (game.chessboard[i][j].color == player_color) {
				if (game.chessboard[i][j].type > P8 && game.chessboard[i][j].type != KING) {
					n_pieces++;
				}
			}
		}
	}

	return n_pieces;
}

Move evaluate(ChessGame& game, Player player, Move move) {
	Move m;
	Color player_color, opponent_color;

	m.line = move.line;
	m.column = move.column;
	m.next_line = move.next_line;
	m.next_column = move.next_column;

	if (player == SAHEREZADE) {
		/* it's SAHEREZADA's turn */
		player_color = game.color;
		if (game.color == BLACK) {
			opponent_color = WHITE;
		} else {
			opponent_color = BLACK;
		}
	} else {
		/* it's the opponent's turn */
		opponent_color = game.color;
		if (game.color == BLACK) {
			player_color = WHITE;
		} else {
			player_color = BLACK;
		}
	}

	m.score = 0;

	for (int i = 1; i <= ROWS; i++) {
		for (int j = 1; j <= COLUMNS; j++) {
			/* check if it's a pawn */
			if (game.chessboard[i][j].type >= P1 && game.chessboard[i][j].type <= P8) {
				if (game.chessboard[i][j].color == player_color) {
					if (player_color == BLACK) {
						m.score += PAWN_SCORE + black_pawn_bonus[i][j];
					} else {
						m.score += PAWN_SCORE + white_pawn_bonus[i][j];
					}
				} else {
					if (opponent_color == BLACK) {
						m.score += -1 * (PAWN_SCORE + black_pawn_bonus[i][j]);
					} else {
						m.score += -1 * (PAWN_SCORE + white_pawn_bonus[i][j]);
					}
				}
			}

			/* check if it's a rook */
			if (game.chessboard[i][j].type == R1 || game.chessboard[i][j].type == R2) {
				if (game.chessboard[i][j].color == player_color) {
					if (player_color == BLACK) {
						m.score += ROOK_SCORE + black_rook_bonus[i][j];
					} else {
						m.score += ROOK_SCORE + white_rook_bonus[i][j];
					}
				} else {
					if (opponent_color == BLACK) {
						m.score += -1 * (ROOK_SCORE + black_rook_bonus[i][j]);
					} else {
						m.score += -1 * (ROOK_SCORE + white_rook_bonus[i][j]);
					}
				}
			}

			/* check if it's a knight */
			if (game.chessboard[i][j].type == K1 || game.chessboard[i][j].type == K2) {
				if (game.chessboard[i][j].color == player_color) {
					if (player_color == BLACK) {
						m.score += KNIGHT_SCORE + black_knight_bonus[i][j];
					} else {
						m.score += KNIGHT_SCORE + white_knight_bonus[i][j];
					}
				} else {
					if (opponent_color == BLACK) {
						m.score += -1 * (KNIGHT_SCORE + black_knight_bonus[i][j]);
					} else {
						m.score += -1 * (KNIGHT_SCORE + white_knight_bonus[i][j]);
					}
				}
			}

			/* check if it's a bishop */
			if (game.chessboard[i][j].type == B1 || game.chessboard[i][j].type == B2) {
				if (game.chessboard[i][j].color == player_color) {
					if (player_color == BLACK) {
						m.score += BISHOP_SCORE + black_bishop_bonus[i][j];
					} else {
						m.score += BISHOP_SCORE + white_bishop_bonus[i][j];
					}
				} else {
					if (opponent_color == BLACK) {
						m.score += -1 * (BISHOP_SCORE + black_bishop_bonus[i][j]);
					} else {
						m.score += -1 * (BISHOP_SCORE + white_bishop_bonus[i][j]);
					}
				}
			}

			/* check if it's a queen */
			if (game.chessboard[i][j].type == Q) {
				if (game.chessboard[i][j].color == player_color) {
					if (player_color == BLACK) {
						m.score += QUEEN_SCORE + black_queen_bonus[i][j];
					} else {
						m.score += QUEEN_SCORE + white_queen_bonus[i][j];
					}
				} else {
					if (opponent_color == BLACK) {
						m.score += -1 * (QUEEN_SCORE + black_queen_bonus[i][j]);
					} else {
						m.score += -1 * (QUEEN_SCORE + white_queen_bonus[i][j]);
					}
				}
			}

			/* check if it's a king */
			if (game.chessboard[i][j].type == KING) {
				if (game.chessboard[i][j].color == player_color) {
					/* check the color */
					if (player_color == BLACK) {
						/* check if it's the endgame */
						if (count_pieces(game, player_color) > 4) {
							m.score += KING_SCORE + black_king_middle_bonus[i][j];
						} else {
							m.score += KING_SCORE + black_king_endgame_bonus[i][j];
						}
					} else {
						if (count_pieces(game, player_color) > 4) {
							m.score += KING_SCORE + white_king_middle_bonus[i][j];
						} else {
							m.score += KING_SCORE + white_king_endgame_bonus[i][j];
						}
					}
				} else {
					/* check the color */
					if (opponent_color == BLACK) {
						/* check if it's the endgame */
						if (count_pieces(game, opponent_color) > 4) {
							m.score += -1 * (KING_SCORE + black_king_middle_bonus[i][j]);
						} else {
							m.score += -1 * (KING_SCORE + black_king_endgame_bonus[i][j]);
						}
					} else {
						if (count_pieces(game, opponent_color) > 4) {
							m.score += -1 * (KING_SCORE + white_king_middle_bonus[i][j]);
						} else {
							m.score += -1 * (KING_SCORE + white_king_endgame_bonus[i][j]);
						}
					}
				}
			}
		}
	}

	return m;
}

Move negamax(ChessGame& game, Player player, int depth, Move move) {
	if (depth == 0 || game.check(color_of_the_current_player(game, player)) == true) {
		return evaluate(game, player, move);
	}

	vector<Move> moves = game.generateAllMoves(player); // all possible moves

	Move best_move, new_move;
	Piece yummy;
	best_move.score = SCORE_LOSER - 1;

	if (moves.size() == 0) {
		best_move = move;
		best_move.score = SCORE_LOSER;
		return best_move;
	}

	for (Move move : moves) {
		ChessGame copy = game;
		yummy = apply_move(copy, move, player);

		if (copy.check(color_of_the_current_player(game, player)) == true) {
			continue;
		}

		new_move = negamax(copy, other_player(player), depth - 1, move);
		new_move.score *= -1;

		if (new_move.score > best_move.score) {
			best_move = move;
			best_move.score = new_move.score;
		}
	}

	return best_move;
}

bool valid_move_from_opponent(string s) {
	if (s.size() != 4) {
		return false;
	}

	if (s[0] < 'a' || s[0] > 'h') {
		return false;
	}

	if (s[1] < '1' || s[1] > '8') {
		return false;
	}

	if (s[2] < 'a' || s[2] > 'h') {
		return false;
	}

	if (s[3] < '1' || s[3] > '8') {
		return false;
	}

	return true;
}

int main() {
	ChessGame cg = ChessGame();
	string s;
	int aux;

	cout << "feature sigint=0" << '\n';
    cout << "feature sigterm=0 san=0" << '\n';
    cout << "feature usermove=0" << '\n';

    
	while (1) {
		s.clear();
		getline(cin, s);

		/*check for xboard*/
		if (s.find("xboard") == 0) {
			continue;
		}

		/* check for quit */

		if (s.find("quit") == 0) {
			return 0;
		}

		/* check for new */

		if (s.find("new") == 0) {
			cg.color = BLACK;
			cg.player = OPPONENT;
			cg.force = INACTIVE;
			cg.ColorSwitch = false;
			cg.initChessBoard();
			continue;
		}

		/*check for color switch*/
		
		if (s.find("white") == 0) {
			if(cg.ColorSwitch == false){
				cg.ColorSwitch = true;
				continue;
			}

			cg.color = WHITE;
			cg.player = SAHEREZADE;
			//cg.flipChessBoard();
			cg.ColorSwitch = false;
			continue;
		}

		if (s.find("black") == 0) {
			if(cg.ColorSwitch == false){
				cg.ColorSwitch = true;
				continue;
			}

			cg.color = BLACK;
			cg.player = SAHEREZADE;
			//cg.flipChessBoard();
			cg.ColorSwitch = false;
			continue;
		}

		/*check for force mode*/

		if (s.find("force") == 0) {
			cg.force = ACTIVE;
			continue;
		}

		/*check for exit from force mode*/

		if (s.find("go") == 0) {
			if(cg.force == ACTIVE){
				cg.force = INACTIVE;
			}
			cg.player = SAHEREZADE;
		}

		if (cg.player == OPPONENT) {
			/* if the move is valid, update on the board the move from opponent */
			if (valid_move_from_opponent(s) == true) {
				Move m = cg.opponent_move_from_xboard(s);
				
				/* change the turn */
				cg.player = SAHEREZADE;

				/* print the chessboard */
				cg.printChessBoard();
			} 		
		}


		if (cg.player == SAHEREZADE) {
			if(cg.force == INACTIVE) {
				/* choose the best move */
				Move m = negamax(cg, SAHEREZADE, 4, Move());

				/* send the move to the xboard */
				cout << cg.send_move_to_xboard(m) << endl;

				/* change the turn */
				cg.player = OPPONENT;
				cg.printChessBoard();

			} else {
				if (valid_move_from_opponent(s) == true) {			
					/* engine is in force mode so he receives moves from xboard */
					Move m = cg.opponent_move_from_xboard(s);
			
					cg.player = OPPONENT;
					cg.printChessBoard();
				}
			}
		}

	}

	return 0;
}
