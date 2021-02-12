#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "neslib.h"
#include <nes.h>
//#link "chr_generic.s"
//#link "fruit_background.s"

//#link "famitone2.s"
void __fastcall__ famitone_update(void);
//#link "effects.s"
extern char effects[];
extern const byte fruit_background_pal[16];
extern const byte fruit_background_rle[];


/*******************META SPRITES FOR PLAYER AND ENEMY************************************************************/

// define a 2x2 metasprite
#define DEF_METASPRITE_2x2(name,code,pal)\
const unsigned char name[]={\
        0,      0,      (code)+0,   pal, \
        0,      8,      (code)+1,   pal, \
        8,      0,      (code)+2,   pal, \
        8,      8,      (code)+3,   pal, \
        128};

// define a 2x2 metasprite, flipped horizontally
#define DEF_METASPRITE_2x2_FLIP(name,code,pal)\
const unsigned char name[]={\
        8,      0,      (code)+0,   (pal)|OAM_FLIP_H, \
        8,      8,      (code)+1,   (pal)|OAM_FLIP_H, \
        0,      0,      (code)+2,   (pal)|OAM_FLIP_H, \
        0,      8,      (code)+3,   (pal)|OAM_FLIP_H, \
        128};

//Metasprite for bear enemy
#define DEF_METASPRITE_B_2x2(name,code,pal)\
const unsigned char name[]={\
        0,      0,      (code)+0,   pal, \
        0,      8,      (code)+1,   pal, \
        8,      0,      (code)+2,   pal, \
        8,      8,      (code)+3,   pal, \
        128};

// define a 2x2 metasprite, flipped horizontally
#define DEF_METASPRITE_B_2x2_FLIP(name,code,pal)\
const unsigned char name[]={\
        8,      0,      (code)+0,   (pal)|OAM_FLIP_H, \
        8,      8,      (code)+1,   (pal)|OAM_FLIP_H, \
        0,      0,      (code)+2,   (pal)|OAM_FLIP_H, \
        0,      8,      (code)+3,   (pal)|OAM_FLIP_H, \
        128};


//Metasprite for bear
DEF_METASPRITE_B_2x2(bearRStand, 0xc4, 0);
DEF_METASPRITE_B_2x2(bearRRun1, 0xc4, 0);
DEF_METASPRITE_B_2x2(bearRRun2, 0xc8, 0);
DEF_METASPRITE_B_2x2(bearRRun3, 0xcc, 0);

//Flipped bear Metasprites
DEF_METASPRITE_B_2x2_FLIP(bearLStand, 0xd8, 0);
DEF_METASPRITE_B_2x2_FLIP(bearLRun1, 0xdc, 0);
DEF_METASPRITE_B_2x2_FLIP(bearLRun2, 0xe0, 0);
DEF_METASPRITE_B_2x2_FLIP(bearLRun3, 0xe4, 0);

//Meta sprite for player 
DEF_METASPRITE_2x2(playerRStand, 0xd8, 0);
DEF_METASPRITE_2x2(playerRRun1, 0xdc, 0);
DEF_METASPRITE_2x2(playerRRun2, 0xe0, 0);
DEF_METASPRITE_2x2(playerRRun3, 0xe4, 0);
DEF_METASPRITE_2x2(playerRJump, 0xe8, 0);
DEF_METASPRITE_2x2(playerRClimb, 0xec, 0);
DEF_METASPRITE_2x2(playerRSad, 0xf0, 0);

DEF_METASPRITE_2x2_FLIP(playerLStand, 0xd8, 0);
DEF_METASPRITE_2x2_FLIP(playerLRun1, 0xdc, 0);
DEF_METASPRITE_2x2_FLIP(playerLRun2, 0xe0, 0);
DEF_METASPRITE_2x2_FLIP(playerLRun3, 0xe4, 0);
DEF_METASPRITE_2x2_FLIP(playerLJump, 0xe8, 0);
DEF_METASPRITE_2x2_FLIP(playerLClimb, 0xec, 0);
DEF_METASPRITE_2x2_FLIP(playerLSad, 0xf0, 0);

DEF_METASPRITE_2x2(personToSave, 0xba, 1);

//Bear movement sequence
const unsigned char* const bearRunSeq[16] = {
  bearLRun1, bearLRun2, bearLRun3, 
  bearLRun1, bearLRun2, bearLRun3, 
  bearLRun1, bearLRun2,
  bearRRun1, bearRRun2, bearRRun3, 
  bearRRun1, bearRRun2, bearRRun3, 
  bearRRun1, bearRRun2,
};

