#include "stdafx.h"

const int WIDTH = 50, HEIGHT = 50;
const int PERTURBATIONS = 10;

struct Cell {
	int from = -1, cost = -1;
	bool isWall = true, isShortestPath = false, processed = false;
};

class Maze {
public:
	const int WIDTH, HEIGHT, SIZE, START;
	std::vector<Cell> cells;
	int goal, shortestPathLength = -1, turns = -1;

	Maze(int width_, int height_, int start_) : WIDTH(width_), HEIGHT(height_), START(start_), SIZE(width_ * height_), cells(width_ * height_) {}
	Maze operator=(Maze& m) {
		assert(WIDTH == m.WIDTH);
		assert(HEIGHT == m.HEIGHT);
		assert(START == m.START);
		cells = m.cells;
		return *this;
	}

	bool canChangeMark(int pos) const {
		if (pos < 0 || pos >= SIZE) {
			return false;
		}

		if (pos % WIDTH > 0 && pos >= WIDTH) {
			if (cells[pos - WIDTH - 1].isWall != cells[pos - 1].isWall && cells[pos - WIDTH - 1].isWall != cells[pos - WIDTH].isWall) {
				return false;
			}
			if (!cells[pos].isWall && cells[pos - 1].isWall && cells[pos - WIDTH].isWall && cells[pos - WIDTH - 1].isWall) {
				return false;
			}
		}
		if (pos >= WIDTH && pos % WIDTH < WIDTH - 1) {
			if (cells[pos - WIDTH + 1].isWall != cells[pos - WIDTH].isWall && cells[pos - WIDTH + 1].isWall != cells[pos + 1].isWall) {
				return false;
			}
			if (!cells[pos].isWall && cells[pos + 1].isWall && cells[pos - WIDTH].isWall && cells[pos - WIDTH + 1].isWall) {
				return false;
			}
		}
		if (pos % WIDTH > 0 && pos + WIDTH < SIZE) {
			if (cells[pos + WIDTH - 1].isWall != cells[pos - 1].isWall && cells[pos + WIDTH - 1].isWall != cells[pos + WIDTH].isWall) {
				return false;
			}
			if (!cells[pos].isWall && cells[pos - 1].isWall && cells[pos + WIDTH].isWall && cells[pos + WIDTH - 1].isWall) {
				return false;
			}
		}
		if (pos % WIDTH < WIDTH - 1 && pos + WIDTH < SIZE) {
			if (cells[pos + WIDTH + 1].isWall != cells[pos + 1].isWall && cells[pos + WIDTH + 1].isWall != cells[pos + WIDTH].isWall) {
				return false;
			}
			if (!cells[pos].isWall && cells[pos + 1].isWall && cells[pos + WIDTH].isWall && cells[pos + WIDTH + 1].isWall) {
				return false;
			}
		}

		return true;
	}

	void perturbate(std::mt19937& engine) {
		std::uniform_int_distribution<int> unif(0, SIZE - 1);
		for (int i = 0; i < SIZE; ++i) {
			cells[i].isShortestPath = false;
			cells[i].processed = false;
			cells[i].from = -1;
			cells[i].cost = i == START ? 0 : -1;
		}
		shortestPathLength = -1;
		turns = -1;
		while (true) {
			int pos = unif(engine);
			if (pos != START && pos != goal && canChangeMark(pos)) {
				cells[pos].isWall = !cells[pos].isWall;
				return;
			}
		}
	}

	void fillDeadSpace() {
		std::vector<bool> visited(SIZE, false);
		std::function<void(int)> fill = [&](int pos) {
			if (visited[pos]) return;
			visited[pos] = true;

			if (pos % WIDTH > 0 && !cells[pos - 1].isWall)         fill(pos - 1);
			if (pos % WIDTH < WIDTH - 1 && !cells[pos + 1].isWall) fill(pos + 1);
			if (pos >= WIDTH && !cells[pos - WIDTH].isWall)        fill(pos - WIDTH);
			if (pos + WIDTH < SIZE && !cells[pos + WIDTH].isWall)  fill(pos + WIDTH);

			return;
		};
		fill(START);

		for (int i = 0; i < SIZE; ++i) {
			if (!visited[i]) {
				cells[i].isWall = true;
			}
		}
	}

	int countBlanks() const {
		int count = 0;
		for (int i = 0; i < SIZE; ++i) {
			if (!cells[i].isWall && !cells[i].isShortestPath) ++count;
		}
		return count;
	}

