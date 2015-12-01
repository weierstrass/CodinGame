/*
 * Tron Battle - mutli game
 */

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <map>
#include <set>
#include <algorithm>
#include <memory>


using namespace std;

enum class Direction {Left = 0, Right, Up, Down};
static vector<string> kDirectionString = {"LEFT", "RIGHT", "UP", "DOWN"};
static vector<Direction> kAllDirections = {Direction::Left, Direction::Right, Direction::Up, Direction::Down};

struct Player
{
	Player() : _direction(Direction::Left) {}

	vector<pair<int, int>> _trail;
	pair<int, int> _position;
	Direction _direction;
};

struct ArenaInterface
{
	virtual ~ArenaInterface() {}
	
	/*
	 * Prints the arena with trails and bots. Can be used for debugging.
	 */
	virtual void print() = 0;

	/*
	 * Returns true if (x, y) contains a trail, a bot or is outside of arena.
	 */
	virtual bool isTrail(int x, int y) = 0;

	/*
	 * Returns true if (x, y) contains a bot.
	 */
	virtual bool isBot(int x, int y) = 0;

	/*
	 * Adds the current position (x, y) for the bot of player (playerId)
	 */
	virtual void addBotPosition(int playerId, int x, int y) = 0;

	/*
	 * Updates the current direction for the bot if player (playerId)
	 */
	virtual void updateBotDirection(int playerId, int deltaX, int deltaY) = 0;

	/*
	 * Returns my player.
	 */ 
	virtual const Player &getMyPlayer() = 0;

};

class Arena : public ArenaInterface
{
public:
	static const int Height = 20;
	static const int Width = 30;

	Arena() : _myId(-1) {}

	void print()
	{
	    for (int y = 0; y < Height; ++y)
		{
			for (int x = 0; x < Width; ++x)
			{
				if (isBot(x, y))
				{
					cerr << "@";
				}	
				else if (isTrail(x, y))
				{
					cerr << "X";
				}
				else
				{
					cerr << ".";
				}					 
			}
			cerr << endl;
		}
  	}

	bool isTrail(int x, int y)
	{
		pair<int, int> c(x, y);
		for (const auto &playerPair : _players)
		{
			const Player player = playerPair.second;
			if (find(player._trail.begin(), player._trail.end(), c) != player._trail.end())
			{
				return true;
			}
		}

		// if position is outside of arena, consider it as a trail.
		if (x < 0 || x >= Width || y < 0 || y >= Height)
		{
			return true;
		}
		return false;
	}

	bool isBot(int x, int y)
	{
		pair<int, int> c(x, y);
		for (const auto&playerPair : _players)
		{
			if (playerPair.second._position == c)
			{
				return true;
			}
		}
		return false;
	}

	void addBotPosition(int playerId, int x, int y)
	{
		Player *player = getPlayer(playerId);

		if (!player)
		{
			if (x >= 0 && y >= 0) // player alive, add him
			{
				_players[playerId] = Player();
				player = &_players[playerId];
			}
			else // player is dead and already removed
			{
				return;
			}
		}
		
		if ( x < 0 || y < 0) // player is dead, remove him from arena
		{
			cerr << "player died" << endl;
			_players.erase(playerId);
		}
		else // player is alive, add his new position to his trail
		{
			player->_trail.push_back(pair<int, int>(x, y));
			player->_position = pair<int, int>(x, y);   
   		}
	}

	void updateBotDirection(int playerId, int deltaX, int deltaY)
	{
		Player *player = getPlayer(playerId);	
		if (!player) return;

		if (deltaX > 0) player->_direction = Direction::Right;
		else if (deltaX < 0) player->_direction = Direction::Left;
		else if (deltaY > 0) player->_direction = Direction::Down;
		else if (deltaY < 0) player->_direction = Direction::Up;
	}
	
	void setMyId(int id) { _myId = id; }


