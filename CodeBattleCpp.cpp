#include <iostream>
#include <random>
#include <unordered_set>
#include "details/BeastGameClient.h"
#include "GameBoard.h"

const int TEST_COUNT = 100;
const int TEST_PATH_LENGHT = 20;

inline bool deadly(const GameBoard& board, BoardPoint pt) {
	if (board.playerIsShadow())
		return false;
	return board.hasEnemyAt(pt)
		|| board.hasShadowEnemyAt(pt);
}
inline bool goodies(const GameBoard& board, BoardPoint pt) {
	return board.hasGoldAt(pt)
		|| board.getElementAt(pt) == BoardElement::THE_SHADOW_PILL
		|| (board.playerIsShadow() && (board.hasOtherHeroAt(pt)
			|| board.hasShadowEnemyAt(pt)
			|| board.hasEnemyAt(pt)));
}

void printPath(std::vector<std::vector<bool>> table, const GameBoard& board) {
	for (int i = 0; i < board.getBoardSize(); i++) {
		for (int j = 0; j < board.getBoardSize(); j++) {
			if (table[i][j]) {
				std::cout << 'X';
			}
			else std::cout << std::to_string(board.getElementAt({ i,j }));
		}
		std::cout << '\n';
	}
}

bool walkable(const GameBoard& board, BoardPoint pt) {
	if (board.getElementAt(pt.shiftBottom()) == BoardElement::BRICK ||
		board.getElementAt(pt.shiftBottom()) == BoardElement::INDESTRUCTIBLE_WALL ||
		board.getElementAt(pt.shiftBottom()) == BoardElement::LADDER ||
		board.getElementAt(pt.shiftBottom()) == BoardElement::HERO_LADDER ||
		board.getElementAt(pt.shiftBottom()) == BoardElement::HERO_SHADOW_LADDER ||
		board.getElementAt(pt.shiftBottom()) == BoardElement::OTHER_HERO_LEFT ||
		board.getElementAt(pt.shiftBottom()) == BoardElement::OTHER_HERO_RIGHT ||
		board.getElementAt(pt) == BoardElement::HERO_LADDER ||
		board.getElementAt(pt) == BoardElement::HERO_SHADOW_LADDER ||
		board.getElementAt(pt) == BoardElement::LADDER ||
		board.getElementAt(pt) == BoardElement::PIPE ||
		board.getElementAt(pt) == BoardElement::HERO_PIPE_LEFT ||
		board.getElementAt(pt) == BoardElement::HERO_PIPE_RIGHT ||
		board.getElementAt(pt) == BoardElement::HERO_SHADOW_PIPE_RIGHT ||
		board.getElementAt(pt) == BoardElement::HERO_SHADOW_PIPE_LEFT ||
		board.getElementAt(pt) == BoardElement::PORTAL)
		return true;
	return false;
}

class pathPair {
public:
	pathPair(BoardPoint* _pt, LodeRunnerAction move_) :pt(*_pt), move(move_) {}
	pathPair(pathPair* pair) : pt((pair->pt)), move(pair->getMoveFromPath()) {}
	BoardPoint pt;

	LodeRunnerAction getMoveFromPath() const {
		return move;
	}
	void setMove(LodeRunnerAction move_) {
		move = move_;
	}
private:

	LodeRunnerAction move;
};

void drillLeftAndGo(const GameBoard& board, BoardPoint& currPt, std::vector<std::vector<bool>>& visited,
	LodeRunnerAction currMove, std::list<pathPair>& queue)
{
	if (board.getElementAt(currPt.shiftLeft().shiftBottom()) == BoardElement::BRICK &&
		!board.isNearToEnemy(board.getMyPosition()) &&
		walkable(board, currPt) &&
		walkable(board, currPt.shiftLeft()) &&
		board.hasElementAt(currPt.shiftLeft(), BoardElement::NONE)) {
		if (!visited[currPt.shiftLeft().shiftBottom().getY()][currPt.shiftLeft().shiftBottom().getX()]) {
			visited[currPt.shiftLeft().shiftBottom().getY()][currPt.shiftLeft().shiftBottom().getX()] = true;
			if (currMove == LodeRunnerAction::IDLE)
				queue.push_back({ &(currPt.shiftLeft().shiftBottom()), LodeRunnerAction::DRILL_LEFT });
			else
				queue.push_back({ &(currPt.shiftLeft().shiftBottom()), currMove });
		}
	}
}

