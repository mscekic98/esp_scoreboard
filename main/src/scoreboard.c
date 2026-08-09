#include <stdlib.h>
#include "scoreboard.h"
#include "common.h"


static player_struct player1 = {
    .alias = "P01",
    .pid = 1,
    .elo = 0
};
static player_struct player2 = {
    .alias = "P02",
    .pid = 2,
    .elo = 0
};
static player_struct player3 = {
    .alias = "P03",
    .pid = 3,
    .elo = 0
};
static player_struct player4 = {
    .alias = "P04",
    .pid = 4,
    .elo = 0
};

static team_struct team1 = {
    .alias = "T01"
};

static team_struct team2= {
    .alias = "T02"
};

static match_struct match;

static int is_set_over(){
    int challenger_points = match.match_progress.current_display_score.challenger_num_points;
    int defender_points = match.match_progress.current_display_score.defender_num_points;
    if((challenger_points > match.set_end) || ( defender_points > match.set_end)){
        if(abs(challenger_points - defender_points) >= 2){
            return 1;
        }
    }
    return 0;
}

static void store_set_result(){
    int challenger_points = match.match_progress.current_display_score.challenger_num_points;
    int defender_points = match.match_progress.current_display_score.defender_num_points;
    match.match_result.set_result_array[match.match_progress.current_set_num -1].challenger_num_points = challenger_points;
    match.match_result.set_result_array[match.match_progress.current_set_num -1].defender_num_points = defender_points;
    if(challenger_points > defender_points){
        match.match_result.set_result_array[match.match_progress.current_set_num -1].winning_team = match.challenger_team;
        match.match_progress.challanger_num_of_won_sets++;
    }else{
        match.match_result.set_result_array[match.match_progress.current_set_num -1].winning_team = match.defender_team;
        match.match_progress.defender_num_of_won_sets++;
    }

    match.match_progress.current_set_num++;
    match.match_result.number_of_sets_played++;

}

static int is_match_over(){
    int required_sets_for_win;
    int challenger_num_of_sets = match.match_progress.challanger_num_of_won_sets;
    int defender_num_of_sets = match.match_progress.defender_num_of_won_sets;

    switch (match.number_of_sets)
    {
    case BO3:
        required_sets_for_win = 2;
        break;

    case BO5:
        required_sets_for_win = 3;
        break;

    case BO7:
        required_sets_for_win = 4;
        break;
    
    default:
        required_sets_for_win = 3;
        break;
    }

    if((challenger_num_of_sets == required_sets_for_win) || (defender_num_of_sets == required_sets_for_win)){
        return 1;
    }

    return 0;
}

static void set_serving_player(player_struct player){
    match.match_progress.player_serving = player;
}

static int get_player_side_number(player_struct player){
    if(match.match_type == singles){
        if(player.pid == match.challenger_team.player1.pid){
            return 1;
        }else{
            return 2;
        }
    }else{
        if(player.pid == match.challenger_team.player1.pid){
            return 1;
        }else if(player.pid == match.challenger_team.player2.pid){
            return 2;
        }else if(player.pid == match.defender_team.player1.pid){
            return 3;
        }else{
            return 4;
        }
    }

    return 0;
}

static void update_serving_player(){
    int current_serving_num = get_player_side_number(match.match_progress.player_serving);
    if((match.match_progress.current_display_score.challenger_num_points + match.match_progress.current_display_score.defender_num_points) % 2 == 0){    
        if(current_serving_num){
            if(match.match_type == singles){
                if(current_serving_num == 1){
                    set_serving_side(2);
                }else{
                    set_serving_side(1);
                }
            }else{
                switch (current_serving_num)
                {
                case 1:
                    set_serving_side(3);
                    break;

                case 2:
                    set_serving_side(4);
                    break;

                case 3:
                    set_serving_side(2);
                    break;

                case 4:
                    set_serving_side(1);
                    break;
                
                default:
                    ESP_LOGE(TAG, "NOT VALID SERVING_PLAYER NUM!");
                    break;
                }
            }
        }else{
            ESP_LOGE(TAG, "INVALID SERVING PLAYER!");
        }
    }
}