	bool solve() {
		std::queue<int> queue;
		std::vector<bool> queued(SIZE, false);
		queue.push(START);
		while (!queue.empty()) {
			int pos = queue.front();
			Cell *processedCell = &cells[pos];
			queue.pop();
			queued[pos] = false;
			processedCell->processed = true;

			for (int i = 0; i < 4; ++i) {
				if (edgeExists(pos, i)) {
					int to = directionToPos(pos, i);
					int cost = processedCell->cost + 1;
					if (cells[to].cost < 0 || cost < cells[to].cost) {
						cells[to].from = pos;
						cells[to].cost = cost;
						if (queued[to] == false) {
							queue.push(to);
							queued[to] = true;
						}
					}
				}
			}
		}

		if (cells[goal].from == -1) {
			return false;
		}

		int pos = goal, prevPos = -1;
		shortestPathLength = 0;
		turns = 0;
		while (pos != START) {
			cells[pos].isShortestPath = true;
			++shortestPathLength;
			if (cells[pos].from - pos != pos - prevPos) {
				++turns;
			}

			pos = cells[pos].from;
			prevPos = pos;
		}

		return true;
	}

	void print() {
		for (int y = 0; y < HEIGHT; ++y) {
			for (int x = 0; x < WIDTH; ++x) {
				if (x + y * WIDTH == START) std::cout << "S";
				else if (x + y * WIDTH == goal) std::cout << "G";
				else if (cells[x + y * WIDTH].isShortestPath) std::cout << "2";
				else std::cout << (cells[x + y * WIDTH].isWall ? "1" : " ");
				if (x != WIDTH - 1) std::cout << ",";
			}
			std::cout << std::endl;
		}
	}

private:
	int numNeighborWalls(int pos) const {
		int numWalls = 0;
		if (pos % WIDTH <= 0 || cells[pos - 1].isWall)         ++numWalls;
		if (pos % WIDTH >= WIDTH - 1 || cells[pos + 1].isWall) ++numWalls;
		if (pos < WIDTH || cells[pos - WIDTH].isWall)          ++numWalls;
		if (pos + WIDTH >= SIZE || cells[pos + WIDTH].isWall)  ++numWalls;

		return numWalls;
	}

	bool edgeExists(int from, int direction) const {
		if (cells[from].isWall) return false;
		switch (direction) {
		case 0:
			if (from < WIDTH) return false;
			break;
		case 1:
			if (from % WIDTH <= 0) return false;
			break;
		case 2:
			if (from % WIDTH >= WIDTH - 1) return false;
			break;
		case 3:
			if (from + WIDTH >= SIZE) return false;
			break;
		default:
			return false;
		}
		return !cells[directionToPos(from, direction)].isWall;
	}

	int directionToPos(int from, int direction) const {
		switch (direction) {
		case 0:
			return from - WIDTH;
		case 1:
			return from - 1;
		case 2:
			return from + 1;
		case 3:
			return from + WIDTH;
		default:
			return -1;
		}
	}
};

Maze generateMaze(int width, int height, std::mt19937& engine) {
	int size = width * height;

	Maze maze(width, height, std::uniform_int_distribution<int>(0, size - 1)(engine));
	maze.cells[maze.START].cost = 0;

	int goal;
	std::vector<int> wallList;
	wallList.emplace_back(maze.START);

	while (!wallList.empty()) {
		int picked = std::uniform_int_distribution<int>(0, static_cast<int>(wallList.size() - 1))(engine);

		int pos = wallList[picked];
		wallList.erase(wallList.begin() + picked);

		if (maze.cells[pos].isWall && maze.canChangeMark(pos)) {
			int numWalls = 0;
			if (pos % width <= 0 || maze.cells[pos - 1].isWall)             ++numWalls;
			if (pos % width >= width - 1 || maze.cells[pos + 1].isWall)     ++numWalls;
			if (pos < width || maze.cells[pos - width].isWall)              ++numWalls;
			if (pos + width >= size || maze.cells[pos + width].isWall) ++numWalls;
			if (numWalls > 2) {
				maze.cells[pos].isWall = false;
				goal = pos;
				if (pos % width > 0)         wallList.emplace_back(pos - 1);
				if (pos % width < width - 1) wallList.emplace_back(pos + 1);
				if (pos >= width)            wallList.emplace_back(pos - width);
				if (pos + width < size)      wallList.emplace_back(pos + width);
			}
		}
	}
	maze.goal = goal;

	return std::move(maze);
}

int main(int argc, char** argv) {
	std::random_device rand_dev;
	std::mt19937 engine(rand_dev());

	auto maze = generateMaze(WIDTH, HEIGHT, engine);
	int maxLength = -1, maxTurns = -1, maxBlanks = -1;
	for (int i = 0; i < WIDTH * HEIGHT;) {
		auto tmpMaze = maze;
		for (int j = 0; j < PERTURBATIONS; ++j) {
			tmpMaze.perturbate(engine);
		}
		if (tmpMaze.solve()) {
			int b = tmpMaze.countBlanks();
			if (std::make_tuple(tmpMaze.shortestPathLength, tmpMaze.turns, b)
				>= std::make_tuple(maxLength, maxTurns, maxBlanks)) {
				maze = tmpMaze;
				maxLength = tmpMaze.shortestPathLength;
				maxTurns = tmpMaze.turns;
				maxBlanks = b;
			}
			++i;
		}
	}
	maze.fillDeadSpace();
	maze.print();

	return 0;
}