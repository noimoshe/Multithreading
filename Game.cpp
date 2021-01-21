#include "Game.hpp"


static const char *colors[7] = {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN};
/*--------------------------------------------------------------------------------

--------------------------------------------------------------------------------*/

Game::Game(game_params gameParams) {
    this->m_gen_num=gameParams.n_gen;
    this->m_thread_num=gameParams.n_thread;
    this->print_on=gameParams.print_on;
    this->interactive_on=gameParams.interactive_on;
    this->filename=gameParams.filename;
    this->jobs_queue = new PCQueue<Job*>();
}

Game::~Game() {

}

void Game::run() {

    _init_game(); // Starts the threads and all other variables you need
    print_board("Initial Board");
    for (uint i = 0; i < m_gen_num; ++i) {
        //start timer
        auto gen_start = std::chrono::system_clock::now();
        _step(i); // Iterates a single generation
        //stop timer
        auto gen_end = std::chrono::system_clock::now();
        //append duration to generation history vector
        m_gen_hist.push_back((double)std::chrono::duration_cast<std::chrono::microseconds>(gen_end - gen_start).count());
        print_board(nullptr);
    } // generation loop
    print_board("Final Board");
    _destroy_game();
}

void Game::_init_game() {
	// Create threads
	// Create game fields
	// Start the threads
	// Testing of your implementation will presume all threads are started here

	//create game fields
	//init the board
    vector<string> lines=utils::read_lines(this->filename);

    this->rows=lines.size();
    //this->cols=lines[0].size();
    //this->board=new vector<string>(rows);

    
    this->current=new vector<vector<int>*>;
    for(int i = 0; i < lines.size(); i++){
        vector<string> splited_line=utils::split(lines.at(i),' ');
        if(i == 0) this->cols = splited_line.size();
        vector<int>* int_line = new vector<int>(splited_line.size());
        for(int i = 0; i < splited_line.size(); ++i){
            int to_insert = stoi(splited_line.at(i));
            int_line->at(i) = to_insert;
        }
        this->current->push_back(int_line);
    }

    next=new vector<vector<int>*>;
    for(int i = 0; i < this->rows; i++){
        vector<int>* int_line = new vector<int>(this->cols);
        for(int j = 0; j < this->cols; ++j){
           int to_insert = this->current->at(i)->at(j);
            int_line->at(j) = to_insert;
        }
        next->push_back(int_line);
    }

    m_thread_num = std::min(m_thread_num, (uint)current->size());
    pthread_mutex_init(&this->mutex, NULL);

	//create threads
    for(uint i=0;i<m_thread_num;i++){
        //create pthread, send them with empty workload
        const ourThread* insert=new ourThread(i, this, &this->mutex, this->jobs_queue, &this->m_tile_hist);
        this->m_threadpool.push_back((Thread*)insert);
    }
    //start threads
    for(uint i=0;i<m_thread_num;i++){
        //create pthread, send them with empty workload
        this->m_threadpool[i]->start();
    }
}

void Game::_step(uint curr_gen) {
	// Push jobs to queue
	// Wait for the workers to finish calculating
	// Swap pointers between current and next field

    Barrier barrier = Barrier();

    uint thread_rows = this->rows / m_thread_num;
    uint thread_rows_res = this->rows % m_thread_num;
    //insert N jobs to queue - phase 1
   // uint urows = this->rows;
    uint ucols = this->cols;
    bool last=false;
    for(uint i=0; i<this->m_thread_num-1; i++){
        uint start = (i*thread_rows);
        if(curr_gen==m_gen_num-1) last=true;
        Job* insert = new Job(thread_rows, ucols, start, 1, current, next, &barrier,last);
        this->jobs_queue->push(insert);
        barrier.increase();
	}
    uint last_start = ((this->m_thread_num-1)*thread_rows);
    uint res = (thread_rows + thread_rows_res);
    if(curr_gen==m_gen_num-1) last=true;
    Job* insert_last = new Job(res, ucols, last_start, 1, current, next, &barrier,last);
    this->jobs_queue->push(insert_last);
    barrier.increase();

    barrier.wait();

    //swap curr and next;
    vector<vector<int>*>* tmp = current;
    current = next;
    next = tmp;

    //insert N jobs to queue - phase 2
    for(uint i=0;i<this->m_thread_num-1; i++){
        uint start = (i*thread_rows);
        if(curr_gen==m_gen_num-1) last=true;
        Job* insert = new Job(thread_rows, this->cols, start, 2, current, next, &barrier,last);
        this->jobs_queue->push(insert);
        barrier.increase();
    }
    last_start = ((this->m_thread_num-1)*thread_rows);
    res = thread_rows + thread_rows_res;
    if(curr_gen==m_gen_num-1) last=true;
    Job* insert_last2 = new Job(res, this->cols, last_start, 2, current, next, &barrier,last);
    this->jobs_queue->push(insert_last2);
    barrier.increase();

    barrier.wait();

    //swap curr and next
    tmp = current;
    current = next;
    next = tmp;
}

