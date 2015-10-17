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


using namespace std;

enum class Direction {Left = 0, Right, Up, Down};
static vector<string> kDirectionString = {"LEFT", "RIGHT", "UP", "DOWN"};

enum class CellStatus {Free = 0, Trail};
static vector<string> kCellStatusString = {".", "X"};

struct Player
{
	Player(int id) : _id(id), _direction(Direction::Left) {}
	int _id;
	vector<pair<int, int>> _trail;
	Direction _direction;
};

class Arena
{
public:
	static const int Height = 20;
	static const int Width = 30;

	void print()
	{
	    for (int y = 0; y < Height; ++y)
		{
			for (int x = 0; x < Width; ++x)
			{
				if (isTrail(pair<int, int>(x, y)))
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

	bool isTrail(const pair<int, int> &c)
	{
		for (const auto &player : _players)
		{
			if (find(player._trail.begin(), player._trail.end(), c) != player._trail.end())
			{
				return true;
			}
		}
		return false;
	}

	void addTrail(int playerId, int x, int y)
	{
		Player *player = getPlayer(playerId);

		if (!player) return;
		
		if ( x < 0 || y < 0) // player is dead, remove him from arena
		{
			_players.erase(remove(_players.begin(), _players.end(), *player), _players.end());
		}
		else // player is alive, add his new position to his trail
		{
			player->_trail.push_back(pair<int, int>(x, y));
   		}
	}


private:
	vector<Player> _players;

	Player *getPlayer(int id)
	{
		 vector<Player>::iterator result = find_if(_players.begin(), _players.end(),
		 										  [id](const Player &p) -> bool { return p._id == id; });

		 if(result != _players.end()) {
		 	return result;
		 }
		
		// for (Player &player : _players)
		// {
		// 	if (player._id == id) return player;
		// }
		cerr << "WARN player not found" << endl;
		return 0;
	}
	
};


struct Strategy
{
	virtual ~Strategy() {};

	virtual Direction getNextDirection(...) = 0; 
};

int main()
{
	Arena arena;
	
	while (true)
	{
		int N; // total number of players (2 to 4).
		int P; // your player number (0 to 3).
		cin >> N >> P; cin.ignore();
		for (int i = 0; i < N; i++) {
			int X0; // starting X coordinate of lightcycle (or -1)
			int Y0; // starting Y coordinate of lightcycle (or -1)
			int X1; // starting X coordinate of lightcycle (can be the same as X0 if you play before this player)
			int Y1; // starting Y coordinate of lightcycle (can be the same as Y0 if you play before this player)
			cin >> X0 >> Y0 >> X1 >> Y1; cin.ignore();
			
			arena.addTrail(i, X1, Y1);
		}

		arena.print();
		// Write an action using cout. DON'T FORGET THE "<< endl"
		// To debug: cerr << "Debug messages..." << endl;

		cout << kDirectionString[(int)Direction::Right] << endl; // A single line with UP, DOWN, LEFT or RIGHT
	}
}
