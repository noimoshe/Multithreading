#ifndef __GAMERUN_H
#define __GAMERUN_H

/*--------------------------------------------------------------------------------
								  Species colors
--------------------------------------------------------------------------------*/
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black - 7 */
#define RED     "\033[31m"      /* Red - 1*/
#define GREEN   "\033[32m"      /* Green - 2*/
#define YELLOW  "\033[33m"      /* Yellow - 3*/
#define BLUE    "\033[34m"      /* Blue - 4*/
#define MAGENTA "\033[35m"      /* Magenta - 5*/
#define CYAN    "\033[36m"      /* Cyan - 6*/

#include "Thread.hpp"
#include "utils.hpp"
#include "PCQueue.hpp"

/*--------------------------------------------------------------------------------
								  Auxiliary Structures
--------------------------------------------------------------------------------*/
struct game_params {
	// All here are derived from ARGV, the program's input parameters. 
	uint n_gen;
	uint n_thread;
	string filename;
	bool interactive_on; 
	bool print_on; 
};
/*--------------------------------------------------------------------------------
									Class Declaration
--------------------------------------------------------------------------------*/
class Barrier {
private:
    int working;
    pthread_mutex_t mutex;
public:
    Barrier();
    void increase();
    void decrease();
    void wait();
};

//Job to be done on a board
class Job {
public:
    Job(uint rows,uint cols,uint start_row, int phase, vector<vector<int>*>* current, vector<vector<int>*>* next, Barrier* barrier, bool last_gen) {
        this->rows=rows;
        this->cols=cols;
        this->start_row=start_row;
        this->phase = phase;
        this->current_board = current;
        this->next_board = next;
        this->barrier = barrier;
        this->last_gen=last_gen;
    }
    uint rows;
    uint cols;
    uint start_row;
    int phase;
    vector<vector<int>*>* current_board;
    vector<vector<int>*>* next_board;
    Barrier* barrier;
    bool last_gen;
};

class Game {
public:

	Game(game_params gameParams);
	~Game();
	void run(); // Runs the game
	const vector<double> gen_hist() const; // Returns the generation timing histogram
	const vector<double> tile_hist() const; // Returns the tile timing histogram
	uint thread_num() const; //Returns the effective number of running threads = min(thread_num, field_height)

protected: // All members here are protected, instead of private for testing purposes

	// See Game.cpp for details on these three functions
	void _init_game(); 
	void _step(uint curr_gen); 
	void _destroy_game();
    inline void print_board(const char* header);

	uint m_gen_num; 			 // The number of generations to run
	uint m_thread_num; 			 // Effective number of threads = min(thread_num, field_height)
	vector<double> m_tile_hist; 	 // Shared Timing history for tiles: First m_gen_num cells are the calculation durations for tiles in generation 1 and so on.
	vector<double> m_gen_hist;  	 // Timing history for generations: x=m_gen_hist[t] iff generation t was calculated in x microseconds
	vector<Thread*> m_threadpool; // A storage container for your threads. This acts as the threadpool.

	bool interactive_on; // Controls interactive mode - that means, prints the board as an animation instead of a simple dump to STDOUT 
	bool print_on; // Allows the printing of the board. Turn this off when you are checking performance (Dry 3, last question)

  
    PCQueue<Job*>* jobs_queue;
    uint rows;
    uint cols;
    string filename;
    vector<vector<int>*>* current;
    vector<vector<int>*>* next;
    Barrier barrier = Barrier();
    pthread_mutex_t mutex;
};


//Our thread
class ourThread: public Thread {
    Game* game;
    pthread_mutex_t* mutex;
    PCQueue<Job*>* jobs_queue;
    vector<double>* tile_hist;
public:
    ourThread(uint thread_id, Game* game, pthread_mutex_t* m, PCQueue<Job*>* jobs_queue, vector<double>* tile_hist) : Thread(thread_id), game(game), mutex(m), jobs_queue(jobs_queue),
    tile_hist(tile_hist) {};
    void thread_workload() override;
    double getAvgSpecies(vector<vector<int>*>* board, int row,int col);
    int countLiveNeighbors(vector<vector<int>*>* board, int row,int col);
    int getDominant(vector<vector<int>*>* board, int row,int col);
};
#endif