void drillRightAndGo(const GameBoard& board, BoardPoint& currPt, std::vector<std::vector<bool>>& visited,
	LodeRunnerAction currMove, std::list<pathPair>& queue)
{
	if (board.getElementAt(currPt.shiftRight().shiftBottom()) == BoardElement::BRICK &&
		!board.isNearToEnemy(board.getMyPosition()) &&
		walkable(board, currPt) &&
		walkable(board, currPt.shiftRight()) &&
		board.hasElementAt(currPt.shiftRight(), BoardElement::NONE)) {
		if (!visited[currPt.shiftRight().shiftBottom().getY()][currPt.shiftRight().shiftBottom().getX()]) {
			visited[currPt.shiftRight().shiftBottom().getY()][currPt.shiftRight().shiftBottom().getX()] = true;
			if (currMove == LodeRunnerAction::IDLE)
				queue.push_back({ &(currPt.shiftRight().shiftBottom()), LodeRunnerAction::DRILL_RIGHT });
			else
				queue.push_back({ &(currPt.shiftRight().shiftBottom()), currMove });
		}
	}
}

void moveDown(const GameBoard& board, BoardPoint& currPt,
	std::vector<std::vector<bool>>& visited, LodeRunnerAction currMove,
	std::list<pathPair>& queue)
{
	if (board.getElementAt(currPt.shiftBottom()) == BoardElement::NONE ||
		board.hasElementAt(currPt.shiftBottom(), BoardElement::LADDER) ||
		board.hasElementAt(currPt.shiftBottom(), BoardElement::PIPE) ||
		board.hasElementAt(currPt.shiftBottom(), BoardElement::DRILL_PIT) ||
		goodies(board, currPt.shiftBottom())) {
		if (!visited[currPt.shiftBottom().getY()][currPt.shiftBottom().getX()]) {
			visited[currPt.shiftBottom().getY()][currPt.shiftBottom().getX()] = true;
			if (currMove == LodeRunnerAction::IDLE)
				queue.push_back({ &(currPt.shiftBottom()), LodeRunnerAction::GO_DOWN });
			else
				queue.push_back({ &(currPt.shiftBottom()), currMove });
		}
	}
}

void moveUp(const GameBoard& board, BoardPoint& currPt, std::vector<std::vector<bool>>& visited,
	LodeRunnerAction currMove, std::list<pathPair>& queue)
{
	if ((board.getElementAt(currPt) == BoardElement::HERO_LADDER ||
		board.getElementAt(currPt) == BoardElement::LADDER ||
		board.getElementAt(currPt) == BoardElement::HERO_SHADOW_LADDER)
		&&
		(board.getElementAt(currPt.shiftTop()) == BoardElement::NONE ||
			board.getElementAt(currPt.shiftTop()) == BoardElement::LADDER ||
			board.getElementAt(currPt.shiftTop()) == BoardElement::PIPE ||
			goodies(board, currPt.shiftTop()))) {
		if (!visited[currPt.shiftTop().getY()][currPt.shiftTop().getX()]) {
			visited[currPt.shiftTop().getY()][currPt.shiftTop().getX()] = true;
			if (currMove == LodeRunnerAction::IDLE)
				queue.push_back({ &(currPt.shiftTop()), LodeRunnerAction::GO_UP });
			else
				queue.push_back({ &(currPt.shiftTop()), currMove });
		}
	}
}

