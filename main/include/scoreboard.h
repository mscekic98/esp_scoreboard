#ifndef SCOREBOARD_H
#define SCOREBOARD_H

typedef char player_alias[3 + 1]; // +1 for the null termination, 3 setters for player short alias
typedef player_alias team_alias;
typedef int player_id;
typedef int player_elo;

typedef enum {BO3 = 3, BO5 = 5, BO7 = 7} set_number_enum;
typedef enum {set_to_11 = 11, set_to_21 = 21} set_end_enum;
typedef enum {singles = 1, doubles = 2} match_type_enum;
typedef enum {challenger = 1, defender = 2} team_side_enum;
typedef enum {not_official = 0, official = 1} official_status_enum;

typedef struct {
    player_id pid;
    player_alias alias;
    player_elo elo;
} player_struct;

typedef struct
{
    player_struct player1;
    player_struct player2;
    team_alias alias;
} team_struct;

typedef struct {
    team_struct winning_team;
    int challenger_num_points;
    int defender_num_points;
} set_score_struct;

typedef struct {
    player_struct player_serving;
    int current_set_num;
    set_score_struct current_display_score;
    int challanger_num_of_won_sets;
    int defender_num_of_won_sets;
} match_progress_struct;

typedef struct t{
    set_score_struct set_result_array[7];
} match_result_struct;

typedef struct {
    official_status_enum is_official;
    match_result_struct match_result;
    team_struct challenger_team;
    team_struct defender_team;
    set_number_enum number_of_sets;
    match_type_enum match_type;
    set_end_enum set_end;
    match_progress_struct match_progress;
} match_struct;


int initalize_scoreboard(void);
void print_scoreboard_info(void);
void print_current_score(void);
void increment_score(team_side_enum player_side);
int set_match_type(match_type_enum match_type);
int setup_match_team(team_struct team, team_side_enum side);
int set_official_status(official_status_enum official_status);
int set_serving_side(int player_num);


#endif