
#ifndef search_h
#define search_h

#include "board.h"
#include "hash.h"
#include "move.h"
#include "evaluate.h"

//#define DEBUG_SEARCH
//#define SEARCH_STATS

const short LMR_THRESHOLD = 5;
const short LMR_HORIZON = 4;

class SEARCH {
public:
	SEARCH(int depth, CHESSBOARD * game, HASHTABLE * h, long time )
	{	root_depth = depth; root_game = game; q_depth = 0;
		start_time =0; end_time = 0; allotted_time = time; fTimeOut = false;
		node_count =0; best_score = -INFINITY; 
		hashtable = h; 
		killer_move[0] = new MOVE[root_depth];
		killer_move[1] = new MOVE[root_depth];
		killer_counter[0] = new short[root_depth];
		killer_counter[1] = new short[root_depth];
		killer_counter[0][root_depth-1] = 0;
		killer_counter[1][root_depth-1] = 0;
		killer_move[0][root_depth-1].from = 64;
		killer_move[1][root_depth-1].from = 64;
	}
	~SEARCH() 
	{	delete killer_move[0];
		delete killer_move[1]; 
		delete killer_counter[0];
		delete killer_counter[1]; }

	void					start(int alpha_bound = -INFINITY, int beta_bound = INFINITY);
	
	void					Hard_PVS_Root();
	int						Hard_PVS(int depth, CHESSBOARD * game, int alpha, int beta, bool fNulled = false);
	int						Hard_qSearch(CHESSBOARD * game, int alpha, int beta, int qDepth);

	void					PVS_Root();
	int						PVS(int depth, CHESSBOARD * game, int alpha, int beta, bool fNulled = false);
	int						qSearch(CHESSBOARD * game, int alpha, int beta, int qDepth);

	void					addKiller(short depth, MOVE killer);
	short					countKillers();
	void					clearKillers(short depth);
	short					countKiller(unsigned char index, short depth) {return killer_counter[index][depth];}
	std::vector<MOVE>		getKillers(CHESSBOARD * game, short ply);
	MOVE					getKiller(unsigned char index, short depth) {return killer_move[index][depth];}

	bool	isOutOfTime();
	bool	TimedOut()		{return fTimeOut;}

	void	setHasheTable(HASHTABLE * t);

	int		getNodes()		{return node_count;}
	int		getNPS()		{return ((int)(node_count/getRunTime()) * 1000);}
	int		getDepth()		{return root_depth; }
	int		getQDepth()		{return q_depth;}
	double	getRunTime()	{return end_time - start_time;}
	MOVE	getBestMove()	{return best_move;};
	int		getScore()		{return best_score; }

	void	collectPV();
	std::vector<MOVE> getPV() {return pv; }

	#ifdef SEARCH_STATS
		void print_stats() {
			std::cout << std::endl;
			long long beta_total = 0;
			long long killer_total = 0;
			for(int i = 1; i<60; i++) {
				std::cout << "Move " << i << ":\t" << beta_cutoffs[i] << "(beta)\t"<< killer_cutoffs[i] << "(killer)" << std::endl;
				beta_total += beta_cutoffs[i];
				killer_total += killer_cutoffs[i];
			}
			std::cout << "Totals: \t" << beta_total << "\t" << killer_total << std::endl;
		}
		long long beta_cutoffs[255];
		long long alpha_cutoffs[255];
		long long killer_cutoffs[255];
	#endif

private:
	
	CHESSBOARD * root_game;
	int root_depth;

	int node_count;
	int q_depth;

	long start_time;
	long allotted_time;
	long end_time;

	int alpha;
	int beta;

	MOVE best_move;
	int best_score;

	bool fTimeOut;

	HASHTABLE * hashtable;
	std::vector<MOVE> pv;
	
	MOVE * killer_move[2];
	short * killer_counter[2];
};

bool isMoveLegal(CHESSBOARD b, MOVE m);
bool isMoveLegal(CHESSBOARD b, MOVE m, bool inCheck);

int NegaMax(int depth, CHESSBOARD * game);

#endif