//Player movement sequence
const unsigned char* const playerRunSeq[16] = {
  playerLRun1, playerLRun2, playerLRun3, 
  playerLRun1, playerLRun2, playerLRun3, 
  playerLRun1, playerLRun2,
  playerRRun1, playerRRun2, playerRRun3, 
  playerRRun1, playerRRun2, playerRRun3, 
  playerRRun1, playerRRun2,
};



/****SETTING THE COLOR PALETTE********************************************************************************/
const char PALETTE[32] = { 
  0x21,			// screen color

  0x11,0x2A,0x27,0x00,	// background palette 0
  0x1C,0x20,0x2C,0x00,	// background palette 1
  0x00,0x2A,0x20,0x00,	// background palette 2
  0x06,0x2A,0x26,0x00,	// background palette 3

  0x07,0x10,0x16,0x00,	// sprite palette 0
  0x1B,0x16,0x07,0x00,	// sprite palette 1
  0x0D,0x28,0x3A,0x00,	// sprite palette 2
  0x16,0x27,0x2F	// sprite palette 3
};
/*************************************************************************************************************/


// number of actors (1 per interactive Object)
#define NUM_ACTORS 1
#define NUM_FRUITS 5

// actor x/y positions and speed
byte actor_x[NUM_ACTORS];
byte actor_y[NUM_ACTORS];
sbyte actor_dx[NUM_ACTORS];
sbyte actor_dy[NUM_ACTORS];


// enemy actor
byte enemy_x;
byte enemy_y;
// actor x/y deltas per frame (signed)
sbyte enemy_dx;
sbyte enemy_dy;

typedef struct Fruit{
  bool falling;
  byte _x;		// fruit x/y position
  byte _y;		
  sbyte _dx;		// frui x/y deltas per frame (signed)
  sbyte _dy;
  int sprite;
  int points;
  
};

/**************************FUNCTION LIST*********************************************************************/
byte rndint(byte, byte);
void setup_graphics(void);
byte iabs(int x);
void add_grass_tiles(void);
void show_title_screen(const byte*, const byte* );
void fruit_collision(int,);
/************************************************************************************************************/


struct Fruit Fruits[4];
int score;
int lives;


