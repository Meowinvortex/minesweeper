
// includes
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <map>
#include <ctime>
#include <iostream>
#include <thread>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <chrono>

// constants for detemrining grid size
const int GRID_WIDTH = 16;
const int GRID_HEIGHT = 16;
const int CELL_SIZE = 40;

// variables for mines and flag amount
int amount = 20;
int flag_amount = 20;



using namespace std;

// create the sfml window and the gui overlay for tgui
sf::RenderWindow window(sf::VideoMode(GRID_WIDTH * CELL_SIZE + 200, GRID_HEIGHT * CELL_SIZE + 200), "Minesweeper");
tgui::Gui gui(window);


// Each cell in the minesweeper grid is defined inside its own grid_cell class, this includes the sfml shape, x and y coords relative to the grid, mines adjacent
// to the given cell and if the cell is a mine, flagged, hidden or been flooded when flooding the adjacent blanks (line: 207)
class grid_cell {
public:
    sf::RectangleShape shape;
    int x;
    int y;
    int adj_mines;
    bool hidden;
    bool mine;
    bool flagged;
    bool flooded;
    grid_cell() : x(0), y(0), hidden(true), mine(false), flagged(false) {
        shape.setSize(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        shape.setOutlineThickness(1);
        shape.setOutlineColor(sf::Color::Black);
        shape.setFillColor(sf::Color::White);
    }


    grid_cell(sf::RectangleShape cell, int x_pos, int y_pos)
        : shape(cell), x(x_pos), y(y_pos), hidden(true), mine(false), flagged(false), flooded(false) {
    }
};

// this map stores all of the classes for each cell
std::map<std::pair<int, int>, grid_cell> cells;

// any textures that are required globally
sf::Texture atlas;
sf::Texture reset_txt;
sf::Texture smile_alive, smile_dead, smile_cool;
sf::Texture exp_atlas;

// these maps are for storing the sub-texture positions when using sprite sheets
map<string,sf::IntRect> textures;
map<int,sf::IntRect> explosions;
map<int,sf::IntRect> digits;


                 
// miscellanous sprites
sf::RectangleShape flags_left_1(sf::Vector2f(75,100));
sf::RectangleShape flags_left_10(sf::Vector2f(75,100));
sf::RectangleShape exp_sprite(sf::Vector2f(200,200));

// clock used for timing the players round
sf::Clock game_clock;


// this bool will turn to true on game loss or completion, to which the majority of functions will be halted until reset
bool pause = false;

// setting up the sub-texture positions for the main texture atlas (this could be more efficient with a vector and a for loop :p)
void load_textures(sf::Texture atlas){
    textures["Blank"] = sf::IntRect(0,0,16,16);
    textures["One"] = sf::IntRect(16,0,16,16);
    textures["Two"] = sf::IntRect(32,0,16,16);
    textures["Three"] = sf::IntRect(48,0,16,16);
    textures["Four"] = sf::IntRect(0,16,16,16);
    textures["Five"] = sf::IntRect(16,16,16,16);
    textures["Six"] = sf::IntRect(32,16,16,16);
    textures["Seven"] = sf::IntRect(48,16,16,16);
    textures["Eight"] = sf::IntRect(0,32,16,16);
    textures["Null"] = sf::IntRect(16,32,16,16);
    textures["Flag"] = sf::IntRect(32,32,16,16);
    textures["Red-flag"] = sf::IntRect(48,32,16,16);
    textures["Bomb"] = sf::IntRect(16,48,16,16);
}

// same as previous function but for the digits
void load_digits(){
     int count = 0;
     for(int y = 0; y < 2; y++){
        for(int x = 0; x < 5; x++){
          digits[count] = sf::IntRect(32 * x, 64 * y, 32, 64);
          count += 1;
        }
     }
}

// sam as previous but for the explosion upon sweeping a mine
void load_explosion(){
    int count = 0;
    for(int y = 0; y < 2; y++){
       for(int x = 0; x < 3; x++){
         explosions[count] = sf::IntRect(189 * x, 220 * y, 189, 220);
         count += 1;
       }
    }
}

// fucntion that creates the grid
void makegrid(sf::RenderWindow& window) {
    for (int x = 0; x < GRID_WIDTH; ++x) {
        for (int y = 0; y < GRID_HEIGHT; ++y) {
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            cell.setPosition(x * CELL_SIZE, y * CELL_SIZE + 150);
            cell.setOutlineThickness(1);
            cell.setOutlineColor(sf::Color::Black);
            cell.setFillColor(sf::Color::White);
            cell.setTexture(&atlas);
            cell.setTextureRect(textures["Null"]);
            grid_cell cell_classed(cell, x, y);
            cells[{x, y}] = cell_classed;
        }
    }
}

// for the main textures map of rectangles to allow mathmatical operation to be done to determine which string version of number
string int_to_string(int num){
    switch(num){
        case 1:
          return "One";
        case 2:
          return "Two";
        case 3:
          return "Three";
        case 4:
          return "Four";
        case 5:
          return "Five";
        case 6:
          return "Six";
        case 7:
          return "Seven";
        case 8:
          return "Eight";
        default:
          return "Blank";
    }
}

// function that randomly selects cells to have a mine based on ow many mines have been selected
void create_mines() {
    std::vector<std::pair<int, int>> free_positions;
    int mines_placed = 0;
    if(amount == 0){
        amount = (GRID_HEIGHT * GRID_WIDTH) - 1;
    }
    for (int i = 0 ; i < amount ; i++) {
        if (mines_placed >= amount) break;
        cells[{rand()%GRID_WIDTH,rand()%GRID_HEIGHT}].mine = true;
        mines_placed++;
    }
}

// is executed after adjacent blanks have been flooded to show cells with mines adjacent to it which also border the flooded blanks to reveal aswell
void show_bordered_cells(){
    map <string,pair<int,int>> adjacent_cells;
    for(int x = 0; x < GRID_WIDTH; x++){
        for(int y = 0; y < GRID_HEIGHT; y++){
          if(!cells[{x,y}].hidden){
            adjacent_cells["left"] = {x-1,y};
            adjacent_cells["right"] = {x+1,y};
            adjacent_cells["up"] = {x,y-1};
            adjacent_cells["down"] = {x,y+1};
            adjacent_cells["left-up"] = {x-1,y-1};
            adjacent_cells["left-down"] = {x-1,y+1};
            adjacent_cells["right-up"] = {x+1,y-1};
            adjacent_cells["right-down"] = {x+1,y+1};
            for(auto& pair : adjacent_cells){
                if((pair.second.second <= GRID_HEIGHT && pair.second.second >=0) &&  (pair.second.first <= GRID_WIDTH && pair.second.first >= 0)){
                    if(cells[pair.second].adj_mines > 0){
                        cells[pair.second].shape.setTextureRect(textures[int_to_string(cells[pair.second].adj_mines)]);
                    }
                }
            }
          }
        }
    }
}

// function that run upon the players sweeping a blank, it takes the adjacent(non diagonal) cells to the blank and reveals them if they are also blank
// it will then repeat this for adjacents of adjacents and so on until there are no more
void flood_blanks(int x,int y){
  vector <pair<int,int>> cells_to_check;
  cells_to_check.push_back({x,y});
  for(int i = 0; i < cells_to_check.size(); i++){
    pair <int,int> current = {cells_to_check[i].first, cells_to_check[i].second};
    if(cells[current].adj_mines == 0 && !cells[current].flooded && !cells[current].mine){
        // define cell as blank
        cells[current].shape.setTextureRect(textures["Blank"]);
        cells[current].flooded = true;
        cells[current].hidden = false;
        // check cell above
        if(current.second > 0 && !cells[{current.first,current.second-1}].flooded && !cells[{current.first,current.second-1}].flagged){
          cells_to_check.push_back({current.first,current.second-1});
        }
        // check cell below
        if(current.second < GRID_HEIGHT - 1 && !cells[{current.first,current.second+1}].flooded && !cells[{current.first,current.second+1}].flagged){
            cells_to_check.push_back({current.first,current.second+1});
          } 
        // check left cell
        if(current.first > 0 && !cells[{current.first-1,current.second}].flooded && !cells[{current.first-1,current.second}].flagged){
          cells_to_check.push_back({current.first-1,current.second});
        }
        // check right cell
        if(current.first < GRID_WIDTH - 1 && !cells[{current.first+1,current.second}].flooded && !cells[{current.first+1,current.second}].flagged){
            cells_to_check.push_back({current.first+1,current.second});
        }    
    }
  }
  thread t(show_bordered_cells);
  t.detach();
}

// runs once the cells have been randomly selected to have mines, it goes through each cell in the grid and counts and stores how many mines that cell is surrounded by
void store_adj(){
    map <string,pair<int,int>> adjacent_cells;
    int adjmines = 0;
    for(int x = 0; x < GRID_WIDTH; x++){
        for(int y = 0; y < GRID_HEIGHT; y++){
            adjmines = 0;
            adjacent_cells.clear();
            adjacent_cells["left"] = {x-1, y};
            adjacent_cells["right"] = {x+1, y};
            adjacent_cells["up"] = {x, y-1};
            adjacent_cells["down"] = {x, y+1};
            adjacent_cells["left-up"] = {x-1, y-1};
            adjacent_cells["left-down"] = {x-1, y+1};
            adjacent_cells["right-up"] = {x+1, y-1};
            adjacent_cells["right-down"] = {x+1, y+1};
            for (auto& pair : adjacent_cells) {
                int adj_x = pair.second.first;
                int adj_y = pair.second.second;
                if (adj_x >= 0 && adj_x < GRID_WIDTH && adj_y >= 0 && adj_y < GRID_HEIGHT) {
                    if (cells[{adj_x, adj_y}].mine) {
                        adjmines++;
                    }
                }
            }
            cells[{x, y}].adj_mines = adjmines;
        }
    }
}

// simple animation function to animate each frame of the explosion (doesnt work perfectly as the previous frame isnt cleared but it looks decent)
void animate_exp(){
    exp_sprite.setFillColor(sf::Color::White);
    for(int i = 0; i < 6; i++){
        exp_sprite.setTextureRect(explosions[i]);
        window.draw(exp_sprite);
        window.display();
        this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    exp_sprite.setFillColor(sf::Color::Transparent);

}

// runs when the player sweeps a mine, will switch pause to true and reveal all cells including the mines
void game_over(){
    pause = true;
    for(auto& pair : cells){
        if(pair.second.mine){
            if(pair.second.flagged){pair.second.shape.setTextureRect(textures["Red-flag"]);}
            else{pair.second.shape.setTextureRect(textures["Bomb"]);}
        }
    }
    
}

// function to reset the game, makes sure the player has enough flags for the chosen amount of mines, clears grid and remakes it as well as replacing mines 
// pause is set back to false to allow the player to play again 
void reset(auto button){
    flag_amount = amount;
    window.clear(sf::Color::White);
    cells.clear();
    makegrid(window);
    create_mines();
    store_adj();
    pause = false;
    button -> getRenderer() -> setTexture(smile_alive);
    flags_left_1.setTextureRect(digits[flag_amount%10]);
    flags_left_10.setTextureRect(digits[fdiv(flag_amount,10)]);
    game_clock.restart();
}

// main function
int main() {
    
    // define the random seed for placing mines
    srand(int(time(0)));

    // for storing the current seconds on the game clock
    sf::Time time_recorded;
    
    // storing the explosion sound
    sf::SoundBuffer exp_buffer;
    exp_buffer.loadFromFile("explosion.mp3");
    sf::Sound exp_sound;
    exp_sound.setBuffer(exp_buffer);   

    //storing the digits
    sf::Texture digits_txt;
    sf::FileInputStream digits_str;
    digits_str.open("digits.png");
    digits_txt.loadFromStream(digits_str);
    load_digits();

    // storing the smiley button
    sf::Image smileys;
    smileys.loadFromFile("smiley.png");
    sf::Image subimage;
    subimage.create(32,32);
    subimage.copy(smileys, 0, 0, sf::IntRect(0, 0, 32, 32), false);
    smile_alive.loadFromImage(subimage);
    subimage.copy(smileys, 0, 0, sf::IntRect(33, 0, 32, 32), false);
    smile_dead.loadFromImage(subimage);
    subimage.copy(smileys, 0, 0, sf::IntRect(0, 33, 32, 32), false);
    smile_cool.loadFromImage(subimage);
    

    // setting up the sprites for counting how many flags the player has left to place down
    flags_left_1.setPosition(sf::Vector2f(765, 0));
    flags_left_10.setPosition(sf::Vector2f(690, 0));
    flags_left_10.setTexture(&digits_txt);
    flags_left_1.setTexture(&digits_txt);
    flags_left_1.setTextureRect(digits[flag_amount%10]);
    flags_left_10.setTextureRect(digits[fdiv(flag_amount,10)]);
    

    // setting up the sprites for counting for how many mines has been selected for the next reset
    sf::RectangleShape select_amount_1(sf::Vector2f(37,50));
    sf::RectangleShape select_amount_10(sf::Vector2f(37,50));
    select_amount_1.setPosition(sf::Vector2f(750, 200));
    select_amount_10.setPosition(sf::Vector2f(713, 200));
    select_amount_1.setTexture(&digits_txt);
    select_amount_10.setTexture(&digits_txt);
    
    // setting up the sprites for showing the players time in the game
    sf::RectangleShape timer_1(sf::Vector2f(75,100));
    sf::RectangleShape timer_10(sf::Vector2f(75,100));
    sf::RectangleShape timer_100(sf::Vector2f(75,100));
    timer_100.setTexture(&digits_txt);
    timer_10.setTexture(&digits_txt);
    timer_1.setTexture(&digits_txt);
    timer_1.setTextureRect(digits[0]);
    timer_10.setTextureRect(digits[0]);
    timer_100.setTextureRect(digits[0]);
    timer_100.setPosition(sf::Vector2f(0,0));
    timer_10.setPosition(sf::Vector2f(75,0));
    timer_1.setPosition(sf::Vector2f(150,0));
    

    // setting up the slider, this is used to select the amount of mines to be placed when the next reset happens
    auto slider = tgui::Slider::create();
    slider -> setRotation(-90);
    slider->setPosition("80%","95%");
    slider->setSize("80%", 20);
    slider->setMinimum(0);
    slider->setMaximum(99);
    slider->setValue(20);
    slider->setStep(1);
    slider->onValueChange([](float newValue) {
        amount = static_cast<int>(newValue);
        std::cout << "Slider value: " << newValue << std::endl;
    });
    gui.add(slider, "Bombamount");
    
    // setting up the smiley / reset button
    auto button = tgui::Button::create();
    button -> getRenderer() -> setTexture(smile_alive);
    button -> setPosition("45%","0%");
    button -> setSize("10%","10%");
    button -> onClick([button]{
       reset(button);
    });
    gui.add(button, "Resetbutton");
    

    // setting up the hitbox, this hitbox is used to make sure the grid is only interacted when the hitbox interesects the grid, it follows the mouse at all times
    sf::RectangleShape m_hitbox;
    m_hitbox.setSize(sf::Vector2f(10,10));
    m_hitbox.setFillColor(sf::Color::Transparent);
    
    // setting up the explosion textures
    sf::FileInputStream exp_str;
    exp_str.open("explosion_atlas.png");
    exp_atlas.loadFromStream(exp_str);
    load_explosion();
    exp_sprite.setTexture(&exp_atlas);
    exp_sprite.setFillColor(sf::Color::Transparent);
    


    // setting up the main textures
    atlas.loadFromFile("textures.png");
    load_textures(atlas);
    bool mousereleased = true;
    bool mouseongrid = false;

    // initiate first reset
    reset(button);
    reset(button);

    // main game loop
    while (window.isOpen()) {
        // bool for checking if the mouse is on the grid
        mouseongrid = false;
        
        // lines for getting the explosion and hitbox to follow the mouse
        m_hitbox.setPosition(sf::Vector2f(sf::Mouse::getPosition(window)));
        if(!pause){
           exp_sprite.setPosition(sf::Vector2f(sf::Mouse::getPosition(window)) - sf::Vector2f(100,100));
        }

        // checking if the mouses hitbox intersects any cell of te grid, therefore the mouse is on the grid
        for(int x = 0; x < GRID_WIDTH; x++){
            for(int y = 0; y < GRID_HEIGHT; y++){
                if(cells[{x,y}].shape.getGlobalBounds().intersects(m_hitbox.getGlobalBounds())){
                   mouseongrid = true;
                }
            }
        }
        // if the game isnt paused the timer is updated
        if(!pause){
            time_recorded = game_clock.getElapsedTime();
            sf::Int64 time_int = time_recorded.asSeconds();
            timer_1.setTextureRect(digits[time_int % 10]);
            timer_10.setTextureRect(digits[(time_int / 10) % 10]);
            timer_100.setTextureRect(digits[time_int / 100]);
        }

        // if statement which is true when the player has used all their flags
        if(flag_amount == 0 and amount < 100){
            bool haswon = true;
            for(int x = 0; x < GRID_WIDTH; x++){
                for(int y = 0; y < GRID_HEIGHT; y++){
                    if(cells[{x,y}].flagged && !cells[{x,y}].mine){
                        haswon = false;
                    }
                }
            }
            // if statement becomes true if all the flagged cells alos are a mine, hence meaning the player has won
            if(haswon){
                pause = true;
                button -> getRenderer() -> setTexture(smile_cool);
                for(int x = 0; x < GRID_WIDTH; x++){
                    for(int y = 0; y < GRID_HEIGHT; y++){
                        if(!cells[{x,y}].flagged){
                            cells[{x,y}].shape.setTextureRect(textures[int_to_string(cells[{x,y}].adj_mines)]);
                        }
                    }
                }
            }
        }

        // updating the sprite for showing the selcted amount chosen with the slider
        if(amount < 100){
            select_amount_1.setTextureRect(digits[amount%10]);
            select_amount_10.setTextureRect(digits[amount / 10]);
        }


        // handling events
        sf::Event event;
        while (window.pollEvent(event)) {
            gui.handleEvent(event);

            // close window on close button or esc key press
            if(event.type == sf::Event::Closed or sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)){
                window.close();
            }
            
            // check for a mouse press
            if(event.type == sf::Event::MouseButtonPressed and mousereleased && !pause && mouseongrid){

                // assumption that the mouse is being pressed down
                mousereleased = false;

                // get mouse position and translate it to the grids coords
                sf::Vector2i m_pos = sf::Mouse::getPosition(window);
                m_pos -= sf::Vector2i(0,150);
                int gridx = m_pos.x / CELL_SIZE;
                int gridy = m_pos.y / CELL_SIZE;

                // if it is the left mouse pressed the selected cell will be sweeped
                if(event.mouseButton.button == sf::Mouse::Left && !(cells[{gridx,gridy}].flagged)){

                   // if the sweeped cell is a mine, game_over function will run as well as the button texture changing and eplosion sound and animation playing
                   if(cells[{gridx,gridy}].mine){game_over();button -> getRenderer() -> setTexture(smile_dead); exp_sound.play();animate_exp();}
                   
                   // or if the cell is a blank with no adjacent mines the flood_blanks function will be ran to reveal adjacent chaining blanks
                   else if(cells[{gridx,gridy}].adj_mines == 0){flood_blanks(gridx, gridy);}

                   // and if all are false then the selected cell has mines adjacent to it so only that cell is revealed with the number of mines adjacent being displayed
                   else{cells[{gridx,gridy}].shape.setTextureRect(textures[int_to_string(cells[{gridx, gridy}].adj_mines)]);}
                }

                // if is the right mouse pressed then the selected cell will either be flagged or deflaggs
                else if(event.mouseButton.button == sf::Mouse::Right && cells[{gridx,gridy}].shape.getTextureRect() == textures["Null"] or cells[{gridx, gridy}].shape.getTextureRect() == textures["Flag"]){

                    // deflag the cell, increase the amount of flags the player has left ot place and returning the cell to a null texture
                    if(cells[{gridx,gridy}].flagged){
                        cells[{gridx,gridy}].shape.setTextureRect(textures["Null"]);
                        cells[{gridx,gridy}].flagged = false;
                        flag_amount += 1;
                    }

                    // flag the cell, decreasing the amount of flags the player has left to play and setting the cells texture to a flag
                    else if(!cells[{gridx,gridy}].flagged && flag_amount != 0){
                        cells[{gridx,gridy}].shape.setTextureRect(textures["Flag"]);
                        cells[{gridx,gridy}].flagged = true;
                        flag_amount -= 1;
                    }
                        // update sprites that display the amount of flags left to place
                        flags_left_1.setTextureRect(digits[flag_amount%10]);
                        flags_left_10.setTextureRect(digits[fdiv(flag_amount,10)]);
                }
            }

            // when the mouse button is released mousereleased turns to true, this prevents the player from drag sweeping the grid
            if(event.type == sf::Event::MouseButtonReleased){
                mousereleased = true;
            }
        }

        // lines for clearing the window and drawing all sprtes and widgets onto the window to be displayed in the next frame
        sf::Color background(84,84,84);
        window.clear(background);
        window.draw(m_hitbox);
        for (auto& pair : cells) {
          if(pair.second.shape.getPosition() != sf::Vector2f(0,0)){
            window.draw(pair.second.shape);
          }
        }
        window.draw(flags_left_1);
        window.draw(flags_left_10);
        window.draw(timer_1);
        window.draw(timer_10);
        window.draw(timer_100);
        window.draw(select_amount_1);
        window.draw(select_amount_10);
        window.draw(exp_sprite);
        gui.draw();
        window.display();
    }
}