void print_scoreboard_info(void){
    ESP_LOGI(TAG, "Current challenger no of points %d", match.match_progress.current_display_score.challenger_num_points);
    ESP_LOGI(TAG, "Current defender no of points %d", match.match_progress.current_display_score.defender_num_points);
    ESP_LOGI(TAG, "Current set num %d ", match.match_progress.current_set_num);
    ESP_LOGI(TAG, "Player 1 name is %s", match.challenger_team.player1.alias);
    ESP_LOGI(TAG, "Player 2 name is %s", match.defender_team.player1.alias);
    ESP_LOGI(TAG, "Team 1 name is %s", match.challenger_team.alias);
    ESP_LOGI(TAG, "Team 2 name is %s", match.defender_team.alias);
    ESP_LOGI(TAG, "Player 1 number of won sets is %d", match.match_progress.challanger_num_of_won_sets);
    ESP_LOGI(TAG, "Player 2 number of won sets is %d", match.match_progress.defender_num_of_won_sets);
    ESP_LOGI(TAG, "Match type is %d", match.match_type);
    ESP_LOGI(TAG, "Set lasts to %d points", match.set_end);
}

void print_current_score(void){
    ESP_LOGI(TAG, "Team 1 name is %s", match.challenger_team.alias);
    ESP_LOGI(TAG, "Team 2 name is %s", match.defender_team.alias);
    ESP_LOGI(TAG, "Current challenger no of points %d", match.match_progress.current_display_score.challenger_num_points);
    ESP_LOGI(TAG, "Current defender no of points %d", match.match_progress.current_display_score.defender_num_points);
    ESP_LOGI(TAG, "The player serving is %s", match.match_progress.player_serving.alias);
    ESP_LOGI(TAG, "The total number of sets played is %d", match.match_result.number_of_sets_played);
}

int initalize_scoreboard(void){

    team1.player1 = player1;
    team2.player1 = player2;

    match.is_official = not_official;
    match.challenger_team = team1;
    match.defender_team = team2;
    match.match_type = singles;
    match.number_of_sets = BO5;
    match.set_end = set_to_11;

    match.match_progress.current_display_score.challenger_num_points = 0;
    match.match_progress.current_display_score.defender_num_points = 0;
    match.match_progress.current_set_num = 1;
    match.match_progress.challanger_num_of_won_sets = 0;
    match.match_progress.defender_num_of_won_sets= 0;
    match.match_progress.player_serving = match.challenger_team.player1;
    match.match_result.number_of_sets_played = 0;

    //Show P01 and P02 by default
    print_scoreboard_info();

    return 0;
}

void increment_score(team_side_enum player_side){

    int operating_score = 0;
    switch (player_side)
    {
    case challenger:
        match.match_progress.current_display_score.challenger_num_points++;
        break;

    case defender:
        match.match_progress.current_display_score.defender_num_points++;
        break;
    
    default:
        break;
    }

    update_serving_player();
    if(is_set_over()){
        store_set_result();
    }

    if(is_match_over()){
        ESP_LOGI(TAG, "MATCH IS OVER!");
    }
    
}

int set_match_type(match_type_enum match_type){
    match.match_type = match_type;
    return 1;
}


int setup_match_team(team_struct team, team_side_enum side){
    switch (side)
    {
    case challenger:
        if(match.match_type == doubles){
            match.challenger_team = team;
        }else{
            match.challenger_team.player1 = team.player1;
        }
        return 1;
        break;

    case defender:
        if(match.match_type == doubles){
            match.defender_team = team;
        }else{
            match.defender_team.player1 = team.player1;
        }
        return 1;
        break;
    
    default:
        return 0;
        break;
    }

    return 0;
}

int set_official_status(official_status_enum official_status){
    match.is_official = official_status;
    return 1;
}

int set_serving_side(int player_num){
    if((player_num<1) || (player_num >4) ){
        return 0;
    }else{
        if((match.match_type == singles) && (player_num > 2)){
            return 0;
        }else{
            if(match.match_type == singles){
                if(player_num == 1){
                    set_serving_player(match.challenger_team.player1);
                }else{
                    set_serving_player(match.defender_team.player1);
                }
                return 1;
            }else{
                switch (player_num)
                {
                case 1:
                    set_serving_player(match.challenger_team.player1);
                    break;

                case 2:
                    set_serving_player(match.challenger_team.player2);
                    break;

                case 3:
                    set_serving_player(match.defender_team.player1);
                    break;

                case 4:
                    set_serving_player(match.defender_team.player2);
                    break;
                
                default:
                    break;
                }
                return 1;
            }
        }
    }

    return 0;
}

match_result_struct get_current_match_result(){
    return match.match_result;
}

int set_match_result(match_result_struct match_result){
    if((match_result.number_of_sets_played <= 7) && (match_result.number_of_sets_played >0)){
        match.match_result = match_result;
        return 1;
    }
    return 0;
}