// main program
void main() {
  char i;	// actor index
  char oam_id;	// sprite ID
  char pad;	// controller flags
  char pad2;
  score =0;
  lives = 3;
  
  // Initialize actor fruits
  for(i=0;i<4;i++){		
    Fruits[i].falling = false;		//Controls when fruit falls
    Fruits[i]._x = rndint(20,230);	//X position
    Fruits[i]._y = rndint(15,70);	//Y position
    Fruits[i]._dy = 0;			//Falling Speed
    Fruits[i].sprite = i+19;		//Sprite used
    Fruits[i].points = i+1;		//Points added when collected
  }
  
  
  //add_grass_tiles();
  show_title_screen(fruit_background_pal,fruit_background_rle);
  setup_graphics();
  famitone_init(effects);
  sfx_init(effects);
  nmi_set_callback(famitone_update);
  
  //press start to begin, randomize fruit positision while waiting
 // while(1){
 //  pad = pad_trigger(0);
 //  for(i=0;i<4;i++){	
 //   Fruits[i]._x = rndint(20,230);
 //  Fruits[i]._y = rndint(15,70);
 // }
 //  if(pad & PAD_START)
 //  {
 //     break;
 //   }
 // };
  

  //Place the player in the middle of the screen
    actor_x[0] = 120;
    actor_y[0] = 191;
    actor_dx[0] = 0;
    actor_dy[0] = 0;
  
  
  //Bear enemy on the corner of the screen
    enemy_x = 0;
    enemy_y = 100;
    enemy_dx = 2;
    enemy_dy = 0;

  
  // Initiate Game loop
  while (1) {
    // start with OAMid/sprite 0
    oam_id = 0;
    
    // set player 0/1 velocity based on controller
    for (i=0; i<1; i++) {
      // poll controller i (0-1)
      pad2 = pad_trigger(i);
      pad = pad_poll(i);
      
      if (pad&PAD_LEFT && actor_x[i]>0) actor_dx[i]=-2;		//Moves player to the left until hits screen border
      else if (pad&PAD_RIGHT && actor_x[i]<240) actor_dx[i]=2;	//Moves player to the right until hits screen border
      else actor_dx[i]=0;					//Else horizontal acceleration = 0
      
      if (pad2 & PAD_A &&  actor_y[i] == 191)			//Prototype jumping
      { 
        actor_dy[i]=-2;
      }
            
      
    }
    //Fall after Jumping to certain height;
    if (actor_y[0] == 155)
        actor_dy[0] = 2;
    
    //Drawing Player character
    for (i=0; i<NUM_ACTORS; i++) {
      byte runseq = actor_x[i] & 7;
      if (actor_dx[0] >= 0)
        runseq += 8;
      oam_id = oam_meta_spr(actor_x[i], actor_y[i], oam_id, playerRunSeq[runseq]);
      actor_x[i] += actor_dx[i];
      //Protoype for jumping
      if(actor_y[i] <= 191)
      actor_y[i] += actor_dy[i];
      //Set actor back on Plane after jumping if he falls too far
      if(actor_y[i] >= 191)
        actor_y[i] = 191;
     
    }
    
    //Drawing BEAR enemy
    if(score > 10){
      enemy_y=191;
    for (i=0; i<1; i++) {
      byte runseq = enemy_x & 7;
      if (enemy_dx >= 0)
        runseq += 8;
      oam_id = oam_meta_spr(enemy_x, enemy_y, oam_id, bearRunSeq[runseq]);
      enemy_x += enemy_dx;
    }
    }
    
    //Draws and updates hearts for lives
    for(i=0;i<lives;i++)
      oam_id = oam_spr(10+(i*10), 10, 23, 1, oam_id);

    //Draws and updates Scoreboard
    oam_id = oam_spr(232, 10, (score/10%10)+48, 2, oam_id);
    oam_id = oam_spr(240, 10, (score%10)+48, 2, oam_id);
    
    for(i = 0; i<4; i++)
     if(Fruits[i].sprite==20)
     oam_id = oam_spr(Fruits[i]._x, Fruits[i]._y, Fruits[i].sprite, 2, oam_id);
    else
      oam_id = oam_spr(Fruits[i]._x, Fruits[i]._y, Fruits[i].sprite, 1, oam_id);
      

    for(i=0;i<4;i++){
      if(rndint(1,200)==1)		//Fruit Hangs on tree for random set of rime
        Fruits[i].falling = true;
      
      if(Fruits[i].falling)		//Set Fruit Fall speed 
      	Fruits[i]._dy = rndint(1,3);
        
      Fruits[i]._y += Fruits[i]._dy;	//Make Fruit Fall
      fruit_collision(i);		// Check Collsion with Player
    }	
    
    //Enemy bear collision
      if(enemy_y >= 210 || ((enemy_x >= actor_x[0]-4 && enemy_x <= actor_x[0]+8)&& (enemy_y >= actor_y[0]-2 && enemy_y <= actor_y[0]+4))){
       lives--;
       sfx_play(2,0);
       enemy_x = 0; 
      }
    
    // hide rest of sprites
    // if we haven't wrapped oam_id around to 0
    if (oam_id!=0) oam_hide_rest(oam_id);
    // wait for next frame
    ppu_wait_frame();

    
      if (lives == 0){
      sfx_play(0,0);
      break;
    }
  }
  while(1){}
}

/***********************************************************************************************/
void setup_graphics() {
  // clear sprites
  oam_hide_rest(0);
  // set palette colors
  pal_all(PALETTE);
  // turn on PPU
  ppu_on_all();
}

/********************************************/
byte rndint(byte a, byte b){
  return (rand() % (b-a)) + a;
}
/*******************************************/
void show_title_screen(const byte* pal, const byte* rle) {
  // disable rendering
  ppu_off();
  // set palette, virtual bright to 0 (total black)
  pal_bg(pal);
  // unpack nametable into the VRAM
  vram_adr(0x2000);
  vram_unrle(rle);
  // enable rendering

}


void fruit_collision(int f){
  if(Fruits[f]._y >= 210 || ((Fruits[f]._x >= actor_x[0]-4 && Fruits[f]._x <= actor_x[0]+8)&& (Fruits[f]._y >= actor_y[0]-2 && Fruits[f]._y <= actor_y[0]+4))) //hits floor or collision detected
      {
        if(Fruits[f]._y < 195){
          sfx_play(1,1);
          score += Fruits[f].points;
          Fruits[f]._y = 210;
        }
        Fruits[f].sprite=0; // erase fruit for a bit
      }
  
  if(Fruits[f].sprite == 0 && Fruits[f]._y <= 160 && Fruits[f]._y >= 130 ){// Make fruit reappear on random spot
       	Fruits[f]._x = rndint(20,230);
    	Fruits[f]._y = rndint(10,50);
        Fruits[f]._dy = 0;
    
    //set sprite before reappearing
    switch(f){
      case 0:
        Fruits[f].sprite=19;
        Fruits[f].falling=false;
        break;
      case 1:
        Fruits[f].sprite=20;
        Fruits[f].falling=false;
        break;
      case 2:
        Fruits[f].sprite=21;
        Fruits[f].falling=false;
        break;
      case 3:
        Fruits[f].sprite= 22;
        Fruits[f].falling=false;
        break;
      default:
        break;
 	}
  }
  

}