void moveRight(const GameBoard& board, BoardPoint& currPt, std::vector<std::vector<bool>>& visited,
	LodeRunnerAction currMove, std::list<pathPair>& queue)
{
	if (walkable(board, currPt) &&
		(board.getElementAt(currPt.shiftRight()) == BoardElement::NONE ||
			board.hasElementAt(currPt.shiftRight(), BoardElement::LADDER) ||
			board.hasElementAt(currPt.shiftRight(), BoardElement::PIPE) ||
			goodies(board, currPt.shiftRight()))) {
		if (!visited[currPt.shiftRight().getY()][currPt.shiftRight().getX()]) {
			visited[currPt.shiftRight().getY()][currPt.shiftRight().getX()] = true;
			if (currMove == LodeRunnerAction::IDLE)
				queue.push_back({ &(currPt.shiftRight()), LodeRunnerAction::GO_RIGHT });
			else
				queue.push_back({ &(currPt.shiftRight()), currMove });
		}
	}
}

void moveLeft(const GameBoard& board, BoardPoint& currPt, std::vector<std::vector<bool>>& visited,
	LodeRunnerAction currMove, std::list<pathPair>& queue)
{
	if (walkable(board, currPt) &&
		(board.getElementAt(currPt.shiftLeft()) == BoardElement::NONE ||
			board.hasElementAt(currPt.shiftLeft(), BoardElement::LADDER) ||
			board.hasElementAt(currPt.shiftLeft(), BoardElement::PIPE) ||
			goodies(board, currPt.shiftLeft()))) {
		if (!visited[currPt.shiftLeft().getY()][currPt.shiftLeft().getX()]) {
			visited[currPt.shiftLeft().getY()][currPt.shiftLeft().getX()] = true;
			if (currMove == LodeRunnerAction::IDLE)
				queue.push_back({ &(currPt.shiftLeft()), LodeRunnerAction::GO_LEFT });
			else
				queue.push_back({ &(currPt.shiftLeft()), currMove });

		}
	}
}

//teleports behavior || MAYBE implement monte for earning monke

LodeRunnerAction makeTurn(const GameBoard& board) {
	BoardPoint position = board.getMyPosition();
	std::cout << "Hero position is (" << position.getX() << "," << position.getY() << ")\n";

	std::list <pathPair> queue;

	std::vector <std::vector<bool>> visited(
		board.getBoardSize(),
		std::vector<bool>(board.getBoardSize(), false));

	visited[position.getY()][position.getX()] = true;
	queue.push_back({ &position , LodeRunnerAction::IDLE });
	while (!queue.empty())
	{
		pathPair temp(queue.front());
		BoardPoint currPt = (temp.pt);
		LodeRunnerAction currMove = temp.getMoveFromPath();
		queue.pop_front();

		//if found gold then return move
		if (goodies(board, currPt)) {
			// ќ—“џЋ№ (с пилюлей все противники = goodies чтобы спокойно через них проходить
			// но стремитс€ бегать за ними нет смысла
			if (!board.hasOtherHeroAt(currPt)
				&& !board.hasShadowEnemyAt(currPt)
				&& !board.hasEnemyAt(currPt)) {
				queue.clear();
				return currMove;
			}
		}
		//if died return -1
		if (deadly(board, currPt))
		{
			continue; // unable to continue path = dead
		}


		moveLeft(board, currPt, visited, currMove, queue);

		moveRight(board, currPt, visited, currMove, queue);

		moveUp(board, currPt, visited, currMove, queue);

		moveDown(board, currPt, visited, currMove, queue);

		drillRightAndGo(board, currPt, visited, currMove, queue);

		drillLeftAndGo(board, currPt, visited, currMove, queue);

	}
	return LodeRunnerAction::GO_UP;
}


int main() {
	const std::string serverUrl = "https://dojorena.io/codenjoy-contest/board/player/dojorena427?code=1388849016264401508";

	while (1) {
		details::BeastGameClient gcb(serverUrl);
		gcb.Run(makeTurn);
	}

	return 0;
}