	Player *getPlayer(int id)
	{
		map<int, Player>::iterator it = _players.find(id);

		if (it != _players.end())
		{
			return &it->second;
		}

		return 0;
	}

	const Player &getMyPlayer()
	{
		return *getPlayer(_myId);
	}

private:
	map<int, Player> _players;
//       ^     ^
//  player id  |
//           player

	int _myId;
	
};

struct StrategyHelper
{
	/*
	 * Returns new position after a one step move from initial in direction dir.
	 */
	static pair<int, int> Move(pair<int, int> inital, Direction dir)
	{
		switch(dir)
		{
		case Direction::Left:
			inital.first--;
			break;
		case Direction::Right:
			inital.first++;
			break;
		case Direction::Up:
			inital.second--;
			break;
		case Direction::Down:
			inital.second++;
			break;
		default:
			cerr << "WARN Unknown direction" << endl;
			break;
		}
		return inital;
	}

	/*
	 * Returns the directions that you can take in the next move without dying.
	 */
	static vector<Direction> GetPossibleDirections(ArenaInterface &arena)
	{
		vector<Direction> possibleDirections = {};
		pair<int, int> position = arena.getMyPlayer()._position;
		
		for (Direction direction : kAllDirections)
		{
			pair<int, int> newPosition = Move(position, direction);
			if (!arena.isTrail(newPosition.first, newPosition.second))
			{
				possibleDirections.push_back(direction);
			}
		}

		return possibleDirections;
	}
	
};

/*
 * Inteface to be implemented by all strategies.
 */
struct StrategyInterface
{
	virtual ~StrategyInterface() {};

	/*
	 * Should return the direction that the strategy has computed.
	 */
	virtual Direction getNextDirection(ArenaInterface &arena) = 0;

};

/*
 * Chooses randomly the next direction.
 * (Only considers directions that will not kill the player.) 
 */
class RandomStrategy : public StrategyInterface
{
public:
	virtual ~RandomStrategy() {}

	virtual Direction getNextDirection(ArenaInterface &arena)
	{
		vector<Direction> directions = StrategyHelper::GetPossibleDirections(arena);

		if (directions.empty()) // no possible moves
		{
			directions = kAllDirections; // you are screwed anyway, let's take whatever...
		}
		
		random_shuffle(directions.begin(), directions.end());

		return directions[0];
	}
};

/**
 * Factory choosing the best strategy according to the situation.
 */
class FactoryStrategy
{
public:
	FactoryStrategy(const Arena &arena) : _arena(arena) {}

	shared_ptr<StrategyInterface> getStrategy()
	{
		// TODO for now we always return the RandomStrategy
		return make_shared<RandomStrategy>();
	}
private:
	const Arena _arena;
};

int main()
{
	Arena arena;

	RandomStrategy randomStrategy;
	
	while (true)
	{
		int N; // total number of players (2 to 4).
		int P; // your player number (0 to 3).
		cin >> N >> P; cin.ignore();

		arena.setMyId(P);

		for (int i = 0; i < N; i++) {
			int X0; // starting X coordinate of lightcycle (or -1)
			int Y0; // starting Y coordinate of lightcycle (or -1)
			int X1; // starting X coordinate of lightcycle (can be the same as X0 if you play before this player)
			int Y1; // starting Y coordinate of lightcycle (can be the same as Y0 if you play before this player)
			cin >> X0 >> Y0 >> X1 >> Y1; cin.ignore();
			
			arena.addBotPosition(i, X1, Y1);
			arena.updateBotDirection(i, X1 - X0, Y1 - Y0);
		}

		arena.print();

		FactoryStrategy aFactoryStrategy(arena);

		shared_ptr<StrategyInterface> strategy = aFactoryStrategy.getStrategy();

		cout << kDirectionString[(int)strategy->getNextDirection(arena)] << endl; // A single line with UP, DOWN, LEFT or RIGHT
	}
}
