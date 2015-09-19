
#ifndef search_h
#define search_h

#include <iomanip>

#include "board.h"
#include "hash.h"
#include "move.h"
#include "evaluate.h"
#include "nextmove.h"

#include "parameters.h"

class SEARCH {
public:
	SEARCH(int depth, CHESSBOARD * game, HASHTABLE * h, long time, unsigned long long nodes_for_time_check)
	{	
		nodes_for_time_check_ = nodes_for_time_check;
		root_depth = depth; root_game = game; q_max_ply = 0; selective_depth = 0;
		start_time =0; end_time = 0; allotted_time = time; fTimeOut = false;
		node_count =0; best_score = -INFTY; 
		hashtable = h; side_to_move = game->getActivePlayer();
		//move_list = new MOVELIST[root_depth+1];
		//for(int i =0; i<=root_depth; i++) {
		for(int i =0; i < MAX_PLY; i++) {
			move_list[i].linkGame(root_game);
		}
		for(int i =0; i < Q_CUTOFF; i++ ) {
			q_move_list[i].linkGame(root_game);
		}
		#ifdef SEARCH_STATS
			for(int i=0; i <= BAD_CAPTURES; i++)
				cutoff_phase[i]=0;
			for(int i=0; i < 256; i++) {
				cutoff_move[i]=0;
				alpha_move[i]=0;
			}
		#endif
	}
	~SEARCH() 
	{	
		//hashtable->printStats();
		#ifdef SEARCH_STATS
			if(root_depth >= 8) print_stats();
		#endif
		//delete move_list;
	}

	void	start(int alpha_bound = -INFTY, int beta_bound = INFTY);
	
	void	Hard_PVS_Root();
	int		Hard_PVS(unsigned char depth, unsigned char ply, CHESSBOARD * game, int alpha, int beta, bool fNulled = false/*, unsigned char parent_moves_searched = 0*/);
	int		Hard_qSearch(CHESSBOARD * game, int alpha, int beta, int qPly);

	bool	isOutOfTime();
	bool	TimedOut()		{return fTimeOut;}

	void	setHasheTable(HASHTABLE * t);

	int		getNodes()		{return node_count;}
	int		getNPS()		{return ((int)(node_count/getRunTime()) * 1000);}
	int		getDepth()		{return root_depth; }
	int		getSelDepth()	{return selective_depth; }
	int		getQDepth()		{return q_max_ply;}
	double	getRunTime()	{return end_time - start_time;}
	MOVE	getBestMove()	{return best_move;};
	int		getScore()		{return best_score; }

	void	collectPV();
	std::vector<MOVE> getPV() {return pv; }

	#ifdef SEARCH_STATS
		void print_stats() {
			std::cout << "SEARCH STATS:\n";
			std::cout << std::setw(20) <<" BETA CUTOFFS:";
			std::cout << std::setw(50) <<"WINNING ALPHA IMPROVEMENT:\n";
			int last_index;
			for(last_index = 255; last_index >=0; last_index--) {
				if(cutoff_move[last_index] > 0 || alpha_move[last_index] > 0)
					break;
			}			

			long long sum_moves =0;
			long long sum_alpha_moves =0;
			for(int i = 0; i <= last_index; i++) {
				sum_moves+=cutoff_move[i];
				sum_alpha_moves+=alpha_move[i];
			}
			
			for(int i = 0; i <= last_index; i++) {
				if(i ==0)
					std::cout << std::setw(12) << "(None)";
				else if(i < 10) 
					std::cout << std::setw(10) << "Move " << i << ":";
				else
					std::cout << std::setw(9) << "Move " << i << ":";
				std::cout << std::setw(15) << (double)cutoff_move[i]/sum_moves *100 << "%";
				std::cout << std::setw(10) << cutoff_move[i];
				std::cout << std::setw(25) << (double)alpha_move[i]/sum_alpha_moves *100<< "%";
				std::cout << std::setw(10) << alpha_move[i] << std::endl;
			}
			std::cout << std::setw(35) << "Total: " <<  sum_moves;
			std::cout << std::setw(35) << "Total: " <<  sum_alpha_moves << std::endl << std::endl;
			long long sum_phases = 0;
			long long sum_alpha_phases = 0;
			for(int i=0; i <= BAD_CAPTURES; i++) {
				sum_phases += cutoff_phase[i];
				sum_alpha_phases += alpha_phase[i];
			}
			
			for(int i=0; i <= BAD_CAPTURES; i++) {
				std::cout << std::setw(15) << "Phase " << i << ":" << std::setw(10) << (double)cutoff_phase[i]/sum_phases *100<< "%" << std::setw(10) << cutoff_phase[i];
				std::cout << std::setw(25) << (double)alpha_phase[i]/sum_alpha_phases *100 << "%" << std::setw(10) << alpha_phase[i] << std::endl;
			}

			system("PAUSE");


		}
		long long cutoff_phase[BAD_CAPTURES+1];
		long long cutoff_move[256];
		long long alpha_move[256];
		long long alpha_phase[BAD_CAPTURES+1];
		
	#endif

private:
	
	CHESSBOARD * root_game;
	bool side_to_move;
	int root_depth;
	short selective_depth;

	int node_count;
	int q_max_ply;

	long start_time;
	long allotted_time;
	long end_time;
	unsigned long long nodes_for_time_check_;
	int root_alpha;
	int root_beta;

	MOVE best_move;
	int best_score;

	bool fTimeOut;

	HASHTABLE * hashtable;
	std::vector<MOVE> pv;

	QMOVELIST q_move_list[Q_CUTOFF];
	//MOVELIST * move_list;
	MOVELIST move_list[MAX_PLY];
};

#endif