void Game::_destroy_game(){
	// Destroys board and frees all threads and resources
	// Not implemented in the Game's destructor for testing purposes.
	// Testing of your implementation will presume all threads are joined here
    for(int i = 0; i < this->rows; i++){
        delete this->current->at(i);
        delete this->next->at(i);
    }
    delete this->current;
    delete this->next;
    delete this->jobs_queue;

    for (uint i = 0; i < m_thread_num; ++i) {
        m_threadpool[i]->join();
    }
}

uint Game::thread_num() const {
    return this->m_thread_num;
}

const vector<double> Game::tile_hist() const {
    return this->m_tile_hist;
}

const vector<double> Game::gen_hist() const {
    return this->m_gen_hist;
}

//ourThread

double ourThread::getAvgSpecies(vector<vector<int>*>* board, int row,int col){
    int neighborhood[9][2]={{-1,-1}, {-1,0} ,{-1,1},
                            {0,-1}, {0,0} ,{0,1},
                            {1,-1}, {1,0} ,{1,1} };
    double sum=0;
    double count_alive=0;
    for(int i=0; i<9; i++){
        if(row+neighborhood[i][0] >= 0 && row+neighborhood[i][0] < board->size() && col+neighborhood[i][1] >= 0 && col+neighborhood[i][1] < board->at(0)->size()) {
            int value = board->at(row + neighborhood[i][0])->at(col + neighborhood[i][1]);
            sum += value;
            if (value != 0) {
                count_alive++;
            }
        }
    }
    double avg=0;
    if(count_alive > 0){
        avg = sum/count_alive;
    }
    return avg;
}


int ourThread::countLiveNeighbors(vector<vector<int>*>* board, int row,int col){
    int neighbors[8][2]={{-1,-1}, {-1,0} ,{-1,1},
                         {0,-1},{0,1},
                         {1,-1}, {1,0} ,{1,1} };
    int count_live=0;
    for(int i=0; i<8; i++){
        if(row+neighbors[i][0] >= 0 && row+neighbors[i][0] < board->size() && col+neighbors[i][1] >= 0 && col+neighbors[i][1] < board->at(0)->size()) {
            if (board->at(row + neighbors[i][0])->at(col + neighbors[i][1]) != 0)
                count_live++;
        }
    }
    return count_live;
}


int ourThread::getDominant(vector<vector<int>*>* board, int row,int col){
    int neighbors[9][2]={{-1,-1}, {-1,0} ,{-1,1},
                         {0,-1},{0,0}, {0,1},
                         {1,-1}, {1,0} ,{1,1} };
    int species_histogram[8]={0};
    for(int i=0; i<9; i++){
        if(row+neighbors[i][0] >= 0 && row+neighbors[i][0] < board->size() && col+neighbors[i][1] >= 0 && col+neighbors[i][1] < board->at(0)->size()) {
            int value = board->at(row + neighbors[i][0])->at(col + neighbors[i][1]);
            species_histogram[value]++;
        }
    }
    int max=0;
    int max_index=0;
    for(int i=0; i<8; i++){
        int calc=i*species_histogram[i];
        if(calc > max){
            max=calc;
            max_index=i;
        }
    }
    return max_index;
}

void ourThread::thread_workload() {
    while(1) {
        Job *todo = this->jobs_queue->pop();
        auto gen_start = std::chrono::system_clock::now();
        int start_row = todo->start_row;
        //execute Job:
        if (todo->phase == 1) {
            //do phase 1
            for (uint i = start_row; i < start_row + todo->rows; i++) {
                for (uint j = 0; j < todo->cols; j++) {
                    int count_live_neighbors = countLiveNeighbors(todo->current_board, i, j);
                    //in case this cell is dead
                    if (todo->current_board->at(i)->at(j) == 0) {
                        if (count_live_neighbors == 3) {
                            int new_species = getDominant(todo->current_board, i, j);
                            todo->next_board->at(i)->at(j)=new_species;
                        }
                        else{ //else - remains dead.
                            todo->next_board->at(i)->at(j)=0;
                        }

                    } else {//in case this cell is alive - remains alive only if count_live_neighbors== 2 or 3. else - dies
                        todo->next_board->at(i)->at(j)=0;
                        if((todo->current_board->at(i)->at(j) != 0)&& (count_live_neighbors == 2 || count_live_neighbors == 3))
                            todo->next_board->at(i)->at(j)=todo->current_board->at(i)->at(j);

                      /*  if (!((todo->current_board->at(i)->at(j) != 0)&& (count_live_neighbors == 2 || count_live_neighbors == 3))) {
                            todo->next_board->at(i)->at(j)=0;
                        }*/

                    }
                }
            }

        } else if (todo->phase == 2) {
            //do phase 2
            int start_row = todo->start_row;
            for (uint i = start_row; i < start_row + todo->rows; i++) {
                for (uint j = 0; j < todo->cols; j++) {
                    if(todo->current_board->at(i)->at(j)!=0){
                        int new_species = std::round(getAvgSpecies(todo->current_board, i, j));
                        todo->next_board->at(i)->at(j)=new_species;
                    }
                    else {
                        todo->next_board->at(i)->at(j)=0;
                    }
                }
            }

        }
        auto gen_end = std::chrono::system_clock::now();

        //append duration to generation history vector
        pthread_mutex_lock(this->mutex);
        this->tile_hist->push_back((double)std::chrono::duration_cast<std::chrono::microseconds>(gen_end - gen_start).count());
        pthread_mutex_unlock(this->mutex);

        todo->barrier->decrease();
        if(todo->phase==2 && todo->last_gen){
            pthread_exit(0);
        }
    }
}

//barrier

Barrier::Barrier(){
    this->working = 0;
    pthread_mutex_init(&this->mutex,NULL);
}

void Barrier::increase() {
    pthread_mutex_lock(&this->mutex);
    working++;
    pthread_mutex_unlock(&this->mutex);
}
void Barrier::decrease(){
    pthread_mutex_lock(&this->mutex);
    working--;
    pthread_mutex_unlock(&this->mutex);
}

void Barrier::wait(){
    while(working!=0){}
}

/*--------------------------------------------------------------------------------

--------------------------------------------------------------------------------*/
inline void Game::print_board(const char* header) {

    if(print_on){

        // Clear the screen, to create a running animation
        if(interactive_on)
            system("clear");

        // Print small header if needed
        if (header != nullptr)
            cout << "<------------" << header << "------------>" << endl;

        // TODO: Print the board
        vector<vector<int>*>* matrix = this->current;
        if(matrix->size() > 0) {
            //int counter = 0;
            cout << u8"╔" << string(u8"═") * this->cols << u8"╗" << endl;
            for (uint i = 0; i < this->rows; ++i) {
                cout << u8"║";
                for (uint j = 0; j < this->cols; ++j) {
                    if (matrix->at(i)->at(j) > 0)
                        cout << colors[(matrix->at(i)->at(j)) % 7] << u8"█" << RESET;
                    else
                        cout << u8"░";
                }
                cout << u8"║" << endl;
            }
            cout << u8"╚" << string(u8"═") * this->cols << u8"╝" << endl;
        }

        // Display for GEN_SLEEP_USEC micro-seconds on screen
        if(interactive_on)
            usleep(GEN_SLEEP_USEC);
    }

}


/* Function sketch to use for printing the board. You will need to decide its placement and how exactly
	to bring in the field's parameters.

	    cout << u8"╔" << string(u8"═") * field_width << u8"╗" << endl;
		for (uint i = 0; i < field_height ++i) {
			cout << u8"║";
			for (uint j = 0; j < field_width; ++j) {
                if (field[i][j] > 0)
                    cout << colors[field[i][j] % 7] << u8"█" << RESET;
                else
                    cout << u8"░";
			}
			cout << u8"║" << endl;
		}
		cout << u8"╚" << string(u8"═") * field_width << u8"╝" << endl;
*